$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)89	1.1  src/msg/bos/base/auditslt.msg, bosmsg, msg411, GOLD410 2/9/94 08:46:43
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
$ (C) COPYRIGHT International Business Machines Corp. 1989-1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$                                                                  

$
$
$ messages for auditselect
$

$set MS_AUDITSLT

$quote "

MISSCLOSE	"** missing closing "
ERRPRIV		"** SET_PROC_AUDIT privilege required"
ERRSUSPEND      "** cannot suspend auditing"
BADUSER		"** unknown user name "
INTERNAL	"** internal error: scan_value(), field type = "
MISSOPER	"** missing operator"
MISSRPAREN	"** missing right parenthesis"
BADFIELD	"** unknown field name"
MISSTERM	"** missing term after logical operator"
MISSRELAT	"** missing relational operator after field name"
BADORDER	"** events are not ordered"
BADRESULTS	"** results are not ordered"
BADCMDS		"** commands are not ordered"
MISSVALUE	"** missing value after relational operator"
INTERNAL2	"** internal error: eval_primary(), field type "
INTERNAL3	"** internal error: eval_primary(), relop type "
USAGE		"Usage: auditselect { -e <expr> | -f <file> } <file> ..."
