$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)80	1.1  src/msg/bos/base/tcpdump.msg, bosmsg, msg411, GOLD410 4/8/94 09:36:14
$
$   COMPONENT_NAME: TCPIP
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1993
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for tcpdump
$
$set MS_TCPDUMP
$
$quote "

$
USAGE1		"Version %s\n"
$ 'interface' refers to the network interface.
USAGE2		"Usage: tcpdump [-defIlnOpqtvx] [-c count] [-i interface]\n"
$ 'expr' is short for expression.
USAGE3		"\t\t[-F filename] [-r filename] [-w filename] [expr]\n"
$
LISTEN		"%s: listening on %s\n"
$
UPACLEN		"[ unreasonable packet length: %d ]"
$
ODMINIT		"tcpdump: odm_initialize failed!\n"
$
ODMLOCK		"tcpdump: odm_lock failed!\n"
$
NOMAJOR		"tcpdump: Could not get major number for bpf"
$
NOMINOR		"tcpdump: getminor failed!"
$
ODMULOCK	"tcpdump: odm_unlock failed!\n"
$
NOCREATE	"tcpdump: error creating %s.\n"
$
NOLOAD		"tcpdump: error loading /usr/lib/drivers/bpf\n"
$
NOENTRY		"Error calling entry point for "
$
PACGOT		"%d packets received by filter\n"
$
PACNOGOT	"%d packets dropped by kernel\n"
$
BYTES		"read %d bytes\n"
$
PROMISC		"tcpdump: (%s):Promiscuous mode not supported for this interface.\n\tContinuing ...\n"
$
NOMEM		"out of memory"
$
SYNFILTER	"syntax error in filter expression"
$
REJECTS		"expression rejects all packets"
$
UNKNOWNLINK	"unknown link type 0x%x"
$
TCPMOD		"'rcp' modifier applied to host"
$
UDPMOD		"'udp' modifier applied to host"
$
ICMPMOD		"'icmp' modifier applied to host"
$
DIRECTGATE	"direction applied to gateway"
$
BADGATE		"illegal modifier of 'gateway'"
$
LNKCONTEXT	"link layer applied in wrong context"
$
UNKNOWNIP	"unknown ip proto '%s'"
$
UNKNOWNPROTO	"unknown proto '%s'"
$
DIRECTPROTO	"direction applied to gateway"
$
ENCAPARP	"arp does nor encapsulate another protocol"
$
ENCAPRARP	"rarp does nor encapsulate another protocol"
$
BOGUSUDP	"'udp proto' is bad"
$
BOGUSTCP	"'tcp proto' id bad"
$
BOGUSICMP	"'icmp proto' is bad"
$
UNKNOWNNET	"unknown network '%s'"
$
UNKNOWNHOST	"unknown host '%s'"
$
ILLPORT		"illegal qualifier of 'port'"
$
UNKNOWNPORT	"unknown port '%s'"
$
TCPPORT		"port '%s' is tcp"
$
UDPPORT		"port '%s' is udp"
$
GATENAME	"'gateway' requires a name"
$
BADEXPR		"bad expression"
$
DATASIZE	"data size must be 1, 2, or 4"
$
BADINDEXOP	"unsupported index operation"
$
TOOMANYREGS	"too many registers needed to evaluate expression"
$
BCASTLINK	"not a braodcast link"
$
IPBCAST		"only ip bradcast filters supported"
$
IPMCAST		"only ip mulicast filters supported"
$
IPCLASS		"unknown IP address class: %08X"
$
DIVZERO		"division by zero"
$
LONGJUMP	"long jumps not supported"
$
BPFLANG		"requires bpf language %d.%d or higher; kernel is %d.%d"
$
FDDICONFIG	"not configured for fddi"
$
PPPCONFIG	"not configured for ppp"
$
SLIPCONFIG	"not configured for slip"
$
OLDFILEFORMAT	"archaic file format"
$
BADFILEFORMAT	"bad dump file format"
$
TRUNCFILE	"truncated dump file"
$
DUMPEOF		"EOF reading dump file"
$
DUMPERROR	"unknown dump file error code in sf_err()"
$
NO_FR		"-f and -r options are incompatible"
$
CANTFIND	"can't find any interfaces"
$
BADADDR		"bad address %s"
$
BADTOKEN	"illegal token: %s\n"
$
BADCHAR		"illegal char '%c'"
$
CANTOPEN	"can't open '%s'"
$
CANTSTATE	"can't state '%s'"
$
CANTREAD	"problem reading '%s'"
