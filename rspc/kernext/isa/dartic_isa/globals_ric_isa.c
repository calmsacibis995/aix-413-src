/* @(#)29        1.1  src/rspc/kernext/isa/dartic_isa/globals_ric_isa.c, dd_artic, rspc41J, 9508A_notx 2/15/95 14:35:12
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic driver for the ISA bus.
 *
 * FUNCTIONS: darticmpx_isa and all global data declarations
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
   WARNING !!!!!!! The darticmpx routine must NOT be moved from
   this source file. It is used by the pincode routine to pin
   the global data structures into memory.
*/

/*
 *      AIX Include files
 */

#include        <sys/types.h>

/**
 *      ARTIC Diag Include files
 */

#include        <dartic_isa.h>
#include        <darticdd_isa.h>

/**--------------------*
 |   Global Variables  |
 *--------------------*/

/**
 *      The adapter table is used to hold information about each adapter installed
 *      that is being used for ARTIC Diag.  The table is defined in articdd.h, and
 *      contains information about the functional state of the adapter, as well as
 *      the operating parameters for the adapter and the RCM running on it.
 */

DARTIC_Adapter      artic_adapter[MAXADAPTERS];
DARTIC_ISA          artic_isa[MAXADAPTERS];

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

struct DARTIC_Proc  *artic_procptr[ARTICPA_SIZE];

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

ulong   artic_intr_count[MAXADAPTERS];

/**
 * wakeup cookies for e_sleep, e_wakeup.
 */

int             artic_wu_cookie[MAXADAPTERS];

/**  The config_lock lock is used to serialize access to the darticconfig
 *   driver function.  It is unlikely, but since it can be entered by
 *   issuing mkdev from the command line, we must guard against it.             */

lock_t  config_lock = LOCK_AVAIL;


/**  CPUPAGE locks.  These locks are used by the device driver on a per-
 *   adapter basis to serialize access to the adapter's I/O space and
 *   shared memory.                                                                                  */

lock_t  cpupage_lock[MAXADAPTERS];    /* One lock per adapter */
lock_t  pcselect_lock[MAXADAPTERS];   /* for access to PC select byte */


/**      Process Table lock.  This lock is used by the device driver to serialize
 *      access to the artic_procptr array
 */

lock_t  proc_lock = LOCK_AVAIL;                 /* one lock for all processes   */


/**  Jumpbuffers.  These are used with the kernel service "setjmpx" to allow
 *      for unforseen exceptions.  When an exception occurs (like a memory
 *      violation), the default action taken by the kernel is to "assert", which
 *      a nice name for system crash.  By Issuing setjmpx, the kernel will transfer
 *      control to where setjmpx (setjmp/longjmp style) return (now with a
 *      non-zero value).  This way the code can take appropriate action, instead
 *      of allowing AIX to crash.                                                                                       */

label_t config_jumpbuffer;           /* darticconfig jump buffer              */
label_t oaintr_jumpbuffer;           /* darticintr jump buffer                */
label_t open_jumpbuffer;             /* darticopen jump buffer                */

label_t artic_jumpbuffers[MAXADAPTERS];  /* per-adapter jump buffers          */
label_t artic_ioctljumpb[MAXADAPTERS];   /* per-adapter jump buffers          */


/**
 *      artic_switchtable is where we build our switch table prior to passing it
 *      to the kernel via "devswadd".  This is used to install the device driver
 *      entry points into the kernel's device switch tables.
 */

struct devsw    artic_switchtable;


/**  The Interrupt Plus Structure.  This structure is allocated one-per-adapter,
 *      and is used to bind the interrupt handler to the interrupt level selected.
 *      By enclosing the system "intr" structure within our own structure (struct
 *      DARTIC_IntrPlus), we can use the same interrupt handler for all our adapters.
 *      The DARTIC_IntrPlus structure consists of the system intr structure with an
 *      integer that contains the adapter number (card number).  This binding
 *      is done with the kernel service "i_init" in driver function articconfig.*/

struct DARTIC_IntrPlus      artic_intr[MAXADAPTERS];   /* one per adapter     */


/**      Device Driver Load Count.  This counter is used to count the number of
 *      times articconfig was called.  It should be called once-per-adapter,
 *      and artic_loadcount will be incremented accordingly.   When articconfig
 *      is called to terminate the device (once per adapter), artic_loadcount
 *      will be decremented accordingly.  When the loadcount reaches 0, the
 *      device driver is removed from the device switch table.
 */

int     artic_loadcount = 0;     /* device driver instance counter           */


/**
 *      Masks used to set interrupt level in adapter registers
 */
uchar       intr_mask[16] = {
                             0xff,0xff,0x03,0x00,
                             0x01,0xff,0xff,0x02,
                             0xff,0x03,0x04,0x05,
                             0x06,0xff,0xff,0x07};
/*
 *      Empty function.  This allows us to declare the artic device as
 *      "multiplexed".  By doing this, we cause the device close routine
 *      to be called for each user-space close, instead of just the last on
 *      on the device.
 */


darticmpx_isa()
{

        return(NO_ERROR);
}

