$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)55	1.1  src/msg/bos/base/inetimp.msg, bosmsg, msg411, GOLD410 4/26/94 16:54:50
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
$ messages for inetimp   
$ 
$ The range of message numbers assigned to this file are 069-085  
$quote "         define quote character.
$len 
$set MS_INETIMP        Define initial set#

$ Do not translate %s, %1$s, %2$s, %3$s.  
$ Do not translate inetimp:  
ERR_MALLOC           "%s: 0822-069 There is not enough memory available now.\n"
ERR_PROTO            "%1$s: 0822-070 %2$s contains protocol %3$s that is not valid.\n"
ERR_SOCKET           "%1$s: 0822-071 %2$s contains socket type %3$s that is not valid.\n"
$ Do not translate socket.  
ERR_WAIT             "%1$s: 0822-072 %2$s contains wait state %3$s that is not valid.\n"
ERR_ADD              "%s: 0822-073 Cannot add an ODM object.\n"
$ Do not translate ODM.  
ERR_CREATE           "%1$s: 0822-074 Cannot create the ODM object %2$s.\n"
$ Do not translate ODM.  
ERR_OPEN             "%1$s: 0822-075 Cannot open the ODM object %2$s.\n"
$ Do not translate ODM.  
ERR_INIT             "%s: 0822-076 Cannot initialize the Object Data Manager (ODM).\n"
$ Do not translate Object Data Manager (ODM).  
ERR_CLOSE            "%1$s: 0822-077 Cannot close %2$s.\n"
$ Error in closing the ODM object: %s is a filename.  
ERR_TERM             "%s: 0822-078 An error occurred while exiting the Object Data Manager.\n"
$ Do not translate Object Data Manager  
ERR_PATH             "%s: 0822-079 Cannot set the ODM default path.\n"
$ Do not translate ODM.  
$ Maximum of MAXRECORDS
ERR_RECNUM            "%s: 0822-080 Too many records in %s, maximum is %d.\n"
