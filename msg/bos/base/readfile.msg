$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)74	1.1  src/msg/bos/base/readfile.msg, bosmsg, msg411, GOLD410 2/9/94 09:13:36
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
$
$ Note to documentation reviewer:  these messages are used by a program
$ that generates a report about the contents of a file.  Thus it is not
$ really an error for this program to report unrecognized record types,
$ damaged records, or whatever.  These are normal reports about what is in
$ the file.
$
$quote "
$
$set MS_READFILE
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc.
$
M_USAGE "Usage: readfile [-dghstu?] [+n] [-n] [-o Outputfile] [-m MsgCatalog SetNumber] [File...]\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_MINUS_M "Specify a message catalog file and set number for the -m flag.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_MINUS_O "0611-452 Specify an output file for the -o flag.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_DUP_M "Use the -m flag only once.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_DUP_O "0611-453 Use the -o flag only once.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_FLAGS_CONFLICT "-m and -o flags conflict.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_WRITE "0611-454 Cannot create the specified output file.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_INVALID "0611-455 Cannot access the specified output file.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_NO_WRITE "0611-456 You must have write permission for the specified output file.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_NO_WRITE_P "0611-457 You must have write permission for file %s.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_UNLINK "0611-458 Cannot unlink %s.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_LINK "0611-459 Cannot link %1$s to %2$s.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_NUMRECS "File %1$s contains %2$d records.\n\n"
$ Here, %s is the name of a file, and %d is the number of records in the file.
$ This message must end with \n\n  (Placing a \n\n at end of line in
$ altu will place a \n\n in the source at the end of the line
$
M_BROKEN "0611-460 File %s is not functional.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_REC "{Record %d} "
$
M_NULL_PTR "NULL pointer\n"
$ Do not translate NULL.
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_ILL_OBJ "The object type is not recognized.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_UNK_NODE "Type code %1$d is not recognized for this node.\n\
The count value is %2$d.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_PTR_TYPE "Type = T_POINTER, Name = '%1$s', Size = %2$d, Flags =0%3$o\n"
$ Do not translate T_POINTER.
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_CHAR_TYPE "Type = T_CHAR, Name = '%1$s', Size = %2$d, Flags = 0%3$o\n"
$ Do not translate T_CHAR.
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_PTR_SHORT "Type = pointer, Name = %s:\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_CHAR_SHORT "Type = character, Name = %s:\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_OPEN "readfile: 0611-461 Cannot open file %s.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_NO_FILE "readfile: 0611-462 File %s does not exist or permission is denied.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_NOT_NORMAL "readfile: 0611-463 File %s is not a normal file.\n\
\tSpecify only normal files (not devices) to readfile.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_IO_ERROR "readfile: Internal Error (i/o error in sf_open on file %s).\n"
$ Do not translate "i/o error in sf_open on file %s"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_EXIST "readfile: 0611-464 File %s does not exist.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_REC_ARRAY "readfile: 0611-465 A record array in file %s is not functional.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_REC_READ "0611-466 Cannot read records from file %s.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_BAD_CHAR "readfile: 0611-467 File %1$s contains character 0%2$o which is not valid.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_NON_PRINTABLE "readfile: Assuming that the input file is not of a printable type.\n"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_BAD_STACK "Internal Error (ancget stack index %d not valid).\n"
$ Do not translate "invalid ancget stack index %d"
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_BAD_INDEX "Internal Error (ancset index %d not valid).\n"
$ Do not translate "invalid ancset index %d."
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_ANCESTOR "Internal Error (Cannot get ancestor %d).\n"
$ Do not translate "unable to get ancestor %d."
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_SONTYP "Internal Error (sontyp %d not expected).\n"
$ Do not translate "unexpected sontyp %d."
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_NO_DESC "Internal Error (no descendants).\n"
$ Do not translate "no descendants."
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
$
M_ERR_MSG "Error\n"
$ This message must end with \n
$
M_UNEXP_TYPE "Internal Error (object of unexpected type %d).\n"
$ Do not translate "object of unexpected type %d."
$ This message must end with \n  (Placing a \n at end of line in altu will
$ place a \n in the source at the end of the line
