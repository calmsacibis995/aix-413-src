$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)69	1.1  src/msg/bos/base/mh.msg, bosmsg, msg411, GOLD410 2/9/94 09:07:39
$ 
$ COMPONENT_NAME: CMDMH mh.msg
$ 
$ FUNCTIONS: 
$
$ ORIGINS: 10  26  27  28  35 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for mh
$

$set MS_MH

$quote "

NOEXEC          "unable to exec "
NOREPLY         "No reply text given"
NOPIPES         "no pipes"
NOFORK          "unable to fork"
NOFDOPEN        "unable to fdopen"
NOFDOPEN2       "unable to fdopen %s"
NODUP           "unable to dup"
NOSTOR          "unable to allocate string storage"
EXTRA1          "extra >"
EXTRA2          "extra )"
ERROR           "Error %d"
CONT            "continuing..."
AMBIG           "-%s ambiguous.  It matches"
ILLEGAL         "illegal \\"
MISSING         "missing >"
MISSING2        "missing )"
MISSING3        "missing \""
BADADDR         "bad address '%1$s' - %2$s"
NULLADR         "null address"
NULLADR2        "null address '%s'"
NOMBOX          "no mailbox in address"
NOMBOX2         "no mailbox in address '%s'"
NOMEM           "insufficient memory to represent address"
ILLADDR         "illegal address: %s"
WERR            "error writing %s"
RERR            "error reading %s"
MKDIR           "mkdir %s"
FCERR           "\"%1$s\": format compile error - %2$s"
NOFSTOR         "unable to allocate format storage"
NOBSP           "formataddr: couldn't get buffer space"
NOABSP          "formataddr: couldn't allocate buffer space"
NOOPEN          "unable to open %s"
NOOPENFF        "unable to open format file %s"
NOSTATFF        "unable to stat format file %s"
NOFSP           "unable to allocate space for format"
NORFF           "error reading format file"
OPTIONS         "Options are:\n"
HELP            " -%s unknown. Hit <CR> for help.\n"
BADDEL          "illegal argument delimiter: `%c'(0%o)"
NOMES           "no %s message"
NOTHERE         "message %s doesn't exist"
OUTOFRAN        "message %1$s out of range 1-%2$d"
BADLIST         "bad message list %s"
FULL            "folder full, no %s message"
NOMAT           "no messages match specification"
NONE            "no messages in range %s"
NOEXIST         "message %d doesn't exist"
BUG             "bug: m_delete(key=\"%s\")"
ERRF            "error on folder %s"
NOCREAT         "unable to create folder %s"
NOREAD          "unable to read %s"
NOREAD2         "unable to read '%s'"
NOREADF         "unable to read folder %s"
NOAFSTOR        "unable to allocate folder storage"
ONE             "only one message draft at a time!"
NOAPSTOR        "unable to allocate profile storage"
NOHOME          "no HOME envariable"
NOCREATE        "unable to create %s"
EOFINFLD        "eof encountered in field \"%s\""
EOLINFLD        "eol encountered in field \"%s\""
BIGFNM          "field name \"%1$s\" exceeds %2$d bytes"
BOGUS           "m_getfld() called with bogus state of %d"
MDEL            "maildrop delimiter must be at least 2 bytes"
FIX             "please fix the %1$s: entry in your %2$s file"
EX1             "extra '%>' or '%|'"
NEEDNM          "component or function name expected"
NEEDBRC         "'}' expected"
DTADDR          "component used as both date and address"
EX2             "'(', '{', ' ' or ')' expected"
UNKFUNC         "unknown function"
EX3             "component name expected"
EX4             "')' expected"
EX5             "'(', '{', '%<' or ')' expected"
EX6             "'(' or '{' expected"
EX7             "'>' expected."
UNKNOWN         "unknown"
AMBIGUOUS       "ambiguous"
OFULL           "full"
EMPTY1          "empty"
SEQ             "sequence %1$s %2$s"
NOAISTOR        "unable to allocate info storage"
NOAMSTOR        "unable to allocate messages storage"
BIGLOSE         "m_gmsg() botch -- you lose big"
NOBLANKS        "no blank lines are permitted in %s/%s"
BADFORMAT       "%s/%s is poorly formatted"
PUMP            "bug: m_readefs called but pump not primed"
NOBLINES        "no blank lines are permitted in %s"
POORFOR         "%s is poorly formatted"
NORAFSTOR       "unable to re-allocate folder storage"
NORAMSTOR       "unable to re-allocate messages storage"
BOTCH1          "m_remsg() botch -- you lose big[1]"
BOTCH2          "m_remsg() botch -- you lose big[2]"
BUG2            "bug: m_replace(key=\"%1$s\",value=\"%2$s\")"
ONLY            "only %1$d sequences allowed (no room for %2$s)!"
NOSUCH          "no such sequence as %s"
EMPSNM          "empty sequence name"
ILLSNM          "illegal sequence name: %s"
NOWRITE         "unable to write %s"
NODIR           "unable to create directory %s"
EXIT            "Exit %d\n"
SIGNAL          "Signal %d"
CORE            " (core dumped)"
NOPUSH          "unable to fork, so can't push..."
NOWDIR          "unable to determine working directory"
NOWDIR2         "unable to determine working directory: %s"
NORMDIR         "unable to remove directory %s"
BIGLOSE2        "vfgets() botch -- you lose big"
UNKWNSW1        "-%s unknown"
HELPSW1         "%s [switches] aliases ..."
NOARG           "missing argument to %s"
ALIASERROR      "aliasing error in %1$s - %2$s"
USAGE           "usage: %s addresses ...  (you forgot the addresses)"
EMPTY2          "<empty>\n"
NOADDRESS       "no address in \"%s\""
BADADDRESS      "bad address \"%s\""
NEEDAF          "'<' without alias-file"
LERROR          "error in line '%s'"
OUTOFMEM        "out of memory while on '%s'"
NOGRP           "no such group as '%s'"
UNKERROR        "unknown error (%d)"
MHELPSW         "%s [+folder] [msgs] [switches]"
OONECOMP        "only one component at a time!"
ONEBODY         "only one body at a time!"
ONEFOLDER       "only one folder at a time!"
NOMESF          "no messages in %s"
COMPNM          "Enter component name: "
NULLCOMPNM      "null component name"
INVCOMPNM       "invalid component name %s"
BIGCOMPNM       "too large component name %s"
NOOPENRR        "unable to open for re-reading %s"
NORENAME        "unable to rename %1$s to %2$s"
SWADDRS         "%s [switches] addrs ..."
MANYADDR        "more than %d addresses"
USWADDR         "usage: %s [switches] addrs ..."
NOAPMEM         "unable to allocate pqpair memory"
NOTKNOWN        "-%s unknown\n"
NOABSTOR        "unable to allocate burst storage"
NORMSG          "unable to read message %s"
BIGLOSE3        "burst() botch -- you lose big"
NOTDF           "message %d not in digest format"
ONEFILE         "only one file at a time!"
ONEDFOLD        "only one draft folder at a time!"
ONEDMSG         "only one draft message at a time!"
ONEMSG          "only one message at a time!"
NOMIX           "can't mix forms and folders/msgs"
NOOPENM         "unable to open message %s"
NOOPENFL        "unable to open form file %s"
NOOPENDEF       "unable to open default components file %s"
NOSTAT          "unable to stat %s"
DRAFT           "Draft \"%1$s\" exists (%2$ld bytes)."
WHAT            "say what?"
ONEADDR         "mail to one address only"
MANYDIRS        "more than %d directories"
AERROR          "aliasing error in %1$s - %2$s\n"
ISOK            "alias file %s is ok\n"
USERSOK         "no duplicate users\n"
DUPUSER         "duplicate user %1$s(uid=%2$d)\n"
DUPGRP          "duplicate group %1$s(gid=%2$d)\n"
MANYGRPS        "more than %d groups (time to recompile)\n"
GRPSOK          "no duplicate groups\n"
UNKMEM          "group %1$s(gid=%2$d) has unknown member %3$s\n"
DUPMEM          "group %1$s(gid=%2$d) has duplicate member %3$s\n"
MULTIGRP        "user %1$s is a member of %2$d groups (max %3$d)"
GRPMEMOK        "all group members accounted for\n"
GRPUOK          "all group-id users accounted for\n"
NOOPENMA        "unable to open maildrop area %s\n"
MAILDROP        "there is a maildrop for the unknown user %1$s in %2$s\n"
MDROPSOK        "all maildrops accounted for in %s\n"
SUSP            "%s: the following is suspicious\n\n"
UNKGRP          "unknown group %s in group leaders file\n"
DUPGROUP        "duplicate group %s in group leaders file\n"
ALLOK           "all groups in group leaders file accounted for\n"
UNKLDR          "group %1$s has unknown leader %2$s\n"
DUPLDR          "group %1$s had duplicate leader %2$s\n"
ALLLDROK        "all group leaders accounted for\n"
MANYGPS         "more than %d groups in group leaders file (time to recompile)\n"
USEHELP         "%s [switches] [aliasfiles ...]"
UNKGRPID        "user %1$s(uid=%2$d) has unknown group-id %3$d\n"
EXP             "%1$d messages exploded from digest %2$d\n"
MTM             "message %1$d becomes message %2$d\n"
TRANS           "message %1$d of digest %2$d becomes message %3$d\n"
NOWRTM          "unable to write message %s"
UMSG            "%s [+folder] [msg] [switches]"
NOFFS           "can't mix files and folders/msgs"
NOTMPF          "unable to create temporary file"
NOTMPF2         "unable to create temporary file %s"
BADHDR1         "please re-edit %1$s to remove the ``%2$s'' header!"
BADTXT1         "please re-edit %s to consist of headers only!"
BADMSG1          "please re-edit %s to include a ``Resent-To:''!"
BADRFT1         "please re-edit %s and fix that header!"
RET             "getfld() returned %d"
NOOPENTF        "unable to re-open temporary file %s"
NOFDESC         "no file descriptors -- you lose big"
MSGFERROR       "format error in message %s"
HELPOPTS        "%s [switches] dates ..."
MANYDTS         "more than %d dates"
USEMSG          "usage: %s [switches] dates ..."
NOADSTOR        "unable to allocate drop storage"
WERROR          "write error %s"
BADINDEX        "%s: missing or partial index"
BADMATCH        "%s: version mismatch"
BADMNUM         "%s: bad magic number"
BADMI           "%1$s: pointer mismatch or incomplete index (%2$ld!=%3$ld)"
CPTINDX1        "%s: corrupt index(1)"
CPTINDX2        "%s: corrupt index(2)"
CURMSG          "only one (current) message at a time!"
NOOTHER         "no other folder"
NOPOP           "sorry, no folders allowed with -pop"
EMPSTK          "folder stack empty"
NOFINDF         "unable to find folder %s"
UNRDBL          "%22s%c unreadable\n"
BADARG          "bad argument %1$s %2$s"
MANYEMSGS       "more than %1$d messages for %2$s exec"
MESG            " Message %d"
NORDIR          "unable to read directory %s "
MANYFS          "more than %d folders to report on"
DEND            "End of %s Digest\n"
NOAFLSTOR       "unable to allocate format line storage"
UMESG           "%s [+folder] [switches]"
USAGEM          "usage: %s [+folder] [switches]"
NOMAIL          "no mail to incorporate"
CREATR_A        "Creating Receive-Audit: %s"
NOAPPND         "unable to append to %s"
INCNEWM         "Incorporating new mail into %s...\n\n"
FILEERR         "error on file %s"
WRITEERR        "write error on %s"
AINC            "inc aborted!\n"
ABORT           "aborted!"
MOREFMSGS       "more than %1$d messages in folder %2$s, %3$s not zero'd"
SCAN1           "scan() botch (%d)"
NEWMAIL         "new messages have arrived!\007"
NOTZERO         "%s not zero'd\n"
NOMSGS          "no messages incorporated"
ADDING          "\nIncorporating encrypted mail into %s...\n\n"
INVERR          "invocation error"
ALRDY           "You already have an MH profile, use an editor to modify it"
WANT            "Do you want help <yes/no>? "
THANKS          "I'm going to create the standard MH path for you.\n"
PATH            "Do you want a path below your login directory <yes/no>? "
WPATH           "What is the path?  %s/"
WHOLEPATH       "What is the whole path?  /"
USEEXST         "[Using existing directory]\n"
ONLYSEQS        "only %d sequences allowed!"
NOCHANGE        "unable to change directory to %s"
READONLY1       "folder %s is read-only, so -public not allowed"
NEEDONE         "-%s requires at least one -sequence argument"
SWIT            "%s [switches] [files ...]"
FFSERR          "format file syntax error: %s"
STATS           "c1: name=\"%1$s\" text=\"%2$s\" ovtxt=\"%3$s\"\n"
MSTATS          "\toffset=%1$d ovoff=%2$d width=%3$d cwidth=%4$d length=%5$d\n"
TFLAGS          "\tflags=%s\n"
ARGMISSING      "missing argument to variable %s"
FORMSGS         " Forwarded Messages"
FORWMSG         " Forwarded Message"
LIST            "Press <return> to list \"%s\"..."
NOABMEM         "unable to allocate buffer memory"
NOACMEM         "unable to allocate comp memory"
TOOSMALL        "component: %1$s width(%2$d) too small for overflow(%3$d)"
NOPIPE          "unable to pipe"
NODUPSO         "unable to dup() standard output"
NODUPSO2        "unable to dup2() standard output"
UNEXEC          "unable to exec %s"
ADDRS           "%s [addrs ... [switches]]"
LTRSAVED        "Letter saved in dead.letter\n"
MANYMSGS        "more than %d messages"
USEUSERS        "%s [switches] [users ...]"
YOULOSE         "you lose"
NOUSER          "no such user as %s"
USEFILE         "%s [switches] file"
NOTBOTH         "use a file or a folder, not both"
NODISP          "no dispatch for %s"
BOTCH3          "setup() botch -- you lose big"
NOAMSTRUCT      "unable to allocate Msgs structure"
NORAMSTRUCT     "unable to reallocate Msgs structure"
NODUPCMSG       "unable to dup cached message"
NOFDOPENCM      "unable to fdopen cached message"
BOTCH4          "msh_ready (%1$d, %2$d) botch"
WERRTF          "write error on temporary file"
NORRFLDR        "unable to re-read folder %s"
NOFSTAT         "unable to fstat %s"
MANYMESS        "more than %1$d messages in %2$s"
IDGAP           "gap in ID:s, last seen %1$d, lowest present %2$d\n"
REDO            "readid (%d) botch"
NODUPSO3        "unable to dup standard output"
NOFDOPENSO      "unable to fdopen standard output"
NOLOCKOPN       "unable to lock and open %s"
NOLOCKFOPN      "unable to lock and fopen %s"
NOLOCK          "unable to lock %s"
NEWMSGNU        "new messages have arrived, no update"
ZERROR          "error zero'ing %s"
CMDS            "commands:\n"
QUIT            "type CTRL-D or use ``quit'' to leave %s\n"
SAYWHAT         "say what: ``%s'' -- type ? (or help) for help\n"
NOMATCH         "unmatched \""
NOQUOTE         "the newline character can not be quoted"
INVNULLCMD      "invalid null command"
NORDNAM         "missing name for redirect"
BADRDNAM        "bad name for redirect"
UNKNUSER        "unknown user: %s"
WRITE           "write"
APPEND          "append"
CREFIL          "Create file \"%s\" <yes/no>? "
CREFOL          "Create folder \"%s\" <yes/no>? "
STANPTH         "Do you want the standard MH path \"%s/Mail\" <yes/no>? "
RMFOLD          "Remove folder \"%s\" <yes/no>? "
UPDFIL          "Update file \"%s\" <yes/no>?"
USE             "Use \"%s\" <yes/no>? "
STANDIR         "You already have the standard MH directory \"%s\".\nDo you want to use it for MH <yes/no>? "
ZEROFL          "Zero file \"%s\" <yes/no>?"
CREPTH          "\"%s\" doesn't exist; Create it <yes/no>? "
NODUP2          "unable to dup %s"
UNABLE3         "unable to %1$s %2$s"
NOPIPE2         "unable to pipe %s"
NOCMDRD         "unknown redirection for command"
NODUPSO4        "unable to dup2 standard output"
WPROB           "problems writing %s"
BROKE           "broken pipe\n"
NEWMSGS         "new messages have arrived!"
IDGAP2          "%1$s: gap in ID:s, last seen %2$d, lowest present %3$d\n"
EMPTY3          "empty!"
QUIT2           "type ``quit'' to leave %s\n"
HELP2           "say what: ``%s'' -- type ? (or help) for help"
PROTOSU         "pLOOP protocol screw-up"
PROTOSU2        "pTTY protocol screw-up"
PROTOSU3        "pWIN protocol screw-up"
PRERR           "error reading from pipe"
NODUPSI         "unable to dup standard input"
NODUPDO         "unable to dup diagnostic output"
NODUPDO2        "unable to dup2 diagnostic output"
NODUPSI2        "unable to dup2 standard input"
NULLCMD1        "null command"
BADINIT         "bad init \"%s\""
UNSUPP          "version %d unsupported"
PROTOSU4        "pINI protocol screw-up"
NOLISTFILE      "unable to create file for annotation list"
NOTDELIVER      "message not delivered to everyone"
NOROPEN         "unable to re-open %s"
MNOTDEL         "\nMessage not delivered to everyone.\n"
UNSENTDRFT      "\n------- Unsent Draft\n\n"
TMPFILE         "temporary file"
DRFTEND         "\n------- End of Unsent Draft\n"
NOSENDDRFT      "send failed on enclosed draft"
NOSEND          "send failed on %s"
NOREMOVE        "unable to remove %s"
BYHAND          "unable to fork, so doing annotations by hand..."
HELPSW2         "%s [+folder] [msgs] [switches] [switches for showproc]"
HELPSW3         "%s [+folder] [switches] [switches for showproc]"
USAGE2          "usage: %s [+folder] [switches] [switches for showproc]"
MANYMSGS2       "more than %d messages for show exec"
MSG1            "(Message %1$s:%2$s)\n"
HELPSW4         "%s [switches] [address info sender]"
NOSUCHUSER      "no such local user as %s"
NOATMPFILE      "unable to access temporary file"
NOTIME          "unable to ascertain local time"
HOOK            "(invoking hook)\n"
NORMATPT        "(trying normal delivery)\n"
BADSET          "%1$s: ownership/modes bad (%2$d, %3$d,%4$d,0$o)\n"
FRMTERR         "format error in message"
INTERR          "internal error"
DELIVERING      "\tdelivering to file \"%s\""
STYLE           " (uucp style)"
NOOPEN2         "unable to open:"
WERROR2         "error writing to:"
NOCOPY          "unable to copy message; skipping hook\n"
NOTTHERE        "\tnot present\n"
DELIVERING2     "\tdelivering to pipe \"%s\""
SYSERR          ", system error\n"
NOMSG           "no message"
ONEDFLD         "only one date field at a time!"
NOSORT          "no messages to sort"
NOASSTOR        "unable to allocate sort storage"
NOFIELD         "no %1$s field in message %2$d"
FRMTERR2        "format error in message %1$d(header #%2$d)"
INTERR2         "internal error -- you lose"
NOPARSE         "unable to parse %1$s field in message %2$d"
SWAP            "swap messages %1$s and %2$s\n"
NORENAME2       "unable to rename %1$s to %2$s "
CHANGES         "message %1$s becomes message %2$s\n"
ALIASERR        "aliasing error in file %1$s - %2$s"
USAGE3          "usage: %s [switches] file"
MFRMTERR        "message format error in component #%d"
NOREOPEN        "can't reopen for sendmail %s"
NOFORK2         "%1$s: can't fork to %2$s\n"
NOEXEC2         "can't exec %s"
ILLLINE         "illegal header line -- %s:"
REFORMAT        "re-format message and try again"
TOOMANY         "too many %ss"
NOREOPEN2       "unable to re-open"
NOVFORK         "unable to vfork"
LOST            "%s lost (status=0%o)"
NOFORKS         "no forks, so not ok\n"
ERRORED         " errored (0%o)\n"
MESSAGE		"\nPrior to using MH, it is necessary to have a file in your \
login\n\directory (%1$s) named %2$s which contains information\n\to direct \
certain MH operations.  The only item which is required\n\is the path to use \
for all MH folder operations.  The suggested MH\n\path for you is %3$s/Mail...\
\n\n"
HELPSW5         "%s [switches for vmhproc]"
PERROR          "pINI peer error"
NOWINDOW        "no such window \"%s\""
QRYPERR         "pLOOP(QRY) peer error"
CMDPERR         "pLOOP(CMD) peer error"
QRYPROTOSU      "pLOOP(QRY) protocol screw-up"
CMDPROTOSU      "pLOOP(CMD) protocol screw-up"
ANYKEY          "Type any key to continue... "
PEERERR         "pTTY peer error"
FLUSH           "flush window"
SMSCREEN        "screen too small"
PEERERR2        "pWIN peer error"
LOST2           "wgetch lost"
BADNUM1         "bad number"
NOGROK          "not understood"
NOALNSTOR       "unable to allocate line storage"
NOTTTY          "not a tty"
PLOST           "read from peer lost(1)"
MLOST           "malloc of %d lost"
PLOST2          "read from peer lost(2)"
WPLOST          "write to peer lost(1)"
WPLOST2         "write to peer lost(2)"
ERRORNO         ": Error %d"
WHATNOW1        "\nWhat now? "
HELPSW6         "%s [switches] [file]"
ONEDRAFT        "only one draft at a time!"
NOALT           "no alternate message to display"
NOUNLINK        "unable to unlink %s"
DLEFT           "draft left on %s"
PROBS           "problems with edit--%s deleted"
PROBS2          "problems with edit--%s preserved"
NOUPDATE        "unable to update %1$s from %2$s"
NOTRUNC         "unable to truncate %s"
DAMAGE          "may have damaged %s"
DIRECT          "unable to fork, so sending directly..."
SHELPSW1        "%s [switches]"
USAGE4          "usage: %s [switches]"
NOLINK          "unable to link %1$s to %2$s"
CHECKING        "unable to fork, so checking directly..."

$ these are the signals from pidstatus.c. Do not change the order or
$ add more.
SIGS		NULL
SIGS1		"Hangup"
SIGS2		"Interrupt"
SIGS3		"Quit"
SIGS4		"Illegal instruction"
SIGS5		"Trace/BPT trap"
SIGS6		"IOT trap"
SIGS7		"EMT trap"
SIGS8		"Floating exception"
SIGS9		"Killed"
SIGS10		"Bus error"
SIGS11		"Segmentation fault"
SIGS12		"Bad system call"
SIGS13		"Broken pipe"
SIGS14		"Alarm clock"
SIGS15		"Terminated"
SIGS16		NULL
SIGS17		"Stopped (signal)"
SIGS18		"Stopped"
SIGS19		"Continued"
SIGS20		"Child exited"
SIGS21		"Stopped (tty input)"
SIGS22		"Stopped (tty output)"
SIGS23		"Tty input interrupt"
SIGS24		"Cputime limit exceeded"
SIGS25		"Filesize limit exceeded"
SIGS26		NULL

DISPOS		"\nDisposition? "
EXP2            "1 message exploded from digest %d\n"

HLPMSG		"The %s program emulates many of the commands found in the Rand MH\nsystem.  Instead of operating on MH folders, commands to %s concern\na \
single file.\n\nTo see the list of commands available, just type a ``?'' followed by\nthe RETURN key.  To find out what switches each command takes, type\nthe \
name of the command followed by ``-help''.  To leave %s, use the\n``quit'' command.\n\nAlthough a lot of MH commands are found in %s, not all are fully\n\
implemented.  %s will always recognize all legal switches for a\ngiven command though, and will let you know when you ask for an\noption that it is unable to \
perform.\n\nRunning %s is fun, but using MH from your shell is far superior.\nAfter you have familiarized yourself with the MH style by using %s,\nyou should \
try using MH from the shell.  You can still use %s for\nmessage files that aren't in MH format, such as BBoard files.\n";
TOTAL           "TOTAL= %*d messages in %d folders.\n"
TOTAL2          "TOTAL= %*d messages in 1 folder.\n"
TOTAL3          "TOTAL= %*d message in %d folders.\n"
TOTAL4          "TOTAL= %*d message in 1 folder.\n"
HEADER          "\t\tFolder  %*s# of messages (%*srange%*s); cur%*smsg  (other files)\n"
NOTANY          "has   no messages%*s"
ONEMSG2         "has %*d message  (%*d-%*d)"
MULTIPLE        "has %*d messages (%*d-%*d)"
OTHERS1         ";%*s (others)"
ENDFORW         "\n------- End of Forwarded Messages\n\n"
ENDFORW2        "\n------- End of Forwarded Message\n\n"
EMPTY4          "%*d  empty\n"
WERROR3         "error writing to file %s"
INTERR3         "internal error!"
ONEHIT          "1 hit\n"
HITS            "%d hits\n"
UNEXP           "%s unexpected"
NODIS           "missing disjunctive"
NOCON           "missing conjunctive"
NONEG           "missing negation"
NOGRP2          "missing group"
NORBRC          "missing -rbrace"
PATERR          "pattern error in %1$s %2$s"
NOPARS          "unable to parse %1$s %2$s"
NOACSTOR        "unable to allocate component storage"
OR              "OR\n"
AND             "AND\n"
NOT             "NOT\n"
PATT            "PATTERN(%1$s) %2$s\n"
HEAD            "header"
MBODY           "body"
DRFT            "draft"
TEMP            "TEMPORAL(%1$s) %2$s: %3$s\n"
AFTER           "after"
BEFORE          "before"
UNKNWN          "UNKNOWN(0x%x)\n"
BOTCH           "advance() botch -- you lose big"
FORMERR         "format error in message %d"
NOPARS2         "unable to parse %1$s field in message %2$d, matching..."
AERROR2         "aliasing error in %1$s - %2$s"
BADWIDTH        "impossible width %d"
NOWRTDN         "unable to write /dev/null"
NOOPNDN         "unable to open /dev/null"
PROCESSD        "Message Processed\n"
NEEDADD         "%s: field requires at least one address"
ORIG            "Original-"
NOGRPS          "%s: field does not allow groups"
DUNKWN          "-deliver unknown"
LOSE            "getname () failed -- you lose extraordinarily big"
BADSIGN         "bad signature '%s'"
NODATE          "message has no Date: header"
NOFROM          "message has no From: header"
NOADDRS         "no addressees"
ADDRSFORM       "-------\n\t-- Addresses --\nlocal:\t"
COPFORM         "\n\t-- Folder Copies --\nfcc:\t"
MMDFPROB        "problem initializing MMDF system [%s]"
SADDRPROB       "problem with sender address [%s]"
SERVPROB        "problem initializing server; %s"
ADDRVER         " -- Address Verification --\n"
LRECIPS         "  -- Local Recipients --\n"
URECIPS         "  -- UUCP Recipients --\n"
NRECIPS         "  -- Network Recipients --\n"
ADDRVEROK       " -- Address Verification Successful --\n"
POST            " -- Posting for %s Recipients --\n"
BLIND           "Blind"
SIGHTED         "Sighted"
POSTALL         " -- Posting for All Recipients --\n"
COPIES          " -- %s Recipient Copies Posted --\n"
COPIES2         " -- Recipient Copies Posted --\n"
PROB            "problem ending addresses [%s]\n"
PROB2           "problem ending addresses; %s"
PROB3           "problem writing text [%s]\n"
PROB4           "problem writing text; %s\n"
PROB5           "problem reading from %s"
PROB6           "problem ending text [%s]\n"
PROB7           "problem getting submission status [%s]\n"
YOULOSE2        "you lose; %s"
NODELIV         "no delivery occurred; %s"
TRYAGAIN        "try again later; %s"
NOTHING         "nothing done; %s"
UNXPRESP        "unexpected response;\n\t[%1$s] -- %2$s"
FAILED          "posting failed; %s"
UNEXPRESP       "unexpected response; %s"
NOTSUPP         "not supported; UUCP address"
PROB8           "problem submitting address [%s]"
ADDOK           "address ok\n"
QUEUED          "nameserver timeout - queued for checking\n"
TMPNFAIL        "temporary nameserver failure"
NOTDEL          "not deliverable"
LATER           "try again later"
NOTHING2        "nothing done"
UNXPRESP2       "unexpected response"
NOTDEL2         "not deliverable; unknown user\n"
NOTDEL3         "not deliverable; unknown system\n"
NOTDEL4         "not deliverable; unknown host\n"
AT              "%1$s at %2$s"
LOSES           "loses; %s\n"
FORMAT1         " -- Filing Folder Copies --\n"
FORMAT2         " -- Folder Copies Filed --\n"
FOLDOK          "folder ok\n"
NOTDEL5         "not deliverable; unknown address\n"
HOOK2           "(invoking hook)\n\t"
TRANSERR        "error in transmission; unable to open maildrop\n"
TRANSERR2       "error in transmission; write to maildrop failed\n"
SENT            "sent\n"
NOHOOK          "unable to invoke hook; fork() failed\n"
ACCEPTED        "accepted\n"
HOOKERR         "%s error on hook; status=0%o\n"
SYSTEM          "system"
USER            "user"
POPENFAIL       "unable to start uux; popen() failed\n"
TRANSERR3       "error in transmission; write to uux failed\n"
TRANSERR4       "error in transmission; read failed\n"
QUEUED2         "queued (via uux)\n"
NODUPFD         "unable to dup fd"
NOFILTER        "unable to filter mail(%1$d), examine %2$s"
NOQUEUE         "  %1$s at %2$s: unable to get queue file\n"
QUEUED3         "  %1$s at %2$s: queued\n"
TRANSERR5       "error in transmission; write to temporary failed"
TRANSERR6       "error in transmission; unable to queue message\n"
ENTERTXT        "\n--------Enter initial text\n\n"
ENTERTXT2       "\n--------Enter additional text\n\n"
POORFMT         "skeleton is poorly formatted"
HELPSW7         "%s [switches] [switches for postproc] address ..."
USAGE5          "usage: %s [switches] [switches for postproc] address ..."
NOACBUF         "unable to allocate component buffers"
NOACBSTACK      "unable to allocate component buffer stack"
NOACBUF2        "unable to allocate component buffer"
RETD            "m_getfld() returned %d"
NONAMEARG       "missing argument name to %s"
NOEXIST2        "folder %s doesn't exist"
EMPTY5          "empty file"
NOFILEMSG       "can't file message %d"
HELPSW8         "%s [command ...]"
HELPSW9         "%s [msgs] [switches] +folder ..."
ONEFOLDER2      "only one source folder at a time!"
MAXFILES        "only %d files allowed!"
MAXFOLDERS      "only %d folders allowed!"
NOFOLDER        "no folder specified"
CHOICE          "use -file or some messages, not both"
NOAVECTOR       "unable to allocate exec vector"
MEXISTS         "message %1$s:%2$s already exists"
LINKINGERR      "error linking %1$s to %2$s"
HELPSW10        "%s: [+folder] [msg] [switches]"
BADADDRS        "\nrepl: bad addresses:\n"
BADADDR2        "\tBAD ADDRRESS: %s\n"
REPLY           "Reply to %s <yes/no>? "
NOREMOVE2       "sorry, you can't remove the current working directory"
CURRENT         "[+%s now current]\n"
DEREF           "[+%s de-referenced]\n"
NOPROFILE       "you have no profile entry for the %1$s folder +%2$s"
UNREADABLE      "unreadable"
RO              "read-only"
NOREADF2        "unable to read folder +%s"
NODELETE        "file \"%s/%s\" not deleted"
NOUNLINK2       "unable to unlink %1$s:%2$s"
NOTGONE         "folder +%s not removed"
UNREADABLE2     "%*d  unreadable\n"
FOLDER          "Folder %-32s%s\n\n"
FERR            "??Format error (message %d) in "
FERR2           "??Format error in "
COMP            "component %d\n"
RETURN          "getfld() returned %d\n"
HELPSW11        "%s [file] [switches]"
NOSTAT2         "unable to stat draft file %s"
WINS            ", wins.\n"
LOSES2          ", loses"
TERM            ", timed-out; terminated\n"
NOINSTALL	"[install-mh aborted]"
NOOPENERR	"error opening"
YOURMH		"Your MH-directory \""
NOEXISTMH	"\" doesn't exist; Create it <yes/no>? "
NOACCESSMH	"unable to access MH-directory \"%s\""
BADVALUE	"bad value for $%s: %s"
FOPENSETS	"FOpen of %s sets %d\n"
FCLSEMU		"FClose emulating close of %d (%d)\n"
NOPGRP		"unable to determine tty's process group"
LOSTPEER	"lost peer"

$ These are the help messages from uip/vmh.c.  Do not add to them or
$ rearrange them.
HELPMSG		"		forward		backwards"
HELPMSG1	"		-------		---------"
HELPMSG2	"next screen	SPACE"
HELPMSG3	"next %d line	RETURN		y"
HELPMSG4	"next %d line	EOT		u"
HELPMSG5	"go		g		G"
HELPMSG6	""
HELPMSG7	"refresh		CTRL-L"
HELPMSG8	"quit		q"
HELPMSG9	NULL

$ NOTE TO TRANSLATOR: These are the same help messages as above.
$ Messages 3 and 4 will need to be pluralized.
PLHELPMSG		"		forward		backwards"
PLHELPMSG1	"		-------		---------"
PLHELPMSG2	"next screen	SPACE"
PLHELPMSG3	"next %d lines	RETURN		y"
PLHELPMSG4	"next %d lines	EOT		u"
PLHELPMSG5	"go		g		G"
PLHELPMSG6	""
PLHELPMSG7	"refresh		CTRL-L"
PLHELPMSG8	"quit		q"
PLHELPMSG9	NULL

DIHELP1         "%s [msgs] [switches]"
SORRY           "sorry, no folders allowed!"
NODFORM         "message %d not in digest format\n"
FIHELP1         "%s +folder... [msgs] [switches]"
SORRY1          "sorry, -%s not allowed!"
ONLYONE         "only one folder at a time!\n"
ONLYONE1        "only one message at a time!\n"
NONULL          "null folder names are not permitted"
HEADER1         "\t\tFolder  %*s# of messages (%*srange%*s); cur%*smsg\n"
NOREADFF        "unable to read default filter file %s"
NOCREATE1       "unable to create "
FORMEND         "\n\n------- End of Forwarded Messages\n"
FORMEND2        "\n\n------- End of Forwarded Message\n"
NONELEFT        "no messages remaining in +%s"
NONELEFT2       "no messages remaining in %s"
SHHELP1         "%1$s %2$s[switches] [switches for showproc]"
USAGE6          "usage: %s [switches] [switches for showproc]\n"
MESG2           "(Message %d"
LIST1           "Press <return> to list \"%d\"..."

SYN             "syntax: %s\n"
SWITCHES        "  switches are:\n"
VER             "\nversion: %s\n"
MSG_1	"You have old old-style bell mail waiting\n"
MSG_2	"You have new old-style bell mail waiting\n"
MSG_3	"You have old old-style bell and old Internet mail waiting\n"
MSG_4	"You have old old-style bell and new Internet mail waiting\n"
MSG_5	"You have new old-style bell and old Internet mail waiting\n"
MSG_6	"You have new old-style bell and new Internet mail waiting\n"
MSG_7	"You have old Internet mail waiting\n"
MSG_8	"You have new Internet mail waiting\n"
MSG_9	"%s has old old-style bell mail waiting\n"
MSG_10	"%s has new old-style bell mail waiting\n"
MSG_11	"%s has old old-style bell and old Internet mail waiting\n"
MSG_12	"%s has old old-style bell and new Internet mail waiting\n"
MSG_13	"%s has new old-style bell and old Internet mail waiting\n"
MSG_14	"%s has new old-style bell and new Internet mail waiting\n"
MSG_15	"%s has old Internet mail waiting\n"
MSG_16	"%s has new Internet mail waiting\n"
MSG_17	"You don't have any mail waiting\n"
MSG_18	"%s doesn't have any mail waiting\n"
MSG_19	"%1$s doesn't have any mail waiting on %2$s\n"
MSG_20	"%1$s has %2$d messages (%3$d bytes) on %4$s\n"
MSG_21	"%1$s has 1 message (%2$d bytes) on %3$s\n"
MSG_22	"You don't have any mail waiting on %s\n"
MSG_23	"You have %1$d messages (%2$d bytes) on %3$s\n"
MSG_24	"You have 1 message (%1$d bytes) on %2$s\n"
MSH_READ1	"Reading %1$s, currently at message %2$d of %3$d\n"
MSH_READ2	"Reading, currently at message %1$d of %2$d\n"
MSH_READ3	"Reading +%1$s, currently at message %2$d of %3$d\n"
POST_MSG1	"1 address unparsable, 1 addressee undeliverable"
POST_MSG2	"1 address unparsable, %d addressees undeliverable"
POST_MSG3	"%d addresses unparsable, 1 addressee undeliverable"
POST_MSG4	"%1$d addresses unparsable, %2$d addressees undeliverable"
POST_MSG5	"1 address unparsable"
POST_MSG6	"%d addresses unparsable"
POST_MSG7	"1 addressee undeliverable"
POST_MSG8	"%d addressees undeliverable"

$ These messages were added for X400 enhancements
SHDATE		"Bad value in short form date."
BADDATE1	"Invalid mm/dd/yy[yy] format."
BADDATE2	"Invalid dd.mm.yy[yy] format."
DAY		"day"
MONTH		"month"
YEAR		"year"
MEMORY		"Out of memory."
NOPARTS		"No recognizable date parts found."
TIMEPAST	"Time is in the past."
TIME		"time"
REPERROR	"Repeated %s argument: \"%s\""
AMBABBR		"Ambiguous abbreviation: \"%.*s\""
AMPMTIME	"Can only use \"am\" or \"pm\" after a time."
AMPMCLOCK	"Can't use \"am\" or \"pm\" with 24 hour clock."
TIMEMOD		"time modifier"
WEEKDAY		"Can only use \"week\" after a weekday name."
DOM		"day of month"
BADYEAR		"Year out of range."
BADTIME1	"Illegal time: %02d:%02d:%02d"
BADTIME2	"Illegal time: %02d:%02d:%02d %s"
BADMONTHD	"Month day out of range. (> %d)"
NUMBER1		"or a number\n"
NUMBER2		"or a number."
$ Do Not translate the date in the following line
LIKE		"Something like: Jan 10 10:30:01 1990\n"
HEADBUF		"header buffer='%s'\n"
NOKNOWN		"%s '%s' unknown.\n"
HELPX400	"Hit <CR> for help.\n"
REENTER1	"Please re-enter the %s field: "
REENTER2	"Please re-enter the date: "
ISBAD		" is bad\n"
PROBLEM		"Problem with %s '%s': %s\n"
CTIME		"ctime() failed, this header is ignored"
$ End x400 messages.
$ Do not translate the word alias in the next line.
ALIFILE		"alias aliasfile"
BADDATE3	"Invalid mm dd,yy[yy] format."
NO		"no"
YES		"yes"
LIST2		"list"
