#if !defined(lint)
static char sccsid[] = "@(#)55	1.7  src/tenplus/util/inseqs.c, tenplus, tenplus411, GOLD410 3/23/94 11:50:32";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	findcanon, doinseqs, domultseq, seqcomp, name_change, 
 *              initialise_inseqs, store_default, load_key,
 *              load_custom_key, same_function
 * 
 * ORIGINS:  9, 10
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
/*****************************************************************************
* File: inseqs.c - digest input sequences for the TENPLUS "termcaps" display *
*****************************************************************************/

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "database.h"
#include    "libobj.h"
#include    "names.h"
#include    "tdigest_msg.h"
#include    "util.h"

extern POINTER *g_outrec;
extern POINTER *g_inrec;
extern int g_recnum;
extern int g_nitems;

extern unsigned char *d_intran;  /* default intran          */
extern POINTER       *d_multseq; /* default multseq         */
unsigned char *intran;           /* custom keyboard intran  */
POINTER       *multseq;          /* custom keyboard multseq */

extern nl_catd g_catd;		/* Catalog descriptor for MF_TDIGEST */
#define MSGSTR(num, str)	catgets(g_catd, MS_TDIGEST, num, str)

int firstime;

/**** special values for input table slots
      SELF says this is a text character
      SEQ  says this is the first character of a sequence
      BAD  says this is a mistake *****/

#define SELF      0xfd
#define BAD       0xff
#define SEQ       0xfe

/*****************************************************************************
* findcanon: find the canonical value for a given name. Returns the canonica *
* or BAD (0xff) if the binary search fails                                   *
*****************************************************************************/

unsigned char findcanon (char *name)
    {
    int top, bottom, i;
    int comp;

    top = 0;
    bottom = (sizeof (names) / sizeof (struct nstruct)) - 1;

    /* binary search the names table                  */
    while (top <= bottom)
	{
	i = (top + bottom) / 2;
	comp = strcmp (name, names [i].name);

	if (comp == 0)
	    return (names [i].value);

	if (comp < 0)
	    bottom = i - 1;
	else if (comp > 0)
	    top = i + 1;
	}
    return (BAD);
    }

/*******************************************************************************
* doinseqs: digest input sequences, make 3 children in g_outrec:               *
* intran, multseq, and multcanon. Intran is the single-character canonical     *
* table, multseq is the multi-character sequence table, and multcanon is       *
* a parallel array to multseq giving the output canonical to go with the       *
* string.                                                                      *
*                                                                              *
* arguments:           int exclude_default_keys - indicates whether to exclude *
*                                                 the default key definitions  *
*                                                 from the keyboard definition *
*******************************************************************************/

void doinseqs (int exclude_default_keys)
    {
    POINTER *inseqs;
    POINTER *thisseq;

    char *thisvalue;
    char *thisname;

    unsigned char thiscanon;
    unsigned char *cvalue;

    int i;

    firstime = TRUE;

    multseq = (POINTER *) s_alloc (0, T_POINTER, "g_multseq");
    intran = (unsigned char *) s_alloc (256, T_CHAR, "g_intran");

    initialise_inseqs(exclude_default_keys);

    inseqs = (POINTER *) s_findnode (g_inrec, "Inseqs");

    if (inseqs == (POINTER *) ERROR || obj_type (inseqs) != T_POINTER)
	(void)fatal(MSGSTR(M_NO_INSEQ, "Input record %d has no defined input sequences."), g_recnum);

    for (i = 0; i < obj_count (inseqs); i++)
	{

	/* get name and value      */
	thisseq = (POINTER *) inseqs [i];
	thisname = s_findnode (thisseq, "Name");
	thisvalue = s_findnode (thisseq, "Value");

	if (thisvalue == (char *) ERROR)
	    continue;

	(void) s_typecheck (thisname, "doinseqs: Name",  T_CHAR);
	(void) s_typecheck (thisvalue, "doinseqs: Value", T_CHAR);

	/* ignore entry for ENTER because EXECUTE */
	/* is the same as ENTER                     */

	if (strcmp (thisname, "ENTER") == 0)
	    continue;

	/* change name to editor internals */
	thisname = name_change(thisname);

	/* do the convert */
	cvalue = convert (thisvalue, thisname, (char)TRUE);

	/* no blank junk in output please       */
	if (cvalue == NULL)
	    {
	    (void)s_free ((char *)thisseq);
	    inseqs [i] = NULL;
	    continue;
	    }

	thiscanon = findcanon (obj_name (cvalue));
	if (thiscanon == BAD)
	    (void)fatal(MSGSTR(M_UNK_CANONICAL,
			"Input record %d input canonical '%s' unknown."),
			g_recnum, obj_name (cvalue));

	if ((d_intran == NULL) || (exclude_default_keys))
	    load_key(thiscanon, cvalue);
	else
	    load_custom_key(thiscanon, cvalue, thisvalue);

	(void)s_free ((char *)thisseq);
	inseqs [i] = NULL;
	}

    if (d_intran == NULL)
	store_default();

    /* if there were any multi-character sequences                      */
    /* sort the table, make and hook into g_outrec the multcanon table  */

    if (obj_count (multseq) > 0)
	domultseq (multseq);
    else
	g_outrec [g_nitems++] = s_alloc (0, T_CHAR, "g_multcanon");

    /* hook up the multseq and intran tables                            */
    g_outrec [g_nitems++] = (char *) multseq;
    g_outrec [g_nitems++] = (char *) intran;
    g_outrec = (POINTER *) s_realloc ((char *)g_outrec, g_nitems);
    }

/*****************************************************************************
* domultseq: sort the multseq table, make up the multcanon table, and        *
* strip the obj_name's from the individual sequences.                        *
*****************************************************************************/

void domultseq (POINTER *multseq)
    {
    int i;
    unsigned char *multcanon;
    unsigned char thiscanon;

    /* sort that table                          */
    (void)qsort (multseq, obj_count (multseq), sizeof (POINTER), seqcomp);

    /* make sure that the same sequence isnt used twice */
    for (i = 0; i < obj_count (multseq) - 1; i++)
	{
	if (!strcmp (multseq [i], multseq [i + 1]))
	    (void)fatal(MSGSTR(M_SAME_KEY,
		"Input record %d Same input sequence used for %s and %s."),
		g_recnum, obj_name (multseq [i]), obj_name (multseq [i + 1]));
	}

    multcanon = (unsigned char *) s_alloc (obj_count (multseq), T_CHAR,
						"g_multcanon");

    for (i = 0; i < obj_count (multseq); i++)
	{
	thiscanon = findcanon (obj_name (multseq [i]));
	if (thiscanon == BAD)
	    (void)fatal(MSGSTR(M_UNK_CANONICAL,
			"Input record %d input canonical '%s' unknown."),
			g_recnum, obj_name (multseq [i]));

	multcanon [i] = thiscanon;

	/* strip the names to save space in the editor          */
	(void)s_newname (multseq [i], NULL);
	}
    g_outrec [g_nitems++] = (char *) multcanon;
    }

/*****************************************************************************
* seqcomp: compare (i.e. pass to strcmp) two sequences, for qsort            *
*****************************************************************************/

int seqcomp (POINTER *p1, POINTER *p2)
    {
    return (strcmp (*p1, *p2));
    }

/*****************************************************************************
* name_change: convert name to editor internal name                          *
*****************************************************************************/
char *name_change(char *name)
{
    int top, bottom, i;
    int comp;

    top = 0;
    bottom = (sizeof (namepairs) / sizeof (struct pairs)) - 1;

    /* binary search the namepairs table                  */
    while (top <= bottom)
	{
	i = (top + bottom) / 2;
	comp = strcmp (name, namepairs [i].extname);

	if (comp == 0)
	    return (namepairs [i].intname);

	if (comp < 0)
	    bottom = i - 1;
	else if (comp > 0)
	    top = i + 1;
	}
    return (name);
    }

/*******************************************************************************
* initialise_inseqs                                                            *
*                                                                              *
* Initialise intran and multseq tables.                                        *
* If the input sequences for the default keyboard have been processed then     *
* intran is initialised with all values from d_intran (the default keyboard    *
* single character canonicals); and                                            *
* multseq is initialised with all values from d_multseq (the default keyboard  *
* multiple character sequences).                                               *
* However, if the default keyboard has not been processed (d_intran is NULL    *
* valued) or we wish to exclude the default keyboard subset from the custom    *
* keyboard definition (exclude_default_keys argument has the value TRUE) then  *
* intran and multseq are initialised without any default keyboard subset.      *
*                                                                              *
* arguments:           int exclude_default_keys - indicates whether to exclude *
*                                                 the default key definitions  *
*                                                 from the keyboard definition *
*******************************************************************************/

void initialise_inseqs(int exclude_default_keys)
    {

    register int i;

    if ((d_intran == NULL) || (exclude_default_keys))
	{

	/*
	** initialise intran without default keyboard subset
	*/

	/*
	 * set the single character table to BAD or SELF ( TEXTKEY )
	 * isprint() takes care of locale dependency.
	 */
	for (i = 0; i < 256; i++)
	    if ( isprint(i) )
		intran [i] = SELF;
	    else
		intran [i] = BAD;

	}
    else
	{

	/*
	** initialise custom keyboard intran and multseq with defaults
	*/

	for (i = 0; i < 256; i++)
	    intran [i] = d_intran [i];

	for (i = 0; i < obj_count(d_multseq); i++)
	    {
	    multseq = s_append(multseq, s_string(d_multseq[i]));
	    s_newname(multseq[i], obj_name(d_multseq[i]));
	    }
	}
    }

/*******************************************************************************
* store_default: save intran and multseq tables to d_intran and d_multseq      *
* respectively. d_intran and d_multseq will contain the intran and multseq     *
* table values used to provide the default keyboard definition subset          *
* in custom keyboard definitions.                                              *
*******************************************************************************/

void store_default(void)
    {
    register int i;

    d_intran = (unsigned char *) s_alloc(256, T_CHAR, "d_intran");
    d_multseq = (POINTER *) s_alloc (0, T_POINTER, "d_multseq");

    for (i = 0; i < 256; i++)
	d_intran [i] = intran [i];

    for (i = 0; i < obj_count(multseq); i++)
	{
	d_multseq = s_append(d_multseq, s_string(multseq [i]));
	s_newname(d_multseq [i], obj_name(multseq[i]));
	}

    }

/*******************************************************************************
* load_key                                                                     *
* Process a key input sequence for the default keyboard or a keyboard which    *
* must not include default keyboard key input sequences.                       *
*******************************************************************************/

void load_key(
    unsigned char thiscanon,
    unsigned char *cvalue)
    {

    if (obj_count (cvalue) == 1)
	{

	/*
	** process single character key sequence
	*/

	if (intran [*cvalue] != BAD)
	    if (firstime)
		{
		int j;
		firstime = FALSE;
		for (j = 0240; j < 0377; j++) 
		    intran [j] = BAD;
		}
	    else
		(void)fatal(MSGSTR(M_ALREADY_ASSIGNED,
			"Input record %d input char 0%o already assigned (canonical '%s')."),
			g_recnum, (int) *cvalue, obj_name (cvalue));

	intran [*cvalue] = thiscanon;

	}
    else
	{

	/*
	** process multiple character key sequences
	*/

	if ((intran [*cvalue] != BAD) && (intran [*cvalue] != SEQ))
	    if (firstime)
		{
		int j;
		firstime = FALSE;
		for (j = 0240; j < 0377; j++) 
		    intran [j] = BAD;
		}
	    else
		(void)fatal(MSGSTR(M_ALREADY_ASSIGNED,
			"Input record %d input char 0%o already assigned (canonical '%s')."),
			g_recnum, (int) *cvalue, obj_name (cvalue));

	intran [*cvalue] = SEQ;

	multseq = s_append (multseq, cvalue);
	}
    }

/*******************************************************************************
* load_custom_key                                                              *
* Process a key input sequence for the custom keyboard. This involves          *
* loading key input sequences from the custom keyboard definition ensuring     *
* that existing key input sequences for the default keyboard subset are        *
* preserved (not overwritten).                                                 *
*******************************************************************************/

void load_custom_key(
    unsigned char thiscanon,
    unsigned char *cvalue,
    char *key_input_sequence)
    {

    if (obj_count(cvalue) == 1)
	{
	
	/*
	** process single character key sequence
	*/

	if (intran[*cvalue] == BAD)
	    intran[*cvalue] = thiscanon;
	else
	    {

	    /*
	    ** ignore keys that are already defined with the same
	    ** existing canonical
	    */

	    if (intran[*cvalue] != thiscanon)
		if (d_intran[*cvalue] == BAD)
		    (void)fatal(MSGSTR(M_ALREADY_ASSIGNED,
			"Input record %d input char 0%o already assigned (canonical '%s')."),
			g_recnum, (int) *cvalue, obj_name(cvalue));
		else
		    (void)fatal(MSGSTR(M_ALREADY_IN_DEFAULT,
			"Input record %d input char 0%o already in default (canonical '%s')."),
			g_recnum, (int) *cvalue, obj_name(cvalue));
	    }
	}
    else
	{

	/*
	** process multiple character key sequences
	*/

	if (intran[*cvalue] == BAD)
	    {

	    /* overwrite BAD default key with custom key definition */

	    intran[*cvalue] = SEQ;
	    multseq = s_append(multseq, cvalue);
	    }
	else if (intran[*cvalue] == SEQ)
	    {

	    /* first character is already part of a sequence */

	    if (s_inlist(cvalue, multseq))
		{

		/* key input sequence is already in multseq */

		if (!same_function(cvalue, multseq))
		    {

		    /* key input sequence function conflicts with */
		    /* previous function assignment               */

		    if (s_inlist(cvalue, d_multseq))
			(void)fatal(MSGSTR(M_ALREADY_IN_DEFAULT,
			"Input record %d input char 0%o already in default (canonical '%s')."),
			g_recnum, key_input_sequence, obj_name(cvalue));
		    else
			(void)fatal(MSGSTR(M_ALREADY_ASSIGNED,
			"Input record %d input char 0%o already assigned (canonical '%s')."),
			g_recnum, key_input_sequence, obj_name(cvalue));
		    }
		}
	    else
		/* place new key input sequence into multseq */
		multseq = s_append(multseq, cvalue);
	    }
	else
	    /* sequence is already assigned */
	    (void)fatal(MSGSTR(M_ALREADY_ASSIGNED,
			"Input record %d input char 0%o already assigned (canonical '%s')."),
			g_recnum, (int) *cvalue, obj_name(cvalue));
	}
    }

/*******************************************************************************
* same_function: Determine whether key input sequence exists in multseq table  *
* and whether the function assigned to be performed by the key input sequence  *
* is different from the function already recorded for the sequence.            *
*******************************************************************************/

int same_function(
    unsigned char *cvalue,
    POINTER *multseq)
    {
    register int i;

    for (i = 0; i < obj_count(multseq); i++)
	if (multseq[i] && (strcmp(multseq[i], cvalue) == 0))
	    if (strcmp(obj_name(multseq[i]), obj_name(cvalue)) == 0)
		return TRUE;

    return FALSE;

    }

