static char sccsid[] = "@(#)69	1.8  src/tcpip/usr/lib/libsnmp/syntax.c, snmp, tcpip411, 9437B411a 9/13/94 16:43:26";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: generic_encode(), generic_decode(), generic_print(),
 *            integer_encode(), integer_decode(), integer_free(),
 *            integer_parse(), integer_print(), add_integer(),
 *            string_encode(), string_oparse(), string_parse(), 
 *            string_print(), string_display(), add_string(), 
 *            object_encode(), object_parse(), object_print(),
 *            add_object(), null_encode(), null_decode(), null_free(),
 *            null_parse(), null_print(), add_null(), ipaddr_encode(),
 *            ipaddr_parse(), ipaddr_print(), add_ipaddr(), add_netaddr(),
 *            counter_encode(), counter_parse(), add_counter(), 
 *            gauge_encode(), add_gauge(), timeticks_encode(), 
 *            timeticks_print(), add_timeticks(), readsyntax(), add_syntax(),
 *            text2syn(), alloc_SMI_ObjectSyntax(), dump_syntax_table(),
 *            dump_syntax()
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/syntax.c
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

/* syntax.c - SMI syntax handling */

#include <ctype.h>
#include <stdio.h>
#include <netdb.h>
#include <isode/pepsy/SMI-types.h>
#include <isode/snmp/objects.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

/*    DATA */

#define	MAXSYN	50

/* GLOBAL */
object_syntax syntaxes[MAXSYN + 1];
OS    synlast = syntaxes;

int Verbose;		/* hack to keep compatibility with 3.1 */

/* EXTERN */
extern long	strtol ();
extern struct qbuf *qb_cpy ();

/* 	GENERIC */

/* NOTE: the new syntax.c is responsible for free'ing only
 *	what free_SMI_ObjectSyntax does not free.
 *	We free all but qbuf's and OID's.
 *
 *	All encoding/decoding is done via generic routines.
 *	Individual encode/decode routines are provided only for
 *	knowledge, and a way to encode/decode to/from a PE, should
 *	one still want to do that.
 *
 *	Motivation behind this extra work:  I wanted to keep the
 *	code generation from the RFC (via pepy/posy) as pure as 
 *	possible, so that when someone takes over my job, they'll 
 *	be able to figure out what I did.
 */

/* no longer an 'encode' routine, rather, we just take a value,
 * and plug it into the appropriate place in the SMI_ObjectSyntax
 * structure.
 */

int
generic_encode (x, o)
caddr_t	*x;					/* incoming value */
struct type_SMI_ObjectSyntax	*o;		/* outgoing */
{

    switch (o -> offset) {
	case type_SMI_ObjectSyntax_number:	/* integer */
	    o -> un.number = (integer)*x;
	    break;

	case type_SMI_ObjectSyntax_string:	/* qbuf * */
	    o -> un.string = qb_cpy ((struct qbuf *)x, 0);
	    break;
	case type_SMI_ObjectSyntax_internet:
	    o -> un.internet = qb_cpy ((struct qbuf *)x, 0);
	    break;
	case type_SMI_ObjectSyntax_arbitrary:
	    o -> un.arbitrary = qb_cpy ((struct qbuf *)x, 0);
	    break;

	case type_SMI_ObjectSyntax_object:	/* OID */
	    o -> un.object = oid_cpy ((OID)x);
	    break;

	case type_SMI_ObjectSyntax_empty:	/* char */
	    o -> un.empty = '\0';
	    break;

	case type_SMI_ObjectSyntax_counter:	/* just an integer... */
	{
	    struct type_SMI_Counter *c;

	    if ((c = (struct type_SMI_Counter *) calloc (1, sizeof *c))
		    == NULL) 
		return NOTOK;
	    c -> parm = (integer)*x;
	    o -> un.counter = c;
	    break;
	}
	case type_SMI_ObjectSyntax_gauge:
	{
	    struct type_SMI_Gauge *g;

	    if ((g = (struct type_SMI_Gauge *) calloc (1, sizeof *g))
		    == NULL) 
		return NOTOK;
	    g -> parm = (integer)*x;
	    o -> un.gauge = g;
	    break;
	}
	case type_SMI_ObjectSyntax_ticks:
	{
	    struct type_SMI_TimeTicks *t;

	    if ((t = (struct type_SMI_TimeTicks *) calloc (1, sizeof *t))
		    == NULL) 
		return NOTOK;
	    t -> parm = (integer)*x;
	    o -> un.ticks = t;
	    break;
	}
	default:			/* unknown */
	    return NOTOK;
    }
    return OK;
}

/* no longer a 'decode' routine, rather, we just take an SMI_ObjectSyntax
 * structure,  and place the approriate answer into value.
 */

int
generic_decode (x, o)
caddr_t	**x;					/* outgoing value */
struct type_SMI_ObjectSyntax	*o;		/* incoming */
{

    switch (o -> offset) {
	case type_SMI_ObjectSyntax_number:	/* integer */
	    **x = (caddr_t) o -> un.number;
	    break;

	case type_SMI_ObjectSyntax_string:	/* qbuf * */
	    *x = (caddr_t *) qb_cpy ((o -> un.string), 1);
	    break;
	case type_SMI_ObjectSyntax_internet:
	    *x = (caddr_t *) qb_cpy ((o -> un.internet), 1);
	    break;
	case type_SMI_ObjectSyntax_arbitrary:
	    *x = (caddr_t *) qb_cpy ((o -> un.arbitrary), 1);
	    break;

	case type_SMI_ObjectSyntax_object:	/* OID */
	    *x = (caddr_t *) oid_cpy ((o -> un.object));
	    break;
	case type_SMI_ObjectSyntax_empty:	/* char */
	    *x = '\0';
	    break;

	case type_SMI_ObjectSyntax_counter:	/* just an integer... */
	    **x = (caddr_t) o -> un.counter -> parm;
	    break;
	case type_SMI_ObjectSyntax_gauge:
	    **x = (caddr_t) o -> un.gauge -> parm;
	    break;
	case type_SMI_ObjectSyntax_ticks:
	    **x = (caddr_t) o -> un.ticks -> parm;
	    break;

	default:			/* unknown */
	    return NOTOK;
    }
    return OK;
}

/* generic print routine.  For those objects that we don't have
 * registered in our database, provide a way to print them.
 */

int
generic_print (o)
struct type_SMI_ObjectSyntax	*o;		/* incoming */
{
    switch (o -> offset) {
	case type_SMI_ObjectSyntax_number:	/* integer */
	    return (integer_print (& (o -> un.number), NULLOS));

	case type_SMI_ObjectSyntax_string:	/* qbuf * */
	    return (string_print (o -> un.string, NULLOS));

	case type_SMI_ObjectSyntax_internet:
	    return (ipaddr_print (o -> un.internet, NULLOS));

	case type_SMI_ObjectSyntax_arbitrary:
	    return (string_print (o -> un.arbitrary, NULLOS));

	case type_SMI_ObjectSyntax_object:	/* OID */
	    return (object_print (o -> un.object, NULLOS));

	case type_SMI_ObjectSyntax_empty:	/* char */
	    return (null_print (NULL, NULLOS));

	case type_SMI_ObjectSyntax_counter:	/* just an integer... */
	    return (integer_print (& (o -> un.counter -> parm), NULLOS));
	case type_SMI_ObjectSyntax_gauge:
	    return (integer_print (& (o -> un.gauge -> parm), NULLOS));

	case type_SMI_ObjectSyntax_ticks:
	    return (timeticks_print (& (o -> un.ticks -> parm), NULLOS));
    }
    return NOTOK;			/* unknown */
}

/*    INTEGER */

static int  
integer_encode (x, obj)
integer	*x;
struct type_SMI_ObjectSyntax     **obj;
{
    if ((*obj = alloc_SMI_ObjectSyntax()) == NULL)
	return NOTOK;
    (*obj) -> offset = type_SMI_ObjectSyntax_number;
    return (generic_encode ((caddr_t *)x, *obj));
}


static int  
integer_decode (x, obj)
integer **x;
struct type_SMI_ObjectSyntax	*obj;
{
    if ((*x = (integer *) malloc (sizeof **x)) == NULL)
	return NOTOK;
    if (generic_decode ((caddr_t **)x, obj) == NOTOK)
	return NOTOK;

    return OK;
}


static int  
integer_free (x)
integer *x;
{
    free ((char *) x);
    return(0);
}


static int  
integer_parse (x, s)
integer **x;
char   *s;
{
    long    l;
    char   *cp;

    if ((l = strtol (s, &cp, 0)) == 0L && cp == s)
	return NOTOK;
    if ((*x = (integer *) malloc (sizeof **x)) == NULL)
	return NOTOK;
    **x = (integer) l;

    return OK;
}


/* ARGSUSED */

static int
uinteger_print (x, os)
integer *x;
OS      os;
{
    printf ("%u", *x);

    return OK;
}


static int  
integer_print (x, os)
integer *x;
OS	os;
{
    printf ("%d", *x);

    return OK;
}

static	
add_integer ()
{
    (void) add_syntax ("INTEGER", 
		integer_encode, integer_decode, integer_free,
		integer_parse, integer_print);
}

/*    OCTET STRING */

static int  
string_encode (x, obj)
struct qbuf	*x;
struct type_SMI_ObjectSyntax     **obj;
{
    if ((*obj = alloc_SMI_ObjectSyntax()) == NULL)
	return NOTOK;
    (*obj) -> offset = type_SMI_ObjectSyntax_string;
    return (generic_encode ((caddr_t *)x, *obj));
}


/*
 * parse an octet string.  Valid format is:
 *	XX:XX:XX:XX:....
 */
static int  
string_oparse (x, s)
struct qbuf **x;
char *s;
{
    char *sp, *t, *tp, buf[3];
    int i, rc;

    if ((tp = t = (char *) malloc (strlen (s))) == NULL)
	return NOTOK;

    /* skip leading white space */
    for (sp = s;*sp && isspace (*sp); sp++)
	;

    /* skip leading : */
    if (*sp == ':')
	sp++;
    if (sp == NULL)
	return NOTOK;

    /* parse out Octet chunks */
    while (*sp) {
	int o;

	for (i = 0;*sp && *sp != ':' && i < 2; sp++, i++) 
	    buf[i] = *sp;
	buf[2] = '\0';
	if ((i = sscanf (buf, "%x", &o)) != 1) 
	    return NOTOK;
	if (*sp == ':')
	    sp++;
	*tp++ = o;
    }
    *tp = '\0';

    /* now we are back in string format again... let string_parse handle it */
    rc = string_parse (x, t);
    free (t);
    return rc;
}

static int  
string_parse (x, s)
struct qbuf **x;
char   *s;
{
    struct qbuf *qb = str2qb (s, strlen (s), 1);

    if (qb == NULL)
	return NOTOK;
    *x = qb;

    return OK;    
}


/* ARGSUSED */

static int  
string_print (x, os)
struct qbuf *x;
OS	os;
{
    register char *cp,
		  *ep;
    char   *p;
    register struct qbuf *qb;

    p = "";
    for (qb = x -> qb_forw; qb != x; qb = qb -> qb_forw)
	for (ep = (cp = qb -> qb_data) + qb -> qb_len; cp < ep; cp++) {
	    printf ("%s%02x", p, *cp & 0xff);
	    p = ":";
	}

    return OK;
}


/* ARGSUSED */

static int  
string_display (x, os)
struct qbuf *x;
OS	os;
{
    register struct qbuf *qb;

    printf ("\"");
    for (qb = x -> qb_forw; qb != x; qb = qb -> qb_forw)
	printf ("%*.*s", qb -> qb_len, qb -> qb_len, qb -> qb_data);
    printf ("\"");

    return OK;
}

static	
add_string ()
{
    (void) add_syntax ("OctetString", 
		string_encode, generic_decode, qb_free,
		string_oparse, string_print);
    (void) add_syntax ("DisplayString", 
		string_encode, generic_decode, qb_free,
		string_parse, string_display);

    /* good enough for now... */
    (void) add_syntax ("PhysAddress", 
		string_encode, generic_decode, qb_free,
		string_oparse, string_print);
}

/*    OBJECT IDENTIFIER */

static int  
object_encode (x, obj)
OID	x;
struct type_SMI_ObjectSyntax     **obj;
{
    if ((*obj = alloc_SMI_ObjectSyntax()) == NULL)
	return NOTOK;
    (*obj) -> offset = type_SMI_ObjectSyntax_object;
    return (generic_encode ((caddr_t *)x, *obj));
}


static int  
object_parse (x, s)
OID   *x;
char   *s;
{
    OID	    oid = text2oid (s);

    if (oid == NULL)
	return NOTOK;
    *x = oid;

    return OK;    
}


/* ARGSUSED */

static int  
object_print (x, os)
OID	x;
OS	os;
{
    char  *cp,
	   ode[BUFSIZ];

    (void) strcpy (ode, Verbose ? oid2ode (x) : sprintoid (x));
    printf ("%s", ode);
    if (strcmp (ode, cp = sprintoid (x)))
	printf (" (%s)", cp);

    return OK;
}

static	
add_object ()
{
    (void) add_syntax ("ObjectID", 
		object_encode, generic_decode, oid_free,
		object_parse, object_print);
}

/*    NULL */

/* ARGSUSED */

static int  
null_encode (x, obj)
char	*x;
struct type_SMI_ObjectSyntax     **obj;
{
    if ((*obj = alloc_SMI_ObjectSyntax()) == NULL)
	return NOTOK;
    (*obj) -> offset = type_SMI_ObjectSyntax_empty;
    return (generic_encode ((caddr_t *)x, *obj));
}


/* ARGSUSED */

static int  
null_decode (x, obj)
char  **x;
struct type_SMI_ObjectSyntax     *obj;
{
    if ((*x = (char *) calloc (1, sizeof **x)) == NULL)
	return NOTOK;

    return OK;
}


static int  
null_free (x)
char *x;
{
    free ((char *) x);
    return(0);
}


static int  
null_parse (x, s)
char  **x;
char   *s;
{
    if (lexequ (s, "NULL"))
	return NOTOK;

    if ((*x = (char *) calloc (1, sizeof **x)) == NULL)
	return NOTOK;

    return OK;    
}


/* ARGSUSED */

static int  
null_print (x, os)
char   *x;
OS	os;
{
    printf ("NULL");

    return OK;
}


static	
add_null ()
{
    (void) add_syntax ("NULL", 
		null_encode, null_decode, null_free, null_parse,
		null_print);
}

/*    IpAddress */

static int  
ipaddr_encode (x, obj)
struct type_SMI_IpAddress *x;		/* just a qbuf */
struct type_SMI_ObjectSyntax     **obj;
{
    if ((*obj = alloc_SMI_ObjectSyntax()) == NULL)
	return NOTOK;
    (*obj) -> offset = type_SMI_ObjectSyntax_internet;
    return (generic_encode ((caddr_t *)x, *obj));
}


static char *addrs[2] = { NULL };

static int  
ipaddr_parse (x, s)
struct type_SMI_IpAddress **x;
char   *s;
{
    u_long iaddr;
    struct hostent hs;
    register struct hostent *hp;
    struct type_SMI_IpAddress *ip;

    iaddr = inet_addr (s);
    if (iaddr == NOTOK) 
	hp = gethostbyname (s);
    else {
	hp = &hs;
	hp -> h_name = s;
	hp -> h_aliases = NULL;
	hp -> h_length = sizeof (iaddr);
	hp -> h_addr_list = addrs;
	bzero ((char *) addrs, sizeof (addrs));
	hp -> h_addr = (char *) &iaddr;
    }

    if (hp == NULL)
	return NOTOK;

    if ((ip = str2qb ((char *) hp -> h_addr, hp -> h_length, 1)) == NULL)
	return NOTOK;
    *x = ip;

    return OK;    
}


/* ARGSUSED */

static int  
ipaddr_print (x, os)
struct type_SMI_IpAddress *x;		/* we know it's a qbuf */
OS	os;
{
    struct qbuf *qb;

    if (qb_pullup (x) == NOTOK || (qb = x -> qb_forw) -> qb_len != 4) 
	return NOTOK;

    printf ("%s", inet_ntoa(*((unsigned long *)qb -> qb_data)) );
    return OK;
}


static	
add_ipaddr ()
{
    (void) add_syntax ("IpAddress", 
		ipaddr_encode, generic_decode, qb_free,
		ipaddr_parse, ipaddr_print);
}

/*    NetworkAddress */

/* good enough for now (and probably forever)... */

static	
add_netaddr ()
{
    (void) add_syntax ("NetworkAddress", 
			ipaddr_encode, generic_decode,
		        qb_free, ipaddr_parse, ipaddr_print);
}

/*    Counter */

static int  
counter_encode (x, obj)
integer	*x;
struct type_SMI_ObjectSyntax     **obj;
{
    if ((*obj = alloc_SMI_ObjectSyntax()) == NULL)
	return NOTOK;
    (*obj) -> offset = type_SMI_ObjectSyntax_counter;
    return (generic_encode ((caddr_t *)x, *obj));
}


static int  counter_parse (x, s)
integer **x;
char   *s;
{
    long    l;
    char   *cp;

    if ((l = strtol (s, &cp, 0)) == 0L && cp == s)
	return NOTOK;
    if (l < 0 || l > 4294967295L)
	return NOTOK;
    if ((*x = (integer *) malloc (sizeof **x)) == NULL)
	return NOTOK;
    **x = (integer) l;

    return OK;    
}


static	
add_counter ()
{
    (void) add_syntax ("Counter", 
		counter_encode, integer_decode, integer_free,
		counter_parse, uinteger_print);
}

/*    Gauge */

static int  
gauge_encode (x, obj)
integer	*x;
struct type_SMI_ObjectSyntax     **obj;
{
    if ((*obj = alloc_SMI_ObjectSyntax()) == NULL)
	return NOTOK;
    (*obj) -> offset = type_SMI_ObjectSyntax_gauge;
    return (generic_encode ((caddr_t *)x, *obj));
}


static	
add_gauge ()
{
    (void) add_syntax ("Gauge", 
		gauge_encode, integer_decode, integer_free,
		counter_parse, uinteger_print);
}

/*    TimeTicks */

static int  
timeticks_encode (x, obj)
integer	*x;
struct type_SMI_ObjectSyntax     **obj;
{
    if ((*obj = alloc_SMI_ObjectSyntax()) == NULL)
	return NOTOK;
    (*obj) -> offset = type_SMI_ObjectSyntax_ticks;
    return (generic_encode ((caddr_t *)x, *obj));
}


/* ARGSUSED */

static int  
timeticks_print (x, os)
integer *x;
OS	os;
{
    unsigned int	    d,
	    h,
	    m,
	    s,
    	    ds;

    if (!Verbose)
	return (uinteger_print (x, os));

    ds = *x;
    s = ds / 100, ds = ds % 100;
    m = s / 60, s = s % 60;
    h = m / 60, m = m % 60;
    d = h / 24, h = h % 24;

    if (d > 0)
	printf (MSGSTR(MS_SYN, SYN_1, "%u days, "), d);
    if (d > 0 || h > 0)
	printf (MSGSTR(MS_SYN, SYN_2, "%u hours, "), h);
    if (d > 0 || h > 0 || m > 0)
	printf (MSGSTR(MS_SYN, SYN_3, "%u minutes, "), m);
    printf ("%u", s);
    if (ds > 0)
	printf (".%02u", ds);
    printf (MSGSTR(MS_SYN, SYN_4, " seconds (%u timeticks)"), *x);

    return OK;
}


static	
add_timeticks ()
{
    (void) add_syntax ("TimeTicks", 
			timeticks_encode, integer_decode,
		        integer_free, integer_parse, timeticks_print);
}

int	
readsyntax ()
{
    add_integer ();
    add_string ();
    add_object ();
    add_null ();
    add_ipaddr ();
    add_netaddr ();
    add_counter ();
    add_gauge ();
    add_timeticks ();
    return(0);
}

/*  */

int	
add_syntax (name, f_encode, f_decode, f_free, f_parse, f_print)
char   *name;
IFP	f_encode,
    	f_decode,
    	f_free,
    	f_parse,
	f_print;
{
    int	    i;
    register OS	    os = synlast++;

    if ((i = synlast - syntaxes) >= MAXSYN)
	return NOTOK;
    os -> os_name = name;
    os -> os_encode = f_encode;
    os -> os_decode = f_decode;
    os -> os_free = f_free;
    os -> os_parse = f_parse;
    os -> os_print = f_print;

    return i;
}

/*  */

OS	
text2syn (name)
char   *name;
{
    register OS	    os;

    for (os = syntaxes; os < synlast; os++)
	if (lexequ (os -> os_name, name) == 0)
	    return os;

    return NULLOS;
}

struct type_SMI_ObjectSyntax *
alloc_SMI_ObjectSyntax ()
{
    register struct type_SMI_ObjectSyntax *obj;

    if ((obj = (struct type_SMI_ObjectSyntax *) calloc (1, sizeof *obj)) 
	    == NULL)
	return NULL;
    obj -> offset = type_SMI_ObjectSyntax_empty;
    return obj;
}

#if 0
/* XXX: removed static on syntaxes and synlast so that this routine
 *	could be implemented elsewhere...
 */
/* shouldn't be here... but syntaxes aren't malloc'ed, so loop must be here. */
dump_syntax_table ()
{
    register OS	    os;
    int	i = 0;

    fprintf (stdout, "/* a mapping to syntax name */\n\n");
    fprintf (stdout, "static char *syntax[] = {\n");
    fprintf (stdout, "#define type_SYNTAX_NULLOS\t%d\n\tNULLCP,\n", i++);
    for (os = syntaxes; os < synlast; os++, i++)
	fprintf (stdout, "#define type_SYNTAX_%s\t%d\n\t\"%s\",\n",
		os -> os_name, i, os -> os_name);
    fprintf (stdout, "};\n\n");
}
#endif

#ifdef DEBUG
dump_syntax()
{
    register OS	    os;

    for (os = syntaxes; os < synlast; os++)
	fprintf (stderr, "os_name = %s\nos_encode = %x\n\
os_decode = %x\nos_free = %x\nos_print = %x\n",
		os -> os_name, os -> os_encode,
		os -> os_decode, os -> os_free, os -> os_print);
}
#endif
