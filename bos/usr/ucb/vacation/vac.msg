$ @(#)97	1.5  src/bos/usr/ucb/vacation/vac.msg, cmdmailx, bos411, 9428A410j 10/28/93 10:30:49
$ 
$ COMPONENT_NAME: CMDMAILX vac.msg
$ 
$ FUNCTIONS: username 
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
$ scmc msg file created Wed Jun 14 16:48:21 1989
$ user:  jeffe

$quote "         define quote character.

$len 
$set MS_vacation        Define initial set#

M_USAGE              "Usage: vacation [-f interval] username (or) vacation -I"
M_NOUSER             "Unknown user %s"
M_NOFROM             "No initial From line"
M_BADFROM            "Funny From line '%s'"
M_NOMSG              "No message to send"
M_BADEXEC            "Cannot exec /usr/sbin/sendmail"
M_NOHOME             "No home!"
M_ECREATE            "Cannot create %s"
M_VAC                "vacation: "
M_EMALLOC            "newstr: cannot alloc memory"
M_SENDING            "Sending %s to %s\n"
