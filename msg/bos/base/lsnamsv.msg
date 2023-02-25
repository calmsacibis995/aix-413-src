$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)51	1.1  src/msg/bos/base/lsnamsv.msg, bosmsg, msg411, GOLD410 2/9/94 09:06:47
$ 
$ messages for lsnamsv   
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
$ The range of message numbers assigned to this file are 121-131 
$quote "         define quote character.
$len 
$set MS_LSNAMSV        Define initial set#

INCOMP_SW            "lsnamsv: 0822-121 The specified flags are not compatible.\n"
UNK_SWITCH           "lsnamsv: 0822-122 %c is not a valid flag.\n"
NOT_PARSED           "lsnamsv: 0822-123 A flag was passed with an option that is not expected.\n"
SPEC_SW              "lsnamsv: 0822-124 Specify either the -C flag or the -S flag.\n"
$ Do not translate -C, -S 
BAD_ATTR             "lsnamsv: 0822-125 The attribute %s is not valid.\n"
ERR_EXEC             "%1$s: 0822-126 Cannot run %2$s.\n"
USAGE                "\n\
Usage: %s {-C|-S} Attribute=Value... [-Z]\n"
