$ @(#)16 1.10.1.1  src/bos/usr/sbin/wall/wall.msg, cmdcomm, bos411, 9428A410j 2/1/93 10:15:52
$
$ COMPONENT_NAME: (CMDCOMM) user to user communication
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$set MS_WALL
$quote "
BRODCAST "\r\n\n\007Broadcast message from %1$s@%2$s (%3$s) at %4$s ... \r\n\n"
$ %1$s is the user who is sending the message.
$ %2$s is the workstation of the user sending the message.
$ %3$s is the name of tty.
TOOLONG "wall: 0803-010 The message cannot be more than %d characters long."
$ %d is the message character limit.
TRYAGAIN "wall: 0803-011 Cannot create another process at this time.\n"
NOMEMORY "wall: 0803-012 Cannot allocate enough memory.\n"
