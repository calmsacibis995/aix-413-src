$ @(#)22	1.4  src/tcpip/usr/sbin/gated/gated.msg, tcprouting, tcpip411, GOLD410 2/22/94 10:18:47
$
$   COMPONENT_NAME: tcprouting
$
$   FUNCTIONS: none
$
$   ORIGINS: 27,36
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1993
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for gated
$
$	Note:	Messages whose message ids start with <name> are issued   
$               by <name>.c. The collection of all messages issued by <name.c>  
$               is grouped under the set MS_<name>. Messages containing only
$		printf format characters, or whose only text consists of words
$		such as "rip", "hello", "egp", etc. are not included in the
$		catalog.
$  		All names preceeded by a colon (:) are routine names
$		and should not be translated.
$

$quote "

$  All names preceeded by a colon (:) are routine names and should not
$  be translated.
$set MS_MAIN
MAIN_1 "Start %s[%d] version %s built %s"
MAIN_2 "SRC not found, %s continuing without SRC support\n"
MAIN_3 "main: *WARNING* IP forwarding disabled!"
MAIN_4 "main: *WARNING* UDP checksums disabled in kernel"
MAIN_5 "No config file, one interface and a default route, gated exiting"

$set MS_PARSE
PARSE_1 "%s: duplicate switch: %s\n"
PARSE_2 "%s: missing argument for switch: %s\n"
PARSE_3 "%s: invalid switch: %s\n"
PARSE_4 "%s: extraneous information on command line: %s\n"
PARSE_5 "Usage: %s [-c] [-C] [-n] [-N] [-t[flags]] [-f config-file] [trace-file]\n"
$  All names preceeded by a colon (:) are routine names and should not
$  be translated.
$set MS_TASK
TASK_1 "task_main: reinitialization not possible"
TASK_2 "gated does not support this option.\n"
