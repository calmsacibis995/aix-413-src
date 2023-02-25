$ @(#)94  1.12  src/rspc/usr/bin/pmctrl/pmctrl.msg, pwrcmd, rspc41J, 9520B_all  5/18/95  12:42:18
$
$   COMPONENT_NAME: PWRCMD
$
$   FUNCTIONS: Power Management Command
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$     ****************************************
$     **                                    **
$     ** Message Catalog for pmctrl command **
$     **                                    **
$     ****************************************
$
$
$     NOTE:
$       If message lies over more than 2 lines, please put space or TAB
$       at the beginning of other lines than the first line.
$
$
$len
$quote "
$
$	Message numbers 0381-001 - 0381-050 and
$	                0381-201 - 0381-300 are reserved for this file.
$
$set MS_PMCTRL
$
$      ********************************
$      *                              *
$      * Message Catalog for PHASE 1  *
$      *                              *
$      ********************************
$
$ list_pm_dds()
MSG1 "No PM aware DD is registered.\n"
MSG2 "List of registered PM aware DDs is as follows:\n"
MSG3  "%1$s\n"
$ query_device_idle_time()
MSG4  "Device idle time for %1$s is %2$d (sec.).\n"
MSG5  "Device standby time for %1$s is %2$d (sec.).\n"
$ query_system_idle_time()
MSG6  "System idle time is %1$d (sec.).\n"
$ query_system_state()
MSG7  "Current system state is %1$s.\n"
$ query_beep()
MSG8  "Beep is %1$s.\n"
$ print_action()
MSG9  "Action for %1$s is %2$s.\n"
$ query_resume_passwd()
MSG10  "Password check at resume time is %1$s.\n"
$ query_lft_termination()
MSG11  "LFT session termination is %1$s.\n"
$ Just keyword
MSG12  "%1$s\n"
$
$ main()
ERRMSG1   "pmctrl: 0381-001 Invalid option %1$s.\n"
$ list_pm_dds()
ERRMSG2   "pmctrl: 0381-002 Failed to query the number of PM aware DDs.\n"
ERRMSG3   "pmctrl: 0381-003 Failed to allocate memory.\n"
ERRMSG4   "pmctrl: 0381-004 Failed to query the list of PM aware DDs.\n"
$ set_lid_close_action(),set_system_idle_action()
$ set_main_switch_action(),set_low_battery_action()
ERRMSG5   "pmctrl: 0381-005 Invalid duration %1$s was specified.\n"
ERRMSG6   "pmctrl: 0381-006 Invalid action %1$s was specified for %2$s.\n"
ERRMSG7   "pmctrl: 0381-007 Invalid state %1$s was specified.\n"
ERRMSG8   "pmctrl: 0381-008 Failed to set action for %1$s.\n"
ERRMSG9   "pmctrl: 0381-009 Failed to get attribute name.\n"
ERRMSG10   "pmctrl: 0381-010 Failed to update data in ODM.\n"
$ query_lid_close_action(),query_system_idle_action()
$ query_main_switch_action(),query_low_battery_action()
ERRMSG11   "pmctrl: 0381-011 Failed to query action for %1$s.\n"
$ set_odm_action()
ERRMSG12   "pmctrl: 0381-012 Error in %1$s.\n"
ERRMSG13   "pmctrl: 0381-013 Failed to set value %1$s for attribute %2$s.\n"
$ set_device_idle_time()
ERRMSG14   "pmctrl: 0381-014 Failed to get devno from ODM %1$s.\n"
ERRMSG15   "pmctrl: 0381-015 Failed to set device idle/standby time for %1$s.\n"
$ Do not translate "PdDvLn".
ERRMSG16   "pmctrl: 0381-016 Failed to get PdDvLn for %1$s from ODM.\n"
$ get_cudv_pddvln()
ERRMSG17   "pmctrl: 0381-017 Error in %1$s with criteria: %2$s.\n"
$ query_device_idle_time()
ERRMSG18   "pmctrl: 0381-018 Failed to query device idle/standby time for\
 %1$s.\n"
$ set_system_idle_time()
ERRMSG19   "pmctrl: 0381-019 Invalid system idle time %1$s.\n"
ERRMSG20   "pmctrl: 0381-020 Failed to set system idle timer to %1$s.\n"
$ query_system_idle_time()
ERRMSG21   "pmctrl: 0381-021 Failed to query system idle timer.\n"
$ start_action()
ERRMSG22   "pmctrl: 0381-022 Failed to request system state transition to\
 %1$s.\n"
$ query_system_state()
ERRMSG23   "pmctrl: 0381-023 Failed to query system state.\n"
ERRMSG24   "pmctrl: 0381-024 Invalid value %1$d for system state returned.\n"
$ query_beep()
ERRMSG25   "pmctrl: 0381-025 Failed to query whether beep is enabled or not.\n"
ERRMSG26   "pmctrl: 0381-026 Invalid value %1$d to determine whether beep is\
 enabled or not is returned.\n"
$ set_beep()
ERRMSG27   "pmctrl: 0381-027 Invalid value %1$s specified for beep.\n"
ERRMSG28   "pmctrl: 0381-028 Failed to set beep flag to %1$s.\n"
$ print_action()
ERRMSG29   "pmctrl: 0381-029 Invalid value for action specified.\n"
$ query_resume_passwd()
ERRMSG30   "pmctrl: 0381-030 Failed to get value for attribute %1$s.\n"
ERRMSG31   "pmctrl: 0381-031 Invalid value %1$s for %2$s.\n"
$ set_resume_passwd()
ERRMSG32   "pmctrl: 0381-032 Invalid value %1$s for password check at resume\
 time.\n"
$ set_lft_termination()
ERRMSG33   "pmctrl: 0381-033 Invalid value %1$s for LFT session termination.\n"
$ get_attribute(),set_attribute()
$ Do not translate "uniquetype =" and "attribute =".
ERRMSG34   "pmctrl: 0381-034 Error in %1$s with uniquetype = %2$s, attribute =\
 %3$s\n"
$ usage()
ERRMSG35 "Usage: %1$s [options]\n"
ERRMSG36 "options:\n"
ERRMSG37 "[-d [/dev/device_name]] [-t [device_idle_time device_standby_time]]\n"
ERRMSG38 "[-c | -t [system_idle_time] | -e | -p | -l] [-a [action]]\n"
ERRMSG39 "[-b | -w | -k] [on|off]\n"
ERRMSG40 "[-v]\n"
$
$      ********************************
$      *                              *
$      * Message Catalog for PHASE 2  *
$      *                              *
$      ********************************
$
$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$    MESSAGE for  USAGE ( added for phase 2 )   
$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$
$
USAGE1	"\
 Usage: pmctrl -h (shows long usage)\n\
        pmctrl {-v|-d|-a|-e -a <state>}\n\
        pmctrl  -d <logical_name> [-t [<time arguments>]]\n\
        pmctrl {-l|-p|-c|-x|-u} [-a [<state>]]\n\
        pmctrl {-b|-w|-r|-k|-y|-s} [on|off] | {-t|-g} [<time>] |\n\
               -S [<[date]time> suspend|hibernation|shutdown] |\n\
               -R [<[date]time>]\n\
 Note:  Power Management parameter can be changed only with root authority.\n\n"
$
USAGE2	"\
     -v            displays all PM information.\n\
     -e -a <state> requests transition to the specified state.\n\
     -a [<state>]  displays the current system state without any argument.\n\
                   specifies the system state in combination with other\n\
	           flags and with the argument <state>.\n\
                   The argument <state> is one of the following items.\n\
                      full_on, enable, standby, suspend, hibernation,\n\
                      shutdown, ignore.\n\n\
     -d            displays the list of all the pm aware device drivers\n\
		   without any argument.\n\
     -d <logical_name> or -d <logical_name> -t \n\
                   displays <idle_time> and <standby_time>\n\
                   of the specified device by <logical_name>.\n\
     -d lft0 or -d lft0 -t\n\
                   displays <dim_time>, <suspend_time>, and <turnoff_time>\n\
		   of the Display Power Management System (DPMS).\n\
                   lft0 as <logical_name> means DPMS.\n\
     -d <logical_name> -t <idle_time> <standby_time>\n\
                   sets the <idle_time>, <standby_time> of the specified\n\
		   device by <logical_name>.\n\
     -d lft0 -t <dim_time> <suspend_time> <turnoff_time>\n\
                   sets the <dim_time>, <suspend_time>, and <turnoff_time>\n\
		   of the Display Power Management System (DPMS).\n\n\
                   Specifying 0  as time arguments means disable.\n\
                   Specifying -1 as time arguments means no change.\n\n"
$
USAGE3	"\
 Following flags without the argument [on|off] only show the current setting.\n\
     -b [on|off]   Beep when the system state transition completes.\n\
     -w [on|off]   Password query in resume from suspend or hibernation\n\
		   when X server is running.\n\
     -r [on|off]   Ringing resume. Ring of the telephone causes the system\n\
		   to resume from suspend or hibernation.\n\
     -k [on|off]   LFT session termination in suspend or hibernation enter.\n\
	           When set to on, LFT session will be terminated when the\n\
                   system enters suspend or hibernation state.\n\
                   When set to off, LFT session will NOT be terminated.\n\
     -y [on|off]   TTY session termination in suspend or hibernation enter.\n\
                   When set to on, TTY session will be terminated when the\n\
		   system enters suspend or hibernation state.\n\
                   When set to off, LFT session will NOT be terminated.\n\
     -s [on|off]   Sync daemon termination during standby.\n\
		   When set to on, the sync daemon will be terminated,\n\
	           if any, when the system enters standby state.\n\
		   In this case, the sync daemon will be invoked again\n\
		   after resuming from standby state.\n\
	           When set to off, the sync daemon will NOT be terminated.\n\n"
$		
USAGE4	"\
 Following flags without the argument [<state>] only show the current\
 setting.\n\
 The argument <state> is one of the following items.\n\n\
        full_on, enable, standby, suspend, hibernation, shutdown, ignore.\n\n\
     -l [-a [<state>]] sets the system state transition initiated when the\n\
		       remaining capacity of optional battery is low.\n\
     -p [-a [<state>]] sets the system state transition initiated when the\n\
		       main power switch is pressed.\n\
     -c [-a [<state>]] sets the system state transition initiated when the\n\
		       notebook-lid is closed.\n\
     -x [-a [<state>]] sets the system state transition initiated when the\n\
		       system idle timer expired.\n\
     -u [-a [<state>]] sets the maximum state authorized for general users.\n\
                       When this is set to suspend with root authority,\n\
		       general users cannot request for the transition to\n\
		       shutdown or hibernation because the maximum state\n\
		       permitted to the general users is suspend.\n\
		       When this is set to ignore with root authority,\n\
		       general users cannot request for any system state\n\
		       transition.\n\
		       The order of the system states is as follows.\n\
                            shutdown > hibernation > suspend > standby >\n\
			    ( enable, full_on ) > ignore.\n\n"
$
USAGE5	"\
 Following flags without the argument [<time>] only show the current setting.\n\
 Specifying 0 as [<time>] argument means disable.\n\
 Specifying 0000 as [<time>] argument means midnight. (for -S and -R flags)\n\
 The range of [<time>] value is from 0 to 120 (minutes). (for -t and -g flags)\n\n\
     -S [<[date]time> suspend | hibernation | shutdown ]\n\
                   sets the time for entering power saving state which is \n\
                   one of the suspend, hibernation, and shutdown. \n\
                   <[date]time> is specified with the format [YYMMDD]hhmm.\n\
                   Only hhmm means everyday function.\n\
     -R [<[date]time>]\n\
                   sets the time for resume.\n\
                   <[date]time> is specified with the format [YYMMDD]hhmm.\n\
                   Only hhmm means everyday function. \n\
     -t [<time>]   sets system idle timer in minutes.\n\
		   Once this is set, this timer begins to tick while the\n\
		   system is left idle. When the timer expires, the system\n\
		   changes to the state specified by the pmctrl -xa command.\n\
     -g [<time>]   sets duration of suspend to hibernation in minutes.\n\
                   Once this is set, after the system has changed its state\n\
	 	   to suspend, this timer begins to tick.\n\
 		   When the timer expires, the system goes to hibernation state.\n\n"
$
$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$     ERROR MESSAGE ( added for phase2 ) 201-    
$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$
ERR_ODM_INIT	"pmctrl: 0381-201 Failed to initialize ODM data.\n"	
ERR_ODM_CHANGE	"pmctrl: 0381-202 Failed to change ODM data.\n"	
ERR_TWICE	"pmctrl: 0381-203 The same flag can not be specified\
 at once.\n"
ERR_ALLINFO	"pmctrl: 0381-204 Failed to show all the parameters for\
 Power Management.\n"
ERR_PARSE	"pmctrl: 0381-205 Invalid usage for %1$s flag.\n"
ERR_Q_DUR	"pmctrl: 0381-206 Failed to query duration.\n"
ERR_S_DUR	"pmctrl: 0381-207 Failed to set duration to %1$s.\n"
ERR_Q_ON_OFF	"pmctrl: 0381-208 Failed to query on/off status for %1$s.\n"
ERR_S_ON_OFF	"pmctrl: 0381-209 Failed to set %1$s to %2$s.\n"
ERR_Q_ALARM	"pmctrl: 0381-210 Failed to query alarm setting for %1$s.\n"
ERR_S_ALARM	"pmctrl: 0381-211 Failed to set alarm for %1$s.\n"
ERR_BUSY	"pmctrl: 0381-212 System is busy processing request\
 for state transition.\n"
ERR_BAD_TIME	"pmctrl: 0381-213 Invalid time value %1$s\n"
ERR_MISS_OPRND 	"pmctrl: 0381-214 Missing operand for %1$s flag.\n"
ERR_INSUF_PERM 	"pmctrl: 0381-215 Specified state transition is not\
 authorized for general users.\n"
ERR_NO_CHANGE 	"pmctrl: 0381-216 Specified device has the fixed standby/idle\
 time.\n"
ERR_Q_PERM 	"pmctrl: 0381-217 Failed to query authorized state\
 for general users.\n"
ERR_S_PERM 	"pmctrl: 0381-218 Failed to set authorized state\
 for general users.\n"
ERR_INVAL 	"pmctrl: 0381-219 Invalid state transition specified.\n"
ERR_NOSYS_STATE	"pmctrl: 0381-220 System does not support the specified\
 state transition.\n"
ERR_NOSYS_PARAM	"pmctrl: 0381-221 System does not allow changing the\
 specified parameter.\n"
ERR_ALREADY 	"pmctrl: 0381-222 Already in the specified state.\n"
ERR_ACCES 	"pmctrl: 0381-223 System does not permit the specified\
 state transition.\n"
ERR_ITIME 	"pmctrl: 0381-224 Specified device has a fixed\
 idle time or specified value is out of range.\n"
ERR_STIME 	"pmctrl: 0381-225 Specified device has a fixed\
 standby time or specified value is out of range.\n"
$
$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$  WARNING for PARAMETERS ( added for phase2 ) 250- 
$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$
WARN_ODM_CHANGE "pmctrl: 0381-250 Warning: ODM data has not been modified.\n"
WARN_ODM_ITIME  "pmctrl: 0381-251 Warning: device idle time has not been\
 modified in ODM.\n"
WARN_ODM_STIME  "pmctrl: 0381-252 Warning: device standby time has not been\
 modified in ODM.\n"
$
$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$  MESSAGE for PARAMETERS ( added for phase2 )   
$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$
DPMS		"DPMS timer is as follows. (in minutes).\n\
 \t Dim Time = %d,  Suspend Time = %d,  Off Time = %d.\n"
DPMS_DISABLE    "DPMS timer is disabled.\n"
DEV_TIME	"Timer setting for %1$s is as follows. (in minutes).\n\
 \t Idle Time = %2$d,  Standby Time = %3$d.\n"
ON_OFF		"%1$s setting is set to %2$s.\n"
DUR_T		"%1$s is set to %2$s.\n"
PERMISSION      "Maximum state authorized for general users is set to %1$s.\n"   
ACTION          "Action for %1$s is transition to %2$s.\n"   
ACTION_TIME	"Timer for %1$s is set to %2$s.\n"
NO_PM_DD	"There are no PM aware DDs.\n"
$
$set 2
1 "Power Management Controller"






