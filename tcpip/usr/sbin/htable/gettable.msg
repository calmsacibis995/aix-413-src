$ @(#)34	1.6  src/tcpip/usr/sbin/htable/gettable.msg, tcpadmin, tcpip411, GOLD410 6/19/90 15:37:30
$ 
$ COMPONENT_NAME: TCPIP gettable.msg
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
$ message file for gettable
$ The range of message numbers assigned to this file are 001-006
$
$len
$quote "

$set MS_GETTABLE
$ Do not translate gettable.

VERS                 "Version\r\n"
MS_QUERY             "All\r\n"
UNKOPT               "Option %s is not known. It is ignored.\n"
USAGE                "Usage: gettable [-v] Host [OutFile]\n"
UNKSERV              "gettable: 0825-001 hostnames/tcp is not a known service.\n"
$ Do not translate hostnames/tcp.
UNKHOST              "gettable: %s: "
$ Leave a space at the end of message.
SOCKET               "gettable: socket"
$ Do not translate socket.
BIND                 "gettable: bind"
$ Do not translate bind.
CONNECT              "gettable: connect"
CONNOK               "The connection to %s is open.\n"
FDOPEN               "gettable: fdopen"
$ Do not translate fdopen.
GETTABLE             "gettable: "
$ Leave a space at end of line.
BEGINERR             "gettable: 0825-002 There is a BEGIN sequence error.\n"
$ Do not translate BEGIN.
ENDERR               "gettable: 0825-003 There is an END sequence error.\n"
$ Do not translate END.
SERVERR              "gettable: 0825-004 There is a hostname service error: %s."
$ Do not translate hostname.
NOBEGIN              "gettable: 0825-005 There is no BEGIN sequence.\n"
$ Do not translate BEGIN.
NOEND                "gettable: 0825-006 There is no END sequence.\n"
$ Do not translate END.
GETHOST              "Received the host table.\n"
VERSRECV             "Received the version number.\n"
CONNCLOSE            "The connection to %s is closed.\n"
