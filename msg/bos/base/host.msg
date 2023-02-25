$ ENGLISH_SOURCE_VERSION= 1.7
$ @(#)65\t1.1  src/msg/bos/base/host.msg, bosmsg, msg411, GOLD410 2/9/94 09:02:15 
$ 
$ COMPONENT_NAME: TCPIP host.msg
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
$ messages for host
$ The range of message numbers assigned to this file are 801-803
$
$len 
$quote "         define quote character.

$set MS_HOST        Define initial set#

NME_NT_FND           "host: 0827-801 Host name %1$s does not exist.\n"
BAD_ADDR             "host: 0827-802 Address %1$s is not formed correctly.\n"
ADDR_NT_FND          "host: 0827-803 Cannot find address %1$s.\n"
ALIASES              "%1$s is %2$s"
$ This message is printed when aliases are given.
ALIAS_TITLE          ",  Aliases: "
$  5  This message is printed as a title to list the aliases for the  
$  5  named host. 
PRNT_ALIAS           "%1$c %2$s"
$  the first parameter is 1 character for a comma or blank space depending
$  whether the first or not the first alias is printed.
USAGE                "Usage: host HostName\n"
