$ @(#)62	1.6  src/tcpip/usr/lbin/rexecd/rexecd.msg, tcprcmds, tcpip411, GOLD410 3/9/90 15:04:05
$ 
$ COMPONENT_NAME: TCPIP rexecd.msg
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
$ messages for rexecd
$ The range of message numbers for this file are 601-613
$
$len 
$quote "         define quote character.
$set MS_REXECD        Define initial set#
$ Do not translate %s, %c, %m and rexecd. 

STAUDIT              "rexecd: 0826-601 The auditproc system call failed.  The error is %m."
$ do not translate auditproc
$               If security is enabled and auditproc() fails, logs error   
$               in syslog.   
$ Do not translate rexecd.  
$ Do not translate auditproc.  
GPEERERR             "%s: "
$               Writes argv[0] (procedure name) to stderr when the   
$               getpeername() call fails (retrieves the name bound to the   
$               socket). Stderr should be the remote. (Passed by inetd   
$               when rexecd exec'ed).   
UNK                  "rexecd: 0826-602 Not known."
$ Security message when getpeername fails.   
PEERERR              "getpeername"
$ Do not translate getpeername.  
$               Writes message above via perror for the local system.   
LOGINERR             "rexecd: 0826-603 The login is not correct."
$ \t\t\tSecurity message - see below.   
LOGINERR2            "rexecd: 0826-604 The login is not correct.\n"
$               The message used in 2 places. The first case is when   
$               getpwnam() fails (no passwd entry for this user). The 2nd   
$               case occurs if received password after encryption does not   
$               match that in the password file. Written to stderr.   
NODIR                "rexecd: 0826-605 There is no remote home directory."
$ home refers to the directory specified by the $HOME directory.
$ Security message - see below.   
NODIR2               "rexecd: 0826-606 There is no remote home directory.\n"
RETRY                "rexecd: 0826-607 Cannot create another process at this time.\n"
$               This message is wriiten to stderr when fork() system call   
$               fails. (Too many processes usually).   
LNGSTR               "rexecd: 0826-608 getstr: The string is too long for the buffer."
$ Do not translate getstr.  
$ \t\t\tSecurity message - see below.   
TOOLNG               "rexecd: 0826-609 The string %s is too long for the buffer.\n"
$ \t\t\tMessage printed to stderr whenn the string read from the   
$ \t\t\tremote (user, password, command) is longer than the buffer   
$               size for it (16,16,NCARGS). Exits.   
SECOPEN              "The connection is open."
$ \t\t\tSecurity message written if connection is opened.   
SECLSE               "The connection is closed."
$ \t\t\tSecurity message when connections are closed.   
REMEXEC              "A remote operation is in process."
$ \t\t\tSecurity message indicating operation being performed.   
MSGMAL               "rexecd: 0826-610 There is not enough memory available now.\n"
NOFORK               "rexecd: 0826-611 Cannot create a remote process."
$ \t\t\tSecurity message indicating the fork() system call failed.   
$ \t\t\tProcess table full ? Out of memory ?   
BADRD                "rexecd: 0826-612 Cannot read from the remote line."
SETDEBUG             "rexecd: 0826-613 setsockopt (SO_DEBUG): %m"
$
USERCRED            "rexecd: 0826-614 setpcred(): Cannot set user credentials\n"
$ Do not translate
$               This warning message is written to syslog when when the   
$               socket level option SO_DEBUG can't be set.   
