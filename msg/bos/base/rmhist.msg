$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)91	1.1  src/msg/bos/base/rmhist.msg, bosmsg, msg411, GOLD410 2/9/94 09:14:45
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
$set MS_RMHIST 
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc.
$
M_MINUS_K "rmhist: 0611-476 Specify a positive integer for the -k flag.\n"
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_USAGE "Usage: rmhist [-f] [-d] [-kNumber] File...\n"
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_ILL_ARG "%1$s: 0611-478 The %2$c flag is not recognized.\n"
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_STAT "0611-479 Cannot access file %s."
$
M_FILE_TYPE "File %s does not contain update records."
$
M_BACKUP "0611-480 Cannot create a backup file for %s."
$
M_RENAME "0611-481 Cannot change file %1$s to %2$s."
$
M_REMOVE "0611-482 Cannot remove file %s."
$
M_CHOWN "0611-483 Cannot change file %1$s\n\
\tto owner number %2$d and group number %3$d."
$
M_OPEN "0611-484 Cannot open file %s."
$
M_CREATE "0611-485 Cannot create file %s."
$
M_RMHIST "rmhist: Removing history from file %s.\n"
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_BAD_SF_READ "Internal Error (sf_read failure, record %d, file %s)."
$ Do not translate "sf_read failure, record %d, file %s"
$
M_S_READ "Internal Error (s_read failure (%d) on record %d)"
$ Do not translate "s_read failure (%d) on record %d"
$
M_SF_WRITE "Internal Error (sf_write failure (%d) on record %d,\n\
\tfile %s)"
$ Do not translate "sf_write failure (%d) on record %d, file '%s'"
$
M_SF_CLOSE "Internal Error (sf_close failure (%d) on file '%s')"
$ Do not translate "sf_close failure (%d) on file '%s'"
$
M_WARNING "rmhist: Warning: "
$ Do not translate "rmhist".
$
M_OPEN_INPUT "0611-486 Cannot open input file %s."
$
M_REMOVING "rmhist: Removing history before %2$.24s from file %1$s.\n"
$ Here, %s is replaced with a time/date string.
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_UNEXP_COOKIE "Internal Error (bad history cookie in file)."
$ Do not translate "bad history cookie in file."
$
M_IO_ERR_1 "Internal Error (I/O error (1) on output file)"
$ Do not translate "I/O error (1) on output file"
$
M_IO_ERR_2 "Internal Error (I/O error (2) on output file)"
$ Do not translate "I/O error (2) on output file"
$
M_IO_ERR_3 "Internal Error (I/O error (3) on output file)"
$ Do not translate "I/O error (3) on output file"
$
M_END_COOKIE "Internal Error (unexpected END cookie)."
$ Do not translate "unexpected END cookie"
$
M_READ "Internal Error (record read error)."
$ Do not translate "record read error"
$
M_CHACL "Internal Error (acl_put ('%s') failed)."
$ Do not translate "acl_put ('%s') failed"
