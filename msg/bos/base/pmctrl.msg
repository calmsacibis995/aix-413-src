$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)00	1.2  src/msg/bos/base/pmctrl.msg, bosmsg, msg411, 9439A411a 9/26/94 15:31:53
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
$
$	Message Catalog for pmctrl command
$
$len
$quote "
$
$	Message numbers 0381-001 - 0381-050 are reserved for this file.
$
$set MS_PMCTRL
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
ERRMSG18   "pmctrl: 0381-018 Failed to query device idle/standby time for %1$s.\n"
$ set_system_idle_time()
ERRMSG19   "pmctrl: 0381-019 Invalid system idle time %1$s.\n"
ERRMSG20   "pmctrl: 0381-020 Failed to set system idle time %1$s.\n"
$ query_system_idle_time()
ERRMSG21   "pmctrl: 0381-021 Failed to query system idle time.\n"
$ start_action()
ERRMSG22   "pmctrl: 0381-022 Failed to request system state transition to %1$s.\n"
$ query_system_state()
ERRMSG23   "pmctrl: 0381-023 Failed to query system state.\n"
ERRMSG24   "pmctrl: 0381-024 Invalid value %1$d for system state returned.\n"
$ query_beep()
ERRMSG25   "pmctrl: 0381-025 Failed to query whether beep is enabled or not.\n"
ERRMSG26   "pmctrl: 0381-026 Invalid value %1$d to determine whether beep is enabled or not is returned.\n"
$ set_beep()
ERRMSG27   "pmctrl: 0381-027 Invalid value %1$s specified for beep.\n"
ERRMSG28   "pmctrl: 0381-028 Failed to set beep flag to %1$s.\n"
$ print_action()
ERRMSG29   "pmctrl: 0381-029 Invalid value for action specified.\n"
$ query_resume_passwd()
ERRMSG30   "pmctrl: 0381-030 Failed to get value for attribute %1$s.\n"
ERRMSG31   "pmctrl: 0381-031 Invalid value %1$s for %2$s.\n"
$ set_resume_passwd()
ERRMSG32   "pmctrl: 0381-032 Invalid value %1$s for password check at resume time.\n"
$ set_lft_termination()
ERRMSG33   "pmctrl: 0381-033 Invalid value %1$s for LFT session termination.\n"
$ get_attribute(),set_attribute()
$ Do not translate "uniqutype =" and "attribute =".
ERRMSG34   "pmctrl: 0381-034 Error in %1$s with uniqutype = %2$s, attribute = %3$s\n"
$ usage()
ERRMSG35 "Usage: %1$s [options]\n"
ERRMSG36 "options:\n"
ERRMSG37 "[-d [/dev/device_name]] [-t [device_idle_time device_standby_time]]\n"
ERRMSG38 "[-c | -t [system_idle_time] | -e | -p | -l] [-a [action]]\n"
ERRMSG39 "[-b | -w | -k] [on|off]\n"
ERRMSG40 "[-v]\n"
$
$
$set 2
1 "Power Management Controller"
