$ @(#)27	1.2  src/tcpip/usr/sbin/lsprtsv/lsprtsv.msg, tcpip, tcpip411, GOLD410 2/9/90 09:46:04
$ 
$ messages for lsprtsv    
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
$ The range of message numbers assigned to this file are 132-141 
$quote "         define quote character.
$len 
$set MS_LSPRTSV        Define initial set#

INCOMP_SW            "lsprtsv: 0822-132 The specified flags are not compatible.\n"
UNK_SWITCH           "lsprtsv: 0822-133 %c is not a valid flag.\n"
NOT_PARSED           "lsprtsv: 0822-134 A flag was passed with an option that is not expected.\n"
SPEC_SW              "lsprtsv: 0822-135 Specify either the -c flag or the -p flag.\n"
$ DO not translate -c, -p 
ERR_EXEC             "%1$s: 0822-136 Cannot run %2$s.\n"
USAGE                "\n\
Usage: %s -c|-p [-h] [-q qentry...] [-Z]\n"
