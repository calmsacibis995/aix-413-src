$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)68	1.1  src/msg/bos/base/watch.msg, bosmsg, msg411, GOLD410 2/9/94 09:28:36
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
$ messages for watch
$

$set MS_WATCH

$quote "

USAGE		"Usage: watch [-e] [-o file] cmd [arg]*"
INTERRUPT	"Interrupted"
ERRCHILDKILL	"** killing process: "
ERRCHILDGONE	"** child process exiting: "
ERRCHILDCATCH 	"** did not track exit of process  "
ERRSTILLTRCKNG	"** still tracking "
ERRNOMEM	"** out of memory"
ERRPRIV		"** SET_PROC_AUDIT privilege required"
ERREVENTS	"** cannot set audit events"
ERROBJECTS	"** cannot set object events"
ERRQUERY	"** cannot query system auditevents()"
ERRSTANZA1	"** cannot find "
ERRSTANZA2	"** stanza in "
ERRREAD		"** error reading from "
ERRSUSPEND 	"** cannot suspend auditing"
ERRDISABLED 	"** auditing must be disabled"
ERRENABLED 	"** cannot enable auditing"
ERRRESET 	"** cannot reset auditing"
OUTPUT 		"** could not open output"
ALLGONE		"** all processes have exited "
PIPE 		"** could not open pipe"
DEVAUDIT 	"/dev/audit"
