$ @(#)19	1.4  src/tcpip/usr/sbin/mknamsv/mknamsv.msg, tcpnaming, tcpip411, GOLD410 2/9/90 09:52:03
$ 
$ messages for mknamsv   
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
$ The range of message numbers assigned to this file are 142-152 
$quote "         define quote character.
$len 
$set MS_MKNAMSV        Define initial set#

INCOMP_SW            "mknamsv: 0822-142 The specified flags are not compatible.\n"
UNK_SWITCH           "mknamsv: 0822-143 %c is not a valid flag.\n"
NOT_PARSED           "mknamsv: 0822-144 A flag was passed with an option that is not expected.\n"
SPEC_SW2             "mknamsv: 0822-145 Specify either the -a flag or the -A flag.\n"
$ Do not translate -a, -A 
BAD_ATTR             "mknamsv: 0822-146 The attribute %s is not valid.\n"
ERR_EXEC             "%1$s: 0822-147 Cannot run %2$s.\n"
USAGE                "\n\
Usage: %s -a Attribute=Value | -A File\n"
