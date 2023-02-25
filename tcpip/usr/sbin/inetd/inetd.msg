$ @(#)65	1.12.1.2  src/tcpip/usr/sbin/inetd/inetd.msg, tcpinet, tcpip411, GOLD410 6/20/94 13:46:48
$ 
$ COMPONENT_NAME: TCPIP inetd.msg
$ 
$ FUNCTIONS: msetsockopt 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for inetd
$ The range of message numbers for this file are 501-532
$
$len 
$quote "         define quote character.
$set MS_INETD        Define initial set#
$ Do not translate inetd, setsockopt, socket, select, accept getpwnam, 
$ setup_socket, setup_sunrpc, bind, rpc, sunrpc, getsockname, SRC, ODM,
$ ftok, shmget, shmat, recvfrom
$ Do not translate anything that is in all caps (such as SIGALRM)

$               GENERAL: Debug messages are written by syslog under 
$               LOG_DEBUG. The debug option is enabled by a cmd line arg. 
SECOPEN              "A connection is open."
$               CSECURITY audit trail message indicating a connection 
$ \t\t\twas opened. 
$ SECOPEN is an obsolete message.
USAGE           "Usage: inetd [-d] [ConfigurationFile]"
STAUDIT              "inetd: 0826-501 The auditproc system call failed. The error number is %m."
$               If security is turned on and the auditproc() call fails, 
$               this message is written to syslog. 
SELECTWRN            "inetd: 0826-502 The select system call failed.  The error number is %m.\n"
$               Select call failed in loop checking for ports requiring 
$               service. This message is written to syslog. 
NEEDSERV             "A connection requires %s service.\n"
$               Debug message written when select() call has identified 
$               a connection requiring service. The type of service 
$               corresponding to the port is printed. 
ACCEPT               "The accept system call returned file descriptor %d.\n"
$               Debug message printing the results of an accept() call. 
$               This call returns a socket file descriptor for the 1st 
$               connection from possibly a que which forms a connection 
$               to the port. 
ACCEPTWRN            "inetd: 0826-503 The accept system call failed.  The error number is %m."
$               This message is output by syslog/LOG_WARNING if the accept 
$               call fails to make a connection. 
SERVFAIL             "inetd: 0826-504 Cannot start the %1$s/%2$s server..\n"
$               This message is written when the service requires a fork() 
$               (in general, not a builtin) and the number of servers started 
$               within the past 60 sec is up to 40. Limits the rate of 
$               process creation to 40 processes/sec. 
$ SERVFAIL is an obsolete message.
PWNAM                "inetd: 0826-505 The getpwnam system call failed.  %s does not exist."
$               This message is written by the fork() child process created 
$               to provide requested service. The user name was not found 
$               in the passwd file by getpwnam(). The user name is usually 
$               "root". 
EXECL                "Process number %1$d. Server %2$s.\n"
$               Debug message written for forked servers indicating pid and 
$               and server. This message has been commented out where print 
$               was done with fprintf(stderr,...). Second write is to 
$               syslog/LOG_DEBUG. 
$ EXECL is an obsolete message.
EXECV                "Running server %s.  The error number is %m."
$               Syslog message: written after the server has been exec'ed 
$               with the server name and errno if any. 
REAPED               "Child process %d has ended.\n"
$               Debug message: the child pid is printed when the child dies. 
EXITSTAT             "Server %1$s has ended with exit status 0x%2$x."
$               Syslog message: the server name and exit code are printed 
$               when a process dies with a non zero exit code. 
RESTORED             "Service %1$s is restored.  The file descriptor is %2$d.\n"
$               Debug message: prints service and file descriptor. Indicates 
$               the port is ready for a new connection. 
CONFMSG              "%1$s: %m"
$               Syslog message: The configuration file name and error are 
$               written if the file could not be opened. 
$ CONFMSG is an obsolete message.
UNKPROTO             "inetd: 0826-506 Protocol %2$s is not found for service %1$s."
$               Syslog message: A protocol in the config file was not found 
$               in etc/protocols by getprotobyname(). The service and protocol 
$               are printed. 
$ UNKPROTO is an obsolete message.
UNKSERV              "inetd: 0826-507 %1$s/%2$s is not a known service."
$               Syslog message: A service in the config file was not found 
$               in etc/services by getservbyname(). The service/protocol 
$               pair are printed in the message. 
UNKKIND              "inetd: 0826-508 The specified type of socket is not correct for \n\
\t\tservice %1$s, kind %2$d."
$               Syslog message: Written when the kind of socket is niether 
$               sunrpc or socket. The service and kind number are printed. 
$ UNKKIND is an obsolete message.
SOCKSETUP            "inetd: 0826-509 %1$s/%2$s: The setup_socket subroutine failed: %m"
$               Syslog message: The call to set up a socket of kind "socket" 
$               failed (routine setup_socket()). The service/protocol pair 
$               are printed in the message. 
$ SOCKSETUP is an obsolete message.
SUNSETUP             "%1$s/%2$s: The setup_sunrpc subroutine failed for server %3$s: %m"
$               Syslog message: The call to set up a socket of kind "sunrpc" 
$               failed (routine setup_sunrpc()). The service/protocol pair 
$               and the server name are printed in the message. 
UNKSETUP             "inetd: 0826-510 setup: %d is not a known kind."
$               Syslog message: This message printed when the kind is not 
$               socket or sunrpc. (1 or 2). 
SOCKERR              "inetd: 0826-511 %1$s/%2$s: Cannot create a socket.  %m"
$               Syslog message: Printed when an Internet socket can not be 
$               created for a server. The requested service/protocol pair 
$               is printed. 
SETSOCK              "setsockopt (SO_DEBUG): %m"
$               Syslog message: Printed when the socket level option SO_DEBUG 
$               could not set for the socket. SO_DEBUG enables debugging in 
$               underlying protocols. This message is also used in the 
$               sunrpc routine. 
SETSOCKOPT           "setsockopt (SO_REUSEADDR): %m"
$               Syslog message: Printed when the socket level option 
$               SO_REUSEADDR could not set for the socket. Allows reuse 
$               of local addresses in bind() calls. 
BIND                 "inetd: 0826-512 %1$s/%2$s: The bind system call failed. %m"
$               Syslog message: Printed when the bind() call on the socket 
$               fails. Bind names the unnamed socket, in this case ties it 
$               to the Internet port. 
SUNSOCK              "inetd: 0826-513 The socket system call failed for server %1$s. %m"
$               Syslog message: Printed when a Sun Rpc socket can not be 
$               created for a server. The requested service/protocol pair 
$               is printed. 
SUNBIND              "inetd: 0826-514 The bind system call failed for socket %1$s. %m"
$               Syslog message: Printed when the bind() call on a sunrpc 
$               socket fails. Bind names the unnamed socket, in this case 
$               ties it to any address. 
SUNGETSOCK           "inetd: 0826-515 The getsockname system call failed for socket %1$s. %m"
$               Syslog message: Printed when the getsockname() call for the 
$               bound sunrpc socket fails. The socket is named by the bind() 
$               call and this call retrieves the name assigned. 
MEMERR               "inetd: 0826-516 There is not enough storage available now."
$               Syslog message: No memory is available for a new server table 
$               entry or for service name string (used in 2 places). exits(). 
UNKSERVKIND          "inetd: 0826-517 Socket kind %1$s is not a known kind for service %2$s.\n"
$               Syslog message: The kind was not a plain socket or rpc socket. 
$               These are the only two kinds defined at this time. 
$ UNKSERVKIND is an obsolete message.
UNKINTSERV           "inetd: 0826-518 Internal service %s is not known.\n"
$               The service designated as builtin in the config file is not one 
$               the internal services. 
$ UNKINTSERV is an obsolete message.
SUNRPCVER            "Specify both the rpc and version numbers for the sunrpc service %s.\n"
$               Both the version and rpc number must be specified for sun i 
$                               rpc services. 
$ SUNRPCVER is an obsolete message.
TIMERR               "inetd: 0826-519 Cannot get the time of day.\n"
$ TIMERR is an obsolete message.
$               The gettimeofday() system call failed. 
SRC1                 "%1$s: 0826-520 SRC was not found, going on without SRC/ODM support\n"
SRC2                 "%1$s: 0826-521 The ftok subroutine for shared memory failed.\n"
$ SRC2 is an obsolete message.
SRC3                 "%1$s: 0826-522 The shmget system call for shared memory failed.\n"
$ SRC3 is an obsolete message.
SRC4                 "%1$s: 0826-523 The shmat system call for shared memory failed.\n"
$ SRC4 is an obsolete message.
SRC5                 "%1$s: 0826-524 The recvfrom system call failed.  The return value is %2$d.\n"
SRC6                 "inetd: 0826-525 The specified option is not supported.\n"
SRC7                 "Debug"
SRC8                 "Active"
SRC9                 "Not active"
SRC10                "Signal"
SRC11                "Purpose"
SRC12                "SIGALRM"
SRC13                "Establishes socket connections for failed services."
$ Do not tranlate longer than 65 bytes 
SRC14                "SIGHUP"
SRC15                "Rereads the configuration database and reconfigures services."
$ Do not tranlate longer than 65 bytes 
SRC16                "SIGINT"
SRC17                "SIGCHLD"
SRC18                "Restarts the service in case the service ends abnormally."
$ Do not tranlate longer than 65 bytes 
SRC19                "Service"
SRC20                "Command"
SRC21                "Description"
SRC22                "Status"
SRC23                "%1$s: 0826-526 Cannot run %2$s.\n"
$ SRC23 is an obsolete message.
SRC24                "%1$s: 0826-527 There is not enough memory available now.\n"
$ SRC24 is an obsolete message.
ERR_OPEN             "%1$s: 0826-528 Cannot open the ODM object %2$s.\n"
$ ERR_OPEN is an obsolete message.
ERR_INIT             "%s: 0826-529 Cannot initialize the Object Data Manager (ODM).\n"
$ ERR_INIT is an obsolete message.
ERR_CLOSE            "%1$s: 0826-530 An error occurred when closing ODM object %2$s.\n"
$ ERR_CLOSE is an obsolete message.
ERR_TERM             "%s: 0826-531 An error occurred while exiting ODM.\n"
$ ERR_TERM is an obsolete message.
ERR_PATH             "%s: 0826-532 Cannot set the ODM default path.\n"
$ ERR_PATH is an obsolete message.
$
LOOPINGERR	"%s/%s server failing (looping), service terminated\n"
$               This message is output by syslog/LOG_ERR if the service
$		is looping.
REDOMSG		"REDO"
$		Syslog message: When re-adding a service to the internal
$		service table.
ADDMSG		"ADD "
$		Syslog message: When adding a new service to the internal
$		service table.
FREEMSG		"FREE"
$		Syslog message: When deleting a service from the internal
$		service table.
PRNTSERV	"%s: %s proto=%s, wait=%d, user=%s builtin=%x server=%s\n"
$		Syslog message: This reports proto (the protocol used, tcp
$		datagram, sun_rpc), to wait or not to wait (wait and nowait),
$		the user to run the service as, is it a built in (builtin)
$		service, and the server (program) to start.
UNKNOWNKIND	"unknown kind %s for service %s, skipping\n"
$		Syslog message: This reports that an unknown socket type
$		for a particular service was found in the inetd.conf file.
NORPCVER	"sunrpc service %s does not have BOTH rpc and version specified\n"
$		Syslog message: This reports that an rpc number and version
$		number was supplied with a particular service.
UNKNOWNPROTO	"%s: unknown protocol: %s"
$		Syslog message: This reports that an unknown protocol
$		was supplied with an entry in inetd.conf.
UNKNOWNSERV	"%s/%s: unknown service"
$		Syslog message: This reports that an unknown service
$		was supplied with an entry in inetd.conf.
UNKNOWNKS	"%s: unknown kind for service: %d"
$		Syslog message: This reports that an unknown socket type
$		was supplied with a service.
WAITTIME	"inetd: invalid wait time: %s\n"
$		This message is printed when a number is put after the -t
$		flag that does not fall into the valid range of numbers.
$		A valid number is between 1 and 999999 and represents the
$		number of seconds to wait in select() before looping.
