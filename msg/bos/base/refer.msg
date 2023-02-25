$ ENGLISH_SOURCE_VERSION= 1.9.1.1
$ @(#)76	1.2  src/msg/bos/base/refer.msg, bosmsg, msg411, 9435A411a 8/25/94 16:05:18
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting Services
$
$ FUNCTIONS:
$
$ ORIGINS: 10,26,27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989,1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for refer
$
$quote "

$set MS_REFER
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

1  "Usage: indxbib database.{ia, ib, ic}\n"
2  "roffbib: 1002-301 The %s is not a valid flag.\n"
3  "indxbib: 1002-102 The database file must be in the current directory.\n"
AUSAGE1 "Usage:  addbib [-p Promptfile] [-a] Database\n"
AUSAGE2 ""
AUSAGE3 ""
BIBSPEC ""
NENARGP ""
NBIBA   ""
INVFLG  ""
TOOMANYARG  ""
INSTRUCT    "Instructions? "
TOOFAR      "Too far back.\n"
ABSTRACT    " Abstract: (control-d to end)"
$ Do not translate control-d.
REFCONTINUE "\nContinue? "
IN1  "Addbib will prompt you for various bibliographic fields."
IN2  "If you don't need a particular field, just hit RETURN,"
IN3  "\tand that field will not appear in the output file."
IN4  "If you want to return to previous fields in the skeleton,"
IN5  "\ta single minus sign will go back a field at a time."
IN6  "\t(This is the best way to input multiple authors.)"
IN7  "If you have to continue a field or add an unusual field,"
IN8  "\ta trailing backslash will allow a temporary escape."
IN9  "Finally, (without -a) you will be prompted for an abstract."
IN10 "Type in as many lines as you need, and end with a ctrl-d."
IN11 "To quit, type n  when asked if you want to continue."
$ Translator:  the n refers to the English word no.
IN12 "To edit the database, type  edit ,  vi , or  ex  instead."
PRFILE      "addbib: 1002-022 Cannot find or open the promptfile.\n"
NTABS       "addbib: 1002-023 The promptfile is missing a <TAB> between prompt string and %%key\n"
$ Do not translate <TAB> and %%key.
TABFMT      ""
TMNYENT     "addbib: 1002-025 Specify no more than 50 entries in the promptfile.\n"
AUTHOR      "   Author:"
TITLE       "    Title:"
JOURNAL     "  Journal:"
VOLUME      "   Volume:"
PAGES       "    Pages:"
PUBLISHER   "Publisher:"
CITY        "     City:"
DATE        "     Date:"
OTHER       "    Other:"
KEYWORDS    " Keywords:"
$ Lookbib messages
LUSAGE1         "Usage:  lookbib Database\n"
LUSAGE2         ""
LKBIBTTY        "lookbib: 1002-202 /dev/tty"
$ Do not translate.
NEITHER         "lookbib: 1002-203 The %1$s.ia index file and the %2$s reference file are not found.\n"
HUNT            "lookbib: 1002-204 /usr/lib/refer/hunt"
$ Do not translate.
EOT             "lookbib: 1002-205 EOT\n"
$ Do not translate.
INS1            "\nType keywords (such as author and date) after the > prompt.\n"
INS2            "References with those keywords are printed if they exist;\n"
INS3            "\tif nothing matches you are given another prompt.\n"
INS4            "To quit lookbib, press CTRL-d after the > prompt.\n\n"
$ Sortbib messages
SUSAGE          "Usage:  sortbib [-sKEYS] Database [...]\n"
SUSAGE2         ""
TOOMANYDB       "sortbib: 1002-402 Specify no more than %d databases.\n"
RECTOOLONG      "sortbib: 1002-403 The record %1$d is longer than %2$d (%3$d)\n"
SORTBIB         "1002-404 sortbib"
SORTERROR       "sortbib: 1002-405 sorting error"
INTERRUPT       "sortbib: 1002-406 Interrupt\n"
ERROR           "sortbib: 1002-407 Error: "
$ Refer message
CANTOPEN    "refer: 1002-503 Cannot find or open %s.\n"
ERR         ""

CALLCALLOC  "refer: 1002-505 calling calloc for %1$d*%2$d bytes\n"
RETCALLOC   "refer: 1002-506 calloc returned %o\n"
$ CALLCALLOC AND RETCALLOC NOT IN CODE.
ARGPRT      "refer: 1002-507 in glue1 argc %1$d argv %2$o %3$o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ARG1PRT     "refer: 1002-508 argv.1 is %s\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
LMASTER     "refer: 1002-509 lmaster now %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
HUNTMAIN    "refer: 1002-510 in huntmain indexname %1$s typeindex %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
FOIDX       "refer: 1002-511 found old index\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
OPENFS      "refer: 1002-512 opened f's as %1$o %2$o %3$o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
FREED       "refer: 1002-513 Freed if needed maxhash %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
HPT         "refer: 1002-514 hpt now %o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NOSPCHASH   "refer: 1002-515 There is not enough memory for hash list (%d).\n"
NSPCFRQ     "refer: 1002-516 There is not enough memory for hash frequencies (%d).\n"
RDPTRF      "refer: 1002-517 Read pointer files\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NOSPANSLST  "refer: 1002-518 There is not enough memory for the answer list.\n"
NOFLS       "refer: 1002-519 Cannot find or open %s\n"
TYPIDX      "refer: 1002-520 typeindex now %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
BCKGRPCL    "refer: 1002-521  back from grepcall\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
APPDOQU     "refer: 1002-522 approaching doquery fb %o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
RETDOQU     "refer: 1002-523 return from doquery with nfound %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
BADDROP     "refer: 1002-524 after baddrop with nfound %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
TAGOUTIS    "refer: 1002-525 tagout is /%1$s/, sout /%2$s/\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
DONEWITH    "refer: 1002-526 done with huntmain\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
TAGOUTIS2   "refer: 1002-527 tagout is /%s/\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
STRINGOUTIS "refer: 1002-528 string out is /%s/\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
WEREBAD     "refer: 1002-529 were %d bad files\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NOWONQ      "refer: 1002-530 Now on %1$s query /%2$s/\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
TAGOUTNOW   "refer: 1002-531 tagout now /%s/\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
CANTRUN     "refer: 1002-532 Cannot run %s.\n"
INDODELIV   "refer: 1002-533 in dodeliv, arg /%s/\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
INGRPCL     "refer: 1002-534 in grepcall, gfile %1$s in %2$o out %3$o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ARSTUP      "refer: 1002-535 %d args set up\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
CNTRDFG     "refer: 1002-536 Cannot read fgrep index %s.\n"
$ Do not translate fgrep.
CNTWRFG     "refer: 1002-537 Cannot write fgrep output %s.\n"
$ Do not translate fgrep.
FGRRET      "refer: 1002-538 fgrep returned, output is..\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
BKFRFG      "refer: 1002-539 back from fgrep\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
CTRDOUT     "refer: 1002-540 Cannot read fgrep output %s.\n"
$ Do not translate fgrep.
RDLNAS      "refer: 1002-541 read line as //%s//\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
OUTNOW      "refer: 1002-542 out now /%s/\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
LINE        "refer: 1002-543 line %1$o s %2$o s-1 %3$o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
BKFRRD      "refer: 1002-544 back from reading %, out %s\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
CNTGTSPC    "refer: 1002-545 There is not enough memory available now.\n"
INCT        "refer: 1002-546 inct %1$d xargv.2. %2$o %3$d\n"
BDFGRP      "refer: 1002-547 bad fgrep call\n"
BFRCGTO     "refer: 1002-548 before cgoto\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
BFRCFAIL    "refer: 1002-549 before cfail\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
BFREXINST   "refer: 1002-550 before execute instr %.20s\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ENDOFSTR    "refer: 1002-551 end of string %1$d %2$c %3$c %4$c\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
RTNSUCC     "refer: 1002-552 returning nsucc %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
FGRPDN      "refer: 1002-553 fgrep done www %o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
INEXCCNT    "refer: 1002-554 in execute ccount %1$d inct %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
DNCCNT      "refer: 1002-555 down ccount\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
EXLOOPCCNT  "refer: 1002-556 ex loop ccount %1$d instr %2$o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NRMLRD      "refer: 1002-557  normal read %d bytres\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
THEYARE     "refer: 1002-558 they are %%.%ds\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ROAMALNG    "refer: 1002-559 roaming along in ex ch %1$c c %2$o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
FOUNDNFND   "refer: 1002-560  found: nfound %1$d need %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
FOUNDP      "refer: 1002-561 found, p %1$o nlp %2$o ccount %3$d buf %4$o buf[2*BUFSIZ] %5$o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
DOWNCCNT2   "refer: 1002-562 down ccount2\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
RDBYTES     "refer: 1002-563  read %d bytes\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
PONLP       "refer: 1002-564 p %1$o nlp %2$o buf %3$o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NRENDLP     "refer: 1002-565 nr end loop p %o\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
INFRLPC     "refer: 1002-566  in for loop c now %1$o %2$c\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NXTARGIS    "refer: 1002-567 next arg is %1$s xargc %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
WRDLST      "refer: 1002-568 wordlist too large\n"
READHASHES  "refer: 1002-569 read %1$d hashes, iflong %2$d, nhash %3$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
READFREQS   "refer: 1002-570 read freqs %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
AFTERDOQUER "refer: 1002-571 after doquery nfound %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
AFTERBADD   "refer: 1002-572 after baddrop nfound %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
WEREBADFILES "refer: 1002-573 were %d bad files\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
GRPQUER     "refer: 1002-574 grepquery %s\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NOWON       "refer: 1002-575 Now on %1$s query /%2$s/\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
GRPMD       "refer: 1002-576 grepmade\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
TAGOUT      "refer: 1002-577 tagout now /%s/\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
IDXINCOMP   "refer: 1002-578 Index incomplete %s"
ENTRING     "refer: 1002-579 entering doquery nitem %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
FSTHSH      "refer: 1002-580 first few hashes are %1$ld %2$ld %3$ld %4$ld %5$ld\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ANDFRQ      "refer: 1002-581 and frequencies are  %1$d %2$d %3$d %4$d %5$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NITEM       "refer: 1002-582 nitem %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
QUERYWD     "refer: 1002-583 query wd X%1$sX has hash %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
PSTTHTLP    "refer: 1002-584 past that loop nhash %1$d hpt is %2$lo\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ITEMHSH     "refer: 1002-585 item %1$s hash %2$d hfreq %3$d\n"
ITMHSH2     "refer: 1002-586 item hash %1$d frq %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
FSTITHSH    "refer: 1002-587 first item hash %d lp %ld 0%lo\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
MSTRHS      "refer: 1002-588 master has %ld\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
MSTRHS2     "refer: 1002-589 master has %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ITMHASH     "refer: 1002-590 item %1$d, hash %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ITHSHSK     "refer: 1002-591 item %1$d hash %2$d seek to %3$ld\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NXTTERM     "refer: 1002-592 next term finds %ld\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
BFWH        "refer: 1002-593 bfwh j %1$d nf %2$d master %3$ld k %4$ld\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
JNF         "refer: 1002-594 j %1$d nf %2$d prevdrop %3$ld prevcoord %4$d colevel %5$d nterm %6$d k %7$ld\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NOTSKIP     "refer: 1002-595  not skip g %1$d doc %2$d coord %3$d note %4$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NTSKIPDOC   " refer: 1002-596 not skip g %1$d doc %2$ld coord %3$d nterm %4$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ATG         "refer: 1002-597  at g %1$d item %2$ld coord %3$d note %4$ld\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
ATGIT       "refer: 1002-598  at g %1$d item %2$d coord %3$d note %4$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
NOWHAVEIT   "refer: 1002-599 now have %d items\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
COPIEDOVR   "refer: 1002-601 copied over %1$ld coord %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
COPIEDD     "refer: 1002-602 copied over %1$d coord %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
COLEVEL     "refer: 1002-603 colevel %1$d best %2$d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
YETGOT      "refer: 1002-604 yet got %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
RETWITH     "refer: 1002-605  returning with %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
QUERYLONG   "refer: 1002-606 query long than %d characters\n"
NOKEYS      "refer: 1002-607 no. keys %d\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
KEYS        "refer: 1002-608 keys X%sX\n"
$ DO not translate, it needs to be removed later its a DEBUG statement.
INBADDRP    "refer: 1002-609 in baddrop, nf %1$d master %2$ld %3$ld %4$ld\n"
INBDRP2     "refer: 1002-610 in baddrop, nf %1$d master %2$d %3$d %4$d\n"
$ Do not Translate
$ INBADDRP AND INBDRP2 NOT IN CODE.
IDXPREDATE  "refer: 1002-611 Warning: index predates file '%s'\n"
MISSING     "refer: 1002-612 Missing expected file %s"
CNTRDINP    "refer: 1002-613 Cannot find or open %s.\n"
CNTGTSCR    "refer: 1002-614 Cannot find or open %s.\n"
PFORK       "refer: 1002-615 pfork %1$o pwait %2$o status %3$d\n"
KEYOCCUR    "refer: 1002-616 %1$ld key occurrences,  %2$d hashes, %3$d docs\n"
NOOLDPTRS   "refer: 1002-617 The file pointer is NULL.\n"
$ Do not translate NULL.
CHGHSHVAL   "refer: 1002-618 Changing hash value to old %d\n"
CANTREAD        "refer: 1002-619 Cannot find or open %s.\n"
REFNOFILE       "refer: 1002-620 Cannot find or open %s.\n"
CNTOPSCR        "refer: 1002-621 Cannot find or open scratch file.\n"
CNTOPSC2        "refer: 1002-622 Cannot find or open scratch file.\n"
CANTRD          "refer: 1002-623 Cannot find or open %s.\n"
QUERTOOLNG      "refer: 1002-624 The %d character query cannot be longer than 512 characters.\n"
RECAT           "refer: 1002-625 The record at line %d cannot be longer than 1536 characters.\n"
ACCUMANSW       "refer: 1002-626 Accumulated answers are too long.\n"
ANSWTOOLNG      "refer: 1002-627 The answer is too long (%d).\n"
NOSCHPAP        "refer: 1002-628 No such paper: %s.\n"
TOOMNYHITS      "refer: 1002-629 Too many hits: %s\n"
QUEST           "refer: 1002-630 Question at %s.\n"
BUFFTOBIG       "refer: 1002-631 The buffer is longer than %d.\n"
BUFFTOLONG      "refer: 1002-632 The output buffer is longer than %d.\n"
FILELINE        "refer: 1002-633 File %1$s line %2$d: punctuation ignored from: %3$s"
SIGOVFLOW       "refer: 1002-634 sig overflow (%d)"
T1OVFLOW        "refer: 1002-635 t1 overflow (%d)"
CANTGETSCR      "refer: 1002-636 Cannot find or open %s.\n"
BFLAB           "refer: 1002-637 bflab overflow (%d)"
NREF            "refer: 1002-638 nref in labc overflow (%d)"
JOURN           "1 journal-article"
$ Do not translate.
ARTICLE         "3 article-in-book"
$ Do not translate.
TECH1           "4 tech-report"
$ Do not translate.
BOOK            "2 book"
$ Do not translate.
BELL            "5 bell-tm"
$ Do not translate.
OTHER0          "0 other"
$ Do not translate.
TOOMNYREF       "refer: 1002-645 There are more than 500 references for the table (%d).\n"
REFPTRS         "refer: 1002-646 reference pointers too long (%d)"
CANTREPOEN      "refer: 1002-647 Cannot reopen %s.\n"
REFCOND         "refer: 1002-648 There is not enough memory available now.\n"
$ These do not appear to be used.
HASHES          "refer: 1002-649 hashes %1$d used %2$d\n"
ELAPSED         "refer: 1002-650 Elapsed %.2f CPU %.2f (user %.2f, sys %.2f)\n"
NOQUERY         "refer: 1002-651 No query.\n"
FILES           "refer: 1002-652    %d files\n"
ADDING          "refer: 1002-653 Adding: "
DFILES          "refer: 1002-654   %d files\n"
BYTES           "refer: 1002-655 %s (%.20s)  %s, %ld bytes\n"
YOULIKE         "You like that one?"
TOTAL           "There is a total of %d items.\n"

$ This is a list of English articles,
$ which are ignored in bibliographic sorting and searching.
$ Translations should be a list of blank-separated words
$ to ignore in such searches.
$ See documentation for further explanation.
ARTICLES	"a an the"

$ Here is punctuation under the control of refer's "-P" flag.
$ See documentation for further explanation.
PUNCTUAT	".?!,;:"
216	"BIBLIOGRAPHY"
