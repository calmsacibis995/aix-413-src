$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)84	1.1  src/msg/bos/base/audit.msg, bosmsg, msg411, GOLD410 2/9/94 08:46:28
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
$ messages for audit
$

$set MS_AUDIT

$quote "

USAGE		"Usage: audit {on|off|query|start|shutdown} args"
USAGEON		"Usage: audit on [panic]"
USAGEOFF	"Usage: audit off"
USAGESTART	"Usage: audit start"
USAGESHUT	"Usage: audit shutdown"
ENABLED		"auditing enabled"
DISABLED	"auditing disabled"
RESET		"auditing reset"
AUDITING	"auditing "
ON		"on"
OFF		"off"
PANIC		"[panic]"
NONE		"none"
BINOFF		"bin processing off"
BINON		"audit bin manager is process "
AUDEVENTS	"audit events:"
AUDOBJECTS	"audit objects:"
WASOFF		"** auditing disabled already"
WASON		"** auditing enabled already"
ERRSTART	"** auditing not started"
ERRENABLE	"** cannot enable auditing"
ERRCMD		"** cannot invoke audit command"
ERRREAD		"** cannot read auditing status"
ERRDISABLE	"** cannot disable auditing"
ERRRESET	"** cannot reset auditing"
ERRSEVNTS	"** failed setting kernel audit events"
ERRGEVNTS	"** failed getting kernel audit events"
ERRSOBJS	"** failed setting kernel audit objects"
ERRGOBJS	"** failed getting kernel audit objects"
ERRSTREAM	"** failed running stream commands"
ERRBIN		"** cannot exec "
ERRSUSPEND	"** cannot suspend auditing"
ERRPRIV		"** AUDIT_CONFIG privilege required"
ERRSTANZA1	"** cannot find "
ERRSTANZA2	" stanza in "
ERRACCESS	"** cannot read config information"
ERRKWD1		"** cannot find "
ERRKWD2		" keyword in "
ERRKWD3		" stanza in "
ERRSTRMODE	"** illegal streammode value"

$ do not translate /audit

ERRMKDIR	"** cannot make /audit directory"

$ do not translate /audit

ERRNOTDIR	"** /audit is not a directory"
