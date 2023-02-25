static char sccsid[] = "@(#)95  1.8  src/rspc/kernext/pm/pmmd/6030/pmcore.c, pwrsysx, rspc41J, 9520A_all 5/12/95 12:48:24";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: 
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

#include <sys/sleep.h>
#include <sys/syspest.h>
#include <sys/lock_def.h>
#include <procinfo.h>

#include <sys/pm.h>
#include <sys/pmdev.h>
#include "../../pmmi/pmmi.h"
#include "pm_6030.h"
#include "slave.h"

#include "pdplanar.h"
#include "pdpower.h"
#include "pdmemctl.h"
#include "pdcommon.h"
#include "pdcooper.h"




/*------------------------*/
/*  PM core data 	  */
/*------------------------*/
callback_entry_block_t	callback_entry_block = {0};

extern 	pm_parameters_t	pm_parameters; 	
extern	pm_control_data_t pm_control_data;
extern 	event_query_control_t event_query_control;
extern pmmd_IF_t pmmd_IF;
extern pmmd_control_data_t pmmd_control_data;
extern OBJCOOPERIO  oCooperIo; 


/*------------------------*/
/* external functions     */ 
/*------------------------*/
extern  int 	pmmd_job(int cmd, caddr_t arg);
extern	int	pm_register_restart_information(void *);
extern 	void 	Reinit_SysTime();


/*------------------------*/
/* Proto type declaration */
/*------------------------*/
void PmPowerOffMachine();
VOID PmSuspendMachine();
uchar ReadVariableSCSIID();
void RetrieveVariableSCSIID(uchar setupID);

/*
 * NAME: transition_core_job
 *
 * FUNCTION: core job for suspend/hibernation
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is run in a very specific environment since 
 *	the CPU register context are handled to save. Page fault
 *	can't be allowed. Any other modules can't be run at the
 *	same time. Mostly, MSR.EE flag is internally cleared. It 
 *      means all of the external interrupt are disabled.  This 
 *      is run in isolating to any other program modules.
 *
 * NOTES: CPU context is transparently preserved over suspend/
 *	hibernation situation. Stack area are also manipurated 
 *	in assembler code level internally.
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS - Successfully processed
 *        PM_ERROR -   Procedure is terminated due to a fatal error. 
 */
void
transition_core_job(int opt)
{
	uchar	DiskSCSIID;

   	pm_map_iospace();

	/* 
	 * Need to save this variable IDregister. This must be retrieved 
	 * within 1.3 sec after resume power-on. 
  	 * In addition, this operation doesn't make any sense for hibernation 
	 * since SCSI ID will be setup by ROS correctly at power-on.
	 */
	DiskSCSIID =  ReadVariableSCSIID(); 	

	if (opt == PM_SYSTEM_SUSPEND) {
	DBG(Calling PmSystemSuspendHibernate() for suspend)
		PmSystemSuspendHibernate(
				pm_register_restart_information, 
				PmSuspendMachine);
	} else {
	DBG(Calling PmSystemSuspendHibernate() for hibernation)
		PmSystemSuspendHibernate(
				callback_entry_block.hibernation_entry,
				PmPowerOffMachine);
	} /* endif */


	/* Woodfield/Wiltwick 2.5inch HDD supports software variable SCSI
	 * ID. However, this input port for ID is only sampled 1.3 sec 
         * after power-on which occurs at normal power-on and resume from 
 	 * suspend. So our resume code needs to retrieve the register for
 	 * this ID as soon as possible. So here is a call to the restore 
	 * routine.
	 */
	(void)RetrieveVariableSCSIID(DiskSCSIID); 	

	pm_unmap_iospace(); 

	DumpOut(0x06)
	return;

	/* Note that "prinf" for debug has not been available yet 
	 * at this point because "restore_save_device" is not called 
	 * so that serial is still dead.
         */

} /* transition_core_job */



/*
 * NAME: callback_addr_job
 *
 * FUNCTION: The callback address for the entries to PMMI are transferred
 *	     from PMMI to PMMD. PMMD must maintain them so that PMMD can
 *	     call PMMI through a certain entry point.
 *
 * NOTES:
 *	This callback address contributes to hide the difference between
 *	suspend and hibernation from the view point of the low level 
 * 	module of PMMD.  
 *
 * RETURN VALUE DESCRIPTION:
 *	   	none
 */
void
callback_addr_job(callback_entry_block_t *arg)
{ 
	callback_entry_block.pmmi_event_receive_entry 
				= arg->pmmi_event_receive_entry;
	callback_entry_block.hibernation_entry 
				= arg->hibernation_entry;
	return;
}



/*
 * NAME: establish_pmmi_interface 
 *
 * FUNCTION: initialize the interface between pmmi and pmmd 
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES: This routine is called at PMMD configuration. The purpose of
 * 	  this routine is to establish the interface from PMMI to PMMD,
 *	  so that PMMI can calls PMMD. Because PMMI can't refer the label
 *	  of PMMD due to the configuration order.
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS - Successfully processed
 *        PM_ERROR - Invalid cmd argument or falied to process
 *                   battery functions.
 */
int
establish_pmmi_interface(int cmd)
{
	switch (cmd) {
	case PM_CONFIG:
	DBG(Establish PMMI interface)
		pmmd_IF.pmmd_entry = (int(*)())pmmd_job;
		return (PM_SUCCESS);

	case PM_UNCONFIG:
		/* 
		 * When coming here, the lock of pmmd_IF.lock was obtained.
		 */
		pmmd_IF.pmmd_entry = NULL; 
		return (PM_SUCCESS);

	}   /* endswitch */
	return (PM_ERROR);
}


/*
 * NAME: PmPowerOffMachine
 *
 * FUNCTION: Turn the machine off.  This fuction is called after 
 *	     hibernation memory saving completes.
 *	    
 * RETURN VALUE DESCRIPTION:
 *	     none because of no return 
 */
void
PmPowerOffMachine()
{
	/*
	 * L1/L2 cache flush is not required for hibernation. Because
	 * eventually, all of the memory are read and carried to 
	 * disk bus master buffer of SCSI where copy-back is guaranteed.
 	 */
	EnableInterrupt();	/* For timer to make a beep interval. */
	pm_planar_control_psumain(NULL,NULL,PM_PLANAR_OFF);	

} /* PowerOffMachine */



/*
 * NAME: PmSuspendMachine
 *
 * FUNCTION: Turn the part of the machine power. Most of the power for
 *	    all the devices other than system memory/memory ctrl/PM logic
 *	    are turn off here. This function is called after restart address
 *	    is saved into NVRAM for suspend/resume. 
 *
 * NOTES: This routine is called at PMMD configuration. The purpose of
 * 	  this routine is to establish the interface from PMMI to PMMD,
 *	  so that PMMI can calls PMMD. Because PMMI can't refer the label
 *	  of PMMD due to the configuration order.
 *
 * RETURN VALUE DESCRIPTION:
 *	  none
 */
VOID  
PmSuspendMachine()
{
   MSGMEMORY   mMem;
   MSGPOWER    mPwr;
   MSGCOOPER   mCoop; /* @@@ */

   /* 
    * Control LED right before turning CPU off 
    */
   EnableInterrupt();
   ChangeLEDstate(Power_LED_Device, TurnOffLED, NULL);
   ChangeLEDstate(Suspend_LED_device, TurnOnLED, NULL);
   io_delay(1000*50);


   /* Set low-power mode to memory controller */
   /*                                         */
   BuildMsg(mMem, MSGMEM_ENTER_SUSPEND);
   SendMsg(mMem, oMemoryCtl);

   /* 
    * If dcrementor expires after MSR:EE is cleared(interrupt disabled),
    * 603 doesn't assert QREQ signal even after entering its sleep mode.
    * This is a bug of 603. QREQ is used to synchronize Vcc-B off with 
    * the completion of L1 write back cache. So we have to set DEC register 
    * to a pretty big value so as not to expire within the operation until
    * Vcc-B is actually turned off. 
    * The following operation, "DisableDecrementer()" is called twice to
    * ensure that the DEC register can surely have a desired value. 
    * Because decrementor exception may occur at the same time the following
    * write access to DEC is performed. If the expiratin occurs, the 
    * interrupt request is not cleared until the corresponding exception
    * is serviced. So calling the "DisableDecrementer()" must be done in
    * the conditin of interrupt enabled. Once the original exception 
    * handler is serviced, it's ensured that the next access to DEC in
    * the second DisableDecrementer() can be executed without any possibily
    * of having next decrementor exception in normal case. 
    */
   DisableDecrementer();
   DisableDecrementer();


    DBG(=================================================)
    DBG(     Clean up the dirty lines of L1/L2 cache     )
    DBG(=================================================)
    /* Before turning CPU off, L1 cache must be flushed and the contents
     * of L1 cache must be stored into memory.
     * If L2 cache is write-back fashion, it must be also flushed. Here,
     * the memory which was allocated through xmalloc is block-read
     * so that all of the existing contents will be flushed into memory.
     */
    DisableInterrupt();
    GetTLBMissIfAny(); 
    L1FlushAndDisable(); 
/*
  In case of write-back L2 cache, it has been discovered that reading
  the memory whose size is equal to L2 cache can't flush the L2 cache
  correctly. Because TLB exception seems to occur during the reading.
  So the data in L2 cache can't be discarded even after the completion
  of the reading the memory.

    FlushL2Cache(pm_control_data.MemForL2Cache);
*/

   /* 
    * The completion of L2 cache flash can be ensured by having the 
    * following IO operation achieved correctly.
    * So IO operation of turning Vcc-B ensures the completion of
    * L2 cache flash.
    */
    BuildMsg(mCoop, MSGCOOP_FLUSHDISABLE_L2CACHE); 
    SendMsg(mCoop, oCooperCtl);                   

   /*---------------------------------------------------------------------
    *  After this point, it's not guarantted that any memory-write data
    *  is correctly registered into not only cache, but also actual memory.
    *---------------------------------------------------------------------
    */


    DBG(=================================================)
    DBG(     	   Vcc-B off operation  		 )
    DBG(=================================================)
   /* Power-off to enter the suspend state */
   /*                                      */
   BuildMsgWithParm1(mPwr, MSGPWR_SET_SUSPENDPOWER,
                           PRMPWR_POWEROFF);
   SendMsg(mPwr, oPowerCtl);

   return;
}


/*
 * NAME: ReadVariableSCSIID
 *
 * FUNCTION: read the register for HDD scsi ID 
 *	    
 * INPUT:    none 
 * 
 * RETURN VALUE DESCRIPTION:
 *	     uchar SCSI ID
 *	      
 */
uchar
ReadVariableSCSIID()
{
   MSGCOOPER      msg;

   BuildMsg(msg, MSGCOOP_READ_SCSIIDSETUP);
   SendMsg(msg, oCooperIo);
   return (SelectParm1(msg));
}

/*
 * NAME: RetrieveVariableSCSIID
 *
 * FUNCTION: write the input ID into the register for HDD scsi ID 
 *	    
 * INPUT: uchar SCSI_ID
 *
 * RETURN VALUE DESCRIPTION:
 *	     none 
 */
void
RetrieveVariableSCSIID(uchar setupID)
{
   MSGCOOPER      msg;

   BuildMsgWithParm1(msg, MSGCOOP_WRITE_SCSIIDSETUP, setupID);
   SendMsg(msg, oCooperIo);
   return;
}

