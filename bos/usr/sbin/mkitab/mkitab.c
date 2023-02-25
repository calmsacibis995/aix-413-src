static char sccsid[] = "@(#)53	1.13.1.1  src/bos/usr/sbin/mkitab/mkitab.c, cmdoper, bos411, 9428A410j 11/22/93 12:31:47";

/*
 * COMPONENT_NAME: (CMDOPER) descriptive name
 *
 * FUNCTIONS: mkitab, chitab, rmitab, lsitab
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<stdio.h>
#include	<errno.h>
#include	<locale.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/acl.h>

#include	<nl_types.h>
#include 	"mkitab_msg.h"
nl_catd		catd;
#define		MSGSTR(Num,Str)	catgets(catd,MS_MKITAB,Num,Str)

#define		IDENT_LEN	14	/*  length of ident field  */
#define		RUNLEVEL_LEN	20	/*  length of runlevel field  */
#define		KEYWORD_LEN	20	/*  length of keyword field  */
#define		COMMAND_LEN	1024	/*  length of command field  */
#define		ENTRY_LEN	1280	/*  length of entry in INITTAB_FILE  */

			/*  cmd values  */
#define		MKITAB		1
#define 	MKITAB_DASHI	2
#define		CHITAB		3
#define		RMITAB		4
#define		LSITAB		5
#define 	LSITAB_DASHA	6

			/*  argv [0] names  */
#define		NAM_MKITAB	"mkitab"
#define		NAM_CHITAB	"chitab"
#define		NAM_RMITAB	"rmitab"
#define		NAM_LSITAB	"lsitab"

#define		INITTAB_FILE	"/etc/inittab"
#define		TEMP_SIZE	sizeof (INITTAB_FILE) + 6
#define		XXXXXX		"XXXXXX"

			/*  usage defines  */
#define	MK_USE	"usage:\t%s \"[-i ident] ident:runlevel:keyword:command\"\n"
#define	CH_USE	"usage:\t%s \"ident:runlevel:keyword:command\"\n"
#define	RM_USE	"usage:\t%s \"ident\"\n"
#define LS_USE  "usage:\t%s <\"ident\"|-a>\n"

			/*  valid entries for runlevel  */
#define		RUNL	"MSabcms0123456789"

char	*arg0 ;		/*  set = argv[0]  */
short	cmd;		/*  MKITAB, CHITAB, RMITAB, LSITAB  */
char	entry [ENTRY_LEN];	/*  entry read from INITTAB_FILE file  */


short	find_entry (char *);
short	getname(char *);
char	*parse_ident (char *);
void	update_inittab(char *,char *);
int	usage ();
void	verify_command (char *);
char	*verify_keyword (char *);
char	*verify_runlevel (char *);
void	do_ls_all ();
struct	acl	*acl_fget (int);


	/*	This command is a high-level SMIT command to manipulate
	*	the /etc/inittab file. This program is compiled as mkitab
	*	and will be linked to chitab, rmitab, and lsitab.
	*
	*	mkitab - adds a record to the inittab file  mkitab "string"
	*	chitab - changes a record in the inittab file  chitab "string"
	*	rmitab - removes a record in the inittab file  rmitab "string"
	*	lsitab - displays a record in the inittab file  lsitab "string"
	*/

main(int argc,char *argv[])
{
	char	*field_ptr;	/*  pointer to current field  */
	char	*ident ;	/*  ident from input line stored  */
	char	*ident_insert ;	/*  new line to insert after ident line */
	short	found = 0;	/*  ident field found in INITTAB_FILE  */

				/*  check for command type  */

	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_MKITAB, NL_CAT_LOCALE);

	arg0 = argv [0] ;
	cmd = getname(argv[0]) ;
				/*  argc must be 2  */
	if ( argc != 2 && argc != 4) {
		fprintf (stderr, MSGSTR (E_FIELD, "%s: Bad command line.\n" )
			, argv [0] );
		usage () ;
	}

				/*  check for valid ident field  */
	field_ptr = parse_ident (argv[1]) ;
	ident = argv[1];

				/*  check for entry in INITTAB_FILE  */

	/* check for '-a' on lsitab */
	if (cmd == LSITAB && !strcmp(argv[1],"-a")) 
	    cmd = LSITAB_DASHA;
	else if (cmd == MKITAB && !strcmp(argv[1],"-i")) {
		field_ptr = parse_ident (argv[2]);
		ident = argv[2];
		cmd = MKITAB_DASHI;
		field_ptr = parse_ident (argv[3]) ;
		ident_insert = argv[3];
	    	found = find_entry (ident) ;
	}
	else 
	    found = find_entry (ident) ;

	switch ( cmd )
	{
		case MKITAB :
		{
			FILE	*fd ;
			short	len ;

			if (found)
			{
				fprintf ( stderr , MSGSTR (E_IDEXIST ,
					"%s: ident entry found in %s\n" ),
					arg0 , INITTAB_FILE );
				exit (1);
			}
			field_ptr = verify_runlevel (field_ptr);
			field_ptr = verify_keyword (field_ptr);
			verify_command (field_ptr);
			fd = fopen ( INITTAB_FILE , "a" ) ;
			if ( fd == NULL )
			{
				fprintf ( stderr , MSGSTR (E_OPEN ,
					"%s: could not open %s\n" ) ,
					arg0 , INITTAB_FILE ) ;
				exit (errno);
			}
			len = strlen (argv [1]) ;
			argv [1] [len++] = '\n' ;
			if (fwrite((void *)argv [1], (size_t)1, (size_t)len, fd) != len )
				fprintf ( stderr, MSGSTR ( E_UPDATE,
					"%s: Unable to update %s: errno = %d.\n"
					), arg0 , INITTAB_FILE , errno );
			fclose(fd);
		}
		break ;

		case MKITAB_DASHI :
			if (find_entry (ident_insert))
			{
				fprintf ( stderr , MSGSTR (E_IDEXIST ,
					"%s: ident entry found in %s\n" ),
					arg0 , INITTAB_FILE );
				exit (1);
			}
		case CHITAB :
			if (!found)
			{
				fprintf ( stderr , MSGSTR ( E_NOIDENT ,
					"%s: no match on ident field\n"),arg0 );
				exit (1);
			}
			field_ptr = verify_runlevel (field_ptr);
			field_ptr = verify_keyword (field_ptr);
			verify_command (field_ptr);
			update_inittab (ident,ident_insert);
			break ;

		case RMITAB :
			if (!found)
			{
				fprintf ( stderr , MSGSTR ( E_NOIDENT ,
					"%s: no match on ident field\n"),arg0 );
				exit (1);
			}
			update_inittab (ident,ident_insert);
			break ;

		case LSITAB :
			if (found)
				fprintf ( stdout , "%s" , entry ) ;
			else
				exit (1);
			break ;

		case LSITAB_DASHA :
			do_ls_all();
			break;
	}

	exit(0);
}

/*
 * NAME: do_ls_all
 *                                                                    
 * FUNCTION: walks through INITTAB_FILE printing records
 *                                                                    
 * EXECUTION ENVIRONMENT: user
 *                                                                   
 *                                                                   
 * (NOTES:) do_ls_all will open the INITTAB_FILE and list all entries
 *
 * (DATA STRUCTURES:) The variable entry is used as a holder variable for the
 *		current INITAB_FILE record.
 *
 * RETURNS: void
 *
 * SIZE EFFECTS: may exit if can't open INITTAB_FILE.
 */  
void
do_ls_all ()
{
	FILE	*fpinit;	/*  file ptr for INITTAB_FILE  */

	if ((fpinit = fopen(INITTAB_FILE, "r")) == NULL) {
	    fprintf(stderr,MSGSTR(E_OPEN,"%s: can't open %s\n"),arg0,
		INITTAB_FILE);
	    exit(1);
	}

	while ( fgets (entry , ENTRY_LEN, fpinit ))
		    if (entry[0] == ':') continue; 
		else 
		    printf("%s",entry);	/* don't need newline, fgets puts it */

	fclose (fpinit);
}



/*
 * NAME: find_entry
 *                                                                    
 * FUNCTION: checks for an entry in INITTAB_FILE
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *                                                                   
 * (NOTES:) find_entry will open the INITTAB_FILE and search for the
 *		ident field entered in the command line.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *	software error recovery.
 *
 * (DATA STRUCTURES:) The variable entry will contain the cooresponding
 *		line of the ident field from the INITTAB_FILE, when a
 *		TRUE value is returned.
 *
 * RETURNS: If the ident field is found a TRUE value is returned,
 *		otherwise a FALSE value is returned.
 */  
short
find_entry (char *ident)
{
	FILE	*fpinit;	/*  file ptr for INITTAB_FILE  */
	char	*e_ident ;	/*  ptr to ident  */
	short	count ;		/*  counts the length of ident field  */
	short	get_next ;	/*  get next input line  */

	if ((fpinit = fopen(INITTAB_FILE, "r")) == NULL)
		return (FALSE);

	e_ident = ident ;
	while ( fgets (entry , ENTRY_LEN, fpinit ))
		for ( get_next=0, e_ident = ident , count = 0 ; get_next == 0 ;)
		{
			if ( entry [count] == ':' )
			{
				if (*e_ident != '\0' && *e_ident != ':')
					break;
				fclose (fpinit);
				return (TRUE);
			}
			if ( entry [count] != *e_ident )
				get_next++;
			else {
				count++ ;
				e_ident++ ;
			}
		}

	fclose (fpinit);
	return (FALSE);
}


	/*	getname will return the name of the program
	 *	invoked by argv [0]. If the program name is
	 *	not an expected value, an error message will
	 *	be displayed and the program will exit.
	*/

short
getname(char *name)
{
	name = (char *)basename(name);
	if (strcmp(name,NAM_MKITAB) == 0)
		return (MKITAB) ;
	if (strcmp(name,NAM_CHITAB) == 0)
		return (CHITAB) ;
	if (strcmp(name,NAM_RMITAB) == 0)
		return (RMITAB) ;
	if (strcmp(name,NAM_LSITAB) == 0)
		return (LSITAB) ;

	fprintf ( stderr , MSGSTR (E_NAME , "Unknown command %s\n" ), name );
	exit (1) ;
}


	/*	parse_ident will determine the length of the ident field. If 
	 *      the field length is greater than IDENT_LEN the entry is invalid.
	*/
char *
parse_ident (char *argv1)
{
	short	count;

	for ( count=0 ; *argv1 != ':' && *argv1 != '\0' ; count++ , argv1++) ;

	if ( count > IDENT_LEN )
	{
		fprintf ( stderr , MSGSTR(E_IDENT, "%s: bad ident field\n" ),
			arg0 );
		usage ();
	}
	return(argv1);
}


	/*	display usage message when incorrect input is entered
	*/
usage ()
{
	switch ( cmd )
	{
		case MKITAB :
			fprintf ( stderr, MSGSTR ( M_MK_USE , MK_USE ) ,
				arg0 );
			break ;
		case CHITAB :
			fprintf ( stderr, MSGSTR ( M_CH_USE , CH_USE ) ,
				arg0 );
			break ;
		case RMITAB :
			fprintf ( stderr, MSGSTR ( M_RM_USE , RM_USE ) ,
				arg0);
			break ;
		case LSITAB :
		case LSITAB_DASHA :
			fprintf ( stderr, MSGSTR ( M_LS_USE , LS_USE ) ,
				arg0);
			break ;
	}
	exit (1) ;
}


	/*	verify_runlevel will check the length of the runlevel entry
	 *	and make sure all the entries are valid. Valid entries
	 *	are "MSabcms0123456789".
	 */
char *
verify_runlevel (char *runl)
{
	short	count ;

	if ( *runl != ':' )
	{
		fprintf ( stderr, MSGSTR ( E_RUNL , "%s: bad runlevel field\n")
			, arg0 ) ;
		usage ();
	}

	runl++ ;
	for ( count = 0 ; strchr (RUNL , *runl) ; count++ , runl++ )
		;
	if ( *runl != ':' || count > RUNLEVEL_LEN )
	{
		fprintf ( stderr, MSGSTR ( E_RUNL , "%s: bad runlevel field\n")
			, arg0 ) ;
		usage ();
	}

	return (runl);
}


	/*	verify_keyword will check the length of the keyword entry.
	 */
char *
verify_keyword (char *keyw)
{
	short	count ;

	keyw++ ;
	for ( count = 0 ; !(*keyw == ':' || *keyw == '\0') ; count++ , keyw++ )
		;
	if ( *keyw != ':' || count > KEYWORD_LEN )
	{
		fprintf ( stderr, MSGSTR ( E_KEYW , "%s: bad keyword field\n")
			, arg0 ) ;
		usage ();
	}

	return (keyw);
}


	/*	verify_command will check the length of the command entry.
	 */
void
verify_command (char *cmnd)
{
	cmnd++ ;
	if ( strlen (cmnd) > COMMAND_LEN )
	{
		fprintf ( stderr, MSGSTR ( E_CMD , "%s: bad command field\n")
			, arg0 ) ;
		usage ();
	}
}


	/*	update_inittab will create a new temporary file
	*	with the new changes and then replace the existing
	*	INITTAB_FILE file with the new temporary file.
	*/
void
update_inittab(char *ident,char *ident_insert)
{
	FILE	*fd;
	FILE	*fdtemp;
	char	curbuf[ENTRY_LEN];
	char	temp_init[TEMP_SIZE];
	short	ident_len;
	struct	acl	*acl;
	mode_t	omask;
	char	*temp_buf;

				/*  create temp file  */
	strcpy(temp_init, INITTAB_FILE);
	strcpy ( &temp_init [TEMP_SIZE - 7] , XXXXXX ) ;
	if ( mktemp (temp_init) == 0 )
	{
		fprintf ( stderr, MSGSTR (E_CREATE ,
			"%s: Unable to create temp file.\n") , arg0 );
		exit (errno);
	}

				/*  open temp file  */
	omask = umask((mode_t)077);
	if ((fdtemp = fopen(temp_init, "w")) == NULL)
	{
		fprintf ( stderr, MSGSTR (E_OPEN ,
			"%s: could not open %s.\n") , arg0 , temp_init);
		exit (errno);
	}
	(void) umask(omask);
				/* open inittab */
	if ((fd = fopen(INITTAB_FILE, "r")) == NULL)
	{
		fprintf ( stderr, MSGSTR (E_OPEN ,
			"%s: could not open %s\n" ) , arg0 , INITTAB_FILE);
		exit (errno);
	}
				/* copy the acl */
	acl = acl_fget(fileno(fd));
	(void) acl_fput(fileno(fdtemp), acl, 1);

				/*  determine length of ident field  */
	if ((temp_buf  = (char *) strchr(ident,':')) == NULL)
		ident_len = strlen (ident) ; 
	else
		ident_len = (unsigned long) (temp_buf) -
				(unsigned long) ident ;

				/*  read whole INITTAB_FILE  */
	while (fgets(curbuf, ENTRY_LEN, fd))
	{
		short	len;
		if (!strncmp(ident, curbuf, ident_len) &&
		    (':' == curbuf[ident_len]) ) {
				/*	if RMITAB then do not write line
				*	to temp file
				*/
			if ( cmd == RMITAB )
				continue ;
			if ( cmd == MKITAB_DASHI ) {
				len = strlen(curbuf);
				if (fwrite((void *)curbuf, (size_t)1, (size_t)len, fdtemp) != len )
				{
					fprintf ( stderr, MSGSTR ( E_UPDATE,
						"%s: Unable to update %s: errno = %d.\n"
						), arg0 , temp_init , errno );
					unlink(temp_init);
					fclose(fdtemp);
					fclose(fd);
					exit (1);
				}
				ident = ident_insert;
			}
			len = strlen(ident);
			ident [len++] = '\n' ;
			if (fwrite((void *)ident, (size_t)1, (size_t)len, fdtemp) != len )
			{
				fprintf ( stderr, MSGSTR ( E_UPDATE,
					"%s: Unable to update %s: errno = %d.\n"
					), arg0 , temp_init , errno );
				unlink(temp_init);
				fclose(fdtemp);
				fclose(fd);
				exit (1);
			}
		}
		else {
				/*	ident field does not match - write
				*	line to temp file
				*/
			len = strlen(curbuf);
			if (fwrite((void *)curbuf, (size_t)1, (size_t)len, fdtemp) != len )
			{
				fprintf ( stderr, MSGSTR ( E_UPDATE,
					"%s: Unable to update %s: errno = %d.\n"
					), arg0 , temp_init , errno );
				unlink(temp_init);
				fclose(fdtemp);
				fclose(fd);
				exit (1);
			}
		}
	}

	/*
 	 * make sure the new file was successfully written to ...
	 */

	if (fflush(fdtemp)) {
		fprintf ( stderr, MSGSTR ( E_UPDATE,
			"%s: Unable to update %s: errno = %d.\n"
			), arg0 , temp_init , errno );
		unlink(temp_init);
		fclose(fdtemp);
		fclose(fd);
		exit (1);
	}

	fsync(fdtemp->_file);
	fclose(fdtemp);
	fclose(fd);

	if (rename(temp_init, INITTAB_FILE)) {
		fprintf ( stderr, MSGSTR ( E_RENAME,
			"%s: Unable to rename temp file.\n"), arg0);
		unlink(temp_init);
		exit (1);
	}
}
