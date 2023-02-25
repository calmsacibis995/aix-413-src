$quote "         define quote character.

$ @(#)27	1.3  src/bos/usr/sbin/sliplogin/sliplogin.msg, cmdnet, bos411, 9428A410j 2/15/94 17:38:09
$ COMPONENT_NAME: (CMDNET) Network commands.  
$ FUNCTIONS:  
$ ORIGINS: 27  
$ (C) COPYRIGHT International Business Machines Corp. 1993
$ All Rights Reserved 
$ Licensed Materials - Property of IBM
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$ messages for sliplogin 
$ 
$len 
$set MS_SLIPLOGIN       Define initial set#
$
ACCESS_DENIED	"Sliplogin access denied for %s. No %s entries available.\n"
BADSOCK		"sliplogin: Socket call failed.\n"
CLOSED_SLUNIT	"User %s closed slip unit %d on signal (%s)\n"
DEVLOCKED 	"sliplogin: Device %s is already in locked for use.\n"
FLAGCHK_FAIL	"Interface flag check failed for %s. Error is %m\n"
FLAGCHECK_FAIL	"Interface flag check failed for %s. Errno = %d\n"
GETLOGIN_FAIL	"Access denied - getlogin returned 0\n"
GOING_SLIP	"Activating slip line discipline.\n>"
HOSTFILE_ACCESS	"sliplogin: Cannot access file %s: errno = %s\n"
IFCONF_FAIL	"sliplogin: Attempt to build interface list failed. Exiting.\n"
IF_LOCK_FAIL	"Cannot lock network interface data. quitting\n"
INET_RETR_FAIL	"Retrieval of internet info failed.\n";
INVOKED		"Sliplogin invoked.  Setting up...\n"
KMEM_OPFAIL	"Attempt to open kmem failed.  Errno = %d.\n"
KNLIST_FAIL	"Knlist call failed. Errno = %d.\n"
LOCAL_ADDR	"Called system's internet address is %s\n"
LOCKFAIL	"Attempt to lock device %s failed.\n"
LOGIN_FAIL	"Login failed for %s. Exit status %d from file %s\n"
LOGOUT_FAIL	"Sliplogin logout script failed. Exit status %d from file %s\n"
NETMASK		"The netmask value to use is %s\n"
NOSLIP_IF	"sliplogin: No slip interface available for %s. Exiting.\n"
NOTATTY		"Not on a tty device.  Exiting.\n"
NO_LOGIN_FILE	"Access denied for %s - %s file unavailable.\n"
NO_LO_FILE	"Could not access %s file.\n"
NO_USERNAME	"sliplogin access denied - no username.\n"
REMOTE_ADDR	"Calling system's internet address is %s\n"
SCHK_FAIL	"Socket check on %s interface failed.\n"
SLIPDISC_FAIL	"Sliplogin: Cannot set slip line discipline for %s\n"
SLIP_RMV_FAIL 	"Cannot remove slip discipline from tty. Errno is %d"
SLOGIN_ATTACH	"Attaching slip unit %d for user %s on %s\n"
SL_START	"Starting slip login for %s\n"
TTYSETUP	"Terminal device setup failed.\n"
LADDRCHECK	"Check of local address for interface %s failed.  Errno = %d\n"
LADDRCHK	"Local address check for interface %s failed.  Errno is %m\n"
RADDRCHECK	"Check of remote address for interface %s failed.  Errno = %d\n"
RADDRCHK	"Remote address check for interface %s failed.  Errno is %m\n"
NOATTACH	"ioctl to attach slip unit failed. errno is %d\n"
