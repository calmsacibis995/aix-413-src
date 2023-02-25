$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)98	1.2  src/msg/bos/base/battery.msg, bosmsg, msg411, 9439A411a 9/26/94 15:18:48
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
$	Message Catalog for battery command
$
$len
$quote "
$
$       Message numbers 0381-050 - 0381-060 are reserved for this file.
$
$set MS_BATTERY
$
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
