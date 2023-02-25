/* @(#)64       1.2  src/rspc/kernext/pcmcia/tok/itok_misc.h, pcmciatok, rspc41J, 9511A_all 3/9/95 02:38:06 */
/* @(#)57	1.2  src/rspc/kernext/isa/itok/itok_misc.h, isatok, rspc41J 7/25/94 16:29:24 */
/*
 * COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS: none
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

#ifndef _H_TOKMISC.H
#define _H_TOKMISC.H

/* States for the open state-event variable. */
/* The state values are significant,i.e. f.ex. STATUS3 should = STATUS2 + 1.*/
#define OPEN_STATUS0    0
#define OPEN_STATUS1    1
#define OPEN_STATUS2    2
#define OPEN_STATUS3    3
#define OPEN_STATUS4    4
#define OPEN_STATUS5    5
#define OPEN_STATUS6    6
#define OPEN_COMPLETE   7

#define TOK_PREFIX		(TOK_DRIVER << 16)   
#define DD_NAME_STR		"itok_dd"
#define DISABLE_INTERRUPTS(lvl)	lvl= i_disable(DDI.intr_priority)
#define ENABLE_INTERRUPTS(lvl)	i_enable(lvl)
#define DISABLE_INTMAX(lvl)	lvl= i_disable(INTMAX)

#define NDD_ELAPSED_TIME(s)	((lbolt -s) / HZ)
#define TOK_CMD_TIMEOUT		10      /* In seconds. */
#define TOK_XMIT_TIMEOUT	5	/* In seconds. */
#define GROUP_ADR_MASK	0x80

#define TOK_4M			0
#define TOK_16M			1
#define TOK_ADD			0x81
#define TOK_DEL			0x82
#define TOK_NO_GROUP            (TOK_PREFIX | 0x0210)
#define TOK_NO_PARMS            (TOK_PREFIX | 0x0550)
#define TOK_NO_RING_INFO        (TOK_PREFIX | 0x0660)

#define TOK_STD_LAN_HDR_LEN	14        /* AC + FC + Dest & source addr. */
#define TOK_MIN_DATA		16
#define TOK_4M_MAX_DATA		2048  /* 4096 Really. */
#define TOK_16M_MAX_DATA	2048  /* 18000 Really. */

#define TOK_STOP_CMD_TIMER	if (WRK.cmd_wdt_active) { 	\
				  w_stop(&(CMDWDT));		\
				  WRK.cmd_wdt_active = FALSE;	\
				}

/* Ioctl constants (tokenring specific). */
#define TOK_GRP_ADDR           (CIO_IOCTL |    0x0103)
#define TOK_FUNC_ADDR          (CIO_IOCTL |    0x0104)
#define TOK_QVPD               (CIO_IOCTL |    0x0105)  /* Not used. */
#define TOK_ACCESS_POS         (CIO_IOCTL |    0x0107)  /* Not used. */
#define TOK_SET_ADAP_IPARMS    (CIO_IOCTL |    0x0108)
#define TOK_SET_OPEN_PARMS     (CIO_IOCTL |    0x0109)
#define TOK_RING_INFO          (CIO_IOCTL |    0x0113)

typedef struct SET_OPEN_OPTS {
   uint     status;
   ushort   options;
   short    buf_size;
   char     xmit_buf_min_cnt;
   char     xmit_buf_max_cnt;
   ushort   i_addr1;
   ushort   i_addr2;
   ushort   i_addr3;
} tok_set_open_opts_t;

/* -------------------------------------------------------------------- */
/*  Group Address ioctl structure                                       */
/* -------------------------------------------------------------------- */

typedef struct GROUP_ADDR {
   uint        status;         /* Returned status code */
   uint        opcode;         /* Add or delete group address */
   uint        group_addr;     /* group address */
}  tok_group_addr_t;

/* -------------------------------------------------------------------- */
/*  Functional Address ioctl structure                                  */
/* -------------------------------------------------------------------- */

typedef struct FUNC_ADDR {
   uint        status;         /* Returned status code */
   uint        opcode;         /* Add or delete func. address */
   uint        func_addr;      /* functional address */
}  tok_func_addr_t;

/* This constant is for the padding we have to do for the if-layer,
   should go away with AIX 4.x .*/
#define TOK_ROUTE_INFO_SIZE		18

#define TRACE_HOOKWORD		HKWD_DD_TOKDD
#define TOK_END_TRACE_TABLE	0x6C617374   /* Spells "last". */

#ifdef DEBUG
#define TOK_TRACE_SIZE       1024            /* 4096 bytes, 256 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)  \
        tok_save_trace(hook, tag, arg1, arg2, arg3)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)  \
        tok_save_trace(hook, tag, arg1, arg2, arg3)
#else
#define TOK_TRACE_SIZE       128             /* 512 bytes, 32 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)  \
        TRCHKGT(hook << 20| HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)
#endif

#define TRACE_BOTH(hook, tag, arg1, arg2, arg3) \
        tok_save_trace(hook, tag, arg1, arg2, arg3)

/*
 * Macros to get/put caller's parameter block when only an address is supplied.
 * Useful for "arg" in ioctl and for extended paramters on other dd entries.
 * Value is 0 if ok, otherwise EFAULT.  Typical usage is:
 * if (rc = COPYIN (devflag, arg, &localdata, sizeof(localdata)))
 *    return (rc);
 */
#define COPYIN(dvf,usa,dda,siz)                               \
( (usa == NULL) ? (EFAULT) :                                  \
      ( (dvf & DKERNEL) ? (bcopy(usa,dda,siz), 0) :           \
            ( (rc = copyin(usa,dda,siz) != 0) ? rc : 0  ) ) )

#define COPYOUT(dvf,dda,usa,siz)                              \
( (usa == NULL) ? (EFAULT) :                                  \
      ( (dvf & DKERNEL) ? (bcopy(dda,usa,siz), 0) :           \
            ( (rc = copyout(dda,usa,siz) !=0) ? rc : 0 ) ) )


#endif   /* If _H_TOKMISC.H */


