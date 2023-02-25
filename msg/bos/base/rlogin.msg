$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)86	1.1  src/msg/bos/base/rlogin.msg, bosmsg, msg411, GOLD410 2/9/94 09:14:21
$ 
$ COMPONENT_NAME: TCPIP rlogin.msg
$ 
$ FUNCTIONS: setsockopt 
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
$ messages for rlogin
$

$set MS_RLOGIN

$quote "

UIDERR          "Who are you?\n"
$				Can't find an entry in etc/passwd for this user id.
UNKSERVER       "rlogin: login/tcp: unknown service\n"
$				the login/tcp service is not available on this host.
SETSOCKERR      "rlogin: setsockopt (SO_DEBUG)"
$				The socket option SO_DEBUG could not be set. Set if debuf
$				option selected from command line.
SETUIDERR       "rlogin: setuid"
$				Could not set the process effective id. The effective id 
$				is not superuser in this case. 
USEAGE          "usage: rlogin host [ -ex ] [ -l username ] [ -8 ] \n"
$				Printed if incorrect arguments are entered.
FORKERR         "rlogin: fork"
$				Attempt to fork process for login failed.
CLSCON          "Connection closed."
$				Child - Normal end of read connection.
CLSCON2         "\007Connection closed."
$				Child - Error end of read connection.
CLSCON3         "Closed connection."
$				Parent - normal termination of link.
SENTLN          "line gone"
$				Writer has lost line (parent). Writer returns and parent
$				exits.
CLSCONPEER      "\007Connection closed, lost peer."
IOCTLERR        "ioctl"
$				Out of band signal handler ioctl call to check if 
$				at high water mark failed.
RDERR			"read"
$				Child read routine failed on read from socket.
$				Program exits.
NME_NT_FND      "host: name %s NOT FOUND\n"
BADADDR         "%s: address misformed\n"
BADHOST         "%s: host not found\n"

