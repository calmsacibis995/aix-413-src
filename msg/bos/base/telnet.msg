$ ENGLISH_SOURCE_VERSION= 1.15
$ @(#)99	1.2  src/msg/bos/base/telnet.msg, bosmsg, msg411, GOLD410 4/26/94 17:06:59
$ 
$ COMPONENT_NAME: TCPIP telnet.msg
$ 
$ FUNCTIONS: ENTER, auditproc, command, displayed, fork,  
$            name, newline, option, perror, setsockopt, setuid, 
$            socket, vt1, vt2 
$
$ ORIGINS: 27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for telnet
$

$set MS_TELNET

$quote "

FS_GOT_CMD      "\r\n>got cmd %02x<\r\n"
$ FS_GOT_CMD      This error message is printed if telnet receives a 3270
$ FS_GOT_CMD      Select, Sense, or Sense Id command from the remote host.
$ FS_GOT_CMD      The command is ignored.
FS_ILL_CMD      "\r\n>Illegal cmd %02x<\r\n"
$ FS_ILL_CMD      This error message is printed if telnet receives a 3270
$ FS_ILL_CMD      command that it does not understand.  The command is ignored.
NO_TERM         "No environment-specified terminal type.\n"
$ NO_TERM         This error message is displayed if the user did not specify
$ NO_TERM         a terminal type in the TERM environment variable.
$ NO_TERM         TERM should be set to the type of terminal being used.
NO_TERMINFO     "No terminfo entry for \"%s\".\n"
$ NO_TERMINFO     This error message is displayed if there is not a
$ NO_TERMINFO     terminfo definition for the terminal type found in
$ NO_TERMINFO     the TERM environment variable.
NO_PEN_SEL      "\rSorry, no pen select yet\r"
$ NO_PEN_SEL      This message informs the user that the pen selection
$ NO_PEN_SEL      capability has not been implemented.  This message is
$ NO_PEN_SEL      displayed while using curses.  The \r causes the message
$ NO_PEN_SEL      to be displayed at the beginning of the current line and
$ NO_PEN_SEL      the cursor placed at the beginning of the error message
$ NO_PEN_SEL      when displayed.  When using curses, carriage returns
$ NO_PEN_SEL      are not mapped to carriage return/newline sequences.
X_SYSTEM_LABEL  "X SYSTEM"
$ X_SYSTEM_LABEL  This message is displayed when keyboard is locked.
$ X_SYSTEM_LABEL  (3270 emulation only.)  If the length of this field is
$ X_SYSTEM_LABEL  changed then the number of spaces in X_SYSTEM_BLANK
$ X_SYSTEM_LABEL  must be changed also.
X_SYSTEM_BLANK  "        "
$ X_SYSTEM_BLANK  This message is used to clear the area of the screen
$ X_SYSTEM_BLANK  where X_SYSTEM_LABEL is displayed. Currently row 0, col 9.
$ X_SYSTEM_BLANK  This field must be the same width as the size of the
$ X_SYSTEM_BLANK  X_SYSTEM_LABEL.
PRESS_ENTER     "\nPress ENTER to continue..."
$ PRESS_ENTER     This prompt message is displayed to prompt the user
$ PRESS_ENTER     to press the ENTER (return) key to continue after escaping
$ PRESS_ENTER     to telnet command mode.  This gives the user more time to
$ PRESS_ENTER     look at the command mode data before the screen is
$ PRESS_ENTER     refreshed with the session data as it was before the
$ PRESS_ENTER     escape to command mode.
$    profile.c
P_TIOCGETP      "TIOCGETP"
$ P_TIOCGETP      This perror message is displayed if the ioctl call fails.
$ P_TIOCGETP      Telnet exits.
INV_USER        "no such user: \"%s\"\n"
$ INV_USER        This error message is displayed if there is not an entry
$ INV_USER        in the password file for <arg 1>.  It is used by the routine
$ INV_USER        that loads 3270.key data.
MIS_IDENTIFIER  "misplaced identifier \"%s\" ignored\n"
$ MIS_IDENTIFIER  This error message is displayed during the parsing of
$ MIS_IDENTIFIER  the 3270.key data if an identifier <arg 1> is misplaced.
MISP_STRING     "misplaced string <%s> ignored\n"
$ MISP_STRING     This error message is displayed during the parsing of
$ MISP_STRING     the 3270.key data if a string <arg 1> was misplaced.
MISP_OR         "misplaced OR ignored\n"
$ MISP_OR         This error message is displayed during the parsing of
$ MISP_OR         the 3270.key data if an OR keyword was misplaced.
MISP_REF        "misplaced reference to %s ignored\n"
$ MISP_REF        This error message is displayed during the parsing of
$ MISP_REF        the 3270.key data if a function key reference is
$ MISP_REF        misplaced. <arg 1> is the name of the function key.
INC_BIND        "incomplete BIND command\n"
$ INC_BIND        This error message is displayed during the parsing of
$ INC_BIND        the 3270.key data if a BIND command is incomplete.
MISS_FILE_NAME  "missing file name\n"
$ MISS_FILE_NAME  This error message is displayed during the parsing of
$ MISS_FILE_NAME  the 3270.key data if a file name is missing.
INC_IF          "incomplete IF statement\n"
$ INC_IF          This error message is displayed during the parsing of
$ INC_IF          the 3270.key data if an IF statement is incomplete.
EOF_IN_ELSE     "end of file in ELSE statement\n"
$ EOF_IN_ELSE     This error message is displayed during the parsing of
$ EOF_IN_ELSE     the 3270.key data if an end of file is reached before
$ EOF_IN_ELSE     the ELSE statement is complete.
ILL_ACT         "*** ILLEGAL ACTION %08x ***\n"
$ ILL_ACT         This error message is displayed during the parsing of
$ ILL_ACT         the 3270.key data if <arg 1> is an illegal action.
ERROR_HEAD      "\"%s\", line %d: "
$ ERROR_HEAD      This format string is used to display information about
$ ERROR_HEAD      errors that occurred while processing 3270 profile data.
$ ERROR_HEAD      <arg 1> is the name of the file containing the error.
$ ERROR_HEAD      <arg 2> is the line number in the file.  This data is
$ ERROR_HEAD      followed by an error message that was passed to this
$ ERROR_HEAD      error routine.
ESC_NOT_VAL     "%s - \\e valid only in sequences : %s aborting ...\n"
$ ESC_NOT_VAL     This message is passed to the profile.c error routine.
$ ESC_NOT_VAL     It is prefaced with ERROR_HEAD.  <arg 1> is the name of
$ ESC_NOT_VAL     some terminal key.  Nothing is passed for <arg 2>, so
$ ESC_NOT_VAL     garbage is probably printed.  I think that this should
$ ESC_NOT_VAL     be the name of the running process.  (i.e. "tn", "telnet",
$ ESC_NOT_VAL     or whatever the telnet command is called on the user's
$ ESC_NOT_VAL     system.)
INV_DSP_COLOR   "%s -  invalid display color : \n"
$ INV_DSP_COLOR   This error message indicates that an unsupported color
$ INV_DSP_COLOR   was selected in a 3270 profile file.  The request was
$ INV_DSP_COLOR   ignored.
P_MALLOC_KEYMAP "can't allocate keymap"
$ P_MALLOC_KEYMAP This perror message indicates that an attempt to
$ P_MALLOC_KEYMAP allocate space for a keymap table failed.  perror()
$ P_MALLOC_KEYMAP will print additional information about the error.
NESTED_FILES    "files nested too deeply (> %d)\n"
$ NESTED_FILES    This error message indicates that the 3270 profiles
$ NESTED_FILES    recursively open too many files.  The profile files
$ NESTED_FILES    should be rewritten to keep them from being nested
$ NESTED_FILES    so deeply.  This message is printed after ERROR_HEAD.
ILL_CHAR        "Illegal character '%c' (%02x) ignored.\n"
$ ILL_CHAR        This error message indicates that a 3270 profile file
$ ILL_CHAR        contained an illegal character.  The character is printed
$ ILL_CHAR        as a character <arg 1> and in hex <arg 2>.
$ telnet.c
OFF_STRING      "off"
$ OFF_STRING      This string is displayed by the telnet display command
$ OFF_STRING      in the [] field for undefined values.  It indicates
$ OFF_STRING      that the named feature is turned off.  The user can also
$ OFF_STRING      enter this string to the set command as an alias for
$ OFF_STRING      the english word "off".  The string must not contain
$ OFF_STRING      white space (i.e. spaces).
P_SELECT        "select"
$ P_SELECT        This perror message is printed if an error occurs in
$ P_SELECT        the select() call.  perror() will print additional
$ P_SELECT        information about the error.
P_TTYFLUSH      "ttyflush "
$ P_TTYFLUSH      This perror message is printed if the ioctl() call to
$ P_TTYFLUSH      flush the tty returns an error.  perror() will print
$ P_TTYFLUSH      additional information about the error.
NET_T_DATA      "%c 0x%x\t"
$ NET_T_DATA         OBSOLETE MESSAGE                                  *
$ NET_T_DATA         Delete me                                         *
NET_T_DATA2     "%.2x"
$ NET_T_DATA2        OBSOLETE MESSAGE                                      *
$ NET_T_DATA2        Delete me                                      *
$ NET_T_DATA2        Delete me                                     *
$ NET_T_DATA2        Del me      .
NET_T_EOL       "\n"
$ NET_T_EOL          OBSOLETE MESSAGE                               *
$ NET_T_EOL          Delete me                                        *
$ NET_T_EOL          Delete me                 *
$     The following messages are used when 'option' has been set.
$     ("set option").  NREPLYSTR and YREPLYSTR are printed in the second
$     printf field in the format strings below.  SENDTAG and RCVDTAG are
$     printed in the first printf field.
NREPLYSTR                "(don't reply)"
YREPLYSTR                "(reply)"
SENDTAG                  "SEND"
RCVDTAG                  "RCVD"
$     The following messages show the 'option' processing.  The arguments
$     for the two printf string fields will be from the two sets of messages
$     described above.  The messages are grouped into sets of 5.  The
$     difference in the messages in each set is the action (do, dont,
$     will, wont, ???).  The do and dont messages indicate that one system is
$     telling the other that it would like for the other system to do or
$     not do some option.  The will and wont messages indicate the one
$     system is telling the other that it will or won't do the requested option.
$     The '???' messages indicate that the action to perform is not understood
$     by the local system.
$     Many of these options have no meaning to this version of telnet.
$     If telnet receives a request for an option it does not support,
$     it simply replies with a wont message.
$ BINARY     These messages are for determining if the systems will send
$ BINARY     eight bit data.
BINARY_DO                "%s do BINARY %s\r\n"
BINARY_DONT              "%s dont BINARY %s\r\n"
BINARY_WILL              "%s will BINARY %s\r\n"
BINARY_WONT              "%s wont BINARY %s\r\n"
BINARY_UNKN              "%s ??? BINARY %s\r\n"
$ ECHO       These messages are used to determine which system will echo
$ ECHO       the typed characters passed to the remote system.
ECHO_DO                  "%s do ECHO %s\r\n"
ECHO_DONT                "%s dont ECHO %s\r\n"
ECHO_WILL                "%s will ECHO %s\r\n"
ECHO_WONT                "%s wont ECHO %s\r\n"
ECHO_UNKN                "%s ??? ECHO %s\r\n"
$ RCP         These messages are used to "prepare to reconnect".  This
$ RCP         doesn't seem to be used by this version of telnet.  However
$ RCP         it could receive a request for this option from another host.
RCP_DO                   "%s do RCP %s\r\n"
RCP_DONT                 "%s dont RCP %s\r\n"
RCP_WILL                 "%s will RCP %s\r\n"
RCP_WONT                 "%s wont RCP %s\r\n"
RCP_UNKN                 "%s ??? RCP %s\r\n"
$ SUPPRESS GO AHEAD   These messages deal with the SUPPRESS GO AHEAD option
$ SUPPRESS GO AHEAD   of telnet.
SUPPRESS_DO              "%s do SUPPRESS GO AHEAD %s\r\n"
SUPPRESS_DONT            "%s dont SUPPRESS GO AHEAD %s\r\n"
SUPPRESS_WILL            "%s will SUPPRESS GO AHEAD %s\r\n"
SUPPRESS_WONT            "%s wont SUPPRESS GO AHEAD %s\r\n"
SUPPRESS_UNKN            "%s ??? SUPPRESS GO AHEAD %s\r\n"
$ NAME         These messages deal with the NAMS (approximate message size)
$ NAME         option of telnet.  NAME seems to be a typo, since the acronym
$ NAME         would be NAMS.
NAME_DO                  "%s do NAME %s\r\n"
NAME_DONT                "%s dont NAME %s\r\n"
NAME_WILL                "%s will NAME %s\r\n"
NAME_WONT                "%s wont NAME %s\r\n"
NAME_UNKN                "%s ??? NAME %s\r\n"
$ STATUS       These messages deal with the STATUS option indicating
$ STATUS       if it will give status.
STATUS_DO                "%s do STATUS %s\r\n"
STATUS_DONT              "%s dont STATUS %s\r\n"
STATUS_WILL              "%s will STATUS %s\r\n"
STATUS_WONT              "%s wont STATUS %s\r\n"
STATUS_UNKN              "%s ??? STATUS %s\r\n"
$ TIMING MARK   These messages deal with the TM (timing mark) option
$ TIMING MARK   of telnet.
TM_MARK_DO               "%s do TIMING MARK %s\r\n"
TM_MARK_DONT             "%s dont TIMING MARK %s\r\n"
TM_MARK_WILL             "%s will TIMING MARK %s\r\n"
TM_MARK_WONT             "%s wont TIMING MARK %s\r\n"
TM_MARK_UNKN             "%s ??? TIMING MARK %s\r\n"
$ RCTE          These messages deal with the RCTE (remote controlled
$ RCTE          transmission and echo) option of telnet.
RCTE_DO                  "%s do RCTE %s\r\n"
RCTE_DONT                "%s dont RCTE %s\r\n"
RCTE_WILL                "%s will RCTE %s\r\n"
RCTE_WONT                "%s wont RCTE %s\r\n"
RCTE_UNKN                "%s ??? RCTE %s\r\n"
$ NAOL          These messages have to do with the NAOL (negotiate about
$ NAOL          output line width) option of telnet.
NAOL_DO                  "%s do NAOL %s\r\n"
NAOL_DONT                "%s dont NAOL %s\r\n"
NAOL_WILL                "%s will NAOL %s\r\n"
NAOL_WONT                "%s wont NAOL %s\r\n"
NAOL_UNKN                "%s ??? NAOL %s\r\n"
$ NAOP          These messages have to do with the NAOP (negotiate about
$ NAOP          output page size) option of telnet.
NAOP_DO                  "%s do NAOP %s\r\n"
NAOP_DONT                "%s dont NAOP %s\r\n"
NAOP_WILL                "%s will NAOP %s\r\n"
NAOP_WONT                "%s wont NAOP %s\r\n"
NAOP_UNKN                "%s ??? NAOP %s\r\n"
$ NAOCRD        These messages have to do with the "negotiate about CR
$ NAOCRD        disposition" option of telnet.
NAOCRD_DO                "%s do NAOCRD %s\r\n"
NAOCRD_DONT              "%s dont NAOCRD %s\r\n"
NAOCRD_WILL              "%s will NAOCRD %s\r\n"
NAOCRD_WONT              "%s wont NAOCRD %s\r\n"
NAOCRD_UNKN              "%s ??? NAOCRD %s\r\n"
$ NAOHTS        These messages have to do with the "negotiate about
$ NAOHTS        horizontal tabstops" option of telnet.
NAOHTS_DO                "%s do NAOHTS %s\r\n"
NAOHTS_DONT              "%s dont NAOHTS %s\r\n"
NAOHTS_WILL              "%s will NAOHTS %s\r\n"
NAOHTS_WONT              "%s wont NAOHTS %s\r\n"
NAOHTS_UNKN              "%s ??? NAOHTS %s\r\n"
$ NAOHTD        These messages have to do with the "negotiate about
$ NAOHTD        horizontal tab desposition" option of telnet.
NAOHTD_DO                "%s do NAOHTD %s\r\n"
NAOHTD_DONT              "%s dont NAOHTD %s\r\n"
NAOHTD_WILL              "%s will NAOHTD %s\r\n"
NAOHTD_WONT              "%s wont NAOHTD %s\r\n"
NAOHTD_UNKN              "%s ??? NAOHTD %s\r\n"
$ NAOFFD        These messages have to do with the "negotiate about
$ NAOFFD        formfeed disposition" option of telnet.
NAOFFD_DO                "%s do NAOFFD %s\r\n"
NAOFFD_DONT              "%s dont NAOFFD %s\r\n"
NAOFFD_WILL              "%s will NAOFFD %s\r\n"
NAOFFD_WONT              "%s wont NAOFFD %s\r\n"
NAOFFD_UNKN              "%s ??? NAOFFD %s\r\n"
$ NAOVTS        These messages have to do with the "negotiate about
$ NAOVTS        vertical tab stops" option of telnet.
NAOVTS_DO                "%s do NAOVTS %s\r\n"
NAOVTS_DONT              "%s dont NAOVTS %s\r\n"
NAOVTS_WILL              "%s will NAOVTS %s\r\n"
NAOVTS_WONT              "%s wont NAOVTS %s\r\n"
NAOVTS_UNKN              "%s ??? NAOVTS %s\r\n"
$ NAOVTD        These messages have to do with the "negotiate about
$ NAOVTD        vertical tab disposition" option of telnet.
NAOVTD_DO                "%s do NAOVTS %s\r\n"
NAOVTD_DONT              "%s dont NAOVTS %s\r\n"
NAOVTD_WILL              "%s will NAOVTS %s\r\n"
NAOVTD_WONT              "%s wont NAOVTS %s\r\n"
NAOVTD_UNKN              "%s ??? NAOVTS %s\r\n"
$ NAOLFD        These messages have to do with the "negotiate about
$ NAOLFD        output LF disposition" option of telnet.
NAOLFD_DO                "%s do NAOLFD %s\r\n"
NAOLFD_DONT              "%s dont NAOLFD %s\r\n"
NAOLFD_WILL              "%s will NAOLFD %s\r\n"
NAOLFD_WONT              "%s wont NAOLFD %s\r\n"
NAOLFD_UNKN              "%s ??? NAOLFD %s\r\n"
$ EXTEND ASCII  These messages deal with the "extended ascii character
$ EXTEND ASCII  set" option of telnet.
EXT_ASC_DO               "%s do EXTEND ASCII %s\r\n"
EXT_ASC_DONT             "%s dont EXTEND ASCII %s\r\n"
EXT_ASC_WILL             "%s will EXTEND ASCII %s\r\n"
EXT_ASC_WONT             "%s wont EXTEND ASCII %s\r\n"
EXT_ASC_UNKN             "%s ??? EXTEND ASCII %s\r\n"
$ LOGOUT        These messages deal with the "force logout" option of telnet.
LOGOUT_DO                "%s do LOGOUT %s\r\n"
LOGOUT_DONT              "%s dont LOGOUT %s\r\n"
LOGOUT_WILL              "%s will LOGOUT %s\r\n"
LOGOUT_WONT              "%s wont LOGOUT %s\r\n"
LOGOUT_UNKN              "%s ??? LOGOUT %s\r\n"
$ BYTE MACRO    These messages deal with the "byte macro" option of telnet.
BYTE_MAC_DO              "%s do BYTE MACRO %s\r\n"
BYTE_MAC_DONT            "%s dont BYTE MACRO %s\r\n"
BYTE_MAC_WILL            "%s will BYTE MACRO %s\r\n"
BYTE_MAC_WONT            "%s wont BYTE MACRO %s\r\n"
BYTE_MAC_UNKN            "%s ??? BYTE MACRO %s\r\n"
$ DATA ENTRY TERMINAL   These messages deal with the "data terminal entry"
$ DATA ENTRY TERMINAL   option of telnet.
DET_DO                   "%s do DATA ENTRY TERMINAL %s\r\n"
DET_DONT                 "%s dont DATA ENTRY TERMINAL %s\r\n"
DET_WILL                 "%s will DATA ENTRY TERMINAL %s\r\n"
DET_WONT                 "%s wont DATA ENTRY TERMINAL %s\r\n"
DET_UNKN                 "%s ??? DATA ENTRY TERMINAL %s\r\n"
$ SUPDUP        These messages deal with the "supdup protocol" option of
$ SUPDUP        telnet.
SUPDUP_DO                "%s do SUPDUP %s\r\n"
SUPDUP_DONT              "%s dont SUPDUP %s\r\n"
SUPDUP_WILL              "%s will SUPDUP %s\r\n"
SUPDUP_WONT              "%s wont SUPDUP %s\r\n"
SUPDUP_UNKN              "%s ??? SUPDUP %s\r\n"
$ SUPDUP OUTPUT    These messages deal with the "supdup output: option
$ SUPDUP OUTPUT    of telnet.
SUPOUT_DO                "%s do SUPDUP OUTPUT %s\r\n"
SUPOUT_DONT              "%s dont SUPDUP OUTPUT %s\r\n"
SUPOUT_WILL              "%s will SUPDUP OUTPUT %s\r\n"
SUPOUT_WONT              "%s wont SUPDUP OUTPUT %s\r\n"
SUPOUT_UNKN              "%s ??? SUPDUP OUTPUT %s\r\n"
$ SEND LOCATION    These messages deal with the "send location" option
$ SEND LOCATION    of telnet.
SEND_LOC_DO              "%s do SEND LOCATION %s\r\n"
SEND_LOC_DONT            "%s dont SEND LOCATION %s\r\n"
SEND_LOC_WILL            "%s will SEND LOCATION %s\r\n"
SEND_LOC_WONT            "%s wont SEND LOCATION %s\r\n"
SEND_LOC_UNKN            "%s ??? SEND LOCATION %s\r\n"
$ TERMINAL TYPE   These messages deal with the "terminal type" option of
$ TERMINAL TYPE   telnet.
DO_NAWS                 "%s do NEGOTIATE WINDOW SIZE %s\r\n"
DONT_NAWS               "%s dont NEGOTIATE WINDOW SIZE %s\r\n"
WILL_NAWS               "%s will NEGOTIATE WINDOW SIZE %s\r\n"
WONT_NAWS               "%s wont NEGOTIATE WINDOW SIZE %s\r\n"
UNK_NAWS		"%s ??? NEGOTIATE WINDOW SIZE %s\r\n"
TTYPE_DO                 "%s do TERMINAL TYPE %s\r\n"
TTYPE_DONT               "%s dont TERMINAL TYPE %s\r\n"
TTYPE_WILL               "%s will TERMINAL TYPE %s\r\n"
TTYPE_WONT               "%s wont TERMINAL TYPE %s\r\n"
TTYPE_UNKN               "%s ??? TERMINAL TYPE %s\r\n"
$ END OF RECORD   These messages deal with the "end of record" option
$ END OF RECORD   of telnet.
EOR_DO                   "%s do END OF RECORD %s\r\n"
EOR_DONT                 "%s dont END OF RECORD %s\r\n"
EOR_WILL                 "%s will END OF RECORD %s\r\n"
EOR_WONT                 "%s wont END OF RECORD %s\r\n"
EOR_UNKN                 "%s ??? END OF RECORD %s\r\n"
$ SAKOPT          These messages deal with the "support SAK" option of telnet.
DO_SAKFMT                "%s do SUPPORT SAK %s\r\n"
DONT_SAKFMT              "%s dont SUPPORT SAK %s\r\n"
WILL_SAKFMT              "%s will SUPPORT SAK %s\r\n"
WONT_SAKFMT              "%s wont SUPPORT SAK %s\r\n"
UNK_SAKFMT               "%s ??? SUPPORT SAK %s\r\n"
$ DECIMAL OPTION FORMAT    These messages show the value of a telnet option
$ DECIMAL OPTION FORMAT    if it is not one of the above standard options.
$ DECIMAL OPTION FORMAT    In these messages, <arg 2> is the decimal value
$ DECIMAL OPTION FORMAT    of the option and <arg 3> is the reply message above.
DO_DECFMT                "%s do %d %s\r\n"
DONT_DECFMT              "%s dont %d %s\r\n"
WILL_DECFMT              "%s will %d %s\r\n"
WONT_DECFMT              "%s wont %d %s\r\n"
UNK_DECFMT               "%s ??? %d %s\r\n"
SUB_TELOPT           "%s suboption TELOPT_TTYPE %s\r\n"
$ SUB_TELOPT           This message indicates that the terminal type
$ SUB_TELOPT           should be <arg 2>.  <arg 1> is the SENDTAG or
$ SUB_TELOPT           RCVDTAG from above.
SUB_NAWS	     "%s suboption TELOPT_NAWS %s\r\n"
$ SUB_NAWS           This message indicates that the window size 
$ SUB_NAWS			(width & height)
$ SUB_NAWS           should be <arg 2>.  <arg 1> is the SENDTAG or
$ SUB_NAWS           RCVDTAG from above.
SUB_UNKNOWN          "%s suboption ???(%d) %s\r\n"
$ SUB_UNKNOWN          This message indicates that the suboption being
$ SUB_UNKNOWN          negotiated is not know to AIX telnet.  <arg 2> is
$ SUB_UNKNOWN          the decimal value of the suboption and <arg 3> is
$ SUB_UNKNOWN          the suboption's argument. (i.e. the data being passed.)
CS_REM_NO_SAK   "Remote side does not support SAK\n"
$ CS_REM_NO_SAK   This information message indicates that the user
$ CS_REM_NO_SAK   attempted to send a SAK to the remote host, but the
$ CS_REM_NO_SAK   remote host does not support SAK.  The request was ignored.
INV_NO_ARGS     "need at least one argument for 'send' command\n"
$ INV_NO_ARGS     This message is printed if the user did not give an
$ INV_NO_ARGS     argument to the send command.
HELP_SEND       "'send ?' for help\n"
$ HELP_SEND       This message is printed after INV_NO_ARGS is printed.
$ HELP_SEND       It is only printed if the user did not give enough
$ HELP_SEND       argument to the send command.
UNK_SEND_ARG    "Unknown send argument '%s'\n'send ?' for help.\n"
$ UNK_SEND_ARG    This error message indicates that the user entered
$ UNK_SEND_ARG    an argument (<arg 1>) that is not a valid send argument.
AMB_SEND_ARG    "Ambiguous send argument '%s'\n'send ?' for help.\n"
$ AMB_SEND_ARG    This error message indicates that the user did not
$ AMB_SEND_ARG    enter enough of the argument (<arg 1>) to make it
$ AMB_SEND_ARG    unique.  "send ?" list list the valid arguments.
NO_ROOM_MSG     "There is not enough room in the buffer TO the network\nto process your request.  Nothing will be done.\n('send synch' will throw away most data in the network\nbuffer, if this might help.)\n"
SND_ERR_ARG_GONE "Telnet 'send' error - argument disappeared!\n"
$ SND_ERR_ARG_GONE This error message indicates that somehow the send
$ SND_ERR_ARG_GONE argument was trashed during preliminary process of
$ SND_ERR_ARG_GONE the send command.  The user should reenter the request.
$ SND_ERR_ARG_GONE This message should never occur.
$    The following SEND_* messages are the help messages for the telnet
$    send command.  The printf argument <arg 1> is the name of the send
$    argument that will cause telnet to send the associated request to
$    the remote host.
SEND_ABORT      "%s\tSend Telnet Abort output\n"
$ SEND_ABORT      This help message indicates that the command
$ SEND_ABORT      "send <arg 1>" will send a request to the remote host
$ SEND_ABORT      to abort the queued output.
SEND_AYT        "%s\tSend Telnet 'Are You There'\n"
$ SEND_AYT        This help message indicates that the command "send <arg 1>"
$ SEND_AYT        will send a query to the remote host to verify that
$ SEND_AYT        it is still connected and responding.
SEND_BREAK      "%s\tSend Telnet Break\n"
$ SEND_BREAK      This help message indicates that the command "send <arg 1>"
$ SEND_BREAK      will send a request to the remote host to send the
$ SEND_BREAK      'quit' character to the process running on the remote host.
SEND_ERASE      "%s\tSend Telnet Erase Character\n"
$ SEND_ERASE      This help message indicates that the command "send <arg 1>"
$ SEND_ERASE      will send a request to the remote host to send the user's
$ SEND_ERASE      erase character on the remote host to the process running
$ SEND_ERASE      on the remote.
SEND_E_LINE     "%s\tSend Telnet Erase Line\n"
$ SEND_E_LINE     This help message indicates that the command "send <arg 1>"
$ SEND_E_LINE     will send a request to the remote host to send the user's
$ SEND_E_LINE     kill character on the remote host to the process running
$ SEND_E_LINE     on the remote.
SEND_ESCAPE     "%s\tSend current escape character\n"
$ SEND_ESCAPE     This help message indicates that the command "send <arg 1>"
$ SEND_ESCAPE     will send the telnet escape character to the remote host
$ SEND_ESCAPE     rather than interpreting it on the local machine.
SEND_GO_AHEAD   "%s\tSend Telnet 'Go Ahead' sequence\n"
$ SEND_GO_AHEAD   This help message indicates that the command "send <arg 1>"
$ SEND_GO_AHEAD   will send a "Go Ahead" sequence to the remote host.
SEND_INTR       "%s\tSend Telnet Interrupt Process\n"
$ SEND_INTR       This help message indicates that the command "send <arg 1>"
$ SEND_INTR       will send a request to the remote host to send the
$ SEND_INTR       'interrupt' character to the process running on the
$ SEND_INTR       remote host.
SEND_NOP        "%s\tSend Telnet 'No operation'\n"
$ SEND_NOP        This help message indicates that the command "send <arg 1>"
$ SEND_NOP        will send the telnet "No Operation" sequence to the
$ SEND_NOP        remote host.
SEND_SAK        "%s\tSend Telnet Secure Attention Key\n"
$ SEND_SAK        This help message indicates that the command "send <arg 1>"
$ SEND_SAK        will send a SAK request (CSECURITY) to the remote host.
SEND_SYNCH      "%s\tPerform Telnet 'Synch operation'\n"
$ SEND_SYNCH      This help message indicates that the command "send <arg 1>"
$ SEND_SYNCH      will synchronize the data and send a DM (data mark) to the
$ SEND_SYNCH      remote host.
SEND_DISPLAY    "%s\tDisplay send options\n"
$ SEND_DISPLAY    This help message indicates that the command "send <arg 1>"
$ SEND_DISPLAY    will cause the list of valid send commands to be displayed.
SOCKET_DEBUG    "Cannot turn off socket debugging\n"
$ SOCKET_DEBUG    This message is printed indicating that debugging cannot
$ SOCKET_DEBUG    be turned off.  This message is only displayed if a
$ SOCKET_DEBUG    compiler flag was set defining NOT43 and should not
$ SOCKET_DEBUG    be set for AIX 3.0 and beyond.
TOG_FLUSH       "%s\ttoggle flushing of output when sending interrupt characters\n"
$ TOG_FLUSH       This help message indicates that the command
$ TOG_FLUSH       "toggle <arg 1>" will toggle the flag that causes telnet
$ TOG_FLUSH       to flush the network output when sending interrupt (intr
$ TOG_FLUSH       and break) to the remote.  <arg 1> currently is "autoflush".
TOG_INTR_URG    "%s\ttoggle automatic sending of interrupt characters in urgent mode\n"
$ TOG_INTR_URG    This help message indicates that the command
$ TOG_INTR_URG    "toggle <arg 1>" will toggle the flag that causes telnet
$ TOG_INTR_URG    to send interrupt characters (intr and break) in urgent
$ TOG_INTR_URG    mode.  <arg 1> is currently "autosynch".
TOG_MAP_CR      "%s\ttoggle mapping of received carriage returns\n"
$ TOG_MAP_CR      This help message indicates that the command
$ TOG_MAP_CR      "toggle <arg 1>" will toggle the flag that causes telnet
$ TOG_MAP_CR      to interpret carriage returns received from the remote
$ TOG_MAP_CR      as carriage return - newline sequences.  <arg 1> is
$ TOG_MAP_CR      currently "crmod".
TOG_REC_CNTL    "%s\ttoggle local recognition of certain control characters\n"
$ TOG_REC_CNTL    This help message indicates that the command
$ TOG_REC_CNTL    "toggle <arg 1>" will toggle the flag that causes certain
$ TOG_REC_CNTL    control characters such as <intr> to be processed by the
$ TOG_REC_CNTL    local system or just passed on the remote.  <arg 1> is
$ TOG_REC_CNTL    currently "localchars".
TOG_SOCK_DBG    "%s\t(debugging) toggle debugging\n"
$ TOG_SOCK_DBG    This help message indicates that the command
$ TOG_SOCK_DBG    "toggle <arg 1>" will turn off or on socket level
$ TOG_SOCK_DBG    debugging.  <arg 1> is currently "debug".
TOG_PR_HEX      "%s\t(debugging) toggle printing of hexadecimal network data\n"
$ TOG_PR_HEX      This help message indicates that the command
$ TOG_PR_HEX      "toggle <arg 1>" will turn on or off the printing of
$ TOG_PR_HEX      network data.  The data is always printed in hex.
$ TOG_PR_HEX      <arg 1> is currently "netdata".
TOG_VW_OPTS     "%s\t(debugging) toggle viewing of options processing\n"
$ TOG_VW_OPTS     This help message indicates that the command
$ TOG_VW_OPTS     "toggle <arg 1>" will toggle the flag to display
$ TOG_VW_OPTS     information about options as the local and remote
$ TOG_VW_OPTS     negotiate the options.  <arg 1> is currently options.
DSP_HELP        "%s\tdisplay help information\n"
$ DSP_HELP        This help message indicates that the command
$ DSP_HELP        "toggle <arg 1>" will display the help information.
$ DSP_HELP        <arg 1> is currently "?".  However, "help" is a pseudonym
$ DSP_HELP        for "?".  "help" does not currently appear in the output
$ DSP_HELP        of this option.
NO_TOG_ARG      "Need an argument to 'toggle' command.  'toggle ?' for help.\n"
$ NO_TOG_ARG      This error message indicates that the user did not enter
$ NO_TOG_ARG      an argument to the toggle command.
AMB_TOG_ARG     "'%s': ambiguous argument ('toggle ?' for help).\n"
$ AMB_TOG_ARG     This error message indicates that the user did not enter
$ AMB_TOG_ARG     enough of the argument to the toggle command to make the
$ AMB_TOG_ARG     argument unique.  The argument as entered by the user is
$ AMB_TOG_ARG     displayed by <arg 1>.
UNK_TOG_ARG     "'%s': unknown argument ('toggle ?' for help).\n"
$ UNK_TOG_ARG     This error message indicates that the user entered an
$ UNK_TOG_ARG     argument that is not valid.  The user should verify that
$ UNK_TOG_ARG     the argument typed is spelled correctly and is in the
$ UNK_TOG_ARG     list of arguments displayed by "toggle ?".  The argument
$ UNK_TOG_ARG     as entered by the user is displayed by <arg 1>.
$     The following messages are grouped into sets of four messages.  Each set
$     contains basically the same message except for capitalizaion and negation.
$     The messages where the id contains wILL and wONT are displayed by the
$ telnet display command.  Each set will be commented as one message.
FLUSH_ACT_WILL      "Will flush output when sending interrupt characters.\n"
FLUSH_ACT_wILL      "will flush output when sending interrupt characters.\n"
FLUSH_ACT_WONT      "Won't flush output when sending interrupt characters.\n"
FLUSH_ACT_wONT      "won't flush output when sending interrupt characters.\n"
$ FLUSH_ACT_*         These messages indicate if an interrupt request will
$ FLUSH_ACT_*         flush the output to the remote or not.
FL_WILL_INT_URG     "Will send interrupt characters in urgent mode.\n"
FL_wILL_INT_URG     "will send interrupt characters in urgent mode.\n"
FL_WONT_INT_URG     "Won't send interrupt characters in urgent mode.\n"
FL_wONT_INT_URG     "won't send interrupt characters in urgent mode.\n"
$ FL_*_INT_URG        These messages indicate how interrupt and quit
$ FL_*_INT_URG        characters will be sent to the remote host.
ACT_WILL_MAP_CR     "Will map carriage return on output.\n"
ACT_wILL_MAP_CR     "will map carriage return on output.\n"
ACT_WONT_MAP_CR     "Won't map carriage return on output.\n"
ACT_wONT_MAP_CR     "won't map carriage return on output.\n"
$ ACT_*_MAP_CR        These messages indicate if the local system will
$ ACT_*_MAP_CR        map carriage returns to <CR><NL> sequences on output
$ ACT_*_MAP_CR        or not.
ACT_WILL_REC_CNTL   "Will recognize certain control characters.\n"
ACT_wILL_REC_CNTL   "will recognize certain control characters.\n"
ACT_WONT_REC_CNTL   "Won't recognize certain control characters.\n"
ACT_wONT_REC_CNTL   "won't recognize certain control characters.\n"
$ ACT_*_REC_CNTL      These messages indicate whether or not the local
$ ACT_*_REC_CNTL      system will recognize certain control characters
$ ACT_*_REC_CNTL      such as kill and erase.
ACT_WILL_SOCK_DBG   "Will turn on socket level debugging.\n"
ACT_wILL_SOCK_DBG   "will turn on socket level debugging.\n"
ACT_WONT_SOCK_DBG   "Won't turn on socket level debugging.\n"
ACT_wONT_SOCK_DBG   "won't turn on socket level debugging.\n"
$ ACT_*_SOCK_DBG      These messages indicate whether or not socket
$ ACT_*_SOCK_DBG      leveling debugging will be done.
ACT_WILL_PR_HEX   "Will print hexadecimal representation of network traffic.\n"
ACT_wILL_PR_HEX   "will print hexadecimal representation of network traffic.\n"
ACT_WONT_PR_HEX   "Won't print hexadecimal representation of network traffic.\n"
ACT_wONT_PR_HEX   "won't print hexadecimal representation of network traffic.\n"
$ ACT_*_PR_HEX        These messages indicate whether or not network traffic
$ ACT_*_PR_HEX        will be dumped to a trace file.
ACT_WILL_SHOW_OPT   "Will show option processing.\n"
ACT_wILL_SHOW_OPT   "will show option processing.\n"
ACT_WONT_SHOW_OPT   "Won't show option processing.\n"
ACT_wONT_SHOW_OPT   "won't show option processing.\n"
$ ACT_*_SHOW_OPT      These messages indicate whether or not telnet will
$ ACT_*_SHOW_OPT      show the option negotiation with telnetd on the remote
$ ACT_*_SHOW_OPT      host.
$     The following messages are the help messages for the set command.
$     The printf argument is the name of the set option the message describes.
TOG_LECHO_CHAR  "%s\tcharacter to toggle local echoing on/off\n"
$ TOG_LECHO_CHAR  This help message describes the set command to set the
$ TOG_LECHO_CHAR  character that will cause the echoing of typed input to be
$ TOG_LECHO_CHAR  echoed either on the local system or the remote host.
$ TOG_LECHO_CHAR  Typing the echo character will switch between local and
$ TOG_LECHO_CHAR  remote echoing.  The printf argument is currently "echo".
TN_ESC_CHAR     "%s\tcharacter to escape back to telnet command mode\n"
$ TN_ESC_CHAR     This help message describes the set command to set the
$ TN_ESC_CHAR     character that will cause telnet to prompt the user for a
$ TN_ESC_CHAR     command to process.  This character is by default ^T, and
$ TN_ESC_CHAR     can also be set in the user's environment with the shell
$ TN_ESC_CHAR     variable TNESC.  The printf argument is currently "escape".
SET_LOCALCHARS  "\tThe following need 'localchars' to be toggled true\n"
$ SET_LOCALCHARS  This help message is displayed by the command "set ?"
$ SET_LOCALCHARS  before the rest of the set command help messages.  It
$ SET_LOCALCHARS  indicates that the following set commands are turned off
$ SET_LOCALCHARS  if no value has been supplied.  Some values are set by
$ SET_LOCALCHARS  default and others are set from ioctl(), (the UNIX stty
$ SET_LOCALCHARS  command).
ERASE_CHAR      "%s\tcharacter to cause an Erase Character\n"
$ ERASE_CHAR      This help message describes the set command to set the
$ ERASE_CHAR      character to erase the last character typed.  The default
$ ERASE_CHAR      character is the user's UNIX erase character.  The printf
$ ERASE_CHAR      argument is currently "erase".
ABRT_OUTPUT_CHAR "%s\tcharacter to cause an Abort Output\n"
$ ABRT_OUTPUT_CHAR This help message describes the set command to set the
$ ABRT_OUTPUT_CHAR character to cause the current output from the remote
$ ABRT_OUTPUT_CHAR system to be aborted.
INTR_CHAR       "%s\tcharacter to cause an Interrupt Process\n"
$ INTR_CHAR       This help message describes the set command to set the
$ INTR_CHAR       character to interrupt the running process on the remote.
$ INTR_CHAR       The default character is the user's UNIX intr character.
$ INTR_CHAR       The printf argument is currently "interrupt".
ERASE_LINE_CHAR "%s\tcharacter to cause an Erase Line\n"
$ ERASE_LINE_CHAR This help message describes the set command to set the
$ ERASE_LINE_CHAR character to erase the current line. (i.e. all character
$ ERASE_LINE_CHAR typed since the last newline.)The default character is the
$ ERASE_LINE_CHAR user's UNIX kill character.  The printf argument is
$ ERASE_LINE_CHAR currently "kill".
BREAK_CHAR      "%s\tcharacter to cause a Break\n"
$ BREAK_CHAR      This help message describes the set command to set the
$ BREAK_CHAR      character to send a break (quit) signal to the process
$ BREAK_CHAR      running on the remote.  The default character is the
$ BREAK_CHAR      user's UNIX quit character.  The printf argument is
$ BREAK_CHAR      currently "quit".
EOF_CHAR        "%s\tcharacter to cause an EOF \n"
$ EOF_CHAR        This help message describes the set command to set the
$ EOF_CHAR        character to send an EOF to the process running on the
$ EOF_CHAR        remote.  The default character is the user's UNIX EOF
$ EOF_CHAR        character (usually ^D).  The printf argument is "eof".
SAK_SEND_CHAR   "%s\tcharacter to cause remote host SAK sequence to be sent\n"
$ SAK_SEND_CHAR   This help message describes the set command to set the
$ SAK_SEND_CHAR   character to send the SAK (secure attention key) to
$ SAK_SEND_CHAR   remote host.  There is not a default value for this
$ SAK_SEND_CHAR   character.  The printf argument is "sak".
DSP_HELP_INFO   "?\tdisplay help information\n"
$ DSP_HELP_INFO   This help message describes the set command to list
$ DSP_HELP_INFO   the valid set command arguments.
DISP_HELP_SET   "Format is 'set Name Value'\n'set ?' for help.\n"
$ DISP_HELP_SET   This message is displayed if the user entered the
$ DISP_HELP_SET   command with the wrong number of arguments.
UNK_SET_ARG     "'%s': unknown argument ('set ?' for help).\n"
$ UNK_SET_ARG     This error message is displayed if the user entered an
$ UNK_SET_ARG     unknown set command.  The printf argument is the argument
$ UNK_SET_ARG     to the set command (set <arg1>).  The user should make
$ UNK_SET_ARG     sure the argument was spelled correctly.  "set ?" will
$ UNK_SET_ARG     list the valid set command arguments.
AMB_SET_ARG     "'%s': ambiguous argument ('set ?' for help).\n"
$ AMB_SET_ARG     This error message is displayed if the user did not enter
$ AMB_SET_ARG     enough of the set command argument to make it unique.
$ AMB_SET_ARG     The printf argument is the set command argument as the
$ AMB_SET_ARG     user typed it.
SET_CHAR_IS     "%s character is '%s'.\n"
$ SET_CHAR_IS     This information message confirms that the set command
$ SET_CHAR_IS     has set the set command argument (arg 1) to the requested
$ SET_CHAR_IS     value (arg 2).
SET_ASCII_2    "Cannot set %s to %s (%2x%2x).\nOnly ascii characters allowed.\n"
$ SET_ASCII_2     This error message is displayed if the user attempted to
$ SET_ASCII_2     set <arg 1> to an extended ascii value.  <arg 2> is the
$ SET_ASCII_2     character as the user entered it.  <arg 3> and <arg 4>
$ SET_ASCII_2     are the hex representation of the character.  Only standard
$ SET_ASCII_2     ascii characters are allowed.
SET_ASCII_1     "Cannot set %s to %s (%2x).\nOnly ascii characters allowed.\n"
$ SET_ASCII_1     This error message is essentially the same as SET_ASCII_2.
$ SET_ASCII_1     It is used when the extended ascii character is a one
$ SET_ASCII_1     byte character, therefore, there is not an <arg 4>.
MODE_FMT        "format is:  'mode Mode', where 'Mode' is one of:\n\n"
$ MODE_FMT        This message is displayed if the user request help for
$ MODE_FMT        the mode command or doesn't enter the correct number
$ MODE_FMT        of arguments for the mode command.  CHAT_ATA_TIME and
$ MODE_FMT        LINE_BY_LINE are printed immediately following this message.
CHAR_ATA_TIME   "%s\tcharacter-at-a-time mode\n"
$ CHAR_ATA_TIME   This help message is displayed by the mode command to
$ CHAR_ATA_TIME   indicate that entering "mode <arg 1>" will cause telnet
$ CHAR_ATA_TIME   to transmit each character as it is typed.
LINE_BY_LINE    "%s\tline-by-line mode\n"
$ LINE_BY_LINE    This help message is displayed by the mode command to
$ LINE_BY_LINE    indicate that entering "mode <arg 1>" will cause telnet
$ LINE_BY_LINE    to transmit only after a newline (i.e. <CR>) has been typed.
UNK_MODE        "Unknown mode '%s' ('mode ?' for help).\n"
$ UNK_MODE        This error message is displayed if the user requested
$ UNK_MODE        an unknown mode.  He/she should check the spelling of
$ UNK_MODE        <arg 1> and request help "mode ?" to list the valid modes.
AMB_MODE        "Ambiguous mode '%s' ('mode ?' for help).\n"
$ AMB_MODE        This error message is displayed if the user did not
$ AMB_MODE        enter enough of <arg 1> to make it unique in the list
$ AMB_MODE        of valid mode.  Currently, this message should not
$ AMB_MODE        happen since there are only two options and they are
$ AMB_MODE        unique on the first character.
EML_LIST       "format is:  'emulate Terminal', where 'Terminal' is one of:\n\n"
$ EML_LIST        This message is printed if the command was called without
$ EML_LIST        a terminal name to emulate or if help was requested for the
$ EML_LIST        command (i.e. "em ?" or "em help").
$ EML_LIST        The following three messages are displayed (EMULATE_OFF,
$ EML_LIST        EMULATE_VT100, and EMULATE_3270) after this message.
EMULATE_OFF     "%s\tturn off emulation\n"
$ EMULATE_OFF     This help message indicates that the command (<arg 1>) will
$ EMULATE_OFF     turn off emulation.
EMULATE_VT100   "%s\tDEC VT100 terminal\n"
$ EMULATE_VT100   This help message indicates that the command (<arg 1>) will
$ EMULATE_VT100   cause telnet to emulate a vt100.
EMULATE_3270    "%s\tIBM 3278 or 3277 terminal\n"
$ EMULATE_3270    This help message indicates that the command (<arg 1>) will
$ EMULATE_3270    cause telnet to emulate a 3270 type terminal.
HELP_3270_FMT   "%s terminal"
$ HELP_3270_FMT   This help message replaces EMULATE_3270 with information
$ HELP_3270_FMT   about the particular type of 3270 that the user's terminal
$ HELP_3270_FMT   can support.  <arg 1> is the name of the 3270 type that
$ HELP_3270_FMT   will fit in the number of lines and columns that the
$ HELP_3270_FMT   user's terminal can handle.  The expanded string is
$ HELP_3270_FMT   currently limited to 80 characters with the NULL at the end.
UNK_EML_TERM    "Unknown terminal '%s' ('emulate ?' for help).\n"
$ UNK_EML_TERM    This error message indicates that the user requested
$ UNK_EML_TERM    emulation of an unknown terminal type (<arg 1>).
AMB_EML_TERM    "Ambiguous terminal name '%s' ('emulate ?' for help).\n"
$ AMB_EML_TERM    This error message indicatest that user did not enter
$ AMB_EML_TERM    enough of the terminal type (<arg 1>) to be unique in the
$ AMB_EML_TERM    list.  (Currently the list is unique on the first
$ AMB_EML_TERM    character.  This message should not occur.  However,
$ AMB_EML_TERM    future versions may allow for more terminal types.)
BAD_EML_TERM    "Can't emulate a %s terminal on a %s terminal\n"
$ BAD_EML_TERM    This error message is displayed if the terminal type
$ BAD_EML_TERM    the user is requesting to emulate (<arg 1>) can not be
$ BAD_EML_TERM    emulated on the terminal that the user is using (<arg 2>).
EML_LINE_MODE   "Using line mode\n"
$ EML_LINE_MODE   This message is displayed after BAD_EML_TERM message
$ EML_LINE_MODE   if an attempt to emulate 3270 failed.
EML_VERIF       "Now emulating a %s terminal\n"
$ EML_VERIF       This message informs the user that terminal emulation
$ EML_VERIF       for <arg 1> terminal type has been successfully initialized.
NOT_EML_TERM    "Not emulating (native type %s)\n"
$ NOT_EML_TERM    This message informs the user that terminal emultaion
$ NOT_EML_TERM    is not being done and the terminal is operating in
$ NOT_EML_TERM    normal mode.
SEP_TLST_SETLST "\n"
$ SEP_TLST_SETLST This newline is printed to separate the toggle
$ SEP_TLST_SETLST information from the set information by the display
$ SEP_TLST_SETLST command.
DOSET_FMT       "[%s]\t%s.\n"
$ DOSET_FMT       This format string is used by the telnet display command
$ DOSET_FMT       to display values controlled by the telnet set command.
$ DOSET_FMT       <arg 1> is the printable representation of the character
$ DOSET_FMT       value and <arg 2> is the name of the feature.
EML_TERM        "Emulating a %s terminal\n"
$ EML_TERM        This message is used by the telnet display command to
$ EML_TERM        inform the user that terminal emulation for <arg 1>
$ EML_TERM        terminal type is being performed.
AMB_DSP_ARG     "?Ambiguous argument '%s'.\n"
$ AMB_DSP_ARG     This error message is displayed by the telnet display
$ AMB_DSP_ARG     command if the user did not give enough information
$ AMB_DSP_ARG     to make the argument <arg 1> unique.  The user should
$ AMB_DSP_ARG     try the command again with a less abbreviated argument.
UNK_DSP_ARG     "?Unknown argument '%s'.\n"
$ UNK_DSP_ARG     This error message is displayed by the telnet display
$ UNK_DSP_ARG     command if the user entered an invalid argument <arg 1>.
$ UNK_DSP_ARG     The user should verify that he has spelled the argument
$ UNK_DSP_ARG     he wants to display correctly.
SET_ESC_INFO   "Deprecated usage - please use 'set escape%s%s' in the future.\n"
$ SET_ESC_INFO    This information message informs the user that in the
$ SET_ESC_INFO    future he/she should use the set command.  The command
$ SET_ESC_INFO    "escape" may disappear in the future.  <arg 1> and
$ SET_ESC_INFO    <arg 2> are used to show the proper way to set the
$ SET_ESC_INFO    escape character to <arg 2>.
NEW_ESC_CHAR    "new escape character: "
$ NEW_ESC_CHAR        This message is used to prompt the user to enter a new
$ NEW_ESC_CHAR        escape character.  This message is only displayed if
$ NEW_ESC_CHAR        the user used the out of date command "escape" without
$ NEW_ESC_CHAR        an argument.
ESC_CHAR        "Escape character is '%s'.\n"
$ ESC_CHAR        This information message shows the user the current
$ ESC_CHAR        telnet escape character in a printable form (<arg 1>).
OLD_TOG_CR      "Deprecated usage - please use 'toggle crmod' in the future.\n"
$ OLD_TOG_CR      This information message informs the user that in the
$ OLD_TOG_CR      future he/she should use the toggle command rather
$ OLD_TOG_CR      than "crmod".  "crmod" may disappear in the future.
P_TN_FORK       "telnet: fork"
$ P_TN_FORK       This perror message indicates that an error occurred
$ P_TN_FORK       while telnet was attempting to fork().  The user had
$ P_TN_FORK       requested the suspend command ("z").  perror() will
$ P_TN_FORK       print additional information.  This error should rarely
$ P_TN_FORK       occur.
CONC_CLOSED     "Connection closed.\n"
$ CONC_CLOSED     This information message indicates the the current
$ CONC_CLOSED     connection has been closed by request of the user.
BYE             "bye"
$ BYE             This information message is displayed by the telnet
$ BYE             in response to a telnet quit command.  It indicates
$ BYE             that telnet is exitting as the user requested.
CONC_TO         "Connected to %s.\n"
$ CONC_TO         This information message shows the host (<arg 1>) that is
$ CONC_TO         currently connected to the local system.  It is displayed
$ CONC_TO         by the status command if the local system is connected to
$ CONC_TO         a host and by the open command if the connection was
$ CONC_TO         successful.
$     The following messages show the current operating mode.
TEL_COM_MODE    "Operating in telnet command mode.\n"
$ TEL_COM_MODE    This message indicates that telnet is operating in
$ TEL_COM_MODE    command mode.  There is currently no way of causing
$ TEL_COM_MODE    this message to print using the status command.
OP_CHAR_MODE    "Operating in character-at-a-time mode.\n"
$ OP_CHAR_MODE    This message displayed by the status command indicates
$ OP_CHAR_MODE    that telnet is transmitting each character to the remote
$ OP_CHAR_MODE    as it is typed.  Echoing is done by the remote host.
OP_CHAR_LOCAL   "Operating in character-at-a-time mode (local echo).\n"
$ OP_CHAR_LOCAL   This message displayed by the status command indicates
$ OP_CHAR_LOCAL   that telnet is transmitting each character to the remote
$ OP_CHAR_LOCAL   host as it is typed.  The local system will echo each
$ OP_CHAR_LOCAL   character as it is typed.
OP_LIN_REMOTE   "Operating in line-by-line mode (remote echo).\n"
$ OP_LIN_REMOTE   This message displayed by the status command indicates
$ OP_LIN_REMOTE   that telnet is transmitting a line of data at a time.
$ OP_LIN_REMOTE   The remote system will echo the line after a newline
$ OP_LIN_REMOTE   has been typed.
OP_LINE_MODE    "Operating in line-by-line mode.\n"
$ OP_LINE_MODE    This message displayed by the status command indicates
$ OP_LINE_MODE    that telnet is transmitting a line of data at a time.
$ OP_LINE_MODE    The local system will echo each character as it is typed.
OP_LINE_SUPR    "Operating in line-by-line mode (local echoing suppressed).\n"
$ OP_LINE_SUPR    This message displayed by the status command indicates
$ OP_LINE_SUPR    that telnet is transmitting a line of data at a time.
$ OP_LINE_SUPR    The characters will not be echoed by either system.
OP_INV_MODE1    "Operating in invalid mode?.\n"
$ OP_INV_MODE1    This message is not currently displayed.
OP_3270_MODE    "Operating in 3270 mode.\n"
$ OP_3270_MODE    This message displayed by the status command indicates
$ OP_3270_MODE    that telnet is operating in 3270 mode.
CATCH_SIG_LOC   "Catching signals locally.\n"
$ CATCH_SIG_LOC   This status information message indicates that telnet
$ CATCH_SIG_LOC   is catching the signals and processing them locally
$ CATCH_SIG_LOC   rather than passing the signals to the connected host.
NO_CONC         "No connection.\n"
$ NO_CONC         This information message displayed by the status command
$ NO_CONC         indicates that telnet is not connected to a host.
ALRDY_CONC      "?Already connected to %s\n"
$ ALRDY_CONC      This information message indicates that the user
$ ALRDY_CONC      attempted to open a connection to a host (<arg 1>)
$ ALRDY_CONC      that was already connected.
TO              "(to) "
$ TO              This message prompts the user for a host name to open
$ TO              a connection with if the user did not supply a host
$ TO              argument to the open command.
USAGE           "usage: %s host-name [port]\n"
$ USAGE           This message is displayed if the user passed too many
$ USAGE           argument to the telnet open command.  <arg 1> is the
$ USAGE           name of the command (i.e. "open").
NME_NT_FND	"telnet: Unknown host %s\n"
UNK_HOST        "telnet: Address %s misformed\n"
$ UNK_HOST        This error message indicates the user is attempting
$ UNK_HOST        to connect to a host (<arg 1>) that the local system
$ UNK_HOST        does not recognize.
BAD_PORT_NO     "%s: bad port number\n"
$ BAD_PORT_NO     This error message indicates that the user is attempting
$ BAD_PORT_NO     to use a portname (<arg 1>) that is unavailable for use
$ BAD_PORT_NO     by telnet.  The portname must be assigned for use by tcp.
TRYING          "Trying...\n"
$ TRYING          This information message lets the user know that the
$ TRYING          local system is attempting to connect with the requested host.
P_TN_SOCKET     "telnet: socket"
$ P_TN_SOCKET     This perror message indicates that the call to socket()
$ P_TN_SOCKET     failed.  perror() will print additional information
$ P_TN_SOCKET     about the error.
P_SETSOCK_DBG   "setsockopt (SO_DEBUG)"
$ P_SETSOCK_DBG   This perror message indicates that the call to
$ P_SETSOCK_DBG   setsockopt() for socket debugging failed.  perror() will
$ P_SETSOCK_DBG   print additional information about the error.
CONC_TO_ADD     "telnet: connect to address %s: "
$ CONC_TO_ADD     This error message indicates that the local system
$ CONC_TO_ADD     could not connect ot the host using address <arg 1>.
$ CONC_TO_ADD     perror() is called after this message is displayed to
$ CONC_TO_ADD     append additional information to this message.
TRYING_HOST     "Trying %s...\n"
$ TRYING_HOST     This information message is displayed to show the next
$ TRYING_HOST     address the system is trying to use to connect to the host.
P_TN_CONNECT    "telnet: connect"
$ P_TN_CONNECT    This perror message is displayed if the local system
$ P_TN_CONNECT    could not connect to the desired host and there are
$ P_TN_CONNECT    no more addresses to try.  perror() will print additional 
$ P_TN_CONNECT    information about the error.
CONC_CLSD_FORGN "Connection closed by foreign host.\n"
$ CONC_CLSD_FORGN This message indicates that the connection was closed
$ CONC_CLSD_FORGN by the foreign host.
HELP_FMT        "%-8s\t%s\n"
$ HELP_FMT        This format string is used to display the help information
$ HELP_FMT        for the basic telnet commands.  The width specification 8
$ HELP_FMT        is the length of "connect" which is the longest command
$ HELP_FMT        name. <arg 1> is the command name.  <arg 2> is the help 
$ HELP_FMT        information and comes from the following *_H_ID messages.
$ HELP_FMT        This format is used if the user requested help without any
$ HELP_FMT        arguments. ("?" or "help").
HELP_FMT_S      "%s\n"
$ HELP_FMT_S      This format string is used to display the help information
$ HELP_FMT_S      for the specific telnet commands.  If the user enters
$ HELP_FMT_S      "? <command> ...", only the help information for those
$ HELP_FMT_S      specific commands are displayed.  The name of the command
$ HELP_FMT_S      is not displayed.
CMD_HELP        "Commands may be abbreviated.  Commands are:\n\n"
$ CMD_HELP        This help message is displayed if the user request help
$ CMD_HELP        at the base level. ("?" or "help" alone.)  The telnet
$ CMD_HELP        commands and help strings are listed after this message.
OPEN_H_ID       "connect to a site"
$ OPEN_H_ID       This help message identifies the command to connect to
$ OPEN_H_ID       a remote system.  Currently the command is "open".
CLOSE_H_ID      "close current connection"
$ CLOSE_H_ID      This help string identifies the command to close a
$ CLOSE_H_ID      connect to the currently connected host.  The command
$ CLOSE_H_ID      is currently named "close".
DISPLAY_H_ID    "display operating parameters"
$ DISPLAY_H_ID    This help string identifies the command to display the
$ DISPLAY_H_ID    operating parameters, i.e. those values set by "set"
$ DISPLAY_H_ID    and "toggle".  The command is currently named "display".
EMULATE_H_ID    "emulate a vt100 or 3270 terminal"
$ EMULATE_H_ID    This help string identifies the command to turn on
$ EMULATE_H_ID    emulation of a vt100 or 3270.  The command is currently
$ EMULATE_H_ID    named "emulate".
MODE_H_ID       "try to enter line-by-line or character-at-a-time mode"
$ MODE_H_ID       This help string identifies the command to change the
$ MODE_H_ID       transmission mode.  The command is currently named "mode".
EXIT_H_ID       "exit telnet"
$ EXIT_H_ID       This help string identifies the command to exit telnet.
$ EXIT_H_ID       The command is currently named "quit".
SEND_H_ID       "transmit special characters ('send ?' for more)"
$ SEND_H_ID       This help string identifies the command to transmit
$ SEND_H_ID       special characters to the remote host.  The command
$ SEND_H_ID       is currently named "send".
SET_H_ID        "set operating parameters ('set ?' for more)"
$ SET_H_ID        This help string identifies the command to set the
$ SET_H_ID        operating parameters such as the interrupt and erase
$ SET_H_ID        characters.  The command is currently named "set".
STATUS_H_ID     "print status information"
$ STATUS_H_ID     This help string identifies the command to give some
$ STATUS_H_ID     status information.  (The machine telnet is connected
$ STATUS_H_ID     to, if any, and the telnet escape character.)  The
$ STATUS_H_ID     command is currently named "status".
TOGGLE_H_ID     "toggle operating parameters ('toggle ?' for more)"
$ TOGGLE_H_ID     This help string identifies the command to toggle the
$ TOGGLE_H_ID     operating parameters on or off.  (ex. CR mapping.)
$ TOGGLE_H_ID     This command is currently named "toggle".
Z_H_ID          "suspend telnet"
$ Z_H_ID          This help string identifies the command to suspend
$ Z_H_ID          telnet and run an interactive shell.  The command is
$ Z_H_ID          currently named "z".
HELP_H_ID       "print help information"
$ HELP_H_ID       This help string identifies the command to display the
$ HELP_H_ID       help messages for telnet commands.  The command is
$ HELP_H_ID       currently named "?".  "help" is a pseudonym for "?".
$ HELP_H_ID       "help" is not listed as a command by the "help" or "?"
$ HELP_H_ID       command but this help string will be displayed if "? help"
$ HELP_H_ID       or "help help" is entered.
OESCAPE_H_ID    "deprecated command -- use 'set escape' instead"
$ OESCAPE_H_ID    This help string is printed only if the use enters
$ OESCAPE_H_ID    "? escape".  This command is not the current method
$ OESCAPE_H_ID    for setting the escape character.  This string will
$ OESCAPE_H_ID    not appear in the general help list. (i.e. entering
$ OESCAPE_H_ID    "?" with no arguments.)
OCR_H_ID        "deprecated command -- use 'toggle crmod' instead"
$ OCR_H_ID        This help string is printed only if the use enters
$ OCR_H_ID        "? crmod".  This command is not the current method
$ OCR_H_ID        for toggling the mapping of carriage returns.  This
$ OCR_H_ID        string and the command name (crmod) will not appear
$ OCR_H_ID        in the general help list. (i.e. entering "?" with no
$ OCR_H_ID        arguments.)
AMB_HELP_CMD    "?Ambiguous help command %s\n"
$ AMB_HELP_CMD    This error message indicates that the user did not
$ AMB_HELP_CMD    enter enough of the command name for which to display help
$ AMB_HELP_CMD    to be unique.  The user should enter more of the
$ AMB_HELP_CMD    command name.  <arg 1> is the name of the command as
$ AMB_HELP_CMD    the user entered it.
INV_HELP_CMD    "?Invalid help command %s\n"
$ INV_HELP_CMD    This error message indicates that the user did not
$ INV_HELP_CMD    enter a valid command name for which to display help.
$ INV_HELP_CMD    The user should check the spelling and reenter or
$ INV_HELP_CMD    just enter "?" to get help for all commands.
TN_PROMPT       "%s> "
$ TN_PROMPT       This string is displayed by telnet to prompt the user
$ TN_PROMPT       for a telnet command.  The string <arg 1> is the command
$ TN_PROMPT       used to run telnet.  Usually "telnet" or "tn".
AMB_CMD         "?Ambiguous command\n"
$ AMB_CMD         This error message indicates that the user did not
$ AMB_CMD         enter enough of the command name to make the name unique.
INV_CMD         "?Invalid command\n"
$ INV_CMD         This error message indicates that the command name
$ INV_CMD         entered is not a valid name.  '?' will list the commands.
CONC_FIRST      "?Need to be connected first.\n"
$ CONC_FIRST      This error message indicates that the user attempted
$ CONC_FIRST      to execute a command that expects telnet is already
$ CONC_FIRST      connected to a remote system.  The user should open
$ CONC_FIRST      the remote host before trying the command again.
HOOK_BAD_BUS    "bad bus open, rc = %d \n"
$ HOOK_BAD_BUS  This error message indicates that the attempt to open the
$ HOOK_BAD_BUS  dynaprobe device, /dev/bus, failed.  The argument currently
$ HOOK_BAD_BUS  is the return code from fopen which is always a -1 when
$ HOOK_BAD_BUS  an error occurs. Telnet exits.
SLOG_AUDIT_PROC "telnet: auditproc: %m"
$ SLOG_AUDIT_PROC This syslog message is logged if auditproc() fails.
$ SLOG_AUDIT_PROC It is part of Csecurity.  Telnet exits.
P_SETUID        "setuid"
$ P_SETUID      This perror message is written if setuid() fails.  Telnet exits.
UNK_SERVICE     "telnet: tcp/telnet: unknown service\n"
$ UNK_SERVICE   This error message is displayed if telnet is not a valid
$ UNK_SERVICE   service on the local host.  Telnet exits.
INV_OCT_ESC     "Invalid octal escape value %s\n"
$ INV_OCT_ESC   The value in the TNESC shell variable was not a numeric
$ INV_OCT_ESC   string. The user should verify that the string displayed
$ INV_OCT_ESC   by the message (<arg 1>) is a string of octal digits.
$ INV_OCT_ESC   The escape character is now an ESC.
NULL_NT         ""
$ NULL_NT       An empty string is passed to perror if the attempt to open
$ NULL_NT       the user specified file for logging network data fails.
$ NULL_NT       The trace file is set to stdout.
USAGE_AIX       "Usage: %s [-d] [-n NetTrace] [-e terminal] [host] [port]\n"
$ USAGE_AIX     This message is output if the user uses invalid options
$ USAGE_AIX     on the command line.  The parameter <arg 1> is the command name.
BLANKLINE       "\n"
$ vt100.c
VT100_XLATE     "vt100(vt1): xlate invalid = %d\n"
$ VT100_XLATE     This error message indicates that something is out of
$ VT100_XLATE     sync in vt1().  vt1() is coded such that this message
$ VT100_XLATE     cannot occur.
C_CLSD_FRGN     "Connection closed by foreign host"
$ C_CLSD_FRGN     This message is logged to the security log.
EM_ASCII_ONLY   "\rOnly ascii characters supported in emulation mode.\r"
$ EM_ASCII_ONLY   This message is displayed to the user if he/she enters
$ EM_ASCII_ONLY   a non-ascii message while in emulation mode.
VT100_LINE	 "Line mode NOT supported while emulating VT-100.\n"
$ VT100_LINE	  This message is displayed if the user tries to go into
$ VT100_LINE	  line mode while emulation vt100.  This is NOT allowed. 
VT100_CHAR	"Selecting character-at-a-time mode.\n"
$ VT100_CHAR	 This msg is displayed when the user selects vt100 emul. and
$ VT100_CHAR	 telnet is currently in line mode.  Telnet will then negotiate
$ VT100_CHAR	 character mode.
$ The following messages are used in profile.c by the 3270 key binding parser.
$
P_NOBIND	"No 3270 key bindings for this terminal type - using defaults.\n"
$		This is a message displayed when telnet in 3270 mode cannot 
$		find specific key bindings for the specified terminal.
P_UNKNOWNMSG    "Unknown message id from input file\n"
$		This message is displayed when the profile parser cannot find
$		the specified message id to print.  This message SHOULD never
$		be displayed.
$
$ P44325 - Telnet freaks when lines < 25.
EML_REJECTED	"Host rejected selected terminal emulation\n"
$ EML_REJECTED	  The user selected a terminal emulation that the host will
$ EML_REJECTED	  not support. If the type is 'none', the native terminal
$ EML_REJECTED	  type has been rejected.
EML_REJ_ALL1	"Host rejected all offered terminal types,\n"
$ EML_REJ_ALL1    All the terminal types, native and emulated, have been
$ EML_REJ_ALL1    rejected by the host during Telnet subnegotiation.	
EML_REJ_ALL2	"check terminal type and window size.\n"
$ EML_REJ_ALL2 	  Either the native terminal type or all of the emulation 
$ EML_REJ_ALL2	  options are unsatisfactory, or the window size prevents 
$ EML_REJ_ALL2	  either the emulators or the host from continuing.
$ EML_REJ_ALL2    Use at least a 80 by 25 window and retry.
NO_CONVD	"iconv_open() failed to return a conversion descriptor.\n"
$ 		  This error message is displayed when iconv_open() is unable
$		  to return a conversion descriptor for the codesets.
$		  Do not translate the word iconv_open().
TOG_DEF_LT       "%s\ttoggle default line terminator\n"
$ TOG_DEF_LT      This help message indicates that the command
$ TOG_DEF_LT      "toggle <arg 1>" will toggle the flag that causes telnet
$ TOG_DEF_LT      to change the default line terminator output to the network
$ TOG_DEF_LT      to CR-LF. <arg 1> is currently "lineterm".
OLD_DEF_LT    "Deprecated usage - please use 'toggle lineterm' in the future.\n"
$ OLD_DEF_LT      This information message informs the user that in the
$ OLD_DEF_LT      future he/she should use the toggle command rather
$ OLD_DEF_LT      than "lineterm".  "lineterm" may disappear in the future.
OLT_H_ID        "deprecated command -- use 'toggle lineterm' instead"
$ OLT_H_ID        This help string is printed only if the use enters
$ OLT_H_ID        "? lineterm".  This command is not the current method
$ OLT_H_ID        for toggling the default line terminator.  This
$ OLT_H_ID        string and the command name (lineterm) will not appear
$ OLT_H_ID        in the general help list. (i.e. entering "?" with no
$ OLT_H_ID        arguments.)
ACT_WILL_CRLF_LT     "Will set default line terminator to CR-LF.\n"
ACT_WONT_CRLF_LT     "Won't set default line terminator to CR-LF.\n"
ACT_wILL_CRLF_LT     "will set default line terminator to CR-LF.\n"
ACT_wONT_CRLF_LT     "won't set default line terminator to CR-LF.\n"
$ ACT_*_CRLF_LT        These messages indicate if the local system will
$ ACT_*_CRLF_LT        set the default end-of-line terminator to CR-LF
$ ACT_*_CRLF_LT        on output to the network.
P_GETWINSIZE	"getwinsize:ioctl"
$ P_GETWINSIZE        This perror message is printed if an error occurs in
$ P_GETWINSIZE        the ioctl() call from getwinsize(). Usually this
$ P_GETWINSIZE        will happen if telnet is not started from a terminal.
