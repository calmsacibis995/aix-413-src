$ @(#)33	1.1  src/tcpip/usr/sbin/compat/inetexp.msg, tcpip, tcpip411, GOLD410 3/30/94 17:29:20
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
$ messages for inetexp 
$
$ The range of message numbers assigned to this file are 055-068
$len 
$quote "         define quote character.
$set MS_INETEXP        Define initial set#
$ Do not translate %s or %1$s.  
$ Do not translate inetexp:
$ Do not move the %s: or %1$s: at the beginning of each message.

ERR_MALLOC           "%s: 0822-055 There is not enough memory available now.\n"
ERR_STAT             "%s: 0822-056 %s does not exist.\n"
ERR_OPEN2            "%s: 0822-057 Cannot open a temporary file.\n"
ERR_RENAME           "%1$s: 0822-058 Cannot move the temporary file back to %2$s.\n"
ERR_OPEN             "%1$s: 0822-059 Cannot open the ODM object %2$s.\n"
$ Do not translate ODM - This stands for Object Data Manager.
ERR_INIT             "%s: 0822-060 Cannot initialize the Object Data Manager (ODM).\n"
$ Do not translate Object Data Manager (ODM)
ERR_CLOSE            "%1$s: 0822-061 Cannot close %2$s.\n"
$ Error in closing the ODM object: %s is a filename.
ERR_TERM             "%s: 0822-062 An error occurred while exiting the Object Data Manager.\n"
$ Do not translate Object Data Manager
ERR_PATH             "%s: 0822-063 Cannot set the ODM default path.\n"
$ Do not translate ODM - This stands for Object Data Manager.
$ Maximum of MAXRECORDS
ERR_RECNUM            "%s: 0822-064 Too many records in %s, maximum is %d.\n"
