$ @(#)37	1.6  src/tcpip/usr/bin/ruser/ruser.msg, tcprcmds, tcpip411, GOLD410 2/9/90 09:55:38
$
$ COMPONENT_NAME:  TCPIP 
$
$ ORIGINS: 27 
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
$ combined with the aggregated modules for this product) 
$                  SOURCE MATERIALS 
$ (C) COPYRIGHT International Business Machines Corp. 1989. 
$ All Rights Reserved 
$
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$                                                                    
$ messages for ruser 
$ The range of message numbers assigned to this file are 207-216 
$                                                                    
$quote "         define quote character.
$len 
$set MS_RUSER        Define initial set#

$ Do not translate %c, %s, %1$, %2$.        
$ Do not translate ruser: 
INCOMP_SW            "ruser: 0822-207 The specified flags are not compatible.\n"
UNK_SWITCH           "ruser: 0822-208 %c is not a valid flag.\n"
NOT_PARSED           "ruser: 0822-209 A flag was passed with an option that is not expected.\n"
MISSINGFLAGS         "ruser: 0822-210 One or more flags are missing."
ERRDEL               "%1$s: 0822-211 Cannot delete from %2$s.\n"
USAGE1               "\n\
Usage: %s  -a  {-f User ...|-p Host...|-r Host...}\n"
USAGE2               "\t%s  -d  {-f User...|-p Host...|-r Host...}\n"
USAGE3               "\t%s  -X  {-F|-P|-R}\n"
USAGE4               "\t%s  -s  {-F|-P|-R} [-Z]\n"
