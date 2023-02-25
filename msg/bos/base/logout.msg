$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)35	1.1  src/msg/bos/base/logout.msg, bosmsg, msg411, GOLD410 2/9/94 09:06:03
$
$ COMPONENT_NAME: (CMDSAUTH) security: user authentication
$
$ FUNCTIONS: none
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set MS_LOGOUT

$
$ Messages for the LOGOUT command
$

$quote "

M_SCCSID	"@(#)35	1.1  src/msg/bos/base/logout.msg, bosmsg, msg411, GOLD410 2/9/94 09:06:03"
M_GETPENV	"3004-061 Error getting privileged environment.\n"
M_TTYNAME	"3004-062 Error getting terminal name.\n"
M_TCQMGR	"3004-063 Error getting SAK Manager Process ID.\n"
M_LOGUSER	"3004-064 You must be the login user.\n"
M_LOGTTY	"3004-065 You must be on the login terminal.\n"
