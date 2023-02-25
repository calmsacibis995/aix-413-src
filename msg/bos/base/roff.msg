$ ENGLISH_SOURCE_VERSION= 1.8.1.3
$ @(#)97	1.1  src/msg/bos/base/roff.msg, bosmsg, msg411, GOLD410 2/9/94 09:15:08
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: roff.msg
$
$ ORIGINS: 3,27,28
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$set MS_ROFF
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "

Q_OPT_ERR       "1004-001 The -q flag is ignored.\n"
EXC_MACROS      "%s: 1004-002 Specify no more than 5 macro packages.\n"
NO_DEBUG        "1004-003 DEBUG is not enabled\n"
$ Do not translate DEBUG.
BAD_OPT         "1004-004 The %s is an option that is not valid.\n"
LOOP_ERR        "1004-005 The number of recursive macros must not be greater than 20.\n"
CREAT_TMP       "1004-006 Cannot create temp file.\n"
LINE_FILE       " line %1$d, file %2$s"
PB_OFLOW        "1004-008 There is a pushback overflow error.\n"
CPB_OFLOW       "1004-009 There is a cpushback overflow error.\n"
CANT_OPEN       "1004-010 Cannot find or open %s\n"
EXC_PGNR        "1004-011 Too many page numbers are specified with -o flag\n"
EXC_HEXESC      "1004-012 The hex escape sequence must be 4 bytes.\n"
BAD_HEXESC      "1004-013 The hex escape sequence must be 4 bytes.\n"
BAD_NLESC	"1004-014 The NLesc sequence must be 2 bytes.\n"
UNK_HEXESC      "1004-015 The <%c> is not a valid character for a hex sequence.\n"
BAD_PIPE        "1004-016 Cannot create pipe.\n"
BAD_EXEC        "1004-017 Cannot exec %s.\n"
$ Do not translate exec.
EXC_STRING      "1004-018 Specify no more than %d string/macro names.\n"
EXC_TEMP        "1004-019 The temporary file space has been exceeded.\n"
BAD_ALLOC       "1004-020 There is not enough memory available now.\n"
EXC_MEMORY      "1004-021 There is not enough memory available now.\n"
SETBRK_INT      "1004-022 internal error in setbrk: i=%1$x, j=%2$d, k=%3$x\n"
EXC_MACARG      "1004-023 The macro argument is too long.\n"
EXC_DIVNEST     "1004-024 Specify no more than 5 nested diversions.\n"
$ Do not translate diversions.
PM_DATA         "pm: total %$1d, macros %$2d, space %$3d\n"
EXC_NUMREG      "1004-026 Specify no more than %d number registers.\n"
DIV_ZERO        "1004-027 Cannot divide by zero.\n"
BAD_TRAP        "1004-028 Cannot set page trap.\n"
$ Do not translate page trap.
USER_ABORT      "1004-029 The user is quitting.\n"
CANT_EV         "1004-030 Cannot do environment switching.\n"
IF_OFLOW  "1004-031 Specify no more than 15 nested if-else constructs.\n"
EXC_STR_COMP    "1004-032 Cannot compare strings over 1280 bytes.\n"
LINE_OFLOW      "1004-033 There is line overflow.\n"
BOTCH_TRAP      "1004-034 Internal error: cannot process page trap.\n"
WORD_OFLOW      "1004-035 There is a word overflow.\n"
EXC_HYPH        "1004-036 The exception word list is full.\n"
ZERO_WID        "1004-037 The field width is zero or two tabs have the same position.\n"
BAD_TERMTAB     "1004-038 Cannot find or open %s.\n"
BAD_DEVTAB      "1004-039 Cannot find or open the device table.\n"
EXC_CHARSET     "1004-040 There are too many names in charset for %s\n"
$ Do not translate charset.
BAD_DESC        "1004-101 Cannot find or open tables for %s\n"
EXC_SPCLCHAR    "1004-102 There are too many special characters in file %s\n"
BAD_FONTP       "1004-103 The %d font position must not be greater than 10.\n"
BAD_FONTNM      "1004-104 Specify a valid font name.\n"
BAD_FONTFILE    "1004-105 Cannot find or open %s\n"
BIG_FONTFILE    "1004-106 Font %1$s is too big for position %2$d\n"
MALFORMED		"1004-107 POSSIBLE INTERNAL MACRO ERROR: Malformed statement.\n"
MISSING_SM		"1004-108 POSSIBLE INTERNAL MACRO ERROR:\n\tMissing set number or message number.\n"
MISSING_DQ		"1004-109 POSSIBLE INTERNAL MACRO ERROR:\n\tMissing double quotes around default message.\n"
NO_KANJI_AF	"1004-111 Cannot use 'k' number register format on this output\n\tdevice, using '1' format instead.\n"
NO_KANJI_KS	"1004-112 Adjustment mode 'k' has no effect on this output device.\n"
NO_KANJI_SET	"1004-113 The name of the output codeset is unrecognized.\n"
BAD_ARG		"1004-114 Argument is not valid for %s.\n"
$ Next two messages (012 and 013) are updates of messages above
HEXESC_EXC	"1004-012 Hex escape sequence too long.\n"
HEXESC_BAD	"1004-013 Hex escape  has odd number of digits.\n"
BAD_MBLEN	"1004-115 Error from call to mblen.\n"
BAD_3BYTEDATA	"1004-116 Invalid 3 byte data.\n"
EXC_FONT	"1004-117 nfonts must be less than %d in file %s.\n"
BAD_PAPERSIZE "1004-118 The paper size specified with the -M option is not supported.\n"
PAPERSIZES "Supported paper sizes include (Letter, Legal, A4, B5, Executive, A5).\n"
$ Do not translate (Letter, Legal, A4, B5, Executive, A5)
$
INVALID_CHAR	"1004-119 Input data has an invalid character in the locale %s.\n"
