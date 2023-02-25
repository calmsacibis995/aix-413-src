static char sccsid[] = "@(#)39	1.1  src/bos/usr/sbin/adfutil/micro.c, cmdadf, bos411, 9428A410j 3/22/90 08:40:02";
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (micro) getadap(), getmicro_list(), getmicro()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cfgodm.h>
#include "symbol.h"
#include "externs.h"

int
getadap(adpname)
char *adpname;
{
    char search[255];
    struct listinfo pInfo;
    struct PdDv *pDv;
    int cardid;

    if (0 > odm_initialize()) {
	fprintf(stderr, NL_MSG(ODM_INIT, "*** FAIL: odm_initialize()\n"));
	return 0;
    }

    sprintf(search, "uniquetype = adapter/mca/%s", adpname);
    pDv = (struct PdDv *) odm_get_list(PdDv_CLASS, search, &pInfo, 1, 1);
    if (-1 == (int)pDv) {
         fprintf(stderr, NL_MSG(DATA_FAIL,
                 "*** FAIL: database error (search %s)\n"), search);
         Fatal("geninit()");
    }
    if (!pDv || (pInfo.num > 1)) {
        fprintf(stderr, NL_MSG(MULT_OBJS,
                "*** ERROR: search (%s) returned %d objects\n"),
                 search, pInfo.num);
	odm_terminate();
        return 0;
    }

    if (odmdebug) {
        printf("\nPredefine Device : search = (%s)\n", search);
        printf("class(%s) subclass(%s) type(%s) prefix(%s)\n",
            pDv->class, pDv->subclass, pDv->type, pDv->prefix);
        printf("uniquetype = %s\n", pDv->uniquetype);
    }
    
    cardid = strtol(pDv->devid, NULL, 16);

    odm_terminate();

    return (0xff00 & cardid << 8) | (0xff & cardid >> 8);
}

struct fl *
getmicro_list(devid)
int devid;
{
    char search[255];
    struct listinfo pInfo;
    struct PdDv *pDv;
    struct PdAt *pAt;
    struct fl *pfl;

    pfl = NULL;
    if (0 > odm_initialize()) {
	fprintf(stderr, NL_MSG(ODM_INIT, "*** FAIL: odm_initialize()\n"));
	return 0;
    }

    sprintf(search, "devid = 0x%x", devid);
    pDv = (struct PdDv *) odm_get_list(PdDv_CLASS, search, &pInfo, 1, 1);
    if (-1 == (int)pDv) {
         fprintf(stderr, NL_MSG(DATA_FAIL,
                 "*** FAIL: database error (search %s)\n"), search);
         Fatal("geninit()");
    }
    if (!pDv || (pInfo.num > 1)) {
        fprintf(stderr, NL_MSG(MULT_OBJS,
                "*** ERROR: search (%s) returned %d objects\n"),
                 search, pInfo.num);
	odm_terminate();
        return NULL;
    }

    if (odmdebug) {
        printf("\nPredefine Device : search = (%s)\n", search);
        printf("class(%s) subclass(%s) type(%s) prefix(%s)\n",
            pDv->class, pDv->subclass, pDv->type, pDv->prefix);
        printf("uniquetype = %s\n", pDv->uniquetype);
    }

    /* get Predefine Attribute */

    sprintf(search, "uniquetype = %s and attribute like microcode*", pDv->uniquetype);
    pAt = (struct PdAt *) odm_get_list(PdAt_CLASS, search, &pInfo, 1, 1);
    if (odmdebug) {
        printf("search (%s) on PdAt returned %d objects\n", search, pInfo.num);
    }

    for ( ; pInfo.num > 0; pInfo.num--) {
	if (pfl)
	    pfl = attachfl(pAt->deflt, pfl);
	else 
	    pfl = makefl(pAt->deflt);
	pAt++;
    }

    odm_terminate();

    return pfl;
}

getmicro(ifiles, adpfiles)
struct fl *ifiles;
struct fl *adpfiles;
{
    struct fl *tfiles;

    if (ifiles) {
	genMicro(ifiles);
    } else if (adpfiles) {
	genMicro(adpfiles);
    } else {
        /* no files to process */
        tfiles = getmicro_list(proot->cardId);
        if (tfiles)
	    genMicro(tfiles);
	else
	    fprintf(stderr, "no files to process.\n");
    }
}
