$ @(#)76	1.4  src/tcpip/usr/bin/rsh/rsh.msg, tcprcmds, tcpip411, GOLD410 3/20/91 10:06:15
$ 
$ COMPONENT_NAME: TCPIP rsh.msg
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
$ messages for rsh
$

$set MS_RSH

$quote "

BLOGIN          "/usr/bin/rlogin"
$				AIX: Prints the execv() call return error if any.
UIDERR          "who are you?\n"
$				Can't find a password entry for this user.
UNKSERVER         "rsh: shell/tcp: unknown service\n"
$				getservbyname() can't find shell/tcp service.
NOSTDERR       "rsh: can't establish stderr\n"
$				Socket call failed to set stderr file descriptor.
SETSOCK         "setsockopt (stdin)"
$				Could not set the debug option on the socket associated with
$				stdin.
SETSOCK2        "setsockopt (stderr)"
$				Could not set the debug option on the socket associated with
$				stderr.
FORK            "fork"
$				The fork system call failed. The system error message is
$				printed following this message.
SELECT          "select"
$				The select() system call to see if the socket is
$				ready to be written failed.
USE            "usage: rsh host [ -l login ] [ -n ] command\n"
$				Uasge printed when incorrect options are specified.
NME_NT_FND      "host: name %s NOT FOUND\n"
BADADDR         "%s: address misformed\n"
BADHOST         "%s: host not found\n"

