static char sccsid[] = "@(#)96  1.8  src/rspc/kernext/pm/pmmd/6030/pmmdmisc.c, pwrsysx, rspc41J, 9520A_all 5/10/95 13:10:27";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: ChangeSystemState, PresetTriggers,  
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
#include <sys/errno.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>
#include <sys/rtc.h>
#include <sys/time.h>

#include <sys/pm.h>
#include "../../pmmi/pmmi.h"
#include "planar.h"
#include "pm_6030.h"
#include "slave.h"
#include "pdplanar.h"
#include "pdrtc.h"


#define  Hibernation 	0x0200
#define  SuspendSupported 0x0400
#define	DATE_ALARM_BYTE	0x49;


/*-------------------------*/
/*    external variable	   */
/*-------------------------*/
extern pm_machine_profile_t 	machine_profile;
extern ISA_DATA    pm_xioc_data;     /* I/O mapping data block  */
extern ISA_DATA    pm_RTC_data;      /* RTC access data block   */
extern pm_control_data_t pm_control_data;
extern pmmd_control_data_t pmmd_control_data;

/*-------------------------*/
/*    function proto type  */
/*-------------------------*/
void 	program_resume_event(rsm_en_event_t *arg);

void	program_RTC_resume(int cmd);

int 	GetRTCAlarmTime(struct tms *tm, resume_time_data_t *arg);

int 	check_HW_resume_trigger();

void 	wait_forever();

void 	turn_LCD_panel_on();

void 	turn_LCD_panel_off();

void 	SetDateAlarm(uchar alarmdate);

/*
 * NAME: program_resume_event
 *
 * FUNCTION: RING RESUME, slave CPU resume for LID open, main power switch
 *	     and keyboard(external)/mouse(internal click key) are enabled
 *	     or disabled in this function. 
 * 
 * Note:     This routine depends upon the uniqueness of power management
 *	     chip on Woodfield/Polo. Power management chip named "Carrera" 
 *	     is used for ringing resume trigger and RTC alarm resume trigger.
 *	     And also slave CPU can generate resume trigger using another
 *	     input pin. Note that only resume from suspend is mentioned here.
 *	     Woodfield(prime) doesn't support automatic resume from hibernation
 *	     such as ring resume from hibernation or RTC alarm resume from
 *	     hibernation.
 *
 * RETURN VALUE DESCRIPTION:
 *		none
 */
void
program_resume_event(rsm_en_event_t *arg)
{
	int	dat;
	
	dat = read_pmc(INDEXPMC_RESUME_MASK);
	if (arg->MDM_ring_resume_from_suspend == RESUME_ENABLE) {
		dat &= ~BITPMCRESUMEMASK_RING_MSK;
	} else {
		dat |= BITPMCRESUMEMASK_RING_MSK;
	}
	dat &= ~BITPMCRESUMEMASK_H8_RSM_MSK;	/* default enable for slave  */ 
	write_pmc(INDEXPMC_RESUME_MASK,dat);

	if(arg->critical_low_battery_hibernation_from_suspend == NOT_SUPPORTED){
		(void)Disable_suspend2hibernation();
	}

	DBG(resume event setup is complete)

	return;
}



/*
 * NAME: GetRTCAlarmTime
 *
 * FUNCTION: Choose the earlier resume time between specified resume
 *	     and specified duration from suspend to hibernation.
 *
 * STRUCTURE: struct tms (<sys/rtc.h>) 
 *	      resume_time_data_t ("pmmi.h")	  
 *
 * RETURN VALUE DESCRIPTION:
 *	      0: no RTC alarm time specified	
 *	      1: the desired time is set into the area refered by input ptr. 
 */
int
GetRTCAlarmTime(struct tms *Ptm, resume_time_data_t *arg)
{
	struct	timestruc_t ct;
	struct 	tms 	temptime;
     	uchar   cvt_hrs, cvt_mins, cvt_no_secs;
	int		data;


	pm_control_data.RTC_everyday = FALSE;
	curtime(&ct);

        if ((arg->resume_at_specified_time != 0) && 
            (arg->resume_at_specified_time <= 3600*24)) {
                /* every day request ? */

                temptime.secs = ct.tv_sec;
                temptime.ms = ct.tv_nsec / (NS_PER_SEC/100);
                                /* nano secs to 100th secs */
                secs_to_date(&temptime);

                /* BCD to decimal */
                cvt_hrs = (temptime.hrs/16 ) * 10 + (temptime.hrs % 16);
                cvt_mins = (temptime.mins/16 ) * 10 + (temptime.mins % 16);
                cvt_no_secs=(temptime.no_secs/16 )*10 + (temptime.no_secs % 16);
		data = 3600*cvt_hrs + 60*cvt_mins + cvt_no_secs;

                if (arg->resume_at_specified_time > data) {
			data = ct.tv_sec - data;
		} else {
			data = ct.tv_sec - data + (3600*24);
		} /* endif */
		arg->resume_at_specified_time += data;	
		pm_control_data.RTC_everyday = TRUE;
		
        } /* endif */


	pmmd_control_data.Suspend2HibByRTC = FALSE;

	if (arg->duration_for_susp2hib) {

		if (arg->resume_at_specified_time <= ct.tv_sec) {
			pm_control_data.RTC_everyday = FALSE;
			ct.tv_sec += (long)arg->duration_for_susp2hib;		
                	arg->selected_resume_time = SuspendToHibernationByRTC;
			pmmd_control_data.Suspend2HibByRTC = TRUE;
			DBG(suspend to hibernation is adopted(1))

		} else if (arg->resume_at_specified_time 
				<= (ct.tv_sec + arg->duration_for_susp2hib)) {
			ct.tv_sec = (long)arg->resume_at_specified_time;
                	arg->selected_resume_time = NULL;
			DBG(resume_at_specified_time is adopted(1))

		} else {
			pm_control_data.RTC_everyday = FALSE;
			ct.tv_sec += (long)arg->duration_for_susp2hib;		
                	arg->selected_resume_time = SuspendToHibernationByRTC;
			pmmd_control_data.Suspend2HibByRTC = TRUE;
			DBG(suspend to hibernation is adopted(2))

		} /* endif */


	} else {

                arg->selected_resume_time = NULL;

		if (arg->resume_at_specified_time <= ct.tv_sec) {
			pm_control_data.RTC_everyday = FALSE;
			DBG(no RTC alarm is adopted)
			return 0;
	
		} else { 
			ct.tv_sec = (long)arg->resume_at_specified_time;
			DBG(resume_at_specified_time is adopted(2))

		} /* endif */
		
	} /* endif */


	Ptm->secs = ct.tv_sec;
	secs_to_date(Ptm);	

	return 1;
}



/*
 * NAME: program_RTC_resume
 *
 * FUNCTION: Enable/disalbe RTC resume trigger 
 *
 * NOTES: This routine depends upon the type of RTC(Real Time Clock).
 *	  PC/AT compatible RTC is assumed here.  
 *
 * RETURN VALUE DESCRIPTION:
 *		none
 */
void	
program_RTC_resume(int cmd)
{
	int	dat;
   	MSGRTC      mRtc;

	/* 
	 * When the two most significant bits are 1,
	 * the byte is handled as a "don't care" byte.
	 */
	SetDateAlarm(0xc0);	


   	pm_map_iospace();
	if (cmd == RESUME_ENABLE) {
      		BuildMsgWithParm1(mRtc, MSGRTC_SET_WAKEUPALARMSTATUS,
                              PRMRTC_ALARMENABLE);
      		SendMsg(mRtc, oRtcCtl);
      		BuildMsgWithParm1(mRtc, MSGRTC_SET_WAKEUPALARM,
                              PRMRTC_ALARMOFF);
      		SendMsg(mRtc, oRtcCtl);
		/* 
		 * This enabling is gotten back to disabling in 
		 * restore_basic_device() called at resume. 
	         */
	} else {
      		BuildMsgWithParm1(mRtc, MSGRTC_SET_WAKEUPALARMSTATUS,
                              PRMRTC_ALARMDISABLE);
      		SendMsg(mRtc, oRtcCtl);
	}
   	pm_unmap_iospace();

	dat = read_pmc(INDEXPMC_RESUME_MASK);
	if (cmd == RESUME_ENABLE) {
		dat &= ~BITPMCRESUMEMASK_ALARM_MSK;
	} else { 
		dat |= BITPMCRESUMEMASK_ALARM_MSK;
	}
	write_pmc(INDEXPMC_RESUME_MASK,dat);

	return;
}

 

/*
 * NAME: SetDateAlarm 
 *
 * FUNCTION: Write a specified value into date alarm byte  
 *
 * NOTES: date alarm byte is not compatible byte with traditional PC 
 *
 * RETURN VALUE DESCRIPTION:
 *	  none 
 */
void
SetDateAlarm(uchar alarmdate)
{
        volatile uchar  *io;
        int             opri;
        int             data;
        int             former_data;


        /*
         * serialize access to RTC register
         */
        opri = disable_lock(INTMAX, &(pm_RTC_data.lock));


        /*
         * I/O access start
         */
        io = (volatile uchar *)iomem_att(&(pm_RTC_data.map))
                                                + pm_RTC_data.base;

        /*
         *  select CMOS bank 1
         */
        *((volatile uchar *)io) = RTC_Status_regA;
        eieio();
        former_data = data = *((volatile uchar *)(io + 1));
        eieio();
        data &=~RTC_RegA_modifiedBits;
        data |=(RTC_Osc_enable + RTC_BankSelect);
        *((volatile uchar *)(io + 1)) = data;
        eieio();


        /*
         *  index for date alarm byte 
         */
        *((volatile uchar *)io) = DATE_ALARM_BYTE;
        eieio();
        *((volatile uchar *)(io + 1)) = alarmdate;
        eieio();


        /*
         *  Restore the former value of RTC status regA
         */
        *((volatile uchar *)io) = RTC_Status_regA;
        eieio();
        *((volatile uchar *)(io + 1)) = former_data;
        eieio();


        /* I/O access end */
        iomem_det((void *)io);

        unlock_enable(opri, &(pm_xioc_data.lock));

        return;
}




/*
 * NAME: check_HW_resume_trigger 
 *
 * FUNCTION: Check if HW resuem trigger was a resume event. 
 *
 * NOTES: Hardware resume triggers such as RING/RTC/GP pin 
 *
 * RETURN VALUE DESCRIPTION:
 *	  resume event(RING, ALARM, or none)	
 */
int
check_HW_resume_trigger()
{
	int	read_dat;

	read_dat = read_pmc_nolock(INDEXPMC_RESUME_STS);
	if (read_dat & BITPMCRESUMESTS_RING_STS) {
		read_dat=PM_EVENT_RING;
	} else if (read_dat & BITPMCRESUMESTS_ALARM_STS) {
		read_dat=PM_EVENT_RTC;
	} else {
		read_dat=PM_EVENT_NONE;
	}
	return(read_dat);
}



/*
 * NAME: wait_forever 
 *
 * FUNCTION: Give a trigger of turning Vcc-B off here.
 *
 * NOTES:  Never return due to being about to be powered off.
 *
 * RETURN VALUE DESCRIPTION:
 *        none
 */
void
wait_forever()
{
	int	a=0;

	while (a == 0) {
		SetMsrEe();		/* in pdppcfnc.s (need for 603 bug) */
		SleepNeverReturn();	/* Never return */
	}
}



/*
 * NAME: turn_LCD_panel_on
 *
 * FUNCTION: Turn LED panel on the specific order and  
 *	     timing.
 * 	    
 * NOTES:  If the specific order and timing are not performed
 * 	   with, LCD panel may be broken. LED panael is a 
 *	   sensitive guy.....
 *
 * RETURN VALUE DESCRIPTION:
 *        none
 */
void
turn_LCD_panel_on()
{
	int	data;

	data = read_pmc_nolock(INDEXPMC_PMCO0);
	data |= 0x02;
	write_pmc_nolock(INDEXPMC_PMCO0, data);
	data |= 0x04;
	write_pmc_nolock(INDEXPMC_PMCO0, data);
/*
    (((	Backlight-on should be dependent on graphics DD. )))
    ((( Graphics DD will call pm_planar_control_lcd.     )))
	data |= 0x01;
	write_pmc_nolock(INDEXPMC_PMCO0, data);
*/
	
	return;
}


/*
 * NAME: turn_LCD_panel_off
 *
 * FUNCTION: Turn LED panel off the specific order and  
 *	     timing.
 * 	    
 * NOTES:  If the specific order and timing are not performed
 * 	   with, LCD panel may be broken. LED panael is a 
 *	   sensitive guy.....
 *
 * RETURN VALUE DESCRIPTION:
 *        none
 */
void
turn_LCD_panel_off()
{
	int	data;

	data = read_pmc_nolock(INDEXPMC_PMCO0);
	data &= ~0x01;
	write_pmc_nolock(INDEXPMC_PMCO0, data);
	data &= ~0x04;
	write_pmc_nolock(INDEXPMC_PMCO0, data);
	data &= ~0x02;
	write_pmc_nolock(INDEXPMC_PMCO0, data);

	/* Wait here for 100 msec to make an interval 
	   between now and next powering on. */
	io_delay(1000*100);	/* unit = usec */

	return;
}


/*
 * NAME: BEEP_job
 *
 * FUNCTION: Procedure to handle BEEP generated by slave CPU
 *
 * NOTES: Slave CPU is ordered with a specific command for BEEP.
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS - Successfully processed
 *        PM_ERROR -   Procedure is terminated due to slave CPU
 *	               communication error. 
 */
int
BEEP_job(BEEP_block_t *arg)
{
	int		d1;
	int		d2;	
	BEEP_block_t	*b;
	b=arg;

	switch (b->BEEP_type) {
	case suspend_beep:
		d1=Suspend_Beep;
		break;
	case partial_suspend_beep:
		d1=Partial_Suspend_Beep;
		break;
	case reject_beep:
	     	d1=F315Hz;
		break;	
	case AC_DC_beep:
		d1=DC_IN_OUT_Beep;
		break;
	case hibernation_beep:
		d1=Suspend_Beep;
		break;
	default:
		return (PM_ERROR);
	}

	switch (b->control) {
	case BEEP_off:
		d2=StopRepeatedBeep;
		break;
  	case BEEP_on:
		d2=2;	/* Interval = 125msec x 2 */
		break;
	case BEEP_oneshot:
		d2=ONE_SHOT;
		break;
	dafault:
		return (PM_ERROR);
	}
	AlertUserWithBeep(d1,d2);	
	return (PM_SUCCESS);
}

/*
 * NAME: LED_job 
 *
 * FUNCTION: LED(Suspend/Power) are controlled via slave CPU commands.
 *
 * NOTES:
 *
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS   
 *        PM_ERROR - Procedure is terminated due to the communication 
 *		     error in communicating with slave cpu. 
 */
int
LED_job(LED_block_t *arg)
{
	int		d1;
	int		d2;	
	int		d3=NULL;
	LED_block_t	*l;

	l=arg;


	switch (l->LED_type) {
	case suspend_LED:
		d1=Suspend_LED_device;
		break;
	case power_on_LED:
		d1=Power_LED_Device;
		break;
	default:
		return (PM_ERROR);
	}

	switch (l->control) {
	case LED_off:
		d2=TurnOffLED;
		break;
	case LED_on:
		d2=TurnOnLED;
		break;
	case LED_blink:
		d2=BlinkLED;
		d3=BlinkFrequency_4Hz;
		break;
	default:
		return (PM_ERROR);
	}

	ChangeLEDstate(d1,d2,d3);
	return (PM_SUCCESS);

} /* LED_job */


