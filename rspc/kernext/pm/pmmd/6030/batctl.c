static char sccsid[] = "@(#)59  1.7  src/rspc/kernext/pm/pmmd/6030/batctl.c, pwrsysx, rspc41J, 9524G_all 6/14/95 12:01:21";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Battery control
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

#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/sleep.h>
#include <sys/lock_alloc.h>
#include <sys/watchdog.h>
#include <sys/pm.h>
#include "../../pmmi/pmmi.h"
#include "pm_6030.h"
#include "slave.h"


/************************/
/*   External label     */
/************************/
/*-------------------------------------------*/
/*    external data                          */
/*-------------------------------------------*/
extern EVENTCONTROL pm_event_control; /* Sync main and int for battery */
extern BATTERY_CTRL battery_control;  /* Battery control datat         */
extern volatile BATTERY_DATA battery_block;  /* Battery latest data    */
extern struct _pm_md_data pm_md_data;   /* Global data */
extern struct watchdog mywatchdog;      /* watchdog timer structure */

/*-------------------------------------------*/
/*    external function                      */
/*-------------------------------------------*/
extern uchar read_xioc();
extern uchar write_xioc();
extern int   read_xioc_status();
/*-------------------------------------------*/


/************************/
/*  Function proto-type */
/************************/
void battery_handler(battery_control_t *arg);
void init_battery_func(int cmd);
struct pm_battery * get_battery_status();
int set_battery_discharge();
void battery_query_timeout();


/*-------------------------------------------*/
int	battery_pending = 0;



/*-------------------------------------*/
/*   code body                         */
/*-------------------------------------*/
/*
 **********************************************************
 * NAME: battery_handler 
 *
 * FUNCTION: Gateway for the functions of 
 *		- Get the current battery status 
 *	        - Discharge battery
 *
 * INPUT:  none
 *
 * EXECUTION ENVIRONMENT: process environment
 *
 **********************************************************
 */
void 
battery_handler(battery_control_t *arg)
{
	pm_battery_data_t *batdat;

	switch (arg->control) {
	case discharge_battery:
		set_battery_discharge();
		break;	

	case query_battery:
		if (batdat=(pm_battery_data_t *)get_battery_status()) {
			memcpy(arg->battery_status, batdat, 
				sizeof(pm_battery_data_t));
		} /* endif */
		break;

	} /* endswitch */
	return;
}


/*
 **********************************************************
 * NAME: get_battery_status
 *
 * FUNCTION: Get the current battery status
 *
 *            - Rated battery capacity
 *            - Last measured capacity
 *            - Remaining level
 *            - Remaining level during discharging
 *            - Elapsed time of discharging
 *            - Full charge count without full-discharging
 *
 *            - Battery support capability
 *            - Battery attach
 *            - NiCd/NiMH
 *            - Charge/discharge
 *            - AC/DC mode
 *
 * NOTES:  It takes several msec to complete to obtain
 *
 * INPUT:  none
 *
 * EXECUTION ENVIRONMENT: process environment
 *
 * RETURN VALUE: pointer to battery block data
 *               Null for non-battery machine
 *
 **********************************************************
 */
struct pm_battery *
get_battery_status()
{
        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        char   battery_query_cmd[6] = {QueryBatteryStatus,
                                       QueryTotalCapacity,
                                       QueryCurrentCapacity,
                                       DischargedCapacity,
                                       QueryDischargedTime,
                                       NULL};
        char   *p;

	DBG(Battery query function has been called now.)
	if ((battery_pending != 0) && (battery_pending <=4)) {

		DBG(battery_query_pending)
		battery_pending++;
        	if (battery_control.battery_count >= 5) {
			battery_pending = 0;
        		battery_control.battery_count=0;
        		return ((struct pm_battery *)&battery_block);
		} else {
			DBG(battery status data not ready yet)
            		return((struct pm_battery *)NULL);
		} /* endif */

	} else {
#ifdef PM_DEBUG
		if (battery_pending > 4) {
		    DBG(Battery status has been given up after 31 seconds)
		}
#endif		
		battery_pending = 0;
        	battery_control.battery_count=0;
	} /* endif */

        if (battery_control.sig_unconfig) {
            /*
             * This sig_unconfig is set in slave.c when unconfig is
             * requested. In case that another process calls battery
             * system call and is blocked at the above "lock" due to
             * the former outstanding query, the checking here avoid
             * unconfigure" request
             */
            return((struct pm_battery *)NULL);
  
        } /* endif */

  
        /*
         * Set up a watchdog timer to avoid unlimited waiting slave CPU
         * to complete to generate several interrupts for battery status.
         */
        mywatchdog.next = NULL;
        mywatchdog.prev = NULL;
        mywatchdog.func = (void (*)())battery_query_timeout;
        mywatchdog.count = 0;
        mywatchdog.restart = 2;         /* 2 second (max) */
        w_init(&mywatchdog);


	/* 
	 * Send 5 battery commands with pm int disabled 
	 */
	DBG(Start to send battery query commands to H8)
  	battery_control.On_going_flags = battery_query;
        simple_lock(&(battery_control.lock));
        for (p=battery_query_cmd; (*p) != NULL; p++) {
            build_slave_block(Battery_Device, (*p),
                                             NULL,
                                             NULL,
                                             NULL);
            if (Send_slave_msg(Pslave_Txblock)==-1) {
        		simple_unlock(&(battery_control.lock));
			DBG(Error in sending battery query command to H8)
            		return((struct pm_battery *)NULL);
 	    } /* endif */ 
        } /* endfor */
  
  
	DBG(Fall in sleep in waiting for battery query completion)

        w_start(&mywatchdog);
        e_sleep_thread ( &(pm_event_control.battery_status_ready), 
		   	 &(battery_control.lock),
			 LOCK_SIMPLE);
        w_clear(&mywatchdog);
        simple_unlock(&(battery_control.lock));

	/* This clearance is effective for the case of unconfiguration. */
        battery_control.On_going_flags = NULL;	

	DBG(Wakeup due to battery status ready or watchdog timeout) 
  

        /*
         * Check whether or not unconditional unconfigure request occurs
         * Except for the case of "unconfigure", "e_sleep" above never
         * returns back until the complete battery status are obtained.
         */
        /* Also check battery status didn't complete to arrive within the
         * expected period.
         */
        if ((battery_control.sig_unconfig==1) || (battery_pending != 0)) {

        /* Because unconfigure request caused "e_sleep" to be released */
        /* Or because battery status itself didn't reach yet */
            return((struct pm_battery *)NULL);

        } /* endif */

	DBG(Exit of battery query function)  
        return ((struct pm_battery *)&battery_block);
  
  }
  
  
/*
 **********************************************************
 * NAME: battery_query_timeout
 *
 * FUNCTION: fail safe operation.
 *           If slave CPU doesn't complete to gather battery information
 *           within a reserved period, this routine is called by watchdog
 *           kernel services.
 *
 * NOTE:  This routine is called by kernel if timeout(2 seconds) occurs.
 *
 * INPUT:  none
 *
 * RETURN VALUE:
 *        none
 *
 **********************************************************
 */
void
battery_query_timeout()
{
  	if (battery_control.On_going_flags == battery_query) {

           if (!(check_SQW())) {
             	enable_RTC_SQW();
           } /* endif */

	   DBG(Watchdog timer has just expired due to incomplete battery status)
	   battery_pending++;
           e_wakeup(&(pm_event_control.battery_status_ready));
    	   battery_control.On_going_flags = NULL;
#ifdef PM_DEBUG
  printf("battery_control.battery_count = %d\n",battery_control.battery_count);
#endif
	} /* endif */

        return;

} /* battery_query_timeout */




/*
 **********************************************************
 * NAME: set_battery_discharge
 *
 * FUNCTION: Start to discharge battery for calibration
 *           Caller is responsible for checking the availability
 *           of AC power and battery before calling this function.
 *
 * NOTES:  The status of discharging can be gotten using
 *         "get_battery_status() above.
 *
 * INPUT:  none
 *
 * EXECUTION ENVIRONMENT: process environment
 *
 * RETURN VALUE: 0: Successfully return (discharging has just started)
 *              -1: Fail to start battery discharge
 *
 **********************************************************
 */
int
set_battery_discharge()
{
        /* Tx cmd/data buffer  */
        SLAVE_BLOCK    slave_Txblock,*Pslave_Txblock=&slave_Txblock;

        build_slave_block(Battery_Device, EnabelRefresh,
                                          NULL,
                                          NULL,
                                          NULL);
        return (Send_slave_msg(Pslave_Txblock));
}


/*
 **********************************************************
 * NAME: Init_battery_func 
 *
 * FUNCTION: Initialize battery control data
 *
 * NOTES: Slave CPU controls battery. Slave CPU supports the following
 *        command under the control of HOST PM code through the commands.
 *
 *               - battery status query
 *               - discharge start/stop
 *               - low/critical low  battery notice
 *               - external event notice (LID state,KB cover,main power sw)
 *
 * INPUT: PM_CONFIG
 *	  PM_UNCONFIG 
 *
 * EXECUTION ENVIRONMENT: process enviroment
 *
 * RETURN VALUE: none
 *
 **********************************************************
 */
void
init_battery_func(int cmd)
{
	switch (cmd) {

	case PM_CONFIG:
        	lock_alloc(&(battery_control.lock), LOCK_ALLOC_PIN,
                                        PM_CORE_LOCK_CLASS, -1);
        	simple_lock_init(&(battery_control.lock));
        	battery_control.battery_count=0;
        	battery_control.sig_unconfig=0;
        	battery_block.obat.attribute=0;
        	battery_block.obat.capacity=0;
        	battery_block.obat.remain=0;
        	battery_block.obat.discharge_remain=0;
        	battery_block.obat.discharge_time=0;
        	battery_block.obat.full_charge_count=0;
		break;

	case PM_UNCONFIG:
           	lock_free(&(battery_control.lock));
		break;

	} /* endswitch */

	return;

} /* init_battery_func */

/*------------------ End of file ------------------------------*/

