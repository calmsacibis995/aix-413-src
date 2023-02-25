#if !defined(lint)
static char sccsid[] = "@(#)84	1.10  src/tenplus/e2/common/fake.c, tenplus, tenplus411, GOLD410 3/3/94 18:48:35";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Esaveascii, makefake, openfake, readfake, backname,
 *		fbackup, saveascii, copyfile, savefakes, bagfakes,
 *		efopen, ffopen, esf_open, fsf_open
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* file:  fake.c                                                            */
/*                                                                          */
/* routines for handling fake (ASCII) files in the e2 editor                */
/*                                                                          */
/****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <dirent.h>

#include "def.h"
#include "INeditor_msg.h"
#include <sys/access.h>

#include <unistd.h>

static POINTER *g_fakenames; /* list of all the fake file names */
static long     g_lastloc;   /* current file location    */

#define TEXTLOC 0x80000000L /* used to make file locations as in the text file */

LOCAL SFILE *makefake (char *);

/****************************************************************************/
/* Esaveascii:  helper routine to save an ascii file                        */
/*                                                                          */
/* arguments:              char *filename - name of ascii file to save,     */
/*                                          or NULL to save all ascii files */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_filename                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the given filename is null, invoke savefakes to save all ascii    */
/*     files opened in the editor.  If the given filename is the same as    */
/*     the file being edited, do an Esync/Ereopen to ensure it is written   */
/*     to the disk and is still available for editing.  Otherwise, invoke   */
/*     saveascii to save the ascii file with the given name.                */
/****************************************************************************/

void Esaveascii (char *filename)
      /* ASCII file name */
    {
#ifdef DEBUG
    debug ("Esaveascii:  filename = '%s'", filename);
#endif

    if ((filename == NULL) || (*filename == '\0'))
        savefakes ();

    else if (seq (filename, g_filename)) /* watch out for current file */
        {
        s_free (Esync ());
        Ereopen ();
        }
    else
        saveascii (filename);
    }


/****************************************************************************/
/* makefake:  makes a fake structured file from an ascii text file          */
/* Returns the SFILE object for the delta file, or ERROR if file cannot be  */
/* created.                                                                 */
/*                                                                          */
/* arguments:              tmpname - name of fake file to make              */
/*                                                                          */
/* return value:           LOCAL SFILE * - pointer into fake file, or       */
/*                                         ERROR if could not be opened     */
/*                                                                          */
/* globals referenced:     g_textfp                                         */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     First invoke Remove to get rid of any previous version of the fake   */
/*     file.  Attempt to open a structured file with the given name and,    */
/*     if not successful, return ERROR.                                     */
/*                                                                          */
/*     Read the characters out of the g_textfp global text file pointer     */
/*     one by one.  Start a new structured file record at each newline      */
/*     character, and put into its fileloc subfield the file location of    */
/*     of the beginning of that line.  Perform re-allocations of the        */
/*     records array as necessary if the file is large.  When end-of-file   */
/*     is encountered, do one last re-allocation to get rid of excess       */
/*     garbage at the end, invoke obj_setflag to indicate the force the     */
/*     record array out to the file when the file is closed, and return.    */
/****************************************************************************/

LOCAL SFILE *makefake (char *tmpname)
    /* tmpname:                  tmp file name   */
    {
    register c;               /* input character */
    register unsigned i;      /* line index      */
    register FILE *fp;        /* local g_textfp  */
    register RECORD *records; /* record array    */
    register SFILE *sfp;      /* TP file sfp     */
    long fileloc;             /* file location   */

#ifdef DEBUG
    debug ("makefake:  tmpname = '%s'", tmpname);
#endif

    Remove (tmpname); /* this is to prevent a .bak file */
    sfp = sf_open (tmpname, "c");

    if (sfp == (SFILE *) ERROR)
        return ((SFILE *) ERROR);

    fp = g_textfp;

    records = (RECORD *) s_alloc (500, T_RECORD, (char *) NULL);
    records [0].r__fileloc = TEXTLOC;
    i = 1;
    fileloc = 0L;

    for (;;)
        {
        c = getc (fp);

        if (feof (fp))
            break;

        fileloc++;

        if (c == '\n')
            {
            if (i >= obj_count (records))
		{
		if (i >= OBJSIZE)
		    {
		    c = getc (fp); /* is more space needed only for the last */
		    if (feof (fp)) /* (NULL) line? */
			break;

		    tmclose ();    /* No, file is too long */
		    Remove (tmpname);
		    {
		    register char *message;

		    message = Egetmessage(M_E_LINES,"The file has too many lines.\nThe number of lines cannot exceed %d.",FALSE);
		    (void) printf (message, OBJSIZE);
		    (void) putchar ('\n');
		    s_free (message);
		    }
		    fatal ("makefake: number of lines exceeded %d.", OBJSIZE);
		    }

                records = (RECORD *) s_realloc ((char *) records,
                                                obj_count (records) + 500);
		}

            records [i].r__fileloc = fileloc | TEXTLOC;
#ifdef DEBUG
            debug ("  line %d at 0x%lx", i, records [i].r__fileloc);
#endif
            i++;
            }
        }
    if (records [i-1].r__fileloc  == (fileloc | TEXTLOC))
	i--; /* decrement i to strip last (NULL) line */
    records = (RECORD *) s_realloc ((char *) records, i);

    s_free ((char *) sf_records (sfp));
    sfp->sf__records = records;

    /***** force the record array out to the file when closed *****/

    obj_setflag (sfp, SF_MODIFIED);
    return (sfp);
    }

/****************************************************************************/
/* openfake:  open an ASCII file and make a fake file for it                */
/* Tries to put the delta (dots) file in same directory as the file.  If    */
/* that doesn't work, the dots file is put on /tmp.                         */
/*                                                                          */
/* arguments:              char *filename - name of text file               */
/*                                                                          */
/* return value:           SFILE *        - pointer into corresponding fake */
/*                                                                          */
/* globals referenced:     g_textfp, g_lastloc, g_readonly, g_fakenames     */
/*                                                                          */
/* globals changed:        g_textfp, g_lastloc, g_fakenames                 */
/*                                                                          */
/* synopsis:                                                                */
/*     Set the g_textfp global to point into the real text file, and set    */
/*     the g_lastloc global to -1L to force an initial seek in readfake.    */
/*     If the file is not readonly, add it to the list of fake files to     */
/*     save on exit (g_fakenames).  Invoke fakename to find the name of the */
/*     fake (dots) file corresponding to this text file.                    */
/*                                                                          */
/*     If the fake file already exists, and its modification date is later  */
/*     than the date on the text file, attempt to open a pointer to it      */
/*     and return.  If the initial attempt to open the file fails, run      */
/*     the fixascii filter and then try again to open it.                   */
/*                                                                          */
/*     If the second attempt to open the existing fake file fails, or if    */
/*     no fake file existed in the first place, invoke makefake to con-     */
/*     struct a new fake file.  If that fails, invoke uniqnam to generate a */
/*     different fake file name and try to open it.  If all these           */
/*     attempts fail, invoke fatal to bring down the editor.  Otherwise     */
/*     return the pointer to the structured fake file.                      */
/****************************************************************************/

SFILE *openfake (register char *filename)
        /* text file name                 */
    {
    register SFILE *sfp = (SFILE *)ERROR; /* delta sfile object      */
    register char  *tmpname;              /* tmp file name           */
    register char  *name;                 /* record array name       */
    auto     char   command [120 * MB_LEN_MAX];  /* fixascii command line     */
    auto     struct stat  buf1;         /* used to check file        */
    auto     struct stat  buf2;         /* used to check tmpfile     */
    auto     char         newname [PATH_MAX]; /* used to make inode string */

#ifdef DEBUG
    debug ("openfake:  filename = '%s'", filename);
#endif

    g_textfp = ffopen (filename, "r", "openfake");

    g_lastloc = -1L; /* force initial seek in readfake */

    if ( !g_readonly)
        {
        if (g_fakenames == NULL) /* make list if necessary */
	    g_fakenames = (POINTER *) s_alloc (0, T_POINTER, (char *) NULL);

        if ( !inlist (g_fakenames, filename))
            g_fakenames = s_append (g_fakenames, s_string (filename));
        }

    tmpname = fakename (filename); /* get name of tmp file */

    if (stat (filename, &buf1) == ERROR) /* if file doesn't exist */
        fatal ("openfake:  cannot stat file '%s'", filename);

    if ((stat (tmpname, &buf2) != ERROR) && /* if tmpfile is newer */
        (buf2.st_mtime > buf1.st_mtime))
        {
        sfp = sf_open (tmpname, "u");

        if (sfp == (SFILE *) ERROR)
            {
            (void) sprintf (command, "$SYS/bin/fixascii -ns %s %s", filename, tmpname);
            filter (command, "/dev/null", "/dev/null");
            sfp = sf_open (tmpname, "u");
            }

        if (sfp != (SFILE *) ERROR)
            {
            name = obj_name (sf_records (sfp));

	    (void) sprintf (newname, "%d.%ld", buf1.st_ino, buf1.st_mtime);
	    if (name && seq (name, newname))
                {
                s_free (tmpname);
                return (sfp);
                }

            /*$$$$ we may want a warning here if sf_eofloc > 0L *****/
            }
        if (sfp != (SFILE *) ERROR) /* catch the zero length record array case */
	    (void) sf_close (sfp);
        }

        sfp = makefake (tmpname); /* make a new tmpname file */

    if (sfp == (SFILE *) ERROR)
        {
        s_free (tmpname);
        tmpname = s_string (uniqname (TRUE));
        sfp = makefake (tmpname); /* make a new tmpname file */
        Remove (tmpname);  /* make tmpfile go when file is closed */

        if (sfp == (SFILE *) ERROR)
            fatal ("openfake:  cannot create temporary file '%s'", tmpname);
        }

    (void) sprintf (newname, "%d.%ld", buf1.st_ino, buf1.st_mtime);

    s_newname ((char *) sf_records (sfp), newname);
    s_free (tmpname);

    return (sfp);
    }

/****************************************************************************/
/* readfake:  version of sf_read for fake files                             */
/* Returns an s_alloc'ed char array containing the text for a given index   */
/*                                                                          */
/* arguments:              SFILE *sfp - pointer into fake file              */
/*                         FILE  *fp  - pointer into real ascii file        */
/*                         int index  - index of line to read               */
/*                                                                          */
/* return value:           char *     - one line from file                  */
/*                                                                          */
/* globals referenced:     g_errno, g_lastloc                               */
/*                                                                          */
/* globals changed:        g_errno, g_lastloc                               */
/*                                                                          */
/* synopsis:                                                                */
/*     readfake reads the specified line from the file, preferably from the */
/*     delta file and, if necessary, from the ascii file itself.  If the    */
/*     index is out of range, the g_errno global is set to E_MISC and the   */
/*     returned value is ERROR.  Otherwise readfakes examines the fileloc   */
/*     entry for the specified line to determine where to get the line.     */
/*                                                                          */
/*     If the fileloc entry is NORECORD, the line exists but contains only  */
/*     NULL, so readfake returns NULL.                                      */
/*                                                                          */
/*     If the TEXTLOC bit is set in the fileloc entry, the line is in the   */
/*     text file rather than the delta file.  If the fileloc entry differs  */
/*     from g_lastloc, set g_lastloc to fileloc and seek to the right part  */
/*     of the file.  Attempt to read the line from the file and, if         */
/*     successful, update g_lastloc to point just after the end of the line */
/*     and return an allocated copy of the line.  If the read fails, set    */
/*     g_errno to E_IO,  g_lastloc to -1, and return ERROR.                 */
/*                                                                          */
/*     Finally, if the requested line is in the delta file, simply use      */
/*     sf_seek and s_read to go to the right spot and read the line.        */
/*     This happens when any line is read that has been changed since the   */
/*     fake file was opened.                                                */
/****************************************************************************/

char *readfake (register SFILE *sfp, register FILE *fp, int index1)
         /* tmp file sfp             */
         /* ASCII file fp            */
  
    {
    register RECORD *records;   /* records array            */
    register char *text1;        /* text of line             */
    long fileloc;               /* file location of record  */

#ifdef DEBUG
    debug ("readfake:  index = %d", index1);
#endif

    records = sf_records (sfp);

    if ((index1 < 0) || (index1 >= obj_count (records)))
        {
        g_errno = E_MISC;
        return ((char *) ERROR);
        }
    fileloc = r_fileloc (&records [index1]); /* get file location of record */

#ifdef DEBUG
    debug ("readfake:  fileloc = 0x%lx, g_lastloc = 0x%lx", fileloc, g_lastloc);
#endif

    if (fileloc == NORECORD) /* if no record, return NULL */
        return ((char *) NULL);

    if (fileloc & TEXTLOC) /* if located in the text file */
        {
        fileloc &= ~TEXTLOC; /* clear high bit */

        if (fileloc != g_lastloc)
            {
            g_lastloc = fileloc;

# ifdef DEBUG
            debug ("readfake:  seeking to location %ld", fileloc);
# endif


# ifdef CAREFUL
            if (fseek (fp, fileloc, 0) == ERROR)
                fatal ("readfake:  seek error (fileloc = %D)", fileloc);
# else
            (void) fseek (fp, fileloc, 0);
# endif
            }

	text1 = s_getline (fp);

#ifdef DEBUG
	if (text1 != NULL)
	    debug ("readfake:  obj_count (text)  = %d", obj_count (text1));
#endif
	if (text1 == NULL)
            {
            g_errno = E_IO;
            g_lastloc = -1L;
            return ((char *) ERROR);
            }
#ifdef DEBUG
	debug ("readfake:  text1 (old) = '%s'", text1);
#endif
	g_lastloc += obj_count (text1) + 1;
	return (text1);
        }
    /***** the line is in the delta file *****/

    fileloc--; /* otherwise convert to file location */
    sf_seek (sfp, fileloc, 'r'); /* seek to read location */
    text1 = s_read (sf_fp (sfp)); /* read record */

#ifdef DEBUG
        debug ("readfake:  text (new) = '%s'", text1);
#endif

    return (text1);
    }

/****************************************************************************/
/* backname: add .bak to end of fname, trunc to name_max - 4 chars if need */
/****************************************************************************/

char *backname (char *filename)
    {
    register char *cp;
    static char bakname[PATH_MAX];    /*variable holding name of backup file */
    /* if backup name has format <path>/<basename>.bak, then
       max_bak_basename holds maximum number of bytes in <basename> */
    long max_bak_basename = pathconf(filename, _PC_NAME_MAX) - 4;

    (void) strcpy(bakname, filename);

    /* let cp point to the basename */
    cp = strrchr(bakname, DIR_SEPARATOR);
    if (cp != NULL)
         cp++; /* DIR_SEPARATOR has length 1 */
    else cp = bakname;

    /* advance cp to either the end of the basename (if basename
       smaller than max_bak_basename) or to the character after
       the 'max_bak_basename'th character.
    */
    if (MultibyteCodeSet) {
	int mlen;
        char *start_basename = cp;
        /* after one go through the loop, cp2 points at the char
           in front of *cp. */
        char *cp2 = cp;
        while ((*cp2 != '\0') &&
               ((cp2 - start_basename) <= max_bak_basename)) {
           cp = cp2;
           mlen = mblen(cp2, MB_CUR_MAX);
	   /* copy at least 1 char */
	   if (mlen < 1) mlen = 1;
           cp2 += mlen;
        }
        if (*cp2 == '\0')
           cp = cp2;
     }
     else cp = cp + min(strlen(cp), max_bak_basename);

     /* cut off rest of the backup name */
     *cp = '\0';

     /* add .bak to name */
     (void) strcat(bakname, ".bak");   

    return (bakname);
    }

/****************************************************************************/
/* fbackup:  moves a file to a backup name                                  */
/* The backup name is the file name concatenated with ".bak"                */
/* Returns ERROR if file cannot be backed up.                               */
/****************************************************************************/

int fbackup (char *filename)
    {
    char *backfile;
    register int retval;

    if ( !fexists (filename))
        return (RET_OK);
 
    backfile = backname (filename);
    retval = rename (filename, backfile);

    return (retval);
    }

/****************************************************************************/
/* saveascii:  converts a tmp file back to its ASCII file                   */
/*                                                                          */
/* arguments:              char *filename - name of ascii file to convert   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_lastloc                                        */
/*                                                                          */
/* globals changed:        g_lastloc                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     Invoke fakename to find the name of the fake (dots) file correspond- */
/*     ing to the given file.  Open the fake file or, if failing, return.   */
/*                                                                          */
/*     If the file has been modified, invoke Emessage to tell the user the  */
/*     file is being saved, back up the original file, and copy the current */
/*     version into it.  (If the KEEP_LINKS #ifdef is in effect, and there  */
/*     are multiple links to the file, ask the user whether to retain them  */
/*     and, if so, make a copy of the original and then modify it in place. */
/*                                                                          */
/*     Copying the current version into the file consists of looping        */
/*     through the records array of the delta file and copying each line    */
/*     into the new copy, using readfake to copy either from the delta file */
/*     for changed lines or from the original file for unchanged lines.     */
/*     In either case, update the records array as we go so that, at the    */
/*     end of the loop through lines, we can create a new fake (dots) file  */
/*     with correct fileloc information for use the next time this file is  */
/*     edited                                                               */
/****************************************************************************/

void saveascii (char *filename)
    /* filename:                 ASCII file name                        */
    {
    register char *cpyname;   /* name of temp copy of file w/links      */
    register i;               /* line index                             */
    register char *tmpname;   /* name of tmp file                       */
    register SFILE *sfp;      /* tmp file sfp                           */
    register RECORD *records; /* sfp record array                       */
    register FILE *oldfp;     /* old ASCII file fp                      */
    register FILE *newfp;     /* new ASCII file fp                      */
    register char *line;      /* text of line                           */
    long fileloc;             /* text file location                     */
    struct stat buffer;       /* stat call buffer                       */
    char newname [PATH_MAX];        /* used for making name                   */
    char	*sptr;	      /* Access Control List			*/

    tmpname = fakename (filename);

#ifdef DEBUG
    debug ("saveascii:  filename = '%s', tmpname = '%s'", filename, tmpname);
#endif

    sfp = sf_open (tmpname, "r");

    if (sfp == (SFILE *) ERROR)
        {
#ifdef DEBUG
        debug ("  cannot open tmp file '%s'", tmpname);
#endif
        s_free (tmpname);
        return;
        }

    g_lastloc = -1L; /* force initial seek in readfake */

    if (sf_eofloc (sfp) > EMPTYEOF) /* if file has been modified */
        {
	records = (RECORD *) s_alloc (obj_count (sf_records (sfp)), T_RECORD,
	  (char *) NULL);
        if (lstat (filename, &buffer) == ERROR)
             fatal ("saveascii:  cannot stat '%s'", filename);
	sptr = (char *)acl_get(filename);	/* save Access Control List */
	Emessage (M_ESAVE1, "Saving file %s.", filename);
        oldfp = ffopen (filename, "r", "saveascii");

        /* if only 1 link, do simple flavor backup      */
        /* don't forget about the possibility of a symbolic link !!! */

        if ( (buffer.st_nlink == 1) && ( ! S_ISLNK(buffer.st_mode) ) )
            {
	    (void) fbackup (filename);
	    newfp = ffopen (filename, "w", "saveascii");
	    cpyname = NULL;

            }
        else    /* multiple link case - preserve links  */
            {
            /* observation: we know we can write in this directory
               or we wouldn't be here.                          */

            /* make up name "foo.bak"                           */
            cpyname = backname (filename);
            newfp = ffopen (cpyname, "w+", "saveascii");

            /* copy old contents into backup file               */

	    while (TRUE)
		{
		if (copyfile (oldfp, newfp) == ERROR)
		    if (g_diskfcn != NULL)
			(*g_diskfcn)(cpyname, "saveascii-3");
		    else
			{
			g_diskfull = TRUE; /* don't try to save it again */
			Emessage (M_ESAVE, "Cannot write to file %s\n(disk probably full).", filename);
			(void) sleep (5);
			fatal ("write error on text file '%s' (disk probably full)",
			       cpyname);
			}
		else
		    break;
		}
            (void)fclose (oldfp);
            oldfp = newfp;

            /* reopen the original file so as to truncate it, then
               arrange newfp to point at it                     */

            i = open (filename, O_WRONLY | O_TRUNC, 0666);
            newfp = fdopen (i, "w");
            }

retry:

        fileloc = 0L;


        for (i = 0; i < obj_count (records); i++)
            {
            line = readfake (sfp, oldfp, i);
            records [i].r__fileloc = fileloc | TEXTLOC; /* save record location */

#ifdef DEBUG
	    if (line == (char *)ERROR)
		debug ("i=%d, fileloc=%ld, line=ERROR", i, fileloc);
	    else if (line == NULL)
		debug ("i=%d, fileloc=%ld, line=NULL", i, fileloc);
	    else
		debug ("  i = %d, fileloc = %ld, line = '%s(%d)'",
		  i, fileloc, line, obj_count (line));
#endif
#ifdef CAREFUL
            if (line == (char *) ERROR)
                fatal ("saveascii:  read error on index %d of file '%s'",
                       i, filename);
            else
#endif
	    if (line)
                {
		(void) fprintf (newfp, "%s", line);
		(void) fputc ('\n', newfp);
		fileloc += obj_count (line) + 1;
                }
            else
                {
		(void) fputc ('\n', newfp);
                fileloc++;
                }
            s_free (line);

	    (void) fflush (newfp);
	    if (ferror (newfp) || feof (newfp))
		{
		if (g_diskfcn != NULL)
		    {
		    (*g_diskfcn)(filename, "saveascii-1");
		    clearerr (newfp);
		    (void) fseek (newfp, 0L, 0);
		    goto retry;
		    }
		else
		    {
		    g_diskfull = TRUE; /* don't try to save it again */
		    Emessage (M_ESAVE, "Cannot write to file %s\n(disk probably full).", filename);
		    (void) sleep (5);
		    fatal ("write error on text file '%s' (disk probably full)",
			   filename);
		    }
		}
            }

        (void)fclose (oldfp);

	(void)fclose (newfp);

#ifdef CAREFUL
	/* set access control list in file */
	if (acl_put (filename, sptr) == ERROR)
            Eerror (M_EFAKEMODES, "Warning: Cannot fix file modes of %s.",
                    filename);
#else
	acl_put (filename, sptr);
#endif

        /* fix up backup copy of file with >1 link              */
        if (cpyname)
            {
	    acl_put (cpyname, sptr);
            chown (cpyname, buffer.st_uid, buffer.st_gid);
            }
	/* try to donate the file back to its original owner. This
	   fails for other than the superuser, but is harmless.         */

	chown (filename, buffer.st_uid, buffer.st_gid);
        /* make up phoney (0 long) record array to keep typechecking
           happy in sf_close. We don't want to throw away the original
           record array which is pointed to by "record"...              */

	/***** records is a new records array in the diskfull code, */
	/***** not a pointer to the current record array, so we     */
	/***** must free the current record array to prevent a leak */

	s_free ((char *)sf_records (sfp));

	sfp->sf__records = (RECORD *) s_alloc (0, T_RECORD, (char *) NULL);
        (void) sf_close (sfp);
        Remove (tmpname); /* this prevents a .bak file */
        sfp = fsf_open (tmpname, "c", "saveascii"); /* make new record array file */

        s_free ((char *) sf_records (sfp));
        sfp->sf__records = records;

        /***** put inode in record array name *****/

        if (stat (filename, &buffer) == ERROR)
            fatal ("saveascii:  cannot restat '%s'", filename);
	(void) sprintf (newname, "%d.%ld", buffer.st_ino, buffer.st_mtime);

        s_newname ((char *) records, newname);

        obj_setflag (sfp, SF_MODIFIED); /* force writing of record array */
        }
    /*$$$$ this may want to use a threshold of lines *****/

    if (obj_count (sf_records (sfp)) == 0) /* don't save file if empty */
        Remove (tmpname);

    (void) sf_close (sfp);
    s_free (tmpname);
    }


/****************************************************************************/
/* copyfile: Copy first file to second file using given file pointers       */
/****************************************************************************/

int copyfile (FILE *oldfp, FILE *newfp)

    {
    int c;

    while ((c = getc (oldfp)) != EOF) (void) putc (c, newfp);

    if ((fflush (newfp) == EOF) || (ferror (newfp)))
	return (ERROR);
    else
	return (RET_OK);
    }

/****************************************************************************/
/* savefakes:  converts all the fake files back to ASCII                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_fakenames                                      */
/*                                                                          */
/* globals changed:        g_fakenames                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     Invoke saveascii on each ascii file named in the g_fakenames global. */
/*     When finished, clear g_fakenames to NULL.                            */
/****************************************************************************/

void savefakes (void)
    {
    register int i;
    register int did_sync = FALSE;

#ifdef DEBUG
    debug ("savefakes called");
#endif

    if (g_fakenames == NULL)
        return;

    for (i = 0; i < obj_count (g_fakenames); i++)
        if (seq (g_fakenames [i], g_filename) && (g_sfp != NULL))
                {
		/* Esync will properly close current file and save it. */
                s_free (Esync ());
		did_sync = TRUE;
                }
        else
            saveascii (g_fakenames [i]);

    s_free ((char *) g_fakenames);
    g_fakenames = NULL;

    /* Reopen current file. If current file is ascii this will cause it
       to be put back in the g_fakenames list. */
    if (did_sync)
	Ereopen ();
    }

/****************************************************************************/
/* bagfakes: "bag" fake files - don't save them or dots files.              */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_fakenames                                      */
/*                                                                          */
/* globals changed:        g_fakenames                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     For each of the ascii files named in the g_fakenames global, invoke  */
/*     fakename to determine the name of the corresponding dots file and    */
/*     then remove the dots file.  This effectively undoes any changes made */
/*     to the ascii files.  When done, set g_fakenames to NULL.             */
/****************************************************************************/

void bagfakes (void)
    {
    register i;
    register char *name;

#ifdef DEBUG
    debug ("bagfakes called");
#endif

    if (g_fakenames == NULL)
        return;

    for (i = 0; i < obj_count (g_fakenames); i++)
        {
        name = fakename (g_fakenames [i]);
        Remove (name);
        s_free (name);
        }
    /* not strictly necessary, since bagfakes is only called
       from e2exit, but here in case we ever get any space again
    s_free ((char *) g_fakenames);
    g_fakenames = NULL;                 */
    }

/****************************************************************************/
/* efopen:  fopen with call to Eerror if return is NULL                     */
/****************************************************************************/

FILE *efopen (char *filename, char *mode)

{
    FILE *rfp;

    if ((rfp = fopen (filename, mode)) == NULL)
        Eerror (M_ESAVEFILE, "Cannot open output file %s.", filename);
    return (rfp);
}

/****************************************************************************/
/* ffopen:  fopen with call to fatal if return is NULL                      */
/****************************************************************************/

FILE *ffopen (char *filename, char *mode, char *function)

{
    FILE *rfp;

    for (;;)
	{
	if ((rfp = fopen (filename, mode)) != NULL)
	    return (rfp);

	if (errno != ENOSPC)
	    fatal ("%s: Cannot open file %s", function, filename);

	if (g_diskfcn != NULL)
	    (*g_diskfcn)(filename, "ffopen");
	}
}

/****************************************************************************/
/* esf_open:  sf_open with call to Eerror if return is ERROR                */
/****************************************************************************/

SFILE *esf_open (char *filename, char *mode)

{
    SFILE *rfp;

    if ((rfp = sf_open (filename, mode)) == (SFILE *) ERROR)
        Eerror (M_ESAVEFILE, "Cannot open output file %s.", filename);
    return (rfp);
}

/****************************************************************************/
/* fsf_open:  sf_open with call to fatal if return is ERROR                 */
/****************************************************************************/

SFILE *fsf_open (char *filename, char *mode, char *function)

{
    SFILE *rfp;

    if ((rfp = sf_open (filename, mode)) == (SFILE *) ERROR)
        fatal ("%s: Cannot open file %s", function, filename);
    return (rfp);
}

