$ @(#)70 1.1.1.2 src/bos/usr/sbin/keycfg/keycfg.msg, dsaukey, bos41J, 9513A_all 3/20/95 07:41:33
$ 
$ COMPONENT_NAME: DSAUKEY
$
$ FUNCTIONS: message file for keycfg command
$ 
$ ORIGINS: 83
$
$ LEVEL 1,5 Years Bull Confidential Information
$
$


$quote "

$set MS_KEYCFG

$ NOTE: The usage of keycfg command are printed as follows.
USAGE		"Usage: keycfg {-d | -c EModeSwitch}\n\
with EModeSwitch = normal or service or secure\n"
$ Do not translate the command name keycfg, normal, service, secure


$
$ The following messages are for the error messages.
$       1104-001, 1104-002, 1104-003, 1104-004
$
$       $ XXXX-XXX                 message number
$       MSGID "XXXX-XXX ......."   messages
$       $ ......                   note for translating

NOTSUP  "1104-001: Command not supported\n"

NV "1104-002: Cannot open /dev/nvram\n"

FAIL "1104-003: Command failed\n"

EXPECTED "1104-004: To modify Electronic Mode Switch, Key Mode Switch must be normal\n"

$ headers when -d flag is used.
HEAD_MS "Mode Switch"
HEAD_KMS "Key Mode Switch"
HEAD_EMS "Electronic Mode Switch"

$ The following strings are the key mode displayed when -d flag is used
NORMAL  "normal"
$ Do not translate normal
SERVICE "service"
$ Do not translate service
SECURE  "secure"
$ Do not translate secure
NONE    "ignored"

$ informational message when command is successfull
SUCCESS "Electronic Mode Switch set to %s\n"

AUTHORISED "1104-005: To modify Electronic Mode Switch,\nElectronic Mode Switch from Service Line must be enabled\n"
$ Do not translate normal
