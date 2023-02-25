$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)87	1.1  src/msg/bos/base/rlogind.msg, bosmsg, msg411, GOLD410 2/9/94 09:14:24
$ 
$ COMPONENT_NAME: TCPIP rlogind.msg
$ 
$ FUNCTIONS:
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1992
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for rlogind
$

$set MS_RLOGIND

$quote "


PEERNM      "Couldn't get peer name of remote host: %m"
$               The call to getpeername() failed, i.e., could not connect to
$               rlogin service port. This error message is written to syslog
$		and records the hostname of the remote host.
PEERNMERR   "getpeername"
$               This meesage is printed at the same time as the one above except
$               that it is written by perror on the local host.
SETDEBUG    "setsockopt (SO_DEBUG): %m"
$               This warning message is written to syslog when when the
$               socket level option SO_DEBUG can't be set. 
SETSOCKOPT  "setsockopt (SO_KEEPALIVE): %m"
$               This warning message is written to syslog when when the
$               socket level option KEEP_ALIVE can't be set. This option
$               forces the connected party to respond to messages to keep the
$               connection alive.
NOPERMIT    "Permission denied"
$               The port address was not a reserved port. (At present < 1024).
$               This message is written to remote host.
ILLEGALPORT "Connection from %s on illegal port"
$		This message is generated at the same time as NOPERMIT. It is
$		written to syslog as a notice recording the hostname of the
$		remote host.
INVIPOPT    "Connection received using IP options (ignored):%s"
$		IP options were received that are unknown/invalid to the 
$		local host.  This message is written as a notice to syslog.
NULLIPOPT   "setsockopt IP_OPTIONS NULL: %m"
$		This message is generated when NULL IP options are received
$		and are required.  This message is written as error to syslog.
ADMISMATCH  "rlogind: Host address mismatch.\r\n"
$		This message is written to the remote host when it's hostname
$		does not match the ip address in the local host's tables.
NOPTY       "Out of ptys"
$               Message written to remote host when no ptys are available.
LOGINERR    "/bin/login"
$               Message written to remote when exec of login fails. System error
$               message is printed following the message.
NOBLKERR    "can not set O_NDELAY on socket"
$               For aiws, fcntl call to set this flag failed. O_NDELAY indicates
$               process should not block for i/o on this socket. Message is
$				written to remote.
NOBLKERR2   "can not set O_NDELAY on pty"
$               Same as above, except that the pty cannot be set nonblocking.
PKTMODE     "can not set TIOCPKT"
$               For aiws, the pty could not be put in the packet mode of 
$				operation.
SELERR      "select"
$               This message and the system error message is written to the
$               remote host. Indicates the select() call to see if any data
$               is pending on selected fd's failed.
RLOGD       "rlogind: %s.\r\n"
$               This message prefaces error messages written to the remote
$               host by fatal().
FATAL       "%s: %s"
$               This message preface error messages written to the remote
$               host that include the system error message written by
$               fatalperror().
FATAL2      "%s: Error %d"
$               Same as above except sytem error value is out of range of
$               sys_nerr so the errno value is printed versus a string.
REMUSER	    "remuser too long"
$		This message is displayed if the remote user name is too long.
LOCUSER     "locuser too long"
$		This message is displayed if the local user name is too long.
TTTOOLONG   "Terminal type too long"
$		This message is displayed if the terminal type name is too long.
UTMPMEM     "utmp malloc failed"
$               An attempt to get memory equal in size to /etc/utmp failed.
$               This is done to later make an entry in the file for the
$               pty. (necessary for aiws).
OPENERR     "open of /etc/portstatus"
$               This message written to syslog when the etc/portstatus file
$               can not be opened. AIX avoids using ptys marked enabled in
$               this file.
NOPTY2      "Out of, or No ptys"
$               This message written to remote when no available pty can
$               be found.`
STATOPENERR "open of port status file failed"
$               This message written to syslog when etc/portstatus file
$               can't be opened.
STATWRT     "write of port status: %m\n"
$               Message written to stslog when an error occurs writing
$               a new pty entry in etc/portstatus.
STATRDERR   "error reading portstatus: i = %d: %m \n"
$               Message written to syslog when an error occurs reading
$               an entry from etc/portstatus.
PTYBUSY     "error: pts%d in use \n"
$               Message written to syslog when an entry for the pty is found
$               in etc/portstatus and it is marked enabled. (shouldn't be)
UNKSTATPORT "unknown portstat 0x%x\n"
$               A matching pty entry was found in etc/portstatus, but the
$               marked status is unknown, i.e., not ENABLE, DISABLE, or HOLD.
UTMPOPENERR "error opening utmp: %m \n"
$               Message written to syslog when etc/utmp can't be opened.
UTMPSEEKERR "error seeking into utmp: offset = %d: %m \n"
$               Message written to syslog when a seek call on the utmp
$               fails. Prints the seek offset and the system error.
UTMPWRT     "write of utmp: %m\n"
$               Message written to syslog when an attempt to update the
$               utmp pty record fails.
VTDMALLOC   "malloc of vtdbuf failed, %m"
$               Messge written to syslog when a malloc call for virtual
$               terminal memory fails.
VTDREALLOC  "realloc of vtdbuf failed, %m"
$               A hft VTD sequence (<esc>,[,x,len1,len2,len3,len4) has
$               indicated a length required greater than the current
$               vtdbuflen. This message is written to syslog when the
$               buffer cannot be reallocated. (realloc() fails).
VTDLEN      "hftvtd: illegal vtd length %d"
$               An hft VTD sequence has indicated a length > 128k.
VTDTYPE     "hftvtd: unknown vtd type 0x%x"
$               Two bytes indicating the vtd type follow the 4 bytes
$               of length. This message is written to syslog when the
$               type is unknown.
VTDREQ      "vtd_hftctl: unknown request: 0x%x"
$               This message is printed when the hft request is unknown.
$               Message is written to syslog. Only request processed is
$               HFQUERY.
VTDACK      "vtd_ack: PTYWRRAW failed, %m"
$               This message is written to syslog when an attempt to
$               acknowledge an hf query using ptywrraw fails.
$
$               Messages on authentication errors
$
M_FAILEXEC    "Sorry. Failed running login shell.\n"
$		This message is displayed when exec of shell fails.
M_FAILTERM    "Sorry. Failed setting terminal ownership and mode.\n"
$		This message is displayed when setting terminal up fails.
M_FAIL_GETSTR   "%s too long\n"
$		This message is displayed when rlogind cannot get the
$		local user, remote user, terminal type and baud rate.
M_FAILAUTH      "Sorry. Failed authentication.\n"
$
M_FAILCRED      "Sorry. Failed setting credentials.\n"
$
M_NOTLOWEST	"Sorry. You must \"exec\" login from the lowest login shell."
$
BAD_UTMP	"Active tty entry not found in utmp file.\n"
$		This message is displayed when the entry for rlogind is not 
$		found in the utmp file during cleanup. 
SOCKNM      	"Couldn't get socket name of remote host: %m"
$               The call to getsockname() failed, i.e., could not connect to
$               rlogin service port. This error message is written to syslog
$		for OCS support.
SOCKNMERR   	"getsockname"
$               This message is printed at the same time as the one above except
$               that it is written by perror on the local host for OCS support.
