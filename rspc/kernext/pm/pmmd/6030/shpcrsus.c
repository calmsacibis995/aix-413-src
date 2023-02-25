static char sccsid[] = "@(#)01  1.5  src/rspc/kernext/pm/pmmd/6030/shpcrsus.c, pwrsysx, rspc41J, 9520A_all 5/12/95 10:08:13";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: shpcrsus.c
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
Abstract:

    This module contains the main routines for freezing the processor and
    suspending the system.
    Currently all routines are applied to PowerPC 603 only.

*/


#include <sys/adspace.h>
#include "pdplanar.h"
#include "pm_6030.h"
#include "../../pmmi/pmmi.h"
#include "shpcrsus.h"
#include "pdcommon.h"
#include "pdcooper.h"



int  
PmSystemSuspendHibernate(void *  WhereTogo, void * HowToControlPower);

int
ProcessorResetFromAsmToC (VOID);

VOID
PrepareFreezeProcessor (VOID);

VOID
PostFreezeProcessor (VOID);

VOID
MoveContiguousArea (VOID);

int
virtual2physical(void * addr);



PMSusHibCB SusHibCB;    /* Suspend/hibernation sequence control block */ 

extern	pm_control_data_t    pm_control_data;
extern  pmmd_control_data_t  pmmd_control_data;


/*
 * NAME: PmSystemSuspendHibernate
 *
 * FUNCTION:  Suspend/hibernation core routine
 *	      There are four stages. The following is the summary.
 *
 *		1). Preparare some key address for callback. 
 *		    Page boundary check of resume code running on real 
 *		    address space. 
 *	        2). CPU register save (asm code)
 *	        3). Callback routine  
 *			- suspend:
 *				register return address into NVRAM
 *			- hibernation:
 *				hibernation entry(meory is dumped into disk.)
 *		4). return from callback routine 
 *			- suspend:
 *				Turn Vcc-B off 
 *				(memory/mem-ctrl/PM logic are still alive.)
 *			- hibernation:
 *				Turn the whole system off
 *
 * NOTES:  As for each routine mentioned above, (2) is asm code and called 
 * 	   by (1). (3) is C code. (3) is called by (2). (3) calls callback 
 *	   entry which is beforehand given in global data structure(SusHibCB). 
 *	   (3) returns to (4). 
 *
 *
 * INPUT: int *WhereTogo --- callback address for the stage (3) above 
 *        void *HowToControlPower --- callback address for the stage (4) above 
 *
 * RETURN VALUE DESCRIPTION:
 *        No return if success 
 *		No error in case of suspend
 *
 *        return with one of the following errors in hibernation operation 
 *	     
 * 		INVALID_HIBERNATION_VOLUME      
 *		NO_EXECUTABLE_WAKEUPCODE       
 *		HIBERNATION_GENERAL_ERROR     
 *
 */
int 
PmSystemSuspendHibernate(void *WhereToGo, void *HowToControlPower)
{
    int  rc;
    ULONG size,entry;

   /* 
    * Callback address is transferred
    */
    SusHibCB.SuspendHibernationCallback = (int (*)())WhereToGo;
    SusHibCB.PowerControlCallback = (void (*)())HowToControlPower;

    SusHibCB.ContiguousAreaShift=0;

    entry=(ULONG)(GetRelocatableCodeStart());     
    GetContiguousCodeSize(&entry,&size,0,0);


   /* Size should be less than PAGE_SIZE/2. Because PAGE_SIZE/2 is prepared
    * for this alternative area. 
    */
    if (size>=PAGE_SIZE/2) {
       DBG(<<<<< SUSPEND/HIBERNATION ABORTED >>>>>)
       return;
    } else {
       if (((entry)%(PAGE_SIZE)+size)>=PAGE_SIZE) {
          SusHibCB.ContiguousAreaShift=size;
       }

       PrepareFreezeProcessor();
       DumpRegisters();


       /* Call ASM routine */
       rc = ProcessorReset(NULL);  /* Dummy input to get a working register */


       PostFreezeProcessor();

       /* MSR DPM */
       SetHid0Dpm();

       DumpOut(0x01)
       return(rc);
    } /* endif */

} /* PmSystemSuspendHibernate */





int
ProcessorResetFromAsmToC (
    VOID
    )

/*

Routine Description:

    This routine is called-backed by ASM routine which is called by
    PmSystemSuspendHibernate.  All of the processor states have been 
    already saved before coming here. 
    
    At this moment, L1 Cache is disabled, L2 cache is enabled. MSR.EE 
    is cleared.
 
    Although this routine doesn't distinguish the difference between
    suspend and hibernation, as a result, one of them is called here.

    The only things to be done in this routine are;
    
    (In case of suspend:)    

    - Inform the firmware of "ResumeEntry" address via NVRAM etc.
    - Turn Vcc-B off by manipulating a machine unique hardware 
         (Vcc-B supplies the power of all the logics other than system 
   	  memory, its memory control, slave CPU and PM related logic.)

    (In case of hibernation:)

    - Callback hibernation entry in PMMI to save all the memory into disk
    - Ask for slave processor to turn the whole system off. 


Return Value:

    None.(No exit)

*/

{
    MSGCOOPER   mCoop; 

    DBG(=================================================)
    DBG(  Check if wakeup code is in page boundary       )
    DBG(=================================================)
    /* If the resume code running on xlation off is located accross
     * 4 kpage boundary, it must be moved so that it is with in 4 kb
     * page.
     * Note that this operation must be performed with L1 cache disabled.
     * Because instruction area results in being modified.
     */
    if (SusHibCB.ContiguousAreaShift != 0) {
       DBG(Call MoveContiguousArea() due to wakeup code accross page boundary)
       MoveContiguousArea();
    }


    DBG(=================================================)
    DBG(   	Re-enable L1 cache again here.		 )
    DBG(=================================================)
    /* For speed-up of the operation of hibernation. L1 cache 
     * is re-enabled again here. Right before Vcc-B is turned off,
     * L1 cache will be disabled and flushed again later.
     * Since debug-printf contains i_enable, disabling interrupt(MSR.EE)
     * is explicitly requred here.
     * Note that "ReenableCache()" invalidates L1 cache lines forcedly
     * before enabling L1 cache. 
     */ 
    DisableInterrupt();
    ReEnableCache();  


    DBG(=================================================)
    DBG(   Callback routine for suspend or hibernation   )
    DBG(=================================================)
    /* call suspend or hibernation pointer for state transition
     * The pointer is for NVRAM write routine in case of suspend, while
     * it is for Hibernation callback routin in case of hibernation.
     * Hibernation callback routine is beforehand send from PMMI on the 
     * second init caused by pm_system_event_query system call of PM daemon. 
     * The input point is the restart address to retrieve the CPU
     * context and then jump back to the caller of suspend/hibernation
     * in pmmi.
     */


    /* writing here is effective because this operation is eventually reflected
     * in case of the successfull completion of saving memory into disk.
     */
    pmmd_control_data.resume_trigger = RESUMEwSUCCESS;  

    /* On the other hand, writing here is NOT effective in case of the      
     * successful completion of saving memory into disk. Because saving 
     * memory has already finished, so the data written here will be gone.
     * Only in the case of the failure of saving memory, writing return 
     * code here will become effective. So the return code other than
     * "RESUMEwSUCCESS" is only used effectively here. 
     */
    pmmd_control_data.resume_trigger 
		= (SusHibCB.SuspendHibernationCallback)(SusHibCB.ResumeEntry);

    if (pmmd_control_data.resume_trigger != RESUMEwSUCCESS) {

		DisableInterrupt();	
		L1FlushAndDisable();

		/* L2 cache is still enabled here. And returning with
		 * error code from here, the code which assumes L2 cache
		 * is disabled at power-on default after resume from suspend
		 * will be run. So clearing memory inconsistency is required
		 * here beforehand.
		 */
    		BuildMsg(mCoop, MSGCOOP_FLUSHDISABLE_L2CACHE); 
    		SendMsg(mCoop, oCooperCtl);                   

		/* Re-enable L1 again. Disabling L1 during L2 flush 
		 * operation is required.
                 * Interrupt is still disabled going back through "return".
                 * Because the code which is about to be run expects it.
                 */
    		ReEnableCache();  

    		DBG(Error of callback of suspend or hibernation)
		return;
    } /* endif */


    /* (Debug purpose routines) 
     * This entry is usefull for debug suspend/resume. So intentionally
     * remains even in the formal code.
     */
    /*
     *	DisableInterrupt();
     *  ShortCutSuspend(SusHibCB.ResumeEntry);
     */


    DBG(=================================================)
    DBG(   Power Control for suspend or hibernation      )
    DBG(=================================================)
    /* Vcc-B off(suspend) or Main power off(hibernation) */
    (SusHibCB.PowerControlCallback)(); 


} /* ProcessorResetFromAsmToC */ 




/*
 * NAME: PrepareFreezeProcessor 
 *
 * FUNCTION: This routine will be done before the processor freezing
 *   	     in order to register the wakeup code jump address
 *
 * RETURN VALUE DESCRIPTION:
 *	  none
 */
VOID
PrepareFreezeProcessor (VOID)
{
    PVOID EntryAddress;

    /*
     * Register entry address after setting translation on at resume.
     * Saving area for the register must be within 4 kb page with the
     * wakeup code together. 
     */
    RegisterReturnToVirtualEntry(GetReturnToVirtualEntry());

    EntryAddress = GetResumeEntry()+SusHibCB.ContiguousAreaShift;
    SusHibCB.ResumeEntry = (void *)virtual2physical((void *)EntryAddress);

#ifdef PM_DEBUG
    printf("Logical Resume Entry(%08xL)\n",EntryAddress);
    printf("Physical Resume Entry(%08xP)\n",SusHibCB.ResumeEntry); 
#endif

} /* PrepareFreezeProcessor */



/*
 * NAME: PostFreezeProcessor 
 *
 * FUNCTION: This routine will be done after the system resume
 *           in order to clean-up the wakeup code jump address
 *
 * RETURN VALUE DESCRIPTION:
 *	  none
 */
VOID
PostFreezeProcessor (VOID)
{
    RegisterReturnToVirtualEntry(0);

} /* PostFreezeProcessor */



/*
 * NAME: MoveContiguousArea 
 * 
 * FUNCTION: This routine moves the contiguous area to the alternative area.
 *   	     Saved cpu context data should be moved, too.
 *
 * RETURN VALUE DESCRIPTION:
 *	  none
 */
VOID 
MoveContiguousArea (VOID)
{
    ULONG size,entry;
    PUCHAR srcaddr,desaddr,start,end;

    GetContiguousCodeSize(&entry,&size,0,0);
    start=(PUCHAR)(entry);
    end=(PUCHAR)(entry+size);
    desaddr=(PUCHAR)(entry+SusHibCB.ContiguousAreaShift);
    for (srcaddr = start ; srcaddr < end ; ) {
       *desaddr++=*srcaddr++;
    }
} /* MoveContiguousArea */



/*
 * NAME: virtual2physical
 * 
 * FUNCTION: This routine moves the contiguous area to the alternative area.
 *   	     Saved cpu context data should be moved, too.
 *
 * RETURN VALUE DESCRIPTION:
 *	  none
 */
int
virtual2physical(void * addr)
{
   int         phys;
   struct xmem x;

   x.aspace_id = XMEM_GLOBAL;
   phys = xmemdma(&x, addr, XMEM_HIDE);

   return phys;
}


