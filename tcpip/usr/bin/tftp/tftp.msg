$ @(#)82      1.6  src/tcpip/usr/bin/tftp/tftp.msg, tcpfilexfr, tcpip411, GOLD410 3/10/94 18:35:06
$ 
$ COMPONENT_NAME: TCPIP tftp.msg
$ 
$ FUNCTIONS: target 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for tftp
$

$set MS_TFTP

$quote "

BAD_BUS_OPEN    "bad bus open, rc = %d \n"
UNKNOWN_SERVICE "tftp: udp/tftp: unknown service\n"
USAGE           "usage: %s host-name [port]\n"
NME_NT_FND	"tftp: Unknown host %s\n"
UNKNOWN_HOST    "tftp: Address %s misformed\n"
UNKNOWN_HOST2   "%s: Unknown host\n"
BAD_PORT_NO     "%s: bad port number\n"
USING_MODE      "Using %s Mode To Transfer Files.\n"
UNKNOWN_MODE    "%s: Unknown Mode\n"
USAGE_2         "usage: %s ["
MODE_SET        "mode set to %s\n"
UNKHOST		"tftp: %s\n"
NO_TARGET       "No target machine specified.\n"
TFTP            "tftp: "
USAGE3          "usage: %s file ... host:target, or\n"
USAGE_4         "       %s file ... target (when already connected)\n"
USAGE_5         "usage: %s host:file host:file ... file, or\n"
USAGE_6         "usage: %s value\n"
BAD_VAL         "%s: bad value\n"
CONNECTED_TO    "Connected to %s.\n"
NOT_CONNECTED   "Not connected.\n"
MODE_V_T        "Mode: %s Verbose: %s Tracing: %s\n"
MAX_TIMEOUT     "Max-timeout: %d seconds\n"
AMBIGUOUS       "?Ambiguous command\n"
INVALID_CMD     "?INVALID COMMAND\n"
AMB_HELP        "?Ambiguous help command %s\n"
INVALID_HELP    "?Invalid help command %s\n"
PACKET_TRACE_ON "Packet tracing on.\n"
PACKET_TRACE_OFF "Packet tracing off.\n"
VERBOSE_MODE_OFF "Verbose mode off.\n"
VERBOSE_MODE_ON "Verbose mode on.\n"
TRANS_TIMED_OUT "Transfer timed out.\n"
ERROR_CODE      "Error code %d: %s\n"
DISCARDED       "discarded %d packets\n"
OPCODE          "%s opcode=%x "
OPCODES         "%s %s "
FILE_MODE       "<file=%s, mode=%s>\n"
BLOCK_BYTES     "<block=%d, %d bytes>\n"
BLOCK           "<block=%d>\n"
CODE_MSG        "<code=%d, msg=%s>\n"
STATS           "%s %d bytes in %.1f seconds"
BITS_PER_SEC    " [%.0f bits/sec]"
FILE_EXISTS     "File already exists; use '-o' to overwrite\n"
FILE_EXT_UNLINK "File already exists; unlink? "
CANT_UNLINK     "Can't unlink; use '-o'\n"
LOCAL_FILE_ERR  "Local file error\nCode = 2\nCan't open local file\n"
LOCAL_FILE_ERR1 "Local file error\nCode = 1\nCan't open stdin\n"
LOCAL_FILE_ERR2 "Local file error\nCode = 1\nCan't open local file %s\n"
USAGE_7         "usage: %s {-r|-g|-o|-w|-p} <local file> <host> <foreign file> [netascii|image]\n"
HELP_TABLE      "Commands may be abbreviated.  Commands are:\n\n"
HELP_CONN       "%-*s\tConnect to remote tftp\n"
HELP_MODE       "%-*s\tMode\n"
HELP_FILE_TRANS "%-*s\tSet file transfer mode\n"
HELP_SEND_FILE  "%-*s\tSend file\n"
HELP_REC_FILE   "%-*s\tReceive file\n"
HELP_EXIT       "%-*s\tExit tftp\n"
HELP_T_VERBOSE  "%-*s\tToggle verbose mode\n"
HELP_TRACE      "%-*s\tToggle packet tracing\n"
HELP_SHOW_STAT  "%-*s\tShow current status\n"
HELP_SET_OCTECT "%-*s\tSet mode to octet\n"
HELP_SET_ASCII  "%-*s\tSet mode to netascii\n"
HELP_SET_TIME   "%-*s\tSet total retransmission timeout\n"
HELP_PRINT      "%-*s\tPrint help information\n"
CONN_TO         "(to) "
PUTTING         "putting %s to %s:%s [%s]\n"
PUT_FILE        "(file) "
GET_FILES       "(files) "
GET_GETTING     "getting from %s:%s to %s [%s]\n"
GET_USAGE       "       %s file file ... file if connected\n"
SETREXMT_VALUE  "(value) "
SENT            "sent"
TFTPSENTO       "tftp: sendto"
TFTPREC         "tftp: recvfrom"
RECEIVED        "received"
CSENT           "Sent"
CRECEIVED       "Received"
UNDEFERR        "Undefined Error Code"
BADFILE         "File Not Found"
ILLACESS        "Access Violation"
DISKFULL        "Disk Full Or Allocation Exceeded"
ILLOPE          "Illegal TFTP Operation"
BADID           "Unknown Transfer ID"
FILEXIST        "File Already Exists"
BADUSER         "No Such User"
DIR_EXISTS      "Cannot overwrite directory: %s\n"
ONOFF_ON	"on"
ONOFF_OFF	"off"
