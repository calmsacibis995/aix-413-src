static char sccsid[] = "@(#)78  1.18  src/rspc/kernext/rspcios/rspcios.c, rspcios, rspc41J, 9520A_all 5/16/95 16:34:41";
/*
 * COMPONENT_NAME: (SYSIOS) RSPC I/O Subsystem Kernel Extension 
 *
 * FUNCTIONS: 
 *		rspcios_init, d_map_init_rspc, d_map_clear_rspc, 
 *		d_map_slave_rspc, d_map_page_rspc, d_map_list_rspc, 
 *		d_unmap_slave_rspc, d_map_disable_rspc, d_map_enable_rspc,
 *		rspcios_pm_handler, rspcios_pm_register_save,
 *		rspcios_pm_register_restore
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
 *
 * GLOBAL NOTES:
 *
 *	These routines represent the RSPC versions of the Non-MicroChannel
 *	DMA services specifically for the RSPC PCI/ISA implementation.
 *
 *	SERIALIZATION
 *
 *	These services only serialize the allocating and freeing of DMA
 *	resources (d_map_init and d_map_clear) and the power management 
 *	handler. The caller is responsible for serializing the calls to
 *	the other DMA services. These services can be thought of as an
 *	extension of the device which the device driver already has to
 *	serialize its access to. These services must serialize when using
 *	registers shared by more than one channel.
 *
 *	SYSTEM INTERACTION
 *
 *	DMA bypasses much of the normal memory reference hardware. Therefore
 *	this code must provide a reliable environment in which to perform
 *	a DMA operation. Much of this work is performed by the xmemdma
 *	service.
 *
 *****************************************************************************/

#ifdef _RSPC
 
#include <sys/dma.h>
#include <sys/iplcb.h>
#include <sys/seg.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/rheap.h>
#include <sys/xmem.h>
#include <sys/syspest.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/systemcfg.h>
#include <sys/system_rspc.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/inline.h>
#include <sys/ioacc.h>
#include <sys/mdio.h>
#include "rspcios.h"

 
int	isa_bus_count = 0;		/* count of registered ISA buses*/	
int	pci_bus_count = 0;		/* count of registered PCI buses*/	
int	lock_count = 0;			/* count of DMA class locks     */

Simple_lock rspc_lock;			/* allocation/deallocation lock */

struct isa_dma *isa_anchor = NULL;	/* anchor for ISA bus info structs */

/*
 * NAME: rspcios_init
 *
 * FUNCTION:  Initialize the RSPCIOS Kernel Extension and Related
 *	      Hardware and Data Structures.
 *
 * EXECUTION ENVIRONMENT: Called only from the process environment at
 *			  configuration time.
 *
 * RETURN VALUE DESCRIPTION: 
 *		0     = Successful Configuration
 *		results of bus_register() if non-zero
 *		results of pincode() if non-zero
 *		ENXIO = Not a supported Bus Type
 *		EINVAL = This bus already initialized
 *		       = Unsupported configuration option
 *		ENOMEM = Couldn't allocate memory
 *
 * EXTERNAL PROCEDURES CALLED: 
 *		xmalloc, pincode, xmfree
 *
 * ACCESSED:	
 */
int 
rspcios_init(
	int op,
	struct uio *uiop)
{

        uint bid;                      /* bus identifier for this bus */
	int	i,rc = 0;
	int	region;
	struct isa_dma *isa = isa_anchor;	/* isa info pointer */
	struct isa_dma *prev = NULL;
	struct isa_dma **new;
	struct siodchan *ch;
	volatile struct rspcsio *sio;	/* sio register pointer     */
	volatile struct sio_cfg *sio_cfg; /* sio config register pointer  */
	struct pm_handle *pmh;
	extern d_handle_t d_map_init_rspc();
	struct businfo b;		/* bus info structure 	    */
	struct buc_info buc;		/* buc info structure       */
	struct mdio mdio;
	uchar revision;
	

	/*
	 * Allocate and initialize the allocation/deallocation
	 * lock.  Notice that we never free the lock...just like we never
	 * unpin the code...since this DMA code is considered part of
	 * the kernel
	 */ 
	lock_alloc (&rspc_lock, LOCK_ALLOC_PAGED, DMA_LOCK_CLASS, lock_count++);
	simple_lock_init (&rspc_lock);

	simple_lock(&rspc_lock);		/* serialize configuration */

        switch (op) {
        case CFG_INIT :                         /* Config Init */


                /*
                 * copy config info into our local structure
                 */
                uiomove((caddr_t)&buc, (int) sizeof(struct buc_info ),
                        UIO_WRITE, (struct uio *) uiop);
		bid = buc.buid_data[0].buid_value;

		if ((BID_TYPE(bid) != IO_ISA) &&
		    (BID_TYPE(bid) != IO_PCI)) {
			rc = ENXIO;
			break;
		}
		/*
		 * Register this bus with the OS
		 */
		b.bid = bid;
		b.d_map_init = (void *(*)())d_map_init_rspc;
		b.num_regions = buc.num_of_buids;
		for(i=0;i<buc.num_of_buids;i++) {
			/*
			 * for each region
			 */
                        region = BID_REGION(buc.buid_data[i].buid_value);
                        ASSERT(region < MAX_REGIONS);
                        b.ioaddr[region] = (long long) 
						buc.buid_data[i].buid_Sptr;
               }

		rc = bus_register(&b);
		ASSERT(rc != 0);
		if (rc) break;
		if (BID_TYPE(bid) == IO_ISA) {
			/* 
			 * if this is an ISA bus
			 */
			while (isa) {
				/*
				 * and at least one is already registered
				 */
				if (bid == isa->bid)	break;			
				else {	
					prev = isa;
					isa = isa->next;
				}
			}
			if (isa != NULL) {
				/*
				 * this bus is already registered
				 */
				rc = EINVAL;
				break;
			} 
			/*
			 * register this bus
			 */
			if (prev) 
				new = &prev->next;
			else
				new = &isa_anchor;
			*new = (struct isa_dma *)xmalloc((uint)
					sizeof(struct isa_dma), 2,
					(heapaddr_t)pinned_heap); 
			if (*new == NULL) {
				rc = ENOMEM;
				break;
			}
			bzero(*new, sizeof(struct isa_dma));
			isa = *new;
			isa->bid = bid;		/* store bid */
			isa->dma_chans = INIT_DMA_CHAN_TAB();
			/*
			 * Set up I/O map structure
			 */
			isa->iom.key = IO_MEM_MAP;
			isa->iom.flags = 0;
			isa->iom.size = SEGSIZE; 
			isa->iom.busaddr = 0;
			isa->iom.bid = BID_ALTREG(bid, ISA_IOMEM);

			/*
			 * Allocate and initialize simple locks for
			 * serializing access to each DMA controller
			 */
			lock_alloc (&isa->lock_per_ctrl[0], LOCK_ALLOC_PIN,
				DMA_LOCK_CLASS, (isa_bus_count * 2));
			simple_lock_init (&isa->lock_per_ctrl[0]);
			lock_alloc (&isa->lock_per_ctrl[1], LOCK_ALLOC_PIN,
				DMA_LOCK_CLASS, (isa_bus_count * 2) + 1);
			simple_lock_init (&isa->lock_per_ctrl[1]);

			/* 
			 * Initialize the power management handler
			 */
			pmh = 
			isa->pmhandle = (struct pm_handle *)xmalloc((uint)
				sizeof(struct pm_handle), 2,
				(heapaddr_t)pinned_heap); 
			if (pmh == NULL) {
				rc = ENOMEM;
				break;
			}
			pmh->activity = PM_ACTIVITY_NOT_SET;
			pmh->mode = PM_DEVICE_FULL_ON;
			pmh->device_idle_time = 0;
			pmh->device_standby_time = 0;
			pmh->idle_counter = 0;
			pmh->handler = rspcios_pm_handler;
			pmh->private = (uchar *)isa;
			pmh->devno = 0;
			pmh->attribute = 0;
			pmh->device_logical_name = "rspcios";
			pmh->pm_version = PM_PHASE2_SUPPORT;

			/* 
			 * Allocate the dma data save area
			 */
			isa->dma_save = (struct d_pm_save_rspc *)xmalloc((uint)
				sizeof(struct d_pm_save_rspc), 2,
				(heapaddr_t)pinned_heap); 
			if (isa->dma_save == NULL) {
				rc = ENOMEM;
				break;
			}

			/* Register Power Management handle */
			rc = pm_register_handle((struct pm_handle *)(pmh),
				PM_REGISTER);
			ASSERT(rc == 0);

			/*
		 	 * determine register offsets
			 * for all channels on this controller
		 	 */
			sio = (struct rspcsio *) 0; /* offsets from 0*/
			for (i=0;i<RSPC_NCHAN;i++) {
				ch = &isa->chan_info[i];	
				ch->sgd_table = (int) &sio->dma_sg_dtp[i];
				ch->sgd_cmd = (int) &sio->dma_sg_cmd[i];
				ch->sgd_cmd_shadow = 
					(int) &isa->ctrl_dma_sgd_cmd[i];
				switch (i) {
				    case 0: case 1: case 2: case 3:
					ch->clear_byte_ptr = 
							(int) &sio->dma1_cbp;
					ch->mode = (int) &sio->dma1_dcm;
					ch->mode_shadow = 
						(int)&isa->ctrl_dma_mode[0];
					ch->ext_mode = (int)&sio->dma1_dcem;
					ch->ext_mode_shadow =
						(int)&isa->ctrl_dma_ext_mode[0];
					ch->mask = (int)&sio->dma1_sm;
					ch->all_mask = (int)&sio->dma1_amr;
					ch->status = (int)&sio->dma1_ds;
					ch->status_shadow = 
						(int)&isa->ctrl_dma_status[0];
					ch->ctrl_lock = &isa->lock_per_ctrl[0];
					break;

				    case 4: case 5: case 6: case 7:
					ch->clear_byte_ptr = 
							(int)&sio->dma2_cbp;
					ch->mode = (int)&sio->dma2_dcm;
					ch->mode_shadow = 
						(int)&isa->ctrl_dma_mode[1];
					ch->ext_mode = (int)&sio->dma2_dcem;
					ch->ext_mode_shadow =
						(int)&isa->ctrl_dma_ext_mode[1];
					ch->mask = (int)&sio->dma2_sm;
					ch->all_mask = (int)&sio->dma2_amr;
					ch->status = (int)&sio->dma2_ds;
					ch->status_shadow = 
						(int)&isa->ctrl_dma_status[1];
					ch->ctrl_lock = &isa->lock_per_ctrl[1];
					break;
				}

				switch (i) {
				    case 0:
					ch->baseaddr = (int)&sio->dma1_ch0_bca;
					ch->count = (int)&sio->dma1_ch0_bcc;
					ch->lopage = (int)&sio->dma_page_ch0;
					ch->hipage = (int)&sio->dma_hipage_ch0;
					break;
		
				    case 1:
					ch->baseaddr = (int)&sio->dma1_ch1_bca;
					ch->count = (int)&sio->dma1_ch1_bcc;
					ch->lopage = (int)&sio->dma_page_ch1;
					ch->hipage = (int)&sio->dma_hipage_ch1;
					break;
			
				    case 2:
					ch->baseaddr = (int)&sio->dma1_ch2_bca;
					ch->count = (int)&sio->dma1_ch2_bcc;
					ch->lopage = (int)&sio->dma_page_ch2;
					ch->hipage = (int)&sio->dma_hipage_ch2;
					break;
			
				    case 3:
					ch->baseaddr = (int)&sio->dma1_ch3_bca;
					ch->count = (int)&sio->dma1_ch3_bcc;
					ch->lopage = (int)&sio->dma_page_ch3;
					ch->hipage = (int)&sio->dma_hipage_ch3;
					break;
			
				    case 5:
					ch->baseaddr = (int)&sio->dma2_ch1_bca;
					ch->count = (int)&sio->dma2_ch1_bcc;
					ch->lopage = (int)&sio->dma_page_ch5;
					ch->hipage = (int)&sio->dma_hipage_ch5;
					break;
			
				    case 6:
					ch->baseaddr = (int)&sio->dma2_ch2_bca;
					ch->count = (int)&sio->dma2_ch2_bcc;
					ch->lopage = (int)&sio->dma_page_ch6;
					ch->hipage = (int)&sio->dma_hipage_ch6;
					break;
			
				    case 7:
					ch->baseaddr = (int)&sio->dma2_ch3_bca;
					ch->count = (int)&sio->dma2_ch3_bcc;
					ch->lopage = (int)&sio->dma_page_ch7;
					ch->hipage = (int)&sio->dma_hipage_ch7;
					break;
		
				    case 4 :		/* reserved */
				    default:
					break;
				}
			}


			/*
			 * Now set up necessary SIO hardware
			 */
				
			sio = (struct rspcsio *)iomem_att(&isa->iom);

			sio->dma2_mclear = 1;	/* reset controller 2 */
			__iospace_eieio();	/* order this */
			sio->dma1_mclear = 1;	/* reset controller 1 */
			__iospace_sync();	/* order/sync this */
			/*
			 * Unmask channel 4
			 */
			sio->dma2_dcm = 0xc0;	/* put chan 4 in cascade mode */
			isa->ctrl_dma_mode[1] = 0xc0;	/* shadow mode */
			__iospace_eieio();	/* order this */
			sio->dma2_sm = UNMASK(4); /* unmask the channel */
			__iospace_sync();	/* order/sync this */
			iomem_det((void *)sio);

			/*
			 * Reserve channel 4 of controller 2, this 
			 * cascades the 2nd controller 
			 *
			 * --------            --------
			 * | 0  C |----------->| 4  C |
			 * |    T |            |    T |
			 * | 1  R |            | 5  R |
			 * |    L |            |    L |
			 * | 2    |            | 6    |
			 * |    1 |            |    2 |
			 * | 3    |            | 7    |
			 * --------            --------
			 */
			RESERVE(4, isa->dma_chans);	
			isa_bus_count++;
		} else {
			/*
			 * PCI Bus
			 * Nothing really needed to support PCI
			 */
			pci_bus_count++;
		}
		if ((pci_bus_count + isa_bus_count) == 1) {
			/*
			 * If this was the first bus, then pin this
			 * kernel extension
			 */
                        if (rc = pincode((int(*)())rspcios_init)) 
				break;
		}
		break;
	default:
		rc = EINVAL;	/* we don't support any other config options*/
	}

	simple_unlock(&rspc_lock);
	return rc;
}


/*
 * NAME: d_map_init_rspc
 *
 * FUNCTION: This service allocates and initializes a DMA channel.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called only from the process level,
 *	and is called by the d_map_init kernel service.
 *
 * RETURN VALUE DESCRIPTION: 
 *	d_handle_t  -  On successful completion. This value is then passed
 *		       as a parameter to other DMA routines. Using this
 *		       value, these routines can determine specific channel
 *		       information.
 *
 *	DMA_FAIL    -  Unable to allocate.
 *
 * EXTERNAL PROCEDURES CALLED: 
 *
 */
d_handle_t 
d_map_init_rspc(
	int	bid,			/* bus identifier	*/	
	int	flags,			/* device flags		*/
	int	bus_flags,		/* bus specific flags   */
	uint	channel)		/* channel assignment	*/
{

	struct isa_dma *isa = isa_anchor;	/* isa info pointer */
	register struct siodchan *ch; 
	volatile char *io;
	struct d_handle_rspc	*rspcd;
	int opri;
	char emode;

	if ((BID_TYPE(bid) != IO_ISA) && (BID_TYPE(bid) != IO_PCI)) {
		/*
		 * Not a recognized bus type
		 */
		return((struct d_handle *)DMA_FAIL);
	}
	if ((BID_TYPE(bid) == IO_PCI) && (flags & DMA_SLAVE))
		/*
		 * Can't do slave DMA with PCI
		 */
		return((struct d_handle *)DMA_FAIL);

	/*
	 * Allocate and initialize d_handle structure
	 */
	rspcd = (struct d_handle_rspc *)
			xmalloc(sizeof(struct d_handle_rspc), 2, pinned_heap);
	if (rspcd == NULL)	return((struct d_handle *)DMA_FAIL);

	rspcd->bid = bid;
	rspcd->dh.flags = flags;
	rspcd->dh.d_map_page = (int(*)())d_map_page_rspc;
	rspcd->dh.d_unmap_page = (void(*)())NULL;	/* no unmap needed */
	rspcd->dh.d_map_list = (int(*)())d_map_list_rspc;	
	rspcd->dh.d_unmap_list = (void(*)())NULL;	/* no unmap needed */
	rspcd->dh.d_map_slave = (int(*)())d_map_slave_rspc;	
	rspcd->dh.d_unmap_slave = (int(*)())d_unmap_slave_rspc;	
	rspcd->dh.d_map_disable = (int(*)())d_map_disable_rspc;	
	rspcd->dh.d_map_enable = (int(*)())d_map_enable_rspc;	
	rspcd->dh.d_map_clear = (void(*)())d_map_clear_rspc;	

	if (BID_TYPE(bid) == IO_ISA) {
		/*
		 * serialize allocation
		 */
		simple_lock(&rspc_lock);
		/*
		 * if this is an initialization for the ISA bus
		 */
		while (isa) {
			/*
			 * find this one in the list
			 */
			if (bid == isa->bid)	break;			
			else isa = isa->next;
		}
		if (isa == NULL) {
			/*
			 * couldn't find it
			 */
			xmfree(rspcd, pinned_heap);
			simple_unlock(&rspc_lock);
			return((struct d_handle *)DMA_FAIL);
		}
		if (ALLOCATED(channel, isa->dma_chans)) {
			/*
			 * This channel is already allocated
			 */
			xmfree(rspcd, pinned_heap);
			simple_unlock(&rspc_lock);
			return((struct d_handle *)DMA_FAIL);
		}
		ch = &isa->chan_info[channel];	/* setup channel ptr */
		rspcd->ch = ch;		/* save channel pointer  */
		rspcd->isa = isa;		/* save isa info pointer */
		rspcd->dh.id = channel;	/* save channel as id    */
				
		io = (volatile char *)iomem_att(&isa->iom);

		if (flags & DMA_SLAVE) {
			/*
			 * Setup for ISA slave
			 */
			*(io + ch->mask) = MASK(channel);  
			GET_CTRL_LOCK(opri, INTMAX, rspcd->ch->ctrl_lock);
			*(io + ch->mode) = MODE(channel, CH_DEFAULT);	
			*(uchar *)rspcd->ch->mode_shadow = 
				MODE(channel, CH_DEFAULT);
			REL_CTRL_LOCK(opri, rspcd->ch->ctrl_lock);
			/*
			 * Initialize a bus dio list for internal use
			 */
			rspcd->slv_dio.dvec = (d_iovec_t) xmalloc(
			    sizeof(struct d_iovec) * RSPC_NTCW, 2, pinned_heap);
			if (!rspcd->slv_dio.dvec) {
				xmfree(rspcd, pinned_heap);
				iomem_det((void *)io);
				simple_unlock(&rspc_lock);
				return((struct d_handle *)DMA_FAIL);
			}
			rspcd->slv_dio.total_iovecs = RSPC_NTCW;
			/*
			 * Initialize the scatter/gather list for this 
			 * channel
			 * NOTE: list is alloced on a page boundary so
			 * the list will be guaranteed to be contiguous
			 */
			rspcd->tcw = (struct rspctcw *) xmalloc( 
				sizeof( struct rspctcw) * RSPC_NTCW, 
				PGSHIFT, pinned_heap);
			if (!rspcd->tcw) {
				xmfree(rspcd->slv_dio.dvec,pinned_heap);
				xmfree(rspcd, pinned_heap);
				iomem_det((void *)io);
				simple_unlock(&rspc_lock);
				return((struct d_handle *)DMA_FAIL);
			}
			rspcd->tcw_real = (char *)
					xmemdma(xmem_global, rspcd->tcw, 
					XMEM_HIDE);
			assert((uint)rspcd->tcw_real != XMEM_FAIL);
			/*
			 * Convert tcw_real to a bus address
			 */
			rspcd->tcw_real = (char *) BUSADDR(rspcd->tcw_real);	

			/*
			 * Allocate page aligned bounce buffer, in case 
			 * scatter/gather is broken
			 */
			rspcd->bounce = (caddr_t)
				xmalloc(PAGESIZE, PGSHIFT, pinned_heap);
			if (!rspcd->bounce) {
				xmfree(rspcd->tcw, pinned_heap);
				xmfree(rspcd->slv_dio.dvec,pinned_heap);
				xmfree(rspcd, pinned_heap);
				iomem_det((void *)io);
				simple_unlock(&rspc_lock);
				return((struct d_handle *)DMA_FAIL);
			}
			rspcd->bounce_real = (char *)
				xmemdma(xmem_global, rspcd->bounce, 
				XMEM_HIDE);
			assert((int) rspcd->bounce != XMEM_FAIL);

			rspcd->b_out_cnt = 0;
		} else {
			/* 
			 * Setup for ISA master
			 */
			GET_CTRL_LOCK(opri, INTMAX, rspcd->ch->ctrl_lock);
			*(io + ch->mode) = MODE(channel, CH_CASCADE);	
			*(uchar *)rspcd->ch->mode_shadow = 
				MODE(channel, CH_CASCADE);
			/*
			 * Defect 177350, force the channel into its default
			 * extended mode register setting.  Channels 0-3 are
			 * 8 bit count by byte while channels 4-7 are 16 bit
			 * count by word.
			 */
			if (channel < 4) emode = EXT_MODE(channel, 
				CH_DEFAULT | CH_8BIT_BYTES);	
			else emode = EXT_MODE(channel, 
				CH_DEFAULT | CH_16BIT_WORDS);
			*(io + ch->ext_mode) = emode;
			*(uchar *)rspcd->ch->ext_mode_shadow = emode;
			REL_CTRL_LOCK(opri, rspcd->ch->ctrl_lock);
			*(io + ch->mask) = UNMASK(channel);  

			/*
			 * Write the High Page Register incase this
			 * DMA controller has support for 32-bit Master
			 * extensions
			 */
			*(io + ch->hipage) = 0x80;  /* MSB for PCI-to-mem */
		} 
		iomem_det((void *)io);
		__iospace_sync();
		RESERVE(channel, isa->dma_chans); /* reserve channel */
		simple_unlock(&rspc_lock);
	} 

	return ((d_handle_t)rspcd);
}

/*
 * NAME: d_map_clear_rspc
 *
 * FUNCTION: This service frees resources allocated on the d_map_init_rspc call 
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called only on the process level.
 *
 * NOTES:
 *
 *	This service should not be called unless a previous allocation has
 *	been made with the d_map_init_rspc call.
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * EXTERNAL PROCEDURES CALLED: 
 *
 * ACCESSED:	indirection through the d_handle structure.  See the
 *		D_MAP_CLEAR macro in dma.h
 */
void 
d_map_clear_rspc(
	d_handle_t handle) 			/* device handle */
{
	struct d_handle_rspc *rspcd = (struct d_handle_rspc *)handle;
	register volatile char *io;
	register int channel;

	ASSERT(handle);
	if (BID_TYPE(rspcd->bid) == IO_ISA) { 
		/*
		 * For ISA devices
		 */
		simple_lock(&rspc_lock);
		channel = handle->id;		/* get channel number   */
		if (AVAILABLE(channel, rspcd->isa->dma_chans)) {
			/*
			 * Not previously allocated
			 */
			simple_unlock(&rspc_lock);
			return;
		}
		FREE(channel, rspcd->isa->dma_chans); /* free the channel */
		io = (volatile char *)iomem_att(&rspcd->isa->iom);
		*(io + rspcd->ch->mask) = MASK(channel);
		iomem_det((void *)io);
		__iospace_sync();
		if (handle->flags & DMA_SLAVE) {
			/*
			 * free the bus list and bounce buffer
			 */
			xmfree(rspcd->slv_dio.dvec, pinned_heap);
			xmfree(rspcd->tcw, pinned_heap);
			xmfree(rspcd->bounce, pinned_heap);
		}
		simple_unlock(&rspc_lock);
	}

	xmfree(rspcd, pinned_heap);
	return;
}

/*
 * NAME: d_map_disable_rspc
 *
 * FUNCTION: This service disables a DMA channel.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *
 *	This service should not be called unless a handle has been previously
 *	allocated with the d_map_init service.
 *
 * RETURN VALUE DESCRIPTION: 
 *			DMA_SUCC - successfully disabled
 *			DMA_FAIL - unable to disable
 *
 * ACCESSED:	indirection through the d_handle structure.  See the
 *		D_MAP_DISABLE macro in dma.h
 */
int 
d_map_disable_rspc(
	d_handle_t	handle)
{
	struct d_handle_rspc *rspcd = (struct d_handle_rspc *)handle;
	register volatile char *io;
	int opri;

	ASSERT(handle);
	if (BID_TYPE(rspcd->bid) == IO_PCI) 
		/*
		 * Don't have a way to disable PCI DMA
		 */
		return(DMA_FAIL);

	io = (volatile char *) iomem_att(&rspcd->isa->iom);

	/*
	 * Stop scatter gather operations
	 */
	GET_CTRL_LOCK(opri, INTMAX, rspcd->ch->ctrl_lock);
	*(io + rspcd->ch->sgd_cmd) = SGC_STOP;
	*(uchar *)rspcd->ch->sgd_cmd_shadow = SGC_STOP;
	REL_CTRL_LOCK(opri, rspcd->ch->ctrl_lock);

	*(io + rspcd->ch->mask) = MASK(handle->id);
	iomem_det((void *)io);
	__iospace_sync();
	return (DMA_SUCC);
}

/*
 * NAME: d_map_enable_rspc
 *
 * FUNCTION: This service enables a DMA channel.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 * NOTES:
 *
 *	This service should not be called unless a handle has been previously
 *	allocated with the d_map_init_rspc service.
 *
 * RETURN VALUE DESCRIPTION: 
 *			DMA_SUCC - successfully enabled
 *			DMA_FAIL - unable to enable
 *
 * ACCESSED:	indirection through the d_handle structure.  See the
 *		D_MAP_ENABLE macro in dma.h
 */
int 
d_map_enable_rspc(
	d_handle_t	handle)
{
	struct d_handle_rspc *rspcd = (struct d_handle_rspc *)handle;
	register volatile char *io;

	ASSERT(handle);
	if (BID_TYPE(rspcd->bid) == IO_PCI) 
		/*
		 * Don't have a way to enable PCI DMA
		 */
		return(DMA_FAIL);

	io = (volatile char *) iomem_att(&rspcd->isa->iom);
	*(io + rspcd->ch->mask) = UNMASK(handle->id);
	iomem_det((void *)io);
	__iospace_sync();
	return (DMA_SUCC);
}


/*
 * NAME: d_map_slave_rspc
 *
 * FUNCTION:
 *	This is the RSPC d_map_slave() implemenation.  This service 
 *	supports the initialization of the DMA channel for a DMA slave 
 *	transfer. This consists of mapping the transfer and initializing 
 *	the system DMA controller to perform the transfer.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *	The memory buffer must remain pinned from the time this service is
 *	called until the DMA transfer is completed and d_unmap_slave_rspc
 *	routine is called
 *
 *	This service should not be called unless the DMA channel has been
 *	allocated with the d_map_init_rspc service.
 *
 *	Note: When DMA_READ flag is specified, the data will be
 *	moved from the device to memory.
 *
 * RETURN VALUE DESCRIPTION: 
 *		DMA_SUCC   - entire transfer mapped successfully
 *		DMA_NORES  - exhausted resources during the mapping.
 *		DMA_NOACC  - bad access permissions to a page in the list
 *		DMA_BAD_MODE   - unsupported mode settings in chan_flags
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * ACCESSED:	indirection through the d_handle structure.  See the
 *		D_MAP_SLAVE macro in dma.h
 */
int
d_map_slave_rspc(
	d_handle_t	handle,	   /* unique handle returned by d_map_init */
	int		flags,	   /* flags field - */
				   /* DMA_READ - xfer from device to memory */
				   /* DMA_WRITE - xfer from memory to device */
				   /* BUS_DMA - xfer from device to device */
				   /* DMA_BYPASS - bypass access checking  */
	int		minxfer,   /* device min transfer (block size      */
	struct dio 	*vlist,	   /* list of buffer addresses and lengths */
	uint		chan_flags) /* device specific flags for the xfer  */	
{
	struct d_handle_rspc *rspcd = (struct d_handle_rspc *)handle;
	register volatile char *io;
	register struct siodchan *ch;
	char	mode, emode;
	int	rc = DMA_SUCC;
	int	i, maxxfer, opri;
	dio_t	blist;
	d_iovec_t	v, b;
	register struct rspctcw *tp;
	int	total_count=0;

	ASSERT(handle);

/*
 * Only allow autoinitialized transfers of a page or less (defect 175861).
 * This is a workaround for a Firecoral bug.  The Firecoral chip does not
 * increment the low page register if a transfer crosses a 16 bit boundary and
 * this restriction guarantees it will never happen.
 */
	if ((chan_flags & CH_AUTOINIT) && ((vlist->dvec->iov_len > PAGESIZE) ||
	    (vlist->used_iovecs > 1))) 
		return(DMA_BAD_MODE);	
	if (WORD_MODE(chan_flags)) {
		/*
		 * If in 16-bit-count-by-word mode, then the starting
		 * address must be 16-bit aligned
		 */
		if ((uint)vlist->dvec->iov_base & 0x1)
			return(DMA_BAD_MODE);	/* not half-word aligned */
		/*
		 * or if the devices minimum transfer size does not
		 * allow multiple of 2 size transfers, or if the virtual
		 * element length is not a multiple of 2
		 */
		for (i=0;i<vlist->used_iovecs;i++) {
			v = &vlist->dvec[i];
			total_count += v->iov_len;
		}	
		if ((minxfer & 0x1) || (total_count & 0x01))
			return(DMA_BAD_MODE);	
		/*
		 * set up the maximum transfer size based on the mode.
		 * The byte/word count register is only 16 bits, so we can 
		 * either do 64K in byte mode or 128K in word mode
		 */
		maxxfer = (1 << 17);
	} else
		maxxfer = (1 << 16);

	/*
	 * Call d_map_list to convert the virtual list into a
	 * list of bus addresses.  NOTE: d_map_list will combine
	 * contiguous requests into a single list entry.
	 */
	rc = d_map_list_rspc(handle, flags, minxfer, vlist, &rspcd->slv_dio);

	if (rc == DMA_NOACC) 
		/*
		 * Encountered an access violation.  d_map_list_rspc
		 * already set up everything in the vlist, so just
		 * return
		 */
		return(rc);

	if (rc == DMA_DIOFULL)  rc = DMA_NORES; /* adjust return code */

	chan_flags |= (flags & DMA_READ) ? RSPC_WRITE : RSPC_READ;

	mode = MODE(handle->id, chan_flags); 
	emode = EXT_MODE(handle->id, chan_flags);

	if (!(chan_flags & CH_AUTOINIT)) {

		uint	word;

		/*
		 * if we can do scatter/gather and this isn't 
		 * requesting autoinitialization, then
		 * Set up Scatter/Gather Descriptor Table (SGDT)
		 */

		blist = &rspcd->slv_dio;
		ch = rspcd->ch;

		if (WORD_MODE(chan_flags)) {
		   /* 
		    * Reset maxxfer value since we have 24 bit count registers
		    * in the SGDT.  Since this is count by word it gets shifted
		    * another bit as well.
		    */
		   maxxfer = (1 << 25);
		   for (i=0;i<blist->used_iovecs;i++) {
			/*
			 * for each entry in the bus list, 
			 * NOTE: since we initialize the slv_dio iovec array to
			 * have RSPC_NTCW elements just like our scatter/gather 
			 * list so we don't have to worry about overrunning it
			 */
			b = &blist->dvec[i];	/* initialize bus pointer */
			tp = &rspcd->tcw[i];    /* initialize tcw pointer */
			/*
		 	 * low 16 bits of address must be shifted right one
			 * bit for word mode
			 */
			word = (((uint)b->iov_base & 0xFFFF0000) | 
				(((uint)b->iov_base >> 1) & 0xFFFF));
			stbrx(word, &tp->phys);	        /* byte reverse addr */
			assert(b->iov_len <= maxxfer);  /* just to make sure */
			word = (b->iov_len >> 1) - 1;	/* divide length by 2*/
			stbrx(word, &tp->bcount); 	/* reverse count */
		   }
		}
		else {
		   /* 
		    * Reset maxxfer value since we have 24 bit count registers
		    * in the SGDT.
		    */
		   maxxfer = (1 << 24);
		   for (i=0;i<blist->used_iovecs;i++) {
			/*
			 * for each entry in the bus list, 
			 * NOTE: since we initialize the slv_dio iovec array to
			 * have RSPC_NTCW elements just like our scatter/gather 
			 * list so we don't have to worry about overrunning it
			 */
			b = &blist->dvec[i];	/* initialize bus pointer */
			tp = &rspcd->tcw[i];    /* initialize tcw pointer */
			stbrx((int) b->iov_base, 
					&tp->phys);	/* byte reverse addr */
			assert(b->iov_len <= maxxfer);  /* just to make sure */
			stbrx((b->iov_len - 1), &tp->bcount); /* rev count */
		   }
		}
		/*
		 * Now set EOL
		 * NOTE: the bytes have already been swapped, 
		 * 	 so EOL is 0x00000080
		 */
		rspcd->tcw[i-1].bcount |= 0x00000080;
	
		io = (volatile char *) iomem_att(&rspcd->isa->iom);
		/*
		 * SETUP MODE REGISTERS
		 */
		GET_CTRL_LOCK(opri, INTMAX, rspcd->ch->ctrl_lock);
		*(io + ch->mode) = mode;	
		*(uchar *)rspcd->ch->mode_shadow = mode;
		*(io + ch->ext_mode) = emode;	
		*(uchar *)rspcd->ch->ext_mode_shadow = emode;

		/*
		 * program scatter/gather table pointer
		 * NOTE: the 32-bit address is stored byte-reversed
		 */
		stbrx((int)rspcd->tcw_real, (void *)(io + ch->sgd_table));	

		/*
		 * Issue the start scatter/gather command, specifying that
		 * EOP should be driven when terminal count is detected
		 */
		*(io + ch->sgd_cmd) = (SGC_START | SGC_EOP);	
		*(uchar *)rspcd->ch->sgd_cmd_shadow = (SGC_START | SGC_EOP);
		__iospace_eieio();
		REL_CTRL_LOCK(opri, rspcd->ch->ctrl_lock);

		/*
		 * Now enable the channel
		 */
		*(io + ch->mask) = UNMASK(handle->id);
		__iospace_sync();
		iomem_det((void *)io);


		return(rc);
	} else {
		ulong	phys;
		int	byte_count;
		union {
			ulong	lword;		/* item as a long word	*/
			uchar	bytes[4];	/* item as bytes	*/
		} val;
		int	bounce = FALSE;
		uint	opri;

		/*
		 * We can handle at most one page (or portion thereof)
		 * at a time....unless of course, multiple pages are
		 * contiguous
		 */

		blist = &rspcd->slv_dio;
		if (blist->used_iovecs > 1)
			/*
			 * The bus list tells us that there are multiple 
			 * entries, which means non-contiguous buffers, which, 
			 * by definition means we'll have a residual since we 
			 * can't do scatter/gather
			 * NOTE: The exception is if we're able to use the 
			 * bounce buffer to handle the first 2 blist entries.  
			 * This is corrected in the "cleanup" section later.
			 */
			rc = DMA_NORES;
		byte_count = blist->dvec->iov_len;
		if (byte_count < minxfer) {
			/*
			 * The first bus entry doesn't meet our minimum transfer
			 * requirements, so grab as much as we can up to a page 
			 * worth and use the bounce buffer
			 */
			if (vlist->dvec->iov_len < minxfer) {
				/*
				 * if the first virtual buffer isn't even as
				 * big as our minimum xfer size...then we're out
				 * of luck...we can't risk the next buffer being
				 * in a different address space.
				 */
				vlist->bytes_done = 0;
				vlist->resid_iov = 0;
				return(DMA_NORES);
			} else {
				/*
				 * prepare to bounce
				 */
				bounce = TRUE;
				byte_count = (vlist->dvec->iov_len <= PAGESIZE) 
					? vlist->dvec->iov_len : PAGESIZE;
			}
		}
		if ((WORD_MODE(chan_flags)) && (byte_count % 2)) {
			/*
			 * in word mode, the transfer length must be a
			 * multiple of 2
			 */
			rc = DMA_NORES;
			byte_count--; 
		}
		if (byte_count % minxfer) {
			/*
			 * if the count is not a multiple of the minimum
			 * transfer size, then we need to adjust it
			 */
			rc = DMA_NORES;
			byte_count = (byte_count / minxfer) * minxfer;
		}
		if (byte_count > maxxfer) {
			/*
			 * if the count is greater than what we can
			 * program, then we need to adjust it
			 */
			rc = DMA_NORES;
			byte_count = (maxxfer / minxfer) * minxfer;
		}
		if (bounce) {
			if (flags & DMA_READ) {
				/*
				 * prepare for copy-out
				 */
				rspcd->b_out_addr = vlist->dvec->iov_base;
				rspcd->b_out_cnt = byte_count;
				rspcd->xm_out = vlist->dvec->xmp;
			} else {
				/*
				 * DMA_WRITE....do copyin now
				 */
				rc = xmemin(vlist->dvec->iov_base, 
				   rspcd->bounce, byte_count, vlist->dvec->xmp);
				ASSERT(rc == 0);
			}
			phys = BUSADDR(rspcd->bounce_real);
		} else {
			phys = (ulong) blist->dvec->iov_base;
		}
	
		vlist->bytes_done = byte_count;
		if (rc) {
			/*
			 * Here we cleanup after a DMA_DIOFULL or DMA_NORES
			 * find out which iovec we were processing when we 
			 * ran out
			 */
			byte_count = 0;
			for(i=0;i<vlist->used_iovecs;i++) {
				v = &vlist->dvec[i];
				byte_count += v->iov_len;
				if (byte_count > vlist->bytes_done)	break;
			}
			vlist->resid_iov = i;	
			if (byte_count == vlist->bytes_done) 
				/*
				 * We we're able to do it 
				 * all anyway (bounce buffer)
				 */
				rc = DMA_SUCC;
			else
				rc = DMA_NORES;
		}
		if (vlist->bytes_done == 0) return(rc);
	
		/*
		 * Now ready to program controller
		 */
	
	
		ch = rspcd->ch;
		io = (volatile char *) iomem_att(&rspcd->isa->iom);
	
		/*
		 * mask DMA on this channel
		 */
		*(io + ch->mask) = MASK(handle->id);
		__iospace_sync();
	
		/*
		 * Disable interrupts to serialize access to the clear byte
		 * pointer register.  There is only one register per controller
		 */
		GET_CTRL_LOCK(opri, INTMAX, rspcd->ch->ctrl_lock);
	
		/*
		 * SETUP ADDRESS GENERATION HARDWARE
		 * NOTE: order is very important for writing the clear byte 
		 * 	 pointer and the base address register, the sequence 
		 *	 must be :
		 * 	1. clear byte pointer (clears latch used to 
		 *			       address 16-bit address and 
		 *			       count registers)
		 *	2. write low-byte (bits 24-31) of address
		 *	3. write low-middle-byte (bits 16-23) of address
		 */
		/*
		 * for count-by-words mode, the baseaddr reg must be programmed
		 * to the address shifted right 1
		 */
		val.lword = (WORD_MODE(chan_flags)) ? (phys >> 1) : phys; 
		*(io + ch->clear_byte_ptr) = 0x01;	/* any data will do */
		__iospace_eieio();
		*(io + ch->baseaddr) = val.bytes[3];	/* 24-31 of address */
		__iospace_eieio();
		*(io + ch->baseaddr) = val.bytes[2];	/* 16-23 of address */
		__iospace_eieio();
		val.lword = phys;
		*(io + ch->lopage) = val.bytes[1];	/* 8-15 of address  */
		*(io + ch->hipage) = val.bytes[0];	/* 0-7 of address   */

		/*
		 * SETUP TRANSFER SIZE
		 * NOTE: order is very important for writing the clear byte 
		 *	 pointer and the byte/word count register, the sequence 
		 *	 must be
		 * 	1. clear byte pointer (clears latch used to address 
		 *			       16-bit address and count regs)
		 *	2. write low-byte (bits 24-31) of count
		 *	3. write low-middle-byte (bits 16-23) of count
		 */
		val.lword = (WORD_MODE(chan_flags)) ? 
			((vlist->bytes_done >> 1) - 1) : vlist->bytes_done - 1; 

		*(io + ch->clear_byte_ptr) = 0x02;	/* any data will do */
		__iospace_eieio();
		*(io + ch->count) = val.bytes[3];	/* 24-31 of count */
		__iospace_eieio();
		*(io + ch->count) = val.bytes[2];	/* 16-23 of count */
		__iospace_eieio();

		/*
		 * SETUP MODE REGISTERS
		 */
		*(io + ch->mode) = mode;	
		*(uchar *)rspcd->ch->mode_shadow = mode;
		*(io + ch->ext_mode) = emode;	
		*(uchar *)rspcd->ch->ext_mode_shadow = emode;
		__iospace_eieio();
	
		/*
		 * Re-enable interrupts
		 */
		REL_CTRL_LOCK(opri, rspcd->ch->ctrl_lock);

		/*
		 * Now enable the channel
		 */
		*(io + ch->mask) = UNMASK(handle->id);
		__iospace_sync();

		iomem_det((void *)io);

		return(rc);
	}
}


/*
 * NAME: d_map_page_rspc
 *
 * FUNCTION:
 *	This is the RSPC d_map_page() implemenation.  This service 
 *	processes a single virtual address and provides a resulting
 *	bus address.	
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *	This service should not be called unless a handle has been
 *	allocated with the d_map_init_rspc service.
 *
 *	Note: When DMA_READ flag is specified, the data will be
 *	moved from the device to memory.
 *
 * RETURN VALUE DESCRIPTION: 
 *		DMA_SUCC   - entire transfer mapped successfully
 *		DMA_NORES  - exhausted resources during the mapping.
 *		DMA_NOACC  - bad access permissions to a page in the list
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * ACCESSED:	indirection through the d_handle structure.  See the
 *		D_MAP_LIST macro in dma.h
 */
int
d_map_page_rspc(
	d_handle_t	handle,	   /* unique handle returned by d_map_init */
	int		flags,	   /* flags field - */
				   /* DMA_READ - xfer from device to memory */
				   /* DMA_WRITE - xfer from memory to device */
				   /* BUS_DMA - xfer from device to device */
				   /* DMA_BYPASS - bypass access checking  */
	caddr_t		baddr,	   /* virtual address */
	uint		*busaddr,  /* address of bus address */
	struct xmem	*xmp)  	   /* cross memory descriptor */
{

	ASSERT(handle); 

	if (!(flags & BUS_DMA)) {
		if ((((uint)baddr >= (uint)rheap->vaddr ) &&
		    ((uint)(baddr) < ((uint)rheap->vaddr + rheap->size))) &&
		    (xmp->aspace_id == XMEM_GLOBAL)) {
			/*
			 * REQUEST OF REAL_HEAP PAGES
			 * fast path for buffers from the real_heap
			 */
			*busaddr = BUSADDR(((uint)(rheap->raddr + 
					((uint)baddr -(uint)rheap->vaddr))));
		} else {
			/*
		 	 * NORMAL VIRTUAL ADDRESS
			 */
			*busaddr = xmemdma(xmp, baddr, XMEM_HIDE |
			       ((flags & DMA_BYPASS)? 0 : (XMEM_ACC_CHK |
			       ((flags & DMA_READ)? 0 : XMEM_WRITE_ONLY))));
	
			if((uint)*busaddr == XMEM_FAIL) 
				 /*
				  * NOACC case
				  */
				  return(DMA_NOACC);
			*busaddr = BUSADDR(*busaddr); /* convert to bus addr */
		}
	} else {
		/*
		 * BUS TO BUS
		 */

		struct d_handle_rspc *rspcd = (struct d_handle_rspc *)handle;
		
        	if ((BID_TYPE(rspcd->bid) == IO_ISA) && 
		    !(handle->flags & DMA_SLAVE))
			/*
			 * We don't support an ISA Master wanting to do 
			 * BUS to BUS
			 */
			return(DMA_FAIL);

		/*
		 * for BUS to BUS, don't modify anything
		 */
		*busaddr = (uint)baddr;
	}
	return(DMA_SUCC);
}

/*
 * NAME: d_map_list_rspc
 *
 * FUNCTION:
 *	This is the RSPC d_map_list() implemenation.  This service 
 *	processes a list of virtual addresses and provides a resulting
 *	list of bus addresses.	
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *	This service should not be called unless a handle has been
 *	allocated with the d_map_init_rspc service.
 *
 *	Note: When DMA_READ flag is specified, the data will be
 *	moved from the device to memory.
 *
 * RETURN VALUE DESCRIPTION: 
 *		DMA_SUCC   - entire transfer mapped successfully
 *		DMA_NORES  - exhausted resources during the mapping.
 *		DMA_NOACC  - bad access permissions to a page in the list
 *		DMA_DIOFULL  - exhausted space in the bus list
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * ACCESSED:	indirection through the d_handle structure.  See the
 *		D_MAP_LIST macro in dma.h
 */
int
d_map_list_rspc(
	d_handle_t	handle,	   /* unique handle returned by d_map_init */
	int		flags,	   /* flags field - */
				   /* DMA_READ - xfer from device to memory */
				   /* DMA_WRITE - xfer from memory to device */
				   /* BUS_DMA - xfer from device to device */
				   /* DMA_BYPASS - bypass access checking  */
	int		minxfer,   /* device min transfer (block size      */
	struct dio 	*vlist,	   /* list of virtual addresses and lengths */
	struct dio 	*blist)	   /* list of bus addresses and lengths    */
{
	register d_iovec_t v;	   /* virtual iovec pointer */ 
	register d_iovec_t b;	   /* bus iovec pointer */ 
	int	rc = DMA_SUCC;
	int	i;
	caddr_t baddr;
	caddr_t phys;
	int	byte_count, count;
	int	num_pages, page_no, bytes_last_page;
	int	xmflags;
	struct xmem *xmp;
	caddr_t next_contig = (char *)-1;

	ASSERT(handle); 

	vlist->bytes_done = 0;
	blist->used_iovecs = -1;	/* initialize used count */
	for (i=0;i<vlist->used_iovecs;i++) {
	    /*
	     * for each iovec entry
	     */
	    v = &vlist->dvec[i];
	    baddr = v->iov_base;
	    count = v->iov_len;
	    xmp   = v->xmp;
	    if (!(flags & BUS_DMA)) {
		if ((((uint)baddr >= (uint)rheap->vaddr ) &&
		    ((uint)(baddr+count) < ((uint)rheap->vaddr + rheap->size))) &&
		    (xmp->aspace_id == XMEM_GLOBAL)) {
			/*
			 * REQUEST OF REAL_HEAP PAGES
			 * fast path for buffers from the real_heap
			 */
			phys = (caddr_t) BUSADDR(((uint)(rheap->raddr + 
					((uint)baddr -(uint)rheap->vaddr))));
			if (phys != next_contig) {
				/* 
				 * if this next entry is not contiguous 
				 * with the last, then we need the next bus 
				 * list entry
	 			 */
		    		if((blist->used_iovecs + 1) >= 
							blist->total_iovecs){
					/*
					 * bus list is full
					 */
					rc = DMA_DIOFULL;
					break;
				}
				/*
				 * setup for new bus iovec pointer
				 */
				b = &blist->dvec[++blist->used_iovecs];
				b->iov_base = phys;
				b->iov_len = count;
			} else
				/*
				 * add to the last one
				 */
				b->iov_len += count;
			next_contig = phys + count; 
/*
 * Remove the combining of contiguous real pages (defect 175861) if this would
 * cause a single transfer to cross a 64K boundary.  This is a workaround for
 * a Firecoral bug.  The Firecoral chip does not increment the low page
 * register if a transfer crosses a 16 bit boundary.
 */
			if (!((uint)next_contig & 0xFFFF)) 
			    next_contig=(char *)-1;
	   		/*
			 * increment what we've just mapped
			 */
			 vlist->bytes_done += count;
		} else { 
			/*
			 * NORMAL REQUEST OF VIRTUAL PAGES
			 * compute number of pages touched by this list
			 * entry
			 */
			num_pages = ((uint)(baddr + count - 1) >> PGSHIFT) -
					((uint)baddr >> PGSHIFT) + 1;
			page_no = 0;
			xmflags = XMEM_HIDE | 
			       ((flags & DMA_BYPASS)? 0 : (XMEM_ACC_CHK |
			       ((flags & DMA_READ)? 0 : XMEM_WRITE_ONLY)));
			bytes_last_page = ((uint)baddr+count) & (PAGESIZE - 1);
			if (bytes_last_page == 0) bytes_last_page = PAGESIZE;
			while (page_no < num_pages) {
			      /*
			       * for each page
			       */
			      phys = (caddr_t) xmemdma(xmp, baddr, xmflags);
	
			      if((uint)phys == XMEM_FAIL) {
				 /*
				  * NOACC case
				  * the bytes_done field is already set to
				  * number of bytes already processed
				  */
				  vlist->resid_iov = i;
				  return(DMA_NOACC);
			      }
			      if ((page_no == 0) && (num_pages == 1))
					/*
					 * This is the only page, so set
					 * byte count to the number in this
					 * request.
					 */
					byte_count = count; 
			      else if (page_no == 0)
					/*
					 * set count to bytes in first page
					 */
					byte_count = PAGESIZE - 
						((uint)baddr & (PAGESIZE-1));
			      else if (page_no == (num_pages - 1))
					/*
					 * set count to bytes in last page
					 */
					byte_count = bytes_last_page;
			      else
					byte_count = PAGESIZE;
			      if (phys != next_contig) {
				/* 
				 * if this next entry is not contiguous 
				 * with the last, then we need the next bus 
				 * list entry
		 		 */
			    	if((blist->used_iovecs + 1)  >= 
							blist->total_iovecs){
					/*
					 * bus list is full
					 */
					rc = DMA_DIOFULL;
					break;
				}
				/*
				 * setup for new bus iovec pointer
				 */
				b = &blist->dvec[++blist->used_iovecs];
				b->iov_base = (caddr_t) BUSADDR((ulong)phys);
				b->iov_len = byte_count;
			      } else
				/*
				 * add to the last one
				 */
				b->iov_len += byte_count;
	    		      next_contig = phys + byte_count;
/*
 * Remove the combining of contiguous real pages (defect 175861) if this would
 * cause a single transfer to cross a 64K boundary.  This is a workaround for
 * a Firecoral bug.  The Firecoral chip does not increment the low page
 * register if a transfer crosses a 16 bit boundary.
 */
			      if (!((uint)next_contig & 0xFFFF)) 
				  next_contig=(char *)-1;
		 	      vlist->bytes_done += byte_count;
			      page_no++;
			      baddr += byte_count;
			}
			if (rc) break;
		}
	   } else {
		/*
		 * BUS TO BUS
		 */
		struct d_handle_rspc *rspcd = (struct d_handle_rspc *)handle;

        	if ((BID_TYPE(rspcd->bid) == IO_ISA) && 
		    !(handle->flags & DMA_SLAVE))
			/*
			 * We don't support an ISA Master doing BUS to BUS DMA
			 */
			return(DMA_FAIL);
		/*
		 * BUS to BUS DMA....don't touch address
		 */
		if (baddr != next_contig) {
			/* 
			 * if this next entry is not contiguous 
			 * with the last, then we need the next bus 
			 * list entry
	 		 */
		    	if((blist->used_iovecs + 1) >= blist->total_iovecs){
				/*
				 * bus list is full
				 */
				rc = DMA_DIOFULL;
				break;
			}
			/*
			 * setup for new bus iovec pointer
			 */
			b = &blist->dvec[++blist->used_iovecs];
			b->iov_base = baddr;
			b->iov_len = count;
		} else
			/*
			 * add to the last one
			 */
			b->iov_len += count;
		next_contig = baddr + count; 
	   	/*
		 * increment what we've just mapped
		 */
		 vlist->bytes_done += count;
	   } /* else was a bus to bus */
	} /* for loop of virtual iovec entries */

	/*
	 * When we get here, the used count for the bus list will always
	 * be 1 less than it should, so increment it here
	 */
	blist->used_iovecs++;
	blist->bytes_done = vlist->bytes_done;

	if (rc == DMA_SUCC) return(rc);
	/*
	 * Here we cleanup after a DMA_DIOFULL
	 * ...make the bytes done a multiple of the device minimum transfer
	 */
	vlist->bytes_done = (vlist->bytes_done / minxfer) * minxfer;
	blist->bytes_done = vlist->bytes_done;
	/*
	 * now find out which iovec we were processing when we ran out
	 */
	byte_count = 0;
	for(i=0;i<vlist->used_iovecs;i++) {
		v = &vlist->dvec[i];
		byte_count += v->iov_len;
		if (byte_count > vlist->bytes_done)	break;
	}
	vlist->resid_iov = i;	
	return(rc);
}



/*
 * NAME: d_unmap_slave_rspc
 *
 * FUNCTION: This service is called upon completion of a slave DMA transfer.
 *	    It will report any errors.  
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *
 *	This service should not be called unless a handle has been
 *	allocated with the d_map_init service.
 *
 * RETURN VALUE DESCRIPTION: DMA_SUCC		- successful completion
 *			     DMA_TC_NOTREACHED  - terminal count not reached
 *			     DMA_FAIL		- failure
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * ACCESSED:	indirection through the d_handle structure.  See the
 *		D_UNMAP_SLAVE macro in dma.h
 */
int
d_unmap_slave_rspc(
	d_handle_t	handle)	   /* unique handle returned by d_map_init */
{
	struct d_handle_rspc *rspcd = (struct d_handle_rspc *)handle;
	register volatile char *io;
	char	status;
	int	rc = DMA_SUCC;
	uint	opri;


	ASSERT(handle); 


	if (rspcd->b_out_cnt) {
		/*
		 * if there is data to bounce out, copy it out to
		 * the buffer
		 */
		rc = xmemout(rspcd->bounce, rspcd->b_out_addr, 
				rspcd->b_out_cnt, rspcd->xm_out);
		ASSERT(rc == 0);
		/*
		 * clear the bounce out count
		 */
		rspcd->b_out_cnt = 0;
	}

	/*
	 * Read the channel status register and manage the
	 * shadow copy, serializing access since 4 channels 
	 * are represented in this 1 register.
	 */
	io = (volatile char *) iomem_att(&rspcd->isa->iom);
	GET_CTRL_LOCK(opri, INTMAX, rspcd->ch->ctrl_lock);
	/*
	 * Read the hardware status register, this read causes the contents
	 * to be reset, so we must save the state of the other 3 channels
	 * on this controller 
	 */
	status = *(io + rspcd->ch->status);	
	*(uchar *)rspcd->ch->status_shadow |= status & 0xF; /* OR into shadow */
	status = *(uchar *)rspcd->ch->status_shadow;
	/*
	 * Now clear this channel's bit in the shadow copy 
	 */ 
	*(uchar *)rspcd->ch->status_shadow &= 0xFF ^ (1 << (handle->id & 0x3)); 
	REL_CTRL_LOCK(opri, rspcd->ch->ctrl_lock);
	iomem_det((void *)io);

	if (TC_REACHED(handle->id, status))
		/*
		 * terminal count reached, so return success
		 */
		return(DMA_SUCC);
	else 
		return(DMA_TC_NOTREACHED);
}

/*
 * NAME: rspcios_pm_handler
 *
 * FUNCTION:
 *	This is the power management (PM) entry point for the RSPC DMA
 *	services.  It is called by the PM core to handle all PM chores
 *	except initialization.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *	This service should not be called except by the PM core.
 *
 * RETURN VALUE DESCRIPTION: 
 *		PM_SUCCESS - requested PM transition successful
 *		PM_ERROR - unable to successfully complete PM transition
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * ACCESSED:	called by the PM core to perform some desired PM function.
 */
int
rspcios_pm_handler (
	caddr_t		private,   /* private data area */
	int		ctrl)	   /* PM control word */
{
	int	rc = PM_SUCCESS, opri;
	struct pm_handle *pmh;
	struct isa_dma *isa;

	GET_CTRL_LOCK(opri, INTMAX, &rspc_lock);
	isa = (struct isa_dma *)private;
	pmh = isa->pmhandle;
	switch(ctrl) {
		case PM_DEVICE_FULL_ON:
			if (pmh->mode != PM_DEVICE_ENABLE) {
				rc = PM_ERROR;
			}
			else
				pmh->mode = ctrl;
			break;
		case PM_DEVICE_ENABLE:
			switch(pmh->mode) {
				case PM_DEVICE_SUSPEND:
				case PM_DEVICE_HIBERNATION:
					rspcios_pm_register_restore(private);
				case PM_DEVICE_FULL_ON:
				case PM_DEVICE_ENABLE:
					pmh->mode = ctrl;
					break;
				default:
					rc = PM_ERROR;
					break;
			}
			break;
		case PM_DEVICE_HIBERNATION:
			if ((pmh->mode != PM_DEVICE_ENABLE) && 
			    (pmh->mode != PM_DEVICE_SUSPEND)) {
				rc = PM_ERROR;
			}
			else {
				pmh->mode = ctrl;
				rspcios_pm_register_save(private);
			}
			break;
		case PM_DEVICE_SUSPEND:
			if (pmh->mode != PM_DEVICE_ENABLE) {
				rc = PM_ERROR;
			}
			else {
				pmh->mode = ctrl;
				rspcios_pm_register_save(private);
			}
			break;
		/*
		 * Do nothing on FREEZE/UNFREEZE notices
		 */
		case PM_PAGE_UNFREEZE_NOTICE:
		case PM_PAGE_FREEZE_NOTICE:
			break;
		default:
			rc = PM_ERROR;
			break;
	}
	REL_CTRL_LOCK(opri, &rspc_lock);
	return(rc);
}

/*
 * NAME: rspcios_pm_register_save
 *
 * FUNCTION:
 *	This is the power management (PM) register save function for the
 *	RSPC DMA services.  It is called by the PM handler to save all relevant
 *	registers
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *	This service should not be called except from the rspcios PM handler.
 *
 * RETURN VALUE DESCRIPTION: 
 *		None
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * ACCESSED:	called by the PM handler to save DMA registers
 */
void
rspcios_pm_register_save (
	caddr_t		private)   /* private data area */
{
	int	opri, i;
	struct isa_dma *isa;
	register volatile char *io;
	struct siodchan *ch;
	struct per_channel *save;
	struct per_controller *save1;

	isa = (struct isa_dma *)private;
	io = (volatile char *)iomem_att(&isa->iom);

	/* Save registers */
	for (i=0;i<RSPC_NCHAN;i++) {
		ch = &isa->chan_info[i];
		save = &isa->dma_save->perchan[i];

		/*
		 * Order is important for the pieces of the base address
		 * and count registers
		 */

		*(io + ch->clear_byte_ptr) = 0x01;
		__iospace_eieio();
		save->addr.byte[1] = *(io + ch->baseaddr);
		__iospace_eieio();
		save->addr.byte[0] = *(io + ch->baseaddr);
		__iospace_eieio();

		save->lopage = *(io + ch->lopage);
		save->hipage = *(io + ch->hipage);

		*(io + ch->clear_byte_ptr) = 0x01;
		__iospace_eieio();
		save->count.byte[1] = *(io + ch->count);
		__iospace_eieio();
		save->count.byte[0] = *(io + ch->count);
		__iospace_eieio();

		save->sgd_table_ptr = *(io + ch->sgd_table);

		save->sgd_cmd = *((uchar *)(ch->sgd_cmd_shadow));

		if ((i == 0) || (i == 4)) {
			save1 = &isa->dma_save->percont[i/4];
			save1->all_mask = *(io + ch->all_mask);
			save1->ext_mode = *((uchar *)(ch->ext_mode_shadow));
			save1->mode = *((uchar *)(ch->mode_shadow));
		}
	}
	iomem_det((void *)io);
}

/*
 * NAME: rspcios_pm_register_restore
 *
 * FUNCTION:
 *	This is the power management (PM) register restore function for the
 *	RSPC DMA services.  It is called by the PM handler to restore all
 *	relevant registers
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *	This service should not be called except from the rspcios PM handler.
 *
 * RETURN VALUE DESCRIPTION: 
 *		None
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * ACCESSED:	called by the PM handler to restore DMA registers
 */
void
rspcios_pm_register_restore (
	caddr_t		private)   /* private data area */
{
	int	opri, i;
	struct isa_dma *isa;
	register volatile char *io;
	register struct siodchan *ch;
	struct per_channel *save;
	struct per_controller *save1;
	struct rspcsio *sio;

	isa = (struct isa_dma *)private;

	/*
	 * Defect 178189.  This code is lifted from rspcios_init and uses the
	 * rspcsio structure for its mapping.
	 */
	sio = (struct rspcsio *)iomem_att(&isa->iom);
	sio->dma2_mclear = 1;	/* reset controller 2 */
	__iospace_eieio();	/* order this */
	sio->dma1_mclear = 1;	/* reset controller 1 */
	__iospace_sync();	/* order/sync this */
	iomem_det((void *)sio);

	io = (volatile char *)iomem_att(&isa->iom);
	/* Restore registers */
	for (i=0;i<RSPC_NCHAN;i++) {
		ch = &isa->chan_info[i];
		save = &isa->dma_save->perchan[i];

		/*
		 * Order is important for the pieces of the base address
		 * and count registers
		 */

		*(io + ch->clear_byte_ptr) = 0x01;
		__iospace_eieio();
		*(io + ch->baseaddr) = save->addr.byte[1];
		__iospace_eieio();
		*(io + ch->baseaddr) = save->addr.byte[0];
		__iospace_eieio();

		*(io + ch->lopage) = save->lopage;
		*(io + ch->hipage) = save->hipage;

		*(io + ch->clear_byte_ptr) = 0x01;
		__iospace_eieio();
		*(io + ch->count) = save->count.byte[1];
		__iospace_eieio();
		*(io + ch->count) = save->count.byte[0];
		__iospace_eieio();

		*(io + ch->sgd_table) = save->sgd_table_ptr;

		*(io + ch->sgd_cmd) = save->sgd_cmd;
		*((uchar *)(ch->sgd_cmd_shadow)) = save->sgd_cmd;

		if ((i == 0) || (i == 4)) {
			save1 = &isa->dma_save->percont[i/4];

			*(io + ch->all_mask) = save1->all_mask;

			*(io + ch->ext_mode) = save1->ext_mode;
			*((uchar *)(ch->ext_mode_shadow)) = save1->ext_mode;
			*(io + ch->mode) = save1->mode;
			*((uchar *)(ch->mode_shadow)) = save1->mode;
		}
		__iospace_sync();
	}
	iomem_det((void *)io);
}
#endif /* _RSPC */

