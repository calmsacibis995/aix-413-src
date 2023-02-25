$ @(#)10  1.1.1.2  src/bos/usr/sbin/auditconv/auditcv.msg, cmdsaud, bos411, 9434A411a 8/19/94 09:08:14
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
