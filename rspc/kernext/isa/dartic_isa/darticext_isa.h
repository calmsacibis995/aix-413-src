/* @(#)36        1.1  src/rspc/kernext/isa/dartic_isa/darticext_isa.h, dd_artic, rspc41J, 9508A_notx 2/15/95 14:35:29
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic driver.
 *
 * FUNCTIONS: External data and function references
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 *      AIX Include files
 */

#include        <sys/types.h>
#include        <sys/param.h>
#include        <sys/signal.h>
#include        <sys/conf.h>
#include        <sys/proc.h>
#include        <sys/sysmacros.h>
#include        <sys/errno.h>
#include        <sys/systm.h>
#include        <sys/uio.h>
#include        <sys/device.h>
#include        <sys/adspace.h>
#include        <sys/ioctl.h>
#include        <sys/ioacc.h>
#include        <sys/iocc.h>
#include        <sys/pin.h>
#include        <sys/malloc.h>
#include        <sys/lockl.h>
#include        <sys/watchdog.h>
#include        <sys/poll.h>
#include        <sys/sleep.h>
#include        <sys/xmem.h>
#include        <sys/inline.h>

/**
 *      ARTIC Diag Include files
 */

#include        "dartic_isa.h"
#include        "darticdd_isa.h"

/**------------------------------*
 |   Global Variable references  |
 *-------------------------------*/

/**
 *      The adapter table is used to hold information about each adapter installed
 *      that is being used for ARTIC Diag.  The table is defined in articdd.h, and
 *      contains information about the functional state of the adapter, as well as
 *      the operating parameters for the adapter and the RCM running on it.
 */

extern DARTIC_Adapter artic_adapter[];
extern DARTIC_ISA     artic_isa[];

/**
 *      A process table entry is allocated for a process when articopen is called.
 *      Only one open per process is allowed.  The DARTIC_Proc structure contains the
 *      pointers to the per-adapter pointer arrays to linked lists of ProcReg
 *      structures.
 *
 *      The process table uses a primitive hash method for access.  The artic_procptr
 *      array of pointers point to the first structure of a linked list.  Criteria
 *      for being on the list are the last log2(ARTICPA_SIZE) bits of the Process ID.
 *      Thus to find the index of the pointer for a process, use:
 *
 *                      #define ARTICPA_SIZE     some_power_of_2
 *                      #define ARTICPA_MASK     (ARTICPA_SIZE - 1)
 *
 *                              index = PID & ARTICPA_MASK;
 */

extern struct DARTIC_Proc  *artic_procptr[];

/**
 *      The artic_intr_count array is used to count interrupts from adapter
 *      tasks.  There is one counter for each task/adapter combination.
 *      These counters are incremented by the interrupt handler ricdintr
 *      once the task and adapter numbers have been validated.
 *      This driver only supports Task 0.
 *
 *                              Interrupt Register
 *                              Interrupt Wait
 *                              Interrupt Deregister                                                                    */

extern ulong  artic_intr_count[];

/**
 * wakeup cookies for e_sleep, e_wakeup.
 */

extern int artic_wu_cookie[];

/**  The config_lock lock is used to serialize access to the darticconfig
 *   driver function.  It is unlikely, but since it can be entered by
 *   issuing mkdev from the command line, we must guard against it.             */

extern lock_t config_lock;


/**  CPUPAGE locks.  These locks are used by the device driver on a per-
 *   adapter basis to serialize access to the adapter's I/O space and
 *   shared memory.                                                                                  */

extern lock_t  cpupage_lock[];    /* One lock per adapter */
extern lock_t  pcselect_lock[];   /* for access to PC select byte */


/**      Process Table lock.  This lock is used by the device driver to serialize
 *      access to the artic_procptr array
 */

extern lock_t proc_lock;          /* one lock for all processes   */


/**  Jumpbuffers.  These are used with the kernel service "setjmpx" to allow
 *      for unforseen exceptions.  When an exception occurs (like a memory
 *      violation), the default action taken by the kernel is to "assert", which
 *      a nice name for system crash.  By Issuing setjmpx, the kernel will transfer
 *      control to where setjmpx (setjmp/longjmp style) return (now with a
 *      non-zero value).  This way the code can take appropriate action, instead
 *      of allowing AIX to crash.                                                                                       */

extern label_t config_jumpbuffer;    /* darticconfig jump buffer              */
extern label_t oaintr_jumpbuffer;    /* darticintr jump buffer                */
extern label_t open_jumpbuffer;      /* darticopen jump buffer                */

extern label_t artic_jumpbuffers[];  /* per-adapter jump buffers          */
extern label_t artic_ioctljumpb[];   /* per-adapter jump buffers          */


/**
 *      artic_switchtable is where we build our switch table prior to passing it
 *      to the kernel via "devswadd".  This is used to install the device driver
 *      entry points into the kernel's device switch tables.
 */

extern struct devsw artic_switchtable;


/**  The Interrupt Plus Structure.  This structure is allocated one-per-adapter,
 *      and is used to bind the interrupt handler to the interrupt level selected.
 *      By enclosing the system "intr" structure within our own structure (struct
 *      DARTIC_IntrPlus), we can use the same interrupt handler for all our adapters.
 *      The DARTIC_IntrPlus structure consists of the system intr structure with an
 *      integer that contains the adapter number (card number).  This binding
 *      is done with the kernel service "i_init" in driver function articconfig.*/

extern struct DARTIC_IntrPlus artic_intr[];   /* one per adapter     */


/**      Device Driver Load Count.  This counter is used to count the number of
 *      times articconfig was called.  It should be called once-per-adapter,
 *      and artic_loadcount will be incremented accordingly.   When articconfig
 *      is called to terminate the device (once per adapter), artic_loadcount
 *      will be decremented accordingly.  When the loadcount reaches 0, the
 *      device driver is removed from the device switch table.
 */

extern int artic_loadcount;     /* device driver instance counter           */

/**
 *      Masks used to set interrupt level in adapter registers
 */

extern uchar intr_mask[];

/**
 *      External Function Declarations
 */

extern  int nodev();

/**
 *      Function Declarations
 */

void intboard();
int  darticconfig_isa();
int  darticclose_isa();
int  darticioctl_isa();
int  darticopen_isa();
int  darticintr_isa();
int  darticmpx_isa();


struct DARTIC_Proc *remove_articproc();
struct DARTIC_Proc *lookup_articproc();

uchar   setCPUpage();
uchar   readdreg();
ushort  to16local();
char   *inteltolocal();
void    artic_time_func();
void    artic_stime_func();
int     artic_read_mem();
void    cleanup_for_return();
void    outbyte_isa();
uchar   inbyte_isa();
void    artic_initialize();

