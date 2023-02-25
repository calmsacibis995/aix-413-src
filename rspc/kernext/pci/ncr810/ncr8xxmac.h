/* @(#)43	1.4  src/rspc/kernext/pci/ncr810/ncr8xxmac.h, pciscsi, rspc41J, 9516A_all 4/18/95 11:20:12 */

/*
 *   COMPONENT_NAME: PCISCSI
 *
 *   FUNCTIONS: ADP_SCRIPT
 *		AtoCBO
 *		AtoD
 *		BUILD_TIM
 *		CBOtoA
 *		CBOtoL
 *		CBOtoS
 *		DEVH_PATCH
 *		DEV_SCRIPT
 *		DtoA
 *		GET_STATUS
 *		INDEX
 *		IN_BYTE
 *		IN_SHORT
 *		IN_WORD
 *		IOWAIT_INT_OFF
 *		IOWAIT_INT_ON
 *		LIGHT_OFF
 *		LIGHT_ON
 *		LUN
 *		LtoCBO
 *		OUT_SHORT
 *		OUT_WORD
 *		P8else
 *		P8if
 *		P8printf
 *		RESTART_CHIP
 *		SAVE_CHIP_STATE
 *		SID
 *		SUSPEND_CHIP
 *		SYNC
 *		S_COUNT
 *		S_INDEX
 *		StoCBO
 *		W_START
 *		W_STOP
 *		XFER_PRD
 *		XFER_VAL
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

#ifndef _H_NCR8XXMAC
#define _H_NCR8XXMAC

/************************************************************************/
/* MACROS								*/
/************************************************************************/


/*
 * Our DEBUG macros
 */

/* Debug mask values are as follows: */
#define	P8_ENTRY		0x00000001	/* function entry */
#define	P8_EXIT			0x00000002	/* function exit */
#define	P8_ENTRY_EXIT		0x00000003
#define	P8_READ_REG		0x00000004	/* reading of chip regs */
#define	P8_WRITE_REG		0x00000008	/* writing of chip regs */
#define	P8_READ_WRITE_REG	0x0000000A
#define	P8_DEVICE_STATUS_1	0x00000010	/* "before" changes */
#define	P8_DEVICE_STATUS_2	0x00000020	/* "after" changes */
#define	P8_DEVICE_STATUS	0x00000030
#define	P8_DUMP_SCRIPTS_1	0x00000040	/* creation/initialization */
#define	P8_DUMP_SCRIPTS_2	0x00000080	/* patching */
#define	P8_DUMP_SCRIPTS		0x000000C0
#define	P8_INFO_1		0x00000100	/* control flow */
#define	P8_INFO_2		0x00000200	/* data */
#define	P8_INFO_3		0x00000400	/* chip status & tim print */
#define	P8_INFO_4		0x00000800	/* some script patching */
#define P8_INFO			0x00000F00
#define	P8_BREAK		0x00001000	/* enable general break points */
#define	P8_ASSERT		0x00002000	/* enable some bad asserts */
#define	P8_SYNC			0x00004000	/* disp sync negotiation rslts */
#define	P8_ABORT		0x00008000	/* various abort/BDR/reset &
						   some serious errors */
#define P8_SEL_TMO		0x00010000	/* Selection timeout stuff */
#define P8_UNEXP_BF             0x00020000      /* Unexpected bus free error */

#define P8_PRTCNT		0x00080000	/* Print counters */
#define	P8_STD_DBMK		0x0001ff3f	/* our standard debug flags mask
						   for lots of noise */
#define	P8_FLW_DBMK		0x00000103	/* entry, exit flow debug */
#define	P8_ALL_DBMK		0x00ffffff	/* All of the debug mask flags */
#define	P8XX_SVMAX		20		/* size of saved debug flags
						   array */

/*
 * Define some values for debugging function executions
 */
#define	P8_CONFIG		0x80000000	/* debug p8xx_config() */
#define	P8_OPEN			0x40000000	/* debug p8xx_open() */
#define	P8_START		0x20000000	/* debug p8xx_start() */
#define	P8_START_DEV		0x10000000	/* debug p8xx_start_dev() */
#define	P8_STRATEGY		0x08000000	/* debug p8xx_strategy() */
#define	P8_INTR			0x04000000	/* debug p8xx_intr() */
#define	P8_IOCTL		0x02000000	/* debug p8xx_ioctl() */

/* this macro reset the n th bit in the long word passed                */
#define ALLOC(lw,n)     lw = lw &~(1<<(31-(n)));

/* this macro set the n th bit in the long word passed                  */
#define FREE(lw,n)      lw = lw | (1<<(31-(n)));

/* this macro checks the n th bit in the long word passed               */
#define IN_USE(lw,n)    (~(lw) & (1<<(31-(n))))

/* FUNCTION:  Frees a command tag                                         */
/* EXECUTION ENVIRONMENT:  Interrupts should be disabled.                 */
/* NOTES:  This routine frees a command tag for later use.                */
#define p8xx_FREE_TAG(com_ptr)                                             \
        {                                                                  \
                TRACE_2 ("freeTAG", (int) (com_ptr)->tag, (int) (com_ptr)) \
                (com_ptr)->in_use = FALSE;                                 \
                FREE(ap->TAG_alloc[(com_ptr)->tag / PSC_WORDSIZE],     \
                     (com_ptr)->tag % PSC_WORDSIZE)                        \
        }

/*
 * Name:        BUILD_TIM
 * Function:    Builds the "Table Indirect Move" table for the 53C8XX
 *              scatter/gather function.
 * Input:       ads_t *ap       addr of adapter_def struct
 *              cmd_t *com_ptr  addr of command info struct
 *              struct buf *bp  addr of the buf struct to process.
 *              tim_t *tbl      addr of TIM table to build.
 *              int tbl_no      size of the TIM table
 * Returns:     Number of entries in constructed table.
 */
#define BUILD_TIM(ap, com_ptr, bp, tbl, tbl_no)			  \
	(p8xx_TIM_build(ap, com_ptr,				  \
            ((bp->b_flags & B_READ)   ? DMA_READ       : 0) |	  \
            ((bp->b_flags & B_NOHIDE) ? DMA_WRITE_ONLY : 0),	  \
            bp->b_un.b_addr, (size_t) bp->b_bcount, &bp->b_xmemd, \
            tbl, tbl_no))


#ifdef DEBUG_8XX
#include <stdio.h>

/*
 * Macros to save and restore more debug flags
 */
#define P8_PUSHDBG()	{						\
			    if (p8xx_dbidx < P8XX_SVMAX)		\
				p8xx_dbsv[p8xx_dbidx++] = p8xx_debug;	\
			}
#define P8_POPDBG()	{						\
			    if (p8xx_dbidx > 0) 			\
				p8xx_debug = p8xx_dbsv[--p8xx_dbidx];	\
			}


/*
 * Macros to turn on and off more extensive debugging noise
 */
#define	P8_SETDBG(mask,val)	if (p8xx_debug & (mask)) {	\
				    P8_PUSHDBG();		\
				    p8xx_debug &= ~P8_ALL_DBMK;	\
				    p8xx_debug |= (ulong)(val);	\
				}
#define	P8_RSTDBG(mask)		if (p8xx_debug & (mask)) {	\
				    P8_POPDBG();		\
				}

#define P8printf(mask,args)		if (p8xx_debug & (mask)) printf args
#define	P8if(mask,args)			if (p8xx_debug & (mask)) args
#define	P8else(args)			else args
#define	P8debug(args)			args
#else	/* DEBUG_8XX */
#define P8printf(mask,args)
#define P8if(mask,args)
#define P8else(args)
#define	P8debug(args)
#define P8_PUSHDBG()
#define	P8_POPDBG()
#define	P8_SETDBG(mask,val)
#define	P8_RSTDBG(mask)

#endif	/* DEBUG_8XX */

/* top-half debug printfs */
#ifdef DEBUGT_8XX
#include <stdio.h>
#define P8Tprintf(args)		printf args;printf("\n") 
#else
#define P8Tprintf(args)
#endif

#define	W_START(p)	w_start(p)
#define	W_STOP(p)	w_stop(p)

/*
 * Define pragma for inline sync instruction
 */
#ifdef	NO_SYNC
#define	SYNC()				/* disable it for now */
#else
void	SYNC();
/*#pragma mc_func SYNC { "780004ac" }  ??* Synchronize (sync) instruction */
#pragma mc_func SYNC { "7c0006ac" }	/* eieio instruction */
#pragma reg_killed_by SYNC		/* (none killed) */
#endif


/*
 * Macros to convert to and from bus byte order.  Note that these macros
 * assume (and require) that shorts are 16 bits and longs are 32 bits.
 * Conversions both directions are provided for consistancy sake only;
 * i.e., the operations are isomorphic.
 *
 *	StoCBO()	Convert short to Canonical Byte Order
 *	LtoCBO()	Convert long to Canonical Byte Order
 *	CBOtoS()	Convert short from Canonical Byte Order
 *	CBOtoL()	Convert long from Canonical Byte Order
 */
#define	StoCBO(h)	((ushort) (                   \
			(((ushort)(h)<<8) & 0xff00) | \
			(((ushort)(h)>>8) & 0x00ff)   \
			))
#define	LtoCBO(w)	((ulong) (                         \
			(((ulong)(w)>>24)             )  | \
			(((ulong)(w)>> 8) & 0x0000ff00)  | \
			(((ulong)(w)<< 8) & 0x00ff0000)  | \
			(((ulong)(w)<<24)             )    \
			))
#define	CBOtoS(h)	StoCBO(h)
#define	CBOtoL(w)	LtoCBO(w)


/*
 * These two macros are merely shorthand for referencing the two kinds
 * of SCRIPTS we have
 */
#define	DEV_SCRIPT(a)	dev_ptr->Scripts.vscript_ptr[(a)]
#define	ADP_SCRIPT(a)	ap->Scripts.vscript_ptr[(a)]

/*
 * The following macro is used to quickly generate the device index.
 * The macro assumes "a" is SCSI ID, and "b" is LUN.
 */
#define INDEX(a,b)    ((((a) & 0x0F)<<3) | ((b) & 0x07))

/* This macro returns the scsi id from a previously generated index. */
#define SID(x)	      ((x)>>3)

/* This macro returns the lun id from a previously generated index. */
#define LUN(x)	      ((x) & 0x07)

/* Calculate length of a SCRIPTS pointer array */
#define	S_COUNT(a)	(sizeof(a)/sizeof((a)[0]))

/* Generate index into SCRIPTS array from an offset value */
#define	S_INDEX(a)	((a)/sizeof(ULONG))

/* Convert SCSI Transfer Period to and from nanoseconds */
#define	XFER_PRD(a)	((a) / XFER_UNIT)	/* cnvt to 4ns units */
#define	XFER_VAL(a)	((a) * XFER_UNIT)	/* cnvt from 4ns units */


/*
 * This goes to the buffer area of the main SCRIPT and reads back the value
 * being held in the status buffer.
 */
#define GET_STATUS(script_bus_addr)	\
		((script_bus_addr)[Ent_status_buf/4] >> 24)

/* remove an element from the bp_save queue */
#define p8xx_DEQ_BP_SAVE(dev_ptr)    \
            dev_ptr->bp_save_head = \
                (struct sc_buf *) dev_ptr->bp_save_head->bufstruct.av_forw;

/* remove an element from the cmd_save queue */
#define p8xx_DEQ_CMD_SAVE(dev_ptr)    \
            dev_ptr->cmd_save_head = dev_ptr->cmd_save_head->next_dev_cmd;

/*
 * These calls are used to control the state of the activity light.
 */
#define	LIGHT_ON(a)   	{if (a->light_supported) \
			    (void) write_operator_panel_light(1);}
#define	LIGHT_OFF(a)   	{if (a->light_supported) \
			    (void) write_operator_panel_light(0);}

/*
 * This macro restarts the chip at the current DSP address.
 */
#define	RESTART_CHIP(d, c){				\
	    p8xx_start_chip(d->ap, c,		\
		d->sid_ptr->bscript_ptr,			\
		p8xx_read_reg(d->ap, DSP, DSP_SIZE)	\
		- d->sid_ptr->bscript_ptr, NULL, 0);	\
	    }


/*
 * Name:        SUSPEND_CHIP
 * Description: Suspends the chip from a wait for reselect instruction so
 *              that we can start a new I/O request (to a non-busy target).
 *		Updates the power mgmt. activity field.
 * Input:       ads_t   *ap     address of adapter structure
 * Returns:     nothing
 */

#define SUSPEND_CHIP(a){				   \
        p8xx_write_reg(a, ISTAT, ISTAT_SIZE, ISTAT_SIGP);  \ 
	a->pmh.activity = PM_ACTIVITY_OCCURRED;	   	   \
}


/*
 * Eases an ugly task
 */
#define	INT_ON(p,a)	(p)[a[0]-1] = LtoCBO(P8XX_INTR_PATCH)
#define	INT_OFF(p,a)	(p)[a[0]-1] = LtoCBO(P8XX_NOP_PATCH)


/*
 * This macro is a shorthand for patching dev_info_hash values into the
 * SCRIPTS int instructions.
 */
#define	DEVH_PATCH(tgt,lbl,devh)			\
	{						\
	    int	i;					\
	    for (i=0; i < S_COUNT(lbl); i++) {		\
		tgt[lbl[i]] &= 0xFFFF0000;		\
		tgt[lbl[i]] |= (devh);			\
	    }						\
	}

#endif	/* _NCR8XXMAC */
