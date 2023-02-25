$ @(#)96	1.6  src/bos/usr/sbin/comsat/comsat.msg, cmdmailx, bos411, 9428A410j 6/15/90 22:02:34
$ 
$ COMPONENT_NAME: CMDMAILX comsat.msg
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
$ scmc msg file created Wed Jun 14 16:46:04 1989
$ user:  jeffe

$quote "         define quote character.

$len 
$set MS_comsat        Define initial set#

M_USAGE              "usage: comsat [-s] [-d directory]"
M_ECHDIR             "chdir: %s: %m"
M_ETMP               "open: /etc/utmp: %m"
M_EMALLOC            "malloc failed"
M_NEWMAIL            "%s\n\
\007New mail for %s@%.*s\007 has arrived:%s\n\
----%s\n"
M_MORE               "...more...%s\n"
M_GETSOCK            "getsockname"
M_COMSAT             "comsat"
SETDEBUG             "setsockopt (SO_DEBUG): %m"
