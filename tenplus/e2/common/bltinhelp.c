#if !defined(lint)
static char sccsid[] = "@(#)96	1.6  src/tenplus/e2/common/bltinhelp.c, tenplus, tenplus411, GOLD410 3/24/93 09:26:10";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	noop_hinit, noop_hafter, noop_hbefore, noop_hdel,
 *		noop_hins, noop_hmod, noop_restart, noop_savestate,
 *		noop_alarm, builtin_helper
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
 
#ifdef BLTHELP

#include <def.h>
#include <builtin.h>

/* dummy functions to use when helper doesn't support the indicated entry */
static int noop_hinit (void);
static void noop_hafter (void);
static int noop_hbefore (void);
static void noop_hdel (void);
static void noop_hins (void);
static int noop_hmod (void);
static int noop_restart (void);
static char *noop_savestate (void);
static void noop_alarm (void);

/* !!! The following entries under #if BLT_IN_TEMPLATE are valid for
   building in the corresponding helpers !!! */
# if BLT_IN_TEMPLATE
/* externs for C helper */
extern int C_uinit ();
extern int C_ubefore ();
extern void C_udel ();
extern void C_uins ();
extern char *C_usavestate ();
extern int C_urestart ();

/* externs for the print helper */
extern int PThinit ();

/* externs for the editorprf helper */
extern int N_uinit ();
extern int N_ubefore ();
extern void N_uafter ();
# endif

/* list of built in helpers and their entry points */
struct hlprtab hlprtab [] = {
    {"NO HELPER", noop_hinit, noop_hafter, noop_hbefore, noop_hdel, noop_hins, noop_hmod, noop_restart, noop_savestate, noop_alarm},
# if BLT_IN_TEMPLATE
    {"c", C_uinit, noop_hafter, C_ubefore, C_udel, C_uins, noop_hmod, C_urestart, C_usavestate, noop_alarm},
    {"h", C_uinit, noop_hafter, C_ubefore, C_udel, C_uins, noop_hmod, C_urestart, C_usavestate, noop_alarm},
    {"print", PThinit, noop_hafter, noop_hbefore, noop_hdel, noop_hins, noop_hmod, noop_restart, noop_savestate, noop_alarm},
    {"editorprf", N_uinit, N_uafter, N_ubefore, noop_hdel, noop_hins, noop_hmod, noop_restart, noop_savestate, noop_alarm},
# endif
};

static int numhelpers = (sizeof (hlprtab) / sizeof (struct hlprtab));

static int noop_hinit (void)
{
    return (RET_OK);
}

static void noop_hafter (void)
{
}

static int noop_hbefore (void)
{
    return (0);         /* tell editor to process key normally */
}

static void noop_hdel (void)
{
}

static void noop_hins (void)
{
}

static int noop_hmod (void)
{
    return (0);     /* permit the modification */
}

static int noop_restart (void)
{
    /* for compatibility with old helpers that do not distinguish between
       Hrestart and Hinit, the appropriate Hrestart stub is to determine
       the filename and call Hinit with it. */

    char *filename;
    int   retval;

    filename = Efilename ();
    retval = Hinit (filename);
    s_free (filename);
    return (retval);
}

static char *noop_savestate (void)
{
    return ((char *) NULL);
}

static void noop_alarm (void)
{
}

/* builtin_helper: routine which returns the index of the helper in the
		   hlprtab array if present, otherwise 0                 */


int builtin_helper (char *name)

{
    register int i;

    /* compare the requested helper name against the names of built-in helpers */
    for (i=1; i<numhelpers; i++) {
	if (seq (name, hlprtab [i].name))
	    return (i);
    }
    return (0);
}
#endif
