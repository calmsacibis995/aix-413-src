$ @(#)68	1.5  src/tcpip/usr/sbin/htable/htable.msg, tcpadmin, tcpip411, GOLD410 3/5/90 17:03:01
$ 
$ COMPONENT_NAME: TCPIP htable.msg
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
$ message for htable
$ The range of message numbers assigned to this file are 051-056
$
$len 
$quote "         define quote character.
$set MS_HTABLE        Define initial set#
$ Do not translate htable. 

OPENHOSTS            "hosts"
OPENGATEWAY          "gateways"
OPENNETWORK          "networks"
USAGE                "Usage: %s [-c ConnectedNets] [-l LocalNets] [File]\n"
TOOMANY              "%s: 0825-051 Too many networks in list\n"
UNKNOWNNET           "%1$s: 0825-052 %2$s is not a known network.\n"
MISSING1             "htable: net"
MISSING2             " missing names.\n"
GATEWAY              "\t# gateway\n"
UNKNOWNKEY           "htable: 0825-053 %d is not a known keyword.\n"
NET                  "net "
GATEMATRIC           " gateway %1$s metric %2$d passive\n"
NOFILE               "There is no %s file.\n"
OUTMEMORY            "htable: 0825-054 There is not enough memory available now.\n"
NONSTANDARD          "htable: 0825-055 %s is not a standard name.\n"
ILLEGAL              "htable: 0825-056 %s is not a legal character.\n"
PARSERR              "\"%1$s\", line %2$d: %3$s\n"
