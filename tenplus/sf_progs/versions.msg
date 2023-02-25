$ @(#)53	1.1  src/tenplus/sf_progs/versions.msg, tenplus, tenplus411, GOLD410 3/17/93 14:03:50
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
$set MS_VERSIONS
$
$ Translator:  do not translate anything that looks like %s or %d or
$ %1$s or %2$d etc.
$
M_USAGE "Usage: %s File"
$ Here, the %s will be replaced by the name of the program, either "history"
$ or "versions".
$
M_OPEN "%1$s: 0611-202 Cannot open file %2$s."
$ Here, the first %s will be replaced by the name of the program, either
$ "history" or "versions". Do not move it.
$
M_S_READ "%s: 0611-203 There is a read error."
$ Here, the first %s will be replaced by the name of the program, either
$ "history" or "versions".  Do not move it.  Do not translate read
$
M_RECORD "Record %d:\n"
$ Here, the word "record" is a noun.
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_WRITE "%s: 0611-204 The write to standard output failed."
$ Do not translate write
$
$ This has nothing to do with the INSERT-LINE key, and should not be changed.
$
M_INSERT "-----> Insert %d records\n\n"
$ This message is used to produce part of a table of output.  The arrow
$ should be included.  Don't worry about making a complete sentence.
$ This message must end with \n\n
$ On the altu edit screen, this will appear as '\n\n'.  If you then
$ display the source, you will see the '\n\n'
$
$ This has nothing to do with the DELETE-LINE key, and should not be changed.
$
M_DELETE "-----> Delete %d records\n\n"
$ This message is used to produce part of a table of output.  The arrow
$ should be included.  Don't worry about making a complete sentence.
$ This message must end with \n\n
$ On the altu edit screen, this will appear as '\n\n'.  If you then
$ display the source, you will see the '\n\n'
$
M_CHNG_IDX "-----> Change current index to %d\n\n"
$ This message is used to produce part of a table of output.  The arrow
$ should be included.  Don't worry about making a complete sentence.
$ This message must end with \n\n
$ On the altu edit screen, this will appear as '\n\n'.  If you then
$ display the source, you will see the '\n\n'
$
M_START "=====> Start token:  user number = %1$d, group number= %2$d,\n\
                     time = %3$s\n"
$ This message is used to produce part of a table of output.  The arrow
$ should be included.  Don't worry about making a complete sentence.
$ Indent the second line to line up with the first.
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_END "-----> End token, array location = %ld\n\n"
$ This message is used to produce part of a table of output.  The arrow
$ should be included.  Don't worry about making a complete sentence.
$ This message must end with \n\n
$ On the altu edit screen, this will appear as '\n\n'.  If you then
$ display the source, you will see the '\n\n'
$
M_COMMENT "-----> Comment token: '%s'\n\n"
$ This message is used to produce part of a table of output.  The arrow
$ should be included.  Don't worry about making a complete sentence.
$ This message must end with \n\n
$ On the altu edit screen, this will appear as '\n\n'.  If you then
$ display the source, you will see the '\n\n'
$
M_ARRAY "-----> Start of array token, current location = %ld\n"
$ This message is used to produce part of a table of output.  The arrow
$ should be included.  Don't worry about making a complete sentence.
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
$
M_READ_ARRAY "%s: 0611-205 %s: Cannot read array from structured file."
$
M_REC_ARRAY "The record array looks like:\n\n"
$ This message must end with \n\n
$ On the altu edit screen, this will appear as '\n\n'.  If you then
$ display the source, you will see the '\n\n'
$
M_ILL_COOKIE "0611-206 ***** Token of type %d in input file is not valid. *****\n"
$ This message must end with \n
$ On the altu edit screen, this will appear as '\n'.  If you then
$ display the source, you will see the '\n'
