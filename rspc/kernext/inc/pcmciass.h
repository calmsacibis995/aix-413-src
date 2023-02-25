/* @(#)94  1.1  src/rspc/kernext/inc/pcmciass.h, pcmciaker, rspc411, 9433A411a 8/6/94 15:08:49 */
/*
 * COMPONENT_NAME: (PCMCIAKER)
 *
 * FUNCTIONS: PCMCIA Socket Services Interface
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_PCMCIA
#define _H_PCMCIA
#include <sys/err_rec.h>

/* FOR PCMCIA Socket Service D/D */

/* Definitions for PCMCIA standard */

#define PCM_GET_ADP_CNT       0x80

#define PCM_GET_SS_INFO       0x83
#define PCM_INQ_ADAPTER       0x84
#define PCM_GET_ADAPTER       0x85
#define PCM_SET_ADAPTER       0x86
#define PCM_INQ_WINDOW        0x87
#define PCM_GET_WINDOW        0x88
#define PCM_SET_WINDOW        0x89
#define PCM_GET_PAGE          0x8A
#define PCM_SET_PAGE          0x8B
#define PCM_INQ_SOCKET        0x8C
#define PCM_GET_SOCKET        0x8D
#define PCM_SET_SOCKET        0x8E
#define PCM_GET_STATUS        0x8F
#define PCM_RESET_SOCKET      0x90

#define PCM_INQ_EDC           0x95
#define PCM_GET_EDC           0x96
#define PCM_SET_EDC           0x97
#define PCM_START_EDC         0x98
#define PCM_PAUSE_EDC         0x99
#define PCM_RESUME_EDC        0x9A
#define PCM_STOP_EDC          0x9B
#define PCM_READ_EDC          0x9C
#define PCM_GET_VENDOR_INFO   0x9D
#define PCM_ACK_INTERRUPT     0x9E
#define PCM_PRIOR_HANDLER     0x9F
#define PCM_SS_ADDR           0xA0
#define PCM_ACCESS_OFFSETS    0xA1
#define PCM_VEND_SPECIFIC     0xAE
#define PCM_CARD_SERVICES     0xAF

/* Return codes : PCMCIA standard return codes */
#define PCM_SUCCESS           0x00
#define PCM_BAD_ADAPTER       0x01
#define PCM_BAD_ATTRIBUTE     0x02
#define PCM_BAD_BASE          0x03
#define PCM_BAD_EDC           0x04

#define PCM_BAD_IRQ           0x06
#define PCM_BAD_OFFSET        0x07
#define PCM_BAD_PAGE          0x08
#define PCM_READ_FAILURE      0x09
#define PCM_BAD_SIZE          0x0A
#define PCM_BAD_SOCKET        0x0B

#define PCM_BAD_TYPE          0x0D
#define PCM_BAD_VCC           0x0E
#define PCM_BAD_VPP           0x0F

#define PCM_BAD_WINDOW        0x11
#define PCM_WRITE_FAILURE     0x12

#define PCM_NO_CARD           0x14
#define PCM_BAD_FUNCTION      0x15
#define PCM_BAD_MODE          0x16
#define PCM_BAD_SPEED         0x17
#define PCM_BUSY              0x18


/* Valid power level bit-masks */
#define PCM_VCC                 0x80 /* Power signal VCC  */
#define PCM_VPP1                0x40 /* Power signal VPP1 */
#define PCM_VPP2                0x20 /* Power signal VPP2 */

/* Adapter capabilities bit-masks */
#define PCM_AC_IND              0x01
#define PCM_AC_PWR              0x02
#define PCM_AC_DBW              0x04

/* Adapter state */
#define PCM_AS_POWERDOWN        0x01
#define PCM_AS_MAINTAIN         0x02

/* Generic window capabilities bit-masks */
#define PCM_WC_COMMON           0x01
#define PCM_WC_ATTRIBUTE        0x02
#define PCM_WC_IO               0x04
#define PCM_WC_WAIT             0x80

/* Memory and I/O window capabilities bit-masks */
#define PCM_WC_BASE             0x0001
#define PCM_WC_SIZE             0x0002
#define PCM_WC_WENABLE          0x0004
#define PCM_WC_8BIT             0x0008
#define PCM_WC_16BIT            0x0010
#define PCM_WC_BALIGN           0x0020
#define PCM_WC_POW2             0x0040

/* Memory window (page) capabilities only */
#define PCM_WC_CALIGN           0x0080
#define PCM_WC_PAVAIL           0x0100
#define PCM_WC_PSHARED          0x0200
#define PCM_WC_PENABLE          0x0400
#define PCM_WC_WP               0x0800

/* I/O window capabilities only */
#define PCM_WC_INPACK           0x0080
#define PCM_WC_EISA             0x0100
#define PCM_WC_CENABLE          0x0200

/* Generic window sate */
#define PCM_WS_IO               0x01
#define PCM_WS_ENABLED          0x02
#define PCM_WS_16BIT            0x04

/* Memory window state */
#define PCM_WS_PAGED            0x08

/* I/O window state */
#define PCM_WS_EISA             0x08
#define PCM_WS_CENABLE          0x10

/* Page state */
#define PCM_PS_ATTRIBUTE        0x01
#define PCM_PS_ENABLED          0x02
#define PCM_PS_WP               0x04

/* IRQ level bit-masks (low word of 32-bit mask) */
#define PCM_IRQ_0               0x0001 /* IRQ level  0 */
#define PCM_IRQ_1               0x0002 /* IRQ level  1 */
#define PCM_IRQ_2               0x0004 /* IRQ level  2 */
#define PCM_IRQ_3               0x0008 /* IRQ level  3 */
#define PCM_IRQ_4               0x0010 /* IRQ level  4 */
#define PCM_IRQ_5               0x0020 /* IRQ level  5 */
#define PCM_IRQ_6               0x0040 /* IRQ level  6 */
#define PCM_IRQ_7               0x0080 /* IRQ level  7 */
#define PCM_IRQ_8               0x0100 /* IRQ level  8 */
#define PCM_IRQ_9               0x0200 /* IRQ level  9 */
#define PCM_IRQ_10              0x0400 /* IRQ level 10 */
#define PCM_IRQ_11              0x0800 /* IRQ level 11 */
#define PCM_IRQ_12              0x1000 /* IRQ level 12 */
#define PCM_IRQ_13              0x2000 /* IRQ level 13 */
#define PCM_IRQ_14              0x4000 /* IRQ level 14 */
#define PCM_IRQ_15              0x8000 /* IRQ level 15 */

/* IRQ level bit-masks (high word of 32-bit mask */
#define PCM_IRQ_NMI             0x0001
#define PCM_IRQ_IO              0x0002
#define PCM_IRQ_BUSERR          0x0004

/* IRQ state bit-masks */
#define PCM_IRQ_HIGH            0x40
#define PCM_IRQ_ENABLE         0x80

/* Socket bit-masks */
#define PCM_SBM_WP              0x01
#define PCM_SBM_LOCKED          0x02
#define PCM_SBM_EJECT           0x04
#define PCM_SBM_INSERT          0x08
#define PCM_SBM_BVD1            0x10
#define PCM_SBM_BVD2            0x20
#define PCM_SBM_RDYBSY          0x40
#define PCM_SBM_CD              0x80

#define PCM_SBM_LOCK            0x10
#define PCM_SBM_BATT            0x20
#define PCM_SBM_BUSY            0x40
#define PCM_SBM_XIP             0x80

/* Interface bit-masks */
#define PCM_IF_MEMORY           0x01 /* Memory-only interface    */
#define PCM_IF_IO               0x02 /* I/O and memory interface */

/* EDC definitions             */
/*     They are not supported, but defined  */
#define PCM_EC_UNI              0x01
#define PCM_EC_BI               0x02
#define PCM_EC_REGISTER         0x04
#define PCM_EC_MEMORY           0x08
#define PCM_EC_PAUSABLE         0x10

#define PCM_EC_WRITE            0x02

#define PCM_EC_CHECK8           0x01
#define PCM_EC_SDLC16           0x02

/* End of definitions for PCMCIA standard */

/* Definitions for IBM PCMCIA Adapter/A */


/* Defined data types */
/*   Internal definition for adaptation to AIX */
typedef unsigned char  PCM_BYTE;
typedef unsigned short PCM_WORD;
typedef unsigned int   PCM_DWORD;
typedef char *         PTR;
typedef PCM_BYTE       BYTE;
typedef PCM_WORD       WORD;
typedef PCM_DWORD      DWORD;

/*   PCMCIA standard definition */
typedef PCM_BYTE ADAPTER;
typedef PCM_WORD BASE;
typedef PCM_WORD BCD;
typedef PCM_BYTE COUNT;
typedef PCM_BYTE EDC;
typedef PCM_BYTE FLAGS8;
typedef PCM_WORD FLAGS16;
typedef PCM_DWORD FLAGS32;
typedef PCM_BYTE IRQ;
typedef PCM_WORD OFFSET;
typedef PCM_BYTE PAGE;
typedef PCM_BYTE PWRINDEX;
typedef PCM_BYTE RETCODE;
typedef PCM_WORD SIGNATURE;
typedef PCM_WORD SIZE;
typedef PCM_BYTE SOCKET;
typedef PCM_BYTE SPEED;
typedef PCM_BYTE WINDOW;
typedef PCM_BYTE SKTBITS;

/* CIS data format */
#define PCM_700NS ( 0x0E << 3 | 0x02 )
#define PCM_250NS 0x01
#define PCM_200NS 0x02
#define PCM_150NS 0x03
#define PCM_100NS 0x04

#define PCM_IOREAD   0 /* PCMCIA I/O register read  operation */
#define PCM_IOWRITE  1 /* PCMCIA I/O register write operation */

/* Structures */

/* Socket Service Arguments Structures */
typedef struct tagACHARTBL {
	FLAGS16 AdpCaps;
	FLAGS32 ActiveHigh;
	FLAGS32 ActiveLow;
} ACHARTBL;

typedef struct tagPWRENTRY {
	PWRINDEX PowerLevel;
	FLAGS8 ValidSignals;
} PWRENTRY;

#define NUM_ENTRIES 16
typedef struct tagAISTRUCT {
	PCM_WORD wBufferLength;
	PCM_WORD wDataLength;
	ACHARTBL CharTable;
	PCM_WORD wNumPwrEntries;
    PWRENTRY PwrEntry[NUM_ENTRIES];
} AISTRUCT;

typedef struct tagCHARTBL {
	union {
		FLAGS16 AdpCaps;
		FLAGS16 SktCaps;
	} Caps;
	FLAGS32 ActiveHi;
	FLAGS32 ActiveLo;
} CHARTBL;

typedef struct tagMEMWINTBL {
	FLAGS16 MemWndCaps;
	BASE FirstByte;
	BASE LastByte;
	SIZE MinSize;
	SIZE MaxSize;
	SIZE ReqGran;
	SIZE ReqBase;  
	SIZE ReqOffset;
	SPEED Slowest;
	SPEED Fastest;
} MEMWINTBL;

typedef struct tagIOWINTBL {
	FLAGS16 IOWndCaps;
	BASE FirstByte;
	BASE LastByte;
	SIZE MinSize;
	SIZE MaxSize;
	SIZE ReqGran;
	COUNT AddrLines;
	FLAGS8 EISASlot;
} IOWINTBL;

typedef union tagWINTBL {
	MEMWINTBL MemWinTbl;
	IOWINTBL  IOWinTbl;
} WINTBL;

#define NUM_TYPES 2
typedef struct tagWISTRUCT {
	PCM_WORD wBufferLength;
	PCM_WORD wDataLength;
	WINTBL WinTable[NUM_TYPES];
} WISTRUCT;

typedef struct tagSCHARTBL {
	FLAGS16 SktCaps;
	FLAGS32 ActiveHigh;
	FLAGS32 ActiveLow;
} SCHARTBL;

typedef struct tagSISTRUCT {
	PCM_WORD wBufferLength;
	PCM_WORD wDataLength;
	SCHARTBL CharTable;
} SISTRUCT;

#define BUF_SIZE 256
typedef struct tagVISTRUCT {
	PCM_WORD wBufferLength;
	PCM_WORD wDataLength;
	char szImplementor[BUF_SIZE - 4];
} VISTRUCT;

typedef struct
{
	RETCODE retcode;
	SKTBITS Sockets;
} AcknowledgeInterrupt_S;

typedef struct
{
	RETCODE retcode;
	BCD Compliance;
	COUNT NumAdapters;
	ADAPTER FirstAdapter;
} GetSSInfo_S;

typedef struct {
	RETCODE retcode;
	AISTRUCT *pBuffer;
	COUNT NumSockets;
	COUNT NumWindows;
	COUNT NumEDCs;
} InquireAdapter_S;

typedef struct
{
	RETCODE retcode;
	FLAGS8 State;
	IRQ SCRouting;
} GetAdapter_S;

typedef GetAdapter_S SetAdapter_S;

typedef struct
{
	RETCODE retcode;
	SOCKET Socket;
	SISTRUCT *pBuffer;
	FLAGS8 SCIntCaps;
	FLAGS8 SCRptCaps;
	FLAGS8 CtlIndCaps;
} InquireSocket_S;

typedef struct
{
	RETCODE retcode;
	SOCKET Socket;
	FLAGS8 SCIntMask;
	PWRINDEX VccLevel;
	PWRINDEX VppLevels;
	FLAGS8 State;
	FLAGS8 CtlInd;
	IRQ IREQRouting;
	FLAGS8 IFType;
} GetSocket_S;

typedef GetSocket_S SetSocket_S;

typedef struct
{
	RETCODE retcode;
	SOCKET Socket;
} ResetSocket_S;

typedef struct
{
	RETCODE retcode;
	SOCKET Socket;
	FLAGS8 CardState;
	FLAGS8 SocketState;
	FLAGS8 CtlInd;
	IRQ IREQRouting;
	FLAGS8 IFType;
}  GetStatus_S;

typedef struct
{
	RETCODE retcode;
	WINDOW Window;
	WISTRUCT *pBuffer;
	FLAGS8 WndCaps;
	SKTBITS Sockets;
} InquireWindow_S;

typedef struct
{
	RETCODE retcode;
	WINDOW Window;
	SOCKET Socket;
	SIZE Size;
	FLAGS8 State;
	SPEED Speed;
	BASE Base;
} GetWindow_S;

typedef GetWindow_S SetWindow_S;

typedef struct
{
	RETCODE retcode;
	WINDOW Window;
	PAGE Page;
	FLAGS8 State;
	OFFSET Offset;
} GetPage_S;

typedef GetPage_S SetPage_S;

typedef struct
{
	RETCODE retcode;
	BYTE Type;
	VISTRUCT *pBuffer;
	BCD Release;
} GetVendorInfo_S;

typedef struct
{
	RETCODE retcode;
	EDC EDC;
	SOCKET Socket;
	FLAGS8 State;
	FLAGS8 Type;
} GetEDC_S;

typedef GetEDC_S SetEDC_S;

typedef struct
{
	RETCODE retcode;
	EDC EDC;
	SKTBITS Sockets;
	FLAGS8 Caps;
	FLAGS8 Types;
} InquireEDC_S;

typedef struct
{
	RETCODE retcode;
	EDC EDC;
} StartEDC_S;

typedef StartEDC_S StopEDC_S;
typedef StartEDC_S PauseEDC_S;
typedef StartEDC_S ResumeEDC_S;

typedef struct
{
	RETCODE retcode;
	EDC EDC;
	WORD Value;
} ReadEDC_S;

struct pcmcia_ioreg
{
	unsigned int index;
	unsigned int data;
};

typedef struct
{
	RETCODE retcode;
	PCM_WORD function;
	union {
		struct pcmcia_ioreg ioreg; /* for PCM_IOREAD and PCM_IOWRITE */
		unsigned char vcmd[256];
	} varg;
} VendorSpecific_S;

/* 
 * PCMCIA device specific data
 * This area is pointed by the d_dsdptr member of the device switch table.
 * the intr_func should point same address as ioctl.
 * Sample:
 *     devswqry( devno, &rc1, &rc2);
 *     (*((struct pcmcia_dsd *)rc2)->intr_func)( devno, op, arg );
 */
struct pcmcia_dsd
{
	int (*intr_func)();
};

struct pcmcia_err
{
	struct err_rec0 err;
	unsigned errdata[32];
};

#endif /* _H_PCMCIA */
