$ @(#)29	1.1  src/tcpip/usr/sbin/compat/stinet.msg, tcpip, tcpip411, GOLD410 3/30/94 17:28:31
$
$   COMPONENT_NAME: tcpip
$
$   FUNCTIONS: Manager
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for stinet  
$
$ The range of message numbers assigned to this file are 217-230 
$quote "         define quote character.
$len 
$set MS_STINET        Define initial set#

$ Do not translate %s, %1$s, %2$s, 
ERR_FTOK             "%s: 0822-217 The shared memory system call ftok failed.\n"
$ Do not translate ftok 
ERR_SHMGET           "%s: 0822-218 The shared memory system call shmget failed.\n"
$ Do not translate shmget 
ERR_SHMAT            "%s: 0822-219 The shared memory system call shmat failed.\n"
$ Do not translate shmat 
ERR_SHMDT            "%s: 0822-220 The shared memory system call shmdt failed.\n"
$ Do not translate shmdt 
ERR_OPEN             "%1$s: 0822-221 Cannot open the ODM object %2$s.\n"
$ Do not translate ODM. This stands for Object Data Manager 
ERR_INIT             "%s: 0822-222 Cannot initialize the Object Data Manager (ODM).\n"
$ Do not translate Object Data Manager (ODM) 
ERR_CLOSE            "%1$s: 0822-223 Cannot close %2$s.\n"
$ Error in closing the ODM object: %s is a filename. 
ERR_TERM             "%s: 0822-224 An error occurred while exiting the Object Data Manager.\n"
$ Do not translate Object Data Manager  
ERR_PATH             "%s: 0822-225 Cannot set the ODM default path.\n"
$ Do not translate ODM. This stands for Object Data Manager 
SRC_MSG1             "Service"
$ This message when translated cannot be longer than 12 characters. 
$ This is the heading for the service name. 
SRC_MSG2             "Command"
$ This message when translated cannot be longer than 24 characters. 
SRC_MSG3             "Description"
$ This message when translated cannot be longer than 24 characters. 
SRC_MSG4             "Status"
$ This message when translated cannot be longer than 20 characters. 
SRC_MSG5             "active"
$ This message when translated cannot be longer than 20 characters. 
SRC_MSG6             "inactive"
$ This message when translated cannot be longer than 20 characters. 
SRC_MSG7             "Pid"
$ This message when translated cannot be longer than 24 characters. 
$ Pid is the process identifier. 
SRC_MSG8             "Inet Address"
$ This message when translated cannot be longer than 24 characters. 
SRC_MSG9             "Hostname"
$ This message when translated cannot be longer than 24 characters. 
SRC_MSG10            "Debug"
$ This message when translated cannot be longer than 12 characters. 
SRC_MSG11            "Logging"
$ This message when translated cannot be longer than 12 characters. 
SRC_MSG12            "Tracing"
$ This message when translated cannot be longer than 12 characters. 
