/* @(#)39	1.5  src/rspc/kernext/pcmcia/cs/cs.h, pcmciaker, rspc41J, 9515A_all 4/11/95 16:26:09  */
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_PCMCIA_CS
#define _H_PCMCIA_CS
#include <sys/sleep.h>
#include <sys/lockl.h>
#include <sys/lock_def.h>
#include <sys/intr.h>

#include <sys/pmdev.h>
#include <sys/pcmciass.h>
#include "Evh.h"

/*****************************************
 * CARD SERVICES INTERNAL DATA STRUCTURE *
 *****************************************/
#define CSID            -1

#define	MAX_ADAPTER	4
#define MAX_SOCKET	8
#define MAX_WINDOW	60

#define DEFPOWER        50

/*
 * Window Structure
 */
typedef struct {
    int	          AdapterIndex;	/* Adapter Index  	*/
    int	          LogSocket;	/* Logical Socket 	*/
    int	          LogWindow;	/* Logical Window 	*/
    int	          PhyWindow;	/* Physical Window 	*/

    int           ownerID;      /* exclusive or firstshared owner */
    int	          Type;		/* IO / Memory 		          */
    char*         Base;         /* base address of IO/MEM window  */ 
    ulong         Size;         /* size of IO/MEM window          */
    int           Attributes;   /* exclusive/shared/firstshared   */

    /*
     * following fields are filled when SS InquireWindow is called.
     */
    unsigned char WinCaps;
    unsigned char Sockets;
    WINTBL        WinTable[NUM_TYPES];

    /* keep the current setting of window if SS does not */
    SetWindow_S   setWin;
    SetPage_S     setPage;
} Window;

/*
 * Socket Structure
 */
typedef struct {
    int	                AdapterIndex;	/* adapter number (index)	*/
    int	                PhySocket;	/* physical socket number	*/
    int	                LogSocket;	/* logical  socket number	*/

    Complex_lock        rw_lock;        /* complex lock for this struct */
    int                 card_present;   /* \ flags to control           */
    int                 critical_state; /* /   card insertion / removal */

    int   	        exclusive;	/* Status information           */
    int    	        cfg_owner;	/* Client who requested CFG     */
    int    	        irq_owner;	/* Client who requested IRQ     */
    int                 io_owner;       /* Client who requested IO      */
    Window            * io_window1;     /* window handle of io_window1  */
    Window            * io_window2;     /* window handle of io_window2  */

    int                 tuple_window;   /* Physical Window to read tuple*/
    unsigned long	window_base;    /* tuple window base address	*/
    unsigned long	window_size;	/* tuple window size		*/
    
    /*
     * The following Structure includes the configuration
     * information of CSReqCfg, CSReqIRQ and CSReqIO
     */	
    CSGetCfgInfoPkt	cfgInfo;        /* Current Configuration Info   */

    /*
     * Capability of Socket
     */
    SCHARTBL         SktTable;
    unsigned char    SCIntCaps;
    unsigned char    SCRptCaps;
    unsigned char    CtlIndCaps;

    struct trb *pwrofft;                 /* timer for device level PM   */ 
#if defined( PM_ENABLE )
    struct pm_planar_control_handle ppch;/* pm control handler          */
    int client_Vcc;                      /* Vcc specified by client     */
    int client_LED;                      /* LED(on/off) specified by client */
#endif
} Socket;

/*
 * Adapter Structure
 */
typedef struct Adapter{
    struct intr      intr;		  /* This should be the first member */
    int              int_mode;		  /* Interrupt mode: 'I' or 'N' */

    dev_t	     PhyAdapter;	  /* device number (major+minor)*/ 
    struct file      *fp;		  /* fp for fpioctl             */
    int		     (* ddioctl)();  	  /* SS IOCTL entry point	*/
#define SS_LEVEL 0x210
    uint	     compliance;	  /* PCMCIA compliance level	*/
    uint	     release;		  /* SS release in BCD		*/
    uint             ackintr;		  /* ORed results of ackintr	*/

    /*
     * Window address for reading tuple data
     */
    int              mapped_socket;	  /* socket using tuple address */
    unsigned long    window_base;         /* tuple window base address  */
    unsigned long    window_size;	  /* tuple window size 		*/

#define BUS_ISA 5
    unsigned long    bus_id;		  /* bus ID			*/
    ushort           bus_type;		  /* bus type			*/

    /*
     * Capability of Adapter
     */
    ACHARTBL	     CharTable;
    int              numPwrEntries;
    PWRENTRY         PwrEntry[NUM_ENTRIES];
    int	numSockets;	        /* number of sockets	       */ 
    int	numWindows;	        /* number of windows	       */ 
    int	numEDCs;	        /* number of EDCs	       */ 

    Simple_lock win_lock;       /* lock for windows on adapter */
    Socket   *socket;		/* array of Socket Structure   */
    Window   *window;		/* array of Window Structure   */

#if defined( PM_ENABLE )
    struct pm_handle pmh;       /* PM handle struct            */
#endif
    char devname[0x10];         /* logical device name         */ 
} Adapter;


struct cfg_cs_mdi
{
    dev_t   devno;
    ulong   bus_id;
    ulong   waddr;
    ulong   wsize;
    int     irqlvl;
    int     irqpri;
    ulong   bus_type;
    int     dev_itime;
    int     dev_stime;
    char    devname[0x10];
    int     int_mode;
};

/* Flags for tuple scanning routines */
#define TP_LINKSET	0x1
#define TP_NOLINK	0x2
#define TP_COMMON	0x4
#define TP_FINISHED     0x8
#define TP_COMMONLINK  0x10
#define TUPLE_ATTR_RETURN_LINK	  0x01

/* Flags for tuple_search function */
#define TPGetFirst      0x1
#define TPGetNext       0x2
#define TPGetData       0x4
#define TPGetFirstData  ( TPGetFirst | TPGetData )
#define TPGetNextData   ( TPGetNext  | TPGetData )

/* Device Level PM : Idle exparation time */
#define CS_PWROFF_TIME  1

extern int SocketServices( Adapter *, int, caddr_t );
extern void  cs_busio_putc( Adapter *, ulong, uchar );
extern uchar cs_busio_getc( Adapter *, ulong );
extern int AssignWindow( Adapter *, int, ulong, ulong,  SetWindow_S * );
extern int SetPageOffset( Adapter *, int, int, ulong, int );
extern int GetWindow( Adapter *, GetWindow_S *, GetPage_S * );
extern int SetWindow( Adapter *, SetWindow_S *, SetPage_S * );
extern int EnableWindow( Adapter *, SetWindow_S *, SetPage_S * );
extern int DisableWindow( Adapter *, SetWindow_S *, SetPage_S * );
extern int EnableTupleWindow( Adapter *, Socket * );
extern int DisableTupleWindow( Adapter *, Socket * );
extern int tuple_search( CSGetTplPkt *, int );
extern void set_PowerOff_timer( Socket * );
extern void set_activity( Socket * );

#endif /* _H_PCMCIA_CS */
