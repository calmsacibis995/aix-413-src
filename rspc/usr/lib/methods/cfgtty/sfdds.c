static char sccsid[] = "@(#)80	1.6  src/rspc/usr/lib/methods/cfgtty/sfdds.c, isatty, rspc41J, 9520A_all 4/27/95 15:50:26";
/*
 *   COMPONENT_NAME: CFGTTY
 *
 *   FUNCTIONS: Build_DDS
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>          /* standard I/O */
#include <string.h>         /* string manipulation */
#include <malloc.h>         /* Memory allocation */
#include <errno.h>          /* standard error numbers */
#include <ctype.h>
#include <math.h>
#include <cf.h>             /* error messages */
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/strconf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */
#include <termios.h>
#include <sys/termiox.h>
#include "sf.h"
#include "cfgdebug.h"

/* These defines MUST be incoherent, with ODM database attribute names */
#define TBC_ATT                     "tbc16"
#define RTRIG_ATT                   "rtrig"

/*
 * sfdds - create sandalfoot DDS
 */
Build_DDS(cusDevPtr, ddsPtr, ddsSize, termiosPtr, termioxPtr,
               attrList)
struct CuDv * cusDevPtr;/* Customized object pointer */
uchar ** ddsPtr;        /* ptr to ptr to dds structure */
int *    ddsSize;       /* ptr to size fo dds */
struct   termios * termiosPtr;
struct   termiox * termioxPtr;
struct  attr_list *   attrList;

{
    int    rc,count;           
    int current_att;  /* current found attribute */
    static struct sf_dds dds;


    strncpy(dds.rc_name, cusDevPtr->name, sizeof (dds.rc_name) - 1);

    dds.rc_flags = SFDDS_LINE;	/* line specific portion valid	*/
    dds.rc_tios  = *termiosPtr;
    dds.rc_tiox  = *termioxPtr;

    if ((rc = getatt(attrList, TBC_ATT, &current_att ,
                                 'i', &count)) == 0) {
	    dds.rc_conf.tbc = (uchar)current_att;
	    DEBUG_2("Build_DDS: '%s' attribute found = %d\n"
		    , TBC_ATT, current_att);
    } else {
	    DEBUG_1("Build_DDS: '%s' attribute not found\n", TBC_ATT);
	    return E_NOATTR;
    }

    if ((rc = getatt(attrList, RTRIG_ATT, &current_att,
                                 'i', &count)) == 0) {
	    dds.rc_conf.rtrig = (uchar)current_att;
	    DEBUG_2("Build_DDS: '%s' attribute found = %d\n"
		    , RTRIG_ATT, current_att);
    } else {
	    DEBUG_1("Build_DDS: '%s' attribute not found\n", RTRIG_ATT);
	    return E_NOATTR;
    }

    DEBUG_0("sfdds:\n");
    DEBUG_1("\trc_name    = %s\n", dds.rc_name);
    DEBUG_0("\ttermios and termiox not dumped\n");
    DEBUG_1("\ttbc        = 0x%x\n", dds.rc_conf.tbc);
    DEBUG_1("\trtrig      = 0x%x\n", dds.rc_conf.rtrig);
    DEBUG_1("\tdds size      = 0x%x\n", sizeof dds);

    *ddsPtr = (uchar *)&dds;
    *ddsSize = sizeof dds;

    return 0;	/* success!	*/
}

