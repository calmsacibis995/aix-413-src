/* @(#)79  1.1  src/rspc/kernext/pcmcia/ssisa/pcm_ss_data.h, pcmciaker, rspc411, 9433A411a 8/6/94 15:08:18 */
/*
 * COMPONENT_NAME: PCMCIAKER
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Adapter Specific Definitions
 */

#define ADP_CAPS    0
#define ACTIVE_HIGH 0
#define ACTIVE_LOW  (IRQ_MAP) 

/* Window Capabilities used by InquireWindow : constant */
struct tagWindow {
    int WndCaps;
    int Index;
    int EnableBit; /* used by Address Window Enable Register (0x06) */
} Window[] = {
    { PCM_WC_COMMON | PCM_WC_ATTRIBUTE | PCM_WC_WAIT, 0x10, BIT_0 }, /* 0 */
    { PCM_WC_COMMON | PCM_WC_ATTRIBUTE | PCM_WC_WAIT, 0x18, BIT_1 }, /* 1 */
    { PCM_WC_COMMON | PCM_WC_ATTRIBUTE | PCM_WC_WAIT, 0x20, BIT_2 }, /* 2 */
    { PCM_WC_COMMON | PCM_WC_ATTRIBUTE | PCM_WC_WAIT, 0x28, BIT_3 }, /* 3 */
    { PCM_WC_COMMON | PCM_WC_ATTRIBUTE | PCM_WC_WAIT, 0x30, BIT_4 }, /* 4 */
    { PCM_WC_IO                        | PCM_WC_WAIT, 0x08, BIT_6 }, /* 5 */
    { PCM_WC_IO                        | PCM_WC_WAIT, 0x0C, BIT_7 }, /* 6 */
};
#define NUM_WINDOWS (sizeof Window / sizeof Window[0])

#define NUM_EDCS 0

#define IRQ_MAP (PCM_IRQ_3  | PCM_IRQ_4  | PCM_IRQ_5  | PCM_IRQ_6  \
               | PCM_IRQ_7  | PCM_IRQ_9  | PCM_IRQ_10 | PCM_IRQ_11 \
               | PCM_IRQ_12 | PCM_IRQ_14 | PCM_IRQ_15 )


#define PCM_COMPLIANCE 0x210 /* PCMCIA compliance level: 2.1 */

/* Power Entries: constant */
PWRENTRY PwrEntry [] = {
    /* PowerLevel, ValidSignals */
    {           0, PCM_VCC | PCM_VPP1 | PCM_VPP2 }, /*  0 : 0x00 */
    {          50, PCM_VCC | PCM_VPP1 | PCM_VPP2 }, /* Vcc: 0x01 */
    {         120,           PCM_VPP1 | PCM_VPP2 }  /* Vpp: 0x02 */
};
#define NUM_PWR_ENTRY ( sizeof PwrEntry / sizeof PwrEntry[0] )

#define SKT_CAPS ( PCM_IF_IO | PCM_IF_MEMORY )
#define SC_INT_CAPS (PCM_SBM_BVD1 | PCM_SBM_BVD2 | PCM_SBM_RDYBSY | PCM_SBM_CD)
#define SC_RPT_CAPS (PCM_SBM_WP | \
                     PCM_SBM_BVD1 | PCM_SBM_BVD2 | PCM_SBM_RDYBSY | PCM_SBM_CD)
#define SC_IND_CAPS PCM_SBM_BUSY

#define MEMWNDCAPS (PCM_WC_BASE | PCM_WC_SIZE | PCM_WC_WENABLE \
                   | PCM_WC_8BIT | PCM_WC_16BIT | PCM_WC_PENABLE | PCM_WC_WP)

const MEMWINTBL MemWinTbl = {
    MEMWNDCAPS, /* MemWndCaps */
    0,          /* FirstByte */
    0xFFFF,     /* LastByte */
    0x1000,     /* MinSize: 4KB */
    0,          /* MaxSize */
    0x1000,     /* ReqGran: 4KB */
    1,          /* ReqBase */
    1,          /* ReqOffset */
    PCM_700NS,  /* Slowest */
    PCM_250NS   /* Fastest */
    };

#define IOWNDCAPS ( PCM_WC_BASE | PCM_WC_SIZE | PCM_WC_WENABLE \
                  | PCM_WC_8BIT | PCM_WC_16BIT | PCM_WC_INPACK \
                  | PCM_WC_CENABLE )

const IOWINTBL IOWinTbl = {
    IOWNDCAPS, /* IOWndCaps */
    0,         /* FirstByte */
    0xFFFF,    /* LastByte */
    1,         /* MinSize */
    0xFFFF,    /* MaxSize */
    1,         /* ReqGran */
    16,        /* AddrLiines */
    0          /* EISASlot */
    };

char PCM_IMPLEMENTOR[] =  "International Business Machines Corp.";
#define PCM_RELEASE 0x100
