$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)95	1.2  src/msg/bos/base/ipreport.msg, bosmsg, msg411, 9430C411a 7/23/94 12:24:34
$
$ COMPONENT_NAME: (TCPIP) tcp/ip communication protocol support
$
$ FUNCTIONS:  ipreport.msg
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ messages for ipreport
$ The range of messages numbers for this file are 841-843
$
$ note:do not translate the phrase 'ipreport:' at the beginning of the messages,
$ it is the command name
$
$set MS_IPREPORT

$quote "

$
OPEN		"ipreport: 0827-841 Cannot open %s.\n"
CANTREAD	"ipreport: 0827-842 Cannot read from %s.\n"
NOTTRACEF	"ipreport: 0827-843 %s is not a trace file\n"
$  the first few characters of the trace file stored the key word "version"
$  in English to indicate the file is a trace file.
ERRORF		"\tError"
$  the above message will be followed by a : and then the actual error
$  message from the standard system call error message.
USAGE         "Usage: ipreport LogFile\n"
USAGE2        "Usage: ipreport -[rsn12] TraceFile\n"
$  where tracefile is the name of the file which stored the trace info
$  after running the iptrace command.
