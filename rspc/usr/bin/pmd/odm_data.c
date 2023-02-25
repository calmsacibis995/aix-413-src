static char sccsid[] = "@(#)99  1.6  src/rspc/usr/bin/pmd/odm_data.c, pwrcmd, rspc41J, 9520A_all 5/12/95 14:36:41";
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS: pm_attribute_check, read_odm_data
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <odmi.h>
#include "pmd.h"
#include "pmd_global_ext.h"
#include "pmodm.h"

int check_pm_attribute(int, PM_attribute);
int read_odm_data(pmd_parameters_t *); 

/*
 * NAME:     check_pm_attribute
 *
 * FUNCTION: check the specified flag includes the specified attribute.
 *
 * INPUT:    flag , pm_attribute
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE  : specified flag includes specified pm_attribute
 *           FALSE : specified flag does not include specified pm_attribute
 *
 * NOTE:  refer to pmsptpnp.h in src/rspc/kernext/pm/pmmi
 *
 */
int 
check_pm_attribute(int flag, PM_attribute pm_attribute)
{
    return( (flag & pm_attribute) != 0 );
}


/*
 * NAME:  read_odm_data
 *
 * FUNCTION: get pmd_parameters_t data
 *
 * DATA STRUCTURES:
 *        pmdp    points a pmd_parameters_t structure
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : successfully processed
 *        PM_ERROR   : unsuccessfully processed
 *
 */
int
read_odm_data(pmd_parameters_t *pmdp)
{
   int val1 = 0, val2 = 0;
   int errcount = 0;
   char buf[32]; 

   Debug0("read_odm_data\n");

   /* initialize */
   memset(pmdp,0,sizeof(pmd_parameters_t));

   /* read residual data to set system supported transition */

   pmdp->system_support = PM_NONE;
   pmdp->firmware_support = 0;

   if ( firmware_support(&val2) != 0 ){
       fprintf(stderr,MSGSTR(ERR_RES_DATA,DFLT_ERR_RES_DATA));
       return PM_ERROR;
   }
   else{
       pmdp->firmware_support = val2;
   }

/*----------------------------------------------------------*/
/*   firmware_support has the following bits.               */
/*----------------------------------------------------------*/
/* hibernation_support                             = 0x0001 */
/* suspend_support                                 = 0x0002 */
/* LID_support                                     = 0x0004 */
/* battery_support                                 = 0x0008 */
/* ringing_resume_from_hibernation                 = 0x0010 */
/* ringing_resume_from_suspend                     = 0x0020 */
/* resume_from_hibernation_at_specified_time       = 0x0040 */
/* resume_from_suspend_at_specified_time           = 0x0080 */
/* hibernation_from_suspend_at_specified_time      = 0x0100 */
/* software_controllable_main_power_switch         = 0x0200 */
/* general_purpose_input_pin_for_resume_trigger    = 0x0400 */
/* fail_safe_hardware_main_power_off_switch        = 0x0800 */
/* resume_button_support                           = 0x1000 */
/* inhibit_auto_transition                         = 0x2000 */
/*----------------------------------------------------------*/

   /* check system support */

   if ( check_pm_attribute( pmdp->firmware_support , suspend_support) ){
       Debug0("SUSPEND is supported by system \n");
       pmdp->system_support
	   |= PM_SYSTEM_SUSPEND;
   }		

   /* check hibernation support */

   if ( check_pm_attribute( pmdp->firmware_support , hibernation_support) ){
       Debug0("HIBERNATION is supported by system \n");
       pmdp->system_support
	   |= PM_SYSTEM_HIBERNATION;
   }
   pmdp->system_support
       |= PM_SYSTEM_FULL_ON |
	   PM_SYSTEM_ENABLE |
	       PM_SYSTEM_STANDBY |
		   PM_SYSTEM_SHUTDOWN;

   RDebug1(64," system supported state is %x \n",pmdp->system_support);
   RDebug1(64," firmware supported flag is %x \n",pmdp->firmware_support);

   /* Now read odm data */

   if ( odm_initialize() == -1 ){
      fprintf(stderr,MSGSTR(ERR_ODM_INIT,DFLT_ERR_ODM_INIT));
      return PM_ERROR;
   }
   /* for runtime debugging */
   if(get_pmmi_data("reserved1", &val1, &val2) == 0){
       pmdp->reserved1 = val2;
   }
   else{
       Debug0(" odm does not have reserved1 entry.\n");
   }

   if(get_pmmi_data("reserved2", &val1, &val2) == 0){
      pmdp->reserved2 = val2;
   }
   else{
      Debug0(" odm does not have reserved2 entry.\n");
   }

   /* check firmware_support on Runtime debugging mode */

   if (check_pm_attribute(pmdp->firmware_support,hibernation_support)){
       RDebug0(64,"residual:hibernation_support is on \n");
   }
   else{
       RDebug0(64,"residual:hibernation_support is off \n");
   }

   if (check_pm_attribute(pmdp->firmware_support,suspend_support)){
       RDebug0(64,"residual:suspend_support is on \n");
   }
   else{
       RDebug0(64,"residual:suspend_support is off \n");
   }

   if (check_pm_attribute(pmdp->firmware_support,LID_support)){
       RDebug0(64,"residual:LID_support is on \n");
   }
   else{
       RDebug0(64,"residual:LID_support is off \n");
   }

   if (check_pm_attribute(pmdp->firmware_support,battery_support)){
       RDebug0(64,"residual:battery_support is on \n");
   }
   else{
       RDebug0(64,"residual:battery_support is off \n");
   }

   if (check_pm_attribute(pmdp->firmware_support,ringing_resume_from_hibernation)){
       RDebug0(64,"residual:ringing_resume_from_hibernation on.\n");
   }
   else{
       RDebug0(64,"residual:ringing_resume_from_hibernation off.\n");
   }

   if (check_pm_attribute(pmdp->firmware_support,ringing_resume_from_suspend)){
       RDebug0(64,"residual:ringing_resume_from_suspend on.\n");
   }
   else{
       RDebug0(64,"residual:ringing_resume_from_suspend off.\n");
   }

   if (check_pm_attribute(pmdp->firmware_support,resume_from_hibernation_at_specified_time)){
       RDebug0(64,"residual:resume_from_hibernation_at_specified_time is on \n");
   }
   else{
       RDebug0(64,"residual:resume_from_hibernation_at_specified_time is off \n");
   }

   if (check_pm_attribute(pmdp->firmware_support,resume_from_suspend_at_specified_time)){
       RDebug0(64,"residual:resume_from_suspend_at_specified_time is on \n");
   }
   else{
       RDebug0(64,"residual:resume_from_suspend_at_specified_time is off \n");
   }

   if (check_pm_attribute(pmdp->firmware_support,inhibit_auto_transition)){
       RDebug0(64,"residual:inhibit_auto_transition is on \n");
   }
   else{
       RDebug0(64,"residual:inhibit_auto_transition is off \n");
   }
   /* debug message end */

   /* initial system state */
   if (get_pmmi_data("sys_state", &val1, &val2) == 0){

       if (val2 == PM_SYSTEM_FULL_ON || val2 == PM_SYSTEM_ENABLE){
	   pmdp->pm_present_state.state = val2;
	   pmdp->pm_target_state.state = val2;
       }
       else{
	   pmdp->pm_present_state.state = val1;
	   pmdp->pm_target_state.state = val1;
	   fprintf(stderr,
		   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		   "sys_state");
       }
   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"sys_state");
       errcount++;
   }

   /* transition in case system idle timer expired  */

   if (get_pmmi_data("sys_idle_action", &val1, &val2) == 0){
       if (is_pm_action_state(val2)){
	   if ( (val2 & pmdp->system_support || val2 == PM_NONE)
	       && !( check_pm_attribute(pmdp->firmware_support,inhibit_auto_transition) 
		     && (val2 == PM_SYSTEM_SUSPEND || val2 == PM_SYSTEM_HIBERNATION)) ){
	       pmdp->system_idle_action = val2;
	   }
	   else{
	       pmdp->system_idle_action = PM_NONE;
	   }
       }
       else{
	   if ( (val1 & pmdp->system_support || val1 == PM_NONE) 
	       && !( check_pm_attribute(pmdp->firmware_support,inhibit_auto_transition) 
		     && (val1 == PM_SYSTEM_SUSPEND || val1 == PM_SYSTEM_HIBERNATION)) ){
	       pmdp->system_idle_action = val1;
	       fprintf(stderr,
		       MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		       "sys_idle_action");
	   }
	   else{
	       pmdp->system_idle_action = PM_NONE;
	   }
       }
   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"sys_idle_action");
       errcount++;
   }

   /* transition in case notebook lid is closed     */

   if( check_pm_attribute(pmdp->firmware_support, LID_support)){
       /* if the machine has lid ( like notebook machine ) */
       RDebug0(64,"lid supported. Now read odm data for lid action.\n");

       if(get_pmmi_data("lid_action", &val1, &val2) == 0){
	   if (is_pm_action_state(val2)){
	       if ((val2 & pmdp->system_support)||(val2 == PM_NONE)){
		   pmdp->lid_close_action = val2;
	       }
	       else{
		   pmdp->lid_close_action = PM_NONE;
	       }
	   }
	   else{
	       if ((val1 & pmdp->system_support)||(val1 == PM_NONE)){
		   pmdp->lid_close_action = val1;
		   fprintf(stderr,
			   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
			   "lid_action");
	       }
	       else{
		   pmdp->lid_close_action = PM_NONE;
	       }
	   }
       }	
       else{
	   fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"lid_action");
	   errcount++;
       }	
   }	
   else{
       /* if the machine does NOT have lid ( like desktop machine ) */
       RDebug0(64,"lid not supported. Now set lid action to ignore.\n");
       pmdp->lid_close_action = PM_NONE;
   }

   /* transition in case main power switch is pressed. */
   if(get_pmmi_data("switch_action", &val1, &val2) == 0){
       if (is_pm_action_state(val2)){
	   if ((val2 & pmdp->system_support)||(val2 == PM_NONE)){
	       pmdp->main_switch_action = val2;
	   }
	   else{
	       pmdp->main_switch_action = PM_NONE;
	   }
       }
       else{
	   if ((val1 & pmdp->system_support)||(val1 == PM_NONE)){
	       pmdp->main_switch_action = val1;
	       fprintf(stderr,
		       MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		       "switch_action");
	       pmdp->main_switch_action = val1;
	   }
	   else{
	       pmdp->main_switch_action = PM_NONE;
	   }
       }
   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"switch_action");
       errcount++;
   }


   /* transition in case optional battery is low.   */

   if( check_pm_attribute(pmdp->firmware_support, battery_support)){
       /* if the machine has battery ( like notebook machine ) */
       RDebug0(64,"battery supported. Now read odm data for low battery action.\n");
       if(get_pmmi_data("battery_action", &val1, &val2) == 0){
	   
	   if (is_pm_action_state(val2)){
	       if ((val2 & pmdp->system_support)||(val2 == PM_NONE)){
		   pmdp->low_battery_action = val2;
	       }
	       else{
		   pmdp->low_battery_action = PM_NONE;
	       }
	   }
	   else{
	       if ((val1 & pmdp->system_support)||(val1 == PM_NONE)){
		   pmdp->low_battery_action = val1;
		   fprintf(stderr,
		       MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		       "battery_action");
	       }	
	       else{
		   pmdp->low_battery_action = PM_NONE;
	       }
	   }
       }
       else{
	   fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"battery_action");
	   errcount++;
       }	
   }
   else{
       /* if the machine does NOT have battery ( like desktop machine ) */
       RDebug0(64,"battery not supported. Now set low battery action to ignore.\n");
       pmdp->low_battery_action = PM_NONE;
   }

   /* transition general users can request                 */
   if(get_pmmi_data("permission", &val1, &val2) == 0){

       if (is_pm_permission(val2)){
	   pmdp->permission = val2;
       }
       else{
	   pmdp->permission = val1;
	   fprintf(stderr,
		   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		   "permission");
       }

   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"permission");
       errcount++;
   }

   /* beep setting (on/off)                         */
   if(get_pmmi_data("pm_beep", &val1, &val2) == 0){

       if (is_boolean(val2)){
	   pmdp->pm_beep = val2;
       }
       else{
	   pmdp->pm_beep = val1;
	   fprintf(stderr,
		   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		   "pm_beep");
       }
   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"pm_beep");
       errcount++;
   }

   /* password query in resume (on/off)             */
   if(get_pmmi_data("resume_passwd", &val1, &val2) == 0){

       if (is_boolean(val2)){
	   pmdp->resume_passwd = val2;
       }
       else{
	   pmdp->resume_passwd = val1;
	   fprintf(stderr,
		   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		   "resume_passwd");
       }

   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"resume_passwd");
       errcount++;
   }


   /* LFT termination flag (on/off)                 */
   if(get_pmmi_data("lft_session", &val1, &val2) == 0){

       if (is_boolean(val2)){
	   pmdp->kill_lft_session = val2;
       }
       else{
	   pmdp->kill_lft_session = val1;
	   fprintf(stderr,
		   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		   "lft_session");
       }

   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"lft_session");
       errcount++;
   }


   /* TTY termination flag (on/off)                 */
   if(get_pmmi_data("tty_session", &val1, &val2) == 0){

       if (is_boolean(val2)){
	   pmdp->kill_tty_session = val2;
       }
       else{
	   pmdp->kill_tty_session = val1;
	   fprintf(stderr,
		   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		   "tty_session");
       }

   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"tty_session");
       errcount++;
   }

   
   /* sync daemon termination flag (on/off)         */
   if(get_pmmi_data("kill_syncd", &val1, &val2) == 0){

       if (is_boolean(val2)){
	   pmdp->kill_syncd = val2;
       }
       else{
	   pmdp->kill_syncd = val1;
	   fprintf(stderr,
		   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		   "kill_syncd");
       }
   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"kill_syncd");
       errcount++;
   }


   /* ringing resume (on/off)                       */

   if ( check_pm_attribute(pmdp->firmware_support, 
			  ringing_resume_from_hibernation) ||
        check_pm_attribute(pmdp->firmware_support, 
			  ringing_resume_from_suspend) 
      ){
       /* if the ringing resume feature supported */
       RDebug0(64,"ringing resume supported.\
 Now read odm data for ringing resume setting.\n");

       if(get_pmmi_data("ringing_resume", &val1, &val2) == 0){
	   if (is_boolean(val2)){
	       pmdp->ringing_resume = val2;
	   }
	   else{
	       pmdp->ringing_resume = val1;
	       fprintf(stderr,
		       MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		       "ringing_resume");
	   }
       }	
       else{
	   fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"ringing_resume");
	   errcount++;
       }
   }
   else{
       /* if the ringing resume feature supported */
       RDebug0(64,"ringing resume not supported.\
 Now set ringing resume disable.\n");
       pmdp->ringing_resume = FALSE;
   }

   /* specified time for entering power saving state */
   if(get_pmmi_data("specified_time", &val1, &val2) == 0){

       if (is_specified_time(val2)){
	   pmdp->specified_time = val2;
       }
       else{
	   pmdp->specified_time = val1;
	   fprintf(stderr,
		   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		   "specified_time");
       }

   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"specified_time");
       errcount++;
   }


   /* transition in case specified timer expired    */

   if(get_pmmi_data("specified_act", &val1, &val2) == 0){
       
       if (is_pm_action_state(val2)){
	   if ((val2 & pmdp->system_support||val2 == PM_NONE)
	       && !( check_pm_attribute(pmdp->firmware_support,inhibit_auto_transition) 
		     && (val2 == PM_SYSTEM_SUSPEND || val2 == PM_SYSTEM_HIBERNATION)) ){
	       pmdp->specified_time_action = val2;
	   }
	   else{
	       pmdp->specified_time_action = PM_SYSTEM_SHUTDOWN;
	       pmdp->specified_time = 0;
	   }
       }	 
       else{
	   if ((val1 & pmdp->system_support || val1 == PM_NONE)
	       && !( check_pm_attribute(pmdp->firmware_support,inhibit_auto_transition) 
		     && (val1 == PM_SYSTEM_SUSPEND || val1 == PM_SYSTEM_HIBERNATION)) ){
	       pmdp->specified_time_action = val1;
	       fprintf(stderr,
		       MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		       "specified_act");
	   }
	   else{
	       pmdp->specified_time_action = PM_SYSTEM_SHUTDOWN;
	       pmdp->specified_time = 0;
	   }
       }
   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"specified_act");
       errcount++;
   }

   /* specified time for resume from power saving state */

   if ( (check_pm_attribute(pmdp->firmware_support, 
			  resume_from_hibernation_at_specified_time) ||
         check_pm_attribute(pmdp->firmware_support, 
			  resume_from_suspend_at_specified_time))
         && !(check_pm_attribute(pmdp->firmware_support,
			       inhibit_auto_transition))
      ){
       /* if the specified time resume feature supported */
       RDebug0(64,"resume time supported.\
 Now read odm data for resume time setting.\n");
       if(get_pmmi_data("resume_time", &val1, &val2) == 0){

	   if (is_specified_time(val2)){
	       pmdp->resume_time = val2;
	   }
	   else{
	       pmdp->resume_time = val1;
	       fprintf(stderr,
		       MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		       "resume_time");
	   }
       }
       else{
	   fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"resume_time");
	   errcount++;
       }
   }
   else{
       RDebug0(64,"resume time not supported.\
 Now set resume time to be disabled.\n");
       pmdp->resume_time = 0;
   }

   /* system idle time in seconds. */
   if(get_pmmi_data("sys_idle_time", &val1, &val2) == 0){

       if (is_timer(val2)){
	   pmdp->system_idle_time = val2;
       }
       else{
	   pmdp->system_idle_time = val1;
	   fprintf(stderr,
		   MSGSTR(WARN_ODM_INIT,DFLT_WARN_ODM_INIT),
		   "sys_idle_time");
       }

   }
   else{
       fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"sys_idle_time");
       errcount++;
   }


   /* duration of suspend to hibernation in seconds.*/
   if ( check_pm_attribute(pmdp->firmware_support, 
			   hibernation_from_suspend_at_specified_time)
        && ( (pmdp->system_support & PM_SYSTEM_SUSPEND)!=0 )
        && ( (pmdp->system_support & PM_SYSTEM_HIBERNATION)!=0 )
        && !(check_pm_attribute(pmdp->firmware_support,
				inhibit_auto_transition))
  
      ){
       /* if the duration feature supported */
       RDebug0(64,"duration supported. Now read odm data for duration setting.\n");

       if(get_pmmi_data("sus_to_hiber", &val1, &val2) == 0){
	   pmdp->sus_to_hiber = val2;
       }
       else{
	   fprintf(stderr,MSGSTR(ERR_ODM_READ,DFLT_ERR_ODM_READ),"sus_to_hiber");
	   errcount++;
       }
   }
   else{
       /* if the duration feature not supported */
       RDebug0(64,"duration not supported. Now set duration to be disabled.\n");
	   pmdp->sus_to_hiber = 0;
   }


   odm_terminate();
   
   if (errcount){
       return PM_ERROR;
   }

   return PM_SUCCESS;
}
