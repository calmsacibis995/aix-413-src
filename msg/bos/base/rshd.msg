$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)02	1.1  src/msg/bos/base/rshd.msg, bosmsg, msg411, GOLD410 2/9/94 09:15:25
$ 
$ COMPONENT_NAME: TCPIP rshd.msg
$ 
$ FUNCTIONS: fork, getpeernamesetsockopt, pipe, rresvport 
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
$ messages for rshd 
$ The range of message numbers for this file are 801-827
$
$len 
$quote "         define quote character.
$set MS_RSHD        Define initial set#
$ Do not translate system call names (such as getpeername)
$ Do not translate anything that is in all caps (such as SO_DEBUG)

PEERNMERR            "rshd: 0826-801 %s: "
$               The call to getpeername() failed, i.e., could not get the name 
$               of the peer connected to socket 0. This message prints argv[0], 
$               the process name, to stderr which should be connected to the 
$               remote host. 
PEERNM               "rshd: 0826-802 The getpeername system call failed."
$               This meesage is printed at the same time as the one above except 
$               that it is written by perror on the local host. 
SETDEBUG             "rshd: 0826-803 Cannot set the SO_DEBUG socket level option: %m."
$               This warning message is written to syslog when when the 
$               socket level option SO_DEBUG can't be set.  
SETSOCKOPT           "rshd: 0826-804 Cannot set the SO_KEEPALIVE socket level option: %m."
$               This warning message is written to syslog when when the 
$               socket level option KEEP_ALIVE can't be set. This option 
$               forces the connected party to respond to messages to keep the 
$               connection alive. 
SETSOCKLING          "rshd: 0826-805 Cannot set the SO_LINGER socket level option: %m."
$               This warning message is written to syslog when when the 
$               socket level option SO_LINGER can't be set. This option 
$               would block the process from closing the socket if unsent 
$               messages were still queued on the socket. 
FRMADDRERR           "rshd: 0826-806 The from address is not valid.\n"
$               This message is written to syslog prior to exit when 
$               the form socket family is not AF_INET or port > 1023 
$               (reserved ports). 
RDERR                "rshd: 0826-807 Cannot read the from socket.  The error is %m."
$               A read error has occurred on the from socket. Exits. 
STDERR               "rshd: 0826-808 Cannot get a reserved port for standard error: %m."
$               This message written to syslog when a reserved port can't be 
$               obtained for stderr. (rresvport() fails). Exits. 
NO2PORT              "rshd: 0826-809 The second port is not reserved.\n"
$               This message written to syslog when port number read from 
$               socket is not a reserved port. (< IPPORT_RESERVED). 
NO2CONN              "rshd: 0826-810 Cannot connect the second port.  The error is %m."
$               This message written to syslog when an attempt to connect 
$               to the port read from the remote fails. 
LOGINERR             "rshd: 0826-811 The remote user login is not correct.\n"
$               The user read from the remote does not have an entry in 
$               etc/passwd. 
NODIR                "rshd: 0826-812 No remote directory.\n"
$               The home dir read from the passwd file for the user does 
$               not exist or the user does not have execute permission for 
$               a dir in the path. 
NOPERMIT             "rshd: 0826-813 Permission is denied.\n"
$               The remote user is not super user and the remote host is 
$               not in host.equiv or the remote host and user are not in 
$               the .rhosts file in users home directory. 
USERINFO             "rshd: 0826-814 Cannot insert a record into utmp.\n"
$
USERCRED             "rshd: 0826-828 Cannot set user credentials.\n"
$ Do not translate utmp
$               This message is written to the remote user when an attempt 
$               to insert a record for this user into utmp fails. 
PIPERR               "rshd: 0826-815 The pipe system call failed.\n"
$               This message written to the remote user when pipe() fails. 
RETRY                "rshd: 0826-816 Cannot run another process at this time.\n"
$               This message written to the remote user when fork() fails. 
PIPESETERR           "rshd: 0826-817 Cannot set the O_NDELAY pipe.\n"
$               For AIX, this message is written to remote when the read end 
$               of the pipe can't be set to non blocking. 
TOOLONG              "rshd: 0826-818 %s is too long for its associated buffer."
$               This message is written to remote when the remote user, 
$               local user, or command string longer than its associated 
$               buffer (16,16,NCARGS). 
USAGE                "Usage: rshd [-ln]"
CONN_IP              "rshd: 0826-819 Received the connection.  IP options %s ignored."
SETSCKIP             "rshd: 0826-820 setsockopt IP_OPTIONS NULL: %m"
$ Do not translate
ILLEGAL_PORT         "rshd: 0826-821 Received a connection from %s on a port that is not valid."
NOADDR               "rshd: 0826-822 Cannot look up the address for %s."
NOADDR2              "rshd: 0826-823 Cannot look up the address for your host."
HOSTADDR1            "rshd: 0826-824 Host address %1$s is not listed for host %2$s."
HOSTADDR2            "rshd: 0826-825 There is a host address that does not match."
HOSTUNK              "rshd: 0826-826 The host name for your address is not known.\n"
LOGINDISABLE         "rshd: 0826-827 Logins not enabled currently.\n"
