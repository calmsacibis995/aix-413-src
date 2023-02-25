/* @(#)42	1.3  src/rspc/kernext/pci/ncr810/ncr8xxdef.h, pciscsi, rspc41J, 9516A_all 4/18/95 11:19:57 */
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

#ifndef _H_NCR8XXDEF
#define _H_NCR8XXDEF


/* configuration values - limit the size of the component dump table */
#define	MAX_BUS_NUM	2		/* We support two PCI busses */
#define	MAX_SLOT	32		/* and 32 slots (devices) per bus */
#define	MAX_ADAPTERS	(MAX_BUS_NUM*MAX_SLOT) /* maximum adapters we
					   support */


/************************************************************************/
/* General Device Driver Defines					*/
/************************************************************************/
#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif


/* Default & min/max Synchronous Data Transfer Request parameters */
#define NCR8XX_MIN_PERIOD	25		/* Min sync xfer period
						   (times 4 ns) */
#define NCR8XX_DEF_PERIOD	NCR8XX_MIN_PERIOD
#define	NCR8XX_FAST_PERIOD	NCR8XX_MIN_PERIOD
#define	NCR8XX_SLOW_PERIOD	50

/* Default & max synchronous offset of the chip */
#define NCR8XX_MAX_OFFSET	8		/* Max REQ/ACK offset */
#define	NCR8XX_DEF_OFFSET	NCR8XX_MAX_OFFSET
#define	SCSI_SDTR_MSG		0x01030100	/* Synchronous Data Transfer
						   Request msg */
#define	SCSI_SDTR_OPCODE	0x01		 
#define SCSI_SDTR_MSG_LEN	0x05
#define	SCSI_WDTR_MSG		0x01020301	/* Wide (16 bit) Data Transfer
						   Request msg */
#define	SCSI_WDTR_OPCODE	0x03		 
#define SCSI_WDTR_MSG_LEN	0x04
#define	SCSI_EXT_MSG_OPCODE	0x01000000	/* Extended message opcode */

/* Mode parameter to p8xx_mode_patch() */
#define	ASYNC_MODE	0			/* patch to async */
#define	SYNC_MODE	1			/* patch to sync */


#define PSC_WORDSIZE   32
/*
 * This is the maximum size block we will transfer with each
 * TIM entry.  This must be no larger than PAGESIZE and must be
 * an even multiple of it.
 */
#define MAX_TIMLEN	PAGESIZE

/*
 * Largest data xfer size.  BEWARE: increasing this may necessitate changes
 * to the data move SCRIPTS routines since the Table Indirect addressing
 * scheme used by them only increments the least significant two bytes.
 */
#define MAX_TIM		512		/* Max TIM entries on a page */
#define	SMALL_IOVECS     32		/* Number of vectors exclusively 
					 * reserved for 4K transfers
					 */
#define MAXREQUEST	((MAX_TIM-SMALL_IOVECS-1)*4096)
					/* There are MAX_TIM-SMALL_IOVECS
					 * vectors available for a large
					 * transfer.  In the worst case,
					 * each will address 4K.  If the
					 * data buffer is not page-aligned,
					 * we could need 1 additional vector.
					 */

#define MAX_DEVICES	128		/* This stands for a possible of
					   16 SCSI devices, each with 8
					   possible LUNs. */
#define MAX_SIDS	 16		 
#define MAX_LUNS	  8
#define START_Q_TAGS   (MAX_DEVICES / PSC_WORDSIZE)     /* alloc word for  */
                                                        /* the queued tags */
#define PSC_MAX_PREEMPTS 80               /* times a cmd can be preempted */

#define	MAX_BLIST	512

#define	MAX_SCRIPTS	(PAGESIZE / sizeof(ulong))

/* Watchdog timeout values */
#define LONGWAIT	5		/* timeout value for abrt/bdrs  */
#define SIOPWAIT	15		/* timeout value for SIOP ABRT  */
#define RESETWAIT	5		/* timeout value for bus resets */

#define DISC_PENDING    0x02              /* a disc. int. may be pending  */

/* p8xx_cleanup_reset err_type definitions */
#define DMA_ERR		1	/* system dma error */
#define REG_ERR		2	/* error accessing a chip register */
#define PHASE_ERR	3	/* bad phase transition on scsi bus */
#define DEVICE_ERR	4	/* bad selection time-out */
#define DISC_ERR	5	/* bad disconnect from scsi bus */
#define HBUS_ERR	7	/* error on host bus */
#define SBUS_ERR	8	/* parity or similar error on scsi bus */

/* miscellaneous error return code */
#define PSC_DUMP_HANG   -1

/* The ioctl SCSI commands      */
#define PSC_NO_ERR      0x00            /* routine call success         */
#define PSC_FAILED      0x01            /* routine call failed          */
#define PSC_COPY_ERROR  0x02            /* error returned during xmem   */
#define PSC_DMA_ERROR   0x03            /* error returned during cleanup*/
#define PSC_OVERFLOW    0x04       	/* error during byte recovery   */
#define PSC_RESET_CHIP  0x01            /* flag used to cleanup register*/
#define PSC_RESET_DMA   0x02            /* flag used to cleanup DMA     */
#define PSC_COMP_RESET  0x000100a0      /* @ of component reset register*/

#define P8_CLOSE_LEVEL   99		/* undo level flag value */

/* unsuccessful allocation values */
#define PSC_ALLOC_FAILED -1
#define PSC_NO_TAG_FREE  -1
#define PSC_NO_TCE_FREE  -2
#define PSC_NO_STA_FREE  -3

/************************************************************************/
/* SCSI Interrupt Defines						*/
/* Note that these values assume that SIST0 & SIST1 have been read	*/
/* together as a single 2-byte register.				*/
/************************************************************************/
#define SCSI_PARITY_ERR		      0x0001
#define SCSI_RST		      0x0002
#define SCSI_UNEXP_DISC		      0x0004
#define SCSI_GROSS_ERR		      0x0008
#define SCSI_RESELECTED		      0x0010
#define SCSI_SELECTED		      0x0020
#define SCSI_COMP		      0x0040
#define PHASE_MIS		      0x0080
#define SCSI_HTH_TIMEOUT	      0x0100
#define SCSI_GEN_TIMEOUT	      0x0200
#define SCSI_SEL_TIMEOUT	      0x0400

#endif	/* _NCR8XXDEF */
