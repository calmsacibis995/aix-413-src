$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)97	1.1  src/msg/bos/base/tdigest.msg, bosmsg, msg411, GOLD410 2/9/94 09:24:13
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
$set MS_TDIGEST 
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc.
$
M_USAGE_MESSAGE "Usage: tdigest InputFile OutputFile\n"
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_READ "tdigest: 0611-267 Cannot read %s.\n"
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_CREATE "tdigest: 0611-268 Cannot create %s.\n"
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_READ_RECORD "0611-269 Internal Error (read error on %s, record %d)."
$ Do not translate "read error on %s, record %d"
$
M_WRITE_RECORD "0611-270 Internal Error (write error on %s, record %d)"
$ Do not translate "write error on %s, record %d"
$
M_CLOSE "0611-271 Internal Error (write error while closing %s)."
$ Do not translate "write error while closing %s"
$
M_NO_DEFAULT "Default keyboard definition must be first record in %s."
$
M_NO_INSEQ "Input record %d has no defined input sequences."
$
M_NO_TERM_NAME "Input record %d has no defined terminal name."
$
M_UNK_CANONICAL "Input record %d input canonical '%s' unknown."
$
M_SAME_KEY "Input record %d Same input sequence used for %s and %s."
$
M_ALREADY_ASSIGNED "Input record %d input char 0%o already assigned (canonical '%s')."
$
M_ALREADY_IN_DEFAULT "Input record %d input char 0%o already in default (canonical '%s')."
