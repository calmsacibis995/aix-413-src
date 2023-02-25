$ @(#)83        1.12.1.2  src/bos/usr/sbin/sar/sar.msg, cmdstat, bos41J, 9521B_all 5/23/95 02:51:39
$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$ ORIGINS: 27 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ LEVEL 1, 5 Years Bull Confidential Information
$
$quote "
$set MS_SAR

$ Do not translate sar, sag, timex in this file

ABNORMALEXIT    "Command terminated abnormally.\n"

$ Old message, not used.
$ ARGSTN          "sar: 0551-200 args t & n <= 0"


COMPAT		"Versions of sadc and sar are inconsistent. \n"

BADOPT		"Option P is inconsistent with input file. \n"


INV_INTERVAL    "sar: 0551-200 Invalid interval length specified.\n"

INV_NUMBER      "sar: 0551-214 Invalid number of intervals specified.\n"

AVERAGE         "Average"

CANTOPEN        "sar: 0551-201 Cannot open %s.\n"

CANTPARSE       "Cannot parse  %s:\nNeed odd number of parameters.\n"

CHARSBLOCKS     "\nCHARS TRNSFD = %1$ld\nBLOCKS READ  = %2$ld\n"

DNF             "%s: Data does not exist.\n"

EMPTYSTACK      "sar: 0551-202 The stack is empty.\n"

ETIME           "sar: 0551-203 ending time (etime) <= starting time (stime)"

FULLSTACK       "The stack is full.\n"

IGNORED         "%s is ignored.\n"

ILLEGAL         "sar: 0551-204 %1$s is not a valid option of the %2$c flag\n"

MISSCOM         "timex: 0551-205 Specify a command."

MTOX            "sar: 0551-206 Cannot specify more than one x-axis value:\n%s\n"

NODATA          "sar: 0551-207 There is no sar data.\n"

NORECS          "\ntimex: 0551-208 No process records found.\n"
$ There is a blank line in front of this message.

OFILE           "sar: 0551-209 Cannot specify the same file on the -o and -f flags."

REAL            "real"

REDUCE          "sar: 0551-210 Cannot reduce %s.\n"

RESTART         "\tOperating System is restarting.\n"

$ Do not translate sag and hh[:mm[:ss]]
SAGUSAGE1  "Usage: sag [-s hh[:mm]] [-e hh[:mm]] [-f file] [-i seconds] \n"
SAGUSAGE2  "           [-x \"spec\"] [-y \"spec[;spec]...\"] [-T term] \n\n"
SAGUSAGE3  "\t where Spec has the form  Name [Op Name]...[Lo Hi]\n"
SAGUSAGE4  "\t and Name is a string matching a column header in sar command.\n"

$ Do not translate sar and hh[:mm[:ss]] 
SARUSAGE1  "Usage: sar { -A | [-a][-b][-c][-k][-m][-q][-r][-u][-v][-w][-y] } \n"
SARUSAGE2  "           [-s hh[:mm[:ss]]] [-e hh[:mm[:ss]]] \n"
SARUSAGE3  "           [-P processor_id[,...] | ALL] \n"
SARUSAGE4  "           [-f file] [-i seconds] [-o file] [interval [number]] \n"

SYS             "sys "

TIMECHG         "sar: 0551-211 Specify a positive integer for the time change.\n"

TIMEXUSE        "Usage: timex [-s][-o][-p[-fhkmrt]] cmd"
$ Do not translate timex

TRYAGAIN        "timex: 0551-212 Try again.\n"

USER            "user"

TRYRUNNING  "sar: 0551-213 Try running /usr/lib/sa/sa1 <increment> <number>\n"
$ Do not translate /usr/lib/sa/sa1

BADOPTION	"With option P, only options acmuw are accepted. \n"
$ Do not translate acmuw
