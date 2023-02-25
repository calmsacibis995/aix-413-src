$ @(#)98	1.1  src/bos/usr/sbin/open_door/open_door.msg, cmdcfg, bos412, 9445C412b 11/2/94 03:28:18

$ /*
$  * COMPONENT_NAME: (CMDCFG) 
$  *
$  * FUNCTIONS: 
$  *
$  * ORIGINS: 83
$  *
$  * LEVEL 1,5 Years Bull Confidential Information
$  *
$  */


$quote "

$set MS_RDS


$ NOTE: The usages of open_door command are printed as follows.
USAGE		"Usage: open_door -c CabinetNumber\n"
$ Do not translate the command name "open_door"

$
$ The following messages are for the error messages.
$	0114-001, 0114-002, 0114-003, 0114-004, 0114-005
$
$	$ XXXX-XXX                 message number
$	MSGID "XXXX-XXX ......."   messages
$	$ ......                   note for translating

IO	"0114-001: I/O error\n"
$ 0114-001

FAULT	"0114-002: Cannot transfer data between kernel space and user memory\n"
$ 0114-002

NOTSUP	"0114-003: Command not supported on this model\n"
$ 0114-003

FAIL	"0114-004: Command failed\n"
$ 0114-004

NV	"0114-005: Cannot open /dev/nvram\n"
$ 0114-005

DISPLAY		"Door of cabinet %d is open\n"
$ The above messages are the output displayed by the command.
$ the "%d" is the cabinet number
