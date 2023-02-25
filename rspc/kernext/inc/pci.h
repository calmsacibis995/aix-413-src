/* @(#)24    1.1  src/rspc/kernext/inc/pci.h, rspccfg, rspc411, GOLD410  4/22/94  10:59:40 */
#ifndef _H_PCI
#define _H_PCI
/*
 * COMPONENT_NAME: (RSPCCFG) PCI bus support
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define PCI_BASE_SZ	(0x28-0x10)	/* Base Address Register area	*/
#define PCI_NO_VENDOR	0xFFFF		/* Illegal vendor ID		*/

/*
 * PCI configuration registers, PCI Local Bus Specification R2.0 page 150.
 * Elements have been addressed to the byte level for endian independence.
 */
typedef	struct	pci_cfg	{
	uchar	vid_l;			/* vendor ID-low		*/
	uchar	vid_h;			/* vendor ID-high		*/
	uchar	devid_l;		/* device ID-low		*/
	uchar	devid_h;		/* device ID-high		*/
	uchar	cmd_l;			/* command low			*/
#define	PCI_CMD_IO	0x01	/* IO space enable		*/
#define	PCI_CMD_MEM	0x02	/* MEM space enable		*/
#define	PCI_CMD_MAST	0x04	/* Bus Master enable		*/
#define	PCI_CMD_SPC	0x08	/* special cycle monitor enable	*/
#define	PCI_CMD_MWI	0x10	/* Mem Write/invalidate enable	*/
#define	PCI_CMD_VGA	0x20	/* VGA palette snoop enable	*/
#define	PCI_CMD_PAR	0x40	/* Parity error handler enable	*/
#define	PCI_CMD_STEP	0x80	/* addr/data stepping enable	*/

	uchar	cmd_h;			/* command high			*/
#define	PCI_CMD_SERR	0x01	/* SERR Driver enable		*/
#define	PCI_CMD_FAST	0x02	/* fast back-to-back  enable	*/

	uchar	dev_stat_l;		/* device status-low		*/
#define	PCI_DEV_FAST	0x80	/* fast back-to-back  supported	*/

	uchar	dev_stat_h;		/* device status-high		*/
#define	PCI_DEV_DPD	0x01	/* Data Parity Detected		*/
#define	PCI_DEV_TMASK	0x06	/* DEVSEL timing mask		*/
#define	PCI_DEV_TFAST	0x00	/* DEVSEL timing fast		*/
#define	PCI_DEV_TMED	0x02	/* DEVSEL timing medium		*/
#define	PCI_DEV_TSLOW	0x04	/* DEVSEL timing slow		*/
#define	PCI_DEV_STA	0x08	/* signaled Target Abort	*/
#define	PCI_DEV_RTA	0x10	/* received Target Abort	*/
#define	PCI_DEV_RMA	0x20	/* received Master Abort	*/
#define	PCI_DEV_SSE	0x40	/* signaled System Error	*/
#define	PCI_DEV_DPE	0x80	/* Detected Parity Error	*/

	uchar	revid;			/* revision identifier		*/
	uchar	prog_if;		/* pgming interface class code	*/
	uchar	sub_clas;		/* sub class code		*/
#define	PCI_SC_OTHER	0x80	/* sub-class for undefined controllers	*/

	/* Subclasses for Base class 00 (outdated)	*/
#define	PCI_SC_OLD	0x00	/* other devices		*/
#define	PCI_SC_OLDVGA	0x01	/* VGA compatible device	*/

	/* Subclasses for Mass Storage class (01)	*/
#define	PCI_SC_SCSI	0x00	/* SCSI controller sub-class	*/
#define	PCI_SC_IDE	0x01	/* IDE controller sub-class	*/
#define	PCI_SC_FLOP	0x02	/* Floppy controller sub-class	*/
#define	PCI_SC_IPI	0x03	/* IPI controller sub-class	*/

	/* Subclasses for Network controller class (02)	*/
#define	PCI_SC_ETHER	0x00	/* Ethernet controller sub-class*/
#define	PCI_SC_TOKEN	0x01	/* Token controller sub-class	*/
#define	PCI_SC_FDDI	0x02	/* FDDI controller sub-class	*/

	/* Subclasses for Display controller class (03)	*/
#define	PCI_SC_VGA	0x00	/* VGA compatible controller	*/
#define	PCI_SC_XGA	0x01	/* XGA controller sub-class	*/

	/* Subclasses for Multimedia device class (04)	*/
#define	PCI_SC_VIDEO	0x00	/* Video sub-class		*/
#define	PCI_SC_AUDIO	0x01	/* Audio sub-class		*/

	/* Subclasses for Memory controller class (05)	*/
#define	PCI_SC_RAM	0x00	/* RAM sub-class		*/
#define	PCI_SC_FLASH	0x01	/* FLASH sub-class		*/

	/* Subclasses for Bridge class  (06)	*/
#define	PCI_SC_HOST	0x00	/* PCI-Host bridge sub-class	*/
#define	PCI_SC_ISA	0x01	/* PCI-ISA bridge sub-class	*/
#define	PCI_SC_EISA	0x02	/* PCI-EISA bridge sub-class	*/
#define	PCI_SC_MCA	0x03	/* PCI-MCA bridge sub-class	*/
#define	PCI_SC_PCI	0x04	/* PCI-PCI bridge sub-class	*/
#define	PCI_SC_PCMCIA	0x05	/* PCI-PCMCIA bridge sub-class	*/

	uchar	base_clas;		/* base class code		*/
	/* PCI Configuration Base Class Codes	*/
#define	PCI_BC_NOCLASS	0x00	/* Base class 00 (outdated)	*/
#define	PCI_BC_MASSTOR	0x01	/* Mass Storage class		*/
#define	PCI_BC_NETWORK	0x02	/* Network controller class	*/
#define	PCI_BC_DISPLAY	0x03	/* Display controller class	*/
#define	PCI_BC_MULTMED	0x04	/* Multimedia device class	*/
#define	PCI_BC_MEMORY	0x05	/* Memory controller class	*/
#define	PCI_BC_BRIDGE	0x06	/* Bridge class			*/
#define	PCI_BC_MUTANT	0xFF	/* Does not fit defined class	*/

	uchar	cachl_sz;		/* cache line size		*/
	uchar	laten_tm;		/* latency timer		*/
	uchar	head_typ;		/* header type			*/
#define	PCI_H_MULTF	0x80	/* multi function device flag	*/

	uchar	bist;			/* Built in self-test		*/
#define	PCI_B_OK	0x80	/* BIST capable flag		*/
#define	PCI_B_START	0x40	/* BIST start flag		*/
#define	PCI_B_MASK	0x0F	/* BIST completion mask		*/

	uchar	base_addr[PCI_BASE_SZ];	/* Base Address Registers	*/
	uint	res0[2];		/* reserved			*/
	uint	xrom;			/* expansion ROM Base address	*/
	uint	res1[2];		/* reserved			*/
	uchar	i_line;			/* interrupt line		*/
	uchar	i_pin;			/* interrupt pin		*/
#define	PCI_I_NONE	0x00	/* no interrupt pin used	*/
#define	PCI_I_INTA	0x01	/* interrupt pin INTA# used	*/
#define	PCI_I_INTB	0x02	/* interrupt pin INTB# used	*/
#define	PCI_I_INTC	0x03	/* interrupt pin INTC# used	*/
#define	PCI_I_INTD	0x04	/* interrupt pin INTD# used	*/

	uchar	min_gnt;		/* interrupt line		*/
	uchar	max_lat;		/* interrupt line		*/
}PCI_CFG;

#endif /* _H_PCI */
