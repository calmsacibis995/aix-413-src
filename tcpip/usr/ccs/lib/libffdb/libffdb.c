static char sccsid[] = "@(#)39        1.11  src/tcpip/usr/ccs/lib/libffdb/libffdb.c, tcpip, tcpip411, GOLD410 6/5/94 08:10:04";
/*
 * COMPONENT_NAME:  TCPIP
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * libffdb:	ff_open(), ff_close(), ff_add(), ff_delete(), ff_show(),
 *              ff_exist_key(), ff_rename_by_copying(), ff_key_match(),
 */                                                                   

/*
 * The functions provided here are access routines for flat file databases
 * of the sort used for TCP/IP configuration.  
 * 
 */

#include <stdio.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/lockf.h>
#include <sys/stat.h>
#include "libffdb.h"

#include "libffdb_msg.h"
#include <locale.h>
#include <nl_types.h>
#include <limits.h>
extern *malloc();
char *
getamsg(cat, set, msg, def)  
	char *cat;
	long set;
	long msg;
	char *def;
{
	nl_catd  catd;    
	char	 *message;
	
	catd = catopen(cat,NL_CAT_LOCALE);
	message = catgets(catd, set, msg, def);
	catclose(catd);
	return(message);
}
#define MSGSTR(n,s) getamsg(MF_LIBFFDB, MS_LIBFFDB,n,s)

extern char progname[];

char retbuf[MAXRECLEN];	/* ff_delete() returns deleted record in this buff */
char *strstr();

void ff_rename_by_copying();

ushort st_mode;		/* File Access Permission mode returned from stat */
int local = 0;		/* Check whether file is on NFS mounted system */

/*
 * ff_open()
 * 
 * open a flat file database.  locks the file if it is opened for modification.
 *
 * RETURNS:
 * file pointer, if successful.
 * exits with errno from open if open was unsuccessful.
 *
 */

FILE *
ff_open(fname, op_mode)
    char *fname;		/* name of file to open */
    char *op_mode;		/* mode (for fopen) to open with */
{
    int	fd;
    FILE *fp;
    struct stat sb;
    
    if ((fp = fopen(fname, op_mode)) == NULL) {
	fprintf(stderr,
		MSGSTR(ERROPEN, "%s: error opening %s.\n"),
		progname, fname);
	exit(errno);
    }
    
    st_mode = 0;		/* set to 0 as default in case file is not locked */
    
    /*
     *
     * if we open with any mode that is not just 'read' then we lock the
     * file.  Prior to locking the file however we have to save the current
     * file access permission level and then modify the file access to
     * enforce file locking.  The original file access level will be reset in
     * the file close routine.
     *
     */
    if (strcmp(op_mode, "r") != 0) {
	if (fullstat(fname, STX_NORMAL, &sb) == -1) {
	    fprintf(stderr,
		    MSGSTR(ERRSTAT, "%s: error getting status of file: %s\n"),
		    progname, fname);
	    perror("stat");
	    exit(errno);
	}
	
	if (!(sb.st_flag & FS_REMOTE)) {
		st_mode = sb.st_mode;	/* save current file access level */
		sb.st_mode = sb.st_mode | S_ENFMT;
		local = 1;  /* file is on local machine */
	
		if (chmod(fname, sb.st_mode) == -1) {
	    		fprintf(stderr,
		    	MSGSTR(ERRACC, 
				"%s: error changing access mode of file: %s\n"),
		    		progname, fname);
	    		perror("chmod");
	    		exit(errno);
		}	
	}
	if (lockf(fileno(fp), F_TLOCK, 0) == -1) {
	    fprintf(stderr,
		    MSGSTR(ERRLOCK, "%s: error locking file: %s\n"),
		    progname, fname);
	    perror("lockf");
	    exit(errno);
	}
    }
    return(fp);
}



/*
 * ff_close()
 *
 * close a flat file database.
 *
 * RETURNS:
 * the return code from the close of the flat file database.
 * exits with an error if the file cannot be returned to its original
 * state.
 *
 */

int
ff_close(fname, fp)
    char *fname;
    FILE *fp;			/* file stream pointer */
{
    if (st_mode)		/* must reset if st_mode != 0 */ {
	if (lockf(fileno(fp), F_ULOCK, 0) == -1) {
	    fprintf(stderr,
		    MSGSTR(ERRUNLOCK, "%s: error unlocking file: %s\n"),
		    progname, fname);
	    perror("lockf");
	    exit(errno);
	}
	if (local) {           /* open file is local */
		if (chmod(fname, st_mode) == -1) {
	    		fprintf(stderr,
		    	MSGSTR(ERRACC, 
				"%s: error changing access mode of file: %s\n"),
		    		progname, fname);
	    		perror("chmod");
	    		exit(errno);
		}
	}
    }
    return(fclose(fp));
}


/*
 * ff_add()
 *
 * add a record to a flat file database.  record is assumed to be in the
 * proper format. 
 *
 * RETURNS:
 * 0 if unsuccessful.
 * 1 if successful.
 *
 */

int	
ff_add(fname, rp)
    char *fname;		/* database file name */
    char *rp;			/* points to record */
{
    int	n;			/* number of chars output */
    FILE *fp;
    
    /* we might want to check that the last char in the file was a newline. */
    fp = ff_open(fname, "a");
    if (fp) {
	n = fprintf(fp, "%s\n", rp);
	ff_close(fname, fp);
	return(1);
    } else
	return(0);
}


/* 
 * ff_delete()
 *
 * delete a record from a flat file database by matching key field.
 * if kp1 and kp2 are non-null, then all records with primary key of kp1 and
 * secondary key of kp2 are deleted.  if kp2 is null, then all records with
 * primary key of kp1 are deleted.  if kp1 and kp2 are null then all records
 * are deleted.  if more than one record is deleted, then the last to be 
 * deleted is returned in retbuf.  if all records are to be deleted (kp1 and
 * kp2 are null), then retbuf is returned empty.
 *
 * RETURNS:  
 * if successful, a pointer to a statically allocated buffer containing
 *    deleted record.
 * NULL, otherwise.
 *
 */

char *
ff_delete(fname, fn1, kp1, fn2, kp2)
    char *fname;		/* pointer to flat file database file name */
    int	fn1;			/* primary key field number (0 = any) */
    char *kp1;			/* primary key field pointer (0 = any) */
    int	fn2;			/* secondary key field number (0 = any) */
    char *kp2;			/* secondary key field pointer (0 = any) */
{
    char tmpfile[128];
    char buf[MAXRECLEN];
    FILE *fp, *tfp;
    struct stat sb;
    
    if (stat(fname, &sb) != 0) {
	fprintf(stderr, "%s does not exist.\n", fname);
	exit(errno);
    }
    
    fp = ff_open(fname, "r+");	/* "r+" locks the file */
    strcpy(tmpfile, tmpnam(NULL));
    
    if ((tfp = fopen(tmpfile, "w")) == 0) {
	fprintf(stderr,
		MSGSTR(ERRTMPOPEN, "%s: can't open temp file.\n"),
		progname);
	perror("fopen");
	exit(errno);
    }
    
    if (kp2) {			/* match both keys */
	retbuf[0] = 0;
	while (fgets(buf, MAXRECLEN, fp) > 0) {
	    if (ff_key_match(kp1, fn1, buf) && 
		ff_key_match(kp2, fn2, buf))
		strcpy(retbuf, buf);
	    else
		fputs(buf, tfp);
	}
    } else {			/* match only primary key */
	if (kp1) {		/* delete all records matching primary key. */
	    retbuf[0] = 0;
	    while (fgets(buf, MAXRECLEN, fp) > 0) {
		if (ff_key_match(kp1, fn1, buf))
		    strcpy(retbuf, buf);
		else
		    fputs(buf, tfp);
	    }
	} else {		/* delete all records, leave comments. */
	    retbuf[0] = 0;
	    while (fgets(buf, MAXRECLEN, fp) > 0) {
		if (buf[0] == '#' || buf[0] == '\n')
		    fputs(buf, tfp);
		else
		    strcpy(retbuf, buf); /* save last deleted record */
	    }
	} 
    }
    
    ff_close(fname, fp);
    fclose(tfp);
    
    if (rename(tmpfile, fname) == -1) {
	if (errno == EXDEV)
	    ff_rename_by_copying(tmpfile, fname);
	else {
	    fprintf(stderr,
		    MSGSTR(ERRMVTMP, "%s: can't move temp file back to %s.\n"),
		    progname, fname);
	    perror("rename");
	    exit(errno);
	}
    }
    if (chmod(fname, sb.st_mode) == -1) {
	fprintf(stderr,
		MSGSTR(ERRACC, "%s: error changing access mode of file: %s\n"),
		progname, fname);
	perror("chmod");
	exit(errno);
    }
    
    return((retbuf[0]) ? retbuf : NULL);
}


/*
 * ff_show()
 *
 * shows records by key, or all records.
 *
 * RETURNS:
 * 1 if something to show was found.
 * 1 if nothing found.
 * exits with error if the file to search wasn't found.
 *
 */

int
ff_show(fname, fn, kp, colonizing_hook)
    char *fname;		/* pointer to file name */
    int	 fn;			/* field number (0 = any) */
    char *kp;			/* key field pointer (0 = match any) */
    char *colonizing_hook();	/* ptr to function that puts
				   record in (colon) format */
{
    FILE *fp;
    char buf[MAXRECLEN];
    register char *cp;
    struct stat sb;
    int	found = 1;
    
    if (stat(fname, &sb) != 0) {
	fprintf(stderr,
		MSGSTR(ERRENOENT, "%s does not exist.\n"),
		fname);
	exit(errno);
    }
    
    fp = ff_open(fname, "r");

    while ((cp = fgets(buf, MAXRECLEN, fp)) != NULL) {
        cp = buf;
        SKIPDELIM(cp);
        if (*cp != '#' && *cp != '\0') {
           if ( (kp && ff_key_match(kp, fn, buf)) || !kp) {
		 printf(((colonizing_hook)?(*colonizing_hook)(buf):buf));
                 found = 1;
            }
        }
    }

    ff_close(fname, fp);
    return(found);
}


/* 
 * ff_exist_key()
 *
 * determines if a record exists, by key.
 *
 * RETURNS:
 * 1, if record exists
 * 0, if record doesn't exist.
 *
 */

int	
ff_exist_key(fname, kp, fn)
    char *fname;		/* pointer to file name */
    char *kp;			/* key field pointer (0 = any) */
    int	fn;			/* field number (0 = any) */
{
    FILE *fp;
    char buf[MAXRECLEN];
    struct stat sb;
    int	found;
    
    if (stat(fname, &sb) != 0)
    return(FALSE);
    
    found = FALSE;
    fp = ff_open(fname, "r");
    while (fgets(buf, MAXRECLEN, fp) != NULL) {
	if (ff_key_match(kp, fn, buf)) {
	    found = TRUE;
	    break;
	}
    }
    ff_close(fname, fp);
    return(found);
}


/*
 * ff_rename_by_copying()
 *
 * rename a file by copying it from one file name to another.
 *
 * RETURNS:
 * nothing (void)
 * exits with error if file i/o fails.
 *
 */

void
ff_rename_by_copying(from, to)
    char *from;
    char *to;
{
    int	fdfrom, fdto;
    register int n;
    char buf[MAXRECLEN];
    struct stat sb, sb2;

    if ((fdfrom = open(from, O_RDONLY)) == -1) {
	fprintf(stderr,
		MSGSTR(ERROPEN, "%s: error opening %s.\n"),
		progname, from);
	perror("open");
	exit(errno);
    }
    
    if (fstat(fdfrom, &sb) == -1) {
	fprintf(stderr,
		MSGSTR(ERRSTAT, "%s: error getting status of file: %s\n"),
		progname, from);
	perror("fstat");
	exit(errno);
    }

    /* "from" must be a regular file that we are copying from */
    if (!S_ISREG(sb.st_mode)) {
	fprintf(stderr,
		MSGSTR(ERRSTAT, "%s: error getting status of file: %s\n"),
		progname, from);
	exit(-1);
    }

    /* "to" can exist (?), but if it does, it must be a regular file */
    if (! stat(to, &sb2)) {
	if (!S_ISREG(sb2.st_mode)) {
	    fprintf(stderr,
		    MSGSTR(ERRSTAT, "%s: error getting status of file: %s\n"),
		    progname, to);
	    exit(-1);
	}
    }

    if ((fdto = creat(to, O_WRONLY)) == -1) {
	perror("creat");
	exit(errno);
    }
    
    while ((n = read(fdfrom, buf, MAXRECLEN)) > 0) {
	if (write(fdto, buf, n) != n) {
	    perror("write");
	    exit(errno);
	}
    }
    if (n == -1) {
	perror("read");
	exit(errno);
    }
    
    if (chmod(to, sb.st_mode) == -1) {
	fprintf(stderr,
		MSGSTR(ERRACC, "%s: error changing access mode of file: %s\n"),
		progname, to);
	perror("chmod");
	exit(errno);
    }
    close(fdfrom);
    close(fdto);
    
    if (unlink(from) == -1) {
	perror("unlink");
	exit(errno);
    }
}


/*
 * FF_TOKEN_DELIMITED(buff, tok, toklen)    (used by ff_key_match())
 *
 * determines if the substring ('tok', of length 'toklen') we are 
 * pointing to in buffer 'buf' is a delimited token.
 *
 * RETURNS:
 * TRUE if the substring is preceded by and ends with either:
 *      a delimeter character, or a buffer boundary.
 * FALSE otherwise.
 *
 */

#define  FF_TOKEN_DELIMITED(buff, tok, toklen) \
  (((tok == buff)               ||     /* token is at start of buffer, or */\
    (strchr(DELIM, *(tok-1)))   ||     /* prec char is a delimiter, or */\
    (*(tok-1) == '\0'))         &&     /* prec char is null, AND */\
   ((*(tok + toklen) == '\0')   ||     /* succ char is null, or */\
    (strchr(DELIM, *(tok + toklen))))) /* succ char is a delimiter */


/* todo:  check for comments '#' in record 
 * ff_key_match()
 *
 * matches a key against a specified field in a record buffer.  the key
 * must match the entire delimited field.
 *
 * RETURNS:
 * 1 if there is a match
 * 0 if there is no match
 *
 */

int
ff_key_match(kp, fn, buf)
    char *kp;			/* points to key string */
    int	fn;			/* key field number (0 = any) */
    char *buf;			/* buffer containing record to match against */
{
    register char *cp = buf;
    register int ntok, len, found;

    SKIPDELIM(cp);		/* move past any initial delimeters */

    /* move past 'fn' number of tokens */
    for (ntok = 1; ntok < fn && *cp; ntok++) {
	SKIPTOKEN(cp);		/* move past any subsequent token */
	SKIPDELIM(cp);		/* move past any initial delimeters */
    }
    len = strlen(kp);
    found = FALSE;
    if (*cp) {
	if (fn == 0) {		/* search for key in any field */
	    while ((cp = strstr(cp, kp)) != NULL &&
		   !FF_TOKEN_DELIMITED(buf, cp, len)) {
		cp++;
	    }
	    if (cp)
		found = TRUE;
	} else			/* key should be in field 'fn' */
	    if (strncmp(kp, cp, len) == 0 && 
		FF_TOKEN_DELIMITED(buf, cp, len))
		found = TRUE;
    }
    return(found);
}




