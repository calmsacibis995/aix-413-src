$ @(#)39	1.4.1.1  src/tcpip/usr/sbin/iptrace/iptrace.msg, tcpras, tcpip411, GOLD410 9/15/93 13:09:26
$
$ COMPONENT_NAME: (TCPIP) tcp/ip communication protocol support
$
$ FUNCTIONS:  iptrace.msg
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ messages for iptrace
$ The range of message numbers for this file are 861-879
$
$set MS_IPTRACE
$
$ Do Not Translate iptrace, kernel, socket

$quote "

$
CANTLDTREXT	"iptrace: 0827-861 Cannot load or configure trace kernel extension.\n"
$
$  the trace program is dynamically loaded as kernel extension.  The
$  above message indicates either the system cannot load or configure
$  the trace program.
$
ERRSOCK		"iptrace: 0827-862 Cannot create socket"
$
$  socket is the communication endpoints used in the BSD version of UNIX or 
$  later version of AT&T UNIX.
$
BYTEREV		"\n%d bytes received\n"
OPEN		"iptrace: 0827-863 Cannot open %s.\n"
NOSTAT		"iptrace: 0827-864 Cannot get information about %s.\n"
CANTWRITE	"iptrace: 0827-865 Cannot write to %s.\n"
CANTWRITET	"iptrace: 0827-866 Cannot write to the trace file.\n"
CANTREAD	"iptrace: 0827-867 Cannot read from %s.\n"
NOTTRACEF	"iptrace: 0827-868 %s is not a trace file.\n"
$
$  The trace file has a special format in which the first few characters
$  in the file have the English characters "version" at the beginning of
$  the file.  The software uses those special characters as keys.
$
ERRORF		"\tError"
NOOPENKMEM	"iptrace: 0827-869 Cannot open /dev/kmem -"
$  Do not translate /dev/kmem -
$  the above message will be followed by a ":" and then the the standard
$  errno (returned from system calls) message.
NOLSEEKKMEM	"iptrace: 0827-870 Cannot lseek on /dev/kmem -"
$  Do not translate lseek, /dev/kmem 
$  the above message will be followed by a ":" and then the the standard
$  errno (returned from system calls) message.
NORDKMEM	"iptrace: 0827-871 Cannot read /dev/kmem -"
$  Do not translate /dev/kmem -
$  the above message will be followed by a ":" and then the the standard
$  errno (returned from system calls) message.
NOPROTO		"iptrace: 0827-872 Protocol %s not in /etc/protocols.\n"
$ Do not translate /etc/protocols
NOHOSTN		"iptrace: 0827-873 Hostname %s not found.\n"
ILLGADDR	"iptrace: 0827-874 Inet Address %s cannot be recognized.\n"
NOSERV		"iptrace: 0827-875 Service %s is not in /etc/services.\n"
$ Do not translate /etc/services
CANTOFFTRACE	"iptrace: 0827-876 Cannot turn off tracing.\n"
USAGE		"Usage: iptrace [-a] [-e] [-d Host [-b]] [-s Host [-b]] [-p Port] [-P Protocol] [-i Interface] LogFile\n"
SETSOCK		"iptrace: 0827-877 setsockopt -"
$ Do not translate setsockopt
SRC1 		"%s: 0827-878 ERROR: SRC not found, continuing without SRC support\n"
$ Do not translate SRC
SRC2		"%1$s: ERROR: '%2$d' recvfrom\n"
$ Do not translate
SRC3		"iptrace: 0827-879 ERROR: This option is not supported.\n"
SRC4		"setsockopt SO_DEBUG: %m"
$ Do not translate
