$ ENGLISH_SOURCE_VERSION= 1.3.1.1
$ @(#)00	1.3  src/msg/bos/base/cpu_state.msg, bosmsg, msg411, 9431A411a 8/1/94 06:34:07

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

$set MS_CPUSTATE

$ NOTE: The usages of cpu_state command are printed as follows.
USAGE		"Usage: cpu_state { -l | -d Number | -e Number }\n"
$ Do not translate the command name "cpu_state"

$
$ The following messages are for the error messages.
$	1731-001, 1731-002, 1731-003, 1731-004
$
$	$ XXXX-XXX                 message number
$	MSGID "XXXX-XXX ......."   messages
$	$ ......                   note for translating

$ 1731-001
NOTAVAIL1	"1731-001: Cpu %d is not available\n"
$ "%d" is the processor number

$ 1731-002
$ NO MORE IN USE - REPLACED BY HEAD
HEADER		"\tCpu\tStatus\tLocation\n"

$ 1731-003
NOTAVAIL2 	"1731-003: Cannot enable cpu %d\n"
$ "%d" is the physical processor number

$ 1731-004
NOTAVAIL3	"1731-004: Cannot disable cpu %d\n"
$ "%d" is the physical processor number

$ 1731-005
ODM_ERR		"1731-005: Cannot read cpu objects in ODM\n"
$ ODM means Object Data Manager

$ 1731-006
PHYS		"1731-006: Cannot read physical number of cpu in ODM\n"

$ 1731-007
NOTSUP		"1731-007: Command not supported\n"

$ 1731-008
FAIL		"1731-008: Command failed\n"

$ 1731-009
NUMBER		"1731-009: Invalid cpu number\n"

$ 1731-010
NV		"1731-010: Cannot open /dev/nvram\n" 

$ 1731-011
GETFAIL		"1731-011: Failed in getting state of cpu%d\n"

$ The following strings are the status of the processors displayed
$ when the l flag (listing) is used.
DISABLED	"disabled"
ENABLED		"enabled "
NONE		"????????"

$ New header for listing
HEAD		"\tName\tCpu\t Status \tLocation\n"
