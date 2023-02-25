$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)02	1.1  src/msg/bos/base/fingerd.msg, bosmsg, msg411, GOLD410 2/9/94 08:58:40
$ 
$ COMPONENT_NAME: TCPIP fingerd.msg
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
$ messages for fingerd
$

$set    MS_FINGERD

$quote  "

$ The name of the peer to which a connection will be made cannot be
$ ascertained. Getpeername returns an error message which replaces %m.
PEER         "getpeername"
PIPE		"pipe"
FORK		"fork"
FOPEN		"fopen"
ERRMSG		"fingerd: %s: %s\r\n"
SETDEBUG        "setsockopt (SO_DEBUG): %m"
