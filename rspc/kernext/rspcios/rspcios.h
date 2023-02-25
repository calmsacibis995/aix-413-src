/* @(#)79       1.4  src/rspc/kernext/rspcios/rspcios.h, rspcios, rspc41J, 9518A_all 5/2/95 11:39:24 */
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:  RSPC I/O Hardware Support
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _h_RSPCIOS
#define _h_RSPCIOS

#ifdef _RSPC

#include <sys/system_rspc.h>
#include <sys/xmem.h>
#include <sys/dma.h>
#include <sys/pmdev.h>


/*
 * The DMA controller in the RSPC is an 82378IB System I/O chip 
 * interfacing the ISA bus to the system.  
 */
#define RSPC_NCTRL	2		/* 2 8237 DMA controllers */
#define RSPC_NCHAN_CTRL 4		/* 4 channels per controller */
#define RSPC_NCHAN	RSPC_NCTRL * RSPC_NCHAN_CTRL	/* total # channels*/
#define RSPC_NTCW	32		/* max # of `tcw'	*/

/*
 * Mode register settings
 */
#define RSPC_WRITE	0x04000000	/* write Transfer Type  */
#define RSPC_READ	0x08000000	/* Read Transfer Type   */

/*      Initializes DMA channel table
 */
#define INIT_DMA_CHAN_TAB() ((ulong)0xFFFFFFFF << (32 - RSPC_NCHAN))

/*      Checks if resource is allocated
 */
#define ALLOCATED(index, bitmask) (!((bitmask)&((ulong)0x80000000 >> (index))))

/*      Checks if resource is available
 */
#define AVAILABLE(index, bitmask) ((bitmask) & ((ulong)0x80000000 >> (index)))

/*      Marks a resource as in use
 */
#define RESERVE(index, bitmask) ((bitmask) &= ~((ulong)0x80000000 >> (index)))

/*      Marks a resource as free
 */
#define FREE(index, bitmask)    ((bitmask) |= ((ulong)0x80000000 >> (index)))

/*      Creates a value for setting the mode register
 */
#define MODE(channel, flags)   ((flags >> 24) | (channel & 3))
#define EXT_MODE(channel,flags)	((flags >> 16) | (channel & 3))

/*      Creates a value for disabling a given channel
 */
#define MASK(channel)   ((channel & 3) | 0x4)

/*      Creates a value for enabling a given channel
 */
#define UNMASK(channel)   (channel & 3)

/*      Checks for terminal count for a given channel
 */
#define TC_REACHED(channel, status) ((status >> (channel & 3)) & 0x01)

/*	Convert a real address into a Bus Address
 */
#define BUSADDR(real)	((ulong)0x80000000 | (ulong)real)

/*	Determine if chan_flags indicate 16-bit word mode
 */
#define SIZE_BITS 0x000C0000
#define WORD_MODE(chan_flags)	((chan_flags & SIZE_BITS) == CH_16BIT_WORDS)
					
#define GET_CTRL_LOCK( oP, nP, pL )    (oP = disable_lock( nP, pL ))

#define REL_CTRL_LOCK( oP, pL )        (unlock_enable( oP, pL ))


/*
 * The following structure represents the scatter/gather Descriptor format
 * These fields are in Little-Endian format.
 *
 *	  MSB				       LSB
 *        byte 3      byte 2      byte 1      byte 0
 *	+---------------------------------------------+
 *      |         32 - bit  Memory Address            |
 *      |          |           |           |          |
 *	+---------------------------------------------+
 *      |EOL|Resvd |     24 - bit  Byte  Count        |
 *      |          |           |           |          |
 *	+---------------------------------------------+
 */
struct rspctcw {
	ulong phys;		/* physical address	     */
	ulong bcount;		/* End Of List - bytecount   */
};

/*
 * The following structure is the info structure per ISA bus
 */
struct isa_dma {
	struct siodchan {
		/*
		 * The following registers have 1 address for
		 * all channels on a given controller....so a
		 * channel select is contained in the data byte
		 * These values represent the register offsets from 0
		 */ 
		int mode;		/*  mode register */
		int mode_shadow;	/*  mode register shadow */
		int ext_mode;		/* extended mode */
		int ext_mode_shadow;	/* extended mode shadow  */
		int mask;		/* single mask bit enable/disable */
		int all_mask;		/* all mask enable/disable */
		int clear_byte_ptr;	/* clear pointer  */
		/*
		 * The following register has 1 address for all
		 * channels on a given controller, and is bit encoded
		 * to represent each channel on the controller
		 */
		int status;		/* DMA status register */
		int status_shadow;	/* shadow of DMA status register */
		/*
		 * lock word address for serializing to this
		 * channels controller
		 */
		Simple_lock *ctrl_lock;
		
		/*
		 * The following registers have a unique address
		 * per channel, so no channel select is needed
		 * These values represent the register offsets from 0
		 */
		int baseaddr;		/* bits 16-31 of addr */
		int count;		/* byte count	      */
		int lopage;		/* bits  8-15 of addr */
		int hipage;		/* bits  0-7  of addr */
		int sgd_table;		/* scatter/gather list ptr */
		int sgd_cmd;		/* scatter/gather cmd */
		int sgd_cmd_shadow;	/* scatter/gather cmd shadow*/
	} chan_info[RSPC_NCHAN];
	uchar  ctrl_dma_status[RSPC_NCTRL]; /* shadow of DMA status */
	uchar  ctrl_dma_mode[RSPC_NCTRL]; /* shadow of DMA mode */
	uchar  ctrl_dma_ext_mode[RSPC_NCTRL]; /* shadow of DMA extmode */
	uchar  ctrl_dma_sgd_cmd[RSPC_NCHAN]; /* shadow of DMA sgd_cmd */
	Simple_lock lock_per_ctrl[RSPC_NCTRL]; /* 1 lock per controller */
	uint	bid;			/* this buses ID 	*/
	struct pm_handle *pmhandle;	/* PM handler for DMA services */
	struct d_pm_save_rspc *dma_save;/* save area for DMA data */
	uint	dma_chans;		/* allocation bit mask  */
	struct io_map iom;		/* I/O map structure for this bus */
	struct isa_dma *next;		/* pointer to next isa struct */
};

/*
 * RSPC specific d_handle structure 
 */
struct d_handle_rspc {
	struct d_handle dh;		/* real d_handle struct          */
	int	bid;			/* bus identifier for this device*/
	struct isa_dma *isa;		/* pointer to isa info	         */
	struct siodchan *ch;		/* pointer to channel info       */
	struct dio slv_dio;		/* slave internal dio struct     */
	struct rspctcw *tcw;	        /* ptr to chans scat/gath list   */
	caddr_t tcw_real;		/* real address of tcw list      */
	caddr_t	bounce;			/* pointer to bounce buffer      */
	caddr_t bounce_real;		/* real address of bounce buffer */ 
	caddr_t b_out_addr;		/* buffer address for copyout    */
	int	b_out_cnt;		/* copyout count		 */
	struct xmem* xm_out;		/* xmem descrip for copyout      */
};	

/*
 * RSPC specific PM DMA save area.
 */
struct d_pm_save_rspc {
	struct per_channel {
		union {
			ushort	baseaddr;
			uchar	byte[2];
		} addr;
		union {
			ushort	basecount;
			uchar	byte[2];
		} count;
		uchar	lopage;
		uchar	hipage;
		uchar	sgd_cmd;
		uint	sgd_table_ptr;
	} perchan[RSPC_NCHAN];
	struct per_controller {
		uchar	all_mask;
		uchar	mode;
		uchar	ext_mode;
	} percont[RSPC_NCTRL];
};

/*************************  FUNCTION PROTOTYPES ******************************/
#ifndef _NO_PROTO

/*
 * Initialize a session with the RSPC DMA mapping services 
 */
d_handle_t d_map_init_rspc(int bid, int flags, int bus_flags, uint channel);	
/* arguments:
 *	int bid;			bus type/number identifier
 *	int flags;			device capabilities
 *	int bus_flags;			flags specific to the target bus
 *	uint channel;			channel assignment specific to dev/bus
 */


void d_map_clear_rspc(d_handle_t handle);	/* free registered handle */
/* arguments:
 *	d_handle_t handle;		handle to be freed
 */

int d_map_disable_rspc(d_handle_t handle);	/* disable a DMA channel */
/* arguments:
 *	d_handle handle;		handle for channel to disable
 */

int d_map_enable_rspc(d_handle_t handle);		/* enable a DMA channel */
/* arguments:
 *	d_handle handle;		handle for channel to enable
 */

/*
 *  Map a slave DMA request
 */
int d_map_slave_rspc(d_handle_t handle, int flags, int minxfer,
		struct dio *vlist, uint chan_flags);
/* arguments:
 *	d_handle handle;		handle 
 *	int flags;			flags controlling transfer
 *	int minxfer;			min xfer size for device (block size)
 *	struct dio *vlist;		list of virtual buffers
 *	uint chan_flags;		channel specific flags
 */

/*
 *  Map a single page DMA request
 */
int d_map_page_rspc(d_handle_t handle, int flags, caddr_t baddr, uint *busaddr,
		   struct xmem *xmp);
/* arguments:
 *	d_handle handle;		handle 
 *	int flags;			flags controlling transfer
 *	caddr_t baddr;			virtual buffer
 *	uint *busaddr;			location to put bus address
 *	struct xmem *xmp;		cross memory descriptor
 */

/*
 *  Map list of DMA requests
 */
int d_map_list_rspc(d_handle_t handle, int flags, int minxfer, struct dio *vlist, 
		struct dio *blist);
/* arguments:
 *	d_handle handle;		handle 
 *	int flags;			flags controlling transfer
 *	int minxfer;			min xfer size for device (block size)
 *	struct dio *vlist		list of virtual buffers
 *	struct dio *blist		list of bus addresses and lengths 
 */

/*
 *  UnMap Slave DMA request
 */
int d_unmap_slave_rspc(d_handle_t handle);
/* arguments:
 *	d_handle handle;		handle 
 */

/*
 * Handler for Power Management calls to DMA kernel extension
 */
int rspcios_pm_handler(caddr_t private, int ctrl);
/* arguments:
 *      caddr_t private;   private data area
 *      int ctrl;          PM control word
 */

/*
 * Function for saving relevant DMA registers
 */
void rspcios_pm_register_save(caddr_t private);
/* arguments:
 *      caddr_t private;   private data area
 */

/*
 * Function for restoring relevant DMA registers
 */
void rspcios_pm_register_restore(caddr_t private);
/* arguments:
 *      caddr_t private;   private data area
 */

#else 

d_handle_t d_map_init_rspc();		/* Initialize session  */
void	d_map_clear_rspc();		/* clear session */
int	d_map_disable_rspc();		/* disable channel */
int	d_map_enable_rspc();		/* enable channel */
int	d_map_slave_rspc();		/* map slave transfer */
int	d_map_page_rspc();		/* map single page */
int	d_map_list_rspc();		/* map list  */
int	d_unmap_slave_rspc();		/* unmap slave transfer */
int 	rspcios_pm_handler();		/* handle PM calls */
void 	rspcios_pm_register_save();	/* save relevant DMA registers */
void 	rspcios_pm_register_restore();	/* restore relevant DMA registers */

#endif /* not _NO_PROTO */

#endif /* _RSPC */

#endif /* _h_RSPCIOS */
