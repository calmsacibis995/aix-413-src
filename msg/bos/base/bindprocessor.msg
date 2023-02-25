$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)05	1.2  src/msg/bos/base/bindprocessor.msg, bosmsg, msg411, GOLD410 4/14/94 09:29:09

$ /*
$  * COMPONENT_NAME: (CMDMP) 
$  *
$  * FUNCTIONS: 
$  *
$  * ORIGINS: 83
$  *
$  * LEVEL 1,5 Years Bull Confidential Information
$  *
$  */


$quote "

$set MS_BINDPROC

$ NOTE: The usage of bindprocessor command is printed as follows.
USAGE		"Usage: bindprocessor { -q | -u ProcessID | ProcessID [ProcessorNum] }\n"
$ Do not translate the command name "bindprocessor"

$
$ The following messages are for the error messages.
$	1730-001, 1730-002, 1730-003
$	1730-004, 1730-005, 1730-006
$	$ XXXX-XXX                 message number
$	MSGID "XXXX-XXX ......."   messages
$	$ ......                   note for translating

$ 1730-001
NOTAVAIL	"1730-001: Processor %d is not available\n"
$ "%d" is the logical processor number

$ 1730-002
NOPROC	 	"1730-002: Process %d does not match an existing process\n"
$ "%d" is the process ID

$ 1730-003
NOPERM		"1730-003: Operation not permitted\n"

$ 1730-004
AVAIL		"The available processors are: "

$ 1730-005
FAIL		"1730-004: Command failed\n"

