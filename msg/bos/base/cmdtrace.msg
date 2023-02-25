$ ENGLISH_SOURCE_VERSION= 1.24.1.1
$ @(#)75        1.3  src/msg/bos/base/cmdtrace.msg, bosmsg, msg411, 9431A411a 8/1/94 06:33:40
$ COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
$
$ FUNCTIONS: .msg message catalog file for trace
$
$ ORIGINS: 27, 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$  @IBM_COPYRIGHT@
$
$ (C) COPYRIGHT International Business Machines Corp. 1993
$ All Rights Reserved
$
$  LEVEL 1, 5 Years Bull Confidential Information
$  

$set 1
$quote "

$ $ trace/main.c
CAT_TRC_CHAN2BIG         "\
0454-001 Channel %1$d is out of range.\n\
         The range of channels is from 0 to %2$d.\n"
$ %d will be expanded to a number.
$ %d will be expanded to a number.

CAT_TRC_USAGE1           "\
0454-002 Usage:trace -fl -a -g -d -s -h -j event_list -k event_list -m message\n\
                -o outfile -T trace_buffer_size -L log_file_size -c -n\n\
-f          Log only the first buffer of trace data collected.\n\
-l          Log only the last  buffer of trace data collected.\n\
-a          Start tracing and exit. Default is start trace, enter subcommand.\n\
-g          Start generic tracing. Option -a must also be specified.\n\
-d          Delay trace data collection until 'trcon' subcommand\n\
-s          Stop tracing when trace log fills. Default is to wraparound.\n\
-h          Suppress date/sysname/message header to the trace log.\n\
-j e1,,eN   Collect trace data only for the events in list. Default is all.\n"
$ Do not translate the usage flags (words that begin with '-').
$ Do not translate the left side of the screen, except for the words
$ 'message', 'filename, and 'size'.

CAT_TRC_USAGE2           "\
-k e1,,eN   Do not collect trace data for the events in list.\n\
-m message  Text to be included in trace log header record.\n\
-o filename Write trace log to 'filename'. Default is %1$s\n\
-o -        Write trace log to stdout.\n\
-T size     Trace buf size = 'size' bytes. Default is %2$d\n\
-L size     Log file size = 'size' bytes.  Default is %3$d\n\
-c          Save the old trace log in a .old file.\n\
-n          Write in trace log the nm of all loader entries.\n\
modes f and l are mutually exclusive.\n"
$ This is the second half of the USAGE screen. It does not have a
$ message number.
$ Do not translate the left side of the screen, except for the words
$ 'message', 'filename, and 'size'.

CAT_TRC_USAGE3           "\
0454-040\n\
Error: Maximum logfile size (%1$d bytes) must be at least twice\n\
the size of the trace buffer (%2$d bytes).\n"

CAT_TRC_USAGE4           "\
0454-041\n\
Error: Maximum logfile size (%1$d bytes) must be at least\n\
the size of the trace buffer (%2$d bytes).\n"

CAT_TRC_USAGE5		"\
0454-042\n\
Warning : Flags -c and -o- are mutually exclusive.\n\
Only flag -o- is accepted.\n"

CAT_TRC_USAGE6		"\
0454-043\n\
Warning : Option -n and -h are mutually exclusive.\n\
Only option -h is accepted.\n"

CAT_TRC_HELP             "\
0454-003\n\
trcon       Start collection of data\n\
trcoff      Stop  collection of data\n\
q,quit      Stop  collection of data and exit trace\n\
! command   Run command 'command'\n\
?           Display summary of trace commands\n"
$ This is a help screen.
$ Do not translate the left side, except for the word 'command'.

CAT_TRC_OFF              "\
0454-004 tracing is now OFF\n"
$ This message occurs when the user types <DEL> to tell him that
$ tracing is now off.

CAT_TRC_ON               "\
0454-005 tracing is now ON\n"
$ This message occurs when the user types <DEL> to tell him that
$ tracing is now on.

$ $ trcrpt/getevent.c
CAT_TPT_BADTYPE       "\
0454-077\n\
Undefined event type detected on event %1$d, file offset 0x%2$x.\n\
This means that the trace file %3$s is not a properly formatted trace file.\n"
$ %d will be expanded to the number of the bad event
$ %x will be expanded to file offset of the bad event
$ %s will be expanded to name of the trace file

CAT_TRC_STOP         "\
0454-010 Usage:  %s \n"

CAT_TRC_DEAD         "\
Usage:  trcdead [-o filename] dumpimage\n\
\n\
-o filename     Write trace entries to 'filename'.\n\
dumpimage       System dump image.\n\
\n\
A trace session must be in progress when the system dump\n\
is invoked.\n"

CAT_TRC_UPDATE          "\
0454-012 Usage:  trcupdate -o -t tmpltfile -x idlist file\n\
\n\
Update /etc/trcfmt template file from file.trc\n\
\n\
-o           No version number checking.\n\
\n\
-t tmpltfile Use 'tmpltfile' as template file instead of /etc/trcfmt.\n\
-x idlist    Extract the templates in 'idlist' from template file to stdout.\n"

$ $ trcrpt/gettmplt.c
CAT_TPT_MULT        "\
0454-022 Multiple occurrences of trace id %03x.\n"
$ %x will be expanded to the numberic value of the trace id.

CAT_TPT_TMPLT2BIG   "\
0454-024 The template is too large.\n\
         The maximum size of a template must be less than %1$d characters.\n"
$ %d will be expanded to the line number of the bad template.
$ %s will be expanded to name of the template file containing the template.
$ %d will be expanded to a number.

CAT_TPT_SOL         "\
0454-025 Expected a 3 hex digit trace id at the start of the line.\n\
         This could mean that the beginning of this template is wrong,\n\
         or that the previous template is missing a '\\' continuation\n\
         character.\n"

CAT_TPT_UNEXPEOL    "\
0454-026 Unexpected end of line encountered.\n\
         A template must begin with a 3 hex digit template id,\n\
         a version field, and a trace label.\n"

CAT_TPT_TFMT_C       "\
0454-013 Bad format for traceid %s\n\
A traceid consists of 2 or 3 hexadecimal digits.\n\
A 2 digit traceid represents a range of 16 traceids.\n"
$ The %s will be expanded to the bad traceid.
$ traceid stands for trace identifier.

CAT_TPT_TFMT        "\
0454-027 Bad format for trace id %s.\n\
         A trace id must consist of exactly 3 hex numbers.\n"

CAT_TPT_VFMT        "\
0454-028 Bad format for version field %s\n\
         A version field must be of the form:\n\
         vv.rr, where vv is a 1 or 2 digit version number, and\n\
         rr is a 1 or 2 digit release number.\n"

$ $ trcrpt/lex.c
CAT_TPT_NODFLT        "\
0454-029 No default template (%03x).\n"

CAT_TPT_STACKOVF      "\
0454-030 The total size of the templates has exceeded the\n\
         internal memory capacity of trcrpt.\n"

CAT_TPT_UNDEFFMT      "\
0454-031 Undefined format code %1$s in line %2$d.\n\
         The code will be interpreted as a string.\n"

$ main.c

CAT_TPT_CHAN2BIG         "\
0454-033 Channel %1$d too big.\n\
         The maximum channel number is %2$d.\n"

$ parseutil.c
CAT_TPT_FMTCODE       "\
0454-035 The format code %1$c not allowed for the LOOP statement.\n"

$ sym.c
CAT_TPT_SYMBOLS        "\
0454-036 The register %1$s exceeds the maximum count of %2$d.\n"

$ updt.c
CAT_UPT_MAGIC    "\
0454-061 Expecting '%1$s' on the first line of the update file.\n"

CAT_UPT_EOL      "\
0454-062 Unexpected End of Line in the + or - template.\n"

CAT_UPT_FIELDS   "\
0454-063 There are too many fields on the delete template.\n"

CAT_UPT_EXPPM    "\
0454-064 Expecting + or - at the beginning of the line.\n\
         This could mean that the beginning of this template is wrong,\n\
         or that the previous template is missing a '\\' continuation\n\
         character.\n"

CAT_UPT_SC       "\
0454-065 Errors were detected in the update file %1$s.\n\
         No update to %2$s made.\n"

CAT_UPT_EARLIER   "\
0454-066 Cannot update traceid %1$03X to an earlier version.\n\
         %2$d.%3$d vs. %4$d.%5$d.\n"

CAT_UPT_TRACEID   "\
0454-067 Traceid %1$03X is not in the file %2$s. Cannot delete.\n"

CAT_UPT_UNXPEOF   "\
0454-069 Unexpected end of file condition was detected during the\n\
         read of template file. This could mean that a temporary file\n\
         was deleted from the system. Re-run the command.\n"

CAT_TRC_STPIACT   "\
0454-070 Cannot %1$s an interactive trace session.\n\
         Use the trcoff trace sub-command to stop tracing.\n"

$ $ This dummy message starts the numbering at 512.
512          "YES"
513          "NO"
514          "Warning"
515          "Line"

CAT_TRC_ROOTUSAGE "\
0454-071 permission denied: must have root authority.\n"

CAT_TRC_SESS	"\
0454-072 The trace daemon is currently active.  Only one trace session\n\
         may be active at a time.\n"


CAT_TPT_USAGE4 "\
timestamp   0    0=elapsed+delta, 1=short_elapsed, 2=microseconds, 3=none\n\
pagesize    0    0-500  0=no page breaks,  n=lines between column headers\n\
ids         on   on=print trace id, off=none\n\
exec        off  on=print exec pathnames, off=none\n\
pid         off  on=print process id, off=none\n\
svc         off  on=print current svc, off=none\n\
2line       off  on=use two lines per event, off=use one line\n\
starttime   -    starting time in seconds\n\
endtime     -    ending time in seconds\n\
hist        off  on=generate a histogram of hook\n\
tid         off  on=print thread id, off=none\n\
cpuid       off  on=print cpu id, off=none\n"

$ $ trcrpt/getevent.c
CAT_TPT_BUFOVRFLW "\
0454-075 Event Buffer size of 4K exceeded,\n\
	 last %1$d bytes in event may be incorrect.\n"

$ main.c
CAT_TPT_DATE_2    "\
0454-032 Bad format for date string: '%1$s'\n\
         A date string must be of the form:  mmddhhmmssyy , where\n\
         mm is a 2 digit month,\n\
         dd is a 2 digit day of the month,\n\
         hh is a 2 digit hour,\n\
         mm is a 2 digit minute,\n\
         ss is a 2 digit second,\n\
         yy is a 2 digit year.\n"

CAT_TPT_USAGE5 "\
usage: trcrpt -h -r -j -d idlist -k idlist -p pidlist -t tmpltfile \n\
      -s startdate -e enddate -c  -DLvq -T tidlist -x -n namelist \n\
	   -O optlist logfile(s)\n\
Write formatted report of logfile to stdout\n\
\n\
-h           Omit heading and footing\n\
-r           Output unformatted trace entries in length(4)|data format\n\
-j           Output the list of trace ids, version numbers, and primary label\n\
-d idlist    Output trace entries only for ids in 'idlist'\n\
-k idlist    Exclude trace entries in 'idlist'\n\
-p pidlist   Output only pids (or execnames) in list. INTR = interrupts\n\
-t tmpltfile Use 'tmpltfile' as template file instead of /etc/trcfmt\n\
-s startdate Select entries later than date. (MMddhhmmssyy)\n\
-e enddate   Select entries earlier than date. (MMddhhmmssyy)\n\
-c           Check the template file for syntax errors. No report\n\
-D           Output debug information to file Btrace\n\
-L           Generate list file to %s. Requires -c flag.\n\
-v           Print filenames as they are opened. -vv gives additional info\n\
-q           Suppress detailed output of syntax error messages\n\
-T tidlist   Exclude or include trace entries in tidlist \n\
-x           Print executable name for every event\n\
-n namelist  Specify the kernel name list file to be used to interpret\n\
             addresses\n\
-O optlist   -O option=value, ... (-O help for additional info)\n\
\n\
The -s and -e flags are exclusive of the -O starttime and endtime.\n\
If logfile is unspecified, trcrpt uses the default logfile\n\
%s\n"

CAT_NO_TRACE    "\
0454-076\n\
No trace data available in this dumpimage,\n\
because the trace session was not active at the time of the dump.\n"

CAT_TRCNM_USAGE	"\
usage:\n\
trcnm -a Filename\n\
trcnm -K Symbol1 ...\n\
\n\
Write a kernel name list to standard output.\n\
\n\
-a          Print all loader symbols.  Default is system calls only.\n\
-K Symbol1  Print the value for the specified symbols via knlist\n\
            system call.\n\
\n\
If the Filename is not specified, the default Filename is %s.\n"

CAT_TPT_FMTFUNC "\
0454-077\n\
The function %s is not defined.\n"

