$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)08	1.1  src/msg/bos/base/rwhod.msg, bosmsg, msg411, GOLD410 2/9/94 09:15:49
$ 
$ COMPONENT_NAME: TCPIP rwhod.msg
$ 
$ FUNCTIONS: n/etc/utmpchd 
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
$ messages for rwhod
$

$set MS_RWHOD

$quote "


SUPERUSER   "rwhod: not super user\n"
$               Only superuser can execute this program. Message written to
$               stderr.
UNKSERV     "rwhod: udp/who: unknown service\n"
$               The who/udp service/protocol pair was not found in
$               etc/services by getservbyname(). Message written to stderr.
HOSTNM      "gethostname: %m"
$               The gethostname() call failed.  Redundant as only fails if
$               the addr arg was invalid. Message written to syslog.
UTMP        "/etc/utmp: %m"
$               Could not open or create etc/utmp file. Lists current users.
$               Message written to syslog.
SOCKET      "socket: %m"
$               Could not create a udp socket.
$               Message written to syslog.
SETSOCK     "setsockopt SO_BROADCAST: %m"
$               Could not set broadcast option on the socket.
$               This message is written to syslog.
BIND        "bind: %m"
$               A name could not be bound to the created socket using the port
$               in the sockaddr_in struct. This message is written to syslog.
RECVWRN     "recv: %m"
$               A read error occurred in a recvfrom() call. This message is
$               written to syslog and the process loops listening at its port.
BADADDR     "%d: bad from port"
$               The port number received in the from socket struct does not
$               equal that listed in the etc/services file.
UNKHOST     "%s: unknown host"
$               The received host name is not listed in etc/hosts file, i.e.,
$               the gethostbyname() call failed. The message is written to
$               syslog and the server continues listening.
HOSTNMERR   "malformed host name from %x"
$               The hostname has nonascii or punctuation chars in it.
$               Message is written to syslog. Appears redundant
$               as name is ok in gethostbyname() call above.
WHOPEN      "%s: %m"
$               Can't open rwhod.xxx file where xxx is the host name.
$               Message written to syslog.
MALLOCERR   "rwhod: malloc failed\n"
$               Could not increse the size of utmp image, i.e., malloc or
$               realloc failed. This message is written to stderr.
UTMPRD      "/etc/utmp"
$               System error message written on stderr after this message
$               when a read error occurs on the utmp file.
NODIR       "chdir(/dev): %m"
$               Could not change directory to /dev to check times on each
$               tty found in utmp. Message is written to syslog followed
$               by an exit.
NODIR2      "chdir(%s): %m"
$               Could not change directory to /usr/spool/rwho. Exits.
UNIXNLST    "/unix namelist botch"
$               For AIX, could not read the namelist in the kernal. Used
$               to get sysinfo (idle,user,kernal,wait states) and boottime.
$               Message written to syslog.
VMUNIXNLST  "/vmunix namelist botch"
$               Same as above except average load time is also available.
KMEMOPEN    "/dev/kmem: %m"
$               Cannot open kmem file. Message written to syslog. Exits.
IOCTLERR    "ioctl (get interface configuration)"
$               The ioctl call to get interface information (fill struct
$               ifconf) failed. Message written to syslog. Returns and exits.
IOCTLERR2   "ioctl (get interface flags)"
$				A ioctl call on the socket to get the interface flgs failed.
$				This message is written to syslog.
IFMSG       "Interface %s is down - will ignore"
$				Informational message that a particular interface is down
$				written on syslog.
BRDCSTERR   "Broadcast unsupported on interface %s - ignoring"
$				Informational message written to syslog indicating interface
$				does not support broadcast mode. Prints the interface name.
IOCTLERR3   "ioctl (get dstaddr)"
$				Error message written to syslog when an ioctl call to get
$				the destination address for a pt-to-pt interface fails.
IOCTLERR4   "ioctl (get broadaddr)"
IOCTLERR5   "ioctl (get my addr)"
$				Error message written to syslog when the braodcast address
$				for the interface can't be determined with an ioctl call.
CONFIGM     "config: to %s (0x%x)\n"
$				Prints to stdout a summary of the configuration information.
$				The interface name and address are listed.
$	$$$$ THE FOLLOWING DEBUG MESSAGES ONLY PRINTED WHEN COMPILED WITH DEBUG FLAG
$	$$$$ DEBUG replaces system sendto() routine with a debug routine.
SENDMSG     "sendto %x.%d\n"
$				Debug message listing the socket address and port.
$				Printed on stdout.
HOSTNAME    "hostname %s %s\n"
$				Debug message listing remote host name and time up since
UP          "  up"
$				Debug information appended to time up. Written to stdout.
LOAD        "load %4.2f, %4.2f, %4.2f\n"
$				Debug information: load averages 5, 10, 15 minutes.
UPTIMERR    "   %s ??:??"
$				Debug information: Time interval is < 0 or > 90 days.
DAYHRMIN    "%s %2d+%02d:%02d"
$				Debug information: Updown, days, hours, minutes.
$				Written to stdout if time > 1 day.
HRMIN       "%s    %2d:%02d"
$				Debug information: Updown, hours, minutes.
$				Written to stdout if time is < 24 hours.
SRC1	    "%s: ERROR: SRC not found, continuing without SRC support\n"
SRC2	    "%s: ERROR: '%d' recvfrom\n"
SRC3	    "ERROR: rwhod does not support this option.\n"
SRC4	    "set sockopt SO_DEBUG: %m"
