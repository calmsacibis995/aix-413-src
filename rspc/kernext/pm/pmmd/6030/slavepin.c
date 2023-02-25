static char sccsid[] = "@(#)71  1.12  src/rspc/kernext/pm/pmmd/6030/slavepin.c, pwrsysx, rspc41J, 9524G_all 6/15/95 09:03:17";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Power Management interrupt(IRQ11) handling (bottom half)
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
#include <sys/inline.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/pm.h>
#include <sys/time.h>
#include "slave.h"
#include "pm_6030.h"
#include "planar.h"
#include "../../pmmi/pmmi.h"


/*
 ************************
 *  external label      *
 ************************
 */
extern struct   _pm_md_data   pm_md_data;    /* Global data */
extern struct   _pm_core_data pm_core_data;  /* ODM PM feature control data */
extern struct   _pm_isadev_data  pm_isadev_data; /* ISA GP data block */
extern pm_HW_state_data_t pm_HW_state_data;  /* HW state data */
extern pm_parameters_t pm_parameters;	     /* pmmi global data sent by pmd */
extern pm_hibernation_t pm_hibernation;


/* callback address block for calling back pmmi */
extern callback_entry_block_t  callback_entry_block; 

/* PM core control data block */
extern pm_control_data_t  pm_control_data; 

/*--external function---*/

   /*--another files ---*/
extern int  modify_pmc_nolock(int index, int modify_bits, int data_bits);
extern int  modify_pmc(int index, int modify_bits, int data_bits);
extern int  read_pmc_nolock(int index);
extern int  read_pmc(int index);
extern int  write_pmc_nolock(int index, char data);
extern int  write_pmc(int index, char data);
extern int  pm_planar_control_cpu(dev_t devno, int devid, int cmd);
extern int  modify_isadev_nolock(int addr, int modify_bits, int data_bits);
extern int  modify_isadev(int addr, int modify_bits, int data_bits);
extern int  read_isadev_nolock(int addr);
extern int  read_isadev(int addr);
extern int  write_isadev_nolock(int addr, char data);
extern int  write_isadev(int addr, char data);


/*
 ************************
 *  Function proto-type *
 ************************
 */

 /*  PINNED features  */
uchar       read_xioc(int index);

uchar       write_xioc(int index, uchar data);

int         read_xioc_status();

int         pm_planar_control_psumain(dev_t devno, int devid, int cmd);

int	    inform_slave_of_earlylowbattery(uchar Lowbat_forWhat, int time);

int         inform_slave_of_standby();

int         inform_slave_of_suspend();

int	    inform_slave_of_hibernation();

int         inform_slave_of_resume();

void	    PM_interrupt_control(int control);

void        ChangeLEDstate(char LED_dev, char LEDcmd, char blink_speed);

int         Send_slave_msg(SLAVE_BLOCK *block);

void        AlertUserWithBeep(char beep_t, uint beep_interval);

void        StopAlertUser();

int         pm_event_kproc();

void        enable_RTC_SQW();

int         check_SQW();

void        NeedSendCMD2H8forPMINT();

int         pm_intr(struct intr *intrp);

void        ObtainBatteryStatus(int data);

void 	    disable_RTC_bat_cntrl_clk();

int         init_slave_interface();

int         init_slave();

int         SpecialSlaveInitForNoteBook();

/*
 ************************
 *  Global data         *
 ************************
 */
EVENTCONTROL       pm_event_control    /* Master data for event Control block */
                     = {NULL,
                        EVENT_NULL,
                        EVENT_NULL,
                        operational_state,
                        NoExternalAttached,
                        0,0,0,0,0,0 };

PMEVENTCELL        pm_event_block      /* PM event buffer                 */
                     = {no_event_occurrence,
                        PM_EVENT_NONE};


struct _pm_planar_id_struct planar_devid; /* Planar devid from ODM scanning */

BATTERY_CTRL   battery_control = {{0},0,0,0};   /* Battery control data     */
volatile BATTERY_DATA   battery_block;  /* Battery latest data              */

ISA_DATA    pm_xioc_data;     /* Slave CPU I/O mapping data block  */
ISA_DATA    pm_RTC_data;        /* RTC access data block           */

struct intr PMI_block;          /* input data to register IRQ11    */


#define	PM_KBD_COVER_OPEN	200
#define	PM_KBD_COVER_CLOSE	201
#define	    MaxEvent		16

PMEVENTCELL EventRingBuff[MaxEvent];
int	    inP = 0;
int	    outP = 0;
int	    EventCount = 0;



/*
 ************************
 *   code body          *
 ************************
 **********************************************************
 *
 *  PM event kernel process (private)
 *
 **********************************************************
 */
int
pm_event_kproc()
{
	PMEVENTCELL XferEvent; 

        /*
         * Change parent process to avoid signal
         */
        DbgKproc(PM event kproc entry)
        setpinit();

        while(1)
        {
	     pm_event_control.pmmd_kproc_running = on_run;

            /*
             *  This kernel process is waken by PM interrupt.
             *  Only when actual event occurs, this kproc can
             *  run for it.
             */
            while (GetEvent(&XferEvent) != 0) {
		  DBG(PMMD kproc sleep due to no event)
		  pm_event_control.in_sleep = 1;
                  e_sleep (&(pm_event_control.EventAssertion), EVENT_SHORT);
		  pm_event_control.in_sleep = 0;
	    	  if (pm_event_control.KillPMMD_Kproc == TRUE) {
			break;
		  } /* endif */
	    } /* endwhile */

	    pm_event_control.mainpw_failsafe = 0; /* clear watch-dog counter */

	    if (pm_event_control.KillPMMD_Kproc == TRUE) {

                     /*
                      *  Check whether or not battery status query is on-going
                      */
		     if (battery_control.On_going_flags == battery_query){
                        /*
                         *  Set signal to make battery system
                         *  call be rejected.
                         */
                        battery_control.sig_unconfig=1;
                        battery_control.battery_count=0;
                        e_wakeup(&(pm_event_control.battery_status_ready));
                        /*
                         *  Yield control so that the process currently
                         *  requesting battery status can obtain control
                         *  and terminate itself soon without any lock.
                         */
			while (battery_control.On_going_flags == battery_query){
                        	delay(10);
			} /* endwhile */
                     } /* endif */

                     /*
                      * Terminate this kernel process itself
                      */
                     DbgKproc(Private kproc is killed)
		     pm_event_control.pmmd_kproc_running = NULL;
		     DBG(Getting out of pmmd_kproc)
                     return(0);

	    } /* endif */


            /*
             *  Analyize detected event
             */
            switch (XferEvent.EventType) {
               case RTS_batteryStatus:

                    DbgKproc(Battery status ready is informed to kproc)
		/* 
		 *  Need to consider the case in which both battery status and
		 * watchdog timer expiration occur between sending H8 cmds and
		 * falling into sleep(e_sleep()) when system is in stress test.
		 */ 
		      DBG(PMMD kproc: Wakeup due to the battery status ready)
		      battery_control.On_going_flags = NULL;
        	      simple_lock(&(battery_control.lock));
                      e_wakeup(&(pm_event_control.battery_status_ready));
        	      simple_unlock(&(battery_control.lock));
                      break;


               case ExternalEvent:

                      switch (XferEvent.EventDetail) {

                           case PM_EVENT_LID_CLOSE:
                              DbgKproc(PM event: LID close)
                              pm_HW_state_data.LID_state = PM_LID_CLOSE;
        		      modify_pmc( INDEXPMC_PMCO0,
                           	BITPMCPMCO0_LCDBACKLIGHTOFF, RESET_BITS );
			      DBG(LCD backlight OFF due to LID_close)
                              break;

                           case PM_EVENT_LID_OPEN:
                              DbgKproc(PM event: LID open)
                              pm_HW_state_data.LID_state = PM_LID_OPEN;
        		      if (pm_HW_state_data.video_dd_request 
							== PM_PLANAR_ON) {
                	      	modify_pmc( INDEXPMC_PMCO0,
                                   BITPMCPMCO0_LCDBACKLIGHTOFF, SET_BITS );
				DBG(LCD backlight ON due to LID_open)
			      }
				/* no break, here */			
                           case PM_EVENT_MAIN_POWER:
                           case PM_EVENT_KEYBOARD:
                           case PM_EVENT_PD_CLICK:
    			      if (pm_control_data.phase_1_only == TRUE) {
                                 /*
                                  * Whenever either LID_open or main power SW
                                  * transition occurs and the system state is
                                  * in partial suspend state, the event causes
                                  * the system to resume from the suspend.
                                  */

                                 /*
                                  * The activty of ext-keyboard and PD click
                                  * only occurs during suspend state. Because
                                  * they are only enabled to occur right before
                                  * entering suspend state.
                                  */

                              		if (pm_event_control.internal_state
                                                !=operational_state) {
                                 		(void)inform_slave_of_resume();

                              		} /* endif */
                              } /* endif */
                              break;

                           case PM_EVENT_AC:
                              /* LCD backlight full brightness mode */
                              pm_HW_state_data.ACDC_state = in_AC;
                              modify_pmc( INDEXPMC_PMCO0,
                                          BITPMCPMCO0_LCDHALFBRIGHTNESS,
                                          SET_BITS );
                              pm_event_control.H8cmdSend=ReqAC_inoutBeep;
                              break;

                           case PM_EVENT_DC:
                              /* LCD backlight half brightness mode */
	          	      pm_HW_state_data.ACDC_state = in_DC;
                              modify_pmc( INDEXPMC_PMCO0,
                                          BITPMCPMCO0_LCDHALFBRIGHTNESS,
                                          RESET_BITS );
                              pm_event_control.H8cmdSend=ReqAC_inoutBeep;
                              break;
	
			   case PM_KBD_COVER_OPEN: 
                              pm_event_control.H8cmdSend=ReqKBDcoverBeep;
			      break;

			   case PM_KBD_COVER_CLOSE: 
                              pm_event_control.H8cmdSend=ReqStopBeepForKBDclose;
			      break;

                      } /* endswitch */

                     /*
                      *  Inform PMMI(machine independent module) of PM event
                      */
		      DBG(External Event is propagated to PMMD kernel proc())

		      if (pm_control_data.phase_1_only == TRUE) {

			  DBG("This event is processed by PHASE_1 routine.")
                      	  (void)pm_set_event (XferEvent.EventDetail);

		      } else {
			  if(callback_entry_block.
					pmmi_event_receive_entry != NULL) {
			     DBG("This event is processed by PHASE_2 routine") 

			     (*(callback_entry_block.pmmi_event_receive_entry))
						   (XferEvent.EventDetail);

			  } else {
            			XferEvent.EventType=NULL;
				XferEvent.EventDetail = NULL;	
				/* Clear event forcedly */

				/*
				 *  Later, we need to gather the initial state 
				 * of pmmd because such events which occurs 
				 * before PM daemon's query are forcedly 
				 * ignored here. 
				 */

			  } /* endif */
		      } /* endif */

                      break;


               case MonitorSQW:
                  /*
                   *  In fact, this is a software workaround to continue
                   *  to enable PM interrupt. PM chip uses RTC SQW
                   *  clock though it may be disabled asynchronously by a
                   *  kernel mode routine. Fortunately, although it's disabled,
                   *  at least one more PM interrupt can be generated.
                   *  Therefore, PM interrupt handler every time causes this
                   *  kernel process to check SQW availability so that it
                   *  can be ensured to be always enabled.
                   */
                      DbgKproc(Check SQW)
                      break;

            } /* endswitch */


            /*
             *  When SQW is detected to be already disabled, it may be disabled
             *  even though it was once enabled if it is unexpectedly disabled
             *  again by someone(kernel mode routine) within 5 msec after
             *  enabling it.
             *  So there is 20 msec waiting routine in this kernel process
             *  to prove that at least for the 5 msec SQW was continuously
             *  enabled after re-enabling.
             *  If this 5 msec can be ensured, PM interrupt can be generated
             *  once though SQW is unfortunately disabled again. Hence, we
             *  can always ensure to recover this issue.
             */
            delay(2);
            while (!(check_SQW())) {

                   DbgKproc(Enable SQW)
                   enable_RTC_SQW();
                   delay (2);

            } /* endwhile */


            /*
             * Check whether or not PM interrupt handler requested
             * to do something like sending a command related to H8
             * which sometimes requires a long time to complete.
             *
             * This is also effective when "inform_slave_of_resume"
             * which is called in this kproc and located above needs
             * to send some command to slave.
             */
            NeedSendCMD2H8forPMINT();

            /*
             * Clear the communication word before issuing EOI.
             * This word must not be cleared after EOI since it's
             * capable of being accessed by both kproc and IRQ11.
             */
            XferEvent.EventType=NULL;

        } /* endwhile */

} /* PMMD kernel process */



/*
 ***********************************************
 * Check whether or not PM interrupt handler
 * requested to do something related to H8 which
 * should not be achieved in interrupt handler
 * since it sometimes takes a long time.
 ***********************************************
 */
void
NeedSendCMD2H8forPMINT()
{
        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        switch (pm_event_control.H8cmdSend) {

            case  SuppressKBevent:

                  DbgKproc(Send H8 command to disable resume by ext-KBD)

            case  DisRsmByExtKBDactivity:

                  DbgKproc(Send H8 command to suppress KB activity notice)
                  build_slave_block(action_manager,
                                    SetOnLineEventFlag,
                                    KeyboardDevice,
                                    ext_KBD_plug_in      \
                                    +ext_KBD_plug_out,
                                    NULL);
                  (void)Send_slave_msg(Pslave_Txblock);
                  break;

            case  EnableExtKBDactivity:

                  DbgKproc(Send H8 command to enable KB activity notice)
                  build_slave_block(action_manager,
                                    SetOnLineEventFlag,
                                    KeyboardDevice,
                                    KBD_any_key          \
                                    +ext_KBD_plug_in     \
                                    +ext_KBD_plug_out,
                                    NULL);
                  (void)Send_slave_msg(Pslave_Txblock);
                  break;


            case  DisPDclickresume:

                  DbgKproc(Send H8 command to disable PD click resume)
                  build_slave_block(action_manager,
                                    SetOnLineEventFlag,
                                    PointD_ClickDevice,
                                    NULL,
                                    NULL);
                  (void)Send_slave_msg(Pslave_Txblock);
                  break;

            case  ReqKBDcoverBeep:

                  DbgKproc(Send H8 command to request KBD cover open beep)
                  build_slave_block(Beep_Device,
                                    Make_beep,
                                    WarningInOffLine,
                                    1*8,              /* interval=125msec x 8 */
                                    NULL);
                  (void)Send_slave_msg(Pslave_Txblock);

                  modify_isadev( PORTXIOC_AUDIOSUPPORT,
                                     BITXIOCAUDIOSUPPORT_SPEAKERMUTE,
                                     RESET_BITS );
                  break;

            case  ReqAC_inoutBeep:

                if (pm_control_data.system_state != PM_SYSTEM_FULL_ON) {

                  DbgKproc(Send H8 command to request AC/DC transition beep)
                  if (pm_event_control.internal_state==partial_suspend_state) {
                     modify_isadev( PORTXIOC_AUDIOSUPPORT,
                                        BITXIOCAUDIOSUPPORT_SPEAKERMUTE,
                                        RESET_BITS );

                     AlertUserWithBeep(DC_IN_OUT_Beep, ONE_SHOT);
                     delay (70);

                     modify_isadev( PORTXIOC_AUDIOSUPPORT,
                                        BITXIOCAUDIOSUPPORT_SPEAKERMUTE,
                                        SET_BITS );
                  } else {
                     AlertUserWithBeep(DC_IN_OUT_Beep, ONE_SHOT);
                  } /* endif */
       		} /* endif */

                  break;

            case  ReqStopBeepForKBDclose:

                  if (pm_event_control.internal_state==partial_suspend_state) {
                     modify_isadev( PORTXIOC_AUDIOSUPPORT,
                                        BITXIOCAUDIOSUPPORT_SPEAKERMUTE,
                                        SET_BITS );
                  } /* endif */

            case  ReqStopBeep:

                  DbgKproc(Send H8 command to stop the current beep)
                  build_slave_block(Beep_Device,
                                    StopRepeatedBeep,
                                    NULL,
                                    NULL,
                                    NULL);
                  (void)Send_slave_msg(Pslave_Txblock);
                  break;

        } /* endswitch */

        pm_event_control.H8cmdSend=NULL;
        return;
}



/*
 **********************************************************
 *  According to the action for low battery, low battery notice
 *  is programmed as to how early the notice should be generated.
 *  This is to extend the battery working time as much as possible.
 *
 **********************************************************
 */
void
SetupLowbatteryEvent()
{
 	uchar 	Lowbat_forWhat; 
	int 	time;

	if (pm_hibernation.slist == NULL) {
		/* 
		 * When hibernation volume can't be created due to disk full 
		 * and low battery event occur, PM daemon performs shutdown.
		 * So low battery event must be generated early enough to
		 * complete the shutdown operation.
		 */
			Lowbat_forWhat = early_low_battery;
			time = 240;	/* 240 sec earlier than exhaust */
	} else {
		switch (pm_parameters.daemon_data.low_battery_action) {
		case PM_SYSTEM_SHUTDOWN:
			Lowbat_forWhat = early_low_battery;
			time = 240;	/* 240 sec earlier than exhaust */
			break;

		case PM_SYSTEM_HIBERNATION:
			Lowbat_forWhat = early_low_battery;
			time = 120;	/* 120 sec earlier than exhaust */
			break;

		case PM_SYSTEM_SUSPEND:
		default:
			Lowbat_forWhat = low_battery;
			time = 120;	/* 120 sec earlier than exhaust */ 
					/* to support suspend to hibernation */
			break;

		} /* endswitch */
	} /* endif */

	(void)inform_slave_of_earlylowbattery(Lowbat_forWhat, time);

	return;
}



/*
 **********************************************************
 *
 *  Inform slave CPU of the necessity of earlier notice of
 *  low battery state.  Because low battery event may be
 *  used for shutdown which is different from suspend
 *  as to the required time to complete.
 *  For instance, slave CPU is normally programmed through
 *  an appropriate command so that low battery notice can
 *  occurs 240 sec before actual battery exhaust occurs if
 *  low battery event is used to invoke shutdown.
 *
 **********************************************************
 */
int
inform_slave_of_earlylowbattery(uchar Lowbat_forWhat, int time)
{
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        if (time > 360) {
            time = Early480sec;
        } else if (time > 240) {
            time = Early360sec;
        } else if (time > 120) {
            time = Early240sec;
        } else if (time > 60 ) {
            time = Early120sec;
        } else {
            time = Normal60sec;
        } /* endif */

        build_slave_block(Battery_Device, SetBatteryNoticeTime,
                                          time,
                                          NULL,
                                          NULL);

        if (Send_slave_msg(Pslave_Txblock) !=-1) {

               build_slave_block(action_manager,
                             SetOnLineEventFlag,
                             PowerMonitorDevice,
                             Lowbat_forWhat + AC_mode + SuspendToHiber,
                             DC_mode + Event_Status_reset);
               if (Send_slave_msg(Pslave_Txblock) != -1) {

                     build_slave_block(action_manager,
                             SetOffLineEventFlag,
                             PowerMonitorDevice,
                             SuspendToHiber,
                             NULL);
                     if (Send_slave_msg(Pslave_Txblock)!=-1) {

        		return (PM_SUCCESS);

                     } /* endif */
               } /* endif */
        } /* endif */

        return (PM_ERROR);

}



/*
 **********************************************************
 *
 *  Disable suspend to hibernation  
 *  Note that this routine should be called in case of 
 *  hibernation volume can't have just been detected right 
 *  before entering suspend mode.  
 *
 **********************************************************
 */
int
Disable_suspend2hibernation()
{
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        build_slave_block(action_manager,
                     SetOffLineEventFlag,
                     PowerMonitorDevice,
                     NULL,
                     NULL);
        Send_slave_msg(Pslave_Txblock);
        return (PM_SUCCESS);
}



/*
 **********************************************************
 *
 *  Sync to slave CPU to enter
 *   standby
 *
 **********************************************************
 */
int
inform_slave_of_standby()
{
        return (PM_SUCCESS);
}

/*
 **********************************************************
 *
 *  Sync to slave CPU to enter
 *  	hibernation 
 *
 **********************************************************
 */
int	    
inform_slave_of_hibernation()
{
	/* do nothing in case of Woodfield. */
	return (PM_SUCCESS);
}


/*
 **********************************************************
 *
 *  Sync to slave CPU to enter suspend
 *
 **********************************************************
 */
int
inform_slave_of_suspend()
{

    /* Tx cmd/data buffer  */
    SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;
    SLAVE_BLOCK    slave_Rxblock,*Pslave_Rxblock=&slave_Rxblock;
    DbgKmode(Entry: Suspend procedure of slave control)

    if (pm_control_data.phase_1_only == TRUE) {
        build_slave_block(action_manager, SetOnLineEventFlag,
                                        PointD_ClickDevice,
                                        Click_depressed,
                                        NULL);
        if (Send_slave_msg(Pslave_Txblock)==-1) {
              DbgKmode(Failure exit: enable resume trigger of PD click)
              return (PM_ERROR);

        } /* endif */

        if (pm_event_control.ExternalKBD == ExternalAttached) {
              DbgKmode(Going with external keyboard attached)

              build_slave_block(action_manager, SetOnLineEventFlag,
                                       KeyboardDevice,
                                       KBD_any_key         \
                                       +ext_KBD_plug_in    \
                                       +ext_KBD_plug_out,
                                       NULL);
              if (Send_slave_msg(Pslave_Txblock)==-1)   {
                 DbgKmode(Failure exit: Suspend procedure of slave control)
                 return (PM_ERROR);
              } /* endif */
              DbgKmode(Enable resume trigger of any key activity)

        } /* endif */


        AlertUserWithBeep(Partial_Suspend_Beep, ONE_SHOT);
        ChangeLEDstate(Power_LED_Device,TurnOffLED,NULL);
        ChangeLEDstate(Suspend_LED_device,TurnOnLED,NULL);

        pm_event_control.internal_state=partial_suspend_state;

        delay (70);  /* Wait suspend beep to complete to be generated. */
        modify_isadev( PORTXIOC_AUDIOSUPPORT,
                               BITXIOCAUDIOSUPPORT_SPEAKERMUTE, SET_BITS );

        DbgKmode(Successfully exit:
                Suspend procedure of slave control(phase 1))
        return (PM_SUCCESS);

   } 
   else
   {

	while (!(Receive_slave_msg(Pslave_Rxblock))) {

	/* If any msg from slave at this moment, ignore it. */

	    	/* clear internal ext-PMI */
            	(void)read_pmc_nolock(INDEXPMC_PM_REQ_STS);

	    	/* clear ext-PMI signal   */
            	(void)write_pmc_nolock(INDEXPMC_PMI_CLR,0);  
            	(void)write_xioc(slave_SCR,slave_EOI);

        	if (!(check_SQW())) {
           		enable_RTC_SQW();
        	} /* endif */

	} /* endwhile */
	
           build_slave_block(action_manager,
                                        SetOffLineEventFlag,
                                        MainPowerDevice,
                                        power_off_request,
                                        NULL);
           if (Send_slave_msg(Pslave_Txblock)!=-1) {

              build_slave_block(action_manager,
                                        SetOffLineEventFlag,
                                        LIDSwitchDevice,
                                        LIDopen_notice,
                                        NULL);
              if (Send_slave_msg(Pslave_Txblock)!=-1) {

                  build_slave_block(action_manager,
                                        SetOffLineEventFlag,
                                        PointD_ClickDevice,
                                        Click_depressed,
                                        NULL);
                  if (Send_slave_msg(Pslave_Txblock)!=-1) {

                        build_slave_block(action_manager,
                                        SetOffLineEventFlag,
                                        KeyboardDevice,
                                        ext_KBD_off_line,
                                        NULL);
                        if (Send_slave_msg(Pslave_Txblock)!=-1) {

        		   build_slave_block(StateManager, 
					StartSuspend,
                                        NULL,
                                        NULL,
                                        NULL);
        		   if (Send_slave_msg(Pslave_Txblock)!=-1) {

                              build_slave_block(StateManager,
                                        CompSuspend,
                                        NULL,
                                        NULL,
                                        NULL);
                              if (Send_slave_msg(Pslave_Txblock)!=-1) {

                                 pm_event_control.internal_state
                                                         = full_suspend_state;
                                 AlertUserWithBeep(Suspend_Beep, ONE_SHOT);
                  	 	 io_delay(1000*990);  /* to hear the beep */
				 /* max < 1 sec, unit: usec */

                                 return (PM_SUCCESS);

                              } /* endif */
                           } /* endif */
                        } /* endif */
                  } /* endif */
              } /* endif */
           } /* endif */
	DBG(Failure exit: Suspend procedure of slave control)
        return (PM_ERROR);
   } /* endif */

} /* inform_slave_of_suspend() */



/*
 **********************************************************
 *
 *  Sync to slave CPU to get out
 *   of standby/suspend mode
 *
 **********************************************************
 */
int
inform_slave_of_resume()
{
    /* Tx cmd/data buffer  */
    SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;
    /* Rx cmd/data buffer  */
    SLAVE_BLOCK    slave_Rxblock,*Pslave_Rxblock=&slave_Rxblock;
    int	rsm_trigger = NULL;

    DbgKmode(Entry: Resume procedure of slave control)

    if (pm_control_data.phase_1_only == TRUE) {
        if (pm_event_control.internal_state==partial_suspend_state) {
           /*
            * The following requests to send cmds to slave are executed
            * in pm private kproc, which is in this source file,
            * after calling this "inform_slave_of_resume" function.
            */
           pm_event_control.H8cmdSend=DisPDclickresume;
           NeedSendCMD2H8forPMINT();
           pm_event_control.H8cmdSend=SuppressKBevent;
           NeedSendCMD2H8forPMINT();
        } /* endif */


        modify_isadev( PORTXIOC_AUDIOSUPPORT,
                           BITXIOCAUDIOSUPPORT_SPEAKERMUTE, RESET_BITS );

        pm_event_control.internal_state=operational_state;
        AlertUserWithBeep(Suspend_Beep, ONE_SHOT);
        ChangeLEDstate(Power_LED_Device,TurnOnLED,NULL);
        ChangeLEDstate(Suspend_LED_device,TurnOffLED,NULL);

        DbgKmode(Successfully exit:
                Resume procedure of slave control)
        return (PM_SUCCESS);

   }
   else
   {
       	if (!(check_SQW())) {
        	enable_RTC_SQW();
       	} /* endif */

        pm_event_control.internal_state=operational_state;
        build_slave_block(StateManager, StartResume,
                                        NULL,
                                        NULL,
                                        NULL);
        if (Send_slave_msg(Pslave_Txblock)!=-1) {

		while (1) {
		    if (!(Receive_slave_msg(Pslave_Rxblock))) {
	
	    		/* clear internal ext-PMI */
            		(void)read_pmc_nolock(INDEXPMC_PM_REQ_STS);

	    		/* clear ext-PMI signal   */
            		(void)write_pmc_nolock(INDEXPMC_PMI_CLR,0);  
            		(void)write_xioc(slave_SCR,slave_EOI);

			if ((Pslave_Rxblock->TRx_whom == action_manager) &&
			    (Pslave_Rxblock->TRx_msg  == NoMoreEvent)) {
				break;

			} else if (Pslave_Rxblock->TRx_msg 
						     == EventNotification) {

				switch(Pslave_Rxblock->TRx_whom) {
				case PowerMonitorDevice:
			    		rsm_trigger = 
						PM_EVENT_CRITICAL_LOW_BATTERY;
					break;
				case PointD_ClickDevice:
			    		rsm_trigger = PM_EVENT_MOUSE;
					break;
				case LIDSwitchDevice:
			    		rsm_trigger = PM_EVENT_LID_OPEN;
					break;
				case KeyboardDevice:
			    		rsm_trigger = PM_EVENT_KEYBOARD;
					break;
				case MainPowerDevice:
			    		rsm_trigger = PM_EVENT_POWER_SWITCH_ON;
					break;
				} /* endswitch */
			} /* endif */
		    } /* endif */
		} /* endwhile */

           	build_slave_block(StateManager, CompResume,
                                        NULL,
                                        NULL,
                                        NULL);
	        if (Send_slave_msg(Pslave_Txblock)!=-1) {

			/* To get the current state of LID, here is a code
			 * to make slave CPU generate LID interrupt. So
			 * even on LID open, LID open interrupt is generated.
			 * Since LID state can be only obtained by the notice	
			 * based on slave interrupt.
			 */
           		build_slave_block(action_manager,
                             SetOnLineEventFlag,
                             LIDSwitchDevice,
                             LIDopen_notice
                             +LIDclose_notice,
                             Event_Status_reset);
           		if (Send_slave_msg(Pslave_Txblock) != -1) {

				/* Debug purpose only */
             			/* AlertUserWithBeep(Suspend_Beep, ONE_SHOT); */

             	 		DbgKmode(Successfully exit:
                     			Resume procedure of slave control)

             			return (rsm_trigger);
			} /* endif */

		} /* endif */
	} /* endif */
	return (rsm_trigger);
   } /* endif */

} /* inform_slave_of_resume() */


/*
 **********************************************************
 * PM interrupt enable/disable control
 *  interrupt origin itself is masked on or off here.
 **********************************************************
*/
void	    
PM_interrupt_control(int control)
{
	switch (control) {
	case Enable_PM_int:
		write_xioc(slave_BCR,0x04);
		break;

	case Disable_PM_int:	
		write_xioc(slave_BCR,0x00);
		break;
	}
	return;
}


/*
 **********************************************************
 *
 *  Powering main power off is controlled by slave CPU(H8).
 *  Hence, the communication to H8 is required before
 *  entering suspend.
 *
 **********************************************************
 */
int
pm_planar_control_psumain(dev_t devno, int devid, int cmd)
{
	volatile int a=1;

        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        DbgDump(Planar_control_psumain: Order to H8 to turn off the machine)
        switch (cmd) {

            case PM_PLANAR_OFF:

                  /*
                   * 300 msec waiting for beep completion
                   * (Not use kernel service due to critical situation)
                   */
		  
		  (void)disable_RTC_bat_cntrl_clk();
                  AlertUserWithBeep(Power_Off_Beep, ONE_SHOT);
                  io_delay(1000*990);  /* to hear the beep */
                  /* max < 1 sec, unit: usec */

                  build_slave_block(StateManager,MainPowerOff,
                                                 NULL,
                                                 NULL,
                                                 NULL);
                  if (Send_slave_msg(Pslave_Txblock)==-1) 
			return (PM_ERROR);
		  wait_forever();
		  break;	/* never come here */

            case PM_PLANAR_ON:
            case PM_PLANAR_LOWPOWER1:
            case PM_PLANAR_LOWPOWER2:
                  return (PM_SUCCESS);

            case PM_PLANAR_QUERY:
		return(PM_PLANAR_OFF);

	    case PM_PLANAR_CURRENT_LEVEL:
		return (PM_PLANAR_ON);

        } /* endswitch */
}



/*
 **********************************************************
 *
 *    Power Management interrupt(IRQ11) handler
 *
 *---------------------------------------------------------
 * Communicate with slave through eXtended I/O Controller registers to
 * check PM activity and detect which PM event occurred.
 **********************************************************
 *
 *  <<<< Important note to handle slave CPU interrupt >>>>
 *
 *  Note that slave CPU generates event interrupt even when actual
 *  event doesn't occur in case that the corresponding event is
 *  newly enabled or disabled through the command to the slave CPU.
 *  This virtual event interrupt can be used to know the current
 *  state since the current state can't be inquired to the slave
 *  CPU directly. However, such a virtual event interrupt must be
 *  distinguished according to the current system status. And all
 *  the unexpected  virtual event must be discarded. Esepcially at
 *  the resume process, some events notice are disabled since they
 *  are desired to be generated only during suspend state. In
 *  response to this disabling, such a virtual interrupt results
 *  in being generated. It must be discarded.
 *                                                 end of comment
 **********************************************************
 */
int
pm_intr(struct intr *intrp)
{
	PMEVENTCELL ThisEvent;

        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        /* Rx cmd/data buffer  */
        SLAVE_BLOCK    slave_Rxblock,*Pslave_Rxblock=&slave_Rxblock;

        DbgPMint(==================================);
        DbgPMint(======= enter pm_interrupt =======);
        DbgPMint(==================================);

        Pslave_Rxblock=&slave_Rxblock;

        if (Receive_slave_msg(Pslave_Rxblock)==-1) {

            return (INTR_FAIL);

        } else {

            ThisEvent.EventType = MonitorSQW;
            ThisEvent.EventDetail=PM_EVENT_NONE;

            switch (Pslave_Rxblock->TRx_whom) {

                case action_manager:
               /* NoMoreEvent cmd may be received, but it can be ignored. */
                   break;

                case HOST_IF_Manager:
               /* Only invalid response may be received. */
                   break;

                case Revision_Manager:
               /* As long as no revision is inquired, no cmd is received here.*/
                   break;

                case Battery_Device:

                   if (++(battery_control.battery_count) >= 5) {
                        ThisEvent.EventType = RTS_batteryStatus;
			DBG(PMMD PM intr: 5 battery data have just arrived)
                   } /* endif */

                   switch (Pslave_Rxblock->TRx_msg) {

                      case BatteryStatusReport:

                         ObtainBatteryStatus(Pslave_Rxblock->TRx_para0);
                         battery_block.obat.full_charge_count=0;
                         battery_block.ibat.full_charge_count_low
                                     =Pslave_Rxblock->TRx_para1;
                         battery_block.ibat.full_charge_count_high=0;/*always*/
                         break;


                      case TotalCapacityReport:

                         ObtainBatteryStatus(Pslave_Rxblock->TRx_para0);
                         battery_block.obat.capacity=0;
                         battery_block.ibat.capacity_low
                                     =Pslave_Rxblock->TRx_para1;
                         battery_block.ibat.capacity_high
                                     =Pslave_Rxblock->TRx_para2;
                         break;


                      case CurrentCapacityReport:

                         ObtainBatteryStatus(Pslave_Rxblock->TRx_para0);
                         battery_block.obat.remain=0;
                         battery_block.ibat.remain_low
                                     =Pslave_Rxblock->TRx_para1;
                         battery_block.ibat.remain_high
                                     =Pslave_Rxblock->TRx_para2;
                         break;


                      case DischargedCapacityReport:

                         ObtainBatteryStatus(Pslave_Rxblock->TRx_para0);
                         battery_block.obat.discharge_remain=0;
                         battery_block.ibat.discharge_remain_low
                                     =Pslave_Rxblock->TRx_para1;
                         battery_block.ibat.discharge_remain_high
                                     =Pslave_Rxblock->TRx_para2;
                         break;


                      case DischargedTimeReport:

                         ObtainBatteryStatus(Pslave_Rxblock->TRx_para0);
                         battery_block.obat.discharge_time=0;
                         battery_block.ibat.discharge_time_low
                                     =Pslave_Rxblock->TRx_para1;
                         battery_block.ibat.discharge_time_high
                                     =Pslave_Rxblock->TRx_para2;
                         break;

                   } /* endswitch */

                   break;


                case MainPowerDevice:

                   if (Pslave_Rxblock->TRx_msg != EventNotification) {
                        break;
                   } else if (!(Pslave_Rxblock->TRx_para0
                                               & power_off_request)) {
                        break;
                   } /* endif */

                   ThisEvent.EventType = ExternalEvent;
                   ThisEvent.EventDetail=PM_EVENT_MAIN_POWER;

                   break;


                case KeyboardDevice:
                   /* In phase1, external kbd event is used. */
                   /* In Phase2, ext_kbd_off_line is used.   */

                  if (Pslave_Rxblock->TRx_msg == EventNotification) {

                     (Pslave_Rxblock->TRx_para0) &= (KBD_any_key+
                                                     ext_KBD_plug_in+
                                                     ext_KBD_plug_out+
                                                     ext_KBD_off_line);

                     switch (Pslave_Rxblock->TRx_para0) {

                        case KBD_any_key:

                           if (pm_event_control.internal_state
                                             == partial_suspend_state) {
                              /*-----------------------------------
                               *  Resume event at partial suspend
                               *-----------------------------------
                               *  Any key resume is only enabled in
                               *  case that external keyboard is
                               *  attached effectively.
                               */
                              ThisEvent.EventType = ExternalEvent;
                              ThisEvent.EventDetail=PM_EVENT_KEYBOARD;

                           } /* endif */

                           break;


                        case ext_KBD_plug_in:

                           if (pm_event_control.ExternalKBD
                                                ==NoExternalAttached) {
                              pm_event_control.ExternalKBD = ExternalAttached;


                              if (pm_event_control.internal_state
                                                == partial_suspend_state) {

                                 pm_event_control.H8cmdSend
                                                =EnableExtKBDactivity;

                              } /* endif */
                           } /* endif */

                           break;


                        case ext_KBD_plug_out:

                           /*
                            *  This notice is only generated at init
                            *  and resume from real suspend state.
                            */
                           pm_event_control.ExternalKBD = NoExternalAttached;
                           break;


                        case ext_KBD_off_line:

                           /*
                            *  This external event is one of resume trigger
                            *  in real suspend state. When external keyboard
                            *  is attached, it's not turned off during suspend
                            *  state in order to receive the depressing its key.
                            */

                           ThisEvent.EventType = ExternalEvent;
                           ThisEvent.EventDetail=PM_EVENT_KEYBOARD;

                           /*
                            * It's not necessary to disable ext-kbd activity
                            * notice such as one here because it's only
                            * generated in off-line meaning real suspend state.
                            */

                           break;

                     } /* endswitch */
                  } /* endif */
                  break;


                case KBDcoverDevice:

                  if (Pslave_Rxblock->TRx_msg != EventNotification) {

                        break;

                  } else if ((Pslave_Rxblock->TRx_para0) & KBDopen_notice){

                         ThisEvent.EventType = ExternalEvent;
                         ThisEvent.EventDetail= PM_KBD_COVER_OPEN;

                  } else if (Pslave_Rxblock->TRx_para0 & KBDclose_notice) {

                         ThisEvent.EventType = ExternalEvent;
                         ThisEvent.EventDetail= PM_KBD_COVER_CLOSE;

                  } /* endif */
                  break;


                case LIDSwitchDevice:

                   if (Pslave_Rxblock->TRx_msg != EventNotification) {

                         break;

                   } else if (Pslave_Rxblock->TRx_para0 & LIDopen_notice) {

                         ThisEvent.EventType = ExternalEvent;
                         ThisEvent.EventDetail=PM_EVENT_LID_OPEN;

                   } else if (Pslave_Rxblock->TRx_para0 & LIDclose_notice) {

                         ThisEvent.EventType = ExternalEvent;
                         ThisEvent.EventDetail=PM_EVENT_LID_CLOSE;

                   } /* endif */

                   /*
                    *  DDX/Display DD will order to DIM after detectin here.
                    */
                   break;


                case PointD_ClickDevice:

                   if ((Pslave_Rxblock->TRx_msg) == EventNotification) {

                        if ((Pslave_Rxblock->TRx_para0) & Click_depressed) {

                           switch (pm_event_control.internal_state) {

                              case partial_suspend_state:
                              case full_suspend_state:
                                 ThisEvent.EventType = ExternalEvent;
                                 ThisEvent.EventDetail = PM_EVENT_PD_CLICK;
                                 break;

                           } /* endswitch */
                        } /* endif */
                   } /* endif */

                   break;


                case PowerMonitorDevice:

                   if (Pslave_Rxblock->TRx_msg != EventNotification) {

                         break;

                   } else if (Pslave_Rxblock->TRx_para0
                              & (low_battery | early_low_battery))   {

                         ThisEvent.EventType = ExternalEvent;
                         ThisEvent.EventDetail=PM_EVENT_LOW_BATTERY;

                   } else if (Pslave_Rxblock->TRx_para0 & AC_mode)   {

                         ThisEvent.EventType = ExternalEvent;
                         ThisEvent.EventDetail=PM_EVENT_AC;
                         pm_event_control.ACDCmode=AC;
                   } else if (Pslave_Rxblock->TRx_para1 & DC_mode)   {

                         ThisEvent.EventType = ExternalEvent;
                         ThisEvent.EventDetail=PM_EVENT_DC;
                         pm_event_control.ACDCmode=DC;
                   }
                   break;


            } /* endswitch */
        } /* endif */


	/*
	 * PM external interrupt event is put into the buffer.
	 */
	PutEvent(&ThisEvent);


        /*
         *  Actually, any external event results in calling "e_wakeup" below.
         *  Because RTC SQW check for software workaround is required once
         *  PM interrupt occurs.
         */
        if (ThisEvent.EventType)  {

            e_wakeup(&(pm_event_control.EventAssertion));
            DbgPMint(Event detected)

        } /* endif */


        if (!(check_SQW())) {
           /*
            *  Since SQW is detected to be ALREADY stopped,
            *  private kernel process starts to monitor the
            * state after enalbing it.
            */
           enable_RTC_SQW();

        } /* endif */

	/*
	 *  Issue EOI to slave CPU I/F register
	 */
	/* clear internal ext-PMI */
        (void)read_pmc_nolock(INDEXPMC_PM_REQ_STS);

	/* clear ext-PMI signal   */
        (void)write_pmc_nolock(INDEXPMC_PMI_CLR,0);  
        (void)write_xioc(slave_SCR,slave_EOI);

        if (ThisEvent.EventDetail == PM_EVENT_MAIN_POWER) { 
		if (++(pm_event_control.mainpw_failsafe) >= 7) { 
			DBG(Failsafe power-off in intr handler)
    			pm_planar_control_psumain(0,0, PM_PLANAR_OFF);
		} /* endif */
#ifdef PM_DEBUG
	    printf("failsafe counter = %D\n", pm_event_control.mainpw_failsafe);
#endif
	} /* endif */

        return (INTR_SUCC); /* return INTR_FAIL when the processing fails */

}



/*
 ******************************************************
 *
 *  Pick up the effective flags of battery
 *   status which slave CPU transfers.
 *
 ******************************************************
 */
void
ObtainBatteryStatus(int data)
{
        battery_block.ibat.attrib &= ~(PM_BATTERY
                                      +PM_BATTERY_EXIST
                                      +PM_NICD
                                      +PM_CHARGE
                                      +PM_DISCHARGE
                                      +PM_AC
                                      +PM_DC
                                      +battery_dischg_req);

        battery_block.ibat.attrib |= PM_BATTERY;

        if (data & slave_bat_attached) {
             battery_block.ibat.attrib |= PM_BATTERY_EXIST;
        } /* endif */

        if (!(data & slave_bat_NiMH)) {
             battery_block.ibat.attrib |= PM_NICD;
        } /* endif */

        if (data & slave_bat_charge) {
             battery_block.ibat.attrib |= PM_CHARGE;
        } /* endif */

        if (data & slave_bat_discharge) {
             battery_block.ibat.attrib |= PM_DISCHARGE;
        } /* endif */

        if (data & slave_bat_dischg_req) {
             battery_block.ibat.attrib |= battery_dischg_req;
        } /* endif */

        if (pm_event_control.ACDCmode == AC) {
             battery_block.ibat.attrib |= PM_AC;
        } /* endif */

        if (pm_event_control.ACDCmode == DC) {
             battery_block.ibat.attrib |= PM_DC;
        } /* endif */

        return;

}



/*
 ***************************
 *
 *  Transmit slave command
 ***************************
 *
 *  Environment:
 *       kernel mode only
 *       Not in interrupt
 *
 ***************************
 */
int
Send_slave_msg(SLAVE_BLOCK *block)
{
        if (CheckH8Status() != 0) {
            return (-1);
        } /* endif */

        (void)write_xioc(slave_SCR, Clear_Data_FIFO);
        (void)write_xioc(slave_SDR, block->TRx_msg);
        (void)write_xioc(slave_SDR, block->TRx_para0); /* Data reg is FIFO */
        (void)write_xioc(slave_SDR, block->TRx_para1);
        (void)write_xioc(slave_SDR, block->TRx_para2);
        (void)write_xioc(slave_SCR, block->TRx_whom);

        return (0);
}



/*
 ***************************
 *
 *  Check H8 current status
 *
 ***************************
 */
int
CheckH8Status()
{
        uint   t;
        uchar  data;
	struct	timestruc_t ct;

#ifdef PM_DEBUG
	curtime(&ct);
	printf("Check H8 status(start) %D\n", ct.tv_sec);	
#endif
        data=read_xioc_status();
        for (t=1;
             ( ((data & slave_status_ready)==0) || ((data & slave_busy)!=0) );
             t++) {

               if (t> 2*1000*5) {
                  DbgDump(Error: Slave is illegally not ready)
                  return (-1);  /* fail safe 5 sec timeous */

               } else {

                  io_delay(500);  /* Wait 500 usec */

               } /* endif */

               data=read_xioc_status();

        } /* endfor */
#ifdef PM_DEBUG
	curtime(&ct);
	printf("Check H8 status(end) %D\n", ct.tv_sec);	
#endif

        return (0);

}



/*
 ***************************
 *
 *  Receive slave command
 *
 ***************************
 *  Environment:
 *    Both kernel mode
 *       and interrupt mode
 ***************************
 */
int
Receive_slave_msg(SLAVE_BLOCK *block)
{
        if ((read_xioc_status()) & slave_interrupt) {

               block->TRx_whom  = read_xioc(slave_UCR);
               block->TRx_msg   = read_xioc(slave_UDR0);
               block->TRx_para0 = read_xioc(slave_UDR1);
               block->TRx_para1 = read_xioc(slave_UDR2);
               block->TRx_para2 = read_xioc(slave_UDR3);

               return (0);

        }
        return (-1);

}



/*
 ***************************
 *
 *  Read one of essention I/Os in ISA
 *
 ***************************
 */
uchar
read_xioc(int index)
{
        volatile uchar  *io;
        uchar           data;
        int             opri;

        /* serialize access to pm_xioc_data */
        opri = disable_lock(INTMAX, &(pm_xioc_data.lock));

        /* I/O access start */
        io = (volatile uchar *)iomem_att(&(pm_xioc_data.map))
                                                + pm_xioc_data.base;

        *((volatile uchar *)io) = (uchar)index;
	eieio();
        data = *((volatile uchar *)(io + 1));
	eieio();

        /* I/O access end */
        iomem_det((void *)io);

        unlock_enable(opri, &(pm_xioc_data.lock));

        return data;
}

/*
 ***************************
 *
 *  Write one of essention I/Os in ISA
 *
 ***************************
 */
uchar
write_xioc(int index, uchar data)
{
        volatile uchar  *io;
        int             opri;

        /* serialize access to pm_xioc_data */
        opri = disable_lock(INTMAX, &(pm_xioc_data.lock));

        /* I/O access start */
        io = (volatile uchar *)iomem_att(&(pm_xioc_data.map))
                                                + pm_xioc_data.base;

        *((volatile uchar *)io) = (uchar)index;
	eieio();
        *((volatile uchar *)(io + 1)) = data;
	eieio();

        /* I/O access end */
        iomem_det((void *)io);

        unlock_enable(opri, &(pm_xioc_data.lock));

        return data;
}



/*
 ***************************
 *
 *  Read one of essention I/Os in ISA
 *
 ***************************
 */
uchar
read_xioc_nolock(int index)
{
        volatile uchar  *io;
        uchar           data;
        int             opri;

        /* I/O access start */
        io = (volatile uchar *)iomem_att(&(pm_xioc_data.map))
                                                + pm_xioc_data.base;

        *((volatile uchar *)io) = (uchar)index;
	eieio();
        data = *((volatile uchar *)(io + 1));
	eieio();

        /* I/O access end */
        iomem_det((void *)io);

        return data;
}


/*
 ***************************
 *
 *  Write one of essention I/Os in ISA
 *
 ***************************
 */
uchar
write_xioc_nolock(int index, uchar data)
{
        volatile uchar  *io;
        int             opri;

        /* I/O access start */
        io = (volatile uchar *)iomem_att(&(pm_xioc_data.map))
                                                + pm_xioc_data.base;

        *((volatile uchar *)io) = (uchar)index;
	eieio();
        *((volatile uchar *)(io + 1)) = data;
	eieio();

        /* I/O access end */
        iomem_det((void *)io);

        return data;
}



/*
 ***************************
 *
 *  Check SQW is still enabled
 *
 ***************************
 */
int
check_SQW()
{
        volatile uchar  *io;
        int             opri;
        uchar           data;

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
         * Read status regB
         */
        *((volatile uchar *)io) = RTC_Status_regB;
	eieio();
        data = (*((volatile uchar *)(io + 1)));
	eieio();


         /*
          * I/O access end
          */
         iomem_det((void *)io);


         /*
          *  I/O access end
          */
         unlock_enable(opri, &(pm_xioc_data.lock));


         return ((data & RTC_SQWE) != 0);

}


/*
 ***************************
 *
 *  Re-enable RTC SQW clock for PM chip
 *
 ***************************
 */
void
enable_RTC_SQW()
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
         *  index for externeded control REG 4B
         */
        *((volatile uchar *)io) = Ext_Control_reg4B;
	eieio();
        data = *((volatile uchar *)(io + 1));
	eieio();
        data |= (AuxiliaryBatteryEnable + Enable32kSQWoutput);
        *((volatile uchar *)(io + 1)) = data;
	eieio();


        /*
         *  Restore the former value of RTC status regA
         */
        *((volatile uchar *)io) = RTC_Status_regA;
	eieio();
        *((volatile uchar *)(io + 1)) = former_data;
	eieio();


        /*
         *  Enabler SQW
         */
        *((volatile uchar *)io) = RTC_Status_regB;
	eieio();
        data = *((volatile uchar *)(io + 1));
	eieio();
        data |= RTC_SQWE;
        *((volatile uchar *)(io + 1)) = data;
	eieio();


        /* I/O access end */
        iomem_det((void *)io);

        unlock_enable(opri, &(pm_xioc_data.lock));

        return;
}


/*
 ***************************
 *
 * Disable AuxBatteryEnable and 32kSQW 
 *
 ***************************
 */
void
disable_RTC_bat_cntrl_clk()
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
         *  index for externeded control REG 4B
         */
        *((volatile uchar *)io) = Ext_Control_reg4B;
	eieio();
        data = *((volatile uchar *)(io + 1));
	eieio();
        data &= ~(AuxiliaryBatteryEnable + Enable32kSQWoutput);
        *((volatile uchar *)(io + 1)) = data;
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

        unlock_enable(opri,  &(pm_RTC_data.lock));

        return;
}



/*
 ***************************
 *
 *  Read essential device in ISA bus
 *
 ***************************
 */
int
read_xioc_status()
{
        volatile uchar  *io;
        uchar           data;
        int             opri;

        /* serialize access to pm_xioc_data */
        opri = disable_lock(INTMAX, &(pm_xioc_data.lock));

        /* I/O access start */
        io = (volatile uchar *)iomem_att(&(pm_xioc_data.map))
                                                + pm_xioc_data.base;

        data = *((volatile uchar *)(io + 2));

        /* I/O access end */
        iomem_det((void *)io);

        unlock_enable(opri, &(pm_xioc_data.lock));

        return (int)data;
}



/*
 ***************************
 *
 *  Alert user with BEEP
 *
 ***************************
 */
void
AlertUserWithBeep(char beep_t, uint beep_interval)
{
        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;
        uint   t;

	if (((pm_control_data.phase_1_only == TRUE) &&
	     (pm_core_data.beep == PM_BEEP_ON))
				||
	    ((pm_control_data.phase_1_only == FALSE) &&
	     (pm_parameters.core_data.pm_beep == TRUE))) {

            t = beep_interval * 8;                 /* 1 unit = 125 msec */
            if (t > 0x0ff) {
                   t = 0x0ff;
            }
            build_slave_block(Beep_Device, Make_beep,
                                           beep_t,
                                           (uchar)t,
                                           NULL);
            (void)Send_slave_msg(Pslave_Txblock);   
		/* Any error is ignored here.*/

        } /* endif */

        return;

}



/*
 ***************************
 *
 *  Stop the BEEP for alerting user
 *
 ***************************
 */
void
StopAlertUser()
{
        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        build_slave_block(Beep_Device, StopRepeatedBeep,
                                       NULL,
                                       NULL,
                                       NULL);
        (void)Send_slave_msg(Pslave_Txblock);   /* Any error is ignored here.*/
}



/*
 ***************************
 *
 *  PM LED control
 *
 ***************************
 */
void
ChangeLEDstate(char LED_dev, char LEDcmd, char blink_speed)
{
        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        char   dt=0;

        if (LEDcmd == BlinkLED) {
               dt=blink_speed;
        }
        build_slave_block(LED_dev, LEDcmd,
                                   dt,
                                   NULL,
                                   NULL);
        (void)Send_slave_msg(Pslave_Txblock);   /* Any error is ignored here.*/
}



/*
 **********************************************************
 * NAME: init_slave_interface
 *
 * FUNCTION: Initialize I/F portion between host and Slave CPU(h8)
 *
 * NOTES: I/F registers are located in eXtended I/O Controller (ISA I/O chip)
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
init_slave_interface()
{
        int    rc;
        DbgKmode(Init_slave_interface entry)

        /*
         *  Check slave ready after boot
         */
        DbgKmode(Check H8 ready)
        if ((rc = read_xioc_status()) & slave_status_ready) {

            /* Enable slave interrupt */
            (void)modify_pmc_nolock(INDEXPMC_PM_CNTRL,
                                 BITPMCPMCNTRL_PM_EN,
                                 0);
            (void)modify_pmc_nolock(INDEXPMC_PM_CNTRL,
                                 BITPMCPMCNTRL_PM_EN,
                                 BITPMCPMCNTRL_PM_EN);
            (void)modify_pmc_nolock(INDEXPMC_PM_CNTRL,
                                 BITPMCPMCNTRL_PMI_CTRL
                                 +BITPMCPMCNTRL_PMI_CC_PMI,
                                 BITPMCPMCNTRL_PMI_CTRL);

            /*
             *  Initialize slave internal control data
             */
            DbgKmode(Internal H8 init)
            if (init_slave() ==0)  {

                write_xioc(slave_BCR,0x04);

                DbgKmode(Init_slave_interface exit)
                return (0);

            } else {

                (void)modify_pmc_nolock(INDEXPMC_PM_CNTRL,
                                         BITPMCPMCNTRL_PM_EN,
                                         0);
                return(-1);

            } /* endif */

        } else {

            DbgKmode(H8 is not ready)
            return(-1);

        } /* endif */
}



/*
 **********************************************************
 * NAME: init_slave
 *
 * FUNCTION: Initialize Slave CPU itself
 *
 * NOTES: Several commands are issued to slave CPU through IO chip
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
init_slave()
{
        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        /*
         * Init: If ext-kbd is already attached,
         *       H8 will informs it through PM interrupt.
         */
        pm_event_control.ExternalKBD = NoExternalAttached;

        /*
         * Enable PM event detection caused by slave CPU.
         */
        DbgKmode(Send cmd - EnableEventNotice)
        build_slave_block(action_manager,
                                       EnableEventNotice,
                                       NULL,
                                       NULL,
                                       NULL);
        if (Send_slave_msg(Pslave_Txblock) != -1) {

            DbgKmode(Send cmd - SetOnLineEventFlag for MainPowerDevice)
            build_slave_block(action_manager, SetOnLineEventFlag,
                                       MainPowerDevice,
                                       power_off_request,
                                       NULL);
            if (Send_slave_msg(Pslave_Txblock) != -1) {

               DbgKmode(Send cmd - SetOnLineEventFlag for ext-KBD attachment)
               build_slave_block(action_manager, SetOnLineEventFlag,
                                       KeyboardDevice,
                                       ext_KBD_plug_in
                                       +ext_KBD_plug_out,
                                       Event_Status_reset);
               if (Send_slave_msg(Pslave_Txblock) != -1) {

                  if (SpecialSlaveInitForNoteBook() == -1) {

                        return (-1);

                  } /* endif */

                  DbgKmode(Send cmd - SetPowerOffWatchDog)
                  /*
                   * "SetPowerOffWatchDog" is a command to program the failsafe
                   * timer by slave CPU regarding main power off request. Since
                   * main power off is controlled by software, it's possible
                   * that main power can't be turned off if the software
                   * would hang up. Hence, slave CPU has a timer to forcedly
                   * turn the main power off if the system(PM) wouldn't
                   * process the interrupt(IRQ11) within the programmed
                   * time.
                   */
                  build_slave_block(action_manager,
                                       SetPowerOffWatchDog,
                                       2,          /* 2 seconds timeout for */
                                       0,          /* main power fail safe  */
                                       NULL);
                  if (Send_slave_msg(Pslave_Txblock) != -1) {

                     /*
                      *  Enable events here, which can be
                      *  detected during suspend state
                      *     (phase-2)
                      */

                     DbgKmode(Init slave complete)
                     return (0);

                  } /* endif */
               } /* endif */
            } /* endif */
        } /* endif */

        DbgKmode(Init slave failure)
        return (-1);

}



/*
 **********************************************************
 * NAME: SpecialSlaveInitForNoteBook
 *
 * FUNCTION: Special init of slave CPU 
 *
 * NOTES: Notebook has LID/Battery/Keyboard cover/Built-in Pointing Device.
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
SpecialSlaveInitForNoteBook()
{
        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        DbgKmode(Send cmd - SetOnLineEventFlag for KBDcoverDevice)
        build_slave_block(action_manager,
                             SetOnLineEventFlag,
                             KBDcoverDevice,
                             KBDclose_notice
                             +KBDopen_notice,
                             Event_Status_reset);
        if (Send_slave_msg(Pslave_Txblock) != -1) {

           DbgKmode(Send cmd - SetOnLineEventFlag for LIDSwitchDevice)
           build_slave_block(action_manager,
                             SetOnLineEventFlag,
                             LIDSwitchDevice,
                             LIDopen_notice
                             +LIDclose_notice,
                             Event_Status_reset);
           if (Send_slave_msg(Pslave_Txblock) != -1) {

              DbgKmode(Send cmd - SetOnLineEventFlag for PointingDevice)
              build_slave_block(action_manager,
                             SetOnLineEventFlag,
                             PointD_ClickDevice,
                             NULL,
                             NULL);
              if (Send_slave_msg(Pslave_Txblock) != -1) {

                        return (0);

              } /* endif */
           } /* endif */
        } /* endif */

        return (-1);
}




/*
 * NAME: PutEvent
 *
 * FUNCTION: put the specified PM external interrupt event 
 *
 * INPUT: PMEVENTCELL *event
 *
 * return: 0 - correctly saved  
 *	   -1 - oldest one is gone. 
 */
int
PutEvent(PMEVENTCELL *event)
{
	if (inP == MaxEvent) {
		inP = 0;
	}
#ifdef PM_DEBUG
	printf("PutEvent: Event cell address = %08X\n", &EventRingBuff[inP]);
#endif
	memcpy(&EventRingBuff[inP++],event,sizeof(PMEVENTCELL));

	if (++EventCount > MaxEvent) {

		EventCount = MaxEvent;
		if (outP++ == MaxEvent) {	/* the oldest one is gone */
			outP = 0;
		}
		return -1;
	}
  	return 0;
}



/*
 * NAME: GetEvent
 *
 * FUNCTION: get the oldest PM external interrupt event 
 *
 * INPUT: PMEVENTCELL *event
 *
 * return:  0  - get an effective event 
 *	    -1 - no event 
 */
int
GetEvent(PMEVENTCELL *event)
{
        int             opri;

        opri = disable_lock(INTMAX, &(pm_HW_state_data.lock));

	if (EventCount-- == 0) {
		inP = 0;	/*****************/
		outP = 0; 	/*  fail safe 	 */ 
		EventCount = 0; /*****************/
        	unlock_enable(opri, &(pm_HW_state_data.lock));
		return -1;
	}
	if (outP == MaxEvent) {
		outP = 0;
	}
#ifdef PM_DEBUG
	printf("GetEvent: Event cell address = %08X\n", &EventRingBuff[outP]);
#endif
	memcpy(event, &EventRingBuff[outP++],sizeof(PMEVENTCELL));

        unlock_enable(opri, &(pm_HW_state_data.lock));

	return 0;
}

