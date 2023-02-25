$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)74	1.1  src/msg/bos/base/adf.msg, bosmsg, msg411, GOLD410 2/9/94 08:46:00
$
$   COMPONENT_NAME: CMDADF
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1993
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$
$set ADF_MSG
$quote "
$
$
$ errors from main.c
$
$
CARD_ID    "0460-001: Invalid adapter card id \"%s\".\n"
DEV_OPEN   "0460-002: Cannot open device \"%s\".\n"
FILE_OPEN  "0460-003: Cannot open file \"%s\".\n"
AOPT_MISS  "0460-004: -a option requires argument - adapter name\n"
DOPT_MISS  "0460-005: -d option requires argument - device name\n"
FOPT_MISS  "0460-006: -f option requires argument - file name\n"
MAX_FILES  "0460-007: Warning: Too many files, file \"%s"\ will not be processed.\n"
FILE_OPT   "0460-008: -f option cannot be specified with -d option.\n"
INV_ARG    "0460-010: Invalid argument \"%s\".\n"
INV_ARG_C  "0460-011: Invalid argument \"%c\".\n"
ADF_NONE   "0460-012: No valid ADF input file can be found.\n"
ZERO_ERRS  "0460-013: No errors detected in adf.\n"
ONE_ERR    "0460-014: One error detected in adf.\n"
MANY_ERRS  "0460-015: %d errors detected in adf.\n"
USAGE_ERR  "0460-016: \
usage : \n\
adfutil [ -c XXXX | -a adapter ][ -d device | -f file ][ -q ][ -m [file(s)] ]\n\
\t-a : adapter name \n\
\t-c : PS/2 hex cardid \n\
\t-d : device specification (default: /dev/fd0) \n\
\t-f : filename of adf on filesystem, cardid must match if specified\n\
\t-m : microcode only, must use -c option, -a option, or specify file(s)\n\
\t-q : quiet mode \n"

YY_IN      "0460-017: Close file error - fatal.\n"

$
$
$ errors from file.c
$
$
$ from function yyerror

YY_ERR     "0460-020: %s, line %3d: %s\n"

$
$ errors from dossup.c
$
$
$ from function getdosdir
$

ADF_NUMS   "0460-030: \tFound %s file.\n"

$
$ from function getdosfile
$
DOS_INIT   "0460-040: adfutil [getdosfile:dosinit] device(%s) doserrno(%d)\n"
DOS_NONE   "0460-041: adfutil [getdosdir] no files found, file(%s).\n"
DOS_SOME   "0460-042: adfutil [getdosdir] (%d) files found, file(%s).\n"
DOS_ERR    "0460-043: [adfutil] An error occured trying to read \"%s\" from \"%s\". Error = %d\n"
DOS_STAT   "0460-044: ERROR: wait on [getdosfile] Child(%d) errno(%d).\n"
DOS_STAT2  "0460-045: NOTICE: [getdosfile] Child(%d) exited status(%#X).\n"
$
$
$ errors from gen.c
$
$ function geninit
$

ODM_INIT   "0460-050: *** FAIL: initialize_odm()\n"
MAL_FAIL   "0460-051: *** FAIL: malloc()\n"
DATA_FAIL  "0460-052: *** FAIL: database error (search %s)\n"
MULT_OBJS  "0460-053: *** ERROR: search (%s) returned %d objects.\n"

$
$ from function processTrans
$

ADD_OBJ    "0460-060: *** FAIL[processTrans]: add_obj (attr = %s)\n"
PROC_CON   "0460-061: *** Processing continued.\n"
REM_OBJ    "0460-062: *** ERROR[processTrans]: rm_obj (attr = %s)\n"

$
$ from function genMicro
$

MIC_ERR    "0460-070: Warning : Cannot process microcode files with the -f option.\n"
MIC_TRANS  "0460-071: Cannot transfer %s to %s from device %s.\n"
MIC_TRANS_OK "0460-072: Successful transfer of %s\n"

$
$ from function genFunction
$

BUFF_FULL  "0460-080: (genFunction)BUFFER FULL : %s not added to values.\n"

$
$
$ errors from scan.c
$
$
$ from function main
$

SCAN_ERR "0460-090: scan: file error.\n"
SCAN_ERR2 "0460-091: scan: cannot open file %s.\n"

$
$
$ errors from util.c
$
$ function _mount
$

UTIL_IDISK "Insert diskette for drive %s and strike\nany key when ready\n"
$ %s is the name of a device such as "/dev/fd0".

$
$
$ errors from devio.c
$
$ functions _devio and _devio1
$

DEVIO_NOTREADY        "Not ready error during %1$s on drive %2$s.\n"
$ %1$s is the name of a system call.  (such as "open()" or "write()")
$ %2$s is the name of a diskette device.  (such as "/dev/rfd0")

DEVIO_DEVBUSY         "Device busy error during %1$s on drive %2$s.\n"
$ %1$s is the name of a system call.  (such as "open()" or "write()")
$ %2$s is the name of a diskette device.  (such as "/dev/rfd0")

DEVIO_DATA            "Data error during %1$s on drive %2$s.\n"
$ %1$s is the name of a system call.  (such as "open()" or "write()")
$ %2$s is the name of a diskette device.  (such as "/dev/rfd0")

DEVIO_WRIPROT         "Write protect error during %1$s on drive %2$s.\n"
$ %1$s is the name of a system call.  (such as "open()" or "write()")
$ %2$s is the name of a diskette device.  (such as "/dev/rfd0")

DEVIO_GENFAIL         "General Failure error during %1$s on drive %2$s.\n"
$ %1$s is the name of a system call.  (such as "open()" or "write()")
$ %2$s is the name of a diskette device.  (such as "/dev/rfd0")

DEVIO_ABORT           "Abort, Retry, Ignore?"

$
$
$ errors from semafore.c
$
$ function Xerror
$

SEM_FATAL "XXX FATAL ERROR: line %1$d, file %2$s: "
$ %1$d is a line number.   %2$s is a file name.
$ This message is followed by an error message from the perror() subroutine.

