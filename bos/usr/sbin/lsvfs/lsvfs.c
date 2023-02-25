static char sccsid[] = "@(#)88	1.8.2.1  src/bos/usr/sbin/lsvfs/lsvfs.c, cmdfs, bos411, 9428A410j 1/10/94 17:59:36";
/*
 * COMPONENT_NAME: (CMDFS) commands that manipulate files
 *
 * FUNCTIONS: crvfs, chvfs, lsvfs, rmvfs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
** single source /etc/{cr,ls,ch,rm}vfs
*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/limits.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define EQ(a,b) (!strcmp(a,b))			/* compare two strings */
#define NELEM(a) (sizeof(a)/sizeof(a[0]))	/* # elements in an array */
#define VFSFILE "/etc/vfs"	/* file to operate on */

#define VFS_DEFAULT "%defaultvfs"

/*
** format of /etc/vfs file:
**  	name	vfs_number mount_helper filsys_helper
**
*/
struct vfs_f {
    char *name; 		/* vfs name */
    char *number; 		/* vfs number */
    char *mount;		/* mount helper */
    char *filsys;		/* filesystem helper */
};
typedef struct vfs_f *vfs_t;


#include <sys/types.h>
#include <locale.h>

#include <nl_types.h>
#include "lsvfs_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_LSVFS,Num,Str)

/* forward declare functions */
char *storestring();
char *vfs_nextrec();
vfs_t find_vfs(), merge_vfs(), str_to_vfs(), vfs_malloc();
FILE *open_vfsfile();
void append_vfs();
void print_vfs();
char *vfs_name(), *mytok();

/* declare action routines so they can be used in the structure below */
extern int do_cr(), do_ls(), do_ch(), do_rm();

/*
 * function dispatch structure
 */
struct progs {
    char *progname;		/* program name */
    int (*func)();		/* function to call */
    int args;			/* number of args */
    int msgnum;			/* usage message number */
    char *usage;		/* text of usage message */
    int errexit;		/* what code to exit on bad return from func */
} prog[] = { 
#define DEFPROG 0		/* default program: lsfs */
    { "lsvfs", do_ls, 1, LS_USAGE, "Usage: lsvfs <-a|VfsName>.\n", 1 }, 
    { "crvfs", do_cr, 1, CR_USAGE, "Usage: crvfs VfsEntry.\n", 1 },
    { "chvfs", do_ch, 1, CH_USAGE, "Usage: chvfs VfsEntry.\n", 1 },
    { "rmvfs", do_rm, 1, RM_USAGE, "Usage: rmvfs VfsName.\n", 1 } 
};

char *vfsfile = VFSFILE;
char *progname;		/* the program's name: used throughout this program */
int prognum;		/* program's index into the prog array */

main(argc,argv) 
int argc;
char **argv;
{
    int i;
    int rv;
   
    if (!(vfsfile = getenv("VFSFILE"))) vfsfile = VFSFILE;
    (void) setlocale (LC_ALL,"");
    catd = catopen(MF_LSVFS, NL_CAT_LOCALE);

    /* get program name from argv[0] by stripping path */
    progname = rindex(argv[0],'/'); 
    if (progname) progname++; else progname= argv[0];

    /* identify our name */
    for ( i = 0 ; i < NELEM(prog); i++)
	if (EQ(progname,prog[i].progname)) break;

    /* did we find our name? */
    if (i == NELEM(prog)) {
	char buf[255];
	fprintf(stderr,
	    MSGSTR(OOPS,"%s: I should be called "),progname);
	for (i = 0; i < NELEM(prog)-1; i++) {
	    strcat(buf,prog[i].progname);
	    strcat(buf,", ");
	}
	strcat(buf,MSGSTR(OR,"or ")); strcat(buf,prog[i].progname);
	printf(MSGSTR(DEFAULT,". Defaulting to %s.\n"),prog[DEFPROG].progname);
	i = DEFPROG;
    }

    prognum = i;

    /* check args before calling function */
    if (argc > prog[i].args+1 || argc < 2) {
	    if (EQ("lsvfs", progname) && argc == 1)
	    	;
	    else
	    	usage();
    }

    /* call the desired function */
    /* change this line if prog[n].args can be greater that 2. */
    if (EQ("lsvfs", progname) && argc == 1)
	rv = (prog[i].func)(NULL);
    else
    	rv = (prog[i].func)(argv[1]);

    exit(rv==0 ? 0 : prog[i].errexit);
}


/***************************************************************************\
**                         START OF CORE FUNCTIONS                         **
\***************************************************************************/

/*
** FUNCTION: do_ch(ch)
** PURPOSE: change the vfs entry pointed to by ch.
** SIDE EFFECTS: the vfs file is modified to reflect the new vfs entry
** RETURN VALUE: none.
*/
do_ch(fs)
char *fs;
{
    vfs_t old_a, new_a;
    char *cp;

    cp = storestring(vfs_name(fs)); 
    old_a = find_vfs(cp,1);

    new_a = str_to_vfs(fs,":",0);

    /* merge vfs's */
    new_a = merge_vfs(old_a,new_a);
    
    /* 
    ** remove vfs from /etc/vfsfile file and put new one in at the
    ** end of the file.
    */
    do_rm(vfs_name(fs));
    append_vfs(new_a);
}

/*
** FUNCTION: do_cr(fs)
** PURPOSE: create a new vfs in the VFSFILE file with the data
**          in 'fs'
** SIDE EFFECTS: the vfs  file is grown by an appropriate ammount
**          with the data from the new record.
** RETURN VALUE: 0.
*/
do_cr(fs)
char *fs;
{
    vfs_t ar;
    char *cp;

    cp = vfs_name(fs);
    if (find_vfs(cp,0) != 0) {
	fprintf(stderr,MSGSTR(STANZAEXISTS,"%s: vfs '%s' already exists.\n"),
	    progname,cp);
	exit(1);
    }

    ar = (vfs_t) str_to_vfs(fs,":",0);

    if (ar->name && ar->number && ar->mount && ar->filsys &&
	*ar->name && *ar->number && *ar->mount && *ar->filsys) {
	append_vfs(ar);
	return(0);
    }
    fprintf(stderr,MSGSTR(BADVFS,"%s: bad vfs entry '%s'.\n"),
	progname, fs);
    exit(1);
}

/*
** FUNCTION: do_ls(a)
** PURPOSE: locate and print a vfs from vfs file
** SIDE EFFECTS: none
** RETURN VALUE: 0.
**
** NOTES:  if parmeter 'a' contains the string '-a', then all of the vfss
**     in vfsfile are listed.
*/
do_ls(a)
char *a;
{
    vfs_t ar;
    char *s;

    if (a == NULL || strcmp(a,"-a")==0) {
	FILE *af;		/* attribute file typedef */
	vfs_t ar;		/* attribute record typedef */
	int aflag;

	if (a == NULL) aflag = 0;
	else aflag = 1;
	af = open_vfsfile();

	while (s = vfs_nextrec(af)) {
	    if (!strncmp(s,VFS_DEFAULT,strlen(VFS_DEFAULT))) {
	    	if (aflag)
			printf("%s\n",s);	/* print out '%default' */
	    }
	    else
	    	printf("%s\n", s);
	}
	fclose(af);
    } else {
	/* lookup entry */
	ar = find_vfs(a,1);
	print_vfs(ar,stdout);
    }
    return (0);
}

/*
** FUNCTION: do_rm(fs)
** PURPOSE: remove the vfs indicated by fs.
** SIDE EFFECTS: none
** RETURN VALUE: 0.
*/
do_rm(fs)
char *fs;
{
	char buf[255], *s;
	FILE *fp1, *fp2;


	(void) find_vfs(fs,1);	/* find_vfs exits if not found */
	
	/*
	 * open a tmp file
	 */
	if ((fp2 = tmpfile()) == NULL) {
		perror("tmpfile()");
		exit(1);
	}

	/* 
	 * open and lock the stanza file
	 */
	if ((fp1 = fopen(vfsfile, "r+")) == NULL) {
		perror(vfsfile);
		exit(1);
	}

	lock_file(fp1);

	/*
	 * read thru the /etc/vfs file, copying everything but the vfs
	 * we're looking for.
	 */

    	while ((s = fgets(buf,(int)sizeof(buf),fp1)) != NULL) {
		if (vfs_match(fs,s)) continue;
		fputs(buf,fp2);
	}

	rewind(fp1);
	rewind(fp2);
	copy_file(fp2, fp1, vfsfile);
	fclose(fp1);
	fclose(fp2);
}

/***************************************************************************\
**                       START OF UTILITY FUNCTIONS                        **
\***************************************************************************/

/*
 *	copy_file
 *
 *	- copies from 'from' to 'to'
 */
copy_file(from, to, tofile)
FILE *from, *to;
char *tofile;
{
	char buf[BUFSIZ];
	int f, t, count;
	off_t len;

	f = fileno(from);
	t = fileno(to);
	while ((count = read(f, buf, BUFSIZ)) > 0) {
		if (write(t, buf, count) < 0) {
			fprintf(stderr, MSGSTR(UPDATFAIL, "%s: update of %s failed\n"), progname, tofile);
			return;
		}
	}
	len = lseek(t, 0, 1);
	ftruncate(t, len);
}

/*
 *	lock_file
 *
 *	- lock file 'fd'
 */
lock_file(fd)
FILE *fd;
{
	struct flock arg;

	arg.l_type = F_WRLCK;
	arg.l_whence = SEEK_SET;
	arg.l_start = 0;
	arg.l_len = 0;
	(void) fcntl(fileno(fd), F_SETLKW, &arg);
}


/* 
** FUNCTION: vfs_match()
** PURPOSE: use regex to determine if 'fs' is in 's'
** SIDE EFFECTS: none
** RETURN VALUE: 1 on match, 0 on fail
*/
vfs_match(fs,s)
char *fs;
char *s;
{
    char buf[1024];
    char *vec;

    sprintf(buf,"^%s[ \t]",fs);		/* form regular expression */
    vec = (char *)regcmp(buf,0);
    if (vec == 0) {
        fprintf(stderr,"%s: Erorr compiling regular expression.\n",progname);
        exit(1);
    }
    if (regex(vec,s) != 0) return 1;
    return(0);
}

/*
** FUNCTION: find_vfs(fs,exitonerr) 
** PURPOSE:  look in vfsfile for a vfs matching parameter fs.
** SIDE EFFECTS: exit's 1 on error if exitonerr is non zero.
** RETURN VALUE: a pointer (allocated somewhere in the AF routines) 
**               to the requested vfs.
*/

vfs_t find_vfs(fs,exitonerr) 
char *fs;
{
    FILE *af;		/* attribute file typedef */
    vfs_t ar;		/* attribute record typedef */
    char *s;

    /* open attribute file and lookup entry */
    af = (FILE *) open_vfsfile();
    ar = NULL;

    while ((s = vfs_nextrec(af)) != NULL) {
	if (vfs_match(fs,s)) { 
	    ar = str_to_vfs(s," \t",1);
	    break;
	}
    }

    if (ar == NULL) {
	if (exitonerr) {
	    fprintf(stderr,MSGSTR(NOMATCH,
		"%s: No record matching '%s' was found in %s.\n"),
		progname, fs, vfsfile);
	    exit(1);
	} else ar = 0;
    }
    (void) fclose(af);
    return(ar);
}

/*
** FUNCTION: print_vfs(ar,outfile)
** PURPOSE: print out the vfs represented by ar.
** SIDE EFFECTS: none
** RETURN VALUE: none.
*/
void print_vfs(ar,outfile)
vfs_t ar;
FILE *outfile;
{
    fprintf(outfile,"%s\t%s\t%s\t%s\n",
	ar->name, ar->number, ar->mount, ar->filsys);
}


/*
** FUNCTION: str_to_vfs(s,delim,multi)
** PURPOSE: convert the string pointer to by 's' to a vfs suitable
**          for entry in the VFSFILE file. (multi is passed to mytok below)
** SIDE EFFECTS:  an vfs_t is malloc'ed.  function exit's 1 on error 
** RETURN VALUE:  the newly created vfs
**
** NOTES: The format of the input string should be as follows:
** name		number		mount		filsys
**
*/
vfs_t str_to_vfs(s,delim,multi)
char *s;
char *delim;
int multi;
{
    vfs_t ar;
    char *line;


    ar = (vfs_t) vfs_malloc();
    line = storestring(s);

    ar->name = mytok(line,delim,multi);
    if (ar->name == NULL) {
	fprintf(stderr,
	   MSGSTR(BADNAME,"%s: Invalid name field in vfs line.\n"), progname);
	exit(1);
    }
    ar->number = mytok(NULL,delim,multi);
    if (ar->number == NULL) {
	fprintf(stderr,
	   MSGSTR(BADNUMB,"%s: Invalid number field in vfs line.\n"), progname);
	exit(1);
    }
    ar->mount = mytok(NULL,delim,multi);
    if (ar->mount == NULL) {
	fprintf(stderr,
	   MSGSTR(BADMOUNT,"%s: Invalid mount field in vfs line.\n"), progname);
	exit(1);
    }
    ar->filsys = mytok(NULL,delim,multi);
    if (ar->filsys == NULL) {
	fprintf(stderr,
	   MSGSTR(BADFILSYS,"%s: Invalid file system field in vfs line.\n"), 
	       progname);
	exit(1);
    }
    return(ar); 
}

/*
** FUNCTION: vfs_name(s)
** PURPOSE: return vfs name from beginning of a string describing
**          a vfs.
** SIDE EFFECTS: returned value is a static value.  save returned string
**          in local storage if you want the value across calls to 
**          vfs_name
** RETURN VALUE: the name of the vfs (char *)
*/
char *vfs_name(s)
char *s;
{
    static char *q, cp[255];
    
    strcpy(cp,s);
    q = strtok(cp,": \t");

    return(q);
}

/*
** FUNCTION: merge_vfs(old,new)
** PURPOSE: merge two vfs's together.  if an attribute in vfs 'old' is 
**          duplicated in vfs 'new', the output vfs will have the value in
**          vfs 'new'.
** SIDE EFFECTS: a new vfs is malloc'ed
** RETURN VALUE: the merged vfs
*/
vfs_t merge_vfs(old,new)
vfs_t old,new;
{
    vfs_t rv;

    rv = (vfs_t) vfs_malloc();
    rv->name = old->name;
    if (new->number && *(new->number) != 0)  {
	rv->number = new->number; 
    } else {
	rv->number = old->number;
    }
    if (new->mount && *(new->mount) != 0)  {
	rv->mount = new->mount; 
    } else {
	rv->mount = old->mount;
    }
    if (new->filsys && *(new->filsys) != 0)  {
	rv->filsys = new->filsys; 
    } else {
	rv->filsys = old->filsys;
    }
    return(rv);
}

/*
** FUNCTION: usage()
** PURPOSE: print a usage message and exit(1).
** SIDE EFFECTS: none, 'cept exiting.
** RETURN VALUE: none.
**
** NOTES:  this function knows what usage message to print by the global
**         'prognum'.
*/
usage()
{
    fprintf(stderr,MSGSTR(prog[prognum].msgnum,prog[prognum].usage));
    exit(1);
}

/*
** FUNCTION: open_vfsfile()
** PURPOSE: open the vfsfile file.
** SIDE EFFECTS: uses the global vfsfile to determine what file to use.
**         exits on error.
** RETURN VALUE: the FILE * of the newly opened vfsfile
**
*/
FILE *open_vfsfile()
{
    FILE *af;

    af = (FILE *) fopen(vfsfile,"r");
    if (af == NULL) {
	fprintf(stderr,MSGSTR(CANTOPEN,"%s: can't open '%s': "),
	    progname,vfsfile);
	perror("");
	exit(1);
    }
    return(af);
}

/*
** FUNCTION: vfs_malloc()
** PURPOSE: allocate memory for a vfs structure.
**    guarantees that memory is zeroed (calls bzero)
** SIDE EFFECTS: exist if malloc(3) returns null.
** RETURN VALUE: pointer to the newly malloced memory in the form of an vfs_t.
*/
vfs_t vfs_malloc()
{
    vfs_t p;

    p = (vfs_t) malloc((size_t)sizeof(struct vfs_f));

    if (p == NULL) {
	fprintf(stderr,MSGSTR(MEMERR,"%s: memory allocation error.\n"),
	    progname);
	exit(1);
    }
    bzero(p,sizeof(struct vfs_f));
    return(p);
}

/*
** FUNCTION: storestring(s)
** PURPOSE: allocate memory for, and save off a string 's'
** SIDE EFFECTS: exist if malloc(3) returns null.
** RETURN VALUE: pointer to the newly malloced memory as a char *.
*/
char *storestring(s)
char *s;
{
    char *p;
    p = (char *) malloc(strlen(s)+1);

    if (p == NULL) {
	fprintf(stderr,MSGSTR(MEMERR,"%s: memory allocation error.\n"),
	    progname);
	exit(1);
    }
    return((char *)strcpy(p,s));
}

/*
** FUNCTION: append_vfs(ar)
** PURPOSE: append a vfs to the vfsfile file.
** SIDE EFFECTS: exits program on error
** RETURN VALUE: none.
*/
void append_vfs(ar)
vfs_t *ar;
{
    FILE *f;

    f = fopen(vfsfile,"a");

    if (f == NULL) {
	fprintf(stderr,"%s: %s: open:",progname,vfsfile);
	perror("");
	exit(1);
    }
    print_vfs(ar,f);

    (void) fclose(f);
}

/*
** FUNCTION: char *vfs_nextrec(f) 
** PURPOSE:  get next record from vfs file.  skipping comments in file.
** RETURN:  pointer to string or null on EOF
*/
char *
vfs_nextrec(f)
FILE *f;
{
    char *s;
    static char buf[255];
    
    s = fgets(buf,(int)sizeof(buf),f);
    if (s != NULL) { 
	while (isspace((int)*s)) s++;	/* clobber whitespace */
	s[strlen(s)-1] = 0;	/* clobber newline */
    }
    while (s != NULL && *s == '#' && *s != 0) {
	s = fgets(buf,(int)sizeof(buf),f);
	if (s != NULL) { 
	    while (isspace((int)*s)) s++;	/* clobber whitespace */
	    s[strlen(s)-1] = 0;	/* clobber newline */
	}
    }
    return(s);
}

/*
** FUNCTION: mytok(str,delim,multi)
** PURPOSE:  return next token seperated by a character in delim.
** 	     if multi != 0, more than one delimter character may be
**	     present between tokens.  on second and subsequent calls,
**	     the 'str' parameter should be passed as NULL.  passing a non-
**	     null string parameter causes the function to be reset (and
**           start at the beginning of the new string (ala strtok(3s))
** RETURN VALUE: a string pointing to the token, an empty string (if multi
**           is not set and there are multiple delimiters in a row i.e.:
**           an empty field) or null on end of string (str).
** SIDE EFFECTS: on second and subsequent calls, the string 's' should
**	     be set to null.
*/
char *mytok(str,delim,multi)
char *str, *delim;
int multi;
{
    static char *s = NULL;	/* our work string */
    char *p;		/* points to head of token */
    char *mystrchr();

    if (str != 0) s = str;	/* take up where we left off if s == 0 */

    /* 
    ** check for empty string 
    */
    if (!s || *s == 0) {
	return(0);
    }

    p = s;		/* start at token */
    while (*s != 0 && mystrchr(delim,*s) == NULL ) {/* while s is not a delim */
	s++;					/* cruise through string */
    }
    if (*s != 0)
	*s++ = 0;				/* clobber delimiter */
    /*
    ** do we want to clobber all delimiters ? 
    */
    if (multi) {
        while (mystrchr(delim,*s) != NULL) *s++ = 0; /* clobber delimiters */
    }

    if (*s == 0) s = 0;
    /* 
    ** return what we found
    */
    return(p);
}

char *
mystrchr(str, c)
char *str;
char c;
{
	while(*str) {
		if (*str == c)
			return((char *)str);
		str++;
	}
	return(NULL);
}
