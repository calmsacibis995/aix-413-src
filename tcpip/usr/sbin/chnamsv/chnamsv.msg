$ @(#)15	1.3  src/tcpip/usr/sbin/chnamsv/chnamsv.msg, tcpnaming, tcpip411, GOLD410 2/9/90 09:51:04
$
$ messages for chnamsv  
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
$ combined with the aggregated modules for this product) 
$                  SOURCE MATERIALS 
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989  
$ All Rights Reserved 
$
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$
$ The range of message numbers assigned to this file are 001-011 
$len 
$quote "         define quote character.
$set MS_CHNAMSV        Define initial set#

$ Do not translate %s, %1$s, %2$s, -a, -A 
$ Do not translate chnamsv. 
INCOMP_SW            "chnamsv: 0822-001 The specified flags are not compatible.\n"
UNK_SWITCH           "chnamsv: 0822-002 %c is not a valid flag.\n"
NOT_PARSED           "chnamsv: 0822-003 A flag was passed with an option that is not expected.\n"
SPEC_SW2             "chnamsv: 0822-004 Specify either the -a flag or the -A flag.\n"
BAD_ATTR             "chnamsv: 0822-005 %s is not a valid attribute.\n"
ERR_EXEC             "%1$s: 0822-006 Cannot run %2$s.\n"
$ Do not move %1$s:  This is the program name. 
USAGE                "\n\
Usage: %s [-a Attribute Value | -A File ].\n"
