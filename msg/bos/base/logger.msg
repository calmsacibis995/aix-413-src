$ ENGLISH_SOURCE_VERSION= 1.9
$ @(#)33	1.1  src/msg/bos/base/logger.msg, bosmsg, msg411, GOLD410 2/9/94 09:05:57
$
$ COMPONENT_NAME: (CMDMISC) miscellaneous commands
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
$ messages for logger  
$quote "         define quote character.

$len 
$set MS_LOGGER        Define initial set#
$ Message numbers have been assigned a range of 006-013
$ Do not translate logger, %d, %s, %1$d, %1$s, etc ...
$
$       Do not remove the space after the colon. 
$
UNKFACILITY	"0481-007 The specified facility name is not known: "
$
UNKPRIORITY	"0481-008 The specified priority name is not known: "
$
USAGE		"Usage: logger [-f File] [-i] [-p Priority] [-t Tag] [Message]\n"
