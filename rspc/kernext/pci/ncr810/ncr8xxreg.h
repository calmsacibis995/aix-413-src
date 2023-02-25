/* @(#)46	1.2  src/rspc/kernext/pci/ncr810/ncr8xxreg.h, pciscsi, rspc41J, 9513A_all 3/28/95 15:12:00 */

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

#ifndef _H_NCR8XXREG
#define _H_NCR8XXREG

/*
 *	53C8XX register addressing definitions
 */

#define	P8XX_BASE_OFFSET	0x80	/* added to all following register
					   offsets */


/* SCSI Control Zero (rw) */
#define SCNTL0_SIZE	0x01
#define SCNTL0		0x0000
#define	 SCNTL0_ARB1	0x80		/* Arbitration mode bit 1 */
#define	 SCNTL0_ARB0	0x40		/* Arbitration mode bit 0 */
#define  SCNTL0_START	0x20		/* Start sequence */
#define  SCNTL0_WATN	0x10		/* Select with SATN/ on a start
					   sequence */
#define  SCNTL0_EPC	0x08		/* Enable Parity Checking */
#define  SCNTL0_AAP	0x02		/* Assert SATN/ on parity error */
#define  SCNTL0_TRG	0x01		/* Target mode */
/* Init to full arbitration and parity checking */
#define	 SCNTL0_INIT	(SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC)

/* SCSI Control One (rw) */
#define SCNTL1_SIZE	0x01
#define SCNTL1		0x0001
#define	 SCNTL1_RST		0x08		/* Assert SCSI RST/ signal */
#define  SCNTL1_IARB		0x02		/* Immediate arbitration */
#define	 SCNTL1_INIT	0x00

/* SCSI Control Two (rw) */
#define SCNTL2_SIZE	0x01
#define SCNTL2		0x0002
#define  SCNTL2_WSR             0x01            /* Wide Scsi Receive */

/* SCSI Control Three (rw) */
#define SCNTL3_SIZE	0x01
#define SCNTL3		0x0003
#define  SCNTL3_MOVE_MASK	0x78030000	/* SCNTL3 move SCRIPT command */
#define  SCNTL3_INIT_FAST	0x13		/* set for up to 10 Mbyte/sec */
#define  SCNTL3_INIT_SLOW	0x33		/* set for 5 Mbyte/sec */

#define  SCNTL3_EWS     	0x08		/* enable wide scsi */

/* Synchronous transfer clock programming values */
#define  SCNTL3_SCF_MASK	0x70		/* SCF divisor bits mask */
#define  SCNTL3_SCF_SHIFT	4		/* SCF divisor bit shift amt */
#define  SCNTL3_SCF_DIV1	0x10		/* Divide by 1 */
#define  SCNTL3_SCF_DIV15	0x20		/* Divide by 1.5 */
#define  SCNTL3_SCF_DIV2	0x30		/* Divide by 2 */
#define  SCNTL3_SCF_DIV3	0x40		/* Divide by 3 */

/* Asynchronous transfer clock programming values */
#define	 SCNTL3_CCF_MASK	0x07		/* CCF divisor bits maks */
#define  SCNTL3_CCF_SHIFT	0		/* CCF divisor bit shift amt */
#define  SCNTL3_CCF_DIV1	0x01		/* Divide by 1 */
#define  SCNTL3_CCF_DIV15	0x02		/* Divide by 1.5 */
#define  SCNTL3_CCF_DIV2	0x03		/* Divide by 2 */
#define  SCNTL3_CCF_DIV3	0x04		/* Divide by 3 */

/* SCSI Chip ID (rw) */
#define SCID_SIZE	0x01
#define SCID		0x0004
#define	 SCID_RRE	0x40			/* Enable reselection */
#define	 SCID_INIT	SCID_RRE

/* SCSI Transfer (rw) */
#define SXFER_SIZE	0x01
#define SXFER		0x0005
#define  SXFER_XFERP_OFS	4	/* XFERP divisor values offset */
#define  SXFER_XFERP_MIN	4	/* minimum supported divisor */
#define  SXFER_XFERP_MAX	11	/* and the maximum */
#define  SXFER_XFERP_MASK	0xE0	/* sync xfer period divisor mask */
#define  SXFER_XFERP_SHIFT	5	/* sync xfer period bit shift amt */
#define  SXFER_INIT		0x08
#define  SXFER_ASYNC_MOVE	0x78050000
#define  SXFER_DEF_MOVE		0x78050800
#define  SXFER_MOVE_MASK	0x78050000
#define	XFER_UNIT	4	/* Transfer period values are in units
				   of 4ns per SCSI spec */
#define	XFER_SLOW	200	/* periods equal or gt are not FAST */
#define XFER_FAST	100	/* as fast as we want go (10MHz) */
#define XFER_FASTEST	80	/* as fast as we will go (12.5MHz) */


/* SCSI Destination ID (rw) */
#define SDID_SIZE	0x01
#define SDID		0x0006
#define  SDID_MASK	0x07

/* General Purpose (rw) */
#define GPREG_SIZE	0x01
#define GPREG		0x0007
/* NCR 810 definitions */
#define  GPREG_INT_CABLE	0x02	/* internal cable !plugged flag */
#define  GPREG_EXT_CABLE	0x01	/* external cable !plugged flag */
#define  GPREG_BOTH_CABLES	(GPREG_INT_CABLE | GPREG_EXT_CABLE)
/* NCR 825 definitions */
#define  GPREG_TP_OKAY		0x01	/* term power sense bit */
#define  GPREG_GPIO1		0x02
#define  GPREG_NCR_RSVD		0x04    
#define  GPREG_SE_DRIVERS     	0x08    /* single-ended */

/* SCSI First Byte Received (rw) */
#define SFBR_SIZE	0x01
#define SFBR		0x0008

/* SCSI Output Control Latch (rw) */
#define SOCL_SIZE	0x01
#define SOCL		0x0009

/* SCSI Selector ID (ro) */
#define SSID_SIZE	0x01
#define SSID		0x000A
#define  SSID_VAL	0x80	/* SSID valid */
#define  SSID_ENCID2	0x07	/* for only encoded SCSI ID bits */

/* SCSI Bus Control Lines (ro) */
#define SBCL_SIZE	0x01
#define SBCL		0x000B

/* DMA Status (ro) */
#define DSTAT_SIZE	0x01
#define DSTAT		0x000C
#define	 IID		0x01    /* DSTAT Illegal Instruction */
#define	 WTD		0x02    /* DSTAT Watchdog Timer */
#define	 SIR		0x04    /* DSTAT Scrpt Intrpt Instr. Recvd. */
#define	 SSI		0x08    /* DSTAT Script Single Step */
#define	 DABRT		0x10    /* DSTAT Abort occurred	*/
#define	 BF		0x20    /* DSTAT Host Bus Fault occurred */
#define	 HPE		0x40    /* DSTAT Host Parity Error detected */
#define	 DFE		0x80    /* DSTAT DMA FIFO Empty */

/* SCSI Status Zero (ro) */
#define SSTAT0_SIZE	0x01
#define SSTAT0		0x000D
#define  SSTAT0_ILF	0x80	/* SIDL LSB full flag (async) */
#define  SSTAT0_ORF	0x40	/* SODR LSB full flag (sync) */
#define  SSTAT0_OLF	0x20	/* SODL LSB full flag (async & sync) */

/* SCSI Status One (ro) */
#define SSTAT1_SIZE	0x01
#define SSTAT1		0x000E
#define SSTAT1_MSG_IN   0x07

/* SCSI Status Two (ro) */
#define SSTAT2_SIZE	0x01
#define SSTAT2		0x000F
/* These three bits are not used with the 810 */
#define  SSTAT2_ILF	0x80	/* SIDL MSB full flag (async) */
#define  SSTAT2_ORF	0x40	/* SODR MSB full flag (sync) */
#define  SSTAT2_OLF	0x20	/* SODL MSB full flag (async & sync) */

/* Data Structure Address (rw) */
#define DSA_SIZE	0x04
#define DSA		0x0010

/* Interrupt Status (rw) */
#define ISTAT_SIZE	0x01
#define ISTAT		0x0014
#define	 ISTAT_DIP	0x01	/* ISTAT DMA Interrupt Pending */
#define	 ISTAT_SIP	0x02	/* ISTAT SCSI Interrupt Pending */
#define	 ISTAT_CONNECTED 0x08	/* Chip currently on SCSI bus */
#define  ISTAT_SEM	0x10	/* Semaphore */
#define  ISTAT_SIGP	0x20	/* Signal Process */
#define	 ISTAT_RST	0x40	/* Software reset of chip */
#define	 ISTAT_ABRT	0x80	/* ISTAT Abort Operation */
#define  ISTAT_INTR	(ISTAT_DIP | ISTAT_SIP)
#define  CONN_INT_TEST	(ISTAT_CONNECTED | ISTAT_SIP | ISTAT_DIP)

/* Chip Test Zero: A general purpose user-definable register (rw) */
#define CTEST0_SIZE	0x01
#define CTEST0		0x0018

/* Chip Test One (ro) */
#define CTEST1_SIZE	0x01
#define CTEST1		0x0019

/* Chip Test Two (ro) */
#define CTEST2_SIZE	0x01
#define CTEST2		0x001A

/* Chip Test Three (rw) */
#define CTEST3_SIZE	0x01
#define CTEST3		0x001B
#define  CTEST3_FLF	0x04	/* Flush DMA FIFO */

/* Temporary Stack (rw) */
#define TEMP_SIZE	0x04
#define TEMP		0x001C	 /* 32 bit stack register 9c-9f */

/* DMA FIFO (rw) */
#define DFIFO_SIZE	0x01
#define DFIFO		0x0020

/* Chip Test Four (rw) */
#define CTEST4_SIZE	0x01
#define CTEST4		0x0021
#define  CTEST4_MPEE    0x08     /* Master Parity Error Enable */
#define CTEST4_INIT	CTEST4_MPEE

/* Chip Test Five (rw) */
#define CTEST5_SIZE	0x01
#define CTEST5		0x0022

/* Chip Test Six (rw) */
#define CTEST6_SIZE	0x01
#define CTEST6		0x0023

/* DMA Byte Counter (rw) */
#define DBC_SIZE	0x03
#define DBC		0x0024	 /* 24 bit DMA Byte Counter Register a4-a6 */

/* DMA Command (rw) */
#define DCMD_SIZE	0x01
#define DCMD		0x0027
#define  DCMD_TYPE_MASK	0xC0	/* Inst type bits */
#define  DCMD_MEM_MOVE	0xC0	/* Inst type code for memory move */
#define  DCMD_BLK_MOVE	0x00	/* Inst type code for block move */
#define  DCMD_TIA_MOVE	0x10	/* Table Indirect Addressing move */
#define  DCMD_OPCD_MASK 0x08	/* Op Code bit */
#define  DCMD_IMODE_MV	0x08	/* Initiator mode move */
#define  DCMD_TMODE_MV	0x00	/* Target mode move */
#define	 DCMD_SCSI_MASK	0x07	/* SCSI phase mask */
#define  DCMD_DATA_OUT	0x00	/* Data out */
#define  DCMD_DATA_IN	0x01	/* Data in */
#define  DCMD_CMD	0x02	/* Command */
#define  DCMD_STATUS	0x03	/* Status */
#define  DCMD_RSVD_OUT	0x04	/* Reserved out */
#define  DCMD_RSVD_IN	0x05	/* Reserved in */
#define  DCMD_MSG_OUT	0x06	/* Message out */
#define  DCMD_MSG_IN	0x07	/* Message in */

/* DBC & DCMD registers, read together to save a PIO */
#define	DBC_DCMD_SIZE	DBC_SIZE+DCMD_SIZE
#define	DBC_DCMD	DBC

/* DMA Next Data Address (rw) */
#define DNAD_SIZE	0x04
#define DNAD		0x0028	 /* 32 bit Next Address Data register a8-ab */

/* DMA SCRIPTS Pointer (rw) */
#define DSP_SIZE	0x04
#define DSP		0x002C	 /* 32 bit SCRIPTS Pointer register ac-af */

/* DMA SCRIPTS Pointer Save (rw) */
#define DSPS_SIZE	0x04
#define DSPS		0x0030	 /* 32 bit SCRIPTS Pointer Save reg b0-b3 */

/* Scratch Register A (rw) */
#define SCRATCHA_SIZE	0x04
#define SCRATCHA	0x0034		/* 32 bit general purpose scratch A */
#define	SCRATCHA0	0x0034
#define SCRATCHA0_SIZE	0x01
#define	SCRATCHA1	0x0035
#define SCRATCHA1_SIZE	0x01
#define	SCRATCHA2	0x0036
#define SCRATCHA2_SIZE	0x01
#define	SCRATCHA3	0x0037
#define SCRATCHA3_SIZE	0x01

/* DMA Mode (rw) */
#define DMODE_SIZE	0x01
#define DMODE		0x0038
#define	 DMODE_BL1	0x80		/* Burst len 1 */
#define	 DMODE_BL0	0x40		/* Burst len 0 */
#define	 DMODE_ER	0x08		/* Enable Read Line */
#define	 DMODE_BOF	0x02		/* Burst Opcode Fetch */
#define  DMODE_INIT	DMODE_BL1 	/* 8-transfer burst */

/* DMA Interrupt Enable (rw) */
#define DIEN_SIZE	0x01
#define DIEN		0x0039
#define  DIEN_MASK	0x7F		/* don't mask any DMA interrupts */

/* DMA Watchdog Timer (rw) */
#define DWT_SIZE	0x01
#define DWT		0x003A

/* DMA Control (rw) */
#define DCNTL_SIZE	0x01
#define DCNTL		0x003B
#define  DCNTL_COM	0x01		/* NOT 53C700 compatibility */
#define  DCNTL_IRQM	0x08		/* Enable totem pole IRQ driver */
#define  DCNTL_INIT	DCNTL_COM | DCNTL_IRQM

/* Adder Sum Output (ro) */
#define ADDER_SIZE	0x04
#define ADDER		0x003C

/* Used to simultaneously access both interrupt enable registers */
#define SIEN_MASK	0x048F	/* mask non-fatal SCSI interrupts, 2 bytes */
#define SIEN_SIZE	0x02
#define SIEN		0x0040	/* pseudo-register for 2 byte operations */

/* SCSI Interrupt Enable Zero (rw) */
#define SIEN0_SIZE	0x01
#define SIEN0		0x0040
#define  SIEN0_MASK	0x8F	/* mask non-fatal SCSI interrupts */

/* SCSI Interrupt Enable Zero (rw) */
#define SIEN1_SIZE	0x01
#define SIEN1		0x0041
#define  SIEN1_MASK	0x04	/* mask non-fatal SCSI interrupts */

/* Used to simultaneously access both interrupt status registers */
#define SIST_SIZE	0x02
#define SIST		0x0042	/* pseudo-register for 2 byte reads */

/* SCSI Interrupt Status Zero (ro) */
#define SIST0_SIZE	0x01
#define SIST0		0x0042

/* SCSI Interrupt Status One (ro) */
#define SIST1_SIZE	0x01
#define SIST1		0x0043

/* SCSI Longitudinal Parity (rw) */
#define SLPAR_SIZE	0x01
#define SLPAR		0x0044

/* SCSI Wide Residue Data (rw) */
#define SWIDE_SIZE	0x01
#define SWIDE		0x0045

/* Memory Access Control (rw) */
#define MACNTL_SIZE	0x01
#define MACNTL		0x0046

/* General Purpose Pin Control (rw) */
#define GPCNTL_SIZE	0x01
#define GPCNTL		0x0047

/* SCSI timer register zero (rw) */
#define STIME0_SIZE	0x01
#define STIME0		0x0048
#define  STIME0_INIT	0x0C	/* No HTH timing, 204.8 msec sel. time-out */

/* SCSI timer register one (rw) */
#define STIME1_SIZE	0x01
#define STIME1		0x0049

/* Response ID (rw) */
#define RESPID_SIZE	0x01
#define RESPID		0x004A

/* SCSI Test Register Zero (ro) */
#define STEST0_SIZE	0x01
#define STEST0		0x004C

/* SCSI Test Register One (ro) */
#define STEST1_SIZE	0x01
#define STEST1		0x004D

/* SCSI Test Register Two (rw) */
#define STEST2_SIZE	0x01
#define STEST2		0x004E
#define  STEST2_DIF     0x20
#define  STEST2_INIT	STEST2_DIF

/* SCSI Test Register Three (rw) */
#define STEST3_SIZE	0x01
#define STEST3		0x004F
#define	 STEST3_TE	0x80	/* TolerANT Enable */
#define  STEST3_CSF	0x02	/* Clear SCSI FIFO */
#define	 STEST3_DSI	0x10	/* Disable Single Initiator Response */
#define  STEST3_INIT	(STEST3_TE | STEST3_DSI)

/* SCSI Input Data Latch (ro) */
/* Should have been named SIDL, but we had a conflict in proc.h */
#define SSIDL_SIZE	0x01
#define SSIDL_SIZE_W	0x02
#define SSIDL		0x0050

/* SCSI Output Data Latch (rw) */
#define SODL_SIZE	0x01
#define SODL_SIZE_W	0x02
#define SODL		0x0054

/* SCSI Bus Data Lines (ro) */
#define SBDL_SIZE	0x01
#define SBDL_SIZE_W	0x02
#define SBDL		0x0058

/* Scratch Register B (rw) */
#define SCRATCHB_SIZE	0x04
#define SCRATCHB	0x005C	 /* 32 bit general purpose scratch pad B */
#define SCRATCHB0	0x005C
#define SCRATCHB0_SIZE	0x01
#define SCRATCHB1	0x005D	
#define SCRATCHB1_SIZE	0x01
#define SCRATCHB2	0x005E	
#define SCRATCHB2_SIZE	0x01
#define SCRATCHB3	0x005F	
#define SCRATCHB3_SIZE	0x01

#endif	/* _NCR8XXREG */
