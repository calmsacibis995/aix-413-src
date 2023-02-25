/* @(#)11 1.3 src/rspc/usr/lib/methods/cfgbus_isa/isa_info.h, rspccfg, rspc41J, 9523C_all 6/2/95 16:18:14 */
#ifndef _H_ISA_INFO
#define _H_ISA_INFO
/*
 *   COMPONENT_NAME: rspccfg
 *
 *   FUNCTIONS: isa device include file
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* subclass strings */
#define INTG_SUBCLASS   "isa_sio"
#define NINTG_SUBCLASS  "isa"


struct ISA_info {
    char *resid;        /* pointer to residual data */
    int  resid_offset;  /* device offset into device id table */
    struct PdDv *pddv;  /* pointer to PdDv object */
    struct CuDv *cudv;  /* pointer to CuDv object */
    int integrated;     /* >0 if device is an integrated device */
    int config;         /* >0 if device is to be configured */
    struct ISA_info *next;      /* pointer to next node */
};


#define ISA_Reserved      0
#define SystemPeripheral  8     /* basetype */
#define BridgeController  6
#define MemoryController  5

#define PowerManagement 6       /* subtype */
#define ISADEVICE         0x01
#define ISABridge         1
#define PCMCIABridge      5

#define GeneralPowerManagement 0        /* interface */

#endif  /* _H_ISA_INFO  */
