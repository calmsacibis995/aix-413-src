static char sccsid[] = "@(#)32	1.5  src/rspc/kernext/pm/pmmd/IBM8301/pmmdentry.c, pwrsysx, rspc41J, 9517A_all 4/25/95 17:05:16";
/*
 *   COMPONENT_NAME: PWRSYSX
 *   FUNCTIONS: pmmd_entry
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
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/rtc.h>
#include <sys/time.h>

#include <sys/pm.h>
#include <pmmi.h>
#include "pm_sig750.h"

/*-------------------------*/
/*  external variables     */
/*-------------------------*/
extern struct _pm_kernel_data	pm_kernel_data;
extern pmmd_IF_t		pmmd_IF;
extern pm_md_data_t		pm_md_data;


/*-------------------------*/
/*    external functions   */ 
/*-------------------------*/
extern void	turn_power_off();
extern void	write_ctl_bit(int byte_addr, int bit_addr, int data);


/*-------------------------*/
/*    function proto type  */
/*-------------------------*/
int 	pmmd_entry(int cmd, caddr_t arg);
void	LED_job(LED_block_t *arg);
void	BEEP_job(BEEP_block_t *arg);
void	resume_event_job(rsm_en_event_t *arg);
int	set_resume_time_job(struct _resume_time_data *res_time);

/*
 * NAME: pmmd_entry 
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
pmmd_entry(int cmd, caddr_t arg)
{
	struct	_rsm_trig_blk		*rsm_trig_blk;
	int				rc;

	switch (cmd) {
	case	set_pmmi_callback:
		pm_md_data.callback.pmmi_event_receive_entry =
		    ((callback_entry_block_t *)arg)->pmmi_event_receive_entry;
		pm_md_data.callback.hibernation_entry =
		    ((callback_entry_block_t *)arg)->hibernation_entry;
		return PM_SUCCESS;
			
	case	LED_control:
		LED_job((LED_block_t *)arg);
		return PM_SUCCESS;

	case	BEEP_control:
		BEEP_job((BEEP_block_t *)arg);
		return PM_SUCCESS;
		
	case	save_basic_devices:
		DBG(Calling "save_basic_device")
		pm_md_data.io_addr = (void *)iomem_att(&(pm_md_data.io_map));
		PmSystemSuspend(NULL);
		iomem_det(pm_md_data.io_addr);
		return PM_SUCCESS;
			
	case	restore_basic_devices:
		pm_md_data.io_addr = (void *)iomem_att(&(pm_md_data.io_map));
		PmSystemResume();
		iomem_det(pm_md_data.io_addr);
		MakeDECavailable();
		DBG("Restore_planar_device" is called to retrieve\
			the former device context)
		return PM_SUCCESS;
			
	case	enter_suspend: 
		return PM_ERROR;

	case	enter_hibernation:
		/*
		 * time maintained by CPU clock is more accurate than
		 * rtc chip's one. So the time of CPU is reflected to rtc chip
		 */
		DBG(Adjust system time )
		adjust_rtc_chip();

		pm_md_data.io_addr = (void *)iomem_att(&(pm_md_data.io_map));
		PmSystemHibernate( pm_md_data.callback.hibernation_entry,
		    turn_power_off);
		iomem_det(pm_md_data.io_addr);
		return PM_SUCCESS; 
			
	case	set_resume_time: 
		rc = set_resume_time_job((struct _resume_time_data *) arg);
		return rc;
			
	case	get_resume_event:
		/*
		 * Query the Signetics to see what the resume event was.
		 */
		rsm_trig_blk = (struct _rsm_trig_blk *) arg;
		if (1 == read_ctl_bit(SWITCH_TURNEDON_BYTE,
		    SWITCH_TURNEDON_BIT)) {
			rsm_trig_blk->event = PM_EVENT_POWER_SWITCH_ON;
		} else if (1 == read_ctl_bit(ALARM_TURNEDON_BYTE,
		    ALARM_TURNEDON_BIT)) {
			rsm_trig_blk->event = PM_EVENT_RTC;
		} else if (1 == read_ctl_bit(MODEM_TURNEDON_BYTE,
		    MODEM_TURNEDON_BIT)) {
			rsm_trig_blk->event = PM_EVENT_RING;
		} else {
			rsm_trig_blk->event = PM_EVENT_NONE;
		}
		return PM_SUCCESS;
			
	case	set_resume_event:
		/*
		 *  Set up resume on modem ring if requested.
		 */
		resume_event_job((rsm_en_event_t *) arg);
		return PM_ERROR;

	case	battery_job:
		return PM_ERROR;
			
	case	TurnPowerOff:
		turn_power_off();
		break;		/* never come here again */

	case	reconstruct_systime:	
		Reinit_SysTime();	/* Reconstruct system time */
		break;

	} /* endswitch */
	return PM_ERROR;	/* argument syntax error */
}

/*
 * NAME: LED_job
 *
 * FUNCTION: flash the LED
 *
 * NOTES: The LED_type (suspend or power-on) field is ignored since there
 *	  is only one LED.  When the control field indicates "blink", the
 *	  Signetics fast-flash will be used.  Other wise the LED will just
 *	  be turned on solid.
 *
 * RETURN VALUE DESCRIPTION:
 * 	  None.       
 */
void LED_job(LED_block_t *arg)
{
	if (arg->control == LED_blink) {
		write_ctl_bit(FAST_FLASH_LED_BYTE, FAST_FLASH_LED_BIT, 1);
	} else {
		write_ctl_bit(FAST_FLASH_LED_BYTE, FAST_FLASH_LED_BIT, 0);
		write_host_pin(LED_CONTROL, 0); /* Make sure LED is on */
	}
}

/*
 * NAME: BEEP_job
 *
 * FUNCTION: beep the speaker
 *
 * NOTES:
 *	The Signetics does not provide a beep function.  We will use
 *	port x61 instead to beep the internal speaker.  The type field
 *	will be ignored.
 *
 * RETURN VALUE DESCRIPTION:
 * 	  None.       
 */
void BEEP_job(BEEP_block_t *arg)
{
	int	tmp = 0;

	if ((arg->control == BEEP_on) || (arg->control == BEEP_oneshot)) {
#ifdef PM_DEBUG 
		if (arg->control == BEEP_on) {
			printf("pmmd:  BEEP_on ...\n");
		} else {
			printf("pmmd:  BEEP_oneshot ...\n");
		}
#endif /* PM_DEBUG */
		write_port(CONTROL_WORD_PORT, MODE3);
		write_port(SPEAKER_TONE_PORT, FREQ_LSB);
		__iospace_sync();
		write_port(SPEAKER_TONE_PORT, FREQ_MSB);
		tmp = read_port(SPEAKER_ENABLE_PORT);
		tmp = tmp & 0x0c;
		write_port(SPEAKER_ENABLE_PORT, tmp | SPEAKER_ON);
		if (arg->control == BEEP_oneshot) {
			io_delay(100000);	/* 100 ms beep */
			write_port(SPEAKER_ENABLE_PORT, tmp | SPEAKER_OFF);
		}
	} else {
#ifdef PM_DEBUG 
		printf("pmmd:  BEEP_off ...\n");
#endif /* PM_DEBUG */
		tmp = read_port(SPEAKER_ENABLE_PORT);
		tmp = tmp & 0x0c;
		if (arg->control == BEEP_off) {
			write_port(SPEAKER_ENABLE_PORT, tmp | SPEAKER_OFF);
		}
	}
}

/*
 * NAME: resume_event_job
 *
 * FUNCTION: enable ringing resume from hibernation
 *
 * NOTES: The only resume event supported in modem ring.  Set up Signetics
 *	  to enable this.
 *
 * RETURN VALUE DESCRIPTION:
 * 	  None.       
 */
void resume_event_job(rsm_en_event_t *rsm_en_event)
{
	if (rsm_en_event->MDM_ring_resume_from_hibernation == RESUME_ENABLE) {
		/*
		 *  Enable resume from hibernation on modem ring by
		 *  writing EXT_WAKEUP_ENABLE control bit (22.5).
		 */
		write_ctl_bit(EXT_WAKEUP_ENABLE_BYTE, EXT_WAKEUP_ENABLE_BIT, 1);
	}
	return;
}

/*
 * NAME: set_resume_time_job
 *
 * FUNCTION: set the resume alarm
 *
 * NOTES: The value given to us is one of 3 things:
 *        1) seconds since the epoch
 *		If the time is greater than curtime, set the alarm.
 *	  2) seconds since midnight
 *		Set the alarm for this time today if possible, else tomorrow.
 *        3) zero
 *		Disable the alarm.
 *
 * RETURN VALUE DESCRIPTION:
 * 	  PM_SUCCESS	The alarm has been set or disabled as instructed.
 *	  PM_ERROR	There was an error in the parameter.
 */
int set_resume_time_job(struct _resume_time_data *res_time)
{
	struct timestruc_t		ct;
	struct tms			mytm;
	uint				alarm_mins, today_secs;

	/*
	 * The alarm on Signetics has 1 minute granularity and may
	 * be off by 1 minute.  0=1 min, 1=2 min, etc.
	 */
	curtime(&ct);
	if (ct.tv_sec >= res_time->resume_at_specified_time) {
		if (res_time->resume_at_specified_time == 0) {
			/*
			 *  If the value is 0, we should disable the alarm.
			 */
#ifdef PM_DEBUG
			printf("deactivating the alarm ...\n");
#endif
			write_ctl_bit(ACTIVATE_ALARM_BYTE,
			    ACTIVATE_ALARM_BIT, 0);
			return PM_SUCCESS;
		} else if (res_time->resume_at_specified_time <= (24*60*60)) {
			/*
			 * A value less than 24 hours means the
			 * system should wake up every 24 hours.
			 * We just set the alarm once here, and
			 * we'll get called again when they
			 * hibernate again.
			 */
			mytm.secs = ct.tv_sec;
			/* nano secs to 100th secs */
			mytm.ms = ct.tv_nsec / (NS_PER_SEC/100);
			secs_to_date(&mytm);
			today_secs = (((mytm.hrs >> 4) * 10 +
			    (mytm.hrs & 0x0f)) * 60 * 60) +
			    (((mytm.mins >> 4) * 10 +
			    (mytm.mins & 0x0f)) * 60) +
			    ((mytm.no_secs >> 4) * 10 +
			    (mytm.no_secs & 0x0f));
			if (today_secs < res_time->resume_at_specified_time) {
				alarm_mins = (res_time->
				    resume_at_specified_time -
				    today_secs) / 60;
			} else {
				alarm_mins = ((res_time->
				    resume_at_specified_time +
				    24*60*60) - today_secs) / 60;
			}
		} else {
			/*
			 * An invalid time was set, return an error.
			 */
			return PM_ERROR;
		}
	} else {
		/*
		 * 0-59 secs = 1 min.
		 * 60-119 secs = 2 min.
		 * ...
		 */
		alarm_mins = (res_time->resume_at_specified_time -
		    ct.tv_sec) / 60;
	}
	write_data_word(ALARM0, alarm_mins & 0xff);
	write_data_word(ALARM1, (alarm_mins >> 8) & 0xff);
	write_data_word(ALARM2, (alarm_mins >> 16) & 0xff);
	write_ctl_bit(ACTIVATE_ALARM_BYTE, ACTIVATE_ALARM_BIT, 1);
#ifdef PM_DEBUG
	printf("Just set alarm for %d minutes.\n",alarm_mins);
#endif
	return PM_SUCCESS;
}
