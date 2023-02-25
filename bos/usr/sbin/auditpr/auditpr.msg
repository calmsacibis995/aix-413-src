$ @(#)59	1.4  src/bos/usr/sbin/auditpr/auditpr.msg, cmdsaud, bos411, 9428A410j 6/15/90 23:09:24
$
$ COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$                                                                  

$
$ messages for auditpr
$

$set MS_AUDITPR

$quote "

HERROR  	"** invalid fields in -h parameter"
FIELDS  	"** valid fields are -- "
NOTAILS		"** warning - no tails will be printed"
ERRPRIV		"** SET_PROC_AUDIT privilege required"
ERRSUSPEND      "** cannot suspend auditing"
ERRREAD		"** cannot read "
ERRSTANZA1	"** cannot find "
ERRSTANZA2	" in "
NOTAIL		"<tail format undefined>"
INTERNAL	"<INTERNAL ERROR - unknown program type>"
NORUN		"** cannot run "
NOPATHS		" paths not in record"
USAGE	"Usage: auditpr [-t 0|1|2] [-m message] [-r] [-v] [-h field[,field]*]"
