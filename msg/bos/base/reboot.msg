$ ENGLISH_SOURCE_VERSION= 1.4.2.2
$ @(#)75	1.1  src/msg/bos/base/reboot.msg, bosmsg, msg411, GOLD410 2/9/94 09:13:41
$
$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs  
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ messages for reboot  
$quote "         define quote character.

$len 
$set MS_REBOOT        Define initial set#

$ Do not translate reboot, %s, %d, %1$s, %1$d, etc ... 

USAGE                "Usage: reboot [-l][-n][-q]\n"
$ NOT USED

LOGIT                "Rebooted by %s.\n"
$ Do not translate Rebooted. 

CAUTION              "Caution: Some process or processes would not terminate.\n"

REBOOT               "Rebooting . . .\n"

PERROR               "reboot"
$ Do not translate reboot 

REBOOT1              "Automatic power on scheduled for %s %s\n"

USAGE1               "Enter a date to power on that is later than %s\n"

USAGE2               "Usage: reboot [-l][-n][-q][-t mmddHHMM[yy]]\n"

USAGE3               "Enter a valid date where:\n\
mm is a 2-digit number for month between 01 and 12\n\
dd is a 2-digit number for day between 01 and 31\n\
HH is a 2-digit number for hour between 00 and 23\n\
MM is a 2-digit number for minute between 00 and 59\n\
yy is an optional 2-digit number for year between 1970-1999 or 2000-2038\n"

