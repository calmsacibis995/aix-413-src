$ @(#)00	1.1  src/bos/usr/sbin/survd/survd.msg, cmdmp, bos411, 9428A410j 12/15/93 07:45:48

$ /*
$  * COMPONENT_NAME: (CMDDIAG) 
$  *
$  * FUNCTIONS: 
$  *
$  * ORIGINS: 83
$  *
$  * LEVEL 1,5 Years Bull Confidential Information
$  *
$  */


$quote "
$set MS_SURVD

$ NOTE: The usage of survd command is printed as follows.
USAGE		"Usage: survd [[-h] [-d Delay]] | [-r]\n"
$ Do not translate the command name "survd"

$
$ The following messages are for the error messages.
$	0043-001, 0043-002, 0043-003, 0043-004, 0043-005,
$	0043-006, 0043-007
$
$	$ XXXX-XXX                 message number
$	MSGID "XXXX-XXX ......."   messages
$	$ ......                   note for translating

$ 0043-001
NV		"0043-001: Cannot open /dev/nvram\n"

$ 0043-002
NOTSUP		"0043-002: Command not supported\n"

$ 0043-003
FAIL		"0043-003: Command failed\n"

$ 0043-004
NO_FILE		"0043-004: Cannot open %s\n"
$ %s is /etc/survd.pid file

$ 0043-005
PID		"0043-005: Pid found in %s is wrong\n"
$ %s is /etc/survd.pid file

$ 0043-006
SIG		"0043-006: Signal was not sent to daemon\n"

$ 0043-007
UNLINK		"0043-007: Cannot remove %s\n"
$ %s is /etc/survd.pid file

$ 0043-008
DELAY		"0043-008: Delay must be greater than 10 sec\n"

$ 0043-009
READPID		"0043-009: Cannot read daemon pid in %s\n"
$ %s is /etc/survd.pid file

$ 0043-010
EXISTS		"0043-010: Process ID found in %s corresponds to an existing process.\nCommand aborted.\n"
$ %s is /etc/survd.pid file

$ 0043-011
CONSOLE1	"0043-011: Surveillance daemon does not communicate with the bump anymore.\nThe machine is going to reboot\n"

$ 0043-012
CONSOLE2	"0043-012: survd : cannot reset surveillance\n"

