static char sccsid[] = "@(#)67  1.5  src/rspc/kernext/pm/pmmd/6030/pmmddata.c, pwrsysx, rspc41J, 9520A_all 5/10/95 13:10:20";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Power Management Kernel Extension Code
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
#include <sys/watchdog.h>
#include <sys/rtc.h>
#include <sys/pm.h>

#include "../../pmmi/pmmi.h"
#include "pm_6030.h"
#include "planar.h"
#include "slave.h"

struct _pm_md_data pm_md_data = {{0}, {0}, 0, 0, {0}, 0, 0, {0}, {0}};
struct watchdog mywatchdog;
struct tms RTC_alarm_resume_time; 
pm_HW_state_data_t pm_HW_state_data = {{0},PM_LID_OPEN,in_AC,PM_PLANAR_OFF};
pmmd_control_data_t pmmd_control_data = {0,0};
