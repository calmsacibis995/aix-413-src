$ @(#)57	1.4.1.2  src/tenplus/termcap/Hkeys.map.msg, tenplus, tenplus41J, 9523C_all 6/8/95 14:31:33
$
$ COMPONENT_NAME: (TENPLUS) INed Editor
$
$ FUNCTIONS:
$
$ ORIGINS:  27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991, 1995
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ This file contains message catalog entries for keyboard layouts
$ provided in message stanza files (.msz).
$
$
$ TRANSLATOR:
$
$ Do not translate any lines that begin with a $ (dollar) value.
$
$ Each message catalog entry begins with a mnemonic which takes the form
$ K_<terminal>_<mnemonic-number>.
$ Example: K_DEFAULT_1 is the first message catalog entry for the default
$ keyboard. Do not translate these mnemonics.
$
$ The text within quotes (") which follows each mnemonic provides a message
$ catalog entry for the corresponding message stanza file (.msz). This text
$ must be translated, HOWEVER, do not translate INed Editor Functions.
$ These include Editor Functions (1) to (8) and all functions given in
$ UPPER CASE.
$ Example: (1), BACKSPACE, DELETE-CHARACTER and ZOOM-IN must not be translated.
$
$ Do not alter escape sequences. These are numeric sequences which represent
$ a single character value (usually a graphic character). Numeric sequences are 
$ prefixed with a \ character and followed by a numeric value.
$ Example: \021 \001 \023 \004 must not be altered.
$
$ Do not alter the length of any lines unless there is a comment below
$ the line which explains that the length may be altered.
$
$
$quote "
$
$set MS_FORMS
$
$
$ DEFAULT KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_DEFAULT_1 "\011\011\011   Keyboard - Default"
K_DEFAULT_4 "\011\011                    Input Sequences"
K_DEFAULT_6 " Editor Function     Key Sequence       Editor Function     Key Sequence"
K_DEFAULT_8 "\001(1)             \001Ctrl-n 1           \001 \001+LINE           \001Ctrl-n j           \001"
K_DEFAULT_9 "\001(2)             \001Ctrl-n 2           \001 \001-LINE           \001Ctrl-n d           \001"
K_DEFAULT_10 "\001(3)             \001Ctrl-n 3           \001 \001LINEFEED        \001Ctrl-n u           \001"
K_DEFAULT_11 "\001(4)             \001Ctrl-n 4           \001 \001LOCAL-MENU      \001Ctrl-n 9           \001"
K_DEFAULT_12 "\001(5)             \001Ctrl-n 5           \001 \001MARGIN          \001Ctrl-n m           \001"
K_DEFAULT_13 "\001(6)             \001Ctrl-n 6           \001 \001MENU            \001Ctrl-n Ctrl-m      \001"
K_DEFAULT_14 "\001(7)             \001Ctrl-n 7           \001 \001NEXT            \001Ctrl-n .           \001"
K_DEFAULT_15 "\001(8)             \001Ctrl-n 8           \001 \001NEXT-WINDOW     \001Ctrl-n Ctrl-w      \001"
K_DEFAULT_16 "\001up arrow        \001Ctrl-k             \001 \001+PAGE           \001Ctrl-p             \001"
K_DEFAULT_17 "\001right arrow     \001Ctrl-l             \001 \001-PAGE           \001Ctrl-n Ctrl-b      \001"
K_DEFAULT_18 "\001down arrow      \001Ctrl-g             \001 \001PICK-COPY       \001Ctrl-n Ctrl-d      \001"
K_DEFAULT_19 "\001left arrow      \001Ctrl-f             \001 \001PICK-UP         \001Ctrl-n Shift-y     \001"
K_DEFAULT_20 "\001BACKSPACE       \001Ctrl-o             \001 \001PREVIOUS        \001Ctrl-n p           \001"
K_DEFAULT_21 "\001BEGIN-LINE      \001Ctrl-n a           \001 \001PRINT           \001Ctrl-n Shift-p     \001"
K_DEFAULT_22 "\001BOX-MARK        \001Ctrl-b             \001 \001PUT-COPY        \001Ctrl-n Shift-c     \001"
K_DEFAULT_23 "\001BREAK  **       \001Ctrl-c             \001 \001PUT-DOWN        \001Ctrl-n Ctrl-f      \001"
K_DEFAULT_24 "\001CANCEL          \001Ctrl-n Ctrl-n      \001 \001QUIT            \001Ctrl-\\      **     \001"
K_DEFAULT_25 "\001CENTER          \001Ctrl-n c           \001 \001QUOTE           \001Ctrl-n q           \001"
K_DEFAULT_26 "\001DELETE-CHARACTER\001Ctrl-d             \001 \001REFRESH         \001Ctrl-n z           \001"
K_DEFAULT_27 "\001DELETE-LINE     \001Ctrl-n y           \001 \001REPLACE         \001Ctrl-n Ctrl-r      \001"
K_DEFAULT_28 "\001DO              \001Ctrl-n Ctrl-x      \001 \001RESTORE         \001Ctrl-r             \001"
K_DEFAULT_29 "\001END-LINE        \001Ctrl-e             \001 \001RETURN          \001Ctrl-m             \001"
K_DEFAULT_30 "\001ENTER           \001Ctrl-x             \001 \001RIGHT           \001Ctrl-n r           \001"
K_DEFAULT_31 "\001EXECUTE         \001Ctrl-x             \001 \001SAVE            \001Ctrl-n Shift-s     \001"
K_DEFAULT_32 "\001EXIT            \001Ctrl-n Ctrl-e      \001 \001+SEARCH         \001Ctrl-n s           \001"
K_DEFAULT_33 "\001FONT            \001Ctrl-n f           \001 \001-SEARCH         \001Ctrl-n b           \001"
K_DEFAULT_34 "\001FORMAT          \001Ctrl-n Shift-f     \001 \001SET-TAB         \001Ctrl-n t           \001"
K_DEFAULT_35 "\001GO-TO           \001Ctrl-n g           \001 \001TAB             \001Ctrl-i             \001"
K_DEFAULT_36 "\001HELP            \001Ctrl-n ?           \001 \001-TAB            \001Ctrl-n Ctrl-i      \001"
K_DEFAULT_37 "\001HOME            \001Ctrl-n h           \001 \001TEXT-MARK       \001Ctrl-t             \001"
K_DEFAULT_38 "\001INSERT-LINE     \001Ctrl-n o           \001 \001USE             \001Ctrl-u             \001"
K_DEFAULT_39 "\001INSERT-MODE     \001Ctrl-v             \001 \001WINDOW          \001Ctrl-w             \001"
K_DEFAULT_40 "\001LAST-ARG        \001Ctrl-n Ctrl-l      \001 \001ZOOM-IN         \001Ctrl-n Ctrl-j      \001"
K_DEFAULT_41 "\001LEFT            \001Ctrl-n l           \001 \001ZOOM-OUT        \001Ctrl-n Ctrl-k      \001"
K_DEFAULT_44 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_DEFAULT_45 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_DEFAULT_46 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_DEFAULT_47 "   is labeled \"quit\"."
$ Do not translate quit
$
K_DEFAULT_48 ""
$
$
$ LFT KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_LFT_1 "\011\011\011     Keyboard - lft"
K_LFT_7 "\011\011\011       Input Sequences" 
K_LFT_9 " Editor Function   Key Name             Editor Function   Key Name"
K_LFT_11 "\001BACKSPACE        \001Backspace        \001  \001PAGE UP          \001Alt-Page Up      \001"
K_LFT_12 "\001BACKTAB          \001Shift Tab        \001  \001PICK COPY        \001F9               \001"
K_LFT_13 "\001BEGIN LINE       \001Alt-Left arrow   \001  \001PICK UP          \001F7               \001"
K_LFT_14 "\001BOX MARK         \001Alt-b            \001  \001PREVIOUS         \001Alt-F11          \001"
K_LFT_15 "\001BREAK            \001Ctrl-c  **       \001  \001PRINT            \001Alt-Print Screen \001"
K_LFT_16 "\001CANCEL           \001Scroll Lock      \001  \001PUT COPY         \001F10              \001"
K_LFT_17 "\001CENTER           \001Alt-c            \001  \001PUT DOWN         \001F8               \001"
K_LFT_18 "\001CURSOR RIGHT     \001Right arrow      \001  \001QUIT             \001Ctrl-\\ **        \001"
K_LFT_19 "\001CURSOR UP        \001Up arrow         \001  \001QUOTE            \001Alt-q            \001"
K_LFT_20 "\001DELETE CHAR      \001Delete           \001  \001REFRESH          \001Alt-z            \001"
K_LFT_21 "\001DELETE LINE      \001Alt-Delete       \001  \001REPLACE          \001Alt-End          \001"
K_LFT_22 "\001DO               \001Alt-x            \001  \001RESTORE          \001Alt-Insert       \001"
K_LFT_23 "\001END LINE         \001Alt-Right arrow  \001  \001RETURN           \001Enter            \001"
K_LFT_24 "\001ENTER            \001Ctrl/Act         \001  \001RIGHT            \001Alt-r            \001"
K_LFT_25 "\001EXECUTE          \001Ctrl/Act         \001  \001SAVE             \001Alt-s            \001"
K_LFT_26 "\001EXIT             \001Alt-d            \001  \001SEARCH DOWN      \001Alt-Down arrow   \001"
K_LFT_27 "\001FONT             \001Alt-f            \001  \001SEARCH UP        \001Alt-Up arrow     \001"
K_LFT_28 "\001FORMAT           \001F5               \001  \001SET TAB          \001Alt-v            \001"
K_LFT_29 "\001GO TO            \001End              \001  \001TAB              \001Tab              \001"
K_LFT_30 "\001HELP             \001F1               \001  \001TEXT MARK        \001Alt-t            \001"
K_LFT_31 "\001HOME             \001Home             \001  \001(1)              \001Alt-F1           \001"
K_LFT_32 "\001INSERT LINE      \001F6               \001  \001(2)              \001Alt-F2           \001"
K_LFT_33 "\001INSERT MODE      \001Insert           \001  \001(3)              \001Alt-F3           \001"
K_LFT_34 "\001LAST ARG         \001Alt-a            \001  \001(4)              \001Alt-F4           \001"
K_LFT_35 "\001LEFT             \001Alt-l            \001  \001(5)              \001Alt-F5           \001"
K_LFT_36 "\001LINE DOWN        \001Page Down        \001  \001(6)              \001Alt-F6           \001"
K_LFT_37 "\001LINE UP          \001Page Up          \001  \001(7)              \001Alt-F7           \001"
K_LFT_38 "\001MARGIN           \001Alt-m            \001  \001(8)              \001Alt-F8           \001"
K_LFT_39 "\001MENU             \001F3               \001  \001USE              \001Alt-u            \001"
K_LFT_40 "\001NEXT             \001Alt-F12          \001  \001WINDOW           \001Alt-w            \001"
K_LFT_41 "\001NEXT WINDOW      \001Alt-n            \001  \001ZOOM IN          \001F11              \001"
K_LFT_42 "\001PAGE DOWN        \001Alt-Page Down    \001  \001ZOOM OUT         \001F12              \001"
K_LFT_45 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_LFT_46 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_LFT_47 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_LFT_48 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_LFT_49 ""
$
$
$ IBM3101 KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_IBM3101_1 "\011\011\011  Keyboard - ibm3101"
K_IBM3101_4 "CANCEL is ALT-RESET (Model 2x only)."
K_IBM3101_6 "The following switch setting must be in effect for the editor to work."
K_IBM3101_8 "\011Group 1           Group 2           Group 3"
K_IBM3101_10 "on \001 \001 \001x\001 \001 \001 \001x\001x\001 \001x\001 \001 \001 \001 \001 \001 \001 \001 \001x\001 \001x\001x\001 \001 \001 \001 \001"
K_IBM3101_12 "off\001x\001x\001 \001x\001x\001x\001 \001 \001 \001 \001x\001x\001x\001x\001x\001x\001x\001 \001 \001x\001 \001 \001x\001x\001x\001x\001"
K_IBM3101_16 "\011\011\011       Input Sequences"
K_IBM3101_18 " Editor Function     Key Sequence       Editor Function     Key Sequence"
K_IBM3101_20 "\001(1)             \001ESC 1              \001 \001LEFT            \001ESC l              \001"
K_IBM3101_21 "\001(2)             \001ESC 2              \001 \001+LINE           \001PF2                \001"
K_IBM3101_22 "\001(3)             \001ESC 3              \001 \001-LINE           \001PF5                \001"
K_IBM3101_23 "\001(4)             \001ESC 4              \001 \001LINEFEED        \001ESC return         \001"
K_IBM3101_24 "\001(5)             \001ESC 5              \001 \001LOCAL-MENU      \001ALT-{              \001"
K_IBM3101_25 "\001(6)             \001ESC 6              \001 \001MARGIN          \001ESC m              \001"
K_IBM3101_26 "\001(7)             \001ESC 7              \001 \001MENU            \001ALT-'              \001"
K_IBM3101_27 "\001(8)             \001ESC 8              \001 \001NEXT            \001ALT-n              \001"
K_IBM3101_28 "\001up arrow        \001up arrow           \001 \001NEXT-WINDOW     \001ESC n              \001"
K_IBM3101_29 "\001right arrow     \001right arrow        \001 \001+PAGE           \001PF1                \001"
K_IBM3101_30 "\001down arrow      \001down arrow         \001 \001-PAGE           \001PF4                \001"
K_IBM3101_31 "\001left arrow      \001left arrow         \001 \001PICK-COPY       \001ERASE INPUT        \001"
K_IBM3101_32 "\001BACKSPACE       \001BACKSPACE          \001 \001PICK-UP         \001ERASE EOL/EOF      \001"
K_IBM3101_33 "\001BEGIN-LINE      \001ALT-a              \001 \001PREVIOUS        \001ALT-p              \001"
K_IBM3101_34 "\001BOX-MARK        \001ALT-b              \001 \001PRINT           \001ALT-a p            \001"
K_IBM3101_35 "\001BREAK           \001CTL-c   **         \001 \001*PRINT          \001PRINT              \001"
K_IBM3101_36 "\001CANCEL          \001ALT-v              \001 \001PUT-COPY        \001CLEAR              \001"
K_IBM3101_37 "\001*CANCEL         \001CANCEL             \001 \001PUT-DOWN        \001ERASE EOS          \001"
K_IBM3101_38 "\001CENTER          \001ALT-c              \001 \001QUIT            \001CTL-\\ or CTL-| **  \001"
K_IBM3101_39 "\001DELETE-CHARACTER\001DEL                \001 \001QUOTE           \001ESC q              \001"
K_IBM3101_40 "\001DELETE-LINE     \001ALT-l              \001 \001REFRESH         \001ALT-y              \001"
K_IBM3101_41 "\001DO              \001ALT-x              \001 \001REPLACE         \001PF8                \001"
K_IBM3101_42 "\001END-LINE        \001ALT-z              \001 \001RESTORE         \001ALT-r              \001"
K_IBM3101_43 "\001ENTER           \001PRINT MSG          \001 \001RETURN          \001return             \001"
K_IBM3101_44 "\001ENTER           \001ALT-e              \001 \001RIGHT           \001ESC r              \001"
K_IBM3101_45 "\001EXECUTE         \001PRINT MSG          \001 \001SAVE            \001ESC s              \001"
K_IBM3101_46 "\001EXECUTE         \001ALT-e              \001 \001+SEARCH         \001PF3                \001"
K_IBM3101_47 "\001EXIT            \001ALT-d              \001 \001-SEARCH         \001PF6                \001"
K_IBM3101_48 "\001FONT            \001ESC o              \001 \001SET-TAB         \001ESC v              \001"
K_IBM3101_49 "\001FORMAT          \001ALT-f              \001 \001TAB             \001tab                \001"
K_IBM3101_50 "\001FUCTIONS        \001ESC h              \001 \001-TAB            \001ALT-t              \001"
K_IBM3101_51 "\001GO-TO           \001ALT-g              \001 \001TEXT-MARK       \001ESC t              \001"
K_IBM3101_52 "\001HELP            \001PF7                \001 \001USE             \001ALT-u              \001"
K_IBM3101_53 "\001HOME            \001home               \001 \001WINDOW          \001ALT-w              \001"
K_IBM3101_54 "\001INSERT-LINE     \001ALT-o              \001 \001ZOOM-IN         \001ALT-j              \001"
K_IBM3101_55 "\001INSERT-MODE     \001ESC i              \001 \001ZOOM-OUT        \001ALT-k              \001"
K_IBM3101_56 "\001LAST-ARG        \001ESC y              \001 \006\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\022\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\007"
K_IBM3101_59 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_IBM3101_60 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_IBM3101_61 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_IBM3101_62 "   is labeled \"quit\"."
$ Do not translate quit
$
K_IBM3101_63 ""
$
$
$ IBM3151 KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_IBM3151_1 "\011\011\011      Keyboard - ibm3151"
K_IBM3151_4 "\011\011\011       Input Sequences"
K_IBM3151_6 "  Editor Function      Key Name         Editor Function      Key Name"
K_IBM3151_8 "\001(1)              \001CTL-a 1          \001 \001MENU             \001F3               \001"
K_IBM3151_9 "\001(2)              \001CTL-a 2          \001 \001LINES UP         \001F11              \001"
K_IBM3151_10 "\001(3)              \001CTL-a 3          \001 \001PAGE UP          \001SH F11           \001"
K_IBM3151_11 "\001(4)              \001CTL-a 4          \001 \001SEARCH UP        \001SH F7            \001"
K_IBM3151_12 "\001(5)              \001CTL-a 5          \001 \001BACKTAB          \001TAB <---         \001"
K_IBM3151_13 "\001(6)              \001CTL-a 6          \001 \001NEXT             \001SH F4            \001"
K_IBM3151_14 "\001(7)              \001CTL-a 7          \001 \001NEXT-WINDOW      \001CTL-a n          \001"
K_IBM3151_15 "\001(8)              \001CTL-a 8          \001 \001PICK-COPY        \001F9               \001"
K_IBM3151_16 "\001BACKSPACE        \001backspace        \001 \001PICK-UP          \001F7               \001"
K_IBM3151_17 "\001BEGIN-LINE       \001SH F5            \001 \001LINES DOWN       \001F12              \001"
K_IBM3151_18 "\001BOX-MARK         \001CTL-a b          \001 \001PAGE DOWN        \001SH F12           \001"
K_IBM3151_19 "\001BREAK            \001CTL-C  **        \001 \001SEARCH DOWN      \001SH F8            \001"
K_IBM3151_20 "\001CANCEL           \001F2               \001 \001PREVIOUS         \001SH F3            \001"
K_IBM3151_21 "\001CENTER           \001CTL-a c          \001 \001PRINT            \001CTL-a p          \001"
K_IBM3151_22 "\001DELETE-CHARACTER \001Delete           \001 \001PUT-COPY         \001F10              \001"
K_IBM3151_23 "\001DELETE-LINE      \001CTL-Delete       \001 \001PUT-DOWN         \001F8               \001"
K_IBM3151_24 "\001DO               \001CTL-a x          \001 \001QUIT             \001CTL-\\ or CTL-|   \001"
K_IBM3151_25 "\001DOWN-ARROW       \001Down Arrow       \001 \001QUOTE            \001CTL-a q          \001"
K_IBM3151_26 "\001END-LINE         \001SH F6            \001 \001REFRESH          \001CTL-a z          \001"
K_IBM3151_27 "\001EXECUTE          \001Send             \001 \001REPLACE          \001SH F1            \001"
K_IBM3151_28 "\001EXIT             \001CTL-a d          \001 \001RESTORE          \001CTL-Insert Char  \001"
K_IBM3151_29 "\001FONT             \001CTL-a f          \001 \001RETURN           \001Return           \001"
K_IBM3151_30 "\001FORMAT           \001F5               \001 \001RIGHT            \001CTL-a r          \001"
K_IBM3151_31 "\001GO-TO            \001SH F2            \001 \001RIGHT-ARROW      \001Right Arrow      \001"
K_IBM3151_32 "\001HELP             \001F1               \001 \001SAVE             \001CTL-a s          \001"
K_IBM3151_33 "\001HOME             \001Home             \001 \001SET-TAB          \001CTL-a v          \001"
K_IBM3151_34 "\001INSERT-LINE      \001F6               \001 \001TAB              \001TAB --->         \001"
K_IBM3151_35 "\001INSERT-MODE      \001Insert Char      \001 \001TEXT-MARK        \001CTL-a t          \001"
K_IBM3151_36 "\001LAST-ARG         \001CTL-a a          \001 \001UP-ARROW         \001Up Arrow         \001"
K_IBM3151_37 "\001LEFT             \001CTL-a l          \001 \001USE              \001CTL-a u          \001"
K_IBM3151_38 "\001LEFT-ARROW       \001Left Arrow       \001 \001WINDOW           \001CTL-a w          \001"
K_IBM3151_39 "\001LINE-FEED        \001                 \001 \001ZOOM-IN          \001SH F9            \001"
K_IBM3151_40 "\001LOCAL-MENU       \001F4               \001 \001ZOOM-OUT         \001SH F10           \001"
K_IBM3151_41 "\001MARGIN           \001CTL-a m          \001 \006\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\022\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\007"
K_IBM3151_44 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_IBM3151_45 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_IBM3151_46 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_IBM3151_47 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_IBM3151_48 ""
$
$
$ IBM3161 KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_IBM3161_1 "\011\011\011          Keyboard"
K_IBM3161_5 "\011\011\011       Input Sequences"
K_IBM3161_7 "  Editor Function      Key Name         Editor Function      Key Name"
K_IBM3161_9 "\001(1)              \001CTL-a 1          \001 \001MENU             \001F3               \001"
K_IBM3161_10 "\001(2)              \001CTL-a 2          \001 \001LINES UP         \001F11              \001"
K_IBM3161_11 "\001(3)              \001CTL-a 3          \001 \001PAGE UP          \001SH F11           \001"
K_IBM3161_12 "\001(4)              \001CTL-a 4          \001 \001SEARCH UP        \001SH F7            \001"
K_IBM3161_13 "\001(5)              \001CTL-a 5          \001 \001BACKTAB          \001TAB <---         \001"
K_IBM3161_14 "\001(6)              \001CTL-a 6          \001 \001NEXT             \001SH F4            \001"
K_IBM3161_15 "\001(7)              \001CTL-a 7          \001 \001NEXT-WINDOW      \001CTL-a n          \001"
K_IBM3161_16 "\001(8)              \001CTL-a 8          \001 \001PICK-COPY        \001F9               \001"
K_IBM3161_17 "\001BACKSPACE        \001backspace        \001 \001PICK-UP          \001F7               \001"
K_IBM3161_18 "\001BEGIN-LINE       \001SH F5            \001 \001LINES DOWN       \001F12              \001"
K_IBM3161_19 "\001BOX-MARK         \001CTL-a b          \001 \001PAGE DOWN        \001SH F12           \001"
K_IBM3161_20 "\001BREAK            \001CTL-C  **        \001 \001SEARCH DOWN      \001SH F8            \001"
K_IBM3161_21 "\001CANCEL           \001F2               \001 \001PREVIOUS         \001SH F3            \001"
K_IBM3161_22 "\001CENTER           \001CTL-a c          \001 \001PRINT            \001CTL-a p          \001"
K_IBM3161_23 "\001DELETE-CHARACTER \001Delete           \001 \001PUT-COPY         \001F10              \001"
K_IBM3161_24 "\001DELETE-LINE      \001CTL-Delete       \001 \001PUT-DOWN         \001F8               \001"
K_IBM3161_25 "\001DO               \001CTL-a x          \001 \001QUIT             \001CTL-4  **        \001"
K_IBM3161_26 "\001DOWN-ARROW       \001Down Arrow       \001 \001QUOTE            \001CTL-a q          \001"
K_IBM3161_27 "\001END-LINE         \001SH F6            \001 \001REFRESH          \001CTL-a z          \001"
K_IBM3161_28 "\001EXECUTE          \001Send             \001 \001REPLACE          \001SH F1            \001"
K_IBM3161_29 "\001EXIT             \001CTL-a d          \001 \001RESTORE          \001CTL-Insert Char  \001"
K_IBM3161_30 "\001FONT             \001CTL-a f          \001 \001RETURN           \001Return           \001"
K_IBM3161_31 "\001FORMAT           \001F5               \001 \001RIGHT            \001CTL-a r          \001"
K_IBM3161_32 "\001GO-TO            \001SH F2            \001 \001RIGHT-ARROW      \001Right Arrow      \001"
K_IBM3161_33 "\001HELP             \001F1               \001 \001SAVE             \001CTL-a s          \001"
K_IBM3161_34 "\001HOME             \001Home             \001 \001SET-TAB          \001CTL-a v          \001"
K_IBM3161_35 "\001INSERT-LINE      \001F6               \001 \001TAB              \001TAB --->         \001"
K_IBM3161_36 "\001INSERT-MODE      \001Insert Char      \001 \001TEXT-MARK        \001CTL-a t          \001"
K_IBM3161_37 "\001LAST-ARG         \001CTL-a a          \001 \001UP-ARROW         \001Up Arrow         \001"
K_IBM3161_38 "\001LEFT             \001CTL-a l          \001 \001USE              \001CTL-a u          \001"
K_IBM3161_39 "\001LEFT-ARROW       \001Left Arrow       \001 \001WINDOW           \001CTL-a w          \001"
K_IBM3161_40 "\001LINE-FEED        \001                 \001 \001ZOOM-IN          \001SH F9            \001"
K_IBM3161_41 "\001LOCAL-MENU       \001F4               \001 \001ZOOM-OUT         \001SH F10           \001"
K_IBM3161_42 "\001MARGIN           \001CTL-a m          \001 \006\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\022\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\007"
K_IBM3161_45 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_IBM3161_46 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_IBM3161_47 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_IBM3161_48 "   is labeled \"quit\"."
$ Do not translate quit
$
K_IBM3161_49 ""
$
$
$ IBM5550 KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_IBM5550_1 "\011\011\011      Keyboard - ibm5550"
K_IBM5550_4 "\011\011\011       Input Sequences"
K_IBM5550_6 " Editor Function   Key Name             Editor Function   Key Name"
K_IBM5550_8 "\001BACKSPACE        \001Backspace        \001  \001PAGE UP          \001Shift+PF9        \001"
K_IBM5550_9 "\001BACKTAB          \001CTRL-t           \001  \001PICK COPY        \001Shift+PF7        \001"
K_IBM5550_10 "\001BEGIN LINE       \001CTRL-a left cursr\001  \001PICK UP          \001PF7              \001"
K_IBM5550_11 "\001BOX MARK         \001CTRL-a b         \001  \001PREVIOUS         \001CTRL-a ,         \001"
K_IBM5550_12 "\001BREAK            \001CTRL-C **        \001  \001PRINT            \001CTRL-a p         \001"
K_IBM5550_13 "\001CANCEL           \001Clear            \001  \001PUT COPY         \001Shift+PF8        \001"
K_IBM5550_14 "\001CENTER           \001CTRL-a c         \001  \001PUT DOWN         \001PF8              \001"
K_IBM5550_15 "\001CURSOR RIGHT     \001Right arrow      \001  \001QUIT             \001CTRL-\\ **        \001"
K_IBM5550_16 "\001CURSOR UP        \001Up arrow         \001  \001QUOTE            \001CTRL-a q         \001"
K_IBM5550_17 "\001DELETE CHAR      \001Delete           \001  \001REFRESH          \001CTRL-a z         \001"
K_IBM5550_18 "\001DELETE LINE      \001CTRL-a delete    \001  \001REPLACE          \001CTRL-a =         \001"
K_IBM5550_19 "\001DO               \001CTRL-a x         \001  \001RESTORE          \001CTRL-a i         \001"
K_IBM5550_20 "\001END LINE         \001CTRL-a right curs\001  \001RETURN           \001Enter            \001"
K_IBM5550_21 "\001ENTER            \001ENTER            \001  \001RIGHT            \001CTRL-a r         \001"
K_IBM5550_22 "\001EXECUTE          \001EXECUTE          \001  \001SAVE             \001CTRL-a s         \001"
K_IBM5550_23 "\001EXIT             \001CTRL-a d         \001  \001SEARCH DOWN      \001CTRL-a down cursr\001"
K_IBM5550_24 "\001FONT             \001CTRL-a f         \001  \001SEARCH UP        \001CTRL-a up cursor \001"
K_IBM5550_25 "\001FORMAT           \001PF5              \001  \001SET TAB          \001CTRL-a v         \001"
K_IBM5550_26 "\001GO TO            \001CTRL-a g         \001  \001TAB              \001Tab              \001"
K_IBM5550_27 "\001HELP             \001PF1              \001  \001TEXT MARK        \001CTRL-a t         \001"
K_IBM5550_28 "\001HOME             \001CTRL-a h         \001  \001(1)              \001CTRL-a+PF1       |"
K_IBM5550_29 "\001INSERT LINE      \001Shift-PF6        \001  \001(2)              \001CTRL-a+PF2       |"
K_IBM5550_30 "\001INSERT MODE      \001PF6              \001  \001(3)              \001CTRL-a+PF3       \001"
K_IBM5550_31 "\001LAST ARG         \001CTRL-a a         \001  \001(4)              \001CTRL-a+PF4       \001"
K_IBM5550_32 "\001LEFT             \001CTRL-a l         \001  \001(5)              \001CTRL-a+PF5       \001"
K_IBM5550_33 "\001LINE DOWN        \001CTRL-a dn arrow  \001  \001(6)              \001CTRL-a+PF6       |"
K_IBM5550_34 "\001LINE UP          \001CTRL-a up arrow  \001  \001(7)              \001CTRL-a+PF7       \001"
K_IBM5550_35 "\001MARGIN           \001CTRL-a m         \001  \001(8)              \001CTRL-a+PF8       \001"
K_IBM5550_36 "\001MENU             \001PF3              \001  \001USE              \001CTRL-a u         \001"
K_IBM5550_37 "\001NEXT             \001CTRL-a           \001  \001WINDOW           \001CTRL-a w         \001"
K_IBM5550_38 "\001NEXT WINDOW      \001CTRL-a n         \001  \001ZOOM IN          \001PF11             \001"
K_IBM5550_39 "\001PAGE DOWN        \001PF9              \001  \001ZOOM OUT         \001PF12             \001"
K_IBM5550_42 " ** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_IBM5550_43 "    from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_IBM5550_44 "    command line:  the BREAK character is labeled \"intr=\", the QUIT"
$ Do not translate BREAK, intr= and QUIT
$
K_IBM5550_45 "    character is labeled \"quit=\"."
$ Do not translate quit
$
K_IBM5550_46 ""
$
$
$ IBM5570 KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_IBM5570_1 "\011\011\011      Keyboard - ibm5570"
K_IBM5570_4 "\011\011\011       Input Sequences"
K_IBM5570_6 " Editor Function   Key Name             Editor Function   Key Name"
K_IBM5570_8 "\001BACKSPACE        \001Backspace        \001  \001PAGE UP          \001Shift+PF9        \001"
K_IBM5570_9 "\001BACKTAB          \001CTRL-t           \001  \001PICK COPY        \001Shift+PF7        \001"
K_IBM5570_10 "\001BEGIN LINE       \001CTRL-a left cursr\001  \001PICK UP          \001PF7              \001"
K_IBM5570_11 "\001BOX MARK         \001CTRL-a b         \001  \001PREVIOUS         \001CTRL-a ,         \001"
K_IBM5570_12 "\001BREAK            \001CTRL-C **        \001  \001PRINT            \001CTRL-a p         \001"
K_IBM5570_13 "\001CANCEL           \001End              \001  \001PUT COPY         \001Shift+PF8        \001"
K_IBM5570_14 "\001CENTER           \001CTRL-a c         \001  \001PUT DOWN         \001PF8              \001"
K_IBM5570_15 "\001CURSOR RIGHT     \001Right arrow      \001  \001QUIT             \001CTRL-\\ **        \001"
K_IBM5570_16 "\001CURSOR UP        \001Up arrow         \001  \001QUOTE            \001CTRL-a q         \001"
K_IBM5570_17 "\001DELETE CHAR      \001Delete           \001  \001REFRESH          \001CTRL-a z         \001"
K_IBM5570_18 "\001DELETE LINE      \001CTRL-a delete    \001  \001REPLACE          \001CTRL-a =         \001"
K_IBM5570_19 "\001DO               \001CTRL-a x         \001  \001RESTORE          \001CTRL-a i         \001"
K_IBM5570_20 "\001END LINE         \001Ctrl-a right curs\001  \001RETURN           \001Enter            \001"
K_IBM5570_21 "\001ENTER            \001ENTER            \001  \001RIGHT            \001CTRL-a r         |"
K_IBM5570_22 "\001EXECUTE          \001EXECUTE          \001  \001SAVE             \001CTRL-a s         \001"
K_IBM5570_23 "\001EXIT             \001CTRL-a d         \001  \001SEARCH DOWN      \001CTRL-a down cursr\001"
K_IBM5570_24 "\001FONT             \001CTRL-a f         \001  \001SEARCH UP        \001CTRL-a up cursor \001"
K_IBM5570_25 "\001FORMAT           \001PF5              \001  \001SET TAB          \001CTRL-a v         \001"
K_IBM5570_26 "\001GO TO            \001CTRL-a g         \001  \001TAB              \001Tab              \001"
K_IBM5570_27 "\001HELP             \001PF1              \001  \001TEXT MARK        \001CTRL-a t         \001"
K_IBM5570_28 "\001HOME             \001HOME             \001  \001(1)              \001CTRL-a+PF1       \001"
K_IBM5570_29 "\001INSERT LINE      \001Shift-PF6        \001  \001(2)              \001CTRL-a+PF2       \001"
K_IBM5570_30 "\001INSERT MODE      \001PF6              \001  \001(3)              \001CTRL-a+PF3       \001"
K_IBM5570_31 "\001LAST ARG         \001CTRL-a a         \001  \001(4)              \001CTRL-a+PF4       \001"
K_IBM5570_32 "\001LEFT             \001CTRL-a l         \001  \001(5)              \001CTRL-a+PF5       \001"
K_IBM5570_33 "\001LINES DOWN       \001CTRL-a dn arrow  \001  \001(6)              \001CTRL-a+PF6       \001"
K_IBM5570_34 "\001LINES UP         \001CTRL-a up arrow  \001  \001(7)              \001CTRL-a+PF7       \001"
K_IBM5570_35 "\001MARGIN           \001CTRL-a m         \001  \001(8)              \001CTRL-a+PF8       \001"
K_IBM5570_36 "\001MENU             \001PF3              \001  \001USE              \001CTRL-a u         \001"
K_IBM5570_37 "\001NEXT             \001CTRL-a           \001  \001WINDOW           \001CTRL-a w         \001"
K_IBM5570_38 "\001NEXT WINDOW      \001CTRL-a n         \001  \001ZOOM IN          \001PF11             \001"
K_IBM5570_39 "\001PAGE DOWN        \001PF9              \001  \001ZOOM OUT         \001PF12             \001"
K_IBM5570_42 " ** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_IBM5570_43 "    from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_IBM5570_44 "    command line:  the BREAK character is labeled \"intr=\", the QUIT"
$ Do not translate BREAK, intr= and QUIT
$
K_IBM5570_45 "    character is labeled \"quit=\"."
$ Do not translate quit
$
K_IBM5570_46 ""
$
$
$ VI640 KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_VI640_1 "\011\011\011      Keyboard - vi640"
K_VI640_4 "\011 Visual 640 X-Station (VT-220 style keyboard),"
K_VI640_5 "\011 running xterm with TERM=aixterm or aixterm-m"
K_VI640_7 "Before running INed on a Visual 640, the terminal should be linked to an"
K_VI640_8 "IBM font server in order for graphic boxes to display correctly."
K_VI640_11 "\011\011\011       Input Sequences"
K_VI640_13 " Editor Function   Key Name             Editor Function   Key Name"
K_VI640_15 "\001BACKSPACE        \001Backspace        \001  \001PAGE UP          \001CC-Previous Scrn \001"
K_VI640_16 "\001BACKTAB          \001Shift Tab        \001  \001PICK COPY        \001F9               \001"
K_VI640_17 "\001BEGIN LINE       \001CC-Left arrow    \001  \001PICK UP          \001F7               \001"
K_VI640_18 "\001BOX MARK         \001CC-b             \001  \001PREVIOUS         \001CC-F11           \001"
K_VI640_19 "\001BREAK            \001Ctrl-c  **       \001  \001PRINT            \001CC-F18           \001"
K_VI640_20 "\001CANCEL           \001F19              \001  \001PUT COPY         \001F10              \001"
K_VI640_21 "\001CENTER           \001CC-c             \001  \001PUT DOWN         \001F8               \001"
K_VI640_22 "\001CURSOR RIGHT     \001Right arrow      \001  \001QUIT             \001Ctrl-\\ **        \001"
K_VI640_23 "\001CURSOR UP        \001Up arrow         \001  \001QUOTE            \001CC-q             \001"
K_VI640_24 "\001DELETE CHAR      \001<X]              \001  \001REFRESH          \001CC-z             \001"
K_VI640_25 "\001DELETE LINE      \001CC-<X]           \001  \001REPLACE          \001CC-Select        \001"
K_VI640_26 "\001DO               \001CC-x             \001  \001RESTORE          \001CC-Insert Here   \001"
K_VI640_27 "\001END LINE         \001CC-Right arrow   \001  \001RETURN           \001Enter            \001"
K_VI640_28 "\001ENTER            \001Do               \001  \001RIGHT            \001CC-r             \001"
K_VI640_29 "\001EXECUTE          \001Do               \001  \001SAVE             \001CC-s             \001"
K_VI640_30 "\001EXIT             \001CC-d             \001  \001SEARCH DOWN      \001CC-Down arrow    \001"
K_VI640_31 "\001FONT             \001CC-f             \001  \001SEARCH UP        \001CC-Up arrow      \001"
K_VI640_32 "\001FORMAT           \001F5               \001  \001SET TAB          \001CC-v             \001"
K_VI640_33 "\001GO TO            \001Select           \001  \001TAB              \001Tab              \001"
K_VI640_34 "\001HELP             \001Help             \001  \001TEXT MARK        \001CC-t             \001"
K_VI640_35 "\001HOME             \001Home             \001  \001(1)              \001CC-F1            \001"
K_VI640_36 "\001INSERT LINE      \001F6               \001  \001(2)              \001CC-F2            \001"
K_VI640_37 "\001INSERT MODE      \001Insert Here      \001  \001(3)              \001CC-F3            \001"
K_VI640_38 "\001LAST ARG         \001CC-a             \001  \001(4)              \001CC-F4            \001"
K_VI640_39 "\001LEFT             \001CC-l             \001  \001(5)              \001CC-F5            \001"
K_VI640_40 "\001LINE DOWN        \001Next Screen      \001  \001(6)              \001CC-F6            \001"
K_VI640_41 "\001LINE UP          \001Previous Screen  \001  \001(7)              \001CC-F7            \001"
K_VI640_42 "\001MARGIN           \001CC-m             \001  \001(8)              \001CC-F8            \001"
K_VI640_43 "\001MENU             \001PF3              \001  \001USE              \001CC-u             \001"
K_VI640_44 "\001NEXT             \001CC-F12           \001  \001WINDOW           \001CC-w             \001"
K_VI640_45 "\001NEXT WINDOW      \001CC-n             \001  \001ZOOM IN          \001F11              \001"
K_VI640_46 "\001PAGE DOWN        \001CC-Next Screen   \001  \001ZOOM OUT         \001F12              \001"
K_VI640_49 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_VI640_50 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_VI640_51 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_VI640_52 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_VI640_53 ""
$
$
$ VT100 KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_VT100_1 "\011\011\011        Keyboard - vt100"
K_VT100_3 ""
K_VT100_6 "\011\011\011       Input Sequences"
K_VT100_8 " Name              Value                Name             Value"
K_VT100_10 "\001(1)              \001CTRL-a PF1       \001  \001LINES UP         \001Number-Pad 1     \001"
K_VT100_11 "\001(2)              \001CTRL-a PF2       \001  \001LOCAL-MENU       \001PF3              \001"
K_VT100_12 "\001(3)              \001CTRL-a PF3       \001  \001MARGIN           \001CTRL-a m         \001"
K_VT100_13 "\001(4)              \001CTRL-a PF4       \001  \001MENU             \001PF2              \001"
K_VT100_14 "\001(5)              \001CTRL-a Num Pad 7 \001  \001NEXT             \001CTRL-a .         \001"
K_VT100_15 "\001(6)              \001CTRL-a Num Pad 8 \001  \001NEXT-WINDOW      \001CTRL-a n         \001"
K_VT100_16 "\001(7)              \001CTRL-a Num Pad 9 \001  \001PAGE DOWN        \001Number-Pad .     \001"
K_VT100_17 "\001(8)              \001CTRL-a Num Pad - \001  \001PAGE UP          \001Number-Pad 3     \001"
K_VT100_18 "\001BACKSPACE        \001BACKSPACE        \001  \001PICK-COPY        \001Number-Pad 6     \001"
K_VT100_19 "\001BEGIN-LINE       \001CTRL-a left curso\001  \001PICK-UP          \001Number-Pad 4     \001"
K_VT100_20 "\001BOX-MARK         \001CTRL-a b         \001  \001PREVIOUS         \001CTRL-a ,         \001"
K_VT100_21 "\001BREAK            \001CTRL-c **        \001  \001PRINT            \001CTRL-a p         \001"
K_VT100_22 "\001CANCEL           \001PF4              \001  \001PUT-COPY         \001Number-Pad ,     \001"
K_VT100_23 "\001CENTER           \001CTRL-a c         \001  \001PUT-DOWN         \001Number-Pad 5     \001"
K_VT100_24 "\001DELETE-CHARACTER \001Delete           \001  \001QUIT             \001CTRL-\\  **       \001"
K_VT100_25 "\001DELETE-LINE      \001CTRL-a Delete    \001  \001QUOTE            \001CTRL-a q         \001"
K_VT100_26 "\001DO               \001CTRL-a x         \001  \001REFRESH          \001CTRL-a z         \001"
K_VT100_27 "\001DOWN-ARROW       \001down cursor      \001  \001REPLACE          \001CTRL-a =         \001"
K_VT100_28 "\001END-LINE         \001CTRL-a right curs\001  \001RESTORE          \001CTRL-a i         \001"
K_VT100_29 "\001ENTER            \001Number-Pad Enter \001  \001RETURN           \001Return           \001"
K_VT100_30 "\001EXECUTE          \001Number-Pad Enter \001  \001RIGHT            \001CTRL-a r         \001"
K_VT100_31 "\001EXIT             \001CTRL-a d         \001  \001RIGHT-ARROW      \001right cursor     \001"
K_VT100_32 "\001FONT             \001CTRL-a f         \001  \001SAVE             \001CTRL-a s         \001"
K_VT100_33 "\001FORMAT           \001Number-Pad 7     \001  \001SEARCH DOWN      \001CTRL-a down curso\001"
K_VT100_34 "\001GO-TO            \001CTRL-a g         \001  \001SEARCH UP        \001CTRL-a up cursor \001"
K_VT100_35 "\001HELP             \001PF1              \001  \001SET-TAB          \001CTRL-a v         \001"
K_VT100_36 "\001HOME             \001CTRL-a h         \001  \001TAB              \001Tab              \001"
K_VT100_37 "\001INSERT-LINE      \001Number-Pad 8     \001  \001-TAB             \001CTRL-t           \001"
K_VT100_38 "\001INSERT-MODE      \001Number-Pad 0     \001  \001TEXT-MARK        \001CTRL-a t         \001"
K_VT100_39 "\001LAST-ARG         \001CTRL-a a         \001  \001UP-ARROW         \001up cursor        \001"
K_VT100_40 "\001LEFT             \001CTRL-a l         \001  \001USE              \001CTRL-a u         \001"
K_VT100_41 "\001LEFT-ARROW       \001left cursor      \001  \001WINDOW           \001CTRL-a w         \001"
K_VT100_42 "\001LINEFEED         \001LINEFEED         \001  \001ZOOM-IN          \001Number-Pad 9     \001"
K_VT100_43 "\001LINES DOWN       \001Number-Pad 2     \001  \001ZOOM-OUT         \001Number-Pad -     \001"
K_VT100_46 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_VT100_47 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_VT100_48 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_VT100_49 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_VT100_50 ""
$
$
$ VT100X KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_VT100X_1 "\011\011\011       Keyboard - vt100x"
K_VT100X_3 " NOTE: The vt100x terminal type is for vt100 terminal emulation on AIX."
K_VT100X_6 "\011\011\011       Input Sequences"
K_VT100X_8 "  Editor Function     Key Name          Editor Function     Key Name"
K_VT100X_10 "\001BACKSPACE        \001BACKSPACE        \001 \001PICK-UP          \001Number-Pad 7     \001"
K_VT100X_11 "\001BEGIN-LINE       \001Ctrl-A left curso\001 \001+LINE            \001Pg Dn            \001"
K_VT100X_12 "\001BOX-MARK         \001Ctrl-A b         \001 \001+PAGE            \001Ctrl-A Pg Dn     \001"
K_VT100X_13 "\001BREAK            \001Ctrl-c  **       \001 \001+SEARCH          \001Ctrl-A down curso\001"
K_VT100X_14 "\001CANCEL           \001Number-Pad -     \001 \001PREVIOUS         \001Ctrl-A ,         \001"
K_VT100X_15 "\001CENTER           \001Ctrl-A c         \001 \001PRINT            \001Ctrl-A p         \001"
K_VT100X_16 "\001DELETE-CHARACTER \001Delete           \001 \001PUT-COPY         \001Number-Pad 1     \001"
K_VT100X_17 "\001DELETE-LINE      \001Ctrl-A Delete    \001 \001PUT-DOWN         \001Number-Pad 5     \001"
K_VT100X_18 "\001DO               \001Ctrl-A x         \001 \001QUIT             \001Ctrl-\\ **        \001"
K_VT100X_19 "\001DOWN-ARROW       \001down cursor      \001 \001QUOTE            \001Ctrl-A q         \001"
K_VT100X_20 "\001END-LINE         \001Ctrl-A right curs\001 \001REFRESH          \001Ctrl-A z         \001"
K_VT100X_21 "\001EXECUTE          \001Number-Pad Enter \001 \001REPLACE          \001Ctrl-A Num Pad 5 \001"
K_VT100X_22 "\001ENTER            \001Number-Pad Enter \001 \001RESTORE          \001Ctrl-A Ins       \001"
K_VT100X_23 "\001EXIT             \001Ctrl-A d         \001 \001RETURN           \001Return           \001"
K_VT100X_24 "\001FONT             \001Ctrl-A f         \001 \001RIGHT            \001Ctrl-A r         \001"
K_VT100X_25 "\001FORMAT           \001PF3              \001 \001RIGHT-ARROW      \001right cursor     \001"
K_VT100X_26 "\001GO-TO            \001Ctrl-A g         \001 \001SAVE             \001Ctrl-A s         \001"
K_VT100X_27 "\001HELP             \001PF1              \001 \001SET-TAB          \001Ctrl-A v         \001"
K_VT100X_28 "\001HOME             \001Ctrl-A h         \001 \001TAB              \001Tab              \001"
K_VT100X_29 "\001INSERT-LINE      \001PF4              \001 \001TEXT-MARK        \001Ctrl-A t         \001"
K_VT100X_30 "\001INSERT-MODE      \001Ins              \001 \001UP-ARROW         \001up cursor        \001"
K_VT100X_31 "\001LAST-ARG         \001Ctrl-A a         \001 \001USE              \001Ctrl-a u         \001"
K_VT100X_32 "\001LEFT             \001Ctrl-A l         \001 \001WINDOW           \001Ctrl-a w         \001"
K_VT100X_33 "\001LEFT-ARROW       \001left cursor      \001 \001ZOOM-IN          \001Ctrl-A Prt Sc    \001"
K_VT100X_34 "\001LOCAL-MENU       \001Ctrl-A PF2       \001 \001ZOOM-OUT         \001Ctrl-A Num Pad 1 \001"
K_VT100X_35 "\001MARGIN           \001Ctrl-A m         \001 \001(1)              \001Ctrl-A 1         \001"
K_VT100X_36 "\001MENU             \001PF2              \001 \001(2)              \001Ctrl-A 2         \001"
K_VT100X_37 "\001-LINE            \001Pg Up            \001 \001(3)              \001Ctrl-A 3         \001"
K_VT100X_38 "\001-PAGE            \001Ctrl-A Pg Up     \001 \001(4)              \001Ctrl-A 4         \001"
K_VT100X_39 "\001-SEARCH          \001Ctrl-A up cursor \001 \001(5)              \001Ctrl-A 5         \001"
K_VT100X_40 "\001-TAB             \001Ctrl-t           \001 \001(6)              \001Ctrl-A 6         \001"
K_VT100X_41 "\001NEXT             \001Ctrl-a .         \001 \001(7)              \001Ctrl-A 7         \001"
K_VT100X_42 "\001NEXT-WINDOW      \001Ctrl-a n         \001 \001(8)              \001Ctrl-A 8         \001"
K_VT100X_43 "\001PICK-COPY        \001Prt Sc           \001 \006\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\022\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\007"
K_VT100X_46 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_VT100X_47 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_VT100X_48 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_VT100X_49 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_VT100X_50 ""
$
$
$ VT220 KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_VT220_1 "\011\011\011   Keyboards - vt220 vt330 vt340"
K_VT220_4 "\011\011\011       Input Sequences"
K_VT220_6 " Editor Function   Key Name             Editor Function   Key Name"
K_VT220_8 "\001(1)              \001Ctrl-a F7        \001  \001LINES DOWN       \001Next Screen      \001"
K_VT220_9 "\001(2)              \001Ctrl-a F8        \001  \001LINES UP         \001Prev Screen      \001"
K_VT220_10 "\001(3)              \001Ctrl-a F9        \001  \001LOCAL-MENU       \001F8               \001"
K_VT220_11 "\001(4)              \001Ctrl-a F10       \001  \001MARGIN           \001Ctrl-a m         \001"
K_VT220_12 "\001(5)              \001Ctrl-a F11       \001  \001MENU             \001F7               \001"
K_VT220_13 "\001(6)              \001Ctrl-a F12       \001  \001NEXT             \001Ctrl-a .         \001"
K_VT220_14 "\001(7)              \001Ctrl-a F13       \001  \001NEXT-WINDOW      \001Ctrl-a n         \001"
K_VT220_15 "\001(8)              \001Ctrl-a F14       \001  \001PAGE DOWN        \001Ctrl-a Next Scren\001"
K_VT220_16 "\001BACKSPACE        \001backspace        \001  \001PAGE UP          \001Ctrl-a Prev Scren\001"
K_VT220_17 "\001BACKTAB          \001not available    \001  \001PICK-COPY        \001F13              \001"
K_VT220_18 "\001BEGIN-LINE       \001Ctrl-a left-arrow\001  \001PICK-UP          \001F11              \001"
K_VT220_19 "\001BOX-MARK         \001Ctrl-a b         \001  \001PREVIOUS         \001Ctrl-a ,         \001"
K_VT220_20 "\001BREAK            \001Ctrl-c  **       \001  \001PRINT            \001Ctrl-a p         \001"
K_VT220_21 "\001CANCEL           \001PF4              \001  \001PUT-COPY         \001F14              \001"
K_VT220_22 "\001CENTER           \001Ctrl-a c         \001  \001PUT-DOWN         \001F12              \001"
K_VT220_23 "\001DELETE-LINE      \001Ctrl-a Remove    \001  \001QUIT             \001Ctrl-\\  **       \001"
K_VT220_24 "\001DELETE-CHARACTER \001Remove           \001  \001QUOTE            \001Ctrl-a q         \001"
K_VT220_25 "\001DO               \001Ctrl-a x         \001  \001REFRESH          \001Ctrl-a z         \001"
K_VT220_26 "\001DO               \001DO               \001  \001REPLACE          \001PF3              \001"
K_VT220_27 "\001DOWN-ARROW       \001down arrow       \001  \001RESTORE          \001Ctrl-a Insert Hre\001"
K_VT220_28 "\001END-LINE         \001Ctrl-a right arrw\001  \001RETURN           \001Return           \001"
K_VT220_29 "\001EXECUTE / ENTER  \001F17              \001  \001RIGHT            \001Ctrl-a r         \001"
K_VT220_30 "\001EXIT             \001Ctrl-a d         \001  \001RIGHT-ARROW      \001Right arrow      \001"
K_VT220_31 "\001FONT             \001Ctrl-a f         \001  \001SAVE             \001Ctrl-a s         \001"
K_VT220_32 "\001FORMAT           \001F9               \001  \001SEARCH DOWN      \001Select           \001"
K_VT220_33 "\001GO-TO            \001PF2              \001  \001SEARCH UP        \001Find             \001"
K_VT220_34 "\001HELP             \001PF1              \001  \001SET-TAB          \001Ctrl-a v         \001"
K_VT220_35 "\001HELP             \001help (F15)       \001  \001TAB              \001Tab              \001"
K_VT220_36 "\001HOME             \001F18              \001  \001TEXT-MARK        \001Ctrl-a t         \001"
K_VT220_37 "\001INSERT-LINE      \001F10              \001  \001UP-ARROW         \001Up arrow         \001"
K_VT220_38 "\001INSERT-MODE      \001Insert Here      \001  \001USE              \001Ctrl-a u         \001"
K_VT220_39 "\001LAST-ARG         \001Ctrl-a a         \001  \001WINDOW           \001Ctrl-a w         \001"
K_VT220_40 "\001LEFT             \001Ctrl-a l         \001  \001ZOOM-IN          \001F19              \001"
K_VT220_41 "\001LEFT-ARROW       \001left arrow       \001  \001ZOOM-OUT         \001F20              \001"
K_VT220_42 "\001LINE-FEED        \001Ctrl-j           \001  \006\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\022\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\007"
K_VT220_45 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_VT220_46 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_VT220_47 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_VT220_48 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_VT220_49 ""
$
$
$ WYSE KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_WYSE_1 "\011\011\011   Keyboards - wyse50 wyse350 wyse60"
K_WYSE_2 "\011\011\011   (Wyse60 with ASCII keyboard only)"
K_WYSE_5 "\011\011\011       Input Sequences"
K_WYSE_7 "   Editor Function      Key Name          Editor Function      Key Name"
K_WYSE_9 " \001(1)              \001Funct-1          \001  \001LINES UP         \001F11              \001"
K_WYSE_10 " \001(2)              \001Funct-2          \001  \001LOCAL-MENU       \001F4               \001"
K_WYSE_11 " \001(3)              \001Funct-3          \001  \001MARGIN           \001Shift F12        \001"
K_WYSE_12 " \001(4)              \001Funct-4          \001  \001MENU             \001F3               \001"
K_WYSE_13 " \001(5)              \001Funct-5          \001  \001NEXT             \001Shift F4         \001"
K_WYSE_14 " \001(6)              \001Funct-6          \001  \001NEXT-WINDOW      \001F13              \001"
K_WYSE_15 " \001(7)              \001Funct-7          \001  \001PAGE DOWN        \001PAGE Next        \001"
K_WYSE_16 " \001(8)              \001Funct-8          \001  \001PAGE UP          \001Shift Prev PAGE  \001"
K_WYSE_17 " \001BACKSPACE        \001Del              \001  \001PICK-COPY        \001F9               \001"
K_WYSE_18 " \001BACKTAB          \001Shift TAB        \001  \001PICK-UP          \001F7               \001"
K_WYSE_19 " \001BEGIN-LINE       \001Shift F5         \001  \001PREVIOUS         \001Shift F3         \001"
K_WYSE_20 " \001BREAK            \001CTL-C  **        \001  \001PRINT            \001Funct-p          \001"
K_WYSE_21 " \001BOX-MARK         \001F14              \001  \001PUT-COPY         \001F10              \001"
K_WYSE_22 " \001CANCEL           \001F2               \001  \001PUT-DOWN         \001F8               \001"
K_WYSE_23 " \001CENTER           \001Shift F14        \001  \001QUIT             \001CTL- or CTL-| ** \001"
K_WYSE_24 " \001DELETE-LINE      \001Shift Line DEL Ch\001  \001QUOTE            \001Funct-q          \001"
K_WYSE_25 " \001DELETE-CHARACTER \001DEL Char         \001  \001REFRESH          \001Funxt-z          \001"
K_WYSE_26 " \001DO               \001Funct-x          \001  \001REPLACE          \001Shift Repl       \001"
K_WYSE_27 " \001DOWN-ARROW       \001Down arrow       \001  \001RESTORE          \001Shift F1         \001"
K_WYSE_28 " \001END-LINE         \001Shift F6         \001  \001RETURN           \001Enter            \001"
K_WYSE_29 " \001ENTER            \001Line INS Char    \001  \001RIGHT            \001Shift F16        \001"
K_WYSE_30 " \001EXECUTE          \001Line INS Char    \001  \001RIGHT-ARROW      \001Right arrow      \001"
K_WYSE_31 " \001EXIT             \001F16              \001  \001SAVE             \001Funct-s          \001"
K_WYSE_32 " \001FONT             \001F15              \001  \001SEARCH DOWN      \001Shift F8         \001"
K_WYSE_33 " \001FORMAT           \001F5               \001  \001SEARCH UP        \001Shift F7         \001"
K_WYSE_34 " \001GO-TO            \001Shift F2         \001  \001SET-TAB          \001Funct-v          \001"
K_WYSE_35 " \001HELP             \001F1               \001  \001TAB              \001Tab              \001"
K_WYSE_36 " \001HOME             \001Shift F11        \001  \001TEXT-MARK        \001Funct-t          \001"
K_WYSE_37 " \001INSERT-LINE      \001Shift Line INS Ch\001  \001UP-ARROW         \001Up arrow         \001"
K_WYSE_38 " \001INSERT-MODE      \001Shift Ins Repl   \001  \001USE              \001F6               \001"
K_WYSE_39 " \001LAST-ARG         \001Shift F13        \001  \001WINDOW           \001Funct-w          \001"
K_WYSE_40 " \001LEFT             \001Shift F15        \001  \001ZOOM-IN          \001Shift F9         \001"
K_WYSE_41 " \001LEFT-ARROW       \001Left Arrow, Backs\001  \001ZOOM-OUT         \001Shift F10        \001"
K_WYSE_42 " \001LINES DOWN       \001F12              \001  \006\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\022\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\007"
K_WYSE_45 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_WYSE_46 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_WYSE_47 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_WYSE_48 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_WYSE_49 ""
$
$
$ WYSE30 KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_WYSE30_1 "\011\011\011      Keyboard - wyse30"
K_WYSE30_4 "\011\011\011       Input Sequences"
K_WYSE30_6 "  Editor Function     Key Name           Editor Function     Key Name"
K_WYSE30_8 "\001(1)              \001Funct 1          \001  \001LINES UP         \001Funct keypad 3   \001"
K_WYSE30_9 "\001(2)              \001Funct 2          \001  \001LOCAL-MENU       \001Funct F3         \001"
K_WYSE30_10 "\001(3)              \001Funct 3          \001  \001MARGIN           \001Funct m          \001"
K_WYSE30_11 "\001(4)              \001Funct 4          \001  \001MENU             \001F3               \001"
K_WYSE30_12 "\001(5)              \001Funct 5          \001  \001NEXT             \001Funct F2         \001"
K_WYSE30_13 "\001(6)              \001Funct 6          \001  \001NEXT-WINDOW      \001Funct keypad 2   \001"
K_WYSE30_14 "\001(7)              \001Funct 7          \001  \001PAGE DOWN        \001Ctrl keypad 3    \001"
K_WYSE30_15 "\001(8)              \001Funct 8          \001  \001PAGE UP          \001Ctrl keypad 3    \001"
K_WYSE30_16 "\001BACKSPACE        \001Del              \001  \001PICK-COPY        \001Funct keypad 6   \001"
K_WYSE30_17 "\001BACKTAB          \001Shift TAB        \001  \001PICK-UP          \001Ctrl keypad 6    \001"
K_WYSE30_18 "\001BEGIN-LINE       \001Funct <-         \001  \001PREVIOUS         \001Funct F1         \001"
K_WYSE30_19 "\001BOX-MARK         \001Funct keypad 4   \001  \001PRINT            \001Funct p          \001"
K_WYSE30_20 "\001BREAK            \001CTL-C  **        \001  \001PUT-COPY         \001Funct keypad 7   \001"
K_WYSE30_21 "\001CANCEL           \001F2               \001  \001PUT-DOWN         \001Ctrl keypad 7    \001"
K_WYSE30_22 "\001CENTER           \001Funct keypad 5   \001  \001QUIT             \001CTL-\\ or CTL-| **\001"
K_WYSE30_23 "\001DELETE-CHARACTER \001Ctrl keypad 8    \001  \001QUOTE            \001Funct q          \001"
K_WYSE30_24 "\001DELETE-LINE      \001Ctrl keypad 5    \001  \001REFRESH          \001Funct z          \001"
K_WYSE30_25 "\001DO               \001Funct x          \001  \001REPLACE          \001Ctrl keypad -    \001"
K_WYSE30_26 "\001DOWN-ARROW       \001Down arrow       \001  \001RESTORE          \001Funct keypad -   \001"
K_WYSE30_27 "\001END-LINE         \001Funct ->         \001  \001RETURN           \001Enter            \001"
K_WYSE30_28 "\001ENTER            \001F4               \001  \001RIGHT            \001Contr F2         \001"
K_WYSE30_29 "\001EXECUTE          \001F4               \001  \001RIGHT-ARROW      \001Right arrow      \001"
K_WYSE30_30 "\001EXIT             \001Funct d          \001  \001SAVE             \001Funct s          \001"
K_WYSE30_31 "\001FONT             \001Funct keypad 1   \001  \001SEARCH DOWN      \001Contr F4         \001"
K_WYSE30_32 "\001FORMAT           \001Funct keypad 8   \001  \001SEARCH UP        \001Contr F3         \001"
K_WYSE30_33 "\001GO-TO            \001Ctrl keypad 9    \001  \001SET-TAB          \001Funct v          \001"
K_WYSE30_34 "\001HELP             \001F1               \001  \001TAB              \001Tab              \001"
K_WYSE30_35 "\001HOME             \001Funct Home       \001  \001TEXT-MARK        \001Funct t          \001"
K_WYSE30_36 "\001INSERT-LINE      \001Ctrl keypad 4    \001  \001UP-ARROW         \001Up arrow         \001"
K_WYSE30_37 "\001INSERT-MODE      \001Ctrl keypad ,    \001  \001USE              \001Funct keypad 9   \001"
K_WYSE30_38 "\001LAST-ARG         \001Funct F4         \001  \001WINDOW           \001Funct w          \001"
K_WYSE30_39 "\001LEFT             \001Contr F1         \001  \001ZOOM-IN          \001Funct Down-arrow \001"
K_WYSE30_40 "\001LEFT-ARROW       \001Left Arrow, Backs\001  \001ZOOM-OUT         \001Funct Up-arrow   \001"
K_WYSE30_41 "\001LINES DOWN       \001Funct keypad .   \001  \006\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\022\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\007"
K_WYSE30_44 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_WYSE30_45 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_WYSE30_46 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_WYSE30_47 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_WYSE30_48 ""
$
$
$ XTERM KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_XTERM_1 "\011\011\011        Keyboard - xterm"
K_XTERM_6 "\011\011\011       Input Sequences"
K_XTERM_8 " Name              Value                Name             Value"
K_XTERM_10 "\001(1)              \001CTRL-a F1        \001  \001LINES UP         \001Page Up          \001"
K_XTERM_11 "\001(2)              \001CTRL-a F2        \001  \001LOCAL-MENU       \001F3               \001"
K_XTERM_12 "\001(3)              \001CTRL-a F3        \001  \001MARGIN           \001CTRL-a m         \001"
K_XTERM_13 "\001(4)              \001CTRL-a F4        \001  \001MENU             \001F2               \001"
K_XTERM_14 "\001(5)              \001CTRL-a F5        \001  \001NEXT             \001CTRL-a F12       \001"
K_XTERM_15 "\001(6)              \001CTRL-a F6        \001  \001NEXT-WINDOW      \001CTRL-a n         \001"
K_XTERM_16 "\001(7)              \001CTRL-a F7        \001  \001PAGE DOWN        \001CTRL-a Page Down \001"
K_XTERM_17 "\001(8)              \001CTRL-a F8        \001  \001PAGE UP          \001CTRL-a Page Up   \001"
K_XTERM_18 "\001BACKSPACE        \001BACKSPACE        \001  \001PICK-COPY        \001F9               \001"
K_XTERM_19 "\001BEGIN-LINE       \001CTRL-a left curso\001  \001PICK-UP          \001F7               \001"
K_XTERM_20 "\001BOX-MARK         \001CTRL-a b         \001  \001PREVIOUS         \001CTRL-a F11       \001"
K_XTERM_21 "\001BREAK            \001CTRL-c **        \001  \001PRINT            \001CTRL-a p         \001"
K_XTERM_22 "\001CANCEL           \001F4               \001  \001PUT-COPY         \001F10              \001"
K_XTERM_23 "\001CENTER           \001CTRL-a c         \001  \001PUT-DOWN         \001F8               \001"
K_XTERM_24 "\001DELETE-CHARACTER \001Delete           \001  \001QUIT             \001CTRL-\\  **       \001"
K_XTERM_25 "\001DELETE-LINE      \001CTRL-a Delete    \001  \001QUOTE            \001CTRL-a q         \001"
K_XTERM_26 "\001DO               \001CTRL-a x         \001  \001REFRESH          \001CTRL-a z         \001"
K_XTERM_27 "\001DOWN-ARROW       \001down cursor      \001  \001REPLACE          \001CTRL-a =         \001"
K_XTERM_28 "\001END-LINE         \001CTRL-a right curs\001  \001RESTORE          \001CTRL-a i         \001"
K_XTERM_29 "\001ENTER            \001CTRL-a Enter     \001  \001RETURN           \001Enter            \001"
K_XTERM_30 "\001EXECUTE          \001CTRL-a Enter     \001  \001RIGHT            \001CTRL-a r         \001"
K_XTERM_31 "\001EXIT             \001CTRL-a d         \001  \001RIGHT-ARROW      \001right cursor     \001"
K_XTERM_32 "\001FONT             \001CTRL-a f         \001  \001SAVE             \001CTRL-a s         \001"
K_XTERM_33 "\001FORMAT           \001F5               \001  \001SEARCH DOWN      \001CTRL-a down curso\001"
K_XTERM_34 "\001GO-TO            \001CTRL-a g         \001  \001SEARCH UP        \001CTRL-a up cursor \001"
K_XTERM_35 "\001HELP             \001F1               \001  \001SET-TAB          \001CTRL-a v         \001"
K_XTERM_36 "\001HOME             \001CTRL-a h         \001  \001TAB              \001Tab              \001"
K_XTERM_37 "\001INSERT-LINE      \001F6               \001  \001-TAB             \001CTRL-t           \001"
K_XTERM_38 "\001INSERT-MODE      \001Insert           \001  \001TEXT-MARK        \001CTRL-a t         \001"
K_XTERM_39 "\001LAST-ARG         \001CTRL-a a         \001  \001UP-ARROW         \001up cursor        \001"
K_XTERM_40 "\001LEFT             \001CTRL-a l         \001  \001USE              \001CTRL-a u         \001"
K_XTERM_41 "\001LEFT-ARROW       \001left cursor      \001  \001WINDOW           \001CTRL-a w         \001"
K_XTERM_42 "\001LINEFEED         \001LINEFEED         \001  \001ZOOM-IN          \001F11              \001"
K_XTERM_43 "\001LINES DOWN       \001Page Down        \001  \001ZOOM-OUT         \001F12              \001"
K_XTERM_46 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_XTERM_47 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_XTERM_48 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_XTERM_49 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_XTERM_50 ""
$
$
$ DTTERM KEYBOARD LAYOUT - MESSAGE CATALOG ENTRIES
$
K_DTTERM_1 "\011\011\011        Keyboard - dtterm"
K_DTTERM_6 "\011\011\011       Input Sequences"
K_DTTERM_8 " Name              Value                Name             Value"
K_DTTERM_10 "\001(1)              \001CTRL-a F1        \001  \001LINES UP         \001Page Up          \001"
K_DTTERM_11 "\001(2)              \001CTRL-a F2        \001  \001LOCAL-MENU       \001F3               \001"
K_DTTERM_12 "\001(3)              \001CTRL-a F3        \001  \001MARGIN           \001CTRL-a m         \001"
K_DTTERM_13 "\001(4)              \001CTRL-a F4        \001  \001MENU             \001F2               \001"
K_DTTERM_14 "\001(5)              \001CTRL-a F5        \001  \001NEXT             \001CTRL-a F12       \001"
K_DTTERM_15 "\001(6)              \001CTRL-a F6        \001  \001NEXT-WINDOW      \001CTRL-a n         \001"
K_DTTERM_16 "\001(7)              \001CTRL-a F7        \001  \001PAGE DOWN        \001CTRL-a Page Down \001"
K_DTTERM_17 "\001(8)              \001CTRL-a F8        \001  \001PAGE UP          \001CTRL-a Page Up   \001"
K_DTTERM_18 "\001BACKSPACE        \001BACKSPACE        \001  \001PICK-COPY        \001F9               \001"
K_DTTERM_19 "\001BEGIN-LINE       \001CTRL-a left curso\001  \001PICK-UP          \001F7               \001"
K_DTTERM_20 "\001BOX-MARK         \001CTRL-a b         \001  \001PREVIOUS         \001CTRL-a F11       \001"
K_DTTERM_21 "\001BREAK            \001CTRL-c **        \001  \001PRINT            \001CTRL-a p         \001"
K_DTTERM_22 "\001CANCEL           \001F4               \001  \001PUT-COPY         \001F10              \001"
K_DTTERM_23 "\001CENTER           \001CTRL-a c         \001  \001PUT-DOWN         \001F8               \001"
K_DTTERM_24 "\001DELETE-CHARACTER \001Delete           \001  \001QUIT             \001CTRL-\\  **       \001"
K_DTTERM_25 "\001DELETE-LINE      \001CTRL-a Delete    \001  \001QUOTE            \001CTRL-a q         \001"
K_DTTERM_26 "\001DO               \001CTRL-a x         \001  \001REFRESH          \001CTRL-a z         \001"
K_DTTERM_27 "\001DOWN-ARROW       \001down cursor      \001  \001REPLACE          \001CTRL-a =         \001"
K_DTTERM_28 "\001END-LINE         \001CTRL-a right curs\001  \001RESTORE          \001CTRL-a i         \001"
K_DTTERM_29 "\001ENTER            \001CTRL-a Enter     \001  \001RETURN           \001Enter            \001"
K_DTTERM_30 "\001EXECUTE          \001CTRL-a Enter     \001  \001RIGHT            \001CTRL-a r         \001"
K_DTTERM_31 "\001EXIT             \001CTRL-a d         \001  \001RIGHT-ARROW      \001right cursor     \001"
K_DTTERM_32 "\001FONT             \001CTRL-a f         \001  \001SAVE             \001CTRL-a s         \001"
K_DTTERM_33 "\001FORMAT           \001F5               \001  \001SEARCH DOWN      \001CTRL-a down curso\001"
K_DTTERM_34 "\001GO-TO            \001CTRL-a g         \001  \001SEARCH UP        \001CTRL-a up cursor \001"
K_DTTERM_35 "\001HELP             \001F1               \001  \001SET-TAB          \001CTRL-a v         \001"
K_DTTERM_36 "\001HOME             \001CTRL-a h         \001  \001TAB              \001Tab              \001"
K_DTTERM_37 "\001INSERT-LINE      \001F6               \001  \001-TAB             \001CTRL-t           \001"
K_DTTERM_38 "\001INSERT-MODE      \001Insert           \001  \001TEXT-MARK        \001CTRL-a t         \001"
K_DTTERM_39 "\001LAST-ARG         \001CTRL-a a         \001  \001UP-ARROW         \001up cursor        \001"
K_DTTERM_40 "\001LEFT             \001CTRL-a l         \001  \001USE              \001CTRL-a u         \001"
K_DTTERM_41 "\001LEFT-ARROW       \001left cursor      \001  \001WINDOW           \001CTRL-a w         \001"
K_DTTERM_42 "\001LINEFEED         \001LINEFEED         \001  \001ZOOM-IN          \001F11              \001"
K_DTTERM_43 "\001LINES DOWN       \001Page Down        \001  \001ZOOM-OUT         \001F12              \001"
K_DTTERM_46 "** The BREAK and QUIT characters can be changed using the \"stty\" command"
$ Do not translate BREAK, QUIT and stty
$
K_DTTERM_47 "   from the command line.  They can be viewed by typing \"stty -a\" on the"
$ Do not translate stty -a
$
K_DTTERM_48 "   command line; the BREAK character is labeled \"intr=\", the QUIT character"
$ Do not translate BREAK, intr= and QUIT
$
K_DTTERM_49 "   is labeled \"quit=\"."
$ Do not translate quit
$
K_DTTERM_50 ""
$
