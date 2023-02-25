$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)85	1.1  src/msg/bos/base/auditbin.msg, bosmsg, msg411, GOLD410 2/9/94 08:46:30
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
$ messages for auditbin
$

$set MS_AUDITBIN

$quote "

KILLBIN1	"AUDITBIN: kill process "
KILLBIN2	" and restart <auditbin>"
AUDITBIN	"AUDITBIN: "
ERRPRIV		"** AUDIT_CONFIG privilege required"
ERRSUSPEND      "** cannot suspend auditing"
ERRATTR1 	"** no file attribute "
ERRATTR2 	" in "
ERRFORK		"** fork failed"
ERRSYNC		"** synchronization with child process failed"
ERRMKBIN	"** cannot create bin file "
ERRAUDBIN	"** auditbin() system call failed"
ERRNOBIN	"** cannot find binfile "
ERRRECOVER	"** cannot run recover on binfile "
ERRBACKEND	"** cannot open backend command file "
ERRRCVRBIN 	"** auditbin() system call failed during recovery"
ERRBACKCMD	"** failed backend command "
ERRDUPBIN	"** duplicate auditbin exiting"
ERRNOLOCK	"** cannot create lock file "
ERRBADBIN	"** corrupted bin file "
