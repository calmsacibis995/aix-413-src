#if !defined(lint)
static char sccsid[] = "@(#)73	1.7  src/tenplus/e2/common/salloc.c, tenplus, tenplus411, GOLD410 3/23/93 11:53:35";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_nzalloc, s_read, s_realloc, s_string, s_write, scopy
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* File: salloc.c - structured alloc and free routines                      */
/****************************************************************************/

#include    "def.h"

/****************************************************************************/
/* s_nzalloc: version of s_alloc that doesn't zero data array               */
/*                                                                          */
/* arguments:              int  count - number of items to allocate         */
/*                         int  type  - object datatype code                */
/*                         char *name - object name, or NULL                */
/*                                                                          */
/* return value:           char * - the structured allocation               */
/*                                                                          */
/* globals referenced:     g_dtypes, g_formcache                            */
/*                                                                          */
/* globals changed:        g_formcache                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     s_nzalloc first checks the validity of the type argument,            */
/*     and then invokes malloc to pull the right number of bytes            */
/*     from the heap.  It refers to the g_dtypes global to find             */
/*     the number of bytes required for an object of the given              */
/*     type, and adds some bytes for header and trailer data.               */
/*                                                                          */
/*     If the malloc fails, it tries throwing away the g_formcache global   */
/*     (which is both big and expendable) and trying the malloc again.      */
/*     If the malloc still fails, s_nzalloc invokes allocerr.               */
/*                                                                          */
/*     With the space for the object in hand, set its type, flag,           */
/*     name, and count subfields, and set a null after the last             */
/*     byte of the data field.  Do not bother to remove whatever            */
/*     garbage may be in the data field itself.  Return a pointer           */
/*     to the start of the data subfield ( !the start of the object.)     */
/****************************************************************************/

char *s_nzalloc (int count, int type, char *name)
            /* number of items to allocate             */
            /* object datatype                         */
            /* object name                             */
    {
    register SHEADER *shp; /* structured allocation                   */
    register unsigned dlength;  /* number of data bytes in this allocation */
    register char *cp;     /* pointer to data area of allocation      */

#ifdef DEBUG

    debug ("s_nzalloc:  called w/ count = %d, type = %d, ",
	     count, type);
    debug ("name = '%s'\n", (name == NULL) ? "NULL" : name);

#endif


#ifdef CAREFUL

    if ((type <= 0) || (type >= MAX_TYPES) ||
      (dt_name (g_dtypes + type) == NULL)) /* if type invalid */
	fatal ("s_nzalloc:  illegal type (%d)", type);

    if (count < 0)
	fatal ("s_nzalloc:  negative count (%d)", count);
#endif

    dlength = dt_size (&g_dtypes [type]) * count; /* get data length */

    /* add size of header to request plus an extra byte     */
    /* the extra byte at the end of the data is set to NULL */

    shp = (SHEADER *) malloc ((unsigned) (dlength + HEADERSIZE + 1));

    if (shp == NULL) /* watch for allocation failure */
	{
	s_free ((char *) g_formcache);
	g_formcache = NULL;
	shp = (SHEADER *) malloc ((unsigned) (dlength + HEADERSIZE + 1));

	if (shp == NULL) /* watch for allocation failure */
	    allocerr ();

	g_formcache = (POINTER *) s_alloc (0, T_POINTER, "g_formcache");
	}
    shp->sh__type = type;
    shp->sh__flag = 0;
    shp->sh__name = (name == NULL) ? NULL : scopy (name);
    shp->sh__count = count;

    cp = sh_data (shp);
    cp [dlength] = '\0'; /* NULL out the last byte */
#ifdef	DEBUG
    debug ("shp = 0x%X, cp = 0x%X", shp, cp);
#endif

    return (sh_data (shp));
    }

/****************************************************************************/
/* s_read:  reads a structured allocation from a file                       */
/*                                                                          */
/* arguments:              FILE *fp - file opened for reading               */
/*                                                                          */
/* return value:           char * - the next allocation from the file       */
/*                                                                          */
/* globals referenced:     g_errno, g_dtypes                                */
/*                                                                          */
/* globals changed:        g_errno                                          */
/*                                                                          */
/* synopsis:                                                                */
/*     The disk format for a structured allocation is:                      */
/*                                                                          */
/*       1 byte   type                                                      */
/*       1 byte   flag                                                      */
/*       1 byte   length of name (n)                                        */
/*       n bytes  name                                                      */
/*       2 bytes  count (number of items)                                   */
/*                                                                          */
/*       this is followed by the data for each of the count items.          */
/*       The format of these items is determined by the read and write      */
/*       routines for the datatype involved.  This may involve recursive    */
/*       calls to s_read in the case of POINTER sallocs                     */
/*                                                                          */
/*       If the flag byte is zero, the name is NULL, and the count is       */
/*       less than 255, an abbreviated version of this format is used with  */
/*       only the type byte (with the high bit on) and a one byte count.    */
/*                                                                          */
/*     A NULL pointer is represented by a NULL byte in the file.            */
/*                                                                          */
/*     Returns ERROR on I/O error, else a pointer to the object read in.    */
/*     Returns ERROR w/ g_errno = E_TYPECHECK if object type is illegal.    */
/****************************************************************************/

char *s_read (register FILE *fp)
            /* file pointer to read from     */
    {
    register type;             /* datatype being read in        */
    register flag;             /* flag bits for object          */
    register length;           /* length of name                */
    register count;            /* size of data array for object */
    register char *cp;         /* data pointer for object       */
    register int (*reader) (char *, FILE *); /* read function for data type   */
    char namebuf [NAMESIZE];   /* input buffer for names        */


#ifdef DEBUG

    debug ("s_read:  called w/ fp = 0%o\n", fp);

#endif

    type = fgetc (fp);               /* one byte of salloc type */

    if (type == 0)                /* handle NULL pointers    */
	return (NULL);

    if (type & 0x80) /* if abreviated header */
	{
	type &= 0x7f;               /* strip high bit */
	flag = 0;                   /* no flag        */
	length = 0;                 /* no name        */
	count = fgetc (fp) & 0xff;  /* one byte count */
	}
    else
	{
	flag = fgetc (fp);   /* one byte of flag bits   */
	length = fgetc (fp); /* one byte length of name */

	if (length >= NAMESIZE) /* if name too long */
	    fatal ("s_read:  length (%d) too large", length);

	if (length > 0) /* if there is a name */
	    {
	    (void)fread (namebuf, 1, (unsigned) length, fp);  /* read in name */
	    namebuf [length] = '\0';         /* null terminate name     */
	    }
	count = geti (fp); /* read in count */
	}
    if (count < 0)
	{
	g_errno = E_MISC;
	return ((char *) ERROR);
	}
    /* check type against valid types */

    if ((type <= 0) || (type >= MAX_TYPES) ||
      (dt_name (g_dtypes + type) == NULL))
	{
	g_errno = E_TYPECHECK;
#ifdef DEBUG
	debug ("s_read:  type = %d, EOF = %s", type, feof (fp) ? "YES" : "NO");
#endif
	return ((char *) ERROR);
	}
    /* allocate object and use its name if it has one */

    cp = s_nzalloc (count, type, (length > 0) ? namebuf : NULL);

    if (cp == (char *) ERROR)    /* check for allocation failure */
	return ((char *) ERROR); /* Note:  g_errno already set   */

    obj_setflag (cp, flag);                 /* set flag bits in object      */
    reader = dt_reader (&g_dtypes [type]);  /* get read function            */

    if (reader == NULL)
	reader = (int(*)(char *, FILE *))e_read;

    /* Note:  the datatype specific readers set g_errno */

    if ((*reader) (cp, fp) == ERROR)        /* read in each object in array */
	return ((char *) ERROR);            /* check for I/O error          */

    return (cp);
    }

/****************************************************************************/
/* s_realloc: reallocate a structured allocation to be n items long         */
/*                                                                          */
/* arguments:              char *cp     - pointer to old allocation         */
/*                         int newcount - desired new count                 */
/*                                                                          */
/* return value:           char * - new version of allocation               */
/*                                  with same contents but correct size,    */
/*                                  or ERROR if could not allocate          */
/*                                                                          */
/* globals referenced:     g_dtypes, g_formcache                            */
/*                                                                          */
/* globals changed:        g_formcache                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     s_realloc changes the size of a structured allocation to n items,    */
/*     where n may be bigger or smaller than the original n.   The returned */
/*     pointer may or may not be the same as the supplied pointer, or may   */
/*     be ERROR if there is no space available.  If the buffer has to be    */
/*     moved, the old one is marked as freed (if the header for the old     */
/*     buffer is not inside the new buffer).  The g_formcache global can    */
/*     be set to point to null if we are nearly out of space.  If the       */
/*     new buffer is larger than the old, the extra bytes in the data       */
/*     field are set to NULL before returning.                              */
/****************************************************************************/

char *s_realloc (register char *cp, unsigned newcount)
         /* pointer to old data area */
        /* new count of data items  */
    {
    register SHEADER *oldshp; /* old salloc pointer       */
    register SHEADER *newshp; /* new salloc pointer       */
    register i;               /* used to transfer data    */
    register char *newdata;   /* pointer to new data area */
    register unsigned newlength;  /* new length of data area  */
    register unsigned oldlength;  /* old length of data area  */
    register oldcount;        /* old count of data items  */
    register type;            /* salloc data type         */

#ifdef DEBUG

    debug ("s_realloc:  called w/ cp = 0%o, newcount = %d\n",
	     cp, newcount);

#endif

#ifdef CAREFUL
    (void) s_typecheck (cp, "s_realloc", T_ANY); /* make sure datatype is valid */

    if (newcount < 0)
	fatal ("s_realloc:  negative count (%d)", newcount);
#endif

    oldshp = obj_shp (cp);

    newlength = dt_size (&g_dtypes [sh_type (oldshp)]) * newcount;

    oldcount = sh_count (oldshp);    /* so we can clear the new real estate */
    oldlength = dt_size (&g_dtypes [sh_type (oldshp)]) * oldcount;

    type = sh_type (oldshp);      /* save type                  */
    oldshp->sh__type = T_FREED;   /* mark old buffer as invalid */

    /* add size of header to request plus an extra byte     */
    /* the extra byte at the end of the data is set to NULL */
    /* Note that realloc will copy the name, type and data  */

    newshp = (SHEADER *) realloc ((char *) oldshp,
	(unsigned) (newlength + HEADERSIZE + 1));

    if (newshp == NULL) /* watch for allocation failure */
	{
	s_free ((char *) g_formcache);
	g_formcache = NULL;
	newshp = (SHEADER *) realloc ((char *) oldshp,
	    (unsigned) (newlength + HEADERSIZE + 1));

	if (newshp == NULL) /* watch for allocation failure */
	    allocerr ();

	g_formcache = (POINTER *) s_alloc (0, T_POINTER, "g_formcache");
	}
    newshp->sh__type = type;      /* restore type in object  */
    newshp->sh__count = newcount; /* put new count in object */
    newdata = sh_data (newshp);
    newdata [newlength] = '\0'; /* zero the extra byte after the data area */

    /* zero out new data are if object has grown */

    for (i = oldlength; i < newlength; i++)
	newdata [i] = '\0';

    return (sh_data (newshp));
    }

/****************************************************************************/
/* s_string:  converts string into a salloc'ed char array                   */
/*                                                                          */
/* arguments:              char *string - to be converted                   */
/*                                                                          */
/* return value:           char * - allocated version of string,            */
/*                                  or ERROR if allocation failure          */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     s_string uses s_nzalloc to create a structured allocation large      */
/*     enough to accept the string contents, and then uses strcpy to        */
/*     copy the string characters into the data field of the allocation.    */
/*     If the s_nzalloc fails, s_string passes its error value on up.       */
/****************************************************************************/

char *s_string (char *string)
     /* string to make into a char array */
    {
    register char *cp;

    if (string == NULL)
	return (s_nzalloc (0, T_CHAR, (char *) NULL));

    cp = s_nzalloc (strlen (string), T_CHAR, (char *) NULL);

    if (cp == (char *) ERROR)
	return ((char *) ERROR); /* Note:  g_errno already set */

    (void) strcpy (cp, string);
    return (cp);
    }

/****************************************************************************/
/* s_write:  writes a structured allocation to a file                       */
/*                                                                          */
/* arguments:              char *cp - allocation to write into file         */
/*                         FILE *fp - pointer into file opened for write    */
/*                                                                          */
/* return value:           int - RET_OK or ERROR                            */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     The disk format for a structured allocation is:                      */
/*                                                                          */
/*       1 byte   type                                                      */
/*       1 byte   flag                                                      */
/*       1 byte   length of name (n)                                        */
/*       n bytes  name                                                      */
/*       2 bytes  count (number of items)                                   */
/*                                                                          */
/*       this is followed by the data for each of the count items.          */
/*       The format of these items is determined by the read and write      */
/*       routines for the datatype involved.  This may involve recursive    */
/*       calls to s_write in the case of POINTER sallocs                    */
/*                                                                          */
/*       If the flag byte is zero, the name is NULL, and the count is       */
/*       less than 255, an abreviated version of this format is used with   */
/*       only the type byte (with the high bit on) and a one byte count.    */
/*                                                                          */
/*     A NULL pointer is represented by a NULL byte in the file.            */
/*                                                                          */
/*     Does fatal error exit if passed an invalid pointer.                  */
/*     Returns ERROR on I/O error, else RET_OK.                             */
/****************************************************************************/

int s_write (char *cp, register FILE *fp)
                     /* pointer to object data area     */
           /* file pointer to write object on */
    {
    register length;           /* length of name                  */
    register type;             /* object datatype                 */
    register count;            /* object item count               */
    register int (*writer) (char *, FILE *); /* write function for the datatype */


#ifdef DEBUG

    debug ("s_write:  called w/ cp = 0%o, fp = 0%o\n", cp, fp);

#endif

    if (cp == NULL) /* handle NULL pointers */
	{
	(void) fputc ('\0', fp);
	return (RET_OK);
	}
    (void) s_typecheck (cp, "s_write", T_ANY); /* make sure it is a valid pointer */

    /* if object has a name, get length */

    length = (obj_name (cp) != NULL) ? strlen (obj_name (cp)) : 0;
    type = obj_type (cp);
    count = obj_count (cp);

    if ((obj_flag (cp) == 0) && (length == 0) && (count <= 255))
	{
	(void) fputc (type | 0x80, fp); /* use abreviated form of header */
	(void) fputc (count, fp);
	}
    else
	{
	(void) fputc (type, fp);          /* one byte of salloc type */
	(void) fputc (obj_flag (cp), fp); /* one byte of flag bits   */
	(void) fputc (length, fp);        /* one byte length of name */

	if (length > 0) /* if there is a name, write it out */
	    (void) fwrite (obj_name (cp), 1, (unsigned) length, fp);

	puti (count, fp);         /* 2 bytes of count        */
	}
    writer = dt_writer (&g_dtypes [type]); /* get write function */

    if (writer == NULL)
	writer = (int(*)(char *, FILE *))e_write;

    /* Note:  the datatype specific writer set g_errno */

    if ((*writer) (cp, fp) == ERROR) /* write out data */
	return (ERROR);

    return (RET_OK);
    }

/****************************************************************************/
/* scopy:  returns an allocated copy of a string                            */
/*                                                                          */
/* arguments:              char *oldstring - string to be copied            */
/*                                                                          */
/* return value:           char * - copy of oldstring,                      */
/*                                  or NULL if oldstring was null,          */
/*                                  or ERROR if allocation failure          */
/*                                                                          */
/* globals referenced:     g_formcache                                      */
/*                                                                          */
/* globals changed:        g_formcache                                      */
/*                                                                          */
/* synopsis:                                                                */
/*     scopy invokes malloc to get enough bytes from the heap to store      */
/*     the data in oldstring.  If the first malloc fails, scopy clears      */
/*     the g_formcache global to make more room and tries again.  If the    */
/*     malloc still fails, scopy invokes allocerr.  The characters from     */
/*     the old string are copied into the new, and the new string is        */
/*     returned.  Note that scopy does !return a structured allocation.  */
/****************************************************************************/

char *scopy (char *oldstring)
     /* string to copy */
    {
    register char *newstring;
    register length;

    if (oldstring == NULL)
	return (NULL);

    length = strlen (oldstring) + 1;
    newstring = malloc ((unsigned) length);

    if (newstring == NULL) /* watch for allocation failure */
	{
	s_free ((char *) g_formcache);
	g_formcache = NULL;
	newstring = malloc ((unsigned) length);

	if (newstring == NULL) /* watch for allocation failure */
	    allocerr ();

	g_formcache = (POINTER *) s_alloc (0, T_POINTER, "g_formcache");
	}
    (void) strcpy (newstring, oldstring);
    return (newstring);
    }


