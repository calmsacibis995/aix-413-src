static char sccsid[] = "@(#)84  1.13  src/bos/usr/sbin/devinstall/devinstall.c, cmdcfg, bos412, 9448A 11/23/94 15:33:06";
/*
 *   COMPONENT_NAME: (CMDCFG) 
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/priv.h>	/* privilege */
#include <sys/audit.h>	/* auditing */
#include <sys/cfgodm.h> /* classes structure dcls */
#include <sys/cfgdb.h>

#include <nl_types.h>
#include <locale.h>

/*---------------------------------- device packaging ----------------------*/
#define RC_SIZE		32			/* return code field size in installp.summary */
#define FNAME_SIZE	256			/* file name size */
#define BUF_SIZE	256			/* buffer size */
#define LANG_SIZE	48			/* size of language name string */
#define	DEVPKGFILE	"/tmp/device.pkgs"
#define	DIAGFILE	"/usr/sys/inst.data/sys_bundles/Hdwr-Diag.def"
#define LOGFILE		"/var/adm/ras/devinst.log"
#define	DEVPKGFAIL	"/var/adm/dev_pkg.fail"
#define INSTALLP_CMD	"/usr/sbin/installp"
#define INSTALLP_ARGS	" -acqNXQg -e /var/adm/ras/devinst.log "
#define INSTALLPSUMMARY	"/var/adm/sw/installp.summary"
#define	ALREADY_INSTALLED	402		/* code in installp.summary */ 
#define	DEVPKG_PREFIX	"devices."		/* device package prefix */
#define	BOSPKG_PREFIX	"bos."		/* bos package prefix */
#define	BOSINST_DATA	"/var/adm/ras/bosinst.data"	
#define	DEVPKG_MSG_FILE	"/tmp/device.pkgs.msgs"

/*---------------------------------- status codes ---------------------------*/
#define	NEW_NAME	1
#define	OLD_NAME	2
#define	DEL_NAME	3

/*---------------------------------- exit status ---------------------------*/
#define EXIT_NOT	-1
#define EXIT_OK		0
#define EXIT_FATAL	2

/*--------------------------------- msg indices ----------------------------*/

#define	DEVINST_CATALOG	"devinstall.cat"

#define MSG_SET		1		/* message set number */

#define	MSG_INSTALL	1
#define	MSG_DIAGNOS	2
#define	MSG_PKGFAIL	3

char *msgstr[] =
{
"",
"running installp command with args %s.\n",
"updating diagnostic bundle file %s\n",
"updating failed device packages file %s\n"
};

#define	ERR_SET		2		/* message set number */

#define	ERR_USAGE	1
#define ERR_MEM		2
#define	ERR_FOPEN	3
#define	ERR_FWRITE	4
#define	ERR_FREAD	5
#define ERR_ODM_INIT	6
#define ERR_GET_PDAT	7

char *errstr[] =
{
"",
"devinstall: 0514-700 Usage error :\n\tUsage: devinstall -f file -d device [-s] [-v]\n",
"devinstall: 0514-710 There is not enough memory available now.\n",
"devinstall: 0514-720 Unable to open the file %s\n",
"devinstall: 0514-730 Unable to write to the file %s\n",
"devinstall: 0514-740 Unable to read the file %s\n",
"devinstall: 0514-750 Cannot initialize the ODM data base.\n",
"devinstall: 0514-760 Cannot access the PdAt object class in the\n\
\tdevice configuration database.\n"
};


/*--------------------------------- global vars -----------------------------*/
extern	int	errno;			/* errno */

struct	pkgname {
	char	name[FNAME_SIZE];
	int	status;
	struct pkgname *next;
}	*pkglist, 			/* input package list */
	*badpkglist ;			/* bad package list   */

char	pkgfile[FNAME_SIZE];		/* given file name with package list */
char	instdev[FNAME_SIZE];		/* install device name */
char	*argfile;			/* installp argument file */
int	*devinstall_cat = -1;
int	verbose = 0;
int	runtime = 0;
int	suppress_bosboot = 0;

char 	*nlsmsg();

/*
* NAME:  Main
*
* FUNCTION: Main program of devinstall command
*
* EXECUTION ENVIRONMENT:  User level command with privilege acquired
*
* RETURN VALUE DESCRIPTION:  exit code = 0 successful execution 
*			     otherwise return exit code of installp or
*			     -1 
*/

main(argc, argv)
int	argc;
char	*argv[];
{
	extern char *optarg;				/* getopt */
	extern int optind;				/* getopt */
	int	   c,rc,cleanup;
	int        return_code = 0;
	char	   argsbuf[FNAME_SIZE * 3];	/* room for 2 file names + arguments */
	char	   args[FNAME_SIZE * 3];	/* room for 2 file names + arguments */

	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);
	privilege(PRIV_ACQUIRE);

	setlocale(LC_ALL,"");
	devinstall_cat = (int *) catopen(DEVINST_CATALOG,0);

	pkgfile[0] = instdev[0] = '\0';
	pkglist = badpkglist = NULL;
	cleanup = 0;

	while (( c = getopt(argc, argv, "d:f:bsv")) != EOF ) {
	switch (c)
	   {  		case 's' :	cleanup = 1;
					break;	
	      		case 'f' :	strncpy(pkgfile,optarg,strlen(optarg));
					argfile = pkgfile;
                          		break;
			case 'd' :	strncpy(instdev,optarg,strlen(optarg));
                          		break;
			case 'v' :	verbose = 1;
					break;
	     		case 'b' :	suppress_bosboot = 1;
					break;
	      		default  :	showerr(ERR_USAGE,"",EXIT_FATAL);	
			 		break;
	   } 
	}

	runtime = get_runtime();

	if(cleanup) {
	     unlink(DEVPKGFAIL);
	     if (runtime)
		unlink(LOGFILE);
	}

	/* if runtime, check language in BOSINST_DATA */
	if( runtime ) {
		if(check_lang(pkgfile))
			argfile = DEVPKG_MSG_FILE;
	}

	/* read in the list of package names from the pkgfile specified */
	read_pkgfile(argfile, &pkglist);

	/* Run installp with given package list file which
	 	is in argsbuf */

	/* Pre-pend the suppress bosboot flag, if specified */
	if (suppress_bosboot)
		sprintf(args, "%s%s", " -b", INSTALLP_ARGS);
	else
		strcpy(args, INSTALLP_ARGS);
	
	if ( verbose )
	{
		sprintf(argsbuf,"%s -d%s -f%s",args,instdev,argfile);
		printf(catgets(devinstall_cat,MSG_SET,MSG_INSTALL,msgstr[MSG_INSTALL]), argsbuf);
	}
	else
		sprintf(argsbuf,"-V0 %s -d%s -f%s",args,instdev,argfile);

	rc = odm_run_method(INSTALLP_CMD,argsbuf,NULL,NULL);
	
	/* check the installp.summary file and
	 * generate the list of packages which
	 * has failed installation.
	 */
        return_code = chk_summary();

	/* Check failed package list against installed list */
	cmp_list(pkglist, badpkglist);

	/* Update the diagnostics bundle file */
	if( verbose ) 
		printf(catgets(devinstall_cat,MSG_SET,MSG_DIAGNOS,msgstr[MSG_DIAGNOS]),DIAGFILE);


	update_file(DIAGFILE,pkglist);

	if( verbose ) 
		printf(catgets(devinstall_cat,MSG_SET,MSG_PKGFAIL,msgstr[MSG_PKGFAIL]),DEVPKGFAIL);


	if(cleanup) 
		write_file(DEVPKGFAIL,badpkglist);
	else 
		update_file(DEVPKGFAIL,badpkglist);

	if( runtime ) 
		unlink( DEVPKG_MSG_FILE ); 
	privilege(PRIV_LAPSE);
	exit(return_code);
} 

/* NAME:  get_runtime
*
*
*/
int
get_runtime()
{
	if(!strcmp(getenv("SYSCFG_PHASE"), "")) return (1); 
	return(0);
}


/*
* NAME:  read_pkgfile
*
* FUNCTION: read device package names from input file
*	    and build up a lit of names for later reference.
*
* INPUT PARAMETERS:	input file name, list header
*
* RETURN VALUE DESCRIPTION:  exit if error occurs in open or fgets 
*
* EXTERNAL PROCEDURES CALLED: none.
*/
	
read_pkgfile(pfile,list)
char *pfile;
struct pkgname **list;
{
	FILE *fp;
	int rc;
	char cp[FNAME_SIZE+1];

	if( (fp = fopen(pfile,"r")) == NULL) 
		showerr(ERR_FOPEN,pfile,EXIT_FATAL);

	pkglist = NULL;
	while( (fgets(cp,FNAME_SIZE,fp)) > 0 ) {
	    if( cp[ strlen(cp)-1 ] == '\n')
			cp[ strlen(cp)-1 ] = '\0';	
	    addpkglist(cp,list);
	}

	if( ferror(fp) )
		showerr(ERR_FREAD,pfile,EXIT_FATAL);
	fclose(fp);
}


/*
* NAME:  addpkglist
*
* FUNCTION: add a package name to the list.
*	   It makes sure the same name has not been
*	   in the list and then add the name in the
*	   alphabetically sorted order. 
*
* INPUT PARAMTERS: package name, list header
*
* RETURN VALUE DESCRIPTION:  exit when malloc failed
*
* EXTERNAL PROCEDURES CALLED:
*
*/

addpkglist(name, pkgl)
char *name;
struct pkgname **pkgl;
{
    struct pkgname *node, *new_node, *prev_node;
    int rc;

	node = *pkgl;		/* list header */
	prev_node = NULL;	/* previous node */
	rc = -1;		/* non-zero value */
		
    	/* loop until one of the return conditions is met */
	while( 1 ) 
	{

	    /* If the list is null OR if we have found the 
		correct place in the list, then insert this name */
	    if ( !node || (rc = strcmp(node->name, name)) >= 0 ) 
	    {
		/* if strcmp returned 0, then this name is already in the list */
		if (!rc) 
			return; 	/* duplicate node */

		/* not a duplicate - we need to create a new one */
	        new_node = (struct pkgname *) malloc( sizeof( struct pkgname) );
		if( !new_node )
			showerr(ERR_MEM,NULL,EXIT_FATAL);

		/* fill in the new node with data */
		strcpy( new_node->name, name);
		new_node->status = NEW_NAME;	/* marked new node */

		/* make the new node point to the node we found
			with strcmp - its alphabetical antecedent */
		new_node->next = node;

		/* if the node is going to be the first in the list */
		/* set up package list and return.		    */
		if( !*pkgl || node == *pkgl ) 	/* 1st node */
		{ 
			*pkgl = new_node;
			return;
		}
		
		/* set up the prev_node pointer to hook the new node in */
		prev_node->next = new_node;
		return;
	    }

	    /* we need to loop again, so set up the previous node pointer 
		to point to the one we're currently looking at....  */
	    prev_node = node;	

	    /* ...and examine the next node in the list */
	    node  = node->next;
	} 
}		

/*
* NAME:  update_file
*
* FUNCTION: Update the file with the package names
*	    from the given list.  The given file is
*	    read, checked and the duplicates marked 
*	    so that they would not be appended to
*	    the file. 
*
* INPUT PARAMETERS : name of file to update, header of package list
*
* RETURN VALUE DESCRIPTION:  exit if openi, read or write error.
*
* EXTERNAL PROCEDURES CALLED:
*
*/
	
update_file(fname, list)
char *fname;
struct pkgname *list;
{
	FILE *fp;
	char cp[FNAME_SIZE];
	int  rc,ssize;
	struct pkgname *p;

	/* Open the diagnostic bundle file for append */
	if( (fp = fopen(fname,"a+")) == NULL ) {	
		showerr(ERR_FOPEN,fname,EXIT_NOT);
		return(-1);
	}

	/* Mark duplicates as old names */
	while( (rc = fgets(cp,FNAME_SIZE,fp)) > 0 ) {
		for( p = list; p != NULL; p = p->next )  {
			if( p->status == DEL_NAME ) continue;
			if( !strncmp(p->name,cp,strlen(p->name)))
				p->status = OLD_NAME;
		}
	}
	if( ferror(fp) )
		showerr(ERR_FREAD,fname,EXIT_FATAL);

	fflush(fp);
	
	/* Update the bundle file with new package names */
	ssize = strlen(DEVPKG_PREFIX);
	for( p = list; p !=NULL; p = p->next) {
	/* update the bundle file with only DEVPKG_PREFIX packag names */
		if( (p->status !=  OLD_NAME) && (p->status != DEL_NAME) &&
			!strncmp(p->name,DEVPKG_PREFIX,ssize) ) {
			if( fprintf(fp,"%s\n",p->name) < 0 )
				showerr(ERR_FWRITE,fname,EXIT_FATAL);
		}		
	}
	fclose(fp);
}

/*
* NAME:  chk_summary
*
* FUNCTION: check the installp.summary file and
*	    generate the list of packages which
*	    has failed installation.
*	    It parses the first two field of the
*	    summary file and update the package list
*	    with the status.  If any part of the
*	    failed, that package is marked failed.
*
*
* INPUT PARAMETERS: None.
*
* RETURN VALUE DESCRIPTION:  returns 1
*	if package is successfully installed
*	i.e., if a bosboot needs to be done.
*
* EXTERNAL PROCEDURES CALLED:
*
*/
int	
chk_summary()
{
	FILE    *f;	      /* summay file pointer */
	int	c,i,rc;	
	int	do_bosboot = 0;	 /* flag for tracking need for bosboot */
	char    buf[RC_SIZE];    /* return code buf */
	char	pkg[FNAME_SIZE]; /* name of package */

	if( (f = fopen(INSTALLPSUMMARY,"r")) == NULL)
		showerr(ERR_FOPEN,INSTALLPSUMMARY,EXIT_FATAL);

	/* This loop checks the return code and package name fields only */
	while( (c = fgetc(f)) != EOF) {
		/* scan return code field */
		for( i = 0; !( c == ':' || c == EOF); i++, c = fgetc(f))
			buf[i] = c;
		buf[i] = '\0';
		
		/* scan package name field */
		c = fgetc(f);
		for( i = 0; !( c == ':' || c == EOF); i++, c = fgetc(f))
			pkg[i] = c;
		pkg[i] = '\0';

		rc = atoi(buf);
		/* if zero return code or an already installed package, 
			don't add to bad package list */
        	if( !( !rc || rc == ALREADY_INSTALLED) )
                	addpkglist(pkg,&badpkglist);
	
		/* if rc = 0, then a package was installed 
			and we need to do a bosboot */
		else if (!rc)
			do_bosboot = 1;

		/* skip the remaing fields on the line */
		while(((c = fgetc(f)) != '\n') && (c != EOF));
		if( c == EOF) break;
   	}
	if(ferror(f))
		showerr(ERR_FREAD,INSTALLPSUMMARY,EXIT_FATAL);
	fclose(f);

	return(do_bosboot);
}

/*
* NAME:  cmp_list
*
* FUNCTION: Compares the input package names and failed package names
*	from two lists.
*	Mark the failed package names from input package list as
*	deleted packages, so that the list will only contain 
*	successfully installed packages.  
*
* INPUT PARAMTERS: input package list, failed package list
*
* RETURN VALUE DESCRIPTION:  None, failed packages are marked DEL_NAME
*			     in their status.
*
* EXTERNAL PROCEDURES CALLED:
*/
	
cmp_list(p1,p2)
struct pkgname *p1, *p2;
{
   struct pkgname *p,*q;

	for( p = p1; p != NULL; p = p->next ) {
		for( q = p2; q != NULL; q = q->next) {
			if( !strncmp( p->name, q->name, strlen(p->name)) ) {
				p->status = DEL_NAME;			 
				break;
			}
		}
	}
}

/*
* NAME:  write_file
*
* FUNCTION: Write the failed package names in dev_pkg.fail
*
* INPUT PARAMTERS: output file name, list of packages 
*
* RETURN VALUE DESCRIPTION:  
*
* EXTERNAL PROCEDURES CALLED:
*
*/

write_file(ofile,list)
char *ofile;
struct pkgname *list;
{
   struct pkgname *p;
   FILE	*f;

      if( (f = fopen(ofile,"w")) == NULL )
	showerr(ERR_FOPEN,ofile,EXIT_FATAL);

      for( p = list; p != NULL; p = p->next) {
	if( p->status != DEL_NAME )
		if( fprintf(f,"%s\n",p->name) < 0 )
			showerr(ERR_FWRITE,ofile,EXIT_FATAL);
      }
	
      fclose(f);
}

/*
* NAME:  showerr
*
* FUNCTION:	display error messages and exit if fatal
*
* EXECUTION ENVIRONMENT:
*		user, privileged mode
*
* DATA STRUCTURES: errstr is used if nls is enabled.
*
* RETURN VALUE DESCRIPTION:  0 to caller if not exiting.
*
* EXTERNAL PROCEDURES CALLED:
*
*/
	
showerr(errnum, arg, exit_status)
int errnum;	/* error number */
char *arg;	/* character to be displayed */
int exit_status;	/* exit status code */
{
	/* write an error message */
	fprintf( stderr, catgets(devinstall_cat, ERR_SET, errnum, errstr[errnum]), arg);
	if (exit_status == EXIT_NOT)
	   return( 0 );
	else
	{  /* assuming fatal error */
		/* close the message catalogs */
		if (devinstall_cat != -1)
			catclose( devinstall_cat );
		if (devinstall_cat != -1)
			catclose( devinstall_cat );
	   /* bye-bye */
	   privilege(PRIV_LAPSE);
	   exit( exit_status );
	}
}

/*
* NAME:  check_lang
*
* FUNCTION:	check the bosinstall language so that correct msg opts can 
*		be included.
*
* EXECUTION ENVIRONMENT:
*		user, privileged mode
*
* DATA STRUCTURES:	Need to read and parse BOSINST_DATA file. 
*
* RETURN VALUE DESCRIPTION:  Note: 
*			returns 0 => use LANG as C.
*			returns 1 => LANG is NOT C 
*
* EXTERNAL PROCEDURES CALLED:
*
*/
	
check_lang( pfile)
char *pfile;
{
	FILE   	*instdata_file;		    /* bos install data file */
	FILE	*orgpkg_file, *modpkg_file; /* package list file pointer */
	char   	*cp;
        char   	buf[BUF_SIZE];    /* temporary buf */
	char	lang[LANG_SIZE];  /* language name., e.g., en_US */
        int    	i, found_locale,len;  

	strcpy(lang,"C");	  /* default language */
        if( (instdata_file = fopen(BOSINST_DATA,"r")) == NULL) {
		return (0);
        }
	found_locale = 0;

	/* parses the bosinst.data file, and retrieve the lang value 
	   from MESSAGES = <lang> in locale stanza. */

        while( (cp = fgets(buf, BUF_SIZE,instdata_file)) != NULL ) {
		if( cp[0] == '#' ) continue;	/* skip comments in first column */
		if( !found_locale )  {
		        cp = strstr( buf, "locale:");
			if( cp == NULL ) continue;
			found_locale = 1;
		}
		else {
			cp = strstr(buf,"MESSAGES" );
			if( cp == NULL ) continue;
			
			cp = strstr( cp, "=" );
			if( cp == NULL ) continue;
			
			len = strlen(cp);
			for( cp++, i = 1; i < len; i++, cp++) 
				if( *cp != ' ' && *cp != '\t' ) break; 

			len = strlen(cp);
			for( i=0; i < len; i++)  {
				if( cp[i] == ' ' ||  cp[i] == '(' ||
				    cp[i] == '\n' || cp[i] == '.' ||  
			            cp[i] == '\0')  break; 
			}
			cp[i] = '\0';
			strcpy(lang,cp);
			break;
		}
	}
	fclose(instdata_file);

	/* if not C, update the pkg list file with 
	   bos.msg.<lang> and devices.msg.<lang> */

	if( strcmp(lang,"C") ) {
        	if( (orgpkg_file = fopen(pfile,"r")) == NULL) {
			return (0);
        	}
        	if( (modpkg_file = fopen(DEVPKG_MSG_FILE,"w")) == NULL) {
			return (0);
        	}
        	while( fgets(buf,BUF_SIZE,orgpkg_file) != NULL ) {
			fputs(buf,modpkg_file);	
		}	
		fclose(orgpkg_file);
		sprintf(buf,"%smsg.%s\n",BOSPKG_PREFIX,lang);
		fputs(buf, modpkg_file);
		sprintf(buf,"%smsg.%s\n",DEVPKG_PREFIX,lang);
		fputs(buf, modpkg_file);
		fclose(modpkg_file);
		return (1);
	}
	return (0);
}
