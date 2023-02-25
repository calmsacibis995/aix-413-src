/* "@(#)50  1.1  src/rspc/kernext/pci/stok/stok_mac.h, pcitok, rspc41J 3/21/95 17:27:53" */
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: CLEAR_STATS
 *		COPY_NADR
 *		DEC_DESC
 *		INC_DESC
 *		NDD_ELAPSED_TIME
 *		SAME_NADR
 *		TRACE_BOTH
 *		TRACE_DBG
 *		TRACE_SYS
 *		TX_DISABLE
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

#ifndef _H_STOK_MAC
#define _H_STOK_MAC

#define STOK_DD_NAME		"stok_dd"/* driver name for dump             */
#define MEM_ALIGN		2
#define MAX_MULTI               256     /* max group addr support by adapter */
#define MULTI_ENTRY             16      /* len of multi_address table entry  */
#define STOK_HDR_LEN		14	/* Skyline  header length            */
#define MULTI_BIT_MASK		0x80	/* multicast bit in first addr. byte */
#define CARD_ENABLE		0x01	/* card enable bit in pos2 register  */
#define TOKEN_ACCESS_PRIORITY   0x06    /* priority Tx token access priority */
#define MAX_INDEX               (160)

#define TOK_MAX_ADAPTERS   	(32)    /* maximum adapters in system        */
#define MAX_TX_LIST      	(32)    /* Maximum Transmit frame list       */
#define MAX_DMA_LIST      	(96)    /* Maximum DMA list                  */
#define MAX_BUF_LIST      	(80)    /* Maximum BUF list                  */
#define TX_LIST_SIZE      	(0x20)  /* length of TX element              */
#define RX_LIST_SIZE     	(16)    /* size of receive elem              */
#define HAFT_PAGE     		(2048)  /* Haft page size                    */
#define MAX_STOK_VPD_LEN         128    /* max vpd size for stok adapter     */
#define STOK_IO_SIZE         	 256    /* size of io space                  */ 
#define LONG			  32 
#define ERROR			  1 
#define OK			  0 



/*----------------------------------------------------------------------------*/
/* adapter configuration space defines                                        */
/*----------------------------------------------------------------------------*/
#define CID			0x14101800
#define	VID			0x00
#define	DID			0x02
#define	PCR			0x04
#define	CLS			0x0C
#define	LTR			0x0D
#define	BA0			0x10
#define	BA1			0x14
#define	BAR			0x30
#define	ILR			0x3C
#define	GPR			0x4A

/*----------------------------------------------------------------------------*/
/* PCI Command Register bits define                                           */
/*----------------------------------------------------------------------------*/
#define IO_SPACE		0x0001
#define MEM_SPACE		0x0002
#define BUS_MASTER		0x0004
#define MWI_EN			0x0010
#define PRTY_ERR		0x0040
#define SEER_ENABLE		0x0100
#define FBB_ENABLE		0x0200

/*----------------------------------------------------------------------------*/
/*                                RETURN CODES                                */
/* These are the possible error codes return from the adapter.  Some of them  */
/*			     are not currently use.                           */
/*----------------------------------------------------------------------------*/
#define OPERATION_SUCCESSFULLY  0x00    /* Operation Complete successfully    */
#define UNRECOGNIZED_CODE       0x01    /* Unrecognized command code          */
#define ADAPTER_OPEN            0x03    /* Adapter Open, should be close      */
#define ADAPTER_CLOSE           0x04    /* Apapter close, should be open      */
#define PARAMETER_NEED          0x05    /* Required parameter not provided    */
#define COMMAND_CANCELLED       0x07    /* Command cancelled                  */
#define UNAUTHORIZED_ACCESS     0x08    /* Unauthorized access priority       */
#define LOST_DATA               0x20    /* Lost data on receive               */
#define ERROR_FRAME             0x22    /* Error on frame transmission        */
#define ERROR_DATA              0x23    /* Error in frame transmit or strip   */
#define UNAUTHORIZED_MAC        0x24    /* Unauthorized MAC frame             */
#define UNRECOGNIZED_CORRELATOR 0x26    /* Unrecognized command correlator    */
#define INVALID_LEN             0x28    /* Invalid frame length for transmit  */
#define UNAUTHORIZED_LLC        0x29    /* Unauthorized LLC frame             */
#define INVALID_ROUTE           0x2A    /* Invalid routing information        */
#define ROUTE_NOT_EXPECT        0x2B    /* Routing information not expected   */
#define UNAUTHORIZE_ENT         0x2E    /* Unauthorized Ethernet frame        */
#define INVALID_NODE_ADDR       0x32    /* Invalid node address               */
#define INVALID_BUF_LEN         0x34    /* Invalid adapter transmit buf_len   */
#define GROUP_FULL              0x35    /* Group address register full        */
#define RING_OUT_OF_RANGE       0x36    /* Ring number out of range           */
#define CHANNEL_INUSER          0x37    /* Priority channel already in use    */
#define BRIDGE_NOT_ISSUED       0x38    /* Set bridge parameter not issued    */
#define NO_GROUP_ADDRESS        0x39    /* Group address not found            */
#define INVALID_SOURCE_ADDRESS  0x3A    /* Invalid source address             */
#define BRIDGE_OUT_RANGE        0x3B    /* Bridge number out of range         */
#define NOT_SET_GROUP           0x3C    /* Group/functional address indicator */
                                        /* bit not set correctly              */
#define NOT_MULTI_PORT          0x3D    /* Multi_port bridge option not select*/
#define INDEX_GROUP_FULL        0x3E    /* Indexed group register full        */
#define GEN_GROUP_FULL          0x3F    /* General group register full        */
#define NOT_MULTI_ADDR          0x50    /* Multiple address not expected      */
#define LOCAL_ADDR_LARGE        0x51    /* Number local address too large     */
#define NO_MULTI_BRIDGE         0x52    /* Target ring not expected for multi */
                                        /* port bridge                        */
#define RING_NOT_FOUND          0x53    /* Target ring not found              */
#define INVALID_GROUP_TYPE      0x54    /* Invalid group type                 */
#define ALREADY_SET_GROUP       0x55    /* Group address already set          */
#define NO_BRIDGE_CONFIG        0x56    /* Config bridge channel not issued   */
#define GROUP_ADDRESS_SET       0x5A    /* Group address set                  */
#define ADDRESS_NOT_FOUND       0x5B    /* address no found                   */
#define INVALID_CONFIGURATION   0x62    /* invalid configuration              */
#define RESPONSE_VALID          0xFF    /* Response valid, ASB available      */
#define PROMIS_ON          	0x41    /* promiscuous mode on                */
#define PROMIS_OFF          	0x00    /* promiscuous mode off               */


/******************************************************************************/
/*          LAN STATUS CODES (return code for LAN.STATUS.CHANGE)              */
/******************************************************************************/
#define  SIGNAL_LOSS                 0x8000
#define  HARD_ERROR                  0x4000
#define  SOFT_ERROR                  0x2000
#define  TRANSMIT_BEACON             0x1000
#define  LOBE_WIRE_FAULT             0x0800
#define  AUTO_REMOVAL_ERROR          0x0400
#define  REMOVE_RECEIVE              0x0100
#define  COUNTER_OVERFLOW            0x0080
#define  SINGLE_STATION              0x0040
#define  RING_RECOVERY               0x0020
#define  SR_BRIDGE_COUNTER_OVERFLOW  0x0010
#define  CABLE_NOT_CONNECTED         0x0008
#define  LAN_OPEN_RESERVE            0x0207
#define  ERR_LOG                     0x2000 | 0x0080 | 0x0010

/******************************************************************************
 *      trace macros
******************************************************************************/
#define STOK_TRACE_TOP	"stoktraceTOP!!!"   /* top of trace table */
#define STOK_TRACE_BOT	"stoktraceBOT!!!"   /* bottom of trace table */
#define STOK_TRACE_CUR	"stoktraceCUR!!!"   /* Current trace table entry */

#define STOK_TX        ((HKWD_STOK_SFF_XMIT << 20)  | HKTY_GT | 4)
#define STOK_RV        ((HKWD_STOK_SFF_RECV << 20)  | HKTY_GT | 4)
#define STOK_ERR       ((HKWD_STOK_SFF_ERR << 20)   | HKTY_GT | 4)
#define STOK_OTHER     ((HKWD_STOK_SFF_OTHER << 20) | HKTY_GT | 4)

/******************************************************************************
 * Trace points defined for the netpmon performance tool
******************************************************************************/
#define TRC_WQUE        "WQUE"  /* write data has just been queued */
#define TRC_WEND        "WEND"  /* write is complete              */
#define TRC_RDAT        "RDAT"  /* a packet was received    */
#define TRC_RNOT        "RNOT"  /* a packet was passed up        */
#define TRC_REND        "REND"  /* receive is complete      */

/******************************************************************************
 * Internal trace table size and trace macros
******************************************************************************/

#ifdef TRACE_DEBUG
#define STOK_TRACE_SIZE	1024		/* 4096 bytes, 256 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	stok_trace(hook, tag, arg1, arg2, arg3)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	\
	stok_trace(hook, tag, arg1, arg2, arg3)
#else
#define STOK_TRACE_SIZE	128             /* 512 bytes, 32 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	TRCHKGT(hook, *(ulong *)tag, arg1, arg2, arg3, 0)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	
#endif 

#define TRACE_BOTH(hook, tag, arg1, arg2, arg3)	\
	stok_trace(hook, tag, arg1, arg2, arg3)

/******************************************************************************
 * The component dump table will have the dd_ctl table,  and one dev_ctl 
 * for each adapter that is opened. Leave one extra entry always empty for
 * the table management.
******************************************************************************/
#define STOK_CDT_SIZE		(2 + TOK_MAX_ADAPTERS)

/******************************************************************************
 * Saves the ndd start time for statistics and clears the counters
******************************************************************************/

#define  CLEAR_STATS()		                 		\
        WRK.ndd_stime = WRK.dev_stime =lbolt;			\
        bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));	\
        bzero(&TOKSTATS, sizeof(TOKSTATS));			\
        bzero(&DEVSTATS, sizeof(DEVSTATS));			
                                        

/******************************************************************************
 * Compares two network address
******************************************************************************/
#define SAME_NADR(a, b) ( \
        *((ulong *)(a)) == *((ulong *)(b)) && \
        *((ushort *)((char *)(a) + 4)) == *((ushort *)((char *)(b) + 4)) \
                        )

/******************************************************************************
 * Copys a network address from a to b
******************************************************************************/
#define COPY_NADR(a, b)	{ \
	*((ulong *)(b)) = *((ulong *)(a)); \
	*((ushort *)((char *)(b) + 4)) = *((ushort *)((char *)(a) + 4)); \
			}
/******************************************************************************
 * Calculates the elapsed time by subtract the start tick counter (s) from
 * the current tick counter (lbolt) and convert it to seconds
******************************************************************************/
#define NDD_ELAPSED_TIME(s)     ((lbolt - s) / HZ)


#define TX_DISABLE(x) { \
  ulong         dbas, dba; \
  i = 0; \
  do { \
          if (i++ > 20) { \
                   TRACE_SYS(STOK_ERR, "res1", p_dev_ctl, dba, dbas); \
                   break; \
           } \
           dbas = dba; \
           io_delay(100); \
           PIO_GETLRX(ioa + x, &dba); \
  } while (dbas != dba); \
}

#define DEC_DESC(x,y) {(x)--; if ((x) < 0) (x) = y - 1;}
#define INC_DESC(x,y) {(x)++; if ((x) == y) (x) = 0;}

#endif /* _H_STOK_MAC */
