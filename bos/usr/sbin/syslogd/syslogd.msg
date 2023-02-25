$ @(#)80        1.3.3.1  src/bos/usr/sbin/syslogd/syslogd.msg, cmdnet, bos41J, 9519A_all 5/1/95 17:59:07
$ COMPONENT_NAME: (NETCMD) Network commands.
$ FUNCTIONS:
$ ORIGINS: 27
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Messages for syslogd
$
$ Note:
$	ND_ codes are displayed during debugging only
$	NE_ codes are displayed as error messages
$	N_  codes are informational only (e.g. "(ignored)")
$ 
$ Do not translate syslogd, host, %s, %d, %m, %c, %1$s, %2$s, etc....
$
$ Message numbers have been assigned a range of 600-650.

$len
$quote "		define quote character.
$set MS_SYSLOGD		Define initial set#

ND_BADOPEN	"cannot open %s\n"
$ Do not translate "open"
ND_CFLINE	"cfline(%s)\n"
$ Do not translate.
ND_CREATE	"cannot create %s (%d)\n"
$
ND_CVTHNAME	"cvthname(%s)\n"
$ Do not translate.
ND_DOWN		"syslogd: going down on signal %d\n"
$ Do not translate "signal".
ND_GOTMSG	"got a message (%d, %#x)\n"
$
ND_HOST		"Host name for your address (%s) unknown\n"
$
ND_INIT		"init\n"
$ Do not translate.
ND_ISDATE	"isdate: TimeFmt='%s', msg='%s'\n"
$ Do not translate.
ND_KLOG		"can't open /dev/klog (%d)\n"
$ Do not translate "open", "/dev/klog".
ND_LOGGING	"Logging to %s"
$
ND_LOGMSG	"logmsg: pri %o, flags %x, from %s, msg %s\n"
$ Do not translate.
ND_MALFORM	"Malformed from address\n"
$ Displayed when the "from" address is bad (from is not a keyword).
ND_RESTART	"syslogd: restarted\n"
$ 
ND_RFDS		"readfds = %#x\n"
$ Do not translate.
ND_RUNNING	"off & running....\n"
$
ND_SIGALRM	"sigvec failure on SIGALRM errno = %d\n "
$ Do not translate "sigvec", "SIGALRM", "errno".
ND_SIGCHLD	"sigvec failure on SIGCHLD errno = %d\n "
$ Do not translate "sigvec", "SIGALRM", "errno".
ND_SIGHUP	"sigvec failure on SIGHUP errno = %d\n "
$ Do not translate "sigvec", "SIGALRM", "errno".
ND_SIGINT	"sigvec failure on SIGINT errno = %d\n "
$ Do not translate "sigvec", "SIGALRM", "errno".
ND_SIGQUIT	"sigvec failure on SIGQUIT errno = %d\n "
$ Do not translate "sigvec", "SIGALRM", "errno".
ND_SIGTERM	"sigvec failure on SIGTERM errno = %d\n "
$ Do not translate "sigvec", "SIGALRM", "errno".
ND_TIME		"TimeFmt = '%s', TimeLen = %d\n"
$ Do not translate.
NE_NOSRC	"0821-600 %s: continuing without SRC support\n"
$ Do not translate "ERROR", "getsockname", "SRC".
N_IGN		" (ignored)\n"
$
N_USAGE		"usage: syslogd [-d] [-mmarkinterval] [-ppath] [-fconffile]\n"
$ Do not translate.
NME_NT_FND	"syslogd: host name %s NOT FOUND\n"
BADADDR		"syslogd: %s: address misformed\n"
UNKWN_HST	"syslogd: %s: unknown host\n"
ND_SIGPIPE	"sigvec failure on SIGPIPE errno = %d\n "
$
