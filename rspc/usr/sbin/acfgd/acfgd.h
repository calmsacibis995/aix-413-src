/* @(#)72	1.1  src/rspc/usr/sbin/acfgd/acfgd.h, pcmciaker, rspc41J, 9509A_all 2/20/95 11:17:36  */
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: Auto-config daemon common definitions
 *                   - RDevice, Detector
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#if !defined( ACFGD_H )
#define ACFGD_H

#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#define CFGMTDLEN 256

#define ALL_SLOT (-1)

/* flag for check_Detector() */
#define CALL_DETECT 1

/* removable device */
typedef struct _RDevice{
  int flag;
#define FREED       0 /* this value must be 0 */
#define INSERTED    1
#define CONFIGURED  2
  char devname[DEVIDSIZE];
  struct _RDevice* next;
} RDevice;

/* detector device to detect insertion / removal of removable devices */
typedef struct _Detector{
  struct CuDv cudv;
  RDevice* (*getConfiguredList)();
  RDevice* (*getDetectList)();
  char Configure[CFGMTDLEN];
  struct _Detector* next;
}  Detector;

extern void check_Detector( char* devname, int slot, int flag );
extern Detector* const_Detector( struct CuDv* cudv, struct PdDv* pddv );
extern RDevice* const_RDevice( char* devname, int flag );
extern RDevice* merge_RDevices( RDevice* rd1, RDevice* rd2 );
extern void sync_RDevices( RDevice* rd, int slot );

#endif /* ACFGD_H */
