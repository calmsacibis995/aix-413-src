$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)92	1.1  src/msg/bos/base/rmnamsv.msg, bosmsg, msg411, GOLD410 2/9/94 09:14:49
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
$ messages for rmnamsv  
$ The range of message numbers assigned to this file are 187-195 
$quote "         define quote character.
$len 
$set MS_RMNAMSV        Define initial set#

INCOMP_SW            "rmnamsv: 0822-187 The specified flags are not compatible.\n"
UNK_SWITCH           "rmnamsv: 0822-188 %c is not a valid flag.\n"
NOT_PARSED           "rmnamsv: 0822-189 A flag was passed with an option that is not expected.\n"
ERR_EXEC             "%1$s: 0822-190 Cannot run %2$s.\n"
USAGE                "\n\
Usage: %s -f|-F File.\n"
