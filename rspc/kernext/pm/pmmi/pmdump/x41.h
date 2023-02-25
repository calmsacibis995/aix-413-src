/* @(#)41       1.1  src/rspc/kernext/pm/pmmi/pmdump/x41.h, pwrsysx, rspc41J, 9510A 3/8/95 11:50:33 */
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: Power Management Kernel Extension
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Structure map for X41 partition on PowerPC Reference Platform */
 
#ifndef _H_X41
#define _H_X41
 
typedef enum _X41FLAG {
  OFFlag = 0x80,    /* if set, the loaded program requires Open Firmware */
  LEFlag = 0x40     /* if set, the loaded program is LE */
  } X41FLAG;
 
typedef enum _X41PM_MODE {
  Hibernate = 0x40, /* Nothing in memory - state saved on HibResumeDevice */
  Normal = 0x00     /* No power management in effect */
  } X41PMMode;
 
typedef struct _X41HEAD {
  unsigned int  EntryPointOffset;
  unsigned int  LoadImageLength;
  unsigned char X41Flag;           /* X41FLAG enum above */
  unsigned char PartitionOSid;     /* OS_ID in NVRAM.H */
  unsigned char PartitionName[32]; /* ASCII suitable for Multiboot */
  unsigned char X41PMMode;         /* PM flag; reset to normal by firmware
                                      during resume from hibernation */
  unsigned int  HibResumeSectorListRBA;
  unsigned int  HibResumeSectorListCount;
  unsigned int  HibResumeCodeEntryOffset;
  unsigned char HibResumeDevice[80];  /* Open Firmware Path Specification
                                         Null-terminated string
                                         empty = current drive */
  } X41HEAD;
 
typedef struct _X41_MAP {   /* All fields stored in LE format */
  unsigned char PCBoot[512];       /* DOS compatibility area */
  X41HEAD X41Header;
  unsigned char X41Reserved1[512-sizeof(X41HEAD)]; /* pad to 512 boundary */
  /* unsigned char LoadImage[xxx]; consists of OS specific loader info;
     and the code section containing the entry point */
  /* unsigned char X41Reserved2[xxx]; the rest of the partition */
  } X41_MAP;
 
typedef struct _SECTOR_LIST {
  unsigned int RBA,                /* Relative Block Address */
               sectors;            /* Number of Sectors */
  } SECTOR_LIST;
 
#endif /* _H_X41 */
