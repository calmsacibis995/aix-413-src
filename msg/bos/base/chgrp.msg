$ ENGLISH_SOURCE_VERSION= 1.7.2.2
$ @(#)33	1.1  src/msg/bos/base/chgrp.msg, bosmsg, msg411, GOLD410 2/9/94 08:48:55
$
$ COMPONENT_NAME: (CMDSDAC) security: access control
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989,1993  
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$quote "         define quote character.

$len 
$set MS_chgrp        Define initial set#
$ Message numbers have been assigned a range of 001-005
$ Do not translate chgrp, %d, %s, %1$d, %1$s, etc ... 

M_MSG_0              "%1$s: Usage: %2$s [-fhR] {GroupName | GroupID} File ...\n"

$ NOTE  
$ NOTE: In M_MSG_2, %1$s will be the name of the program and  
$ NOTE: %2$s will be the name of the group.  
$ NOTE   

M_MSG_2              "0481-002 %1$s: %2$s is not a recognized group.\n\
\tCheck the /etc/group file for all known groups.\n"

M_MSG_4              "0481-004 Cannot change to directory %s.\n"
