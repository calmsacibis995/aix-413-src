$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)99	1.1  src/msg/bos/base/routed.msg, bosmsg, msg411, GOLD410 2/9/94 09:15:14
$ 
$ COMPONENT_NAME: TCPIP routed.msg
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
$ messages for routed
$

$set MS_ROUTED

$quote "

SNDERR          "sendto"
FAMERR          "\"from\" address in unsupported address family (%d), cmd %d\n"
NOPACKET 	"RIP version 0 packet received from %s! (cmd %d)"
UNKRTR          "trace command from unknown router, %s"
BOGUS 		"bogus input (from passive interface, %s)"
URTRPKT         "packet from unknown router, %s"
RTEERR          "route in unsupported address family (%d), from %s (af %d)\n"
RTEHOST         "bad host in route from %s (af %d)\n"
BADMETRIC 	"bad metric (%d) from %s\n"
SENDUPDATE	"send dynamic update\n"
DELAYUPDATE	"delay dynamic update\n"
SENDDELAY	"send delayed dynamic update\n"
INHIBUPDATE	"inhibit dynamic update for %d usec\n"
SELECTUDT	"select until dynamic update %d/%d sec/usec\n"
UNKSERV         "routed: router/udp: unknown service\n"
USAGE           "usage: routed [ -s ] [ -q ] [ -t ] [ -g ]\n"
FORMAT		"s %d, ibits %x index %d, mod %d, sh %x, or %x &ibits %x\n"
SETTIME		"setitimer: %m\n"
RECERR          "recvfrom"
SOCKET          "socket"
SOCKERR         "socket: %m"
SKTOPT          "setsockopt SO_BROADCAST: %m"
SKTOPT1         "setsockopt SO_REVBUF: %m"
REVBUF		"recv buf %d\n"
BINDERR         "bind"
BINDERR2        "bind: %m"
SOCKERR2        "socket: %m"
IOCTLGIF        "ioctl (get interface configuration)"
IOCTLGIFL       "%s: ioctl (get interface flags)"
IOCTLGADDR      "%s: ioctl (get dstaddr)"
IOCTLBDCST      "%s: ioctl (get broadaddr)"
IOCTLMET        "%s: ioctl (get metric)"
IOCTLNMSK       "%s: ioctl (get netmask)"
MEMERR          "routed: out of memory\n"
MEMERR2         "routed: ifinit: out of memory\n"
REIF            "re-installing interface %s"
ADDROUTE	"adding route to net/host %s through gateway %s: %m\n"
ADDRTE          "SIOCADDRT"
CHGRTE          "changing route from interface %s (timed out)"
DELRTE          "SIOCDELRT"
DELIF           "deleting route to interface %s (timed out)"
TRINI           "traceinit: can't init %s\n"
SRC1		"%s: ERROR: SRC not found, continuing without SRC support\n"
SRC2		"%s: ERROR: '%d' recvfrom\n"
SRC3		"ERROR: routed does not support this option.\n"
SRC4		"Trace"
SRC5		"Active"
SRC6		"Inactive"
SRC7		"Debug"
SRC8		"Signal"
SRC9		"Purpose"
SRC10		"SIGALRM"
SRC11		"Manages timers on routing table entries"
SRC12		"SIGHUP"
SRC13		"Terminates routed"
SRC14		"SIGINT"
SRC15		"Deletes all existing routes"
SRC16		"SIGTERM"
SRC17		"SIGUSR1"
SRC18		"Toggles packet tracing on"
SRC19		"SIGUSR2"
SRC20		"Toggles packet tracing off"
SRC21		"setsockopt SO_DEBUG: %m"
SRC22		"Supplier"
