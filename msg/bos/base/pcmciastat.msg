$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)99	1.1  src/msg/bos/base/pcmciastat.msg, bosmsg, msg411, 9435A411a 8/29/94 14:19:22
$
$   COMPONENT_NAME: PCMCIAKER
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "

$set ERR_PCMCIASTAT
USAGE		"Usage: pcmciastat -p PCMCIA_Bus_name [ -w Socket_number ]\n"
ELNAME		"pcmciastat: PCMCIA Bus Name %s is invalid.\n"
ESOCKET		"pcmciastat: Socket number %d is invalid.\n"
ECIS		"pcmciastat: Error on reading CIS.\n"
EMALLOC		"pcmciastat: Memory Allocation Error\n"
EODM		"pcmciastat: ODM error.\n"

$set FLD_PCMCIASTAT
FCARDIDIS	"Card ID is %s"
FNOCARDID	"No Card ID"
$ The display width of following messages should be less than 16.
FSOCKET		"Socket"
FEXIST		"Card Exist"
FUNKNOWN	"Unknown Card"
FNOCARD		"No Card"

