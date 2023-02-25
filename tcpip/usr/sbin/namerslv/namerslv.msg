$ @(#)35	1.9  src/tcpip/usr/sbin/namerslv/namerslv.msg, tcpnaming, tcpip411, 9434B411a 8/24/94 16:47:27
$
$ COMPONENT_NAME:  CMDNET   Network Configuration  
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
$ Messages for NAMERSLV.c 
$ The range of message numbers assigned to this file are 167-180  
$quote "         define quote character.
$len 
$set MS_NAMERSLV        Define initial set#

$ Do not translate %s, -a, -i, %c, -X, -s, -B, -e, -E and -D. 
$ Do not translate namerslv: 
$ IP is internet protocol do not translate IP 
DOMEXIST             "%s: 0822-167 A domain already exists: "
NAMEXIST             "%s: 0822-179 A nameserver already exists: "
$ Do not move %s - this is the program name. 
INCOMP_SW            "namerslv: 0822-168 The specified flags are not compatible.\n"
MISSINGFLG           "namerslv: 0822-169 One or more flags are missing.\n"
NOT_PARSED           "namerslv: 0822-170 A flag was passed with an option that is not expected.\n"
NUM_IPADDR           "namerslv: 0822-171 IP address %s is not in the correct format.\n"
SPECNS               "namerslv: 0822-172 Specify a nameserver address.\n"
$ Do not translate nameserver. 
SPEC_SW              "%s: 0822-173 Specify either the -i flag or the -D flag.\n"
SPEC_SW2             "%s: 0822-174 Specify either the -i flag or the -n flag.\n"
UNK_SWITCH           "namerslv: 0822-175 %c is not a valid flag.\n"
USAGE1               "\n\
Usage: %s -a -i IPAddress|-D Domain\n"
USAGE2               "\t%s -d -i IPAddress|-n\n"
USAGE3               "\t%s -X [-I]\n"
USAGE4               "\t%s -c Domain\n"
USAGE5               "\t%s -s [-I|-n] [-Z]\n"
USAGE6               "\t%s -e\n"
USAGE7               "\t%s -E File\n"
USAGE8               "\t%s -b [-i IPAddress [-D Domain]]\n"
USAGE9               "\t%s -B File\n"
NONAMESERV           "%s: 0822-176 There is not a nameserver entry.\n"
NODOMAIN             "%s: 0822-177 There is not a domain entry.\n"
NOENTRIES            "%s: 0822-178 There are no entries.\n"
