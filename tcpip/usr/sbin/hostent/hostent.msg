$ @(#)33	1.7  src/tcpip/usr/sbin/hostent/hostent.msg, tcpip, tcpip411, GOLD410 1/4/93 10:40:39
$ 
$ COMPONENT_NAME:  TCPIP 
$ 
$ ORIGINS: 27 
$ 
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
$ combined with the aggregated modules for this product) 
$                  SOURCE MATERIALS 
$ (C) COPYRIGHT International Business Machines Corp. 1989. 
$ All Rights Reserved 
$ 
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$                                                                    
$ messages for hostent 
$ The range of message numbers assigned to this file are 042-054 
$ messages for hostent   
$quote "         define quote character.
$len 
$set MS_HOSTENT        Define initial set#

$ Do not translate %s, %c.  
$ Do not translate -a, -h, -d, -x, -c, -s, -Z, -S.  
$ Do not translate hostent: 
$ IP is internet protocol. Do not translate IP. 
IPEXIST              "hostent: 0822-041 The IP address %s already exists.\n"
INCOMP_SW            "hostent: 0822-042 The specified flags are not compatible.\n"
UNK_SWITCH           "hostent: 0822-043 %c is not a valid flag."
NOT_PARSED           "hostent: 0822-044 A flag was passed with an option that is not expected.\n"
NUM_IPADDR           "hostent: 0822-045 The IP address %s must be numeric.\n"
ONEHOST              "hostent: 0822-046 Specify at least one host.\n"
HOST_OR_IP           "hostent: 0822-047 Specify either a host or an IP address, but not both.\n"
IPNOEXIST            "hostent: 0822-048 The IP address %s does not exist.\n"
NOTFOUND             "hostent: 0822-049 %s is not found.\n"
USAGE1               "\n\
Usage: %s -a IPAddress -h Hostname ...\n"
USAGE2               "\t%s -d IPAddress\n"
USAGE3               "\t%s -X\n"
USAGE4               "\t%s -c IPAddress1 [-h Host ...|-i IPAddress2]\n"
USAGE5               "\t%s -s {IPAddress|Host} [-Z]\n"
USAGE6               "\t%s -S [-Z]\n"
BADADDRESS	     "hostent: 0822-050 %s is an invalid address.\n"
