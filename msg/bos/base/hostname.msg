$ ENGLISH_SOURCE_VERSION= 1.2
$quote "         define quote character.

$ @(#)68	1.1  src/msg/bos/base/hostname.msg, bosmsg, msg411, GOLD410 2/9/94 09:02:25
$    
$ COMPONENT_NAME: CMDNET hostname.msg   
$    
$ FUNCTIONS:    
$ ORIGINS: 10  26  27    
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989   
$ All Rights Reserved   
$ Licensed Materials - Property of IBM   
$ US Government Users Restricted Rights - Use, duplication or   
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.   
$ messages for hostname   
$ The range of message numbers assigned to this file are 266-273 
$len 
$set MS_HOSTNAME        Define initial set#

AUD_SYS              "hostname: 0821-266 The routine auditproc failed.  The error is %m.\n"
$  This message is printed when there is an error in the audit routine.  
USAGE                "Usage: %s [-s] [Host].\n"
SETHOST              "hostname: 0821-267 The sethostname system call failed.\n"
GETHOSTNAME          "hostname: 0821-268 The gethostname system call failed.\n"
