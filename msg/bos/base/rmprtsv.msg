$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)93	1.1  src/msg/bos/base/rmprtsv.msg, bosmsg, msg411, GOLD410 2/9/94 09:14:54
$
$ messages for rmprtsv   
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
$ The range of message numbers assigned to this file are 196-206 
$quote "         define quote character.
$len 
$set MS_RMPRTSV        Define initial set#

$ Do not translate rmprtsv, %c, %s, etc.. 
INCOMP_SW            "rmprtsv: 0822-196 The specified flags are not compatible.\n"
UNK_SWITCH           "rmprtsv: 0822-197 %c is not a valid flag. \n"
NOT_PARSED           "rmprtsv: 0822-198 A flag was passed with an option that is not expected.\n"
SPEC_SW              "rmprtsv: 0822-199 Specify either the -c flag or the -s flag. \n"
SPEC_SW2             "rmprtsv: 0822-200 Specify the -q and -v flags.\n"
ERR_EXEC             "%1$s: 0822-201 Cannot run %2$s. \n"
USAGE1               "\n\
Usage: %s [-c|-s] [-A|-T|-U] [-h Host...|-H File]\n"
USAGE2               "\t[-q qentry...|-q qentry -v Device]\n"
