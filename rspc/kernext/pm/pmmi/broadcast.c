static char sccsid[] = "@(#)03  1.7  src/rspc/kernext/pm/pmmi/broadcast.c, pwrsysx, rspc41J, 9520A_all 5/16/95 11:12:28";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: broadcast2DD
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

#include <sys/errno.h>
#include <sys/lock_def.h>

#include <sys/pm.h>
#include "pmmi.h"

/*---------------------*/
/*  System global      */
/*---------------------*/
extern	caddr_t	pinned_heap;

/*---------------------*/
/*  PM core data       */
/*---------------------*/
extern pm_parameters_t pm_parameters;
extern pm_control_data_t pm_control_data;
extern struct _pm_kernel_data pm_kernel_data;
extern int still_screen_dim;
extern struct _pm_core_data  pm_core_data;

/*---------------------*/
/* function proto type */
/*---------------------*/
int broadcast2DD(broadcast_block_t *bc_blk);
void retrieve_PM_aware_DD(broadcast_block_t *pbd);
int circulatePMinformation(broadcast_block_t *pbd);
void setPMDDwAttrib(broadcast_block_t *pbd);
void setSystemStandby();
int ScanPMawareDD(dev_t * devno_of_Phase1PMDD);
int get_NumberOfAwareDD(); 
void circulateResumeNotice(broadcast_block_t *pbd);


#ifdef PM_DEBUG
/*-------------------------*/
/* Debug for broadcasting  */ 
/*-------------------------*/
   /* 
    * This "ignoreID" field is useful to omit the relationship to the
    * PM aware DD which just bothers PM core normal operation.
    */
   char * ignoreID = "xXx";
/*
   char * ignoreID = "mouse0";
*/
#endif


/*
 * NAME:  broadcast2DD
 *
 * FUNCTION:  PM event/notice are broadcasted to all or some of PM aware DD.
 *
 * NOTES:
 *      This routine should not be called regardless of consideration to
 *      power management operation. This can be run in very strict env.
 *
 * DATA STRUCTURES:
 *      *bc_blk    - variety of requested device mode or notice (input)
 *                 - device logical name of failed device (output)
 *
 * RETURN VALUE DESCRIPTION:
 *      BROADCAST_COMPLETE - SUCCESS
 *      BROADCAST_TERMINATED - ERROR
 */
int 
broadcast2DD(broadcast_block_t *bc_blk)
{
   switch (bc_blk->control) {
   case PM_DEVICE_FULL_ON:             	/* Setting PM disable (full-ON) */ 
   case PM_DEVICE_ENABLE:    		/* coming back from full-on */

	(void)circulatePMinformation(bc_blk);
	break;
		
   case PM_PAGE_FREEZE_NOTICE:     	/* Preparation for entering sus/hib */
	
	if (circulatePMinformation(bc_blk) == PM_ERROR) {
		(void)retrieve_PM_aware_DD(bc_blk);
		return PM_ERROR;
	}
	break;
 
   case PM_DEVICE_SUSPEND:            	/* suspend broadcasting */
   case PM_DEVICE_HIBERNATION:          /* hibernation broadcasting */
	
	if (circulatePMinformation(bc_blk) == PM_ERROR) {
		/* 
		 * In case of rejection, user process is restarted then. 
		 */
		(void)pm_proc_start();	

		/* 
		 * Enable or idle is informed first. And then, unfreeze
		 * is informed, too.
		 */
		(void)retrieve_PM_aware_DD(bc_blk);
		return PM_ERROR;
	}
	break;
 
   case PM_DEVICE_DPMS_STANDBY:	  /* only for graphical output DD*/
   case PM_DEVICE_DPMS_SUSPEND: 
	bc_blk->attribute = PM_GRAPHICAL_OUTPUT; 
	(void)setPMDDwAttrib(bc_blk);
	break;
 
   case PM_RING_RESUME_ENABLE_NOTICE: /* only for some serial DD*/
	DBG(MDM RING RESUME ENABLE NOTICE is about to be sent.)
	bc_blk->attribute = PM_RING_RESUME_SUPPORT; 
	(void)setPMDDwAttrib(bc_blk);
	break;
 
   case PM_DEVICE_IDLE:	/* This is used for PM_SYSTEM_STANDBY. */ 
	(void)setSystemStandby();
	break;

   case PM_DEVICE_RESUME: 		/* broadcast all of aware DDs */
   case PM_PAGE_UNFREEZE_NOTICE:   	/* Post process in resuming */
	(void)circulateResumeNotice(bc_blk);
	break;

   } /* endswitch */

   return PM_SUCCESS;

} /* broadcast2DD */


	
/*
 * NAME:  PrepareSuspHib
 *
 * FUNCTION: Prepare for suspend/hibernation. Extra memory which will be used
 * 	     for broadcasting of suspend/hibernation is obtained beforehand. 
 *
 * RETURN VALUE DESCRIPTION:
 *	PM_SUCCESS - all of PM aware DDs accepted the current PM 
 *		     notice/request with success.
 * 		     In addition, the obtained memory pointer is set into
 *		     "pm_control_data.bd_xmalloc_ptr" 
 *
 *      PM_ERROR   - can't obtain memory. 
 */
int
PrepareSuspHib()
{
   	int	t;

   	if (pm_control_data.bd_xmalloc_ptr != NULL) {
   		(void)xmfree(pm_control_data.bd_xmalloc_ptr, pinned_heap);
   	} /* endif */

	t = get_NumberOfAwareDD();
	if (!(pm_control_data.bd_xmalloc_ptr 
	   	= (int *)xmalloc(t*sizeof(int),2,pinned_heap))) {
		return PM_ERROR;
	} /* endif */
	return PM_SUCCESS;

} /* PrepareSuspHib */


/*
 * NAME:  TerminateSuspHib
 *
 * FUNCTION: Dealloc the extra memory used for broadcasting of 
 * 				suspend/hibernation 
 *
 * RETURN VALUE DESCRIPTION:
 * 	none
 */
void
TerminateSuspHib()
{
    if (pm_control_data.bd_xmalloc_ptr != NULL) {
   	(void)xmfree(pm_control_data.bd_xmalloc_ptr, pinned_heap);	
    	pm_control_data.bd_xmalloc_ptr = NULL;
    } /* endif */
    return;
}


/*
 * NAME:  GetFirstOrLastOne
 *
 * FUNCTION: Get the device logical name of the PM aware DD which is chained
 *	     at either the beginning or end of the register list. 
 *	     And also the position number in the DD list is obtained here.
 *	     In order to handle the plural device drivers whose logical name
 *	     are the same, the order number(position in DD list) is used
 *	     to search the desired DD handle data structure. 
 *	     (It's either the first or the last register.)
 *
 * DATA STRUCTURES:
 *	broadcast_block_t
 *
 * RETURN VALUE DESCRIPTION:
 *	PM_ERROR:   No PM ware DD is regitered.
 *	PM_SUCCESS: the last registerd PM ware DD's logical name is set 
 *	 	    into input arg.	
 */
int
GetFirstOrLastOne(broadcast_block_t *pbd,int WhichOne)
{
   struct pm_handle 	*p;
   int			i;

   if (!(pm_kernel_data.handle_head)) {
	return PM_ERROR;
   } /* endif */

   *(pbd->device_logical_name) = NULL;
   for(p = pm_kernel_data.handle_head, i=1; p!=NULL; p=p->next1,i++) {
	if (p->pm_version == PM_PHASE2_SUPPORT) {
    		memcpy(pbd->device_logical_name, p->device_logical_name,0x10);
		pbd->pm_handle_ptr = p;
		pbd->order_in_list = i;
		if (WhichOne == FirstOne) {
			break;
		} /* endif */
	} /* endif */
   } /* endfor */

   if (*(pbd->device_logical_name) == NULL) {
	return PM_ERROR;
   } /* endif */

   return PM_SUCCESS;

} /* GetFirstOrLastOne */



/*
 * NAME: GetNextOne 
 *
 * FUNCTION: Get the device logical name of the PM aware DD which follows 
 * 	     the PM aware DD specified by its logical name. 
 *	     The "follow" means the next DD to the specified DD in the 
 *	     fashion of either the forward order or the backward order.
 *	     And also the position number in the DD list is obtained here.
 *	     In order to handle the plural device drivers whose logical name
 *	     are the same, the order number(position in DD list) is used
 *	     to search the desired DD handle data structure. 
 *
 * NOTE:     Phase-I PM aware DD is ignored to handle. However, when phase-I
 *	     PM aware DD is installed and loaded, the control never comes here.
 *	     Because neither suspend nor hibernation is supported in that case.
 *
 * DATA STRUCTURES:
 *	     broadcast_block_t
 *
 * INPUT:   
 *	     broadcast_block_t *pbd:  
 *			pbd->device_logical_name - specified device name
 *
 *	     direction: BACKWARD - previous PM aware DD in DD list
 *		        FORWARD  - next PM aware DD in DD list 
 * OUTPUT:    
 *	     broadcast_block_t *pbd:
 *			pbd->device_logical_name - desired DD's logical name
 *			pbd->pm_handle_ptr 	 - address of the DD's handle 
 *						   structure data 
 * RETURN VALUE DESCRIPTION:
 *	     PM_SUCCESS: The DD with the specified logical name could be found.
 *			 And the information of next one is put into input arg.
 *			
 *	     PM_ERROR:   No PM aware DD with the specified logical name	
 */
int
GetNextOne(broadcast_block_t *pbd,int direction)
{
   struct pm_handle 	*p;
   broadcast_block_t	internal_bd_bk;
   int			i;


   internal_bd_bk.pm_handle_ptr = NULL;
   memset(internal_bd_bk.device_logical_name,NULL,0x10);

   for(p = pm_kernel_data.handle_head,i=1; p!=NULL; p=p->next1,i++) {

	if (p->pm_version == PM_PHASE2_SUPPORT) {

	   if (pbd->order_in_list == i) {

		if (direction == BACKWARD) {

			memcpy(pbd->device_logical_name,
				internal_bd_bk.device_logical_name,
				0x10);
	   		pbd->pm_handle_ptr = internal_bd_bk.pm_handle_ptr;
			pbd->order_in_list = --i;
			return PM_SUCCESS; 

		} else {

			for (p=p->next1; p!=NULL; p=p->next1) {

				DBG(GetNextOne with FORWARD option)
				++i;
				if(p->pm_version == PM_PHASE2_SUPPORT){
					memcpy(pbd->device_logical_name,
					       p->device_logical_name,0x10);
	   				pbd->pm_handle_ptr = p;
					pbd->order_in_list = i;
					return PM_SUCCESS; 

				}/* endif */

			} /* endfor */
   			pbd->pm_handle_ptr = NULL;
   			memset(pbd->device_logical_name,NULL,0x10);
			return PM_SUCCESS; 

		}/* endif */
	   }/* endif */

	   memcpy(internal_bd_bk.device_logical_name, 
			p->device_logical_name, 0x10);	
	   internal_bd_bk.pm_handle_ptr = p;

	}/* endif */

   } /* endfor */
	
   /*
    *  Normally, the control never comes here
    */
   pbd->pm_handle_ptr = NULL;
   memset(pbd->device_logical_name,NULL,0x10);
   return PM_ERROR;	/* no PM aware DD with the specified logical name */

} /* GetNextOne */


/*
 * NAME:  circulatePMinformation
 *
 * FUNCTION:  All of PM aware DDs are informed of the PM notice/request 
 *            by this routine whenever entering.
 *
 * NOTES:
 *	The order of the circulation is the reversed order of device 
 *  	configuration in the registered PM aware DDs.
 *
 * DATA STRUCTURES:
 *	broadcast_block_t 
 *	pm_handle
 *
 * INPUT:  
 *	pbd->control
 *
 * RETURN VALUE DESCRIPTION:
 *	PM_SUCCESS - all of PM aware DDs accepted the current PM 
 *		     notice/request with success.
 *
 *      PM_ERROR   - One of PM aware DDs rejected on the way of the circulation.
 *		     The information of the PM aware DD which rejected is put 
 * 		     into broadcast_block.
 */
int
circulatePMinformation(broadcast_block_t *pbd)
{
   struct pm_handle *p;


   /*
    * This pointer points to the xmemory which has beforehand been
    * obtained prior to coming here. This area is used to save the
    * current PM mode of each PM aware DD. This mode information 
    * will be used to get them to go back to the former mode if one 
    * of them refuses the current state transition.
    */
    pm_control_data.bd_xmalloc_work = pm_control_data.bd_xmalloc_ptr; 


   /* 
    * Get the PM aware DD which is configured at the end of DD register list.
    */
    if (GetFirstOrLastOne(pbd,FirstOne) == PM_ERROR) {

	DBG(No PM aware DD(Phase-II) is loaded.)
	return PM_SUCCESS;  

    } /* endif */
	


    /*
     *  Loop of broadcasting
     */
    do {
	p = pbd->pm_handle_ptr;
        if (p == NULL) {

	  /*
	   *------------------------------------
	   *      <<< normal exit >>> 
	   *------------------------------------
	   * broadcasting has completed with success. 
	   */
		DBG(Exit of circulatePMinformation() with success)
   		return PM_SUCCESS;

	} /* endif */

	/* Originally, in case that phase_1 DD is included, phase2
	 * function is not featured at all. This is a debug purpose
 	 * check, so that the debug can be proceeded though some 
 	 * phase_1 DD is included. 
	 */
	if (p->handler == NULL) {
		/* just fail safe */
		continue;
	} /* endif */	

#ifdef PM_DEBUG
	if (strcmp(p->device_logical_name,ignoreID) == 0) {
		printf ("ignoreID = %s\n", ignoreID);
		continue;
	} /* endif */

	if((p->pm_version != PM_PHASE1_SUPPORT_ONLY) 	||
	   (pbd->control == PM_DEVICE_FULL_ON)		||
	   (pbd->control == PM_DEVICE_ENABLE)		)
	{
		switch (pbd->control) {
		case PM_PAGE_FREEZE_NOTICE:
printf("PAGE_FREEZE_NOTICE is sent to %s\n", (p->device_logical_name));
			break;
		case PM_PAGE_UNFREEZE_NOTICE:
printf("PAGE_UNFREEZE_NOTICE is sent to %s\n",(p->device_logical_name));
			break;
		case PM_DEVICE_SUSPEND:
printf("PM_SYSTEM_SUSPEND is sent to %s\n", (p->device_logical_name));
			break;
		case PM_DEVICE_HIBERNATION:
printf("PM_SYSTEM_HIBERNATION is sent to %s\n",(p->device_logical_name));
			break;
 		case PM_DEVICE_FULL_ON:
			if(p->pm_version != PM_PHASE1_SUPPORT_ONLY) { 	
printf("PM_DEVICE_FULL_ON is sent to %s\n", (p->device_logical_name));
			} else {
printf("PM_DEVICE_FULL_ON is sent to %08X\n", (p->devno));
			}
			break;
		case PM_DEVICE_ENABLE:
			if(p->pm_version != PM_PHASE1_SUPPORT_ONLY) { 	
printf("PM_DEVICE_ENABLE is sent to %s\n", (p->device_logical_name));
			} else {
printf("PM_DEVICE_ENABLE is sent to %08X\n", (p->devno));
			}
			break;
		default:
DBG(Invalid cmd/notice has just sent to PM aware DDs !!)
			break;
		} /* endswitch */
	} else {
		DBG(Phase_1 DD is ignored)
	} /* endif */

printf("pmh address = %08X\n", p);
#endif

	switch (pbd->control) {
	case PM_PAGE_FREEZE_NOTICE:
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
	    	if((p->pm_version != PM_PHASE1_SUPPORT_ONLY)) {	
			if ((p->handler)(p->private,pbd->control)==PM_ERROR) { 
				memcpy(pbd->device_logical_name,
		       			p->device_logical_name,0x10);
				pbd->pm_handle_ptr = p;
#ifdef PM_DEBUG
	DBG($$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$)
	printf("DD which has rejected is %s\n",(p->device_logical_name));
	DBG($$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$)
#endif
		/*
		 *--------------------------------------
		 *   A PM aware DD has refused to enter
		 *--------------------------------------
		 */
				return PM_ERROR;

			} /* endif */
   			*((pm_control_data.bd_xmalloc_work)++) = p->mode; 
	    	} /* endif */
		break;

	case PM_DEVICE_ENABLE:
        	if (((pm_control_data.phase_1_only == TRUE) &&
                    (pm_core_data.system_state == PM_SYSTEM_STANDBY))
                                ||
                   ((pm_control_data.phase_1_only == FALSE) &&
                    (pm_control_data.system_state == PM_SYSTEM_STANDBY)))
		{
		 	if(!((p->attribute & PM_GRAPHICAL_OUTPUT) 
					&&
			     (still_screen_dim == TRUE))) {
				p->idle_counter = 0;
				(p->handler)(p->private,pbd->control); 
			} else {
			DBG(Transition caused by tty activity. Still screen dim)
			} /* endif */

		} else {
			p->idle_counter = 0;
			(p->handler)(p->private,pbd->control); 
		} /* endif */
		break;

	case PM_PAGE_UNFREEZE_NOTICE:
	    	if((p->pm_version != PM_PHASE1_SUPPORT_ONLY)) {	
			(p->handler)(p->private,pbd->control); 
		} /* endif */
		break;

 	case PM_DEVICE_FULL_ON:
		if ((p->mode == PM_DEVICE_IDLE)  ||
		    (p->mode == PM_DEVICE_DPMS_STANDBY) ||
		    (p->mode == PM_DEVICE_DPMS_SUSPEND) ){
#ifdef PM_DEBUG
printf("ENABLE was sent to %s prior to FULL_ON\n", (p->device_logical_name));
#endif
			(p->handler)(p->private,PM_DEVICE_ENABLE); 
		} /* endif */

		(p->handler)(p->private,PM_DEVICE_FULL_ON); 
		break;

	} /* endswitch */

   } while (GetNextOne(pbd,FORWARD) == PM_SUCCESS);   /* do-while loop end */

   /*
    * Normally, the control never comes here. If comes, the program
    * has a bug. Anyway, it shouldn't be handled as an error to avoid
    * the following recovery process to PM aware DD.
    */
   DBG(Abnormal End of circulateResumeNotice)
   return PM_SUCCESS;

} /* circulatePMinformation */



/*
 * NAME:  circulateResumeNotice
 *
 * FUNCTION:  Resume notice broadcasting on the reversed order of entering
 *	      It is the same order as device configuration of system boot.
 *
 * DATA STRUCTURES:
 *	broadcast_block_t
 *
 * RETURN VALUE DESCRIPTION:
 *	none	
 */
void
circulateResumeNotice(broadcast_block_t *pbd)
{
   struct pm_handle *p;


    DBG(Entry of circulateResumeNotice())
   /* 
    * Get the PM aware DD which is configured at the end beginning of DD 
    * register list.
    */
    if (GetFirstOrLastOne(pbd,LastOne) == PM_ERROR) {

	DBG(No PM aware DD(Phase-II) is loaded.)
	return; 

    } /* endif */

 
    /*
     *  Loop of broadcasting
     */
    do {
        p = pbd->pm_handle_ptr;
        if (p == NULL) {

	  /*
	   *------------------------------------
	   *      <<< normal exit >>> 
	   *------------------------------------
	   * broadcasting has completed with success. 
	   */
		DBG(Exit of circulateResumeNotice() with success)
   		return;

	} /* endif */

	/* Originally, in case that phase_1 DD is included, phase2
	 * function is not featured at all. This is a debug purpose
 	 * check, so that the debug can be proceeded though some 
 	 * phase_1 DD is included. 
	 */
	if ((p->handler == NULL) ||
	    (p->pm_version == PM_PHASE1_SUPPORT_ONLY)) {
		/* just fail safe */
		continue;
	} /* endif */	

#ifdef PM_DEBUG
	if (strcmp(p->device_logical_name,ignoreID) == 0) {
		printf ("ignoreID = %s\n", ignoreID);
		continue;
	} /* endif */
#endif

	switch (pbd->control) {
	case PM_PAGE_UNFREEZE_NOTICE:
#ifdef PM_DEBUG
printf("PM_PAGE_UNFREEZE_NOTICE is sent to %s\n", (p->device_logical_name));
#endif
        	(p->handler)(p->private,pbd->control);
		break;	

	case PM_DEVICE_RESUME: 	
        	switch (p-> activity) {
        	case  PM_ACTIVITY_NOT_SET:      /* activity flag = "-1" */
                	pbd->control = PM_DEVICE_ENABLE;
#ifdef PM_DEBUG
printf("PM_DEVICE_ENABLE is sent to \n");
printf("   %s at resume due to activity flag = -1\n",(p->device_logical_name));
#endif
                	break;

        	case  PM_NO_ACTIVITY:           /* activity flag = 0 or 1 */
                	pbd->control = PM_DEVICE_IDLE;
#ifdef PM_DEBUG
printf("PM_DEVICE_IDLE is sent to \n");
printf("   %s at resume due to activity flag = 0\n",(p-> device_logical_name));
#endif
                	break;

        	case  PM_ACTIVITY_OCCURRED:
                	pbd->control = PM_DEVICE_IDLE;
#ifdef PM_DEBUG
printf("PM_DEVICE_IDLE is sent to \n");
printf("  %s at resume due to activity flag = 1\n",(p-> device_logical_name));
#endif
                	break;
        	} /* endswitch */

		p->idle_counter = 0;
        	(p->handler)(p->private,pbd->control);
		pbd->control = PM_DEVICE_RESUME;	/* for next loop */
		break;

	default:
	/* do nothing */
		return;	

        } /* endswitch */
        
   } while (GetNextOne(pbd,BACKWARD) == PM_SUCCESS);   /* do-while loop end */

   DBG(Abnormal End of circulateResumeNotice)
   return; 

} /* circulateResumeNotice */




/*
 * NAME:  retrieve_PM_aware_DD
 *
 * FUNCTION:  When a PM aware DD rejects to accept a request from PM core,
 *	      all of PM aware DDs which have already accepted with success
 *	      need to be notified of the rejection. This routine informs 
 *	      those DDs of the fact that the former request was terminated
 *	      and they have to get back to the former state. 
 *	      The PM aware DD mentioned here is only phase 2 PM aware DD.
 *	      The phase 1's one is sent anything there since originally 
 *	      neither suspend nor hibernation event is sent to it. 
 *
 * DATA STRUCTURES:
 *	broadcast_block_t
 *	pm_handle
 *
 * RETURN VALUE DESCRIPTION:
 *	none	
 */
void
retrieve_PM_aware_DD(broadcast_block_t *pbd)
{
   struct pm_handle *p;
   broadcast_block_t name_saving_buf;

    DBG(Entry of retrieve_PM_aware_DD())

    /* save the name of the DD which rejected pm event */
    memcpy(name_saving_buf.device_logical_name,pbd->device_logical_name,0x10);

   /* 
    * Get the PM aware DD next to the focal DD which had either 
    * rejection(SUSPEND/HIBERNATION) or error(PM_PAGE_FREEZE_NOTICE).
    */
    if (GetNextOne(pbd,BACKWARD) == PM_ERROR) {
	return; 	/* fail safe */
   } /* endif */ 


   /* 
    * Loop of retrieving the former mode or send PM_PAGE_UNFREEZE
    */
    do {
        p = pbd->pm_handle_ptr;
        if (p == NULL) {

	  /*
	   *------------------------------------
	   *      <<< normal exit >>> 
	   *------------------------------------
	   * broadcasting has completed with success. 
	   */
		DBG(Exit of retrieve_PM_aware_DD())

		/* retore the name of the DD which rejected pm event */
    		memcpy(pbd->device_logical_name,
		       name_saving_buf.device_logical_name, 0x10);
   		return;

	} /* endif */

	/* Originally, in case that phase_1 DD is included, phase2
	 * function is not featured at all. This is a debug purpose
 	 * check, so that the debug can be proceeded though some 
 	 * phase_1 DD is included. 
	 */
	if ((p->handler == NULL) ||
	    (p->pm_version == PM_PHASE1_SUPPORT_ONLY)) {
		/* just fail safe */
		continue;
	} /* endif */	


#ifdef PM_DEBUG
	if (strcmp(p->device_logical_name,ignoreID) == 0) {
		printf ("ignoreID = %s\n", ignoreID);
		continue;
	} /* endif */
#endif 
	/* save original "control" in temporal */
	name_saving_buf.control = pbd->control;

	switch (pbd->control) {
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
  
 		pbd->control = (int)*(--(pm_control_data.bd_xmalloc_work)); 
		if(p->activity == -1) {
			pbd->control = PM_DEVICE_ENABLE;   /* overridden */
		} 
#ifdef PM_DEBUG 
		if (pbd->control == PM_DEVICE_ENABLE) {
		printf("PM_DEVICE_ENABLE is sent\n"); 
		printf("       to %s for recovery\n",(p->device_logical_name)); 
		} else {
		printf("PM_DEVICE_IDLE is sent\n");
		printf("       to %s for recovery\n",(p->device_logical_name));
		} /* endif */
#endif 
		(p->handler)(p->private, pbd->control);
		/* After sending enable/idle request, unfreeze is also sent */

	case PM_PAGE_FREEZE_NOTICE:

#ifdef PM_DEBUG 
		printf("PM_PAGE_UNFREEZE_NOTICE is sent\n"); 
		printf("     to %s for recovery\n", (p->device_logical_name)); 
#endif
		(p->handler)(p->private, PM_PAGE_UNFREEZE_NOTICE);
		break;	
	
	} /* endswitch */
 
	/* retrieve original "control" from temporal saving buffer */
	pbd->control = name_saving_buf.control;

    } while (GetNextOne(pbd,BACKWARD) == PM_SUCCESS);   /* do-while loop end */

    /* 
     * Normally, never come here. If comes, it's a program bug.
     */
    DBG(Abnormal End of circulateResumeNotice)
    return;

} /* retrieve_PM_aware_DD */




/*
 * NAME:  setPMDDwAttrib 
 *
 * FUNCTION: Send a specified notice to PM aware DD specified with attribute.
 *
 * NOTES:
 * 	The PM aware DD is specified with its attribute here.	
 *
 * DATA STRUCTURES:
 *	broadcast_block_t
 *      attribute
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 */
void
setPMDDwAttrib(broadcast_block_t *pbd)
{
   struct pm_handle 	*p;

   for(p = pm_kernel_data.handle_head; p != NULL; p = p->next1) {
	if (((p->attribute & pbd->attribute)==pbd->attribute)  	&& 
	    (p->handler != NULL)  				&&
	    (p->pm_version == PM_PHASE2_SUPPORT) 		){ 

		(p->handler)(p->private,pbd->control);
#ifdef PM_DEBUG 
		printf("The PM aware DD with a specified attribute\n"); 
		printf("		are sent PM message/notice\n");
		printf("The specified attribute is %X\n", pbd->attribute);

		switch (pbd->control) {
		case PM_DEVICE_DPMS_STANDBY:
			printf("PM_DEVICE_DPMS_STANDBY is sent\n"); 
			printf("	to %s\n",(p->device_logical_name));
			break;
		case PM_DEVICE_DPMS_SUSPEND:
			printf("PM_DEVICE_DPMS_SUSPEND is sent\n"); 
			printf("	to %s\n", (p->device_logical_name));
			break;
		case PM_RING_RESUME_ENABLE_NOTICE:
			printf("PM_RING_RESUME_ENABLE_NOTICE is sent\n");
			printf("	 to %s\n", (p->device_logical_name));
			break;
		default:
			printf("Unexpected notice is sent\n");
			printf("	 to %s\n", (p->device_logical_name));
			break;

		} /* endswitch */
#endif

	} /* endif */
   } /* endfor */
   return;
}


/*
 * NAME:  setSystemStandby
 *
 * FUNCTION:  Send PM DEVICE IDLE to all the PM aware DDs except for 
 *            the PM aware DD whose both idle timer and standby timer 
 * 	      are zero or activity flag is -1. 
 *
 * NOTES:
 *	      Before calling this function, graphical output device is
 *	      already sent PM_DEVICE_DPMS_STANDBY since it's about to 
 *	      enter system standby state.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 */
void
setSystemStandby()
{
   struct pm_handle 	*p;

   for(p = pm_kernel_data.handle_head; p != NULL; p = p->next1) {
	if ((p->activity != PM_ACTIVITY_NOT_SET) &&
	    ((p->device_idle_time || p->device_standby_time) != 0) &&
	    (!(p->attribute & PM_GRAPHICAL_OUTPUT))) {

		(p->handler)(p->private,PM_DEVICE_IDLE);
#ifdef PM_DEBUG
		printf("PM_DEVICE_IDLE is sent\n"); 
		printf("	to %s\n",(p->device_logical_name));
#endif

	} /* endif */
   } /* endfor */
   return;

} /* setSystemStandby */



/*
 * NAME: ScanPMawareDD 
 *
 * FUNCTION: Check if all of PM aware DDs support PM phase_2 function 
 *
 * DATA STRUCTURES:
 *	pm_handle
 *
 * RETURN VALUE DESCRIPTION:
 *	PM_SUCCESS - All of PM aware DDs support PM phase-2 function.
 *	PM_ERROR   - At least, one PM aware DD only supports PM phase_1.	
 */
int
ScanPMawareDD(dev_t * devno_of_Phase1PMDD)
{
   struct pm_handle 	*p;

   for(p = pm_kernel_data.handle_head; p != NULL; p = p->next1) {
	if (p->pm_version != PM_PHASE2_SUPPORT) { 
		(*devno_of_Phase1PMDD) = p->devno;
		return PM_ERROR;
	} /* endif */
   } /* endfor */
   return PM_SUCCESS;
}

/*
 * NAME: get_NumberOfAwareDD 
 *
 * FUNCTION:  Get the number of PM aware DD
 *
 * NOTES:
 *      The number of PM aware DD may be changed dynamically. PM core does
 *	not know when it's changed. Because registering PM aware DD is not 
 *      done in PM core, but in base kernel. Hence, this routine needs to be 
 *      called whenever the number is required.
 *
 * RETURN VALUE DESCRIPTION:
 *      Integer of the number of PM aware DD currently registered. 
 *      PM_ERROR - Caller doesn't have priveledge or invalid argument.
 */
int
get_NumberOfAwareDD() 
{
   int  device_count;
   struct pm_handle *p;

   DBG(*****************************************************************)
   DBG(	 	Query all of PM aware DD prior to starting broadcast    )
   DBG(*****************************************************************)
   for (device_count = 0,
	p = pm_kernel_data.handle_head; p != NULL; p = p->next1) {
	device_count++;

   #ifdef PM_DEBUG
	if (p->pm_version == PM_PHASE2_SUPPORT) {
		printf("PM aware DD(phase-II): %s\n", (p->device_logical_name));
	} else {
		printf("PM aware DD(phase-I): devno = %08X\n", p->devno);
	}
	printf("pmh address = %08X\n", p);
   #endif

   } /* endfor */

   DBG(*****************************************************************)
   #ifdef PM_DEBUG
   printf("Number of PM aware DDs is %X\n", device_count);
   #endif
   DBG(*****************************************************************)
   return (device_count);

} /* get_NumberOfAwareDD */


