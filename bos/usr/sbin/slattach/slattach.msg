$ @(#)14	1.9  src/bos/usr/sbin/slattach/slattach.msg, cmdnet, bos411, 9428A410j 2/15/94 17:36:01
$quote "         define quote character.

$ COMPONENT_NAME: (CMDNET) Network commands.  
$ FUNCTIONS:  
$ ORIGINS: 27  
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1994  
$ All Rights Reserved 
$ Licensed Materials - Property of IBM
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$ messages for slattach 
$ The range of message numbers assigned to this file are 291-304 
$len 
$set MS_SLATTACH        Define initial set#

$ The first %1$s  or %s is replaced by slattach. Do not move this. 
$ Do not translate %s, %1$s, %2$s, etc.. 
USAGE                "Usage: %s Ttyname [BaudRate[DialString[DebugLevel]]\n"
$ \t\t\tUsage message - printed on error in arguments 
LOCKS                "%s: 0821-291 Device already in use (locked) \n"
$ \t\t\tCould not lock device.  Already in use. 
DEVICE               "%1$s: 0821-292 Cannot open the device %2$s.\n"
$ \t\t\tFailed to open serial device.  User must have read and write permission. 
GETOLDLD             "%1$s: 0821-293 Cannot get current line discipline for %2$s.\n"
$ \t\t\tCould not get current line discipline 
SETOLDLD             "%1$s: 0821-294 Cannot restore line discipline for %2$s to %3$s.\n"
$ \t\t\tCould not restore original line discipline 
LINEDD               "%1$s: The line discipline is %2$s.\n"
$ \t\t\tprint current line discipline 
POSIXDD              "%1$s: 0821-295 Cannot set line discipline for %2$s to posix.\n"
$ Do not translate posix. 
$ \t\t\tCould not switch to the posix line discipline.  Is it loaded? 
SLIPDD               "%1$s: 0821-296 Cannot set line discipline for %2$s to slip.\n"
$ Do not translate slip. 
$ \t\t\tCould not switch to the slip line discipline.  Is it loaded? 
DEBUGL               "%1$s: The debug level is set to %2$d.\n"
$ \t\t\tPrint debug level if specified in command line 
ALTCONN              "%1$s: 0821-297 The device %2$s cannot be opened for a 'direct' line.\n\tCheck the device entry in the /usr/lib/uucp/Devices file.\n"
$ Check the device entry in the file means to refer to that file for 
$ further help. 
$ \t\t\tNo device at requested speed in Devices or can't set device to requested speed. 
DEBUGCHAT            "%1$s: chat(%2$d, %3$x, %4$d)\n"
$ Do not translate. 
$ \t\t\tIf chat debugging set, print arguments to chat 
NULLFLDS             "%s: 0821-298 A null dial string is found.  The program is exiting.\n"
$ \t\t\tExit immediately if a null dial string is "encountered."
CONNTERM             "%1$s: The %2$s connection is terminated.\n"
$ \t\t\tSlattach has received a SIGHUP and terminated 
ODDFLDS              "%1$s: 0821-299 The loadflds routine has found odd number of fields (%2$d).\n\tThe program is exiting.\n"
$ \t\t\tAn odd number of fields has been encountered in the chat expect/send sequence 
CONNEST             "%1$s: The %2$s connection is established.\n"
SUCC_DEVICE         "%1$s: The device open successfully %2$s.\n"
NO_IFCONFIG	    "%1$s: 0821-300 ifconfig command must be run for this tty before slattach can be run.\n"
NOPOP               "%s:Could not remove %s line discipline from %s.\n"
NOIU                "%s:Could not determine which slip interface to use.\n"
WHICHIU             "%s:using slip interface sl%d for %s\n"
$ \t\t\tDo not change the "sl" in "sl%d".
NOATTACH	    "ioctl to attach slip unit failed. errno is %d\n"
SETSID	            "Cannot set up a session group.\n"
