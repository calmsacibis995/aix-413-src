$ @(#)74	1.7  src/tcpip/usr/bin/rexec/rexec.msg, tcprcmds, tcpip411, GOLD410 2/5/93 08:49:29
$ 
$ COMPONENT_NAME: TCPIP rexec.msg
$ 
$ FUNCTIONS: setuid getpeername setsockopt 
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
$ messages for rexec
$ The range of message numbers assigned to this file are 301-309
$
$len 
$quote "         define quote character.
$set MS_REXEC        Define initial set#

STAUDIT              "rexec: 0826-301 The auditproc system call failed.  The error is %m."
$ Do not translate rexec, auditproc.
$ Syslog message generated when auditproc() fails. 
HOSTNM               "%1$s: 0826-302 rexec test.  The host name is %2$s.\n"
$ Do not translate rexec.
$ Debug message printing the local and remote host names. 
UNKSERVER            "rexec: 0826-303 The exec/tcp service is not known.\n"
$ Do not translate rexec, exec/tcp.
REXECID              "rexec: 0826-304 Cannot set the user number after the getuid system call."
$ Do not translate rexec, getuid.
$ Could not set uid after call to getuid(). 
REXECPEER            "rexec: 0826-305 The getpeername system call failed."
$ Do not translate rexed, getpeername, socket.
INSOCK               "rexec: 0826-306 Cannot set the standard input socket choices to debug."
$ Do not translate rexec, socket.
$ This message output with the system error message for a failure 
$ set the stdin socket options to debug. 
ERRSOCK              "rexec: 0826-307 Cannot set the standard error socket choices to debug."
$ Do not translate rexec, socket.
$ This message output with the system error message for a failure 
$ set the stderr socket options to debug. 
SOCKTST              "%1$s: rexec test. sockfd: %2$d. errfd: %3$d.\n"
$ Do not translate rexec, sockfd, errfd.
$ For debugging, prints the local host name, stdin, and stderr 
$ decimal values. 
SELTST               "%1$s: rexec test, select wakeup, n = %2$d.\n"
$ Do not translate select, rexec.
$ For debugging, prints the host name and the return value from 
$ a select call to see if either stdin or stderr fd's have data. 
SELEXERR             "Exit on select is -1"
$ Do not translate select.
$ Printed with the sytem error message when the select call to 
$ determine if stdin or stderr have data fails indicating 
$ a disconnect. 
DEBUGRD              "DDD read standard input error count %d:"
$ For debugging, prints the number of bytes read from stdin. 
DRDERR               "DDD read standard error error count %d: "
$ Leave space after : .
$ For debugging, prints the number of bytes read from stderr. 
USEAGE               "Usage: %s [-d] [-n] Host Command\n"
$ Usage message printed when the program is invoked with 
$ erroneous arguments. 
UNK                  "Not known"
$ Part of message written for security audit trail. This message 
$               is put in port id field. 
CLOSE                "closed"
$ Security message field indicating port is closed. 
NME_NT_FND      "rexec: 0826-308 host: name %s NOT FOUND\n"
BADADDR         "rexec: 0826-309 %s: address misformed\n"
BADHOST         "rexec: 0826-310 %s: host not found\n"
SELECT		"select"
FORK		"fork"
NOSTDERR	"rexec: can't establish stderr\n"
