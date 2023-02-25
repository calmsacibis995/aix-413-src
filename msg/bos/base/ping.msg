$ ENGLISH_SOURCE_VERSION= 1.11.2.1
$ @(#)32	1.1  src/msg/bos/base/ping.msg, bosmsg, msg411, GOLD410 2/9/94 09:10:59
$
$   COMPONENT_NAME: CMDNET
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1993
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Do not translate ping.
$
$ Message numbers have been assigned a range of 062 - 077
$
$ len
$quote "         define quote character.
$set MS_PING        Define initial set#
$
MSG1	"0821-072 ping: bad number of packets to transmit.\n"
$ 
MSG2	"0821-073 ping: you must be root to use the -f option.\n"
$ do not translate root (root is the name of the superuser)
MSG3	"0821-074 ping: bad timing interval.\n"
MSG4	"0821-075 ping: bad preload value.\n"
MSG5	"0821-076 ping: packet size too large.\n"
MSG6	"0821-077 ping: illegal packet size.\n"
NME_NT_FND      "0821-062 ping: host name %s NOT FOUND\n"
BADADDR         "0821-063 ping: Address %s misformed\n"
MSG8	"0821-064 ping: -f and -i incompatible options.\n"
$ do not translate -f, -i
MSG9	"0821-065 ping: out of memory.\n"
PROTOERR "0821-066 ping: The icmp protocol is not registered.\n"
$ ICMP is not available, i.e., not listed in /etc/protocols.
$ do not translate icmp
SOCKERR "0821-067 ping: The socket creation call failed."
$ Socket creation call failed. The system error message for the
$ failure is appended to this message.
$ do not translate socket
MSG12	"0821-070 ping: record route not available in this implementation.\n"
DATBYTS1 "PING %1$s: (%2$s): %3$d data bytes\n"
DATBYTS2 "PING %1$s: %2$d data bytes\n"
RECERR "0821-068 ping: recvfrom"
$ do not translate recvfrom
SENDTOERR "0821-069 ping: sendto"
$ do not translate sendto
WRTBYTS "ping: wrote %1$s %2$d chars, ret=%3$d\n"
$ informational.  "chars" is an abbreviation for "characters", 
$ "ret" is an abbreviation for "return value".  these abbreviations
$ should be translated into abbreviations appropriate for the 
$ target language.
$ On sento() error, prints echo request packet information;
$ host name, bytes sent, return value. If return < 0, the
$ sento() error message above is printed. This message is
$ printed both when sento() returns -1 or the return value
$ doesn't equal the bytes written.
SHRTPKT "0821-071 ping: The packet of %1$d bytes from %2$s is too short.\n"
$  $ 	The packet is shorter than the minimum required.
$  $ 	The packet size followed by the from address is printed.
MSG14	"%d bytes from %s: icmp_seq=%u"
$ do not translate icmp_seq
MSG15	" ttl=%d"
$ do not translate
MSG16	" time=%ld ms"
$ do not translate ms (milliseconds)
MSG17	" (DUP!)"
$ DUP means duplicate
MSG18	"\nwrong data byte #%d should be 0x%x but was 0x%x"
BYTRECV "%1$d bytes from %2$s: "
$ 1=size of packet; 2=from address.
MSG19	"\t(same route)"
MSG20	"\nunknown option %x"
PINGHDR "----%s PING Statistics----\n"
$ do not translate PING
$
$ the following 5 messages will (optionally) be concatenated into a single
$ line, e.g. "10 packets transmitted, 9 packets received, 10% packet loss"
XMTPKTS	"%ld packets transmitted, "
RECPKTS	"%ld packets received, "
MSG24	"+%ld duplicates, "
$ prints as "+2 duplicates" (2 is variable)
MSG25	"-- somebody's printing up packets!"
LOSTPKTS	"%d%% packet loss"
$
RDTRIPTM	"round-trip min/avg/max = %ld/%lu/%ld ms\n"
$ minimum/average/maximum = 
$ do not translate ms (milliseconds)
$ abbreviations should be translated to corresponding
$ abbreviations in target language.
$
MSG28	"Echo Reply\n"
MSG29	"Destination Net Unreachable\n"
MSG30	"Destination Host Unreachable\n"
MSG31	"Destination Protocol Unreachable\n"
MSG32	"Destination Port Unreachable\n"
MSG33	"frag needed and DF set\n"
$ frag = fragment
$ do not translate DF (don't frag)
MSG34	"Source Route Failed\n"
MSG35	"Dest Unreachable, Bad Code: %d\n"
$ Dest = destination
MSG36	"Source Quench\n"
MSG37	"Redirect Network"
MSG38	"Redirect Host"
MSG39	"Redirect Type of Service and Network"
MSG40	"Redirect Type of Service and Host"
MSG41	"Redirect, Bad Code: %d"
MSG42	"(New addr: 0x%08lx)\n"
$ addr = address
$ don't translate 0x (hex lead-in)
MSG43	"Echo Request\n"
MSG44	"Time to live exceeded\n"
MSG45	"Frag reassembly time exceeded\n"
MSG46	"Time exceeded, Bad Code: %d\n"
MSG47	"Parameter problem: pointer = 0x%02x\n"
$ don't translate 0x (hex lead-in)
MSG48	"Timestamp\n"
MSG49	"Timestamp Reply\n"
MSG50	"Information Request\n"
MSG51	"Information Reply\n"
MSG52	"Address Mask Request\n"
MSG53	"Address Mask Reply\n"
MSG54	"Bad ICMP type: %d\n"
$ do not translate ICMP
MSG55	"TCP: from port %u, to port %u (decimal)\n"
$ do not translate TCP
MSG56	"UDP: from port %u, to port %u (decimal)\n"
$ do not translate UDP
MSG57	"ping: patterns must be specified as hex digits.\n"
$ hex digits means hexadecimal digits
MSG58	"PATTERN: 0x"
$ don't translate 0x (hex lead-in)
USAGE	"usage: ping [-Rdfnqrv] [-c count] [-i wait] [-l preload]\n\t[-p pattern] [-s packetsize] host\n"
$ usage message
MSGTTL	"ping: multicast ttl %u out of range.\n"
MSGINTF	"ping: bad interface address '%s'\n"
