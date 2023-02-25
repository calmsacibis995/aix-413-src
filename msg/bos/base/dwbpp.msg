$ ENGLISH_SOURCE_VERSION= 1.10
$ @(#)82        1.1  src/msg/bos/base/dwbpp.msg, bosmsg, msg411, GOLD410 2/9/94 08:57:36
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting Services
$
$ FUNCTIONS:
$
$ ORIGINS: 3,10,27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$	DWB preprocessor message file
$	One common file serves tbl, eqn, pic, and grap because there are
$	many common messages
$
$set MS_DWBPP
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}
$quote "
$
$ Pic messages
$
CNTXT	"pic: The context is\n"
NEARFN	" near line %1$d, file %2$s\n"
$ refers to the line number and file name. The message CNTXT and NEARFN are
$ are portion of a complete message.
ARGDOM	"pic: 1003-303 The input value to %s is  either a negative or zero value.\n"
$ refers to a math function being called either log10() or sqrt()..
BADRAD	"pic: 1003-304 The radius %2$g for the %1$s is not valid.\n"
$ refer to either circle|ellipse and the radius value.
RESRNG	"pic: 1003-305 The %s has resulted in an overflow condition. \n"
$ refers to a math function being called exp()..
EDEFNAM	"pic: 1003-306 The %s must be specified as either a define or as a name.\n"
$ Do not translate define.
EVARDEF	"pic: 1003-307 The %s is used as a variable and a define\n"
$ Do not translate define.
NOOPN2	"%1$s: 1003-308 Cannot find or open %2$s.\n"
$ refer to the command name and file to be opened.
NOTBLK	"pic: 1003-309 %s is not in that block.\n"
$ Do not translate block.
PSINPS	"pic: 1003-310 A .PS command has been found within a .PS/.PE construct\n"
$ Do not translate the .PS and .PE refers to start and stop pic macros.
BRAKNST	"pic: 1003-311 There are more than 20 nested blocks.\n"
$ Do not translate block.
ARGFU	"pic: 1003-312 A macro has been used in a way that is not valid.\n"
ARGDEEP	"pic: 1003-313 The parameters are too deep.\n"
NOGROW	"pic: 1003-314 Cannot increase the size of %1$s to %2$d.\n"
$ refer to the name of the variable and size.
IMPOSS	"pic: 1003-315 x,y=%1$g,%2$g are not valid coordinates.\n"
$ refer to the x and y axis.
NOOPN1	"pic: 1003-316 Cannot find or open file %s.\n"
$ refers to the file to be opened.
DLSDEFN	"definition"
$ Do not Translate definition.
BADODEF	"pic: 1003-318 The define must have a left parenthesis.\n"
$ Do not translate define.
DIVZERO	"pic: 1003-319 Cannot divide  by 0.\n"
DLSELSE	"else part"
$ Do not Translate else part.
EOF4	"pic: 1003-321 There is an end of file in %1$s %2$c %3$.20s... %4$c.\n"
EOFGTAR	"pic: 1003-322 Have reached end of file in getarg().\n"
$ Do not translate getarg().
EOFPS	"pic: 1003-323 The end of file has been reached within .PS/.PE constructs.\n"
$ Do not translate the .PS and .PE refers to start and stop pic macros.
FLTEXCP	"pic: 1003-324 There is a floating point exception.\n"
FORNEST	"pic: 1003-325 Cannot nest for-loops more than 10 levels deep.\n"
$ A "for-loop" is a control statement using the "for" command. 
FORPOP	"pic: 1003-326 An internal error has occurred during for-loop processing.\n"
$ A "for-loop" is a control statement using the "for" command. 
INPNEST	"pic: 1003-327 The inputs are nested too deep."
DLSLOOP	"loop body"
$ Do not translate loop body.
MODDV0	"pic: 1003-329 Cannot divide by 0 for the modulo operation.\n"
NLINSTR	"pic: 1003-330 There is a newline in the string.\n"
NOPLACE	"pic: 1003-331 There is no such place as %s.\n"
NOVAR	"pic: 1003-332 There is no such variable as %s.\n"
NOBJSWR	"pic: 1003-333 nobjs wrong%1$d %2$d\n"
$ Do not translate
OUTMKND	"pic: 1003-334 There is not enough memory available now.\n"
OUTTOST	"pic: 1003-335 There is not enough memory available for the %s string.\n"
OUTSYTS	"pic: 1003-336 There is not enough memory available in the symbol table for %s.\n"
PSHRNK	"pic: 1003-337 The  %1$g X %2$g picture is being resized to  %3$g X %4$g.\n"
$ The X stands for implied multiplication when giving dimensions (in inches).
POPWRD	"pic: 1003-338 pop incorrect input %d\n"
$ Do not translate message
PSHOVF	"pic: 1003-339 There is a pushback overflow.\n"
PSHBADT	"pic: 1003-340 Pushed a bad type %d.\n"
$ Do not translate message
LONGSTR	"pic: 1003-341 The string must be no longer than 500 characters.\n"
SYNTAX	"pic: 1003-342 The syntax is not valid.\n"
DLSTHEN	"then part"
$ Do not translate
NODTH	"pic: 1003-344 There is no %dth \n"
NODTHL	"pic: 1003-345 There is no %dth last\n"
NOSUCH	"pic: 1003-346 There is no .%s in that []\n"
DLSTHRU	"thru body"
$ Do not translate message
MANYINP	"pic: 1003-348 Too many inputs popped\n"
UNXEFDT	"pic: 1003-349 Did not expect the end of file in the do_thru.\n"
$ Do not tranlate do_thru.
PICUSG	"Usage: %s [-D] [-T Term] [--] [-] [File] ...\n"
BRCNEST	"pic: 1003-351 There are more than 20 nested {...}.\n"
PIC_DELIM "pic: 1003-352 Delimiters must be printable ASCII characters\n"
$ All delimiters must be in the C locale
PIC_BADREAD "pic: 1003-353 Invalid character in input\n"
$ The last character read is not defined for the current locale.
PIC_FATAL "pic: 1003-354 A call to mbtowc() has failed unexpectedly\n"
$ The string 'mbtowc()' should not be translated. This message should
$ never happen, but is included for completeness.
PIC_UNPUT "pic: 1003-355 Attempt to unput an invalid character\n"
$ Internal fatal error related to multibyte code sets. This should never
$ happen.
$
$ Tbl Messages
$
BADTARG	"tbl: 1003-001 The %s parameter to the -T flag is not valid. Use -TX.\n"
TBLIGN	"tbl: 1003-002 %1$s: line %2$d: The data on line %3$d of table is ignored.\n"
NOASPN	"tbl: 1003-003 Warning: Cannot span columns of type A. The value is changed to type L.\n"
$ Do translate type A and type L. 
$ standard left alignment 
NONSPN	"tbl: 1003-004 Warning: Cannot span colunms of type N. The value is changed to type C.\n"
$ Do translate type N and type C. 
NOOPN0	"tbl: 1003-005 Cannot find or open file.\n"
NOMEM	"tbl: 1003-006 There is not enough memory available now.\n"
EOFTBL	"tbl: 1003-007 End of file was reached when reading the table specification.\n"
IGN2WID	"tbl: 1003-008 The second width specification is ignored.\n"
ILLOPT	"tbl: 1003-009 The specified option for describing a table is not valid.\n"
MISPOPT	"tbl: 1003-010 A global option is not spelled correctly.\n"
NTFONT	"tbl: 1003-011 There is a missing \\fR.\n"
$ Do not translate \fR.
TOOMCHR	"tbl: 1003-012 Specify no more than 20 characters in the table.\n"
TOOMCOL	"tbl: 1003-013 Too many columns for registers.\n"
TOOMDIV	"tbl: 1003-014 Too many text block diversions.\n"
NOVSPN1	"tbl: 1003-015 Vertical spanning must not be used in the first row.\n"
$ Do not translate vertical spanning.
NOTBLDT	"tbl: 1003-016 There is no data in the table.\n"
NOTBLLN	"tbl: 1003-017 There are no real lines in the table.\n"
LINENO	"tbl: \n%1$s: line %2$d: %3$s"
$ refers to the input file and line number and message. Do not put new line 
$ character in this message.
BADTBSC	"tbl: 1003-019 The table specification character is not correct.\n"
LONGCOL	"tbl: 1003-020 Specify a column width no more than 10 characters.\n"
NODELS	"tbl: 1003-021 Specify delimiter characters.\n"
DTNLST	"tbl: 1003-022 A period must be the last character on the format line.\n"
NO1COLS	"tbl: 1003-023 The first column cannot be of type S.\n"
$ Do not translate type S.
NO1ROWV	"tbl: 1003-024 Vertical spanning must not be used in the first row.\n"
$ Do not translate vertical spanning.
NOSPCH	"tbl: 1003-025 There is not enough memory available now.\n"
NOSPVEC	"tbl: 1003-026 There is not enough memory available now.\n"
NOSPEC	"tbl: 1003-027 There is no specification\n"
PS2BIG	"tbl: 1003-028 The point size is too large.\n"
PSBAD	"tbl: 1003-029 The point size is unreasonable.\n"
TBLQUIT	"tbl: The table program quits.\n"
LONGSPC	"tbl: 1003-031 The format specification line must not be longer than 200 charactes.\n"
MANYCOL	"tbl: 1003-032 There are too many columns in table.\n"
MANYLIN	"tbl: 1003-033 The format specification cannot have more than 44 lines.\n"
MUCHBAK	"tbl: 1003-034 There are too much backup.\n"
WIDENT	"tbl: 1003-035 Cannot widen the table in the .T& command.\n"
$ Do not translate .T& which means new format specification within a table.
UNXEOF	"tbl: 1003-036 Did not expect the end of file.\n"
TBLUSG	"Usage: %s [-TX] [--] [File] ... [-]\n"
$ refers to the command name.
BIGVSP	"tbl: 1003-037 The vertical spacing value is too large.\n"
$
$ The following 5 new tbl messages were added in February 1991 for 3.2 
$ ILS/NLS support. The first is a more general problem which can happen
$ anywhere with multibyte characters. The first message is reported whenever
$ mblen is called and fails.
TBL_BADMBLEN "tbl: 1003-038 Unable to obtain multibyte character length.\n"
$ Only single byte ascii characters should be found in either the global
$ commands affecting the entire table or the column specifications. If a
$ multibyte character is found it is an error.
TBL_MBGCPARAM "tbl: 1003-039 Multibyte characters illegal in global commands.\n"
TBL_MBTSPARAM "tbl: 1003-040 Multibyte characters illegal in column specifiers.\n"
TBL_UNWCTOMB  "tbl: 1003-041 Unable to convert wide character to multibyte form.\n"
TBL_ILLREPEAT "tbl: 1003-042 Multibyte character illegal in repeat sequence.\n"
TBL_TOOWIDE "tbl: 1003-043 Table at line %d file %s is too wide - \\n(TW units\n"
$ Do not translate TW
$
$ Neqn messages
$
BIZDEL1	"neqn: 1003-201 The delimiters at %.20s are not correct or cannot be printed.\n"
INCNIMP	"neqn: 1003-202 The include function is not implemented.\n"
NUNXEINP "neqn: 1003-203 Did not expect the end of input at %.20s.\n"
NUNTDEF	"neqn: 1003-204 The definition at %.20s does not have an ending delimeter.\n""
NEQNUSG	"Usage: %s [-d Delimeter1Delimeter2] [-p Number] [-s Number] [-f Number] [File ...]\n"
NNOOPN1	"neqn: 1003-205 Cannot find or open file %s.\n"
$ refers to the file to be opened.
NLONGCTK "neqn: 1003-206 The changed token %.25s cannot be longer than 400 characters.\n"
EQNFAT	"neqn: fatal error: "
FATERR	"neqn: fatal error: "
NFUNTYPE "neqn: 1003-209 The type %d is not correct.\n "
NLONGINP "neqn: 1003-210 The %.20s input line is longer than 1200 characters.\n"
NNOBKSL	"neqn: 1003-211 Specify a quote in %.20s\n"
NNOEQNST "neqn: 1003-212 There are no eqn strings left (%d).\n"
NOUTLOOK "neqn: 1003-213 There is not enough memory available now.\n""
NOUTSTRS "neqn: 1003-214 There is not enough memory available for this string %s now.\n"
NPSHOVF	"neqn: 1003-215 There is a pushback overflow.\n"
NLONGQUO "neqn: 1003-216 The quoted string %.20s cannot be longer than 400 characters.\n"
NEQNERR	"neqn: 1003-217 The program is stopped because of an neqn error.\n"
$ Do not translate neqn.
NSYNTAX	"neqn: 1003-218 The syntax is not valid.\n"
NLONGTKN "neqn: 1003-219 The token %.20s is too long.\n"
FILEP3	"neqn: 1003-220 \nfile %1$s, between lines %2$d and %3$d\n"
$ refer to the file name and line numbers.
NEQN_DELIM	"neqn: 1003-221 Delimiters must be printable ASCII characters.\n"
$ delim keyword and -d command line option allow the user to specify
$ neqn delimiters. These have to be in the C locale.
NEQN_PUSHBAD "neqn: 1003-222 Pushed bad input source type (type = %d)\n"
$ debug message - do not translate 
NEQN_POPUNDERFLOW "neqn: 1003-223 popsrc() called too many times\n"
$ The string 'popsrc()' should not be translated. This message should
$ never happen
NEQN_WEIRDPOP "neqn: 1003-224 Popped bad input source type (type = %d)\n"
$ debug message - do not translate
NEQN_UNPUT "neqn: 1003-225 Attempt to unput an invalid character.\n"
NEQN_BADWC "neqn: 1003-226 Invalid character in the input.\n"
NEQN_WCTOMB "neqn: 1003-227 wctomb() got an invalid pointer address.\n"
$ do not translate 'wctomb()'. This message should never happen.
NEQN_DELIM2	"neqn: 1003-228 Invalid character used as a quote mark. Use only ASCII characters.\n"
$ do not translage ASCII. Language tokens must be ASCII characters
NEQN_HEXONLY "neqn: 1003-229 ILS escape sequences must be composed of hexadecimal digits only. The sequence %1$s is invalid.\n"
NEQN_DEFINE	"neqn: 1003-230 Macro names must be specified in ASCII.\n"
$
$ Eqn
$
ECNTXT	"eqn: The context is\n"
NEARF	" near line %1$d, file %2$s"
$ refers to the line number and file name. The message NCNTXT and NEARF are
$ portions of a complete message.
BIZDEL	"eqn: 1003-103 The delimiters are not correct or cannot be printed.\n"
EUNXEINP "eqn: 1003-104 Did not expect the end of input at %.20s"
EUNTDEF	"eqn: 1003-105 The definition at %.20s does not have an ending delimeter.\n"
EQNUSG	"Usage: %s [-d Delimeter1Delimeter2] [-p Number] [-s Number] [-f Number] [-T Destination] [Files]\n"
$ refers to the command name.
EARGFU	"eqn: 1003-106 A macro has been used in a way that is not valid.\n"
EARGDEEP "eqn: 1003-107 The parameters are too deep.\n"
ENOOPN1	 "eqn: 1003-108 Cannot find or open file %s.\n"
$ refers to the file to be opened.
ELONGCTK "eqn: 1003-109 The changed token %.25s cannot be longer than 400 characters.\n"
DELIMS	"eqn: delimiters: lefteq=%1$c, righteq=%2$c, noeqn is %3$d.\n"
$ Do not translate
EBADODEF "eqn: 1003-111 The define must have a left parenthesis.\n"
EEOFGTAR "eqn: 1003-112 Have reached end of file in getarg().\n"
$ Do not translate getarg().
EQNBRK	"eqn: 1003-113 Warning: multi-line %1$c...%2$c, lines %3$d-%4$d, file %5$s\n"
EQNNQT	"eqn: 1003-114 Warning: unquoted troff command \\%1$c, line %2$d, file %3$s\n"
FNTOVF	"eqn: 1003-115 Cannot have no more than 10 fonts.\n"
EFUNTYPE "eqn: 1003-116 The type %d is not correct.\n"
GSZINFO	"eqn: gsize is %1$d, deltaps is %2$d, ftstack[0].name is %3$s.\n"
$ Do not translate
BADGSZ	"eqn: 1003-118 The global point size %s is not valid, the value is ignored.\n"
$ Refers to gobal point size.
BADSZ	"eqn: 1003-119 The point size %s is not valid, the value is ignored.\n"
$ Do not translate the word size or %s refers to point size.
ELONGINP "eqn: 1003-120 The %.20s input line is longer than 3600 characters.\n"
EINPNEST "eqn: 1003-121 The inputs are nested too deep.\n"
ENOBKSL	"eqn: 1003-122 Specify a quote in %.20s.\n"
ENOEQNST "eqn: 1003-123 There are no eqn strings left (%d).\n"
EOUTLOOK "eqn: 1003-124 There is not enough memory available now.\n"
EOUTSTRS "eqn: 1003-125 There is not enough memory available for the string %s now.\n"
EPSHOVF	"eqn: 1003-126 There is a pushback overflow.\n"
EPSHBADT "eqn: 1003-127 Pushed a bad type %d.\n"
ELONGQUO "eqn: 1003-128 The quoted string %.20s cannot be longer than 400 characters.\n"
EEQNERR	"eqn: 1003-129 The program is stopped because of an eqn error.\n"
$ Do not translate neqn.
ESYNTAX	"eqn: 1003-130 The syntax is not valid.\n"
ELONGTKN "eqn: 1003-131 The token %.20s is too long.\n"
EMANYINP "eqn: 1003-132 Too many inputs popped\n"
TYPESET	"eqn: typesetter is %1$s, res is %2$d, minsize is %3$d, ttype is %4$d.\n"
$ Do not translate
$ smallest point size and terminal type.

$ The following messages added to eqn 1/91

UNXEFEQ	"eqn: 1003-134 Did not expect the end of input inside equation.\n"
EQN_BADCHAR "eqn: 1003-135 Invalid character in input (hexadecimal value %02X).\n"
$ A call to getwc has returned a character that wctomb cannot convert.
EQN_UNEXMB "eqn: 1003-136 Unexpected multibyte character found.\n"
$ This should never happen!! A call to mblen has found a multibyte character
$ where none is expected.
EQN_WCCVT "eqn: 1003-137 Conversion to wide character failed for %.20s\n"
$  The first 20 characters of the string are printed
EQN_MALLOC "eqn: 1003-138 Insufficient memory to allocate buffer of size %d\n"
EQN_ASCIIONLY "eqn: 1003-139 Illegal use of non-ASCII character in input.\n"
$ Macro names and other eqn language tokens must be 7 bit ASCII
$ characters. This message is displayed when a pure ASCII string is expected 
$ and a non-ASCII character is encountered.
EQN_DEFINE	"eqn: 1003-140 Macro names must be specified in ASCII.\n"
$ see last comment
EQN_MBLEN "eqn: 1003-141 A call to mblen failed.\n"
$ do not translate mblen. This should never happen.
EQN_UNPUT "eqn: 1003-142 Function unput was called with invalid character.\n"
$ do not translate unput. This should never happen.
$
$ Grap
$
GCNTXT	"grap: The context is\n"
GNEARFN	" near line %1$d, file %2$s\n"
$ refers to the line number and file name. The message GCNTXT and GNEARFN are
$ portion of a complete message.
GARGDOM	"grap: 1003-403 The input value to %s is either a negative or zero value.\n"
$ refers to a math function being called.
GRESRNG	"grap: 1003-404 The %s has resulted in an overflow condition.\n"
$ refers to a math function being called.
GEDEFNAM "grap: 1003-405 The %s must be specify as either a define or as a name.\n"
$ Do not translate define.
GEVARDEF "grap: 1003-406 The %s is used as a variable and a define.\n" 
$ Do not translate define.
GNOOPN2	"1003-407 %1$s: Cannot find or open %2$s\n"
$ refer to the command name and file to be opened.
GARGFU	"grap: 1003-409 A macro has been used in a way that is not valid.\n"
GARGDEEP "grap: 1003-410 The parameters are too deep.\n"
GNOGROW	"grap: 1003-411 Cannot increase the size of %1$s to %2$d.\n"
$ refer to the name of the variable and size.
NOLOGX2	"grap: 1003-412 Cannot have log of x coord %1$g,%2$g\n"
NOLOGY2	"grap: 1003-413 Cannot have log of y coord %1$g,%2$g\n"
GNOOPN1	"grap: 1003-414 Cannot find or open file %s.\n"
$ refers to the file to be opened.
NOLOGT	"grap: 1003-415 Cannot take log of tick value %g\n"
$ refers to the tick value.
NOLOGX1	"grap: 1003-416 Cannot take log of x coord %g\n"
NOLOGY1	"grap: 1003-417 Cannot take log of y coord %g\n"
GDLSDEFN "definition"
$ Do not translate.
GBADODEF "grap: 1003-419 The define must have a left parenthesis.\n"
GDIVZERO "grap: 1003-420 Cannot divide by 0.\n"
GDLSELSE "else part"
$ Do not translate.
GEOF4	"grap: 1003-422 There is an end of file in %1$s %2$c %3$.20s... %4$c.\n"
GEOFGTAR "grap: 1003-423 Have reached end of file in getarg().\n"
$ Do not translate getarg().
EOFG1	"grap: 1003-424 The end of file has been reached within .G1/.G2 constructs.\n"
$ Do not translate the .G1 and .G2 refers to start and stop grap macros.
GFLTEXCP "grap: 1003-425 There is a floating point exception.\n"
GFORNEST "grap: 1003-426 Cannot nest for-loops more than 10 levels deep.\n"
$ A "for-loop" is a control statement using the "for" command.
GFORPOP	"grap: 1003-427 An internal error has occurred during for-loop processing.\n"
$ A "for-loop" is a control statement using the "for" command.
GNOOPNW	"grap: 1003-428 Cannot find or open %s\n"
GNOOPN	"grap: 1003-429 Cannot find or open %s\n"
MUSTCAP	"grap: 1003-430 The graph name %s must be capitalized.\n"
$ refers to the name of the graph.
GINPNEST "grap: 1003-431 The inputs are nested too deep.\n"
GDLSLOOP "loop body"
$ Do not translate.
GNLINSTR "grap: 1003-433 There is a newline in the string.\n"
NONAMGR	"grap: 1003-434 Specify a name on the graph statement.\n"
$ Do not translate graph
GOUTLOOK "grap: 1003-435 There is not enough memory available now.\n"
OUTMKAT	"grap: 1003-436 There is not enough memory available now.\n"
GOUTTOST "grap: 1003-437 There is not enough memory available for the %s string.\n"
GPSHOVF	"grap: 1003-438 There is a pushback overflow.\n"
GPSHBADT "grap: 1003-439 Pushed a bad type %d.\n"
GLONGSTR "grap: 1003-440 The string must be no longer than 1500 characters.\n"
GSYNTAX	"grap: 1003-441 The syntax is not valid.\n"
GDLSTHEN "then part"
$ Do not Translate.
GMANYINP "grap: 1003-443 There are too many inputs popped.\n"
MANYNUM	"grap: 1003-444 There cannot be more than 200 numbers.\n"
MANYTIC	"grap: 1003-445 There cannot be more than %d ticks.\n"
$ Do not translate tick
GUNXEFDT "grap: 1003-446 Did not expect end of file in the do_thru.\n"
$ Do not tranlate do_thru. 
GRAPUSG	"Usage: %s [-l] [-T Typesetter] [--] [-] [File] ...\n"
GRAP_MBLEN "grap: 1003-447 The function mblen failed unexpectedly.\n"
$ Do not translate mblen.
GRAP_BADCHAR "grap: 1003-448 Invalid multibyte character (first byte is %02X)\n"
$ 
GRAP_WCTOMB1 "grap: 1003-449 The function wctomb failed unexpectdly trying to convert %1$C"\n"
$ Do not translate wctomb. This should never happen.
GRAP_WCTOMB2 "grap: 1003-450 The function wctomb was called with an invalid address\n"
$ Do not translate wctomb. This should never happen.
GRAP_UNPUT "grap: 1003-451 The function unput was called with an invalid character\n"
$ Do not translate unput.
