static char sccsid[] = "@(#)97  1.7  src/rspc/kernext/pm/pmmd/6030/pmmiif.c, pwrsysx, rspc41J, 9520A_all 5/12/95 12:48:32";
/*
 *   COMPONENT_NAME: PWRSYSX
 *   FUNCTIONS: pmmd_job, establish_pmmi_interface 
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
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/rtc.h>
#include <sys/time.h>

#include <sys/pm.h>
#include "../../pmmi/pmmi.h"
#include "slave.h"	
#include "pm_6030.h"
#include "planar.h"

#include "pdplanar.h"
#include "pdcommon.h"
#include "shpcrsus.h"
#include "pdsystem.h"

/*-------------------------*/
/*  Global variables	   */ 
/*-------------------------*/
struct tms RTC_alarm_resume_time;


/*-------------------------*/
/*  external variables     */
/*-------------------------*/
extern pmmd_IF_t pmmd_IF;
extern pm_control_data_t pm_control_data;
extern pmmd_control_data_t pmmd_control_data;

/*-------------------------*/
/*    external functions   */ 
/*-------------------------*/
extern	int	pm_clear_restart_information(void);
extern	int	inform_slave_of_suspend();
extern	int	inform_slave_of_hibernation();
extern	int	inform_slave_of_resume();
extern	int	pm_planar_control_psumain(dev_t devno, int devid, int cmd);
extern	int	planardevice_control(int);
extern	void	set_RTC_resume_time(struct tms *);
extern	int 	GetRTCAlarmTime(struct tms *, resume_time_data_t *);
extern	void 	program_resume_event(rsm_en_event_t *arg);
extern	void 	program_RTC_resume(int);
extern  void 	adjust_rtc_chip();
extern 	void	Reinit_SysTime();
extern  void 	callback_addr_job(callback_entry_block_t *arg);
extern	void  * GetROSMsr(); 


/*-------------------------*/
/*    function proto type  */
/*-------------------------*/
int 	establish_pmmi_interface(int cmd);
int 	pmmd_job(int cmd, caddr_t arg);
void 	transition_core_job(int opt);

			

/*
 * NAME: pmmd_job 
 *
 * FUNCTION: pmmi interface entry 
 *
 * NOTES: This routine is a dispatcher for the jobs requested from PMMI to
 *	  PMMD which are related to the machine dependent job.
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS - Successfully processed
 *        PM_ERROR - Invalid cmd argument or falied to process
 *                   battery functions.
 */
int	
pmmd_job(int cmd, caddr_t arg)
{
	struct 	tms  rsm_tm, *Prsm_tm = &rsm_tm;
        struct  timestruc_t ct;

	switch (cmd) {
	case	set_pmmi_callback:
		(void)callback_addr_job((callback_entry_block_t *)arg);
		SetupLowbatteryEvent();
		break;

	case	LED_control:
		LED_job((LED_block_t *)arg);
		break;

	case	BEEP_control:
		BEEP_job((BEEP_block_t *)arg);
		break;
		
	case	save_basic_devices:
		/* Disable slave int */
		PM_interrupt_control(Disable_PM_int);	

		/* LCD panel control signals will be turned off after     
		 * the current setting are saved in "planardevice_control". 
		 * Those state will be restored at resume. 
 		 * However, before coming here, LCD has already been 
		 * turned off by graphics DD's calling planar_control.
		 */
		DBG(Calling "save_basic_device")
		(void)planardevice_control(Save_planar_device);
		DBG(Exit of "save_basic_devices")
		DumpRegisters();
		break;
			
	case	restore_basic_devices:
		/* retore basic device context */
		(void)planardevice_control(Restore_planar_device);
		DBG("Restore_planar_device" is called to retrieve\
			the former device context)

		/* Here are not to call planar_control_lcd/crt. They should
 		 * be responsible for graphics DD who knows which screen 
		 * should be turned on according to the condition of the 
		 * screen resolution and/or external display attached. 
		 */

       		pm_clear_restart_information(); 
#ifdef PM_DEBUG
		printf("MSR from ROS is %08X\n", GetROSMsr()); 
		printf("HID0 from ROS is %08X\n", GetROSHid0()); 
#endif
		DumpRegisters();
		break;
			
	case	enter_suspend: 
		DBG(Inform slave of suspend)
		(void)inform_slave_of_suspend();
		transition_core_job(PM_SYSTEM_SUSPEND);
		/* The result is checked latter in "get_resume_event". */
		break;

	case	enter_hibernation:
		DBG(Inform slave of hibernation)
		(void)inform_slave_of_hibernation();
		transition_core_job(PM_SYSTEM_HIBERNATION);
		/* The result is checked latter in "get_resume_event". */
		break;
			
	case	set_resume_time: 
        	pm_control_data.RTC_everyday = FALSE;
		if (GetRTCAlarmTime(Prsm_tm,(resume_time_data_t *)arg)!=0){ 
			(void)set_RTC_resume_time(Prsm_tm);
			memcpy(&RTC_alarm_resume_time, Prsm_tm, 
							sizeof(struct tms));
			program_RTC_resume(RESUME_ENABLE);
			break;
		} /* endif */
		program_RTC_resume(RESUME_DISABLE);
		break;
			
	case	get_resume_event:
		/* 
		 * This get_resume_event should be called after the completion 
		 * of restore_basic_device() which includes the restore
		 * routine of slave CPU I/F registers and confirms the
	 	 * slave CPU becomes ready at resume.
		 */
		pm_control_data.TemporalDisableAcDcBEEP = TRUE;
		if (((rsm_trig_blk_t *)arg)->state == PM_SYSTEM_SUSPEND) {

        	    if (!(pmmd_control_data.resume_trigger 
	     			= inform_slave_of_resume())) {

        		pmmd_control_data.resume_trigger 
				= check_HW_resume_trigger();
		    } /* endif */

		    if ((pmmd_control_data.resume_trigger == PM_EVENT_RTC) 
					&&
			(pm_control_data.RTC_everyday == FALSE)
					&&
			(pmmd_control_data.Suspend2HibByRTC == FALSE)) {

                            DBG(Resume event is RTC alarm)
                           /*
                            * When coming here, system time hasn't
                            * been retrieved yet.
                            */
                            if( ((rsm_tm.yrs = read_rtc(RTC_YEAR)) !=
                                                RTC_alarm_resume_time.yrs)  ||
                                ((rsm_tm.mths = read_rtc(RTC_MONTH)) !=
                                                RTC_alarm_resume_time.mths) ||
                                ((rsm_tm.dom = read_rtc(RTC_DOM)) !=
                                                RTC_alarm_resume_time.dom)  ){

                                turn_LCD_panel_off();
                                program_RTC_resume(RESUME_ENABLE);
                                return TurnAround2Suspend;
                            } /* endif */

		    } /* endif */

		     ((rsm_trig_blk_t *)arg)->event
				= pmmd_control_data.resume_trigger;
	
		     /* 
		      * Slave CPU interrupt is re-enabled.
		      */
		     PM_interrupt_control(Enable_PM_int);/* Enable slave int */


		} else { /* In case of resume from hibernation */

		       ((rsm_trig_blk_t *)arg)->event 
				= pmmd_control_data.resume_trigger;

		       if (pmmd_control_data.resume_trigger==RESUMEwSUCCESS) {

		       		if (init_slave_interface() != 0) {
					return AbnormalResume; 
		       		} /* endif */

		      		/* In case of wakeup from hibernation, 
				 * slave_cpu is completely in the state of 
				 * power-on-default. So it must be retrieved 
				 * to the enabled state.
		       		 */
		       		SetupLowbatteryEvent();

		       } else {
				return HibernationEnterFail;	

		       } /* endif */
		} /* endif */

		return NormalResume;
			
	case	set_resume_event:
		program_resume_event((rsm_en_event_t *)arg);
         	/*
          	 * PMD always tries to create hibernation
          	 * volume if not present every time entering
          	 * suspend or hibernation state. So the following
          	 * adjustment is required due to the dependency
          	 * of the availability of hibernation volume.
          	 * The availability is related to how early
          	 * low battery notice should be generated.
          	 */
		SetupLowbatteryEvent();
		break;

	case	battery_job:
		battery_handler((battery_control_t *)arg);  
		break;
			
	case	TurnPowerOff:
		(void)pm_planar_control_psumain(NULL,NULL,PM_PLANAR_OFF); 
		break;		/* never come here again */

        case    adjust_system_time:
                /*
                 * Time maintained by CPU clock is more accurate than rtc
                 * chip's one. So the time of CPU is reflected to rtc chip.
                 */
                DBG(Adjust system time )
                adjust_rtc_chip();
                break;

	case	reconstruct_systime:	
		Reinit_SysTime();	/* Reconstruct system time */
		MakeDECavailable();
		DBGNMB(0x0ee)
		break;

	case	EmergencyScreenOn:
		if (((ScreenOn_req_blk_t *)arg)->LCD_on == TRUE) {
                	modify_pmc( INDEXPMC_PMCO0,
                                   BITPMCPMCO0_LCDOFF, SET_BITS );
                	modify_pmc( INDEXPMC_PMCO0,
                                   BITPMCPMCO0_LCDENABLE, SET_BITS );
			pm_planar_control_lcd(NULL,PMDEV_LCD,PM_PLANAR_ON);
		}
		if (((ScreenOn_req_blk_t *)arg)->CRT_on == TRUE) {
			pm_planar_control_crt(NULL,PMDEV_CRT,PM_PLANAR_ON);
		}
		break;

	case 	ForceTerminate:
		PM_interrupt_control(Disable_PM_int);
		break;
	
	case    pmmd_dbgout:
		DBGNMB(arg)
		break;

	default:
		return (PM_ERROR);	/* argument syntax error */

	} /* endswitch */

	return (PM_SUCCESS);	
}

