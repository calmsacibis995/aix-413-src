static char sccsid[] = "@(#)06  1.7  src/rspc/kernext/pm/pmmi/pmmidata.c, pwrsysx, rspc41J, 9520B_all 5/19/95 07:39:52";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: PM kernel extension global data area which is pinned. 
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

#include <sys/sleep.h>
#include <sys/pm.h>

#include "pmmi.h"


/*------------------------*/
/* Global data symbols    */
/*------------------------*/
pm_parameters_t pm_parameters; 	/* Sent by PMD when PMD is loaded. */
event_query_control_t 	event_query_control= {{0},EVENT_NULL,0,0,0,0,0,0,{0}};
pmmd_IF_t 		pmmd_IF = {{0},NULL};
pm_hibernation_t 	pm_hibernation = {0,0,0,0,0};
pm_sector_t 		pm_sector = {0,0};
pm_control_data_t 	pm_control_data = {FALSE,PM_SYSTEM_FULL_ON,0,0,
					   NULL,NULL,NULL,NULL,0,
					   PM_EVENT_NONE,0,0,0,0,0,0,0,0,0,0};
pm_battery_data_t 	battery_status = {PM_AC+PM_BATTERY, 0,0,0,0,0};
transit_lock_t		transit_lock = {{0},FALSE};
pm_machine_profile_t	pm_machine_profile;
char			LastRejectDD[0x10];
battery_status_ready_t 	battery_status_ready = {EVENT_NULL,FALSE};

