static char sccsid[] = "@(#)69  1.10  src/rspc/kernext/pm/pmmd/6030/slave.c, pwrsysx, rspc41J, 9517A_all 4/24/95 08:45:43";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Power Management interrupt(IRQ11) handling (top half)
 *              Slave CPU handling
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/intr.h>
#include <sys/sleep.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#define  _RSPC
#define  _RSPC_UP_PCI
#include <sys/system_rspc.h>
#include <sys/pm.h>
#include "../../pmmi/pmmi.h"
#include "slave.h"
#include "pm_6030.h"
#include "planar.h"


/*
 ************************
 *  external label      *
 ************************
 */
   /* another files */
extern BATTERY_CTRL battery_control         ;/* Wait for battery report comp  */
extern volatile BATTERY_DATA battery_block; /* Battery latest data           */
extern struct   _pm_md_data  pm_md_data;    /* Global data for I/O config    */
extern pm_HW_state_data_t pm_HW_state_data;  /* HW state data */


   /* Bottom half */
extern EVENTCONTROL pm_event_control; /* Master data for event Control block */
extern PMEVENTCELL pm_event_block;     /* PM event buffer                 */

extern ISA_DATA    pm_xioc_data;     /* Slave CPU I/O mapping data block*/
extern ISA_DATA    pm_RTC_data;        /* RTC access data block           */

extern struct intr PMI_block;          /* input data to register IRQ11    */


/*
 ************************
 *--external function---*
 ************************
 */
   /* Another files */
extern int  modify_pmc_nolock(int index, int modify_bits, int data_bits);
extern int  modify_pmc(int index, int modify_bits, int data_bits);
extern int  read_pmc_nolock(int index);
extern int  read_pmc(int index);
extern int  write_pmc_nolock(int index, char data);
extern int  write_pmc(int index, char data);
extern int  pm_planar_control_cpu(dev_t devno, int devid, int cmd);

   /* Bottom half */
extern uchar read_xioc(int index);
extern uchar write_xioc(int index, uchar data);
extern int   read_xioc_status();

extern int   Send_slave_msg(SLAVE_BLOCK *block);

extern void  AlertUserWithBeep(char beep_t, uint beep_interval);

extern int   pm_event_kproc();

extern void  enable_RTC_SQW();

extern  int  pm_intr(struct intr *intrp);

extern struct _pm_planar_id_struct planar_devid; /* Planar devid from ODM */




/*
 ************************
 *  Function proto-type *
 ************************
 */
int         init_interrupt_job(int cmd, struct _pm_planar_id_struct *pmpis);

int         init_unpin_routine(struct _pm_planar_id_struct *pdevid);

int         init_interrupt_handler();

void        init_desired_IO_data();

void        terminate_HostPM();



/*
 ************************
 *   code body          *
 ************************
 *
 **********************************************************
 * NAME: Init_interrupt_job
 *
 * FUNCTION: Initialize PM interrupt
 *             - I_init
 *             - Establish slave interface
 *             - Initialize slave CPU functions
 *             - create and init a kernel process
 *
 * NOTES:  PM interrupt(default) = IRQ11, no sharing,
 *
 * INPUT:  cmd (config/unconfig)
 *         *pmpis (planar devid through ODM scanning)
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURN VALUE: 0: Success
 *              -1: Failure
 *
 **********************************************************
 */
int
init_interrupt_job(int cmd, struct _pm_planar_id_struct *pmpis)
{

        switch (cmd) {
            case PM_CONFIG:

                return(init_unpin_routine(pmpis));

            case PM_UNCONFIG:
                DbgKmode(Slave interface unconfiguration)
                terminate_HostPM();
		delay(10);	/* wait outstanding PM int to complete */ 
		pm_event_control.KillPMMD_Kproc=TRUE;

                /*
                 * Skip if private kproc has not been run yet
                 */
                if (pm_event_control.pmeventkprocpid != NULL) {
			while (pm_event_control.in_sleep == 0);
                        e_wakeup(&(pm_event_control.EventAssertion));
			while (pm_event_control.pmmd_kproc_running == on_run) {
				delay (10);
			} /* endwhile */
                } /* endif */

                if (PMI_block.handler != NULL) {
                        i_clear(&PMI_block);
                        PMI_block.handler = NULL;
                }
                lock_free(&(pm_xioc_data.lock));
                lock_free(&(pm_RTC_data.lock));
        	lock_free(&(pm_HW_state_data.lock));
                return (0);
        }

        /*
         * Error in case of unexpected cmd
         */
        return (-1);
}




/*
 *
 **********************************************************
 * NAME: init_unpin_routine
 *
 * FUNCTION: Slave.c init routine body
 *
 * NOTES:
 *
 * INPUT: *pmpis (planar devid through ODM scanning)
 *
 *
 * EXECUTION ENVIRONMENT: process environment
 *
 * RETURN VALUE: 0: Success
 *              -1: Failure
 *
 **********************************************************
 */
int
init_unpin_routine(struct _pm_planar_id_struct *pdevid)
{
       /*
        *  To use planar_control, the corresponding planar_devids are needed.
        */
        int    t;
        int    *p = (int *)&planar_devid;
        int    *q = (int *)pdevid;

        for (t=1;
             t<=(sizeof(struct _pm_planar_id_struct)/sizeof(int));
             t++) {

             *(p++) = *(q++);

        } /* endfor */


        DbgKmode(Init entry)
        /*
         *  Init control data related to desired IO
         */
        DbgKmode(Comp: init_desired_IO_data)
        (void)init_desired_IO_data();

	/*
	 * lock to serialize slave int 
	 */
        lock_alloc(&(pm_HW_state_data.lock), LOCK_ALLOC_PIN,
                                        PM_CORE_LOCK_CLASS, -1);
        simple_lock_init(&(pm_HW_state_data.lock));

        /*
         *  Create private kernel process
         *     (init wakeup signal)
         */
        DbgKmode(Create private kernel process)
        pm_event_block.EventType = no_event_occurrence;
        pm_event_block.EventDetail=PM_EVENT_NONE;
        pm_event_control.pmeventkprocpid = creatp();

        /*
         *  Enable SQW(RTC) for Power Management Controller
         */
        DbgKmode(Enable RTC SQW)
        (void)enable_RTC_SQW();

        /*
         *  init private kernel process
         */
        DbgKmode(Init private kernel process)
        pm_event_block.EventType = NULL;
        if (initp  (pm_event_control.pmeventkprocpid,
                    pm_event_kproc,
                    0,
                    0,
                    "PMMD") != 0)  return (-1);
        /*
         *  I_INIT
         *  INIT_H8_interface
         *  INIT_H8
         */
        DbgKmode(Init interrupt handler & init slave interface)
        if ((init_interrupt_handler() != INTR_SUCC) ||
                        (init_slave_interface() != 0)) {
               DbgKmode(Error:
                       init_interrupt_handler or init_slave_interface)
               terminate_HostPM();
               if (PMI_block.handler != NULL) {
                       i_clear(&PMI_block);
                       PMI_block.handler = NULL;
               }
               return (-1);
        }

        return (0);
}



/*
 **********************************************************
 * NAME: init_desired_IO_data
 *
 * FUNCTION: Build control data struct to allocate I/O device space
 *
 * NOTES:  Init data related to IO access control block
 *
 * INPUT:  none
 *
 * EXECUTION ENVIRONMENT: process enviroment
 *
 * RETURN VALUE: none
 *
 **********************************************************
 */
static void
init_desired_IO_data()
{
        struct rspcsio *p;

   /* eXtended I/O Controller IO data block */
        lock_alloc(&(pm_xioc_data.lock), LOCK_ALLOC_PIN,
                                        PM_CORE_LOCK_CLASS, -1);
        simple_lock_init(&(pm_xioc_data.lock));
        pm_xioc_data.base      = pm_md_data.xioc_base;     /* from ODM */
        pm_xioc_data.bus_id    = pm_md_data.xioc_bus_id;   /* from ODM */
        pm_xioc_data.map.key   = IO_MEM_MAP;
        pm_xioc_data.map.flags = 0;
        pm_xioc_data.map.size  = SEGSIZE;
        pm_xioc_data.map.bid   = pm_xioc_data.bus_id;
        pm_xioc_data.map.busaddr = 0;


   /* RTC IO data block   */
        lock_alloc(&(pm_RTC_data.lock), LOCK_ALLOC_PIN,
                                        PM_CORE_LOCK_CLASS, -1);
        simple_lock_init(&(pm_RTC_data.lock));
        pm_RTC_data.base         = (&(p->rtc_index) - (char *)p);
        pm_RTC_data.bus_id       = pm_md_data.xioc_bus_id;   /* ISA bus ID */
        pm_RTC_data.map.key      = IO_MEM_MAP;
        pm_RTC_data.map.flags    = 0;
        pm_RTC_data.map.size     = SEGSIZE;
        pm_RTC_data.map.bid      = pm_RTC_data.bus_id;
        pm_RTC_data.map.busaddr  = 0;

}



/*
 **********************************************************
 * NAME: init_interrupt_handler
 *
 * FUNCTION: Register PM interrupt (Default: IRQ11)
 *
 * NOTES:
 *
 * INPUT:  none
 *
 * EXECUTION ENVIRONMENT: process enviroment
 *
 * RETURN VALUE: 0: Success
 *              -1: Fauilure
 *
 **********************************************************
 */
int
init_interrupt_handler()
{
        DbgKmode(init interrupt entry)

        PMI_block.next     = NULL;
        PMI_block.handler  = (int (*) ())pm_intr;
        PMI_block.bus_type = BUS_BID;
        PMI_block.flags    = 0;
        PMI_block.level    = pm_md_data.intr_data.level;    /* from ODM */
        PMI_block.priority = pm_md_data.intr_data.priority; /* from ODM */
        PMI_block.bid      = pm_xioc_data.bus_id;         /* from ODM */



        if (i_init(&PMI_block) != INTR_SUCC) {
            PMI_block.handler = NULL;
            return -1;
        }

        return 0;
}



/*
 **********************************************************
 * NAME: terminate_HostPM
 *
 * FUNCTION: Unconfigure PMMD (features related to slave cpu)
 *
 * NOTES: Slave cpu interrupt is masked.
 *        Power management chip is disabled.
 *
 * INPUT:  none
 *
 * EXECUTION ENVIRONMENT: process enviroment
 *
 * RETURN VALUE: none
 *
 **********************************************************
 */
void
terminate_HostPM()
{
	/* Don't clear Carrera Mast enable flag so that LCD panel control
	 * signals can be still controllable by host. If the flag would be
	 * cleared, those signal is under the control of slave CPU.
	 */	
        write_xioc(slave_BCR,0x00);
}




/*------------------ End of file ------------------------------*/

