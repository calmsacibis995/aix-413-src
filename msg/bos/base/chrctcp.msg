$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)40	1.1  src/msg/bos/base/chrctcp.msg, bosmsg, msg411, GOLD410 2/9/94 08:49:17
$ 
$ messages for chrctcp   
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
$ The range of message numbers assigned to this file are 301-313
$quote "         define quote character.
$len 
$set MS_CHRCTCP        Define initial set#

INCOMP_SW            "chrctcp: 0822-301 The specified flags are not compatible.\n"
UNK_SWITCH           "chrctcp: 0822-302 %c is not a valid flag.\n"
NOT_PARSED           "chrctcp: 0822-303 A flag was passed with an option that is not expected.\n"
SPEC_SW              "chrctcp: 0822-304 Specify either the -a, -d, or -s flag.\n"
ERR_OPEN2            "%s: 0822-305 Cannot open a temporary file.\n"
ERR_RENAME           "%1$s: 0822-306 Cannot move the temporary file back to %2$s.\n"
ERRACC               "%1$s: 0822-307 Cannot change the access mode of file %2$s.\n"
ERR_STAT             "%1$s: 0822-308 %2$s does not exist.\n"
ERR_EXEC             "chrctcp: 0822-309 Cannot run %s.\n"
BAD_ATTR             "chrctcp: 0822-310 The attribute %s is not valid.\n"
BAD_ROUTE            "chrctcp: 0822-311 Suppress routing and Supply routing options incompatible.\n"
BAD_GATED            "chrctcp: 0822-312 LOG all ERROR... option must also be yes.\n"));
BAD_TIMED            "chrctcp: 0822-313 The Valid and Ignore Network options are incompatible.\n"));

USAGE                "Usage: %s [-S] {-c Daemon [-f \"Attribute=Value...\"]} | {-a|-d|-s} Daemon\n"
