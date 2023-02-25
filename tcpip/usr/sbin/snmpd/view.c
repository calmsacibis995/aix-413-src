static char sccsid[] = "@(#)21	1.6  src/tcpip/usr/sbin/snmpd/view.c, snmp, tcpip411, GOLD410 1/21/94 17:39:37";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: o_viewPrim(), get_prent(), o_viewAcl(), get_acent(),
 *	      o_viewTrap(), get_trent(), view_compar(), comm_compar(),
 *	      trap_compar(), init_view(), init_def_view(), fin_view(), 
 *	      str2sa(), str2comm(), start_view(), export_view()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/view.c
 */

/* 
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */


#include <netdb.h>
#include "snmpd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "view.h"

/* FORWARD */
void  init_def_view ();

/* GLOBAL */

/* COMMUNITIES */
struct community commque;
struct community *CHead = &commque;

/* VIEWS */
static OID viewTree = NULLOID;

static struct view viewque;
struct view *VHead = &viewque;


/*    VIEW GROUP */

#define	viewPrimName	  0
#define	viewPrimTDomain   1
#define	viewPrimTAddr	  2
#define	viewPrimUser	  3
#define	viewPrimCommunity 4
#define	viewPrimType	  5

#define	P_VALID		  1		/* viewPrimType */


static OID	localAgent = NULLOID;
static OID	rfc1157Domain = NULLOID;

struct view *get_prent ();


/*
 *	primitive view table
 */
static int  
o_viewPrim (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register int    i;
    register unsigned int *ip,
			  *jp;
    register struct view *vu;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem)
		return int_SNMP_error__status_noSuchName;
	    if ((vu = get_prent (oid -> oid_elements
				     + ot -> ot_name -> oid_nelem,
				 oid -> oid_nelem
				     - ot -> ot_name -> oid_nelem, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		if ((vu = VHead -> v_forw) == VHead)
		    return NOTOK;

		if ((new = oid_extend (oid, vu -> v_insize)) == NULLOID)
		    return NOTOK;
		ip = new -> oid_elements + new -> oid_nelem - vu -> v_insize;
		jp = vu -> v_instance;
		for (i = vu -> v_insize; i > 0; i--)
		    *ip++ = *jp++;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		int	j;

		if ((vu = get_prent (oid -> oid_elements
				         + ot -> ot_name -> oid_nelem,
				     j = oid -> oid_nelem
				     	     - ot -> ot_name -> oid_nelem, 1))
		         == NULL)
		    return NOTOK;

		if ((i = j - vu -> v_insize) < 0) {
		    OID	    new;

		    if ((new = oid_extend (oid, -i)) == NULLOID)
			return NOTOK;
		    if (v -> name)
			free_SMI_ObjectName (v -> name);
		    v -> name = new;

		    oid = new;
		}
		else
		    if (i > 0)
			oid -> oid_nelem -= i;

		ip = oid -> oid_elements + ot -> ot_name -> oid_nelem;
		jp = vu -> v_instance;
		for (i = vu -> v_insize; i > 0; i--)
		    *ip++ = *jp++;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	case viewPrimName:
	    return o_specific (oi, v, (caddr_t) vu -> v_name);

	case viewPrimTDomain:
		return o_specific (oi, v,
				   (caddr_t) (vu -> v_community ? rfc1157Domain
					      		        : localAgent));

	case viewPrimTAddr:
	    if (vu -> v_community) {
		struct sockaddr_in *sin = (struct sockaddr_in *) &vu -> v_sa;

		return o_string (oi, v, (char *) &sin -> sin_addr, 4);
	    }
	    else
		return o_string (oi, v, NULLCP, 0);

	case viewPrimUser:
	case viewPrimCommunity:
	    if (vu -> v_community)
		return o_qbstring (oi, v, vu -> v_community);
	    else
		return o_string (oi, v, NULLCP, 0);

	case viewPrimType:
	    return o_integer (oi, v, P_VALID);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

static struct view *
get_prent (ip, len, isnext)
register unsigned int *ip;
int	len;
int	isnext;
{
    register struct view *v;

    for (v = VHead -> v_forw; v != VHead; v = v -> v_forw)
	switch (elem_cmp (v -> v_instance, v -> v_insize, ip, len)) {
	    case 0:
	        if (!isnext)
		    return v;
		if ((v = v -> v_forw) == VHead)
		    return NULL;
		/* else fall... */

	    case 1:
		return (isnext ? v : NULL);
	}

    return NULL;
}

/*  */

#define	viewAclView	  0
#define	viewAclCommunity  1
#define	viewAclUser	  2
#define	viewAclPrivileges 3
#define	viewAclType	  4

#define	A_VALID		  1		/* viewAclType */


static struct community *CLex = NULL;

struct community *get_acent ();


/*
 *	access policy table
 */
static int  
o_viewAcl (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register int    i;
    register unsigned int *ip,
			  *jp;
    register struct community *c;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem)
		return int_SNMP_error__status_noSuchName;
	    if ((c = get_acent (oid -> oid_elements
				    + ot -> ot_name -> oid_nelem,
				oid -> oid_nelem
				    - ot -> ot_name -> oid_nelem, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		if ((c = CLex) == NULL)
		    return NOTOK;

		if ((new = oid_extend (oid, c -> c_insize)) == NULLOID)
		    return NOTOK;
		ip = new -> oid_elements + new -> oid_nelem - c -> c_insize;
		jp = c -> c_instance;
		for (i = c -> c_insize; i > 0; i--)
		    *ip++ = *jp++;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		int	j;

		if ((c = get_acent (oid -> oid_elements
				        + ot -> ot_name -> oid_nelem,
				    j = oid -> oid_nelem
				     	    - ot -> ot_name -> oid_nelem, 1))
		         == NULL)
		    return NOTOK;

		if ((i = j - c -> c_insize) < 0) {
		    OID	    new;

		    if ((new = oid_extend (oid, -i)) == NULLOID)
			return NOTOK;
		    if (v -> name)
			free_SMI_ObjectName (v -> name);
		    v -> name = new;

		    oid = new;
		}
		else
		    if (i > 0)
			oid -> oid_nelem -= i;

		ip = oid -> oid_elements + ot -> ot_name -> oid_nelem;
		jp = c -> c_instance;
		for (i = c -> c_insize; i > 0; i--)
		    *ip++ = *jp++;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	case viewAclView:
	    return o_specific (oi, v, (caddr_t) c -> c_view -> v_name);

	case viewAclCommunity:
	case viewAclUser:
	    return o_string (oi, v, c -> c_name, strlen (c -> c_name));

	case viewAclPrivileges:
	    return o_integer (oi, v,
			        ((c -> c_permission & OT_RDONLY) ? 3 : 0)
			      + ((c -> c_permission & OT_WRONLY) ? 8 : 0)
			      + ((c -> c_permission & OT_YYY) ? 4 : 0));

	case viewAclType:
	    return o_integer (oi, v, A_VALID);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

static struct community *
get_acent (ip, len, isnext)
register unsigned int *ip;
int	len;
int	isnext;
{
    register struct community *c;

    for (c = CLex; c; c = c -> c_next)
	switch (elem_cmp (c -> c_instance, c -> c_insize, ip, len)) {
	    case 0:
	        return (isnext ? c -> c_next : c);

	    case 1:
		return (isnext ? c : NULL);
	}

    return NULL;
}

/*  */

#define	viewTrapView	  0
#define	viewTrapGenerics  1
#define	viewTrapSpecifics 2
#define	viewTrapType      3

#define	T_VALID		  1		/* viewTrapType */


OID    trapview = NULLOID;

struct trap *get_trent ();


/*
 *	trap configuration
 */
static int  
o_viewTrap (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register int    i;
    register unsigned int *ip,
			  *jp;
    register struct trap *t;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem)
		return int_SNMP_error__status_noSuchName;
	    if ((t = get_trent (oid -> oid_elements
				    + ot -> ot_name -> oid_nelem,
				oid -> oid_nelem
				    - ot -> ot_name -> oid_nelem, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		if ((t = UHead -> t_forw) == UHead)
		    return NOTOK;

		if ((new = oid_extend (oid, t -> t_insize)) == NULLOID)
		    return NOTOK;
		ip = new -> oid_elements + new -> oid_nelem - t -> t_insize;
		jp = t -> t_instance;
		for (i = t -> t_insize; i > 0; i--)
		    *ip++ = *jp++;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		int	j;

		if ((t = get_trent (oid -> oid_elements
				        + ot -> ot_name -> oid_nelem,
				    j = oid -> oid_nelem
				     	    - ot -> ot_name -> oid_nelem, 1))
		         == NULL)
		    return NOTOK;

		if ((i = j - t -> t_insize) < 0) {
		    OID	    new;

		    if ((new = oid_extend (oid, -i)) == NULLOID)
			return NOTOK;
		    if (v -> name)
			free_SMI_ObjectName (v -> name);
		    v -> name = new;

		    oid = new;
		}
		else
		    if (i > 0)
			oid -> oid_nelem -= i;

		ip = oid -> oid_elements + ot -> ot_name -> oid_nelem;
		jp = t -> t_instance;
		for (i = t -> t_insize; i > 0; i--)
		    *ip++ = *jp++;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	case viewTrapView:
	    return o_specific (oi, v, (caddr_t) t -> t_view -> v_name);

	case viewTrapGenerics:
	    {
		char   c = t -> t_generics & 0xff;
		
		return o_string (oi, v, &c, sizeof c);
	    }

	case viewTrapSpecifics:
	    return o_string (oi, v, NULLCP, 0);

	case viewTrapType:
	    return o_integer (oi, v, T_VALID);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

static struct trap *
get_trent (ip, len, isnext)
register unsigned int *ip;
int	len;
int	isnext;
{
    register struct trap *t;

    for (t = UHead -> t_forw; t != UHead; t = t -> t_forw)
	switch (elem_cmp (t -> t_instance, t -> t_insize, ip, len)) {
	    case 0:
	        if (!isnext)
		    return t;
		if ((t = t -> t_forw) == UHead)
		    return NULL;
		/* else fall... */

	    case 1:
		return (isnext ? t : NULL);
	}

    return NULL;
}

/*  */

static int  
view_compar (a, b)
struct view **a,
            **b;
{
    return elem_cmp ((*a) -> v_instance, (*a) -> v_insize,
		     (*b) -> v_instance, (*b) -> v_insize);
}

static int  
comm_compar (a, b)
struct community **a,
                 **b;
{
    return elem_cmp ((*a) -> c_instance, (*a) -> c_insize,
		     (*b) -> c_instance, (*b) -> c_insize);
}

static int  
trap_compar (a, b)
struct trap **a,
            **b;
{
    return elem_cmp ((*a) -> t_instance, (*a) -> t_insize,
		     (*b) -> t_instance, (*b) -> t_insize);
}

/*  */

static struct wired {
    char  *w_args1;
    char  *w_args2;
}	wired[] = {
    "defViewWholeRW", NULL,
    "defViewWholeRO", NULL,
    "defViewStandardRW", "mgmt",
    "defViewStandardRO", "mgmt",

    NULL
};

/*  */

init_view () 
{
    char	buffer[BUFSIZ];
    register OT	    ot;

    CHead -> c_forw = CHead -> c_back = CHead;
    UHead -> t_forw = UHead -> t_back = UHead;
    VHead -> v_forw = VHead -> v_back = VHead;

    init_def_view ();

    (void) strcpy (buffer, "defViewTrapDest.0");
    if ((trapview = text2oid (buffer)) == NULLOID)
	adios (MSGSTR(MS_VIEW, VIEW_3, "unknown OID \"%s\""), 
	       "defViewTrapDest.0");
    trapview -> oid_nelem--;

    if ((localAgent = text2oid ("localAgent")) == NULLOID)
	adios (MSGSTR(MS_VIEW, VIEW_3, "unknown OID \"%s\""), "localAgent");

    if ((rfc1157Domain = text2oid ("rfc1157Domain")) == NULLOID)
	adios (MSGSTR(MS_VIEW, VIEW_3, "unknown OID \"%s\""), "rfc1157Domain");

    if (ot = text2obj ("viewPrimName"))
	ot -> ot_getfnx = o_viewPrim,
	ot -> ot_info = (caddr_t) viewPrimName;
    if (ot = text2obj ("viewPrimTDomain"))
	ot -> ot_getfnx = o_viewPrim,
	ot -> ot_info = (caddr_t) viewPrimTDomain;
    if (ot = text2obj ("viewPrimTAddr"))
	ot -> ot_getfnx = o_viewPrim,
	ot -> ot_info = (caddr_t) viewPrimTAddr;
    if (ot = text2obj ("viewPrimUser"))
	ot -> ot_getfnx = o_viewPrim,
	ot -> ot_info = (caddr_t) viewPrimUser;
    if (ot = text2obj ("viewPrimCommunity"))
	ot -> ot_getfnx = o_viewPrim,
	ot -> ot_info = (caddr_t) viewPrimCommunity;
    if (ot = text2obj ("viewPrimType"))
	ot -> ot_getfnx = o_viewPrim,
	ot -> ot_info = (caddr_t) viewPrimType;

    if (ot = text2obj ("viewAclView"))
	ot -> ot_getfnx = o_viewAcl,
	ot -> ot_info = (caddr_t) viewAclView;
    if (ot = text2obj ("viewAclCommunity"))
	ot -> ot_getfnx = o_viewAcl,
	ot -> ot_info = (caddr_t) viewAclCommunity;
    if (ot = text2obj ("viewAclUser"))
	ot -> ot_getfnx = o_viewAcl,
	ot -> ot_info = (caddr_t) viewAclUser;
    if (ot = text2obj ("viewAclPrivileges"))
	ot -> ot_getfnx = o_viewAcl,
	ot -> ot_info = (caddr_t) viewAclPrivileges;
    if (ot = text2obj ("viewAclType"))
	ot -> ot_getfnx = o_viewAcl,
	ot -> ot_info = (caddr_t) viewAclType;

    if (ot = text2obj ("viewTrapView"))
	ot -> ot_getfnx = o_viewTrap,
	ot -> ot_info = (caddr_t) viewTrapView;
    if (ot = text2obj ("viewTrapGenerics"))
	ot -> ot_getfnx = o_viewTrap,
	ot -> ot_info = (caddr_t) viewTrapGenerics;
    if (ot = text2obj ("viewTrapSpecifics"))
	ot -> ot_getfnx = o_viewTrap,
	ot -> ot_info = (caddr_t) viewTrapSpecifics;
    if (ot = text2obj ("viewTrapType"))
	ot -> ot_getfnx = o_viewTrap,
	ot -> ot_info = (caddr_t) viewTrapType;
}

/* ^L */

/* 
 * NAME: init_def_view ()
 *
 * FUNCTION:  Initializes the view linked list to the view defaults.
 */
void 
init_def_view ()
{
    char  *vec[4];
    register struct wired *w;

    vec[0] = "view";
    for (w = wired; w -> w_args1; w++) {
	vec[1] = w -> w_args1;
	if (vec[2] = w -> w_args2)
	    vec[3] = NULL;

       if (f_view (vec) == NOTOK)
        adios(MSGSTR(MS_VIEW,VIEW_4,"init_def_view: Cannot add default views"));
    }
}

/*  */

fin_view ()
{
    register int    i;
    char   *vec[3];
    register struct community *c;
    register struct view *v;
    register struct trap *t;

#ifdef	DEBUG
/* no defaults!! */
    if (CHead -> c_forw == CHead) {
	vec[0] = "community";
	vec[1] = "public";
	vec[2] = NULL;

	(void) f_community (vec);
    }
#endif

    for (c = CHead -> c_forw; c != CHead; c = c -> c_forw) {
	for (v = VHead -> v_forw; v != VHead; v = v -> v_forw)
	    if (oid_cmp (v -> v_name, c -> c_vu) == 0) {
		c -> c_view = v;
		break;
	    }
	if (v == VHead)
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_VIEW, VIEW_1,
		    "no such view as %s for community \"%s\""),
		    sprintoid (c -> c_vu), c -> c_name);
    }

    /* finish up views */

    i = 0;
    for (v = VHead -> v_forw; v != VHead; v = v -> v_forw)
	i++;
    if (i > 0) {
	register struct view **base,
			     **bp,
			     **ep;

	if ((base = (struct view **) malloc ((unsigned) (i * sizeof *base)))
	        == NULL)
	    adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		   "fin_view");
	ep = base;
	for (v = VHead -> v_forw; v != VHead; v = v -> v_forw) {
	    register int    j;
	    register unsigned int *ip,
				  *jp;
	    OID	    oid = v -> v_name;

	    v -> v_insize = 1 + (j = oid -> oid_nelem);
	    if ((v -> v_instance =
		    (unsigned int *) calloc ((unsigned) v -> v_insize,
					     sizeof *v -> v_instance)) == NULL)
	        adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		   "fin_view");
	    v -> v_instance[0] = oid -> oid_nelem;
	    for (ip = v -> v_instance + 1, jp = oid -> oid_elements;
		     j > 0;
		     j--)
		*ip++ = *jp++;

	    remque (*ep++ = v);
	}
	VHead -> v_forw = VHead -> v_back = VHead;

	if (i > 1)
#ifdef _ANSI_C_SOURCE
	    qsort ((char *) base, i, sizeof *base, 
		   (int(*)(const void *, const void *))view_compar);
#else /* !_ANSI_C_SOURCE */
	    qsort ((char *) base, i, sizeof *base, view_compar);
#endif /* _ANSI_C_SOURCE */

	bp = base;
	while (bp < ep)
	    insque (*bp++, VHead -> v_back);

	free ((char *) base);
    }

    /* finish up the community table */

    i = 0;
    for (c = CHead -> c_forw; c != CHead; c = c -> c_forw)
	i++;
    if (i > 0) {
	int	j;
	register struct community **base,
			          **bp,
			          **ep;

	if ((base = (struct community **)
			    malloc ((unsigned) (i * sizeof *base))) == NULL)
            adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		   "fin_view");
	ep = base;
	for (c = CHead -> c_forw; c != CHead; c = c -> c_forw) {
	    register char *cp;
	    register unsigned int *ip;

	    j = 4;		/* tcp/ip addresses only */

	    c -> c_insize = 1 + strlen (c -> c_name) + 1 + j;
	    if ((c -> c_instance =
		    (unsigned int *) calloc ((unsigned) c -> c_insize,
					     sizeof *c -> c_instance)) == NULL)
                adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		   "fin_view");
	    ip = c -> c_instance;

	    *ip++ = strlen (c -> c_name);
	    for (cp = c -> c_name; *cp; cp++)
		*ip++ = *cp & 0xff;

	    *ip++ = j;
	    ipaddr2oid (ip, &c -> c_sin.sin_addr);

	    *ep++ = c;
	}

	if (i > 1)
#ifdef _ANSI_C_SOURCE
	    qsort ((char *) base, i, sizeof *base, 
		   (int(*)(const void *, const void *))comm_compar);
#else /* !_ANSI_C_SOURCE */
	    qsort ((char *) base, i, sizeof *base, comm_compar);
#endif /* _ANSI_C_SOURCE */

	bp = base;
	c = CLex = *bp++;
	while (bp < ep) {
	    c -> c_next = *bp;
	    c = *bp++;
	}
	c -> c_next = NULL;

	free ((char *) base);
    }
    else
	CLex = NULL;

    /* finish up the trap table */
    i = 0;
    for (t = UHead -> t_forw; t != UHead; t = t -> t_forw)
	i++;
    if (i > 0) {
	register struct trap **base,
			     **bp,
			     **ep;

	if ((base = (struct trap **) malloc ((unsigned) (i * sizeof *base)))
	        == NULL)
            adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		   "fin_view");
	ep = base;
	for (t = UHead -> t_forw; t != UHead; t = t -> t_forw) {
	    register int    j;
	    register unsigned int *ip,
				  *jp;
	    OID	    oid = t -> t_view -> v_name;

	    t -> t_insize = 1 + (j = oid -> oid_nelem);
	    if ((t -> t_instance =
		    (unsigned int *) calloc ((unsigned) t -> t_insize,
					     sizeof *t -> t_instance)) == NULL)
                adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		   "fin_view");
	    t -> t_instance[0] = oid -> oid_nelem;
	    for (ip = t -> t_instance + 1, jp = oid -> oid_elements;
		     j > 0;
		     j--)
		*ip++ = *jp++;

	    remque (*ep++ = t);
	}
	UHead -> t_forw = UHead -> t_back = UHead;

	if (i > 1)
#ifdef _ANSI_C_SOURCE 
	    qsort ((char *) base, i, sizeof *base, 
		   (int(*)(const void *, const void *))trap_compar);
#else /* !_ANSI_C_SOURCE */
	    qsort ((char *) base, i, sizeof *base, trap_compar);
#endif /* _ANSI_C_SOURCE */

	bp = base;
	while (bp < ep)
	    insque (*bp++, UHead -> t_back);

	free ((char *) base);
    }
}

/*  */
#if 0
extern	int	tcpservice;
extern	int	udport;
extern	int	traport;


static int  
str2sa (s, na, sock, proxy)
char   *s;
struct NSAPaddr *na;
struct sockaddr *sock;
int	proxy;
{
#ifdef	TCP
    register struct hostent *hp;
#endif
    struct TSAPaddr *ta;

#ifdef	TCP
    if (hp = gethostbystring (s)) {
	struct sockaddr_in    sin;

	na -> na_stack = NA_TCP;
	na -> na_community = ts_comm_tcp_default;
	inaddr_copy (hp, &sin);
	(void) strncpy (na -> na_domain, inet_ntoa (sin.sin_addr),
			sizeof na -> na_domain - 1);
    }
    else
#endif
	if ((ta = str2taddr (s)) && ta -> ta_naddr > 0) {
	    *na = ta -> ta_addrs[0];	/* struct copy */
	}
	else
	    return NOTOK;

    if (sock == NULL)
	return OK;

    switch (na -> na_stack) {
#ifdef	TCP
	case NA_TCP:
	    if (!tcpservice)
		goto you_lose;
	    {
		struct sockaddr_in sin;

		sin.sin_port = na -> na_port ? na -> na_port
					     : proxy ? udport : traport;

		if ((hp = gethostbystring (na -> na_domain)) == NULL)
		    return NOTOK;

		sin.sin_family = hp -> h_addrtype;
		inaddr_copy (hp, &sin);

		*((struct sockaddr_in *) sock) = sin;	/* struct copy */
	    }
	    break;
#endif

	default:
you_lose: ;
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_VIEW, VIEW_2,
		    "address type unsupported"));
	    return NOTOK;
    }

    return OK;
}
#endif

/*    COMMUNITIES */

/*
 *    Determines if the community is known and whether access is
 *    permitted.
 *
 *    RETURNS:  NULL 			 (failure)
 *              (struct community) commt (success)
 */

struct community *
str2comm (name, na)
char   *name;
register struct sockaddr_in *na;
{
    register struct community *c,     /* Community we compare against */
			      *commt; /* Community that matches what we are */
				      /* searching for; NULL if no match.  */

    commt = NULL;

    /*
     * Search through community queue to find a community that matches
     * the community of the requester.  If the name matches, the address
     * and netmask must also match, otherwise access is denied.
     */
    for (c = CHead -> c_forw; c != CHead; c = c -> c_forw) {
	if (strcmp (c -> c_name, name) == 0) { 
	    /*
	     * The netmask is &'d with the address to check if a 
	     * particular network has access.
	     */
	    if (((c -> c_sin.sin_addr.s_addr & 
		    c -> c_netmask.sin_addr.s_addr) == 
		    (na -> sin_addr.s_addr & 
		    c -> c_netmask.sin_addr.s_addr)) == 0) { 
		continue;
	    }

	    commt = c;
	    break;
	}
    }

    /*
     * Move this community to the head of the queue to reduce future search
     * time.
     */
    if (commt) {
	remque (commt);
	insque (commt, CHead);
    }

    return commt;
}

start_view () 
{
    register OT     ot;

    /* disallow automatic access into the view tree */
    if (ot = text2obj ("view"))
	viewTree = ot -> ot_name;

    /* Initialize the view mask of all objects in the tree, not just
       the leaves.  This is necessary so that do_pass() will allow
       SMUX sub-agents to mount their subtrees over interior objects.  (EJP) */

    for (ot = text2obj ("ccitt"); ot; ot = ot -> ot_next)
	export_view (ot);
}

export_view (ot)
register OT     ot;
{
    register struct subtree *s;
    register struct view  *v;
    OID     name = ot -> ot_name;

    ot -> ot_views = 0;
    for (v = VHead -> v_forw; v != VHead; v = v -> v_forw)
	if ((s = v -> v_subtree.s_forw) != &v -> v_subtree) {
	    for (; s != &v -> v_subtree; s = s -> s_forw)
		if (inSubtree (s -> s_subtree, name))
		    goto mark_it;
	    }
	else
	    if (!viewTree || !inSubtree (viewTree, name)) {
mark_it: ;
		ot -> ot_views |= v -> v_mask;
	    }
}

