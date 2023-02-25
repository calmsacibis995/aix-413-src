/* @(#)48       1.6  src/rspc/kernext/pci/kent/kent_bits.h, pcient, rspc41J, 9519B_all 5/10/95 15:50:42 */
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: none
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
#ifndef _H_KENTBITS
#define _H_KENTBITS


#include <net/spl.h>
/*
 * state machine possible values
 */
#define CLOSED_STATE 		(0x11111111)	/* the at rest state */
#define OPEN_STATE		(0x22222222)	/* 
						 * open has been called and
						 * completed successfully.
						 * This is the normal 
						 * operational state.
						 */

#define CLOSING_STATE		(0x33333333)	/* shutting down the device */
#define DEAD_STATE		(0x4444dead)	/* the adapter has 
						 * died, it is no 
						 * longer useable 
						 */
#define SUSPEND_STATE		(0x55555555)	/* power management suspend/
						 * hibernate mode has occured 
						 */

#define KENT_OPLEVEL		(PL_IMP)




#define KENT_TX_WDT_RESTART	(10)	/* 10 sec watchdog timer */
#define KENT_MAC_SZ		(14)

/* Chip version ids */
#define KENT_AMD_P1		(0x00)
#define KENT_AMD_P2		(0x10)

/*
 * fastpaths into the driver's acs
 */
#define NDDSTATS (p_acs->ndd.ndd_genstats)
#define ENTSTATS (p_acs->ls.ent_gen_stats)
#define ADAPSTATS (p_acs->ls.kent_stats)
#define MIBS	(p_acs->mibs)
#define MIBSGEN (p_acs->mibs.Generic_mib)
#define MIBSETHSTATS (p_acs->mibs.Ethernet_mib.Dot3StatsEntry)
#define MIBSETHCOLL (p_acs->mibs.Ethernet_mib.Dot3CollEntry)
/*
 * Conversion for dump timers
 */
#define uS_PER_MS      1000      /* Number of microseconds per millisecond */
#define NSEC_PER_MSEC  1000000   /* Number of nanoseconds per millisecond  */

#define	MSEC_PER_SEC	(NS_PER_SEC / NS_PER_MSEC)

/*
 * Component dump table stuff
 */
#define KENT_CDT_ENTRIES	(32)	/* initial size of dump entries */
#define KENT_WORDSHIFT		(2)	/* log 2 for word aligned xmalloc */
#define KENT_DD_STR		"kent_dd"
#define KENT_HDR_LEN		(14)
#define KENT_IO_SIZE		(32)
#define KENT_LADRF_LEN		(8)

#define PIO_RETRY_COUNT       	(3)	/* PIO retry number */
#define KENT_LONGSHIFT		(4)	/* Shift value for mallocs */

/* shift values to get to the tlen and rlen sections of the mode field */
/*  in the initialization block */
#define KENT_TLEN_MODE		(28)	
#define KENT_RLEN_MODE		(20)
				

/*
 * #defines to define the min length acceptable in a call to mib_addr and the 
 * sizeof the mib_addr_elem structure for a KENT address.  The structures 
 * themselves are openended (generic not setup for an actual lan)
 */
#define KENT_MIB_ADDR_MIN_LEN 	(16)
#define KENT_MIB_ADDR_ELEM_LEN	(12)

/*
 * The following are the adapter configuration space pound defines 
 */
#define KENT_CFG_CMD_SERREN	(0x0100)
#define KENT_CFG_CMD_PERREN	(0x0040)
#define KENT_CFG_CMD_BMEN	(0x0004)
#define KENT_CFG_CMD_IOEN	(0x0001)

/*
 * The following are the offset to the different registers in the adapter's
 * io space 
 */
#define KENT_IO_APROM1		(0x00)
#define KENT_IO_APROM2		(0x04)
#define KENT_IO_APROM3		(0x08)
#define KENT_IO_APROM4		(0x0c)
#define KENT_IO_RDP		(0x10)
#define KENT_IO_RAP		(0x14)
#define KENT_IO_RESET		(0x18)
#define KENT_IO_BDP		(0x1c)

/*
 * The following are the bits for the CSR registers used by the device driver
 * For more information about the bit definitions look at the adapter spec
 * for the Am79C970 chip.
 */
/* ERR is set when one of the error bits in CSR0 is set */
#define KENT_CSR0_ERR		(0x00008000) 	

/* BABL is an error bit, it is set when the transmitter has been on the line */
/*  too long */
#define KENT_CSR0_BABL		(0x00004000)

/* CERR is an error bit, it indicates collision inputs to the AUI port failed */
#define KENT_CSR0_CERR		(0x00002000)

/* MISS is an error bit, it indicates that a receive packet was dropped because*/
/*  there wasn't a receive descriptor available */
#define KENT_CSR0_MISS		(0x00001000)

/* MERR is an error bit, it indicates that system bus was not available upon */
/*  request */
#define KENT_CSR0_MERR		(0x00000800)

/* RINT indicates there is a receive packet available */
#define KENT_CSR0_RINT		(0x00000400)

/* TINT indicates there is a transmit packet complete */
#define KENT_CSR0_TINT		(0x00000200)

/* IDON indicates that initialization is complete */
#define KENT_CSR0_IDON		(0x00000100)

/* INTR indicates that there is an interrupt active. */
#define KENT_CSR0_INTR		(0x00000080)

/* IENA indicates that interrupts have been enabled */
#define KENT_CSR0_IENA		(0x00000040)

/* RXON indicates that receive is turned on (adapter can receive) */
#define KENT_CSR0_RXON		(0x00000020)

/* TXON indicates that transmit is turned on (adapter can transmit) */
#define KENT_CSR0_TXON		(0x00000010)


/* TDMD tells the adapter to begin transmitting regardless of the transmit poll*/
/*  timer */
#define KENT_CSR0_TDMD		(0x00000008)

/* STOP tells the adapter to halt all activities */
#define KENT_CSR0_STOP		(0x00000004)

/* STRT tells the adapter to start running */
#define KENT_CSR0_STRT		(0x00000002)

/* INIT tells the adapter to begin the initialization process */
#define KENT_CSR0_INIT		(0x00000001)



#define KENT_CSR3_BABLM		(0x00004000)
#define KENT_CSR3_MISSM		(0x00001000)
#define KENT_CSR3_MERRM		(0x00000800)
#define KENT_CSR3_RINTM		(0x00000400)
#define KENT_CSR3_TINTM		(0x00000200)
#define KENT_CSR3_IDONM		(0x00000100)
#define KENT_CSR3_LAPPEN	(0x00000020)
#define KENT_CSR3_DXMT2PD	(0x00000010)
#define KENT_CSR3_EMBA		(0x00000008)
#define KENT_CSR3_BSWP		(0x00000004)

#define KENT_CSR4_ENTST		(0x00008000)
#define KENT_CSR4_DMAPLUS	(0x00004000)
#define KENT_CSR4_TIMER		(0x00002000)
#define KENT_CSR4_DPOLL		(0x00001000)
#define KENT_CSR4_APAD_XMT	(0x00000800)
#define KENT_CSR4_ASTRP_RCV	(0x00000400)
#define KENT_CSR4_MFCO		(0x00000200)
#define KENT_CSR4_MFCOM		(0x00000100)
#define KENT_CSR4_RCVCCO	(0x00000020)
#define KENT_CSR4_RCVCCOM	(0x00000010)
#define KENT_CSR4_TXSTRT	(0x00000008)
#define KENT_CSR4_TXSTRTM	(0x00000004)
#define KENT_CSR4_JAB		(0x00000002)
#define KENT_CSR4_JABM		(0x00000001)

#define KENT_CSR15_PROM		(0x00008000)
#define KENT_CSR15_DRCVBC	(0x00004000)
#define KENT_CSR15_DRCVPA	(0x00002000)
#define KENT_CSR15_DLNKTST	(0x00001000)
#define KENT_CSR15_DAPC		(0x00000800)
#define KENT_CSR15_MENDECL	(0x00000400)
#define KENT_CSR15_TSEL		(0x00000200)
#define KENT_CSR15_PORTSEL2	(0x00000100)
#define KENT_CSR15_PORTSEL1	(0x00000080)
#define KENT_CSR15_INTL		(0x00000040)
#define KENT_CSR15_DRTY		(0x00000020)
#define KENT_CSR15_FCOLL	(0x00000010)
#define KENT_CSR15_DXMTFCS	(0x00000008)
#define KENT_CSR15_LOOP		(0x00000004)
#define KENT_CSR15_DTX		(0x00000002)
#define KENT_CSR15_DRX		(0x00000001)

#define KENT_CSR58_PCI		(0x00000002)

#define KENT_CSR0_MASK	(KENT_CSR0_BABL|KENT_CSR0_MISS|KENT_CSR0_MERR|	\
			KENT_CSR0_CERR|KENT_CSR0_RINT|KENT_CSR0_TINT|	\
			KENT_CSR0_IDON|KENT_CSR0_IENA)
/*
 * The following are the bits for the BCR registers used by the device driver
 */

#define KENT_BCR2_ASEL		(0x00000002)

#define KENT_BCR18_BREADE	(0x00000040)
#define KENT_BCR18_BWRITE	(0x00000020)

#define KENT_BCR19_PVALID	(0x00008000)
#define KENT_BCR19_PREAD	(0x00004000)
#define KENT_BCR19_EEDET	(0x00002000)

#define KENT_BCR20_SWSTYLE	(0x00000002)

/*
 * Constants for the tx/rx descriptors
 */
#define KENT_BUFF_SZ		(2048)
#define KENT_BLEN_MASK		(0xfff)
#define KENT_MCAST		(0x01)

/*
 * The following defines are for the receive descriptors 
 */

#define KENT_RX_STAT1_OWN	(0x80000000)
#define KENT_RX_STAT1_ERR	(0x40000000)
#define KENT_RX_STAT1_FRAM	(0x20000000)
#define KENT_RX_STAT1_OFLO	(0x10000000)
#define KENT_RX_STAT1_CRC	(0x08000000)
#define KENT_RX_STAT1_BUFF	(0x04000000)
#define KENT_RX_STAT1_STP	(0x02000000)
#define KENT_RX_STAT1_ENP	(0x01000000)
#define KENT_RX_STAT1_ONES	(0x0000f000)

#define KENT_RX_STAT2_RPC	(0x00ff0000)
#define KENT_RX_STAT2_RCVCO	(0xff000000)

/*
 * The following defines are for the transmit descriptors
 */

#define KENT_TX_STAT1_OWN	(0x80000000)
#define KENT_TX_STAT1_ERR	(0x40000000)
#define KENT_TX_STAT1_ADD_FCS	(0x20000000)
#define KENT_TX_STAT1_MORE	(0x10000000)
#define KENT_TX_STAT1_ONE	(0x08000000)
#define KENT_TX_STAT1_DEF	(0x04000000)
#define KENT_TX_STAT1_STP	(0x02000000)
#define KENT_TX_STAT1_ENP	(0x01000000)
#define KENT_TX_STAT1_ONES	(0x0000f000)

#define KENT_TX_STAT2_BUFF	(0x80000000)
#define KENT_TX_STAT2_UFLO	(0x40000000)
#define KENT_TX_STAT2_EXDEF	(0x20000000)
#define KENT_TX_STAT2_LCOL	(0x10000000)
#define KENT_TX_STAT2_LCAR	(0x08000000)
#define KENT_TX_STAT2_RTRY	(0x04000000)
#define KENT_TX_STAT2_TRC	(0x0000000f)

#endif /* end if ! _H_KENTBITS */
