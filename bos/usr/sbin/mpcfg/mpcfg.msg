$ @(#)48	1.4.1.1  src/bos/usr/sbin/mpcfg/mpcfg.msg, dsaubump, bos411, 9436C411a 9/6/94 08:40:35

$ /*
$  * COMPONENT_NAME: DSAUBUMP
$  *
$  * FUNCTIONS: 
$  *
$  * ORIGINS: 83
$  *
$  * LEVEL 1,5 Years Bull Confidential Information
$  *
$  */


$quote "

$set MS_MPCFG


$ NOTE: The usages of mpcfg command are printed as follows.
USAGE		"Usage: mpcfg {-d {-p -m -f} | -c {-p|-m|-f} <index1><value1>... | -r | -s}\n"
$ Do not translate the command name "mpcfg"
$ USAGE is no longer in use


$ The following are the messages asking for a password

NEED_CONTPASS	"Contract flags password:\n"
$ NEED_CONTPASS is no longer in use
NEED_DIAGPASS	"Diagnostic flags password:\n"
$ NEED_DIAGPASS is no longer in use

$
$ The following messages are for the error messages.
$	0044-001, 0044-002, 0044-003, 0044-004, 
$	0044-005, 0044-006
$
$	$ XXXX-XXX                 message number
$	MSGID "XXXX-XXX ......."   messages
$	$ ......                   note for translating

$ 0044-001
NV	"0044-001: Cannot open /dev/nvram\n"

$ 0044-002
OPEN	"0044-002: Cannot open %s\n"
$ %s is the /etc/lpp/diagnostics/data/bump file

$ 0044-003
CORRUPT	"0044-003: Corrupted data in %s\n"
$ %s is the /etc/lpp/diagnostics/data/bump file

$ 0044-004
NOTSUP	"0044-004: Command not supported\n"

$ 0044-005
IO	"0044-005: I/O error\n"

$ 0044-006
FAIL	"0044-006: Command failed\n"

$ 0044-007
OPEN_TTY	"0044-007: Cannot open /dev/tty\n"

$ The following messages are new or changed messages

$ NOTE: The usages of mpcfg command are printed as follows.
USAGE2		"Usage: mpcfg { -c {-f|-m|-p|-S} <index1> <value1> ...\n\
              | -c -w {1|2} | -d {-f -m -p -S} | -r | -s }\n"
$ Do not translate the command name "mpcfg"
$ Translate only "index" and "value"


$ The following are the messages asking for a password

NEED_ROLLPASS	"Rolling Password:"
NEED_DIAGPASS1	"Maintenance Menu Password:"
NEED_DIAGPASS2	"Changing Maintenance Menu Password\n"
NEED_DIAGPASS3	"Customer Maintenance Menu Password:"
NEED_DIAGPASS4	"Changing Customer Maintenance Menu Password\n"
NEED_DIAGPASS5	"Old Password:"
NEED_DIAGPASS6	"New Password:"
NEED_DIAGPASS7	"Enter the new password again:"
NEED_DIAGPASS8	"The password entry does not match, please try again.\n"

$
$ The following messages are for the error messages.
$	0044-001, 0044-002, 0044-003, 0044-004, 
$	0044-005, 0044-006
$
$	$ XXXX-XXX                 message number
$	MSGID "XXXX-XXX ......."   messages
$	$ ......                   note for translating


$ 0044-008
CORRUPT2	"0044-008: Corrupted data in %s (line %d)\n"
$ %s is the /etc/lpp/diagnostics/data/bump file (Do not translate)

$ 0044-009
SCRIPT	"0044-009: Unknown script in %s:%s\n"
$ %s are the modem file name and a script name (Do not translate)

$ 0044-010
LIMIT	"0044-010: Modem Configuration buffer too large\n"
$ %s are the modem file name and a script name

$ 0044-011
YYERR	"0044-011: %s in %s (line %d)\n"
$ Do not translate %s & %d

$ The following messages are for the display option or the save option

SUPPORT		"SERVICE SUPPORT"
$ keep in upper cases

DIAGNOSTIC	"DIAGNOSTIC FLAGS"
$ keep in upper cases

MODEMSITECONF	"MODEM AND SITE CONFIGURATION"
$ keep in upper cases

PHONENB		"PHONE NUMBERS"
$ keep in upper cases

INDEX		"Index"
NAME		"Name"
VALUE		"Value"
PHONE		"Phone"
NUMBER		"Number"

CONTRACTFLAG0	"Remote Service Support"
CONTRACTFLAG1	"Quick On Call Service"
CONTRACTFLAG2	"Service Contract Validity"
CONTRACTFLAG3	"Service Support Type"

DIAGFLAG0	"Remote Authorization"	
DIAGFLAG1	"Autoservice IPL"
DIAGFLAG2	"BUMP Console"
$ Do not translate BUMP which is Bring Up Micro Processor
DIAGFLAG3	"Dial-Out Authorization"
DIAGFLAG4	"Set Mode to Normal When Booting"
DIAGFLAG5	"Electronic Mode Switch from Service Line"
DIAGFLAG6	"Boot Multi-user AIX in Service"
DIAGFLAG7	"Extended Tests"
DIAGFLAG8	"Power On Tests in Trace Mode"
DIAGFLAG9	"Power On Tests in Loop Mode"
DIAGFLAG10	"Fast IPL"
DIAGFLAG11	"Power On Test Error Message"

MODEMCONF0	"Modem Parameters File Name"
MODEMCONF1	"Service Line Speed"
MODEMCONF2	"Protocol Inter Data Block Delay"
MODEMCONF3	"Protocol Time Out"
MODEMCONF4	"Retry Number"
MODEMCONF5	"Customer ID"
MODEMCONF6	"Login ID"
MODEMCONF7	"Password ID"

PHONE0		"Service Center Dial-Out"
PHONE1		"Customer Hub Dial-Out"
PHONE2		"System Dial-In"
PHONE3		"System Operator Voice"

$ 0044-012
PASSERR	"0044-012: You entered an invalid password\n"

$ 0044-013
NOPERM \
"0044-013: Remote Authorization useless:\nRemote Service Support not available\n"
$ Translate "Remote Authorization" as DIAGFLAG0
$ Translate "Remote Service Support" as CONTRACTFLAG0
