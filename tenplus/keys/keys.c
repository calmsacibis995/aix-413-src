#if !defined(lint)
static char sccsid[] = "@(#)10	1.5  src/tenplus/keys/keys.c, tenplus, tenplus411, GOLD410 7/19/91 14:46:33";
#endif /* lint */


/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	main, key_free, get_keys, to_pointer, to_char,
 *		has_value, sort, construct_ufiles, construct_sfiles,
 *		show_ifdefs, show_endifs
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <chardefs.h>
#include <database.h>
#include <keys.h>
#include <libobj.h>


POINTER *edkeys;
char *filename;

char *place_holder = "place holder";

int MultibyteCodeSet;

/* main: build files from editor keys database */
main (int argc, char *argv[])
{
    SFILE *sfp;

    (void) setlocale(LC_ALL, "");
    
    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    /* check for proper invocation */
    if (argc < 2) {
	(void) fprintf (stderr, "Usage: %s file\n", argv [0]);
	exit (1);
    }

    /* save away file name for comment header in generated files */
    filename = argv [1];

    /* open the structured file */
    if ((sfp = sf_open (filename, "r")) == (SFILE *)ERROR) {
	(void) fprintf (stderr, "%s: cannot open %s (g_errno = %d)\n", argv [0], filename, g_errno);
	exit (1);
    }

    /* read the contents of the file */
    get_keys (sfp);
    (void) sf_close (sfp);

    /* build keynames.h and tables.c files */
    construct_ufiles ();

    /* sort the list so that the tdigest tables are properly generated */
    sort ();

    /* build the names.h file used by tdigest */
    construct_sfiles ();

    /* free up miscellaneous allocations */
    s_free ((char *)edkeys);

    /* and exit indicating success */
    exit (0);
    return(0);
    /*NOTREACHED*/
}


int T_KEY;

/* key_free: T_KEY object type data freer */
/*LINTLIBRARY*/
void key_free (struct key *obj)
{
    s_free ((char *)obj->keyname);
    s_free ((char *)obj->editorname);
    s_free ((char *)obj->ifdefs);
}

/* get_keys: read keys database and build an internal list from it */
void get_keys (SFILE *sfp)
{
    int index1 = 0;
    POINTER *record;
    int fnindex = 1;

    /* add a keys data type according to the key structure */
    T_KEY = s_addtype ("keys", sizeof (struct key), 
                       (void(*) (void *, int))noop, 
                       (int(*) (void *, FILE *))noop, 
                       (int(*) (void *, FILE *))noop, key_free);

    /* edkeys is a global containing the extracted data base */
    edkeys = (POINTER *)s_alloc (0, T_POINTER, NULL);

    /* loop over records in the database and process each */
    while ((record = (POINTER *)sf_read (sfp, index1++)) != (POINTER *)ERROR) {
	register int i, len;
	register struct key *key = (struct key *)s_alloc (1, T_KEY, NULL);

	/* ignore empty records */
	if (record == NULL)
	    continue;
	
	key->fnindex = fnindex++;

	len = obj_count (record);
	/* loop over leaves in the data tree processing each */
	for (i=0; i<len; i++) {
	    register char *name;
	    if (record [i] == NULL)
		continue;
	    name = obj_name (record [i]);
	    if (seq (name, "keyname")) {
		key->keyname = to_pointer (record [i]);
		record [i] = NULL;
	    } else if (seq (name, "editorname")) {
		key->editorname = to_pointer (record [i]);
		record [i] = NULL;
	    } else if (seq (name, "ifdefs")) {
		key->ifdefs = to_pointer (record [i]);
		record [i] = NULL;
	    } else if (seq (name, "functionname")) {
		key->functionname = to_char (record [i]);
		record [i] = NULL;
	    } else if (seq (name, "noargs")) {
		if (has_value (record [i]))
		    key->noargs = TRUE;
	    } else if (seq (name, "emptyarg")) {
		if (has_value (record [i]))
		    key->emptyarg = TRUE;
	    } else if (seq (name, "stringarg")) {
		if (has_value (record [i]))
		    key->stringarg = TRUE;
	    } else if (seq (name, "numberarg")) {
		if (has_value (record [i]))
		    key->numberarg = TRUE;
	    } else if (seq (name, "regionarg")) {
		if (has_value (record [i]))
		    key->regionarg = TRUE;
	    } else if (seq (name, "lineregion")) {
		if (has_value (record [i]))
		    key->lineregion = TRUE;
	    } else if (seq (name, "boxregion")) {
		if (has_value (record [i]))
		    key->boxregion = TRUE;
	    } else if (seq (name, "textregion")) {
		if (has_value (record [i]))
		    key->textregion = TRUE;
	    } else if (seq (name, "modifier")) {
		if (has_value (record [i]))
		    key->modifier = TRUE;
	    } else if (seq (name, "cursordef")) {
		if (has_value (record [i]))
		    key->cursordef = TRUE;
	    } else if (seq (name, "delstack")) {
		if (has_value (record [i]))
		    key->delstack = TRUE;
	    } else if (seq (name, "execflag")) {
		if (has_value (record [i]))
		    key->execflag = TRUE;
	    } else if (seq (name, "canflag")) {
		if (has_value (record [i]))
		    key->canflag = TRUE;
	    } else if (seq (name, "invtxtflg")) {
		if (has_value (record [i]))
		    key->invtxtflg = TRUE;
	    } else if (seq (name, "regionhandle")) {
		if (has_value (record [i]))
		    key->regionhandle = TRUE;
	    }
	}
	/* add the processed record to the edkeys list */
	edkeys = s_append ((POINTER *)edkeys, (char *)key);
	s_free ((char *)record);
    }
}

/* to_pointer: make argument into pointer array if necessary */
POINTER *to_pointer (char *ptr)
{
    POINTER *pptr;

    if (obj_type (ptr) == T_POINTER)
	return ((POINTER *)ptr);

    pptr = (POINTER *)s_alloc (1, T_POINTER, NULL);
    pptr [0] = ptr;
    return (pptr);
}

/* to_char: strip pointer array if necessary */
char *to_char (char *ptr)
{
    if (obj_type (ptr) == T_CHAR)
	return (ptr);

    if (obj_type (ptr) == T_POINTER) {
	POINTER *pptr = (POINTER *)ptr;
	char *ret = pptr [0];
	pptr [0] = NULL;
	s_free ((char *)pptr);
	return (ret);
    }
    return (NULL);
}

/* has_value: return TRUE iff non blank character visible on form */
int has_value (char *ptr)
{
    char *to_char (char *);

    if (obj_type (ptr) == T_POINTER)
	ptr = ((POINTER *)ptr)[0];
    if (ptr && (obj_type (ptr) == T_CHAR) && *ptr && *ptr != SPACE)
	return (TRUE);
    return (FALSE);
}

/* sort: sort the edkeys list according to the user key name */
void sort (void)
{
    register int i, len = obj_count (edkeys);

    for (i=0; i<len; i++) {
	register int j;
	char *name = ((struct key *)edkeys [i])->keyname [0];
	
	for (j=i+1; j<len; j++) {
	    char *testname = ((struct key *)edkeys [j])->keyname [0];
	    if (strcmp (name, testname) > 0) {
	        char *tmp;
		tmp = edkeys [i];
		edkeys [i] = edkeys [j];
		edkeys [j] = tmp;
		name = testname;
	    }
	}
    }
}

/* construct_ufiles: generate keynames.h, tables.c and readkeys from unsorted
		     key list */
void construct_ufiles (void)
{
    FILE *fp;
    register int i, len = obj_count (edkeys);

    /* first build tables.c */
    fp = fopen ("tables.c", "w");
    if (fp == NULL) {
	(void) fprintf (stderr, "Cannot open tables.c\n");
	exit (1);
    }

    (void) fprintf (fp, "/* This file was generated from %s, and should not be directly modified */\n\n", filename);

    (void) fprintf (fp, "/* %s sid keywords are %s */\n\n", __FILE__, "src/tenplus/keys/keys.c, tenplus, tenplus411, GOLD410 7/19/91 14:46:33");
    (void) fprintf (fp, "#include <stdio.h>\n\n");
    (void) fprintf (fp, "#define NONE 0\n\n");

    /* put out externs for each function referenced */
    for (i=0; i<len; i++) {
	struct key *key = (struct key *)edkeys [i];
	if (! seq (key->functionname, "NULL")) {
	    show_ifdefs (fp, key->ifdefs);
	    (void) fprintf (fp, "extern void %s ();\n", key->functionname);
	    show_endifs (fp, key->ifdefs);
	}
    }

    /* build the functions list in the ed.keys order */
    (void) fprintf (fp, "\nvoid (*g_edfunction [])() = {\n");
    (void) fprintf (fp, "\t/* %-16s */\t%s,\n", place_holder, "NULL");
    for (i=0; i<len; i++) {
	struct key *key = (struct key *)edkeys [i];
	show_ifdefs (fp, key->ifdefs);
	(void) fprintf (fp, "\t/* %-16s */\t%s,\n", key->keyname[0],key->functionname);
	if (key->ifdefs) {
	    (void) fprintf (fp, "#else\n");
	    (void) fprintf (fp, "\t/* %-16s */\t%s,\n", place_holder, "NULL");
	}
	show_endifs (fp, key->ifdefs);
    }
    (void) fprintf (fp, "};\n\n");

    /* build the arguments list (also in ed.keys order). */
    (void) fprintf (fp, "int g_argbits [] = {\n");
    (void) fprintf (fp, "\t/* %-16s */\t%s,\n", place_holder, "NONE");
    for (i=0; i<len; i++) {
	int num_args = 0;
	int bits = 0;

	struct key *key = (struct key *)edkeys [i];

	show_ifdefs (fp, key->ifdefs);
	(void) fprintf (fp, "\t/* %-16s */\t", key->keyname [0]);

	if (key->noargs) {
	    bits |= NOARGS;
	    num_args++;
	}
	if (key->emptyarg) {
	    bits |= EMPTYARG;
	    num_args++;
	}
	if (key->stringarg) {
	    bits |= STRINGARG;
	    num_args++;
	}
	if (key->numberarg) {
	    bits |= NUMBERARG;
	    num_args++;
	}
	if (key->regionarg) {
	    bits |= REGIONARG;
	    num_args++;
	}
	if (key->lineregion) {
	    bits |= LINEREGION;
	    num_args++;
	}
	if (key->boxregion) {
	    bits |= BOXREGION;
	    num_args++;
	}
	if (key->textregion) {
	    bits |= TEXTREGION;
	    num_args++;
	}
	if (key->modifier) {
	    bits |= MODIFIER;
	    num_args++;
	}
	if (key->cursordef) {
	    bits |= CURSORDEF;
	    num_args++;
	}
	if (key->delstack) {
	    bits |= DELSTACK;
	    num_args++;
	}
	if (key->execflag) {
	    bits |= EXECFLAG;
	    num_args++;
	}
	if (key->canflag) {
	    bits |= CANFLAG;
	    num_args++;
	}
	if (key->invtxtflg) {
	    bits |= INVTXTFLG;
	    num_args++;
	}
	if (key->regionhandle) {
	    bits |= REGIONHANDLE;
	    num_args++;
	}

	if (num_args == 0)
	    (void) fprintf (fp, "NONE,\n");
	else
	    (void) fprintf (fp, "0x%x,\n", bits);

	if (key->ifdefs) {
	    (void) fprintf (fp, "#else\n");
	    (void) fprintf (fp, "\t/* %-16s */\tNONE,\n", place_holder);
	}
	show_endifs (fp, key->ifdefs);
    }
    (void) fprintf (fp, "};\n");
    (void) fflush (fp);
    if (ferror (fp)) {
	(void) fprintf (stderr, "I/O error writing tables.c\n");
	exit (1);
    }
    (void)fclose (fp);

    /* build keynames.h */
    fp = fopen ("keynames.h", "w");
    if (fp == NULL) {
	(void) fprintf (stderr, "Cannot open keynames.h\n");
	exit (1);
    }

    (void) fprintf (fp, "/* This file was generated from %s, and should not be directly modified */\n\n", filename);
    (void) fprintf (fp, "/* %s sid keywords are %s */\n\n", __FILE__, "src/tenplus/keys/keys.c, tenplus, tenplus411, GOLD410 7/19/91 14:46:33");
    for (i=0; i<len; i++) {
	struct key *key = (struct key *)edkeys [i];
	register int j;
	
	show_ifdefs (fp, key->ifdefs);
	(void) fprintf (fp, "#define %-16s %d\n", key->editorname [0], i+1);
	for (j=1; j<obj_count (key->editorname); j++)
	    (void) fprintf (fp, "#define %-16s %s\n", key->editorname [j],
	      key->editorname [0]);
	show_endifs (fp, key->ifdefs);
    }
    (void) fprintf (fp, "\n#define NUMKEYS %d\n", i+1);
    (void) fflush (fp);
    if (ferror (fp)) {
	(void) fprintf (stderr, "I/O error writing keynames.h\n");
	exit (1);
    }
    (void)fclose (fp);

    /* generate readkeys */
    fp = fopen ("readkeys", "w");
    if (fp == NULL) {
	(void) fprintf (stderr, "Cannot open readkeys\n");
	exit (1);
    }

    (void) fprintf (fp, "sed \"\n");

    len = obj_count (edkeys);
    for (i=0; i<len; i++) {
	struct key *key = (struct key *)edkeys [i];
	
	(void) fprintf (fp, "s/key = %d$/key = %s/\n", i+1, key->editorname [0]);
    }
    (void) fprintf (fp, "\"\n");
    (void)fclose (fp);
}

/* construct_sfiles: generate names.h from sorted key list */
void construct_sfiles (void)
{
    FILE *fp;
    register int i, len = obj_count (edkeys);
    POINTER *aliases, *names;

    /* generate names.h */
    fp = fopen ("names.h", "w");
    if (fp == NULL) {
	(void) fprintf (stderr, "Cannot open names.h\n");
	exit (1);
    }

    (void) fprintf (fp, "/* This file was generated from %s, and should not be directly modified */\n", filename);
    (void) fprintf (fp, "/* %s sid keywords are %s */\n\n", __FILE__, "src/tenplus/keys/keys.c, tenplus, tenplus411, GOLD410 7/19/91 14:46:33");
    (void) fprintf (fp, "static struct nstruct {\n");
    (void) fprintf (fp, "\tchar *name;\n");
    (void) fprintf (fp, "\tunsigned char value;\n");
    (void) fprintf (fp, "} names [] = {\n");

    /* generate the list of name value pairs */
    for (i=0; i<len; i++) {
	struct key *key = (struct key *)edkeys [i];
	int stlen = strlen (key->keyname [0]);

	(void) fprintf (fp, "\t\"%s\",", key->keyname [0]);
	if (stlen < 5)
	    (void) fprintf (fp, "\t");
	if (stlen < 13)
	    (void) fprintf (fp, "\t");
	(void) fprintf (fp, "\t%d,\t/* %-16s */\n", key->fnindex, key->editorname [0]);
    }
    (void) fprintf (fp, "};\n\n");

    /* generate the aliases list */
    aliases = (POINTER *)s_alloc (0, T_POINTER, NULL);
    names = (POINTER *)s_alloc (0, T_POINTER, NULL);

    (void) fprintf (fp, "static struct pairs {\n");
    (void) fprintf (fp, "\tchar *extname;\n");
    (void) fprintf (fp, "\tchar *intname;\n");
    (void) fprintf (fp, "} namepairs [] = {\n");
    for (i=0; i<len; i++) {
	struct key *key = (struct key *)edkeys [i];
	register int j, leng;

	/* generate a sorted list of aliases */
	leng = obj_count (key->keyname);
	for (j=1; j<leng; j++) {
	    register int listlen = obj_count (aliases);
	    register int k;
	    char *kname = key->keyname [j];

	    for (k=0; k<listlen; k++) {
		if (strcmp (aliases [k], kname) > 0)
		    break;
	    }
	    aliases = (POINTER *)s_insert ((char *)aliases, k, 1);
	    aliases [k] = kname;
	    names = (POINTER *)s_insert ((char *)names, k, 1);
	    names [k] = key->keyname [0];
	}
    }

    len = obj_count (aliases);
    for (i=0; i<len; i++) {
	register int stlen = strlen (aliases [i]);

	(void) fprintf (fp, "\t\"%s\",", aliases [i]);
	if (stlen < 5)
	    (void) fprintf (fp, "\t");
	if (stlen < 13)
	    (void) fprintf (fp, "\t");

	(void) fprintf (fp, "\t\"%s\",\n", names [i]);
    }

    (void) fprintf (fp, "};\n");

    s_freenode ((char *)aliases);
    s_freenode ((char *)names);

    (void) fflush (fp);
    if (ferror (fp)) {
	(void) fprintf (stderr, "I/O error writing names.h\n");
	exit (1);
    }
}

void show_ifdefs (FILE *fp, POINTER *iflist)
{
    register int i, len;

    if (iflist == NULL)
	return;

    len = obj_count (iflist);
    if (len > 1) {
	(void) fprintf (fp, "#if %s", iflist [0]);
	for (i=1; i<len; i++)
	    (void) fprintf (fp, "| %s", iflist [i]);
	(void) fprintf (fp, "\n");
    } else
	(void) fprintf (fp, "#ifdef %s\n", iflist [0]);
}

void show_endifs (FILE *fp, POINTER *iflist)
{
    if (iflist)
	(void) fprintf (fp, "#endif\n");
}
