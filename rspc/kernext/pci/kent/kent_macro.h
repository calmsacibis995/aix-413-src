/* @(#)59       1.1  src/rspc/kernext/pci/kent/kent_macro.h, pcient, rspc41J, 9507B 1/25/95 11:26:30 */
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: 
 *		SWAPLONG
 *		SWAPSHORT
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
#include <stddef.h>

#ifndef _H_KENTMACRO
#define _H_KENTMACRO

#include <sys/trcmacros.h>
#include <sys/ioacc.h>

/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS     */
/*  accessor macros.  The essential difference is that retries are        */
/*  performed if pio errors occur; if the retry limit is exceeded, a -1   */
/*  is returned (hence all return an int value).  In the cases of         */
/*  PIO_GETL and PIO_GETLR, the -1 is indistinguishable from all FF's so  */
/*  some heuristic must be used to determine if it is an error (i.e., is  */
/*  all FF's a legitimate read value?).                                   */
/*------------------------------------------------------------------------*/

enum pio_func
{
	GETC, GETS, GETSR, GETL, GETLR, PUTSTR, GETSTR,
	PUTC, PUTS, PUTSR, PUTL, PUTLR
};


#define PIO_GETCX(addr, c)						\
{									\
	if (p_acs->dev.iox = BUS_GETCX((void *)addr, c))		\
	{								\
		kent_logerr(p_acs,ERRID_KENT_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			kent_pio_retry(p_acs, GETC, (void *)addr, (ulong)(c), NULL);\
	}								\
}
#define PIO_GETSX(addr, s)						\
{									\
	if (p_acs->dev.iox = BUS_GETSX((void *)addr, s))		\
	{								\
		kent_logerr(p_acs,ERRID_KENT_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			kent_pio_retry(p_acs, GETS, (void *)addr, (ulong)(s), NULL);\
	}								\
}
#define PIO_GETSRX(addr, sr)						\
{									\
	if (p_acs->dev.iox = BUS_GETSRX((void *)addr, sr))		\
	{								\
		kent_logerr(p_acs,ERRID_KENT_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			kent_pio_retry(p_acs, GETSR, (void *)addr, (ulong)(sr), NULL);\
	}								\
}
#define PIO_GETLRX(addr, lr)						\
{									\
	if (p_acs->dev.iox = BUS_GETLRX((void *)addr, lr))		\
	{								\
		kent_logerr(p_acs,ERRID_KENT_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			kent_pio_retry(p_acs, GETLR, (void *)addr, (ulong)(lr), NULL);\
	}								\
}
#define PIO_PUTCX(addr, c)						\
{									\
	if (p_acs->dev.iox = BUS_PUTCX((void *)addr, c))		\
	{								\
		kent_logerr(p_acs,ERRID_KENT_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			kent_pio_retry(p_acs, PUTC, (void *)addr, (ulong)(c), NULL);\
	}								\
}
#define PIO_PUTSRX(addr, sr)						\
{									\
	if (p_acs->dev.iox = BUS_PUTSRX((void *)addr, sr))		\
	{								\
		kent_logerr(p_acs,ERRID_KENT_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			kent_pio_retry(p_acs, PUTSR, (void *)addr, (ulong)(sr), NULL);\
	}								\
}
#define PIO_PUTLRX(addr, lr)						\
{									\
	if (p_acs->dev.iox = BUS_PUTLRX((void *)addr, lr))		\
	{								\
		kent_logerr(p_acs,ERRID_KENT_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			kent_pio_retry(p_acs, PUTLR, (void *)addr, (ulong)(lr), NULL);\
	}								\
}


/*
 * For use with the copy string PIO functions
 */
#define SWAP_SHORT(x)   ((((x) & 0xFF) << 8) | ((x) >> 8))
#define SWAP_LONG(x)    ((((x) & 0xFF)<<24) | (((x) & 0xFF00)<<8) | \
			 (((x) & 0xFF0000)>>8) | (((x) & 0xFF000000)>>24))

#define ADDR_LO(addr)   ((int)(addr) & 0xffff)          /* low 16 bits */
#define ADDR_HI(addr)   (((int)(addr) >> 16) & 0xffff)  /* high 16 bits */

/*
** This routine will delay for ms number of milliseconds.
*/

#define DELAYMS(ms)                                                      \
{                                                                        \
    ulong limit;                                                         \
                                                                         \
    limit = ms * uS_PER_MS;                                              \
                                                                         \
    if (limit > (uS_PER_SECOND - 1))                                     \
    {                                                                    \
        limit = uS_PER_SECOND - 1;                                       \
    }                                                                    \
                                                                         \
    (void) io_delay(limit);                                              \
}


/*
 * For use in accessing the adapter's csr and bcr registers 
 */
#define BCR_READ(iomem,reg, addr)			\
{							\
	PIO_PUTLRX(iomem + KENT_IO_RAP, (reg));		\
	PIO_GETLRX(iomem + KENT_IO_BDP, (addr));	\
}

#define BCR_WRITE(iomem, reg, value)			\
{							\
	PIO_PUTLRX(iomem + KENT_IO_RAP, (reg));		\
	PIO_PUTLRX(iomem + KENT_IO_BDP, (value));	\
}

#define CSR_READ(iomem, reg, addr)			\
{							\
	PIO_PUTLRX(iomem + KENT_IO_RAP, (reg));		\
	PIO_GETLRX(iomem + KENT_IO_RDP, (addr));	\
}

#define CSR_WRITE(iomem, reg, value)			\
{							\
	PIO_PUTLRX(iomem + KENT_IO_RAP, (reg));		\
	PIO_PUTLRX(iomem + KENT_IO_RDP, (value));	\
}
/*
 * KENT_IS_BCAST(addr) : returns TRUE if the address is a Broadcast address 
 * 			and FALSE otherwise
 */
#define KENT_IS_BCAST(addr)					\
	((*(long *)(&(addr)[0]) == 0xffffffff) && 		\
		(*(long *)(&(addr)[2]) == 0xffffffff))		\
			? (TRUE) : (FALSE)

/*
 * Convert a micro-second to a rtcl_t time value.
 */
#define MS2TIME(msec, tp)	\
    (tp).tv_sec = (msec) / MSEC_PER_SEC; \
    (tp).tv_nsec = ((msec) % MSEC_PER_SEC) * NS_PER_MSEC; 

/*
 * Arm a receive descriptor for the adapter.
 */
#define ARM_RX_DESC(stat1, stat2)				\
	(stat2) = 0;						\
	(stat1) = SWAP_LONG((KENT_BLEN_MASK & (~KENT_BUFF_SZ + 1)) | \
			KENT_RX_STAT1_ONES | KENT_RX_STAT1_OWN);

/*
 * Asserts defined with debug
 */
#ifdef KENT_DEBUG
#	define KENT_ASSERT(c)	if (!(c)) brkpoint();
#else
#	define KENT_ASSERT(c)
#endif

/* 
 * Macros for debug and error tracing.
 */

#define KENT_XMIT 	((HKWD_KEN_PCI_XMIT << 20) | HKTY_GT | 4)
#define KENT_RECV 	((HKWD_KEN_PCI_RECV << 20) | HKTY_GT | 4)
#define KENT_OTHER 	((HKWD_KEN_PCI_OTHER << 20) | HKTY_GT | 4)

#define KENT_ETRACE(a1,a2,a3,a4)					\
{									\
	kent_trace(a1,a2,a3,a4);					\
	TRCHKGT(KENT_OTHER, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}

#ifdef KENT_DEBUG
#define KENT_TTRACE(a1,a2,a3,a4)					\
{									\
	kent_trace(a1,a2,a3,a4);					\
	TRCHKGT(KENT_XMIT, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}
#else
#define KENT_TTRACE(a1,a2,a3,a4)					\
{									\
	TRCHKGT(KENT_XMIT, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}
#endif

#define KENT_ETTRACE(a1,a2,a3,a4)					\
{									\
	kent_trace(a1,a2,a3,a4);					\
	TRCHKGT(KENT_XMIT, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}

#ifdef KENT_DEBUG
#define KENT_RTRACE(a1,a2,a3,a4)					\
{									\
	kent_trace(a1,a2,a3,a4);					\
	TRCHKGT(KENT_RECV, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}
#else
#define KENT_RTRACE(a1,a2,a3,a4)					\
{									\
	TRCHKGT(KENT_RECV, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}
#endif

#define KENT_ERTRACE(a1,a2,a3,a4)					\
{									\
	kent_trace(a1,a2,a3,a4);					\
	TRCHKGT(KENT_RECV, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}

#ifdef KENT_DEBUG
#define KENT_TRACE(a1,a2,a3,a4)					\
{									\
	kent_trace(a1,a2,a3,a4);					\
	TRCHKGT(KENT_OTHER, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}
#else
#define KENT_TRACE(a1,a2,a3,a4)					\
{									\
	TRCHKGT(KENT_OTHER, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}
#endif

#endif /* endif ! _H_KENTMACRO */
