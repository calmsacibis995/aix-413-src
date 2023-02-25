$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)23	1.1  src/msg/bos/base/ghost.msg, bosmsg, msg411, GOLD410 2/9/94 08:59:48
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
$set MS_GHOST 
$
$ Translator:  do not translate anything that looks like %s or %d or
$ %1$s or %2$d etc.  Do not translate ghost
$
M_USAGE "Usage:  ghost [-d] [-p] Oldfile [Newfile [mm/dd/yy [hh:mm:ss]]]"
$ In this message, translate "usage", "oldfile" and "newfile".
$ mm/dd/yy stands for month/day/year.  Please translate it to appropriate
$ initials.
$ hh:mm:ss stands for hours:minutes:seconds.  Please translate it to
$ appropriate initials.
$
M_OPTION "ghost: 0611-427  %s is not a recognized flag."
$ Do not translate "ghost"
$
M_DATE "ghost: 0611-428 Specify the date in the form mm/dd/yy or mm/dd."
$ Do not translate "ghost"
$ m/d/y stands for month/day/year.  Please translate it to appropriate
$ initials.
$
M_TIME_SPEC "ghost: 0611-429 Specify the time in the form hh:mm:ss or hh:mm."
$ Do not translate "ghost"
$ h:m:s stands for hours:minutes:seconds.  Please translate it to appropriate
$ initials.
$
M_RECONSTRUCTING "ghost: Reconstructing file %1$s up to time %2$s.\
\tand saving it as file %3$s.\n"
$ The first %s will be replaced with a file name.
$ The second %s will be replaced with a date/time string, which will include
$ a trailing \n (newline).  This means that you should NOT put a newline
$ after the %2$s. In altu, this will appear to "wrap". Let it. When the
$ message actually gets printed out, the line '\tand saving it as file %3$s'
$ will appear on a line by itself.
$ The last line of this message must end with \n. On the altu edit screen,
$ this will appear as '\n'.  If you then display the source, you will see
$ the '\n'
$
M_OPEN_OLD "ghost: 0611-430 Cannot open the old change file %s."
$ Do not translate "ghost"
$
M_OPEN_TMP "ghost: 0611-431 Cannot open the temporary file %s."
$
M_IO_WRITE "ghost: 0611-432 Cannot write to temporary file %s."
$
M_NO_RECS "ghost: Warning: The output file %s is empty.\n"
$ The last line of this message must end with \n.
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_WRITE "ghost: 0611-433 There is a write error."
$
M_SF_CLOSE "ghost: 0611-434 Cannot close the output file."
$
M_CHACL "Warning: Cannot change the access control list for %s."
$ Here, %s will be replaced with the name of a file.
$
M_UNEXP_END "ghost: Warning: There is an unexpected END token in the input file.\n"
$ Do not translate END
$ The last line of this message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_ILL_COOKIE "ghost: Warning: Token type %d in the input file is not valid.\n"
$ The last line of this message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_S_READ "ghost: Warning: Cannot read the input file.\n"
$ Do not translate "ghost"
$ The last line of this message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_COPY "ghost: 0611-435 Cannot open %s."
$ Here, %s will be replaced with the name of a file.
$
M_CREATE "ghost 0611-436 Cannot create file %s."
$ Here, %s will be replaced with the name of a file.
$
M_IO_READ "ghost: 0611-437 Cannot read from file %s."
$ Here, %s will be replaced with the name of a file.
$
M_WRITE_ERR "ghost: 0611-438 Cannot write to file %s."
$ Here, %s will be replaced with the name of a file.
$
M_IO_CLOSE "ghost: 0611-439 Cannot close file %s."
$ Here, %s will be replaced with the name of a file.
