$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)67	1.1  src/msg/bos/base/hostid.msg, bosmsg, msg411, GOLD410 2/9/94 09:02:22
$ 
$ COMPONENT_NAME: TCPIP hostid.msg
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
$ messages for hostid
$ The range of message numbers assigned to this file is 821
$
$len 
$quote "         define quote character.
$set MS_HOSTID        Define initial set#

AUD_SYS              "hostid: auditproc: %m"
$  1  This message is printed for system auditing purposes. 
USAGE                "Usage: %s [HexNumber or InternetAddress or HostName]\n"
$  2    This message is printed when the hostid program is not 
$  2    used correctly. 
SETHOST              "hostid: 0827-821 sethostid.\n"
