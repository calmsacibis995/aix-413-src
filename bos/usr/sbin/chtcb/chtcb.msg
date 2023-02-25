$ @(#)89        1.4  src/bos/usr/sbin/chtcb/chtcb.msg, cmdspriv, bos411, 9428A410j 6/15/90 23:10:13
$
$ COMPONENT_NAME:  (CMDSPRIV) security: privilege management
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ messages for chtcb
$

$set MS_CHTCB

$quote "

$ USAGE		"Usage: %s {on|off|tp|query} <file> ...\n"
$ NOTINTCB	"%s is not in the TCB or TP\n"
$ ISINTP		"%s is in the TP\n"
$ ISINBOTH	"%s is in the TCB and TP\n"
USAGE		"Usage: %s {on|off|query} <file> ...\n"
NOTINTCB	"%s is not in the TCB\n"
ISINTCB		"%s is in the TCB\n"
TCBERROR	"tcb() error=%d\n"
