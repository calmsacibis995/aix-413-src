$ @(#)31	1.1  src/tcpip/usr/sbin/compat/inetserv.msg, tcpip, tcpip411, GOLD410 3/30/94 17:28:57
$
$   COMPONENT_NAME: tcpip
$
$   FUNCTIONS: Manager
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for inetserv   
$
$ The range of message numbers assigned to this file are 086-120  
$quote "         define quote character.
$len 
$set MS_INETSERV        Define initial set#

$ Do not translate inetserv: %c, %s, %1$s, %2$s, etc..  
INCOMP_SW            "inetserv: 0822-086 The specified flags are not compatible.\n"
UNK_SWITCH           "inetserv: 0822-087 %c is not a valid flag.\n"
NOT_PARSED           "inetserv: 0822-088 A flag was passed with an option that is not expected.\n"
ERR_EXEC             "inetserv: 0822-089 Cannot run %s.\n"
ERR_MALLOC           "inetserv: 0822-090 There is not enough memory available now.\n"
ERR_DELETE           "inetserv: 0822-091 Cannot delete the inetserv object.\n"
ERR_ADD              "inetserv: 0822-092 Cannot add an ODM object.\n"
$ Do not translate ODM  
ERR_OPEN             "%1$s: 0822-093 Cannot open the ODM object %2$s.\n"
ERR_INIT             "%s: 0822-094 Cannot initialize the Object Data Manager (ODM).\n"
$ Do not translate Object Data Manager (ODM)  
ERR_CLOSE            "%1$s: 0822-095 Cannot close %2$s.\n"
$ Error in closing the ODM object: %s is a filename.  
ERR_TERM             "%s: 0822-096 An error occurred while exiting the Object Data Manager.\n"
$ Do not translate Object Data Manager  
ERR_PATH             "%s: 0822-097 Cannot set the ODM default path.\n"
$ Do not translate ODM  
SPEC_SW1             "inetserv: 0822-098  Specify the -a, -c, -d, -D, or -s flag.\n"
$ Do not translate -a, -c, -d, -D, -s   
SPEC_SW2             "inetserv: 0822-099 Specify the -v, -p, and -n flags.\n"
$ Do not translate -v, -p, -n   
SPEC_SW3             "inetserv: 0822-100 Specify the -v, -p, -t, -w, -U, and -r flags.\n"
$ Do not translate -v, -p, -t, -w, -U, -r  
SPEC_SW4             "inetserv: 0822-101 Specify either the -S flag or the -I flag.\n"
$ Do not translate -S, -I  
SPEC_SW5             "inetserv: 0822-102 Specify the -v and -p flags.\n"
$ Do not translate -v, -p  
SPEC_SW6             "inetserv: 0822-103 Specify one or more of the flags -V, -P, -n, or -u.\n"
$ Do not translate -V, -P, -n -u  
SPEC_SW7             "inetserv: 0822-104 Specify one or more of the flags -V, -P, -t, -w, -U, -r, -R\n"
$ Do not translate -V, -P, -t -w, -U, -r, -R  
SPEC_SW8             "inetserv: 0822-105 Specify the -X flag or the -v and -p flags.\n"
$ Do not translate -X, -v, -p  
OBJ_SRV1             "inetserv: 0822-106 The %1$s/%2$s service object already exists.\n"
OBJ_SRV2             "inetserv: 0822-107 The %1$s/%2$s service object does not exist.\n\
\tCannot add the configuration.\n"
OBJ_CNF1             "inetserv: 0822-108 The %1$s/%2$s configuration object already exists.\n"
OBJ_CNF2             "inetserv: 0822-109 The %1$s/%2$s object does not exist.\n"
BAD_PORT             "inetserv: 0822-110 The port number must be numeric.\n"
BAD_WAIT             "inetserv: 0822-111 The wait state %s is not valid.\n"
BAD_PROTO            "inetserv: 0822-112 The protocol %s is not valid.\n"
BAD_SOCKET           "inetserv: 0822-113 The socket type %s is not valid.\n"
USAGE1               "\nusage:\t%s -a -S -v servname -p protname -n portnum [-u \"alias...\"]\n"
USAGE2               "\t%s -a -I -v servname -p protname -t socket -w waitind\n"
USAGE3               "\t\t-U user -r serverpath [-R \"serverargs...\"]\n"
USAGE4               "\t%s -c -S -v servname -p protname -V new-name | -P new-prot\n"
USAGE5               "\t\t[-n portnum] [-u \"alias...\"]\n"
USAGE6               "\t%s -c -I -v servname -p protname -V new-name | -P new-prot\n"
USAGE7               "\t\t[-t socktype] [-w waitind] [-U username]\n"
USAGE8               "\t\t[-r serverpath] [-R \"serverargs...\"]\n"
USAGE9               "\t%s -d {-S | -I} {-v servname -p protname | -X}\n"
USAGE10              "\t%s -s [-i] {-S | -I} {-v servname -p protname | -X} [-Z]\n"
USAGE11              "\t%s -D {-v servname -p protname | -X}\n"
SRV_HEADER_MSG	     "servname\t\tPort/protname\tAliases"
CNF_HEADER1_MSG      "servname\tSocket\tprotname Wait/\tUser\tServer Program\tServer Program"
CNF_HEADER2_MSG      "Name\tType\t\t Nowait\t\t\t\tArguments"
BAD_ARGS             "inetserv: 0822-114 Service arguments (-R) must be 50 characters or less"
