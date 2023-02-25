$ ENGLISH_SOURCE_VERSION= 1.2.1.4
$ @(#)98      1.1  src/msg/bos/base/route.msg, bosmsg, msg411, GOLD410 2/9/94 09:15:11
$quote "         define quote character.

$  
$ COMPONENT_NAME: TCPIP route.msg 
$  
$ FUNCTIONS:  
$ ORIGINS: 10  26  27  
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
$ All Rights Reserved 
$ Licensed Materials - Property of IBM 
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$ messages for route 
$ The range of message numbers assigned to this file are 274-290 
$len 
$set MS_ROUTE        Define initial set#

MSG1	"usage: route [ -nqCvf ]  cmd [[ -<qualifers> ] args ]\n"
$ translate only cmd, qualifiers and args
$ cmd = command, qualifiers = command qualifiers, args = command arguments
$
MSG2	"0821-274 (botched keyword: %s)\n"
$ botched is slang for mistyped, probably better phrased as "unknown keyword"
$
MSG3	"0821-275 route: socket"
$ do not translate
$ original message: "0821-275 route-getkerninfo-estimate"
$
MSG4	"0821-276 route: getkerninfo() - estimate"
$ translate only estimate
$
MSG5	"0821-277 out of space\n"
$ original message: "0821-277 actual retrieval of routing table"
$
MSG6	"0821-278 route: getkerninfo() - cannot retrieve routing table"
$
MSG7	"0821-279 writing to routing socket"
$ do not translate socket
$ the following were deleted because the code was removed
$ deleted: MSG8	"got only %d for rlen\n"
$ deleted: this message is a continuation of the previous message
$ deleted: both will always be printed, translate as a sentence
$
MSG9	"done\n"
$ complete, fine'
$
MSG10	"0821-280 old usage of trailing 0, "
MSG11	"assuming route to if\n"
$ this message is a continuation of the previous message
$ both will always be printed, translate as a sentence
$ if = interface
$
MSG12	"0821-281 old usage of trailing digit, "
MSG13	"assuming route via gateway\n"
$ this message is a continuation of the previous message
$ both will always be printed, translate as a sentence
$
MSG14	"%s %s %s: gateway %s"
$ translate only gateway
$
MSG15	"0821-282 not in table\n"
$
MSG16	"0821-283 entry in use\n"
$
MSG17	"0821-284 routing table overflow\n"
$
MSG18	"0821-285 ioctl returns %d\n"
$ do not translate ioctl
$
MSG19	"0821-286 route: out of memory\n"
$ do not translate route
$
MSG20	"0821-287 %s: bad value\n"
MSG21	"got message of size %d\n"
$ the following were deleted because they're duplicates of MSG7,8
$ deleted: MSG22	"0821-288 writing to routing socket"
$ deleted: MSG23	"got only %d for rlen\n"
$
MSG24	"0821-289 routing message version %d not understood\n"
MSG25	"%s\npid: %d, len %d, seq %d, errno %d, flags:"
$ do not translate errno
$ pid = process ID, len = length, seq = sequence
$
MSG26	"\nlocks: "
$ do not translate
$
MSG27	" inits: "
$ do not translate
$
MSG28	"\nsockaddrs: "
$ do not translate
$
MSG29	"%s: link %s; "
$ do not translate
$
MSG30	"%s: iso %s; "
$ do not translate
$
MSG31	"%s: inet %s; "
$ do not translate
$
MSG32	"%s: xns %s; "
$ do not translate
$
$ Note: the following messages form an indexed table.
$
$ do not translate the words before the colon (:)
$ do not translate SIOCADDRT, SIOCDELRT
$
MSG100	"RTM_ADD: Add Route"
MSG101	"RTM_DELETE: Delete Route"
MSG102	"RTM_CHANGE: Change Metrics or flags"
MSG103	"RTM_GET: Report Metrics"
MSG104	"RTM_LOSING: Kernel Suspects Partitioning"
MSG105	"RTM_REDIRECT: Told to use different route"
MSG106	"RTM_MISS: Lookup failed on this address"
MSG107	"RTM_LOCK: fix specified metrics"
MSG108	"RTM_OLDADD: caused by SIOCADDRT"
MSG109	"RTM_OLDDEL: caused by SIOCDELRT"
NOT_SUPPORT	"route: %s: Not Supported.\n"
$
