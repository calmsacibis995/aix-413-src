$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)13	1.1  src/msg/bos/base/traceroute.msg, bosmsg, msg411, GOLD410 2/9/94 09:25:30
$ 
$ COMPONENT_NAME: TCPIP
$ 
$ FUNCTIONS: data 
$
$ ORIGINS: 26 27
$
$   (C) COPYRIGHT International Business Machines Corp. 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for traceroute
$
$set MS_TRACEROUTE

$quote "

USAGE	"Usage: traceroute [-dnrv] [-w wait] [-m max_ttl] [-p port#] [-q nqueries] [-t tos] [-s src_addr] host [data size]\n"
$ Don't translate traceroute, tos, -dnrv, -w, -m, -p, -q, -t or -s.
$ wait is the seconds to wait for reply.
$ port# is the port number to use.
$ nqueries is the number of probe packets to send.
$ src_addr is the source address to put in the out-going probe packet.
$ data size is the amount of data to put in the out-going probe packet.
MAXTTL	"max ttl must be >1\n"
$ Don't translate ttl (Time To Live).
PORTNUM	"port must be >0\n"
$ The port number must be greater than 0.
PROBENUM	"nprobes must be >0\n"
$ The number of probe packets must be greater than 0.
TOSNUM	"tos must be 0 to 255\n"
$ The tos (Type Of Service) of the probe packets must be 0 to 255.
WAITNUM	"wait must be >1 sec\n"
$ The time to wait for probe packet replies must be greater than 1.
HOSTUNK	"%s: unknown host %s\n"
PACKETSIZE	"traceroute: packet size must be %ld <= s < %ld\n"
$ Don't translate traceroute, or s.
ICMPUNK	"icmp: unknown protocol\n"
$ Don't translate icmp.
HOSTUNK1	"traceroute: unknown host %s\n"
$ Don't translate traceroute.
TO	"traceroute to %s (%s)"
$ Don't translate traceroute.
FROM	" from %s"
LINE	", %d hops max, %d byte packets\n"
$ Don't translate hops.
WROTE	"traceroute: wrote %s %d chars, ret=%d\n"
$ Don't translate traceroute, or ret.
SHORTPKT	"packet too short (%d bytes) from %s\n"
FROMTO	"\n%d bytes from %s to %s"
TYPE	": icmp type %d (%s) code %d\n"
$ Don't translate icmp.
BYTESTO	" %d bytes to %s"
