$ ENGLISH_SOURCE_VERSION= 1.1.1.2
$ @(#)51  1.3  src/msg/bos/base/auditcv.msg, bosmsg, msg411, 9435A411a 8/25/94 16:02:27
$
$ COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
$
$ FUNCTIONS: 
$
$ ORIGINS: 83
$
$
$ LEVEL 1,  5 Years Bull Confidential Information
$

$
$ messages for auditconv
$

$set MS_AUDITCV

$quote "

ERRPRIV		"** SET_PROC_AUDIT privilege required"
ERRCORR		"** Corrupted input bin file"
ERRREAD		"** Error reading bin file"
ERRNOSPACE	"** Out of memory"
ERRWRITE	"** Error writing to output bin file"
ERRLIMIT	"** Error trying to raise ulimit value"
ERRPACK		"** auditpack failed"
USAGE		"Usage: auditconv infile outfile"
