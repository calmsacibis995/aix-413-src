$ @(#)90	1.9  src/tcpip/usr/lbin/telnetd/telnetd.msg, tcptelnet, tcpip411, GOLD410 2/13/94 15:34:29
$ 
$ COMPONENT_NAME: TCPIP telnetd.msg
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
$ messages for telnetd
$

$set MS_TELNETD

$quote "

CATBANNER       "\r\n\r\ntelnet (%s)\r\n"
$ CATBANNER      The argument is the hostname of this system.
$ CATBANNER      This message is displayed to the user on the remote before
$ CATBANNER      the login process is exec'ed.  Therefore, the message is
$ CATBANNER      displayed in the default language of this host.
HERESTR         "\r\n%s here\r\n"
$ HERESTR        This string is printed in response to the AYT request from
$ HERESTR        the remote (telnet) system. <arg 1> is the hostname of
$ HERESTR        this system.
CS_AUDITPROC    "telnetd: auditproc: %m"
$ CS_AUDITPROC    This syslog message is logged if auditproc() returns an error.
GPEERNM_ERR     "%s: getpeername"
$ GPEERNM_ERR     This error message is written to stderr if getpeername
$ GPEERNM_ERR     returns an error.  There is not a '\n' in the format
$ GPEERNM_ERR     statement since perror() is called immediately after
$ GPEERNM_ERR     this message is displayed to provide additional
$ GPEERNM_ERR     information about the error.  <arg 1> is the name of
$ GPEERNM_ERR     the process from argv[0] (normally "telnetd".)
SSO_DEBUG       "setsockopt (SO_SO_DEBUG): %m"
$ SSO_DEBUG       This syslog message is logged if the call to setsockopt
$ SSO_DEBUG       to set SO_DEBUG failed. 
SSOCK_KA        "setsockopt (SO_KEEPALIVE): %m"
$ SSOCK_KA        This syslog message is logged if the call to setsockopt
$ SSOCK_KA        to set S)_KEEPALIVE failed.  <arg 1> is the errno set
$ SSOCK_KA        by setsockopt().
TTLOOP_READ     "ttloop:  read: %m\n"
$ TTLOOP_READ     This syslog message is logged if the read from the net
$ TTLOOP_READ     returns an error.  ttloop is the name of the routine
$ TTLOOP_READ     where the error occurred. <arg 1> is the errno set by read().
TTLOOP_P_DIED   "ttloop:  peer died: %m\n"
$ TTLOOP_P_DIED   This syslog message is logged if the read from the net
$ TTLOOP_P_DIED   returns 0.  It indicates that the peer (the process
$ TTLOOP_P_DIED   telnetd is talking to) has died.
C_PEER_DIED     "peer died"
$ C_PEER_DIED     This message is passed to the csecurity logging routine.
$ C_PEER_DIED     It is logged at the same time as TTLOOP_P_DIED is
$ C_PEER_DIED     displayed to the user.
PORTS_IN_USE    "All network ports in use"
$ PORTS_IN_USE    This message is displayed if all the network ports are
$ PORTS_IN_USE    in use.  The message is actually passed to fatal() which
$ PORTS_IN_USE    prints the message as the argument to the FATAL_FMT
$ PORTS_IN_USE    format message below.  telnetd dies.  The user should
$ PORTS_IN_USE    try again later.
FATAL_FMT       "telnetd: %s.\r\n"
$ FATAL_FMT       This format message prepends the process name to the
$ FATAL_FMT       passed message (<arg 1>).  The message is written to
$ FATAL_FMT       the net.  Because the remote may not map newline to
$ FATAL_FMT       carriage return/newline, both should be used.
FATAL_P_FMT     "%s: %s\r\n"
$ FATAL_P_FMT     This format is used to create a perror style message.
$ FATAL_P_FMT     <arg 1> is the message passed to the routine.  <arg 2>
$ FATAL_P_FMT     is the sys_errlist message for the passed errno.
$ FATAL_P_FMT     sys_errlist is an ascii message.  This message is passed
$ FATAL_P_FMT     to another routine which prepends the process name to the
$ FATAL_P_FMT     message.
SELECT_ERR      "select"
$ SELECT_ERR      This perror style message is displayed using the
$ SELECT_ERR      FATAL_P_FMT and FATAL_FMT format messages above.  This
$ SELECT_ERR      error occurs if the select() returns an unexpected error
$ SELECT_ERR      condition.
PANIC_ERR       "telnetd: panic state=%d\n"
$ PANIC_ERR       This error message indicates that the telnet daemon has
$ PANIC_ERR       gotten corrupted.  <arg 1> is the decimal value of the
$ PANIC_ERR       current (invalid) state.  The daemon exits.
C_PANIC         "panic state"
$ C_PANIC         This message is passed to the csecurity logging routine.
$ C_PANIC         It is logged at the same time at PANIC_ERR is displayed
$ C_PANIC         to the user.
$	The following are messages used for V3 authentication of the user.
M_FAILAUTH    	"Sorry. Failed authentication.\n"
M_FAILCRED 	"Sorry. Failed setting credentials.\n"
M_FAILEXEC    	"Sorry. Failed running login shell.\n"
M_FAILTERM    	"Sorry. Failed setting terminal ownership and mode.\n"
$	The following are messages used for SRC shared memory.
SRC1		"telnetd: ftok for shared memory failed.\n"
SRC2		"telnetd: shmget for shared memory failed.\n"
SRC3		"telnetd: shmat for shared memory failed.\n"
$
M_NOTLOWEST	"Sorry. You must \"exec\" login from the lowest login shell."
$
BAD_UTMP	"Active tty entry not found in utmp file.\n"
$		This message is displayed when the entry for telnetd is not 
$		found in the utmp file during cleanup. 
ERR_GETSOCK     "%s: getsockname"
$               The call to getsockname() failed, i.e., could not connect to
$               telnet service port. This error message is written to syslog
$		for OCS support.
