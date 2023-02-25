/* @(#)45	1.4  src/rspc/kernext/pci/ncr810/ncr8xxpci.h, pciscsi, rspc41J, 9521A_all 5/23/95 18:09:42 */
/*
 *   COMPONENT_NAME: PCISCSI
 *
 *   FUNCTIONS: none
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
 */

#ifndef _H_NCR8XXPCI
#define _H_NCR8XXPCI

/*
 *	53C8XX PCI Configuration register definitions
 */

/* Note: PCI configuration data seems to require 4-byte access */
#define	PCI_SIGNATURE_OFFS	0x00
#define	PCI_SIGNATURE_SIZE	0x04

#define	PCI_VID			0x00		/* Vendor ID */
#define	PCI_VID_SIZE		2
#define	P8XX_VID		0x1000		/* NCR's ID */

#define	PCI_DID			0x02		/* Device ID */
#define	PCI_DID_SIZE		2
#define	P810_DID		0x0001		/* 53C810's ID */
#define	P825_DID		0x0003		/* 53C825's ID */

#define	PCI_COMMAND		0x04		/* Command register (rw) */
#define PCI_COMMAND_SIZE	2
#define  P8XX_CSERR		0x100		/* SERR Enable */
#define  P8XX_CPERR		PCI_CMD_PAR	/* Parity error resp enable */
#define  P8XX_CBUSM		PCI_CMD_MAST	/* Enable Bus Mastering */
#define  P8XX_CEMS		PCI_CMD_MEM	/* Enable Memory Space */
#define  P8XX_CEIS		PCI_CMD_IO	/* Enable I/O Space */
#define  P8XX_COMMAND_INIT	(P8XX_CPERR | P8XX_CBUSM | P8XX_CEIS)
#define  P8XX_COMMAND_DISABLE	0x00		/* To reset the Enable bits */

#define PCI_STATUS		0x06		/* Status Register  (rw) */
#define PCI_STATUS_SIZE		2
#define	 P8XX_PARDET		0x8000		/* Detected parity error */
#define  P8XX_SIGSYSER		0x4000		/* Signaled System Error */
#define  P8XX_RMA		0x2000		/* Received Master Abort */
#define  P8XX_RTA		0x1000		/* Received Target Abort */
#define  P8XX_DEVSEL_MASK	0x0600		/* DEVSEL timing mask */
#define  P8XX_DEVSEL_FAST	0x0000		/* DEVSEL timing fast */
#define  P8XX_DEVSEL_MED	0x0200		/* DEVSEL timing medium */
#define  P8XX_DEVSEL_SLOW	0x0400		/* DEVSEL timing slow */
#define  P8XX_DPARRPT		0x0100		/* Data Parity Reported */

#define PCI_REV_ID		0x08		/* Revision ID (ro) */
#define PCI_REV_ID_SIZE		1

#define PCI_CLASS_CD		0x09		/* Class Code (ro) */
#define PCI_CLASS_CD_SIZE	3

#define PCI_LATTMR		0x0D		/* Latency Timer (rw) */
#define PCI_LATTMR_SIZE		1
#define  P8XX_LAT_TMR		0x80

#define PCI_HDRTYP		0x0E		/* Header type (rw) */
#define PCI_HDRTYP_SIZE		1

#define PCI_BASEADDR0		0x10		/* Base address 0 (rw) */
#define PCI_BASEADDR0_SIZE	4		/* for I/O space */

#define PCI_BASEADDR1		0x14		/* Base address 1 (rw) */
#define PCI_BASEADDR1_SIZE	4		/* for memory space */

#define PCI_INT_LINE		0x3C		/* Interrupt line (rw) */
#define PCI_INT_LINE_SIZE	1

#endif	/* _NCR8XXPCI */
