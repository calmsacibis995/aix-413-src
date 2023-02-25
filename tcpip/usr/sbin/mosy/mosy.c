static char sccsid[] = "@(#)38	1.8  src/tcpip/usr/sbin/mosy/mosy.c, isodelib7, tcpip411, 9434A411a 8/18/94 14:27:42";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: 	usage(), yyerror(), warning(), yyerror_aux(), myyerror(),
 *	    	yywrap(), yyprint(), yyprint_aux(), pass1(), pass1_oid(),
 *		pass1_obj(), pass1_type(), pass2(), do_id(), do_obj(),
 *		id2str(), lookup_type(), val2str(), val2int(), val2prf(),
 *		act2prf(), expand(), print_yo(), print_yi(), print_type(),
 *		print_value(), new_symbol(), add_symbol(), free_symbols(),
 *		cleanup(), new_type(), add_type(), new_value(), add_value(), 
 *		new_tag(), new_string(), modsym(), modsym_aux()
 *
 * ORIGINS: 27  60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/mosy/mosy.c
 *
 * Revision 7.4  90/11/20  15:33:41  mrose
 * update
 * 
 * Revision 7.3  90/09/07  17:33:59  mrose
 * new-macros
 * 
 * Revision 7.2  90/08/29  12:24:19  mrose
 * touch-up
 * 
 * Revision 7.1  90/07/01  21:04:36  mrose
 * pepsy
 * 
 * Revision 7.0  89/11/23  22:00:38  mrose
 * Release 6.0
 * 
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


#include <ctype.h>
#include <stdio.h>
#include <string.h> 
#include <varargs.h>
#include <time.h>
#ifdef _POWER
#include <nl_types.h>
#endif /* _POWER */

#define  pepyversion mosyversion
#include "mosy-defs.h"
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

/* EXTERNS */
extern int dumpoid (); 	/* from mosy_dump.c */
extern int optind;
extern char *optarg;

/*    DATA */

int	Cflag = 0;		/* mosy */
int	dflag = 0;
int	Pflag = 0;		/* pepy compat... */
int	oflag = 0;
int	xflag = 0;
int	cflag = 0;		/* compile... */
int	doexternals;
static int linepos = 0;
static int mosydebug = 0;
static int sflag = 0;

int mib_my = 0;

static  char *eval = NULLCP;

char   *mymodule = "";
OID	mymoduleid;

int yysection = 0;
char *yyencpref = "none";
char *yydecpref = "none";
char *yyprfpref = "none";
char *yyencdflt = "none";
char *yydecdflt = "none";
char *yyprfdflt = "none";

static char *yymode = "";

static char *classes[] = {
    "UNIVERSAL ",
    "APPLICATION ",
    "",
    "PRIVATE "
};

char autogen[BUFSIZ];

#define MAXIN 20

char *infile[MAXIN];
char *sysin = NULLCP;
static char *sysout = NULLCP;
static char *descfile = NULLCP;
static char *cfile = NULLCP;
static int tmpflag = 0;

/*  */

typedef struct yot {
    char   *yo_name;

    YP	    yo_syntax;
    YV	    yo_value;

    char   *yo_access;
    char   *yo_status;

    char   *yo_descr;
    char   *yo_refer;
    YV	    yo_index;
    YV	    yo_defval;
}		yot, *OT;
#define	NULLOT	((OT) 0)

typedef struct yoi {
    char   *yi_name;

    YV	    yi_value;
}		yoi, *OI;
#define	NULLOI	((OI) 0)

/*  */

typedef struct symlist {
    char   *sy_encpref;
    char   *sy_decpref;
    char   *sy_prfpref;
    char   *sy_module;
    char   *sy_name;

    union {
	OT	sy_un_yo;

	OI	sy_un_yi;

	YP	sy_un_yp;
    }	sy_un;
#define	sy_yo	sy_un.sy_un_yo
#define	sy_yi	sy_un.sy_un_yi
#define	sy_yp	sy_un.sy_un_yp

    struct symlist *sy_next;
}		symlist, *SY;
#define	NULLSY	((SY) 0)

static	SY	myobjects = NULLSY;

static	SY	myidentifiers = NULLSY;

static	SY	mytypes = NULLSY;


char   *modsym ();
static SY	new_symbol (), add_symbol (), free_symbols ();

char   *id2str ();

YP	lookup_type ();
char   *val2str ();

/*    MAIN */

/* ARGSUSED */

main (argc, argv, envp)
int	argc;
char  **argv,
      **envp;
{
    register char  *cp;
    int  rc,
         input = 0,
         i,
	 pass = 1;
    char *options = "?so:x:c:";
    char tmp[20];
    struct tm timebuf;
    char tmpstring[80];

#ifdef _POWER
    setlocale(LC_TIME, "C");
    strptime(&mosyversion[12], "%a %b %d %T %Z %Y", &timebuf);
    strcpy(tmpstring, mosyversion);

    setlocale (LC_ALL,"");              /* Designates native locale */
    strftime(&tmpstring[12], 80, "%c", &timebuf);
    snmpcatd = catopen (MF_SNMPD,0);
#else
    strcpy(tmpstring, mosyversion);
#endif /* _POWER */

    fprintf (stderr, "%s\n", tmpstring);

    while ((i = getopt (argc, argv, options)) != EOF) {

	 switch (i) {

	    case 'o':
		/*
		 * Create a defsfile in mib defs file format.
		 */
		oflag++;
		if (oflag > 1) 
		    usage ();
		if (*optarg == '-' && (strlen(optarg) > 1))
		    usage();
		sysout = new_string (optarg);

		break;
	    case 's':
		/*
		 * Suppress verification messages.
		 */
		sflag++;
		if (sflag > 1)
		    usage ();
		break;
	    case 'x':
	    case 'c':
		/*
		 * Create a descfile in mib desc file format.
		 * ... or produce importobjects() c-code.
		 */
		i == 'x' ? xflag++ : cflag++;
		if (xflag > 1 || cflag > 1) 
		    usage ();
		if (*optarg == '-' && (strlen(optarg) > 1))
		     usage();
		if (i == 'x') 
		    descfile = *optarg == '-' ? 
			    new_string ("/dev/tty") : new_string (optarg);
		else 
		    cfile = *optarg == '-' ? 
			    new_string ("/dev/tty") : new_string (optarg);

		break;
	    default:
		usage ();
	} /* switch */
    } /* while */
    if ((oflag == 0) && (xflag == 0) && (cflag == 0))
	usage ();

    argv += optind;
    argc = argc - optind;

    if (!argc || argc >= MAXIN)
	usage ();

    for (input = 0; input < argc; input++) {
	infile[input] = new_string (argv[input]);
	if (strcmp (infile[input], "-") == 0) {
	    fprintf (stderr, 
		     MSGSTR(MS_INFO, INFO_1, "invalid inputfile \"-\"\n"));
	    usage ();
	}
    }

    switch (mosydebug = (cp = getenv ("MOSYTEST")) && *cp ? atoi (cp) : 0) {
	case 2:
	    yydebug++;		/* fall */
	case 1:
	    sflag++;		/*   .. */
	case 0:
	    break;
    }

    while ((pass == 1) || (pass == 2)) {
	/*
	 * When the output file is '-' (stdout), a temporary 
	 * file is created containing the mib defs info that 
	 * is used as input for creating the desc file for xgmon.
	 * Mib defs is first written to stdout and then written 
	 * to the temporary file (pass1 and pass2).
	 * If the -x flag is not specified, this temporary
	 * file is not created.
	 */
	if ((strcmp (sysout, "-") == 0) || (pass == 2)) {
	    if (sysout) 
		free (sysout);
	    if (pass == 1) {
		sysout = new_string ("/dev/tty");
		if (xflag > 0 || cflag > 0)
		    pass = 2;
		else
		    pass = 0;
	    }
	    else {
		sysout = (char *) tmpnam(tmp);
		tmpflag = 0;
		sflag++;
		pass = 0;
	    }
	} 
	else  {
	    pass = 0;
	    if (oflag == 0) {
		sysout = (char *) tmpnam(tmp);
		tmpflag = -1;
	    }
	}
		
	if (freopen (sysout, "w", stdout) == NULL) {
	    fprintf (stderr, MSGSTR(MS_INFO, INFO_2, "unable to write ")); 
	    perror (sysout);
	    catclose(snmpcatd);
	    exit (1);
	}

	if (cp = index (mosyversion, ')')) {
	    for (cp++; *cp != ' '; cp++) {
		if (*cp == NULL) {
		    cp = NULL;
		    break;
		}
	    }
	}
	if (cp == NULL)
	    cp = mosyversion + strlen (mosyversion);
	(void) sprintf (autogen, "%*.*s", 
		cp - mosyversion, cp - mosyversion, mosyversion);
	printf (MSGSTR(MS_INFO, INFO_3, 
		"-- automatically generated by %s, do not edit!\n\n"), autogen);

	/*
	 * Create a defs file by converting each input file to the 
	 * mib.defs format.
	 */
	for (i = 0; i < input; i++) {
	    initoidtbl ();
	    sysin = infile[i];
	    if (freopen (sysin, "r", stdin) == NULL) {
		fprintf (stderr, MSGSTR(MS_INFO, INFO_4, "unable to read "));
		perror (sysin);
	    	catclose(snmpcatd);
		exit (1);
	    }
	    if (rc = (yyparse () != 0)) {
		fprintf (stderr, MSGSTR(MS_INFO, INFO_5, 
			 "yyparse failed, rc = %d"), rc);
	    	catclose(snmpcatd);
		exit (rc);
	    }
	    cleanup ();
	}
    } /* while */

    /*
     * Create desc file in mib_desc format from the defs file in 
     * mib.defs format for xgmon usage.
     * ... or create c-code for importobjects ()
     */
    if (xflag || cflag) {
	FILE *xfp = NULL, *cfp = NULL;

	if (cflag && ((cfp = fopen (cfile, "w")) == NULL)) {
	    fprintf (stderr, MSGSTR(MS_INFO, INFO_2, "unable to write "));
	    perror (cfile);
	    catclose(snmpcatd);
	    exit (1);
	}
	if (xflag && ((xfp = fopen (descfile, "w")) == NULL)) {
	    fprintf (stderr, MSGSTR(MS_INFO, INFO_2, "unable to write "));
	    perror (descfile);
	    catclose(snmpcatd);
	    exit (1);
	}
	if (xflag && (mib_my == 0)) {
	    fprintf (stderr, MSGSTR(MS_INFO, INFO_6, 
		"warning: \"%s\" not created: mibII.my must follow smi.my \n"),
		descfile );
	}
	else {

	    if (readobjects (sysout) == NOTOK) {
		fprintf (stderr, "%s\n", PY_pepy);
		if (xflag)
		    fprintf (stderr, MSGSTR(MS_INFO, INFO_7,
			     "warning: \'%s\' not created\n"), descfile);
		if (cflag)
		    fprintf (stderr, MSGSTR(MS_INFO, INFO_7,
			     "warning: \'%s\' not created\n"), cfile);
		if (strcmp (descfile, "/dev/tty") != 0) {
		    if (unlink (descfile) < 0) {
			fprintf (stderr,MSGSTR(MS_INFO, INFO_8,
			         "cannot remove file %s\n"), descfile);
			perror ("unlink");
		    }
		}
		if (strcmp (cfile, "/dev/tty") != 0) {
		    if (unlink (cfile) < 0) {
			fprintf (stderr, MSGSTR(MS_INFO, INFO_8,
				 "cannot remove file %s\n"), cfile);
			perror ("unlink");
		    }
		}
	    }
	    else {
		if (xflag) {
		    dumpoid (xfp);
		    fclose (xfp);
		}
		if (cflag) {
		    object_dump (cfp);
		    fclose (cfp);
		}
	    }
	}
	/*
	 * Remove temporary file created for making the desc file.
	 */
	if (tmpflag) {
	    if (unlink(sysout) < 0) {
		fprintf(stderr,MSGSTR(MS_INFO, INFO_8, 
			"cannot remove file %s\n"), sysout);
		perror("unlink");
	    }
	}
    }
    catclose(snmpcatd);
    exit (0);
}

/*
 * NAME: usage
 *
 * FUNCTION: Display usage
 */
usage()
{
    fprintf (stderr, MSGSTR(MS_INFO, INFO_9,
"usage: mosy -o output_defs_file [-s] inputfile...\n"));

    fprintf (stderr, MSGSTR(MS_INFO, INFO_10,
"       mosy -x output_desc_file [-o output_defs_file] [-s] inputfile...\n"));

    fprintf (stderr, MSGSTR(MS_INFO, INFO_11,
"       mosy -c output_c_file [-x output_desc_file] [-o output_defs_file] [-s] inputfile...\n"));

    catclose(snmpcatd);
    exit (1);
}

/*
 * NAME: cleanup
 *
 * FUNCTION: Frees the identifiers, objects, and types and the export
 * 	     and import tables.
 */
cleanup ()
{

    myidentifiers = free_symbols (myidentifiers);
    myobjects = free_symbols (myobjects);
    mytypes = free_symbols (mytypes);
    free_expimp (TBL_EXPORT);
    free_expimp (TBL_IMPORT);

}

/*
 * NAME: free_symbols
 *
 * FUNCTION: Free symbol table, s1.
 *
 * RETURNS:  NULLSY
 */
static SY
free_symbols (s1)
register SY	s1;

{
    register SY sy;
	     SY tempsy;

    sy = s1;
    while (sy) {
	tempsy = sy;
	sy = sy -> sy_next;
	free (tempsy);
    }

    return NULLSY;
}

/*    ERRORS */

yyerror (s)
register char   *s;
{
    yyerror_aux (s);

    if (*sysout)
	(void) unlink (sysout);

    catclose(snmpcatd);
    exit (1);
}

#ifndef lint
warning (va_alist)
va_dcl
{
    char	buffer[BUFSIZ];
    char	buffer2[BUFSIZ];
    va_list	ap;

    va_start (ap);

    _asprintf (buffer, NULLCP, ap);

    va_end (ap);

    (void) sprintf (buffer2, MSGSTR(MS_INFO, INFO_12, "Warning: %s"), buffer);
    yyerror_aux (buffer2);
}

#else

/* VARARGS1 */
warning (fmt)
char	*fmt;
{
    warning (fmt);
}
#endif

static	
yyerror_aux (s)
register char   *s;
{
    if (linepos)
	fprintf (stderr, "\n"), linepos = 0;

    if (eval)
	fprintf (stderr, "%s %s: ", yymode, eval);
    else
	fprintf (stderr, MSGSTR(MS_INFO, INFO_13, "line %d: "), yylineno);
    fprintf (stderr, "%s\n", s);
    if (!eval)
	fprintf (stderr, MSGSTR(MS_INFO, INFO_14, 
		 "last token read was \"%s\"\n"), yytext);
}

/*  */

#ifndef	lint
myyerror (va_alist)
va_dcl
{
    char    buffer[BUFSIZ];
    va_list ap;

    va_start (ap);

    _asprintf (buffer, NULLCP, ap);

    va_end (ap);

    yyerror (buffer);
}
#else
/* VARARGS */

myyerror (fmt)
char   *fmt;
{
    myyerror (fmt);
}
#endif

/*  */

yywrap () {
    if (linepos)
	fprintf (stderr, "\n"), linepos = 0;

    return 1;
}

/*  */

/* ARGSUSED */

yyprint (s, f, top)
char   *s;
int	f,
	top;
{
}

/*
 * NAME: yyprint_aux
 *
 * FUNCTION: Print verification messages to stderr.
 */
static  
yyprint_aux (s, mode)
char   *s,
       *mode;
{
    int	    len;
    static int nameoutput = 0;
    static int outputlinelen = 79;

    if (sflag)
	return;

    if (strcmp (yymode, mode)) {
	if (linepos)
	    fprintf (stderr, "\n\n");

	fprintf (stderr, "%s", mymodule);
	nameoutput = (linepos = strlen (mymodule)) + 1;

	fprintf (stderr, " %ss", yymode = mode);
	linepos += strlen (yymode) + 1;
	fprintf (stderr, ":");
	linepos += 2;
    }

    len = strlen (s);
    if (linepos != nameoutput)
	if (len + linepos + 1 > outputlinelen)
	    fprintf (stderr, "\n%*s", linepos = nameoutput, "");
	else
	    fprintf (stderr, " "), linepos++;
    fprintf (stderr, "%s", s);
    linepos += len;
}

/*    PASS1 */

/*
 * NAME: pass1
 *
 * FUNCTION: Get and print module id.
 */
pass1 ()
{
    printf (MSGSTR(MS_INFO, INFO_15, 
	    "-- object definitions compiled from %s"), mymodule);
    if (mymoduleid)
	printf (" %s", oidprint(mymoduleid));
    printf ("\n\n");
}

/*  */

/*
 * NAME: pass1_oid
 *
 * FUNCTION: Get object identifiers and add to symbol table.
 */
pass1_oid (mod, id, value)
char   *mod,
       *id;
YV	value;
{
    register SY	    sy;
    register OI	    yi;

    if ((yi = (OI) calloc (1, sizeof *yi)) == NULLOI)
	yyerror (MSGSTR(MS_GENERAL, GENERA_2, "out of memory"));

    yi -> yi_name = id;
    yi -> yi_value = value;

    if (mosydebug) {
	if (linepos)
	    fprintf (stderr, "\n"), linepos = 0;

	fprintf (stderr, "%s.%s\n", mod ? mod : mymodule, id);
	print_yi (yi, 0);
	fprintf (stderr, "--------\n");
    }
    else
	yyprint_aux (id, "identifier");

    sy = new_symbol (NULLCP, NULLCP, NULLCP, mod, id);
    sy -> sy_yi = yi;
    myidentifiers = add_symbol (myidentifiers, sy);
}

/*  */

/*
 * NAME: pass1_obj
 *
 * FUNCTION: Get objects and add to symbol table.
 */
pass1_obj (mod, id, syntax, value, aname, sname, descr, refer, idx, defval)
char   *mod,
       *id,
       *aname,
       *sname,
       *descr,
       *refer;
YP	syntax;
YV	value,
    	idx,
    	defval;
{
    register SY	    sy;
    register OT	    yo;

    {
	register char *cp;

	for (cp = id; *cp; cp++) {
	    if (*cp == '-') {
		warning (MSGSTR(MS_INFO, INFO_16,
			 "object %s contains a `-' in its descriptor"), id);
		break;
	    }
	}

	if (strcmp ((char *)syntax, "Counter") == 0 && 
		    id[strlen (id) - 1] != 's')
	    warning (MSGSTR(MS_INFO, INFO_17, 
		     "descriptor of counter object %s doesn't end in `s'"), id);
    }

    if ((yo = (OT) calloc (1, sizeof *yo)) == NULLOT)
	yyerror (MSGSTR(MS_GENERAL, GENERA_2, "out of memory"));

    yo -> yo_name = id;
    yo -> yo_syntax = syntax;
    yo -> yo_value = value;
    yo -> yo_access = aname;
    yo -> yo_status = sname;
    yo -> yo_descr = descr;
    yo -> yo_refer = refer;
    yo -> yo_index = idx;
    yo -> yo_defval = defval;

    if (mosydebug) {
	if (linepos)
	    fprintf (stderr, "\n"), linepos = 0;

	fprintf (stderr, "%s.%s\n", mod ? mod : mymodule, id);
	print_yo (yo, 0);
	fprintf (stderr, "--------\n");
    }
    else
	yyprint_aux (id, "object");

    sy = new_symbol (NULLCP, NULLCP, NULLCP, mod, id);
    sy -> sy_yo = yo;
    myobjects = add_symbol (myobjects, sy);
}

/*  */

/* ARGSUSED */

pass1_trap (mod, id, enterprise, number, var, descr, refer)
char   *mod,
       *id;
YV	enterprise;
int	number;
YV	var;
char   *descr,
       *refer;
{
}

/*  */

/*
 * NAME: pass1_type
 *
 * FUNCTION: Get object types and add to symbol table.
 */
pass1_type (encpref, decpref, prfpref, mod, id, yp)
register char  *encpref,
	       *decpref,
	       *prfpref,
	       *mod,
	       *id;
register YP	yp;
{
    register SY	    sy;

    /* 
     * No duplicate entries, please... 
     */
    if (dflag && lookup_type (mod, id))	
	return;

    if (mosydebug) {
	if (linepos)
	    fprintf (stderr, "\n"), linepos = 0;

	fprintf (stderr, "%s.%s\n", mod ? mod : mymodule, id);
	print_type (yp, 0);
	fprintf (stderr, "--------\n");
    }
    else
	if (!(yp -> yp_flags & YP_IMPORTED))
	    yyprint_aux (id, "type");

    sy = new_symbol (encpref, decpref, prfpref, mod, id);
    sy -> sy_yp = yp;
    mytypes = add_symbol (mytypes, sy);
}

/*    PASS2 */

/*
 * NAME: pass2
 *
 * FUNCTION: Print identifiers and objects. 
 */
pass2 () {
    register SY	    sy;

    if (!sflag)
	(void) fflush (stderr);

    yymode = "identifier";
    for (sy = myidentifiers; sy; sy = sy -> sy_next) {
	if (sy -> sy_module == NULLCP)
	    yyerror (MSGSTR(MS_INFO, INFO_18,
		     "no module name associated with symbol"));

	do_id (sy -> sy_yi, eval = sy -> sy_name);
    }
    if (myidentifiers)
	printf ("\n");

    yymode = "object";
    for (sy = myobjects; sy; sy = sy -> sy_next) {
	mib_my = -1;
	if (sy -> sy_module == NULLCP)
	    yyerror (MSGSTR(MS_INFO, INFO_18,
		     "no module name associated with symbol"));

	do_obj1 (sy -> sy_yo, eval = sy -> sy_name);
    }
    if (myobjects)
	printf ("\n\n");

    (void) fflush (stdout);

    if (ferror (stdout))
	myyerror (MSGSTR(MS_INFO, INFO_19, "write error - %s"), 
		  strerror (errno));
}

/*  */

/* ARGSUSED */

/*
 * NAME: do_id
 *
 * FUNCTION: Print object identifiers (name and value).
 */
static	
do_id (yi, id)
register OI	yi;
char   *id;
{
    printf ("%-20s %s\n", yi -> yi_name, id2str (yi -> yi_value));
}

/*  */

/* ARGSUSED */

/*
 * NAME: do_obj1
 *
 * FUNCTION: Print objects (name, value, id, access, and value).
 */
static	
do_obj1 (yo, id)
register OT	yo;
char   *id;
{
    register YP	    yp;

    printf ("%-20s %-16s ", yo -> yo_name, id2str (yo -> yo_value));

    if ((yp = yo -> yo_syntax) == NULLYP)
	yyerror (MSGSTR(MS_INFO, INFO_20, 
		 "no syntax associated with object type"));

again: ;
    switch (yp -> yp_code) {
	case YP_INT:
	case YP_INTLIST:
	    id = "INTEGER";
	    break;

       case YP_OCT:
	    id = "OctetString";
	    break;

       case YP_OID:
	    id = "ObjectID";
	    break;

       case YP_NULL:
	    id = "NULL";
	    break;

	default:
	    id = "Aggregate";
	    if (strcmp (yo -> yo_access, "not-accessible"))
		warning ("value of ACCESS clause isn't not-accessible");
	    break;

	case YP_IDEFINED:
	    if (yp = lookup_type (yp -> yp_module, id = yp -> yp_identifier))
		goto again;
	    break;
    }

    printf ("%-15s %-15s %s\n", id, yo -> yo_access, yo -> yo_status);
}

/*    IDENTIFIER HANDLING */

/*
 * NAME: id2str
 *
 * FUNCTION: Convert object id to string.
 */
static char *
id2str (yv)
register YV	yv;
{
    register char *cp,
		  *dp;
    static char buffer[BUFSIZ];

    if (yv -> yv_code != YV_OIDLIST)
	yyerror (MSGSTR(MS_INFO, INFO_21, "need an object identifer"));
	
    cp = buffer;
    for (yv = yv -> yv_idlist, dp = ""; yv; yv = yv -> yv_next, dp = ".") {
	(void) sprintf (cp, "%s%s", dp, val2str (yv));
	cp += strlen (cp);
    }
    *cp = NULL;

    return buffer;
}

/*    TYPE HANDLING */

static YP  
lookup_type (mod, id)
register char *mod,
	      *id;
{
    register SY	    sy;

    for (sy = mytypes; sy; sy = sy -> sy_next) {
	if (mod) {
	    if (strcmp (sy -> sy_module, mod))
		continue;
	}
	else
	    if (strcmp (sy -> sy_module, mymodule)
		    && strcmp (sy -> sy_module, "UNIV"))
		continue;

	if (strcmp (sy -> sy_name, id) == 0)
	    return sy -> sy_yp;
    }

    return NULLYP;
}

/*    VALUE HANDLING */

static char *
val2str (yv)
register YV	yv;
{
    static char buffer[BUFSIZ];

    switch (yv -> yv_code) {
	case YV_BOOL:
	    yyerror (MSGSTR(MS_INFO, INFO_22, 
		     "need a sub-identifier, not a boolean"));

	case YV_NUMBER:
	    (void) sprintf (buffer, "%d", yv -> yv_number);
	    return buffer;

	case YV_STRING:
	    yyerror (MSGSTR(MS_INFO, INFO_23,
		     "need a sub-identifier, not a string"));

	case YV_IDEFINED:
	    return yv -> yv_identifier;

	case YV_IDLIST:
	    yyerror (MSGSTR(MS_INFO, INFO_24,
		     "haven't written symbol table for values yet"));

	case YV_VALIST:
	    yyerror (MSGSTR(MS_INFO, INFO_25, 
		     "need a sub-identifier, not a list of values"));

	case YV_OIDLIST:
	    yyerror (MSGSTR(MS_INFO, INFO_26,
		     "need a sub-identifier, not an object identifier"));

	case YV_NULL:
	    yyerror (MSGSTR(MS_INFO, INFO_27,
		     "need a sub-identifier, not NULL"));

	default:
	    myyerror (MSGSTR(MS_INFO, INFO_28,
		      "unknown value: %d"), yv -> yv_code);
    }
/* NOTREACHED */
}

/*  */

static int  
val2int (yv)
register YV	yv;
{
    switch (yv -> yv_code) {
	case YV_BOOL:
	case YV_NUMBER:
	    return yv -> yv_number;

	case YV_STRING:
	    yyerror (MSGSTR(MS_INFO, INFO_29, "need an integer, not a string"));

	case YV_IDEFINED:
	case YV_IDLIST:
	    yyerror (MSGSTR(MS_INFO, INFO_30, 
		     "haven't written symbol table for values yet"));

	case YV_VALIST:
	    yyerror (MSGSTR(MS_INFO, INFO_31,
		     "need an integer, not a list of values"));

	case YV_OIDLIST:
	    yyerror (MSGSTR(MS_INFO, INFO_32,
		     "need an integer, not an object identifier"));

	case YV_NULL:
	    yyerror (MSGSTR(MS_INFO, INFO_33, "need an integer, not NULL"));

	default:
	    myyerror (MSGSTR(MS_INFO, INFO_28, "unknown value: %d"), 
		      yv -> yv_code);
    }
/* NOTREACHED */
}

/*  */

static	
val2prf (yv, level)
register YV	yv;
int	level;
{
    register YV    y;

    if (yv -> yv_flags & YV_ID)
	printf ("%s ", yv -> yv_id);

#ifdef	notdef
    /* 
     * will this REALLY work??? 
     */
    if (yv -> yv_flags & YV_TYPE)	
	do_type (yv -> yv_type, level, NULLCP);
#endif

    switch (yv -> yv_code) {
	case YV_BOOL: 
	    printf (yv -> yv_number ? MSGSTR(MS_INFO, INFO_34, "TRUE") : 
				      MSGSTR(MS_INFO, INFO_35, "FALSE"));
	    break;

	case YV_NUMBER: 
	    if (yv -> yv_named)
		printf ("%s", yv -> yv_named);
	    else
		printf ("%d", yv -> yv_number);
	    break;

	case YV_STRING: 
	    printf ("\"%s\"", yv -> yv_string);
	    break;

	case YV_IDEFINED: 
	    if (yv -> yv_module)
		printf ("%s.", yv -> yv_module);
	    printf ("%s", yv -> yv_identifier);
	    break;

	case YV_IDLIST: 
	case YV_VALIST: 
	    printf ("{");
	    for (y = yv -> yv_idlist; y; y = y -> yv_next) {
		printf (" ");
		val2prf (y, level + 1);
		printf (y -> yv_next ? ", " : " ");
	    }
	    printf ("}");
	    break;

	case YV_OIDLIST: 
	    printf ("{");
	    for (y = yv -> yv_idlist; y; y = y -> yv_next) {
		printf (" ");
		val2prf (y, level + 1);
		printf (" ");
	    }
	    printf ("}");
	    break;

	case YV_NULL: 
	    printf (MSGSTR(MS_INFO, INFO_36, "NULL"));
	    break;

	default: 
	    myyerror (MSGSTR(MS_INFO, INFO_28, "unknown value: %d"), 
		      yv -> yv_code);
	/* NOTREACHED */
    }
}

/*    ACTION HANDLING */

static	
act2prf (cp, level, e1, e2)
char   *cp,
       *e1,
       *e2;
int	level;
{
    register int    i,
                    j,
                    l4;
    register char  *dp,
                   *ep,
                   *fp;
    char   *gp;

    if (e1)
	printf (e1, level * 4, "");

    if (!(ep = index (dp = cp, '\n'))) {
	printf ("%s", dp);
	goto out;
    }

    for (;;) {
	i = expand (dp, ep, &gp);
	if (gp) {
	    if (i == 0)
		printf ("%*.*s\n", ep - dp, ep - dp, dp);
	    else
		break;
	}

	if (!(ep = index (dp = ep + 1, '\n'))) {
	    printf ("%s", dp);
	    return;
	}
    }


    printf ("\n");
    l4 = (level + 1) * 4;
    for (; *dp; dp = fp) {
	if (ep = index (dp, '\n'))
	    fp = ep + 1;
	else
	    fp = ep = dp + strlen (dp);

	j = expand (dp, ep, &gp);
	if (gp == NULL) {
	    if (*fp)
		printf ("\n");
	    continue;
	}

	if (j < i)
	    j = i;
	if (j)
	    printf ("%*s", l4 + j - i, "");
	printf ("%*.*s\n", ep - gp, ep - gp, gp);
    }

    printf ("%*s", level * 4, "");
out: ;
    if (e2)
	printf (e2, level * 4, "");
}


static	
expand (dp, ep, gp)
register char  *dp,
	       *ep;
char  **gp;
{
    register int    i;

    *gp = NULL;
    for (i = 0; dp < ep; dp++) {
	switch (*dp) {
	    case ' ': 
		i++;
		continue;

	    case '\t': 
		i += 8 - (i % 8);
		continue;

	    default: 
		*gp = dp;
		break;
	}
	break;
    }

    return i;
}

/*    DEBUG */

static	
print_yo (yo, level)
register OT	yo;
register int	level;
{
    if (yo == NULLOT)
	return;

    fprintf (stderr, "%*sname=%s\n", level * 4, "", yo -> yo_name);

    if (yo -> yo_syntax) {
	fprintf (stderr, "%*ssyntax\n", level * 4, "");
	print_type (yo -> yo_syntax, level + 1);
    }
    if (yo -> yo_value) {
	fprintf (stderr, "%*svalue\n", level * 4, "");
	print_value (yo -> yo_value, level + 1);
    }
}

/*  */

static	
print_yi (yi, level)
register OI	yi;
register int	level;
{
    if (yi == NULLOI)
	return;

    fprintf (stderr, "%*sname=%s\n", level * 4, "", yi -> yi_name);

    if (yi -> yi_value) {
	fprintf (stderr, "%*svalue\n", level * 4, "");
	print_value (yi -> yi_value, level + 1);
    }
}

/*  */

static	
print_type (yp, level)
register YP	yp;
register int	level;
{
    register YP	    y;
    register YV	    yv;

    if (yp == NULLYP)
	return;

    fprintf (stderr, "%*scode=0x%x flags=%s\n", level * 4, "",
	    yp -> yp_code, sprintb (yp -> yp_flags, YPBITS));
    fprintf (stderr,
	    "%*sintexp=\"%s\" strexp=\"%s\" prfexp=%c declexp=\"%s\" varexp=\"%s\"\n",
	    level * 4, "", yp -> yp_intexp, yp -> yp_strexp, yp -> yp_prfexp,
	    yp -> yp_declexp, yp -> yp_varexp);
    fprintf(stderr,
	    "%*sstructname=\"%s\" ptrname=\"%s\"\n", level * 4, "",
	    yp -> yp_structname, yp -> yp_ptrname);

    if (yp -> yp_action0)
	fprintf (stderr, "%*saction0 at line %d=\"%s\"\n", level * 4, "",
		yp -> yp_act0_lineno, yp -> yp_action0);
    if (yp -> yp_action1)
	fprintf (stderr, "%*saction1 at line %d=\"%s\"\n", level * 4, "",
		yp -> yp_act1_lineno, yp -> yp_action1);
    if (yp -> yp_action2)
	fprintf (stderr, "%*saction2 at line %d=\"%s\"\n", level * 4, "",
		yp -> yp_act2_lineno, yp -> yp_action2);
    if (yp -> yp_action3)
	fprintf (stderr, "%*saction3 at line %d=\"%s\"\n", level * 4, "",
		yp -> yp_act3_lineno, yp -> yp_action3);

    if (yp -> yp_flags & YP_TAG) {
	fprintf (stderr, "%*stag class=0x%x value=0x%x\n", level * 4, "",
		yp -> yp_tag -> yt_class, yp -> yp_tag -> yt_value);
	print_value (yp -> yp_tag -> yt_value, level + 1);
    }

    if (yp -> yp_flags & YP_DEFAULT) {
	fprintf (stderr, "%*sdefault=0x%x\n", level * 4, "", yp -> yp_default);
	print_value (yp -> yp_default, level + 1);
    }

    if (yp -> yp_flags & YP_ID)
	fprintf (stderr, "%*sid=\"%s\"\n", level * 4, "", yp -> yp_id);

    if (yp -> yp_flags & YP_BOUND)
	fprintf (stderr, "%*sbound=\"%s\"\n", level * 4, "", yp -> yp_bound);

    if (yp -> yp_offset)
	fprintf (stderr, "%*soffset=\"%s\"\n", level * 4, "", yp -> yp_offset);

    switch (yp -> yp_code) {
	case YP_INTLIST:
	case YP_BITLIST:
	    fprintf (stderr, "%*svalue=0x%x\n", level * 4, "", yp -> yp_value);
	    for (yv = yp -> yp_value; yv; yv = yv -> yv_next) {
		print_value (yv, level + 1);
		fprintf (stderr, "%*s----\n", (level + 1) * 4, "");
	    }
	    break;

	case YP_SEQTYPE:
	case YP_SEQLIST:
	case YP_SETTYPE:
	case YP_SETLIST:
	case YP_CHOICE:
	    fprintf (stderr, "%*stype=0x%x\n", level * 4, "", yp -> yp_type);
	    for (y = yp -> yp_type; y; y = y -> yp_next) {
		print_type (y, level + 1);
		fprintf (stderr, "%*s----\n", (level + 1) * 4, "");
	    }
	    break;

	case YP_IDEFINED:
	    fprintf (stderr, "%*smodule=\"%s\" identifier=\"%s\"\n",
		    level * 4, "", yp -> yp_module ? yp -> yp_module : "",
		    yp -> yp_identifier);
	    break;

	default:
	    break;
    }
}

/*  */

static	
print_value (yv, level)
register YV	yv;
register int	level;
{
    register YV	    y;

    if (yv == NULLYV)
	return;

    fprintf (stderr, "%*scode=0x%x flags=%s\n", level * 4, "",
	    yv -> yv_code, sprintb (yv -> yv_flags, YVBITS));

    if (yv -> yv_action)
	fprintf (stderr, "%*saction at line %d=\"%s\"\n", level * 4, "",
		yv -> yv_act_lineno, yv -> yv_action);

    if (yv -> yv_flags & YV_ID)
	fprintf (stderr, "%*sid=\"%s\"\n", level * 4, "", yv -> yv_id);

    if (yv -> yv_flags & YV_NAMED)
	fprintf (stderr, "%*snamed=\"%s\"\n", level * 4, "", yv -> yv_named);

    if (yv -> yv_flags & YV_TYPE) {
	fprintf (stderr, "%*stype=0x%x\n", level * 4, "", yv -> yv_type);
	print_type (yv -> yv_type, level + 1);
    }

    switch (yv -> yv_code) {
	case YV_NUMBER:
	case YV_BOOL:
	    fprintf (stderr, "%*snumber=0x%x\n", level * 4, "",
		    yv -> yv_number);
	    break;

	case YV_STRING:
	    fprintf (stderr, "%*sstring=\"%s\"\n", level * 4, "",
		    yv -> yv_string);
	    break;

	case YV_IDEFINED:
	    if (yv -> yv_flags & YV_BOUND)
		fprintf (stderr, "%*smodule=\"%s\" identifier=\"%s\"\n",
			level * 4, "", yv -> yv_module, yv -> yv_identifier);
	    else
		fprintf (stderr, "%*sbound identifier=\"%s\"\n",
			level * 4, "", yv -> yv_identifier);
	    break;

	case YV_IDLIST:
	case YV_VALIST:
	case YV_OIDLIST:
	    for (y = yv -> yv_idlist; y; y = y -> yv_next) {
		print_value (y, level + 1);
		fprintf (stderr, "%*s----\n", (level + 1) * 4, "");
	    }
	    break;

	default:
	    break;
    }
}

/*    SYMBOLS */

static SY  
new_symbol (encpref, decpref, prfpref, mod, id)
register char  *encpref,
	       *decpref,
	       *prfpref,
	       *mod,
	       *id;
{
    register SY    sy;

    if ((sy = (SY) calloc (1, sizeof *sy)) == NULLSY)
	yyerror ("out of memory");
    sy -> sy_encpref = encpref;
    sy -> sy_decpref = decpref;
    sy -> sy_prfpref = prfpref;
    sy -> sy_module = mod;
    sy -> sy_name = id;

    return sy;
}


static SY  
add_symbol (s1, s2)
register SY	s1,
		s2;
{
    register SY	    sy;

    if (s1 == NULLSY)
	return s2;

    for (sy = s1; sy -> sy_next; sy = sy -> sy_next)
	continue;
    sy -> sy_next = s2;

    return s1;
}

/*    TYPES */

YP	
new_type (code)
int	code;
{
    register YP    yp;

    if ((yp = (YP) calloc (1, sizeof *yp)) == NULLYP)
	yyerror (MSGSTR(MS_GENERAL, GENERA_2, "out of memory"));
    yp -> yp_code = code;

    return yp;
}


YP	
add_type (y1, y2)
register YP	y1,
		y2;
{
    register YP	    yp;

    for (yp = y1; yp -> yp_next; yp = yp -> yp_next)
	continue;
    yp -> yp_next = y2;

    return y1;
}

/*    VALUES */

YV	
new_value (code)
int	code;
{
    register YV    yv;

    if ((yv = (YV) calloc (1, sizeof *yv)) == NULLYV)
	yyerror (MSGSTR(MS_GENERAL, GENERA_2, "out of memory"));
    yv -> yv_code = code;

    return yv;
}


YV	
add_value (y1, y2)
register YV	y1,
		y2;
{
    register YV	    yv;

    for (yv = y1; yv -> yv_next; yv = yv -> yv_next)
	continue;
    yv -> yv_next = y2;

    return y1;
}

/*    TAGS */

YT	
new_tag (class)
PElementClass	class;
{
    register YT    yt;

    if ((yt = (YT) calloc (1, sizeof *yt)) == NULLYT)
	yyerror (MSGSTR(MS_GENERAL, GENERA_2, "out of memory"));
    yt -> yt_class = class;

    return yt;
}

/*    STRINGS */

char   *
new_string (s)
register char  *s;
{
    register char  *p;

/*     fprintf (stderr, "new_string = %s\n", s); */
    if ((p = (char *)malloc ((unsigned) (strlen (s) + 1))) == NULLCP)
	yyerror (MSGSTR(MS_GENERAL, GENERA_2, "out of memory"));

    (void) strcpy (p, s);
    return p;
}

/*    SYMBOLS */

static struct triple {
    char	   *t_name;
    PElementClass   t_class;
    PElementID	    t_id;
}		triples[] = {
    "IA5String", PE_CLASS_UNIV,	PE_DEFN_IA5S,
    "ISO646String", PE_CLASS_UNIV, PE_DEFN_IA5S,
    "NumericString", PE_CLASS_UNIV, PE_DEFN_NUMS,
    "PrintableString", PE_CLASS_UNIV, PE_DEFN_PRTS,
    "T61String", PE_CLASS_UNIV, PE_DEFN_T61S,
    "TeletexString", PE_CLASS_UNIV, PE_DEFN_T61S,
    "VideotexString", PE_CLASS_UNIV, PE_DEFN_VTXS,
    "GeneralizedTime", PE_CLASS_UNIV, PE_DEFN_GENT,
    "GeneralisedTime", PE_CLASS_UNIV, PE_DEFN_GENT,
    "UTCTime", PE_CLASS_UNIV, PE_DEFN_UTCT,
    "UniversalTime", PE_CLASS_UNIV, PE_DEFN_UTCT,
    "GraphicString", PE_CLASS_UNIV, PE_DEFN_GFXS,
    "VisibleString", PE_CLASS_UNIV, PE_DEFN_VISS,
    "GeneralString", PE_CLASS_UNIV, PE_DEFN_GENS,
    "EXTERNAL", PE_CLASS_UNIV, PE_CONS_EXTN,
    "ObjectDescriptor", PE_CLASS_UNIV, PE_PRIM_ODE,

    NULL
};

/*  */

static char *
modsym (module, id, prefix)
register char  *module,
	       *id;
char   *prefix;
{
    char    buf1[BUFSIZ],
            buf2[BUFSIZ],
            buf3[BUFSIZ];
    register struct triple *t;
    static char buffer[BUFSIZ];

    if (module == NULLCP) {
	for (t = triples; t -> t_name; t++) {
	    if (strcmp (t -> t_name, id) == 0) {
		module = "UNIV";
		break;
	    }
	}
    }

    if (prefix)
	modsym_aux (prefix, buf1);
    modsym_aux (module ? module : mymodule, buf2);
    modsym_aux (id, buf3);
    if (prefix)
	(void) sprintf (buffer, "%s_%s_%s", buf1, buf2, buf3);
    else
	(void) sprintf (buffer, "%s_%s", buf2, buf3);

    return buffer;
}


static	
modsym_aux (name, bp)
register char  *name,
	       *bp;
{
    register char   c;

    while (c = *name++) {
	switch (c) {
	    case '-':
		*bp++ = '_';
		*bp++ = '_';
		break;

	    default:
		*bp++ = c;
		break;
	}
    }

    *bp = NULL;
}
