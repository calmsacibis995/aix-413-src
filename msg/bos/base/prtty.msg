$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)46	1.1  src/msg/bos/base/prtty.msg, bosmsg, msg411, GOLD410 2/9/94 09:11:56
$
$ COMPONENT_NAME: (INED) INed Editor
$ 
$ ORIGINS:  27
$ 
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991 
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$quote "
$
$set MS_PRTTY 
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc.
$
USAGE "Usage: %s [-l[Number]] [Files]"
$ Translate only the words "Usage" and "files", and Number.
$
ARGN "0611-402 Specify a positive integer after the -l flag."
$
TERM "0611-403 Cannot write to the terminal."
$
CNOPEN "0611-404 Cannot read %s.\n"
$ Here, %s will be replaced with the name of a file.
$ This message must end in \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
TRTN "Press Enter to continue printing."
$
CNRTERM "0611-405 Cannot read terminal description file %s."
$
TERMNS "0611-406 The TERM shell variable is not in the environment.\n\
\tTo activate prtty, set and export the TERM variable."
$
READERR "Internal Error (Cannot read terminal description record %d.)"
$ Do not translate "error reading terminal description record %d"
$
TUNK "0611-407 Terminal type %s is not recognized."
$
NONPTR "Internal Error (read non-POINTER terminal description record %d)."
$ Do not translate "read non-POINTER terminal description record %d"
