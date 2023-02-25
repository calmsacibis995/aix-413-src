/* @(#)64	1.8  src/rspc/usr/lib/boot/softros/aixmon/model.h, rspc_softros, rspc41J, 9514A_all 3/31/95 16:42:12  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _MODEL
#define _MODEL

extern int model;
extern int processor;
extern ushort	nvram_addr;
extern ushort	nvram_data;
extern uchar	pci_type;
extern uchar	scsi_bus_number;
extern uchar	scsi_dev_func;

#define SF_601_MODEL	 0x08014A4D  /* 601 Sandalfoot Model Code	     */
#define WF_603_MODEL	 0x08014A4F  /* 603 Woodfield Model Code	     */
#define PO_603_MODEL	 0x08014A5D  /* 603 Polo Model Code	     	     */
#define ENTRY_SRV	0x30000  /* Entry server model - supports resid  */
#define DIAG		0x40000  /* supports AIX diagnostics */
#define ANALYZE		0x80000  /* supports AIX diagnostics */
#define OTHER_MODEL	 0x0800004C  /* Other Model - supports residual	data */
		     /* (fill least significant "00" byte with model id) */

#define VER_601		1	     /* 601 version number		     */
#define VER_603		3	     /* 603 version number		     */
#define VER_603PLUS	6	     /* 603+ version number		     */
#define VER_604		4	     /* 604 version number		     */
#define VER_604PLUS	9	     /* 604+ version number		     */

#endif			/* _MODEL */
