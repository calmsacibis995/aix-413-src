$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)01        1.2  src/msg/bos/base/finger.msg, bosmsg, msg411, GOLD410 4/14/94 09:38:40
$ 
$ COMPONENT_NAME: TCPIP finger.msg
$ 
$ FUNCTIONS: s 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for finger
$

$set    MS_FINGER

$quote  "

USAGE		"Usage: finger [-bfhilmpqsw] [login1 [login2 ...] ]\n"
ERR_OPEN		"finger: error opening %s\n"
NOLOG		 "No one logged on\n"
NOSPACE		"finger: out of space.\n"
NOUSER		"finger: %s: no such user.\n"
HEADER1		"Login       Name              TTY Idle    When    Site Info\n"
HEADER2		"Login    TTY Idle    When    Site Info\n"
HEADER3		"Login      TTY            When"
HEADER4		"             Idle"
SITEINFO	"\nSite Info: %s"
PROJECT		"Project: "
NOPLAN		"No Plan.\n"
PALN		"Plan:\n"
PNAME		"%-*.*s  "
QUICKPRINT1		"%c%-*s %-16.16s"
QUICKPRINT2		" %-*s %-16.16s"
NOTLOG		"          Not Logged In\n"
LOGIN1		"Login name: %-10s\t\t\tIn real life: ???\n"
LOGIN2		"Login name: %-10s"
MSGOFF		"\n    (messages off)	"
REALNAME		 "In real life: %s"
PHONE		"\nPhone: %s"
DIR		"\nDirectory: %-25s"
SHELL		"\tShell: %-s"
SINCE1		"\nOn since %15.15s on %s"
IDLE		" Idle Time"
SINCE2		"\nOn since %15.15s on %-*s"
LASTLOGIN1		"\nLast login %10.10s, %4.4s on %s"
FROM		"\n    from %s"
LASTLOGIN2		"\nLast login %16.16s on %s"
STATTTY		"finger: Can't stat %s\n"
DAY		"%d days"
DAYHR1		"%d day %d hours"
DAYHR2		"%d day %d hour"
DAYHR3		"%d days %d hours"
DAYHR4		"%d days %d hour"
HOUR		"%d hours"
HRMIN1		"%d hour %d minutes"
HRMIN2		"%d hour %d minute"
HRMIN3		"%d hours %d minutes"
HRMIN4		"%d hours %d minute"
MINUTE		"%2d minutes"
SECOND		"%2d seconds"
MINSEC1		"%d minute %d seconds"
MINSEC2		"%d minute %d second"
MINSEC3		"%d minutes %d seconds"
MINSEC4		"%d minutes %d second\n"
NME_NT_FND	"finger: Unknown host %s\n"
UNKHOST		"finger: Address %s misformed\n"
UNKSERVICE		"tcp/finger: unknown service\n"
SOCKET		"socket"
CONN		"connect"
