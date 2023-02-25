$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)21	1.1  src/msg/bos/base/setclock.msg, bosmsg, msg411, GOLD410 2/9/94 09:16:51
$ 
$ COMPONENT_NAME: TCPIP setclock.msg
$ 
$ FUNCTIONS: 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for setclock
$

$set MS_SETCLOCK

$quote "

ALERT_SYS	"setclock: auditproc: %m"
$ ALERT_SYS	This message is printed for system auditing purposes.
USAGE           "usage: %s <timeserver>\n"
$ USAGE		This message is printed when setclock is not used correctly.
UNKN_HST        "%s: unknown host\n"
$ UNKN_HST	This message is printed when the named host is unknown.
STIME_FAIL      "stime failed"
$ STIME_FAIL	This message is printed when stime fails.
WR_SOCK_FAIL    "write on socket failed"
$ WR_SOCK_FAIL	This message is printed when the write on the socket fails.
NO_RESPONSE     "Time server %s does not respond.\n"
$ NO_RESPONSE	This message is printed when the named time server does not
$ NO_RESPONSE	respond.
RD_SOCK_FAIL    "read on socket failed"
$ RD_SOCK_FAIL	This message is printed when the read on the socket fails.
$ following message is printed in perror
SOCK		"socket"
CONN		"connect"
SETUID		"setclock : setuid"
SELECT		"select"
