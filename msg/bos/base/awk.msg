$ ENGLISH_SOURCE_VERSION= 1.12.1.1
$ @(#)92   1.2  src/msg/bos/base/awk.msg, bosmsg, msg411, 9431A411a 8/1/94 06:32:41
$ 
$ COMPONENT_NAME: (CMDEDIT) awk.msg
$
$ FUNCTIONS: none
$
$ ORIGINS: 3, 10, 27
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

$
$ messages for AWK
$

$set MS_AWK
$      Do not translate awk.
$      Do not translate %s, %c, %d, etc. These will be replaced by file names,
$      commands, numbers, etc.
$      The term "regular expression" is a unix technical term.
$      Some messages begin with a space. Please leave space in the translation.
$      Please do not add new lines that are not included in the source.
$quote "


YBAIL        "awk: 0602-500 Quitting"
YZERONULL    "awk: 0602-501 $zero&null"
$      Do not translate -- this is a debug message
YILGLSTMT    "awk: 0602-502 The statement cannot be correctly parsed."
YILLNXT      "awk: 0602-503 A next statement cannot be used inside a function."
YIDXNREXP    "awk: 0602-504 Regular expressions cannot be used with the index function."
YARRNFUNC    "awk: 0602-505 The %s array cannot be used as a function."
YFUNCREDEF   "awk: 0602-506 The %s function cannot be defined more than once."
XLEXUSG      "awk: 0602-507 lex %1$s [%2$s]\n"
$       Do not translate -- this is a debug message
XSTRTOOLONG  "awk: 0602-508 The string or regular expression beginning with\n\
\tthe characters \"%.10s\" cannot contain more than 399 bytes."
XILLNSTFN    "awk: 0602-509 Functions cannot contain other functions."
XILLRET      "awk: 0602-510 A return statement must be inside a function."
XNLINRE      "awk: 0602-511 Regular expression %.10s cannot contain a newline."
XNLINSTR     "awk: 0602-512 The string %.10s cannot contain a newline character."
BNORMFA      "awk: 0602-513 There is not enough memory available for the finite automaton."
$       Do not translate finite automaton.
BSPCMKDFA    "awk: 0602-514 There is an internal software error in the makdfa function."
$       Do not translate makdfa.
BSPCMKINIT   "awk: 0602-515 There is an internal software error in the makeinit function."
$       Do not translate makeinit.
BUNKTYPPNTR  "awk: 0602-516 The type %d in the penter function is not known."
$       Do not translate penter.
BUNKTYPFRTR  "awk: 0602-517 The type %d in the freetr function is not known."
$       Do not translate freetr.
BUNKTYPCFOIL "awk: 0602-518 The type %d in the cfoll function is not known."
$        Do not translate cfoll.
BUNKTYPFRST  "awk: 0602-519 The type %d in the first function is not known." 
BCHRCL2BIG   "awk: 0602-520 A regular expression bracket expression\n\
\tcannot contain more than 1,024 bytes."
BERRINRE     "awk: 0602-521 There is a regular expression error."
BSPCTOSTRING "awk: 0602-522 There is an internal software error in the tostring function."
$       Do not translate tostring.
BRE2BIG      "awk: 0602-523 Regular expression %s cannot contain more than 1,024 bytes."
BFOLSETOFLW  "awk: 0602-524 There is an internal software error in the cfoll function.\n\
\tFollow set overflow."
BSPCPMTCH    "awk: 0602-525 There is an internal software error in the pmatch function."
$       Do not translate pmatch.
BNOSTATSPC   "awk: 0602-526 There is not enough memory available now."
BEMPTYRE     "awk: 0602-527 A regular expression cannot be empty."
BSNTXRE      "awk: 0602-528 There is a usage error in regular expression %1$s at %2$s"
BILLPRMRYRE  "awk: 0602-529 The regular expression %1$s contains\n\
\ta primary character that is not correct at %2$s"
BNONTERMCHCL "awk: 0602-530 The regular expression character class %s\n\
\tis not correctly formed with the [ and ] characters."
BNLINCHRCL   "awk: 0602-531 The regular expression character class %s\n\
\tcannot contain a newline character."
BNOSPCCGOTO  "awk: 0602-532 There is an internal software error in the cgoto function."
$       Do not translate cgoto.
LOPEN        "awk: 0602-533 Cannot find or open file %s."
LIREC2LONG   "awk: 0602-534 Input line %.20s cannot be longer than 10,239 bytes."
L2MNYFLDS    "awk: 0602-535 Line %.20s cannot have more than 99 fields."
LCRT2MANYFLDS "awk: 0602-536 Cannot create more than 99 fields."
LGIANTREC    "awk: 0602-537  A single giant record was built\n\
\tstarting at `%.20s...'\n"
LFLDACC      "awk: 0602-538 The field %d must be in the range 0 to 99."
LSRCLINE     " The source line is %d."
LINFUNC      " The function is %s."
LFPE         "awk: 0602-539 A floating point exception exists."
LMSNG        "\tawk: 0602-540 There is a missing %c character.\n"
LDMSNG       "\tawk: 0602-541 There are %1$d missing %2$c characters.\n"
LXTRA        "\tawk: 0602-542 There is an extra %c character.\n"
LDXTRA       "\tawk: 0602-543 There are %1$d extra %2$c characters.\n"
LIRECNUM     " The input line number is %g."
LFILENAME    " The file is %s."
LSRCLINENO   " The source line number is %d.\n"
$       This line is displayed after the MUSAGE message.
LCNTXT       " The error context is\n\t\t"
$       This line is displayed after the LSRCLINENO message.
$       The \t\t at the end is required to place the >>> <<< which surrounds
$       the error line as posted on the next line by the eprint function.
LEDOM        "awk: 0602-544 The %s parameter to a math library function is not in the domain."
LERANGE      "awk: 0602-545 The %s return value of a math library function is out of range."
MUSAGE       "Usage: awk [-F Character][-v Variable=Value][-f File|Commands][Variable=Value|File ...]\n"
MOPEN        "awk: 0602-546 Cannot find or open file %s." 
PNODEALLOC   "awk: 0602-547 Internal software error in the nodealloc function."
$       Do not translate nodealloc.
PFUNCNOTARR  "awk: 0602-548 Use %s as a function and not as an array."
PARRANDFUNC  "awk: 0602-549 %s cannot be an array name and a function name."
RILLSTMT     "awk: 0602-550 A statement occurred that is not valid."
RUNEXPBT     "awk: 0602-551 A break, continue, or next statement is\n\
\tnot in the correct location."
RUNEXPEX     "awk: 0602-552 A break or next statement is\n\
\tnot in the correct location."
RUNDEFFN     "awk: 0602-553 Function %s is not defined."
RFNASARG     "awk: 0602-554 Function %s cannot be used as a parameter."
RFNNESTDP    "awk: 0602-555 Function %s cannot contain more than 99\n\
\tlevels of nesting."
RARGMSG      "awk: 0602-556 Supply parameter number %1$d of function %2$s."
RILLJMPTYP   "awk: 0602-557 Jump type %d is not correct."
RNOTARR      "awk: 0602-558 %s cannot be used as an array."
RUNKBOOL     "awk: 0602-559 The boolean operator %d is not known."
RUNKRELOP    "awk: 0602-560 The relational operator %d is not known."
RNOTEMPSPC   "awk: 0602-561 There is not enough memory available now."
RILLFLD      "awk: 0602-562 Field $(%s) is not correct."
RCONVSUPP    "awk: 0602-563 %%*... conversion is not supported in %s.  Try concatenation."
$       Do not translate the % signs at the beginning of the message.
RFMTLNG      "awk: 0602-564 Format item that begins %.20s cannot be longer than 10,239 bytes."
R2FEWARGS    "awk: 0602-565 There are not enough parameters in printf statement %s."
$       Do not translate printf.
R0DIV        "awk: 0602-566 Cannot divide by zero."
RMOD0DIV     "awk: 0602-567 Cannot divide by zero when using the modulo operator."
$       Modulo is an arithmetic operator like addition or subtraction.
RILLARITH    "awk: 0602-568 Arithmetic operator %d is not correct."
RREADVAL     "Cannot read the value of"
$       This gets handed to message TCANTFN/TCANTARR as %1s$
RDIV0SLEQ    "awk: 0602-569 Cannot divide by zero when using the /= operator."
RDIV0MODEQ   "awk: 0602-570 Cannot divide by zero when using the %%= operator."
$      Do not translate the % signs in the message. 
RILLASGN     "awk: 0602-571 Assignment operator %d is not correct."
RILLSPLT     "awk: 0602-572 Internal software error in the split function."
RNOTARRAY    "awk: 0602-573 %s must be an array."
RILLFNTYP    "awk: 0602-574 Function type %d does not exist."
ROPEN        "awk: 0602-575 Cannot find or open file %s." 
RNULLFILENAME "awk: 0602-576 A print or getline function must have a file name."
$      Do not translate getline or print.
RTOOMANY     "awk: 0602-577 Cannot open file %s.\n\
\tA maximum of 15 files can be open at one time."
RILLREDIR    "awk: 0602-578 The redirection operator was not used correctly."
RSUBTOOBIG   "awk: 0602-579 The result %.20s of the sub function\n\
\tcannot be longer than 10,239 bytes."
RTGTSTR      "awk: 0602-580 target string: %s, *patbeg = %o, patlen = %d\n"
$       Do not translate -- this is a debug message
RMATCHFND    "     match found: "
$       Do not translate -- this is a debug message
RGSUB2BIG    "awk: 0602-581 The result %.20s of the gsub function\n\
\tcannot be longer than 10,239 bytes."
$      Do not translate gsub.
TZERONULL    "awk: 0602-582 $zero&null"
$       Do not translate -- this is a debug message
TSPCMKSYMTAB "awk: 0602-583 Internal software error in the makesymtab function."
$      Do not translate makesymtab.
TSYMTABOFLW  "awk: 0602-584 Internal software error in the symbol table at %s."
TSPCREHASH   "awk: 0602-585 Internal software error in the rehash function."
TASGNTO      "Cannot make an assignment to"
$      This gets handed to message TCANTFN/TCANTARR as %1s$
TCANTARR     "awk: 0602-586 %1$s %2$s. It is an array name."
TCANTFN      "awk: 0602-587 %1$s %2$s. It is a function."
TFUNNYVAR    "awk: 0602-588 Unexpected variable %o: n=%s s=\")%s\" f=%g t=%o"
TILLREF      "awk: 0602-589 The reference to array %s is not valid."
TREADVAL     "Cannot read the value of "
$      This gets handed to message TCANTFN/TCANTARR as %1s$
TSPCTOSTR   "awk: 0602-590 Internal software error in the tostring function on %s." 
TSTR2LNG    "awk: 0602-591 String %.10s cannot be longer than 399 bytes."
TNLINSTR    "awk: 0602-592 String %.10s cannot contain a newline character."
BSPCEXSTR   "awk: 0602-593 There is an internal software error in the expand_str function."
$       Do not translate expand_str.
BSPCMKPREG    "awk: 0602-594 There is an internal software error in the mkpreg function."
$       Do not translate mkpreg.
ILLMBCHAR    "awk: 0602-595 There is an illegal character in the input file.\n"
FSBAD	     "awk: Field separator $(%s) is not correct."
