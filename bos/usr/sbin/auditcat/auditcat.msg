$ @(#)12	1.6  src/bos/usr/sbin/auditcat/auditcat.msg, cmdsaud, bos411, 9428A410j 5/15/91 10:52:23
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
$ messages for audit
$

$set MS_AUDITCAT

$quote "

USAGE	 	"Usage: auditcat [-p|-u] [-o trail] [-r] [bin_file]"
ERRPRIV		"** SET_PROC_AUDIT privilege required"
ERRSUSPEND      "** cannot suspend auditing"
ERRPROC 	"** cannot suspend process auditing"
ERRBIN	 	"** cannot open bin file "
ERRTRAIL	"** cannot create trail file "
ERRPACKING	"** auditpack failed packing "
ERRPACKED	" is already packed"
ERRUNPACK	"** auditpack failed unpacking "
ERRWRITE	"** failed writing to tail"
ERRWRITEBIN	"** failed writing to bin"
ERRBINHD	"** cannot read bin header from "
ERRBADBIN	"** corrupted bin header in "
ERRBADBINTAIL	"** corrupted bin tail in "
ERRNOMEM	"** out of memory"
ERRRDBIN	"** cannot read contents of bin "
ERRSTAT	 	"** cannot stat() "
ERRRDTRAIL	"** cannot read trail file "
ERRTRAILTIME	"** bin older than trail "
ERRUNPACKED	" is already unpacked"
