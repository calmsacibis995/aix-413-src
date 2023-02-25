$ @(#)91  1.6  src/rspc/usr/bin/battery/battery.msg, pwrcmd, rspc41J, 9520A_all  5/15/95  14:44:55
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
$   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
$   All Rights Reserved
$
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$     *****************************************
$     **                                     **
$     ** Message Catalog for battery command **
$     **                                     **
$     *****************************************
$
$     NOTE:
$       If message lies over more than 2 lines, please put space or TAB
$       at the beginning of other lines than the first line.
$
$
$
$len
$quote "
$
$       Message numbers 0381-050 - 0381-060 are reserved for this file.
$
$set MS_BATTERY
$
$
$
$      ********************************
$      *                              *
$      * Message Catalog for PHASE 1  *
$      *                              *
$      ********************************
$
MSG1 "battery type: NiCd.\n"
MSG2 "battery type: NiMH.\n"
$
MSG3  "current battery usage: charging.\n"
MSG4  "current battery usage: discharging.\n"
MSG5  "current battery usage: in use.\n"
$
MSG6  "battery capacity: %1$d (mAH).\n"
$
MSG7  "current remaining capacity: %1$d (mAH).\n"
$
MSG8  "remaining capacity while discharging: %1$d (mAH).\n"
$
MSG9  "discharging time: %1$d (sec.).\n"
$
MSG10  "full charge count: %1$d.\n"
$
ERRMSG1   "Usage: battery [-d]\n"
$
ERRMSG2   "battery: 0381-051 Battery is not supported.\n"
ERRMSG3   "battery: 0381-052 Battery information query failed.\n"
$
ERRMSG4   "battery: 0381-053 discharge failed.\n"
ERRMSG5   "battery: 0381-054 PM daemon is not running.\n"       
$
$      ********************************
$      *                              *
$      * Message Catalog for PHASE 2  *
$      *                              *
$      ********************************
$
ERR_NO_BATTERY  "battery: 0381-055 Battery does not exist.\n"
ERR_SAME_FLAG   "battery: 0381-056 The same flag cannot be specified\
 at once.\n"
ERR_INVAL_OPT   "battery: 0381-057 Invalid option %1$s.\n"
UNKNOWN_STATUS  "battery: 0381-058 Something is wrong with the battery. Please check\
 the battery.\n"
ERR_BUSY	"battery: 0381-059 System is busy processing request\
 for state transition.\n"
FULL_CHARGE     "current battery status: fully charged.\n"
DISCHARGE       "discharge started.\n"
REMAIN_CAPACITY "current remaining capacity: %1$d (mAH) [ %2$d %% ] \n"
DISC_QUANT      "discharge quantity : %1$d (mAH) [ %2$d %% ] \n"




