/* @(#)41	1.3  src/rspc/kernext/pci/ncr810/ncr8xxcfg.h, pciscsi, rspc41J, 9516A_all 4/14/95 15:18:24 */
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

#ifndef _H_CFG8XX
#define _H_CFG8XX

#define NCR8XX_SCSI_CLK		25		/* default Speed of our SCLK
						   in nanoseconds */

/*
 * ddi structure for the NCR 53C8XX
 */

typedef	struct p8xx_ddi {
	struct adap_ddi	addi;
	int		scsi_clk;	/* speed of chip's SCLK in ns */
	int		slow_sync;	/* run 5mb/sec synchronous */
	int		pm_dev_itime;   /* power mgmt idle timer */
	int 		pm_dev_stime;	/* power mgmt standby timer */    
	ushort		intr_flags;	/* flags for interrupt structure */
	uchar		chip_type;	/* encoding of PCI device id */
} p8xx_ddi_t;

/* Vendor & Device IDs for supported chips */
#define P810_SIGNATURE          0x00100100      /* NCR 53C810 */
#define P820_SIGNATURE          0x00100200      /* NCR 53C820 */
#define P825_SIGNATURE          0x00100300      /* NCR 53C825 */

/* values for chip_type */
#define IS_A_ENIGMA     0               /* Unknown, initial state */
#define IS_A_53C810     1               /* For the 810 */
#define IS_A_53C820     2               /* For the 820 */
#define IS_A_53C825     3               /* For the 825 */

#endif	/* _CFG8XX */
