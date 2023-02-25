$ @(#)99	1.1  src/bos/usr/sbin/feprom_update/feprom_update.msg, cmdcntl, bos411, 9428A410j 12/15/93 07:45:18

$ /*
$  * COMPONENT_NAME: (CMDCNTL) 
$  *
$  * FUNCTIONS: 
$  *
$  * ORIGINS: 83
$  *
$  * LEVEL 1,5 Years Bull Confidential Information
$  *
$  */


$quote "

$set MS_FEPROMUPDATE

$ NOTE: The usage of feprom_update command is printed as follows.
USAGE		"Usage: feprom_update [-f] { file }\n"
$ Do not translate the command name "feprom_update"


$ The following is the answer for command confirmation message 
$ and has to be a single character.
Y	"y"

$
$ The following messages are for the error messages.
$	1734-001, 1734-002, 1734-003
$	1734-004, 1734-005, 1734-006
$	1734-007, 1734-008, 1734-009
$	1734-010, 1734-011, 1734-012
$	1734-013, 1734-014, 1734-015
$
$	$ XXXX-XXX                 message number
$	MSGID "XXXX-XXX ......."   messages
$	$ ......                   note for translating

$ 1734-001
MEM	"1734-001: No memory\n"

$ 1734-002
OPEN	"1734-002: Cannot open file %s\n"
$ "%s" is the file name

$ 1734-003
ACCESS	"1734-003: This commands only runs in service mode\n"

$ 1734-004
FAULT	"1734-004: Cannot transfer data between kernel space and user memory\n"

$ 1734-005
FORMAT	"1734-005: The format of file %s is invalid\n"
$ "%s" is the file name

$ 1734-006
CLOSE	"1734-006: Cannot close file %s\n"
$ "%s" is the file name

$ 1734-007
NOTSUP	"1734-007: Command not supported\n"

$ 1734-008
FAIL	"1734-008: Command failed\n"

$ 1734-009
WRITE	"1734-009: Cannot write in FEPROM\n"

$ 1734-010
CONFIRM	"This command will reboot the system. \
         Are you sure you want to run it <y/n> ?\n"

$ 1734-011
EXIT	"1734-011: Command exited\n"

$ 1734-012
READ	"1734-012: Cannot read file %s\n"
$ %s is the file name

$ 1734-013
REBOOT	"1734-013: Reboot failed\n"

$ 1734-014
NV	"1734-014: Cannot open /dev/nvram\n"

$ 1734-015
LOCK  	"1734-015: Cannot run command for lock file %s exists\n"
$ %s is the lock file name

$ 1734-016
UNLINK	"1734-016: Cannot remove %s file\n"
$ %s is the lock file name

