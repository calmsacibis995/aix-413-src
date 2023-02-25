static char sccsid[] = "@(#)59	1.18  src/tcpip/usr/lib/libsnmp/objects.c, snmp, tcpip411, 9434A411a 8/18/94 14:20:24";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: THASH(), ot_compar(), readobjects(), sortobjects(), 
 *            importobjects(), read_type(), add_objects(), add_objects_aux(), 
 *            text2oid(), resolveot(), cresolveot(), name2obj(), name2dpo(), 
 *            name2dpo2(), ltext2obj(), text2obj(), oid2ode_aux(), name2inst(), 
 *            nextot2inst(), next2inst(), text2inst(), dump_objects_by_text(),
 *            dump_objects_by_tree(), dump_object_by_tree(), 
 *            dump_objects_by_xxx(), dump_object(), strdup(), 
 *	      get_anchor(), rcistrcmp(), cistrncmp(), object_remove(), 
 *            unchain()
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/objects.c
 */

/* 
 * Contributed by NYSERNet Inc.  This work was partially supported by the
 * U.S. Defense Advanced Research Projects Agency and the Rome Air Development
 * Center of the U.S. Air Force Systems Command under contract number
 * F30602-88-C-0016.
 *
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* objects.c - SMI object handling */

#include <ctype.h>
#include <stdio.h>
#include <string.h> 
#include <isode/snmp/objects.h>
#include <isode/snmp/logging.h>
#include <isode/general.h>
#include <isode/config.h>
#include "snmpd_msg.h"

#define DUMP		/* provide object dump functions */
#define	CI_HASH		/* use case-insensitive hash function */

#define	ADVISE		if (o_advise) (*o_advise)

/* DATA */

#define	TBUCKETS	0x80

static int once_only = 0;
static	int	compile_flag;
static	OT      compile_heap1;
static	char   *compile_heap2;
static	unsigned int *compile_heap3;
static	OID	compile_heap4;
static	char   *compile_heap5, *ch5;
static	OT	Tbuckets[TBUCKETS];

static	OT	anchor;
static	OT	chain;

static	int	sortobjects ();
static	OT	name2dpo (), name2dpo2 ();

nl_catd snmpcatd;

extern	int	errno;
int debug;

/*    OBJECTS */

#ifdef CI_HASH
#define STRCMP lexequ

/* hash on 1st character, character at len/2 + 1, and 2nd to last character */
static int THASH (name)
char *name;
{
    register char a, b;
    register int len;

    len = strlen (name);
    if (len < 2)
	return (*name & 0x7f);
    if (len > 3) {
	a = name[len/2 + 1];
	b = name[len - 2];
    }
    else {
	a = name[1];
	b = name[2];
    }
    return (((*name - a) & 0x1f) + (b & 0x5f));
}

#else
#define STRCMP strcmp

static int THASH (name)
char   *name;
{
    char    c;
    register char *cp,
		  *dp;

    for (c = *(dp = cp = name); *cp; cp++)
	if (isupper (*cp))
	    c = tolower (*(dp = cp));
    if (*++dp)
	return (((c - *dp) & 0x1f) + (*(dp + 1) & 0x5f));
    else
	return (c & 0x7f);
}
#endif

#define	OT_XXX	0x04

static int	ot_compar (a, b)
OT    *a,
      *b;
{
    int	    i = oid_cmp ((*a) -> ot_name, (*b) -> ot_name);

    if (i == 0
	    && (!((*a) -> ot_access & OT_XXX)
		    || !((*b) -> ot_access & OT_XXX))) {
	OID	oid = (*a) -> ot_name;
	register unsigned int *ip = oid -> oid_elements;

					       /* XXX: 0.0 is a special case */
	if (oid -> oid_nelem == 2 && ip[0] == 0 && ip[1] == 0)
	    return 0;

	/*
	 * allow duplicate objects with identical syntax.
	 * hidden result: aliases
	 */
	if (((*a) -> ot_syntax != (*b) -> ot_syntax)) {
	    sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_1,
		"duplicate objects, different syntax: \"%s\" and \"%s\""),
			(*a) -> ot_text, (*b) -> ot_text);
	    ADVISE (SLOG_EXCEPTIONS, PY_pepy);
	}
	else if (debug) 
	    ADVISE (SLOG_DEBUG, MSGSTR(MS_OBJECTS, OBJECTS_2,
		    "duplicate objects: \"%s\" and \"%s\""),
		    (*a) -> ot_text, (*b) -> ot_text);

	(*a) -> ot_access |= OT_XXX, (*b) -> ot_access |= OT_XXX;
    }

    return i;
}

/*  */
/*
 * a spin-off of _isodefile, without calling isodetailor
 * currently only used by readobjects ().
 */
static char *
etcfile (file)
char *file;
{
    static char buffer[BUFSIZ];
    char *path = "/etc/";

    if (*file == '/'
	    || (*file == '.'
		    && (file[1] == '/'
			    || (file[1] == '.' && file[2] == '/'))))
	(void) strcpy (buffer, file);
    else
	(void) sprintf (buffer, "%s%s", path, file);
    
    return buffer;
}

static char *roots[] = { "ccitt", "iso", "joint-iso-ccitt" };

int	readobjects (file)
char   *file;
{
    register char *cp,
		 **ap;
    char    buffer[BUFSIZ],
	    line[BUFSIZ],
	   *vec[NVEC + NSLACK + 1];
    FILE   *fp;

    PY_pepy[0] = NULL;

    if (file == NULL)
	file = "mib.defs";
    if ((fp = fopen (file, "r")) == NULL
	    && (fp = fopen (etcfile (file), "r")) == NULL) {
	(void) sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_9,
			"unable to read %s: %s\n"),
			file, strerror(errno));
	return NOTOK;
    }

    if (once_only == 0) {
	bzero ((char *) Tbuckets, sizeof Tbuckets);

	readsyntax ();

	compile_flag = -1;
	if (fgets (buffer, sizeof buffer, fp)
	        && strncmp (buffer, "--* compiled",
			    sizeof "--* compiled" - 1) == 0) {
	    int	    i,
		    j,
		    k;

	    compile_flag = 1;
	    anchor = chain = NULL;

	    if (sscanf (buffer, "--* compiled %d %d %d", &i, &j, &k) == 3) {
		if ((compile_heap1 = (OT) calloc ((unsigned) i,
						  sizeof *compile_heap1))
		        == NULL) {
		    (void) sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_10,
				"out of memory allocating %d objects"), i);
		    goto you_lose;
		}

		if ((compile_heap2 = (char *)calloc(1, (unsigned) j)) == NULL) {
		    (void) sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_11,
				"out of memory allocating %d octets"), j);
		    goto you_lose;
		}

		if ((compile_heap3 = (unsigned int *)
				calloc ((unsigned) k, sizeof *compile_heap3))
		        == NULL) {
		    (void) sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_12,
				"out of memory allocating %d sub-identitifers"),
				k);
		    goto you_lose;
		}

		if ((compile_heap4 = (OID) calloc ((unsigned) i,
						   sizeof *compile_heap4))
		        == NULL) {
		    (void) sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_13,
				"out of memory allocating %d identitifers"),
				i);
		    goto you_lose;
		}

		/* just a best guesstimate... */
		if ((ch5 = compile_heap5 = (char *)calloc(1,(unsigned) i * 160)) 
			== NULL) {
		    (void) sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_11,
				"out of memory allocating %d octets"), i * 160);
		    goto you_lose;
		}
	    }
	    else
		compile_heap1 = NULLOT,
		    compile_heap2 = NULL,
			compile_heap3 = NULL,
			    compile_heap4 = NULL,
				ch5 = compile_heap5 = NULL;
	}
	else {
	    compile_flag = 0;
	    (void) rewind (fp);

	    /* install the roots */
	    vec[2] = 0;
	    for (ap = roots + (sizeof roots / sizeof roots[0]) - 1;
		    ap >= roots;
		    ap--) {
		(void) sprintf (buffer, "%d", ap - roots);
		vec[0] = *ap;
		vec[1] = buffer;
		if (read_type (vec) == NOTOK) {
you_lose: ;
		    (void) fclose (fp);
		    return NOTOK;
		}
	    }
	}

	once_only = 1;
    }
    else
	if (compile_flag > 0) {
	    (void) sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_15,
			"only one compiled file is allowed"));
	    goto you_lose;
	}

    while (cp = fgets (buffer, sizeof buffer, fp)) {
	while (*cp && isspace (*cp))
	    cp++;
	if (*cp == '#' || strncmp (cp, "--", 2) == 0)
	    continue;
	if (cp = index (buffer, '\n'))
	    *cp = NULL;
	(void) strcpy (line, buffer);

	bzero ((char *) vec, sizeof vec);
	switch (str2vec (buffer, vec)) {
	    case 0:
	        break;

	    case 2:
	    case 5:
		if (read_type (vec) == NOTOK)
		    return NOTOK;
		break;

	    default:
		(void) sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_16,
				"malformed line: \"%s\""), line);
		return NOTOK;
	}
    }

    (void) fclose (fp);

    if (compile_flag < 1) 
	(void) sortobjects ();
    else {
	if (add_objects_aux () == NOTOK)
	    return NOTOK;
    }

#ifdef	notdef
    dump_objects_by_text ();
#endif

    return (PY_pepy[0] ? NOTOK : OK);
}

/* sorts hashed objects, assigns ot -> ot_next pointers and anchor pointer. */
static int
sortobjects ()
{
    register int   j, i;
    register OT	   ot,
		  *op,
		  *ep;
    OT	   *base;

    j = 0;
    for (i = 0; i < TBUCKETS; i++)
	for (ot = Tbuckets[i]; ot && ot -> ot_text; ot = ot -> ot_chain)
	    j++;

    /* j > 1 ALWAYS */
    
    if ((base = (OT *) malloc ((unsigned) (j * sizeof *base))) == NULL) {
	(void) sprintf (PY_pepy, MSGSTR(MS_GENERAL, GENERA_1, 
			"%s: out of memory"), "sortobjects");
	return NOTOK;
    }

    op = base;
    for (i = 0; i < TBUCKETS; i++)
	for (ot = Tbuckets[i]; ot && ot -> ot_text; ot = ot -> ot_chain)
	    *op++ = ot;
    ep = op;

#ifdef _ANSI_C_SOURCE
    qsort ((char *) base, j, sizeof *base, 
	   (int(*)(const void *, const void *))ot_compar);
#else /* !_ANSI_C_SOURCE */
    qsort ((char *) base, j, sizeof *base, ot_compar);
#endif /* _ANSI_C_SOURCE */

    op = base;
    anchor = ot = *op++;
    ot -> ot_prev = NULL;
    while (op < ep) {
	ot -> ot_next = *op;
	ot -> ot_next -> ot_prev = ot;
	ot = *op++;
    }
    (chain = ot) -> ot_next = NULL;

    free ((char *) base);

    return OK;
}

int
importobjects (iobjects, syntax)
IOT	iobjects;
char	*syntax[];
{
    IOT iot;
    OT	ot, dot;
    int i;

    PY_pepy[0] = NULL;

    if (once_only == 0) {
	bzero ((char *) Tbuckets, sizeof Tbuckets);

	readsyntax ();
	once_only = 1;
    }

    for (iot = iobjects; iot -> iot_text != NULLCP; iot++) {

	dot = text2obj (iot -> iot_text);

	if ((ot = (OT) calloc (1, sizeof *ot)) == NULL) {
	    (void) sprintf (PY_pepy, MSGSTR(MS_GENERAL, GENERA_1, 
			"%s: out of memory"), "importobjects");
	    return NOTOK;
	}

	/* map over minimal information from import_objects */
 	ot -> ot_text = (char *)malloc((int)strlen(iot -> iot_text) + 1);
	ot -> ot_id = (char *)malloc((int)strlen(iot -> iot_id) + 1);

	strcpy(ot -> ot_text , iot -> iot_text);
	strcpy(ot -> ot_id , iot -> iot_id);

	ot -> ot_access = iot -> iot_access;
	ot -> ot_status = iot -> iot_status;

	if (iot -> iot_syntax 
		&& (ot -> ot_syntax = text2syn (syntax[iot -> iot_syntax])) 
			== NULL
		/* && debug */) {

	    /* we should never get here.... */
	    ADVISE (SLOG_WARNING, MSGSTR(MS_OBJECTS, OBJECTS_3, 
		    "object \"%s\" has unknown syntax \"%s\""),
		    ot -> ot_text, syntax[iot -> iot_syntax]);
	}

	if (resolveot (ot, dot) == NOTOK)
	    return NOTOK;
    }

    (void) sortobjects ();
    return (PY_pepy[0] ? NOTOK : OK);
}

/*  */

static int  read_type (vec)
char  **vec;
{
    register OT	   ot, dot = NULL;

    if (compile_flag < 1)
	dot = text2obj (*vec);

    if (compile_flag > 0 && compile_heap1)
	ot = compile_heap1++;
    else
	if ((ot = (OT) calloc (1, sizeof *ot)) == NULL) {
	    (void) sprintf (PY_pepy, MSGSTR(MS_GENERAL, GENERA_1, 
			"%s: out of memory"), "read_type");
	    return NOTOK;
	}

    if (compile_flag > 0 && compile_heap2) {
	(void) strcpy (ot -> ot_text = compile_heap2, *vec++);
	compile_heap2 += strlen (compile_heap2) + 1;

	(void) strcpy (ot -> ot_id = compile_heap2, *vec++);
	compile_heap2 += strlen (compile_heap2) + 1;
    }
    else
	if ((ot -> ot_text = strdup (*vec++)) == NULL
		|| (ot -> ot_id = strdup (*vec++)) == NULL) {
	    (void) sprintf (PY_pepy, MSGSTR(MS_GENERAL, GENERA_2,
			"%s: out of memory"), "read_type");
	    return NOTOK;
	}

    /* ... the rest is for 5 vector types */
    if (*vec) {
	if ((ot -> ot_syntax = text2syn (*vec)) == NULL
		&& lexequ (*vec, "Aggregate")
		&& debug)
	    ADVISE (SLOG_WARNING, MSGSTR(MS_OBJECTS, OBJECTS_3,
		     "object \"%s\" has unknown syntax \"%s\""),
		     ot -> ot_text, *vec);
	vec++;
    
	if (lexequ (*vec, "read-only") == 0)
	    ot -> ot_access = OT_RDONLY;
	else
	    if (lexequ (*vec, "read-write") == 0)
		ot -> ot_access = OT_RDWRITE;
	    else
		if (lexequ (*vec, "write-only") == 0)
		    ot -> ot_access = OT_WRONLY;
		else
		    if (lexequ (*vec, "not-accessible") && debug)
			ADVISE (SLOG_WARNING, MSGSTR(MS_OBJECTS, OBJECTS_4,
			     "object \"%s\" has unknown access \"%s\""),
			     ot -> ot_text, *vec);
	vec++;

	if (lexequ (*vec, "mandatory") == 0)
	    ot -> ot_status = OT_MANDATORY;
	else
	    if (lexequ (*vec, "optional") == 0)
		ot -> ot_status = OT_OPTIONAL;
	    else
		if (lexequ (*vec, "deprecated") == 0)
		    ot -> ot_status = OT_DEPRECATED;
		else
		    if (lexequ (*vec, "obsolete") && debug)
			ADVISE (SLOG_WARNING, MSGSTR(MS_OBJECTS, OBJECTS_5,
			     "object \"%s\" has unknown status \"%s\"\n"),
			     ot -> ot_text, *vec);
    }

    return (compile_flag < 1) ? resolveot (ot, dot) : cresolveot (ot);
}

/*  */

/* does not insert into THASH table... */
/* XXX: nor does it build up ot_ltext... */

int	add_objects (ot)
register OT	ot;
{
    register OID oid = ot -> ot_name;
    register OT	 ot2,
		*otp;

    if (oid_cmp (chain -> ot_name, oid) < 0) {
	chain -> ot_next = ot;
	(chain = ot) -> ot_next = NULLOT;
    }
    else {
	for (otp = &anchor; ot2 = *otp; otp = &ot2 -> ot_next)
	    if (oid_cmp (ot2 -> ot_name, oid) > 0)
		break;
	ot -> ot_next = ot2;
	*otp = ot;
    }

    for (ot = anchor; ot; ot = ot -> ot_next)
	ot -> ot_sibling = ot -> ot_children = NULLOT;

    return add_objects_aux ();
}

static int 
add_objects_aux ()
{
    register OT	ot,
		ot2;

    for (ot = anchor; ot; ot = ot -> ot_next) {
	if (ot -> ot_name -> oid_nelem <= 1)
	    continue;
	if ((ot2 = name2dpo (ot -> ot_name)) != NULLOT) {
	    ot -> ot_sibling = ot2 -> ot_children;
	    ot2 -> ot_children = ot;
	}
	else
	    if (debug)
		ADVISE (SLOG_DEBUG, MSGSTR(MS_OBJECTS, OBJECTS_6,
			"no distant parent for %s"), sprintoid (ot -> ot_name));
    }

    return OK;
}

/*  */

OID	text2oid (rname)
char   *rname;
{
    int	    i,
	    j;
    register unsigned int  *ip,
			   *jp;
    unsigned int elements[NELEM + 1];
    register char *cp;
    register char *name;
    OID	    oid,
	    new;

    name = (char *)malloc((int)strlen(rname) + 1);
    strcpy(name, rname);

    for (cp = name + strlen (name) - 1; cp >= name; cp--)
	if (!isdigit (*cp) && *cp != '.')
	    break;
    cp++;
    if (isdigit (*cp) && cp != name)
	for (cp++; *cp; cp++)
	    if (*cp == '.')
		break;

    if (*cp == NULL)		/* name */
	i = 0;
    else
	if (cp == name) {	/* 1.3.6.1.2.1.1.1.0 */
	    if ((i = str2elem (cp, elements)) < 2)
		return NULL;
	}
	else			/* name.numbers */
	    if ((i = str2elem (cp + 1, elements)) < 1)
		return NULL;

    /* name || name.numbers, lookup "name" */
    if (cp != name) {
	OT ot;

	*cp = NULL;

	ot = text2obj (name);
	if (i != 0)
	    *cp = '.';

	cp = NULL;
	free(name);

	if (ot == NULLOT || ot -> ot_name == NULLOID)
	    return NULLOID;
	oid = ot -> ot_name;

	/* trailing instance(s)? */
	if (i == 0) {		/* no. */
	    if (oid -> oid_nelem <= 1)	/* MUST have more than 1 elem (BER) */
		return NULLOID;
	    else
		return oid_cpy (oid);
	}

	j = oid -> oid_nelem;
    }
    else
	oid = NULL, j = 0;

    if ((new = (OID) malloc (sizeof *new)) == NULLOID) 
	return NULLOID;
    if ((ip = (unsigned int *) malloc ((unsigned) (i + j + 1) * sizeof *ip))
		== NULL) {
	free ((char *) new);
	return NULLOID;
    }

    new -> oid_elements = ip, new -> oid_nelem = i + j;

    if (oid) 
	for (j = 0, jp = oid -> oid_elements; j < oid -> oid_nelem; j++, jp++)
	    *ip++ = *jp;

    if (i > 0)
	for (j = 0, jp = elements; j < i; j++, jp++)
	    *ip++ = *jp;

    new -> oid_nelem = ip - new -> oid_elements;

    return new;
}

/*  RESOLVE */

/* 
 * resolve for "normal" defs files
 * ie:
 *
 * sysDescr         system.1         DisplayString   read-only       mandatory
 */
static int
resolveot (ot, dot)
OT ot, dot;
{
    int	    i;
    unsigned int elements[NELEM + 1];
    register char *cp;
    register OT	   ot2;
    struct OIDentifier oids;
    register OID   oid = &oids;
    int sibmode = 0;

    oid -> oid_elements = elements;

    /* XXX: Special cases, digits for id... */
    if (isdigit (*ot -> ot_id)) {
	ot2 = NULLOT;
	cp = NULLCP;
	if ((oid -> oid_nelem = 
		str2elem (ot -> ot_id, oid -> oid_elements)) < 1){
	    sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_7,
		    "str2elem failture on \"%s\""), ot -> ot_id);
	    ADVISE (SLOG_EXCEPTIONS, PY_pepy);
	    return NOTOK;
	}

	/* XXX: do we care about massaging the long name?
	 *    I think not.  Instead, if we detect digits for
	 *    the ot_id, we just plug ot_text into ot_ltext.
	 *    Old code is left here anyway.
	 */
#ifdef notdef
	/* build up ltext... */
	if (oid -> oid_nelem == 1) {			/* roots! */
	    if ((ot -> ot_ltext = strdup (ot -> ot_text)) == NULL) 
		goto no_mem;
	}
	else if (oid -> oid_nelem == 2 			/* 0.0 */
		&& oid -> oid_elements[0] == 0 
		&& oid -> oid_elements[1] == 0) {
	    if ((ot -> ot_ltext = strdup ("")) == NULL)
		goto no_mem;
	}
	else						/* n.n.? */
	    /* XXX: what do we do here...? May need some massaging... */
	    if ((ot -> ot_ltext = strdup (ot -> ot_id)) == NULL) 
		goto no_mem;
#else
	/* duplicate ot_text into ot_ltext */
	if ((ot -> ot_ltext = strdup (ot -> ot_text)) == NULL) 
	    goto no_mem;
#endif
    }

    /* handle our normal "text.id" cases */
    else {
	/* strip instance off of id to locate the parent */
	if ((cp = index (ot -> ot_id, '.')) != NULL)
	    *cp = '\0'; 

	for (ot2 = chain; 
		ot2 && STRCMP (ot -> ot_id, ot2 -> ot_text);
		ot2 = ot2 -> ot_prev) 
	    ;
	if (cp)
	    *cp = '.';

	/* checks on the parent */
	if (ot2 == NULLOT || ot2 -> ot_ltext == NULL) {
	    sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_17,
		    "unable to locate object parent \"%s\" for \"%s\""),
		    ot->ot_id, ot -> ot_text);
	    return NOTOK;
	}

	if (ot2 -> ot_name == NULLOID) {	/* should never happen... */
	    sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_18,
		     "Unresolved parent for \"%s\""), ot -> ot_text);
	    return NOTOK;
	}

	/* build up our fully qualified long name */
	ot -> ot_ltext = (char *)malloc (strlen (ot -> ot_text) + 
				    strlen (ot2 -> ot_ltext) + 2);
	sprintf (ot -> ot_ltext, "%s_%s", ot2 -> ot_ltext, ot -> ot_text);

	/* build up our OID */
	oid -> oid_nelem = ot2 -> ot_name -> oid_nelem;
	bcopy ((char *) ot2 -> ot_name -> oid_elements,
	       (char *) oid -> oid_elements,
	       oid -> oid_nelem * sizeof *elements);

    }

    if (cp != NULLCP) {		/* should always happen ... (well, almost!) */
	if ((i = str2elem (++cp, oid -> oid_elements + oid -> oid_nelem)) < 1){
	    free (ot -> ot_ltext);
	    free (ot -> ot_id);
	    free (ot -> ot_text);
	    free (ot);          /* discard this one */
	    sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_19,
		     "str2elem failure on \"%s\""), ot -> ot_id);
	    return NOTOK;
	}
	sibmode = 1;
	oid -> oid_nelem += i;
	ot -> ot_sibling = ot2 -> ot_children;
	ot2 -> ot_children = ot;
    }
    if ((ot -> ot_name = oid_cpy (oid)) == NULLOID) {
no_mem:;
	if (sibmode)
	  ot2 -> ot_children = ot -> ot_sibling;
	free (ot -> ot_ltext);
	free (ot -> ot_id);
	free (ot -> ot_text);
	free (ot);              /* discard this one */
	sprintf (PY_pepy, MSGSTR(MS_GENERAL, GENERA_1, "%s: out of memory"),
		 "resolveot");
	return NOTOK;
    }

    /* duplicate entry detected? */
    if (dot) {
	if ((!STRCMP (ot -> ot_ltext, dot -> ot_ltext)) &&
	    (oid_cmp(ot -> ot_name, dot -> ot_name) == 0)) {  /* same tree */
	    if (sibmode)
	      ot2 -> ot_children = ot -> ot_sibling;
	    oid_free(ot -> ot_name);
	    free (ot -> ot_ltext);
	    free (ot -> ot_id);
	    free (ot -> ot_text);
	    free (ot);		/* discard this one */
	    return OK;
	}
	else {
	    sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_20, 
		     "duplicate object name: \"%s\""), ot -> ot_text);
	    if (sibmode)
	      ot2 -> ot_children = ot -> ot_sibling;
	    oid_free(ot -> ot_name);
	    free (ot -> ot_ltext);
	    free (ot -> ot_id);
	    free (ot -> ot_text);
	    free (ot);          /* discard this one */
	     return NOTOK;
	}
    }

    return insert_THASH (ot);
}

/*
 * resolve for "compiled" defs files.
 * ie:
 *
 * sysDescr     1.3.6.1.2.1.1.1      DisplayString   read-only       mandatory
 */
static int
cresolveot (ot)
OT ot;
{
    register OID oid;
    register OT ot2;

    if ((oid = str2oid (ot -> ot_id)) == NULLOID) {
	(void) sprintf (PY_pepy, MSGSTR(MS_OBJECTS, OBJECTS_21,
			"invalid OID (%s) for \"%s\" in compiled file"),
			ot -> ot_id, ot -> ot_text);
	return NOTOK;
    }
    if (compile_heap3) {
	ot -> ot_name = compile_heap4++;
	bcopy ((char *) oid -> oid_elements,
	       (char *) (ot -> ot_name -> oid_elements
						    = compile_heap3),
	       (ot -> ot_name -> oid_nelem = oid -> oid_nelem)
			* sizeof *oid -> oid_elements);
	compile_heap3 += oid -> oid_nelem + 1;
    }
    else
	if ((ot -> ot_name = oid_cpy (oid)) == NULLOID) {
	    (void) sprintf (PY_pepy, MSGSTR(MS_GENERAL, GENERA_1,
			    "%s: out of memory"), "cresolveot");
	    return NOTOK;
	}

    /* deal with ot_ltext */
    if (oid -> oid_nelem == 1 
	    || (oid -> oid_nelem == 2 
		    && oid -> oid_elements[0] == 0
		    && oid -> oid_elements[1] == 0)
	    || (ot2 = name2dpo2 (oid)) == NULLOT
	    || (ot2 && ot2 -> ot_ltext == NULL)) {

	/* duplicate ot_text into ot_ltext */
	if (compile_flag > 0 && compile_heap5) {
	    (void) strcpy (ot -> ot_ltext = ch5, ot -> ot_text);
	    ch5 += strlen (ch5) + 1;
	}
	else if ((ot -> ot_ltext = strdup (ot -> ot_text)) == NULL) {
no_mem:;
		sprintf (PY_pepy, MSGSTR(MS_GENERAL,GENERA_1,
			 "%s: out of memory"), "cresolveot");
		return NOTOK;
	    }
    }
    else {

	/* build up our fully qualified long name */
	/* XXX: should really put some boundry checks on compile_heap5... */
	if (compile_flag > 0 && compile_heap5) {
	    ot -> ot_ltext = ch5;
	    ch5 += strlen (ot -> ot_text) + strlen (ot2 -> ot_ltext) + 2;
	}
	else
	    ot -> ot_ltext = (char *)malloc (strlen (ot -> ot_text) + 
					strlen (ot2 -> ot_ltext) + 2);
	if (ot -> ot_ltext == NULL)
	    goto no_mem;
	sprintf (ot -> ot_ltext, "%s_%s", ot2 -> ot_ltext, ot -> ot_text);
    }

    return insert_THASH (ot);
}

static int
insert_THASH (ot)
OT ot;
{
    register OT ot2;
    int i;

    /* insert into THASH table at the end of the chain */
    for (ot2 = Tbuckets[i = THASH (ot -> ot_text)]; 
		ot2 && ot2 -> ot_chain;
		ot2 = ot2 -> ot_chain)
	;
    if (ot2)
	ot2 -> ot_chain = ot;
    else
	Tbuckets[i] = ot;

    if (chain) {
	chain -> ot_next = ot;
	ot -> ot_prev = chain;
    }
    else
	anchor = ot;
    chain = ot;

    return OK;
}

/*  */

/* partial matches are made only on leaf nodes... */

OT	name2obj (oid)
OID	oid;
{
    register int    i,
    		    j;
    register unsigned *ip;
    register OID   nm;
    register OT	   ot;

    if (oid == NULLOID
	    || oid -> oid_nelem < 1
	    || (i = (ip = oid -> oid_elements)[0])
		    >= (sizeof roots / sizeof roots[0])
	    || (ot = text2obj (roots[i])) == NULL)
	return NULLOT;

    i = 0;
    while (ot) {
	while ((ot) && 
		((j = (nm = ot -> ot_name) -> oid_nelem) > oid -> oid_nelem))
		ot = ot -> ot_sibling; 

	if (!ot)
	    return NULLOT;
	
	if (bcmp ((char *) ip, (char *) (nm -> oid_elements + i),
		  (j - i) * sizeof *ip))
	    ot = ot -> ot_sibling;
	else
	    if (oid -> oid_nelem == j
		    || ot -> ot_children == NULLOT
		    || ot -> ot_smux)
		break;
	    else {
		ot = ot -> ot_children;
		ip = oid -> oid_elements + j, i = j;
	    }
    }

    return ot;
}

/*
 * Name to Distant Parent Object
 * if not a leaf node, walk the OID backwards until we find a distant
 * parent.
 */
static OT	
name2dpo (oid)
OID	oid;
{
    register OT	   ot = NULLOT;
    OIDentifier oids;

    /* XXX: special case, root node */
    if (oid -> oid_nelem == 1)
	return name2obj (oid);

    for (oids.oid_elements = oid -> oid_elements,
		oids.oid_nelem = oid -> oid_nelem - 1;
	    oids.oid_nelem > 0; oids.oid_nelem--)
	if (ot = name2obj (&oids))
	    break;

    return ot;
}

static OT
name2dpo2 (oid)
OID	oid;
{
    register OT	   ot = NULLOT;

    for (ot = chain; ot; ot = ot -> ot_prev)
	if (ot -> ot_name &&
		!bcmp ((char *)oid -> oid_elements, 
		    (char *)ot -> ot_name -> oid_elements,
		    ot -> ot_name -> oid_nelem * sizeof (*oid -> oid_elements)))
	    break;

    return ot;
}

/*  */

/* no need to export this, since it is called automagically by text2obj */
static OT
ltext2obj (ltext)
char *ltext;
{
    register char *cp;
    register OT ot;
    int l;

/** uncomment if exported...
    if (ltext == NULL || once_only == 0)
	return NULLOT;
**/
    l = strlen (ltext);

    if (cp = rindex (ltext, '_'))
	cp++;
    else 
	cp = ltext;

    for (ot = Tbuckets[THASH (cp)]; ot; ot = ot -> ot_chain) 
	if (ot -> ot_ltext && !rcistrncmp (ot -> ot_ltext, ltext, l))
	    break;
    
    return ot;
}

OT	text2obj (text)
char   *text;
{
    register OT	   ot;

    if (text == NULL || once_only == 0)
	return NULLOT;

    /* if text contains '_', let ltext2obj handle it */
    if (index (text, '_'))
	return ltext2obj (text);

    for (ot = Tbuckets[THASH (text)];
	     ot && STRCMP (ot -> ot_text, text);
	     ot = ot -> ot_chain)
	continue;

    return ot;
}


/* ARGSUSED */

#ifdef ISODE6_0
char   *oid2ode (oid)
OID	oid;
#else
char   *oid2ode_aux (oid, quoted)
OID	oid;
int	quoted;
#endif
{
    register int    i;
    register char  *bp;
    register unsigned int *ip;
    register OID    oid2;
    register OT	    ot;
    static char buffer[BUFSIZ];

    if (oid -> oid_nelem == 2		       /* XXX: 0.0 is a special case */
		&& oid -> oid_elements[0] == 0
		&& oid -> oid_elements[1] == 0)
	return sprintoid (oid);

    if ((ot = name2obj (oid)) == NULLOT)
	if ((ot = name2dpo (oid)) == NULLOT)
	    return sprintoid (oid);

    (void) strcpy (bp = buffer, ot -> ot_text);
    bp += strlen (bp);
    for (ip = oid -> oid_elements + (oid2 = ot -> ot_name) -> oid_nelem,
	 	i = oid -> oid_nelem - oid2 -> oid_nelem;
	     i-- > 0;
	     ip++) {
	(void) sprintf (bp, ".%u", *ip);
	bp += strlen (bp);
    }

    return buffer;
}

/*  */

OI	name2inst (oid)
OID	oid;
{
    static object_instance ois;
    register OI oi = &ois;

    if ((oi -> oi_type = name2obj (oi -> oi_name = oid)) == NULLOT)
	return NULLOI;

    return oi;    
}

/* 
 *	FUNCTION: nextot2inst()
 *
 *	PURPOSE: Starting at ot, find the next valid oi past oid.
 *		 Stop early if we walk into a subtree controlled via SMUX.
 */
OI	nextot2inst (oid, ot)
OID	oid;
OT	ot;
{
    static object_instance ois;
    register OI oi = &ois;

    /* Step through the object tree, starting at the specified object,
       to the first object following the specified identifier.  For objects
       that have been mounted over by a subagent, compare only the 
       common portion of the specified identifier and the object's identifier,
       since the subagent may have plys below the object of which the agent
       is unaware.  (EJP) */

    for (; ot; ot = ot -> ot_next) {
	if (ot -> ot_smux) {
	  int nelem = oid->oid_nelem < ot->ot_name->oid_nelem ?
	              oid->oid_nelem : ot->ot_name->oid_nelem;
	  if (elem_cmp(oid->oid_elements, nelem, ot->ot_name->oid_elements, nelem) > 0)
		continue;
	}
	else
	    if (oid_cmp (oid, ot -> ot_name) > 0)
		continue;

	oi -> oi_name = (oi -> oi_type = ot) -> ot_name;
	return oi;
    }

    return NULLOI;
}

/* 
 *	FUNCTION: next2inst()
 *
 *	PURPOSE: same as nextot2oid, but starts at the root OT.
 */
OI	next2inst (oid)
OID	oid;
{
    return nextot2inst (oid, anchor);
}

/*  */

OI	text2inst (text)
char   *text;
{
    static object_instance ois;
    register OI oi = &ois;
    static OID oid = NULLOID;

    if (oid)
	oid_free (oid), oid = NULLOID;

    if ((oid = text2oid (text)) == NULLOID)
	return NULLOI;
    if ((oi -> oi_type = name2obj (oi -> oi_name = oid)) == NULLOT) 
	if ((oi -> oi_type = name2dpo (oi -> oi_name)) == NULLOT) {
	    if (debug)	/* XXX: should never get here... (w/name2dpo) */
		ADVISE (SLOG_DEBUG, MSGSTR(MS_OBJECTS, OBJECTS_8,
			"got name \"%s\", but not object"), text);
	    return NULLOI;
	}

    return oi;    
}

/*    DUMP */

#ifdef	DUMP
dump_objects_by_text ()
{
    int	    hit;
    register int    i;
    register OT	    ot;

    for (i = 0; i < TBUCKETS; i++) {
	hit = 0;
	for (ot = Tbuckets[i]; ot && ot -> ot_text; ot = ot -> ot_chain) {
	    if (!hit)
		printf ("Bucket %d:\n", i), hit = 1;
	    dump_object (ot, 2);
	}
    }
	
    printf ("///////\n");
}


dump_objects_by_tree ()
{
    register char **ap;
    char  **bp;
    register OT	    ot;

    for (bp = (ap = roots) + (sizeof roots / sizeof roots[0]); ap < bp; ap++) {
	if (ot = text2obj (*ap))
	    dump_object_by_tree (ot, 0);
	else
	    printf ("no object for root \"%s\"\n", *ap);
    }
	
    printf ("///////\n");
}


dump_object_by_tree (ot, i)
register OT	ot;
int	i;
{
    if (ot == NULL)
	return;

    dump_object (ot, i);
    dump_object_by_tree (ot -> ot_children, i + 1);
    dump_object_by_tree (ot -> ot_sibling, i);
}


dump_objects_by_xxx ()
{
    register OT	    ot;

    for (ot = anchor; ot; ot = ot -> ot_next)
	dump_object (ot, 0);
	
    printf ("///////\n");
}


static	dump_object (ot, i)
register OT	ot;
int	i;
{
    printf ("%*.*s%s %s %s %s %d %d 0x%x\n", i, i, "",
	    ot -> ot_text, ot -> ot_id, sprintoid (ot -> ot_name),
	    ot -> ot_syntax ? ot -> ot_syntax -> os_name : "NULL",
	    ot -> ot_access, ot -> ot_status, ot -> ot_smux);
}
#endif	/* DUMP */

/*    MISCELLANY */

char   *strdup (s)
char   *s;
{
    char   *p;

    if (p = (char *)malloc ((unsigned) (strlen (s) + 1)))
	(void) strcpy (p, s);

    return p;
}

/* because I didn't feel like making it globally exported... */
OT
get_anchor()
{
    return (anchor);
}

/* case-insensitive reverse strncmp */
static int
rcistrncmp (a, b, n)
register char *a, *b;
int n;
{
    register char c, d, *aa, *bb;

    aa = a + strlen (a);
    bb = b + strlen (b);
    while (aa != a && bb != b && n > 0) {
	c = isupper (*aa) ? tolower (*aa) : *aa;
	d = isupper (*bb) ? tolower (*bb) : *bb;
	if (c != d)
	    break;
	aa--, bb--, n--;
    }

    return (*aa - *bb ? *aa - *bb : n ? n : 0);
}


/* case-insensitive strncmp */
int cistrncmp (a, b, n)
register char *a, *b;
int n;
{
    register char c, d, *aa, *bb;
    register int i = 0;

    aa = a;
    bb = b;
    while (aa != 0 && bb != 0 && n > i) {
	c = isupper (*aa) ? tolower (*aa) : *aa;
	d = isupper (*bb) ? tolower (*bb) : *bb;
	if (c != d)
	    break;
	aa++, bb++, i++;
    }

    return (*aa - *bb);
}

int   delete_objects (ot)
register OT   ot;
{
	register OID oid = ot -> ot_name;
	register OT		ot2,
				*otp;
 
	for (otp = &anchor; ot2 = *otp; otp = &ot2 -> ot_next)
	if (oid_cmp (ot2 -> ot_name, oid) == 0)
		break;
	if (ot2)
		*otp = ot2 -> ot_next;

	object_remove(ot);

	for (ot2 = anchor; ot2 -> ot_next; ot2 = ot2 -> ot_next);
		chain = ot2;
 
	for (ot2 = anchor; ot2; ot2 = ot2 -> ot_next)
		ot2 -> ot_sibling = ot2 -> ot_children = NULLOT;

	return add_objects_aux ();
}

void object_remove(k)
OT k;
{
	unchain(k);
	if (anchor == k)
		anchor = k -> ot_next;
	if (chain == k)
		chain = k -> ot_prev;
	if (k->ot_name)
		oid_free(k->ot_name);
	if (k->ot_ltext)
		free(k->ot_ltext);
	if (k->ot_text)
		free(k->ot_text);
	if (k->ot_id)
		free(k->ot_id);
	if (k->ot_next)
		k->ot_next->ot_prev = k->ot_prev;
	if (k->ot_prev)
		k->ot_prev->ot_next = k->ot_next;
	free(k);
}


void unchain(op)
OT op;
{
    register char *cp;
    register OT ot;
    int l, ix;

	ix = THASH(op->ot_text);
	if (Tbuckets[ix] == op)
	{
		Tbuckets[ix] = op->ot_chain;
		return;
	}
  	for (ot = Tbuckets[ix]; ot; ot = ot->ot_chain) 
	{
		if (ot->ot_chain == op)
		{
			ot->ot_chain = op->ot_chain;
			return;
		}
	}
}

