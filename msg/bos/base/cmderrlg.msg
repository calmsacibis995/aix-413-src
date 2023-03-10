$ ENGLISH_SOURCE_VERSION= 1.26.3.2
$ @(#)56  1.7  src/msg/bos/base/cmderrlg.msg, bosmsg, msg411, 9433A411a 8/11/94 12:41:06
$ COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
$
$ FUNCTIONS: .msg message catalog file for CMDERRLG
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$set 1
$quote "

CAT_DEM_DATABASE      "\
0315-001 Failure to open the logfile '%1$s' for writing.\n\
Possible causes are:\n\
1. The file exists but the invoking process does not have write\n\
   permission. \n\
2. The file exists but the directory '%2$s' does not have write\n\
   permission.\n\
3. The file exists but it is not a valid error logfile.  Remove\n\
   the file and restart the errdemon.\n\
4. The file does exist and the directory '%3$s' does not have enough\n\
   space available. The minimum logfile size is %4$d bytes.\n"
$ $ liberrlg/udbutil.c
$ %1$s will be expanded to the name of the error log file
$ %2$s will be expanded to a directory name which contains the file
$ %3$s will be expanded to the directory name which contains the file
$ %4$s will be expanded to the minimum error log file size 

CAT_USAGE_DEAD  "\
Usage:\n\
errdead -i filename dumpfile\n\
\n\
Extract error log entries from a system dump and insert into error log.\n\
-i filename   Put error log entries into the supplied error log file.\n"
$ $ errdead/main.c
$ 'dumpfile' means the name of the file containing a system dump image.
$ System dump refers to a dump created from a system panic NOT a core
$ dump.

CAT_DEM_USAGE	"\
Usage:\n\
errdemon -i filename -s value -B value | -l\n\
-i filename  Uses the error log file specified by the filename parameter.\n\
             If this flag is not specified, the default system error log file,\n\
             /var/adm/ras/errlog, is used.\n\
-s value     Set the error log file maximum size.\n\
             Valid range is 4096 bytes to your ulimit in bytes.\n\
-B value     Set the error log file memory buffer size.\n\
	     This must be a minimum of %1$d bytes.\n\
-l           Show the error log attributes.\n"
$ errdemon/main.c
$ %1$d will be expanded to a the minimum error log memory buffer size 

CAT_DEAD_NO_DUMP    "\
0315-004 Specify a dump file.\n"
$ errdead/main.c
$ The "dump file" refers to a dump image generated by a system
$ panic.

CAT_RPT_CODEPOINT_W      "\
0315-021 Cannot open error message catalog %1$s.\n\
The error report will still run, but it will not have explanatory messages.\n"
$ errpt/main.c errinstall/main.c
$ %1$s will be expanded to the name of the message catalog
$ This is the file that contains the message id's used by the
$ error log subsystem.

CAT_CLR_NEGDAYS "\
0315-025 You entered %1$s for number of days.\n\
You must enter a positive integer.\n"
$ errclear/main.c
$ %1$s is the number of days of errors you want to delete

CAT_RPT_ERRDESC         "Description"
CAT_RPT_PROBCAUSES      "Probable Causes"
CAT_RPT_USERCAUSES      "User Causes"
CAT_RPT_INSTCAUSES      "Install Causes"
CAT_RPT_FAILCAUSES      "Failure Causes"
CAT_RPT_RECACTIONS      "Recommended Actions"
CAT_RPT_DETAIL_DATA     "Detail Data"
CAT_RPT_SYMPTOM_DATA    "Symptom Data"

$ errpt/pr.c
$ These are used internally as part of a larger message and do not have
$ message numbers.

CAT_RPT_HDR "\
IDENTIFIER TIMESTAMP  T C RESOURCE_NAME  DESCRIPTION\n"
$ Used internally as part of a larger message. Does not have message numbers.

CAT_RPT_AHDR "\
---------------------------------------------------------------------------\n\
LABEL:\t\t%1$s\n\
IDENTIFIER:\t%2$08X\n\
\n\
Date/Time:       %3$s\n\
Sequence Number: %4$d\n\
Machine Id:      %5$s\n\
Node Id:         %6$s\n\
Class:           %7$s\n\
Type:            %8$s\n\
Resource Name:   %9$s\n"
$ Used internally as part of a larger message. Does not have message numbers.

CAT_RPT_OHDR "\
Resource Class:  %1$s\n\
Resource Type:   %2$s\n\
Location:        %3$s\n"
$ Used internally as part of a larger message. Does not have message numbers.

CAT_RPT_VHDR "\
VPD:             \n"
$ Used internally as part of a larger message. Does not have message numbers.

CAT_RPT_THDR "\
Id       Label               Type CL Description\n"
$ Used internally as part of a larger message. Does not have message numbers.

CAT_RPT_TAHDR "\
---------------------------------------------------------------------------\n\
IDENTIFIER %1$08X\n\
\n\
Label: %2$s\n\
Class:     %3$s\n\
Type:      %4$s\n\
Loggable: %5$s   Reportable: %6$s   Alertable: %7$s\n\n"
$ Used internally as part of a larger message. Does not have message numbers.

CAT_UPD_CANNOTINIT  "\
0315-041 Cannot initialize error template file '%1$s'.\n\
         This could be because the directory for the file does not exist,\n\
         that the file does not have read and write permission,\n\
         or that a file with the wrong format has this same name.\n"
$ errupdate/main
$ %1$s will be expanded to the name of the template file.

CAT_UPD_XPASS1  "\
0315-042 No change to error template database.\n",
$ errupdate/main
$ %1$d will be the number of syntax errors.

CAT_UPD_XPASS2  "\
0315-043 Exiting after pass 2 with %1$d errors.\n",
$ errupdate/main
$ %1$d will be the number of syntax errors 

CAT_UPD_USAGE       "\
Usage:\n\
errupdate -cfhnpq -y filename filename\n\
\n\
Update the error template database according to the template stanza file\n\
'filename'.\n\
\n\
-c    check   flag. Scan stanza file for syntax errors.\n\
-f    force   flag. Do not stop on detection of duplicates.\n\
-h    header  flag. Generate header file according to description file.\n\
-n    noadd   flag. Similar to 'check' flag. Does not update repository.\n\
-p    override flag. Override normal template checking for alert flag criteria.\n\
-q    quiet   flag. Do not create an undo file.\n\
-y    filename  Uses the error record template file specified by the filename\n\
                parameter.\n\
\n\
If no filename is specified, the input is taken from stdin.\n\
\n\
A #define .h file is written to 'filename.h' that equates the stanza\n\
labels to their generated unique ERROR ID's. If no filename is specified,\n\
the .h file is written to stdout.\n"
$ errupdate/main

CAT_UPD_STAT        "\
%1$d entries added.\n\
%2$d entries deleted.\n\
%3$d entries updated.\n"
$ errupdate/main
$ %1$d will be the template entries added 
$ %2$d will be the template entries deleted
$ %3$d will be the template entries changes 

CAT_UPD_TOOMANY        "\
0315-046 Too many entries for keyword %1$s"
$ errupdate/etcpy.c
$ %1$s will be a keyword value in the input file 

CAT_UPD_TOOMANYDET     "\
0315-047 The template for %1$s has &2$d Detail Data Fields.\n\
Only %3$d are permitted.\n"  
$ errupdate/etcpy.c
$ %1$s will be expanded to a template name 
$ %2$d will be expanded to the number of detail data fields
$ %3$d will be expanded to the valid number of detail data fields 

CAT_UPD_LE_CLASS       "\
0315-048 Specify a Class in the template for %1$s.\n" 
$ errupdate/etcpy.c
$ %1$s will be expanded to a descriptive label

CAT_UPD_LE_TYPE        "\
0315-049 Specify an Err_Type in the template for %1$s.\n" 
$ errupdate/etcpy.c
$ %1$s will be expanded to a descriptive label

CAT_UPD_LE_DESC        "\
0315-050 Specify an Err_Desc in the template for %1$s.\n"
$ errupdate/etcpy.c
$ %1$s will be expanded to a descriptive label

CAT_UPD_LE_CAUSES      "\
0315-051 Specify Prob_Causes in the template for %1$s.\n"
$ errupdate/etcpy.c
$ %1$s will be expanded to a descriptive label

CAT_UPD_LE_CAUSEACTION "\
0315-052 Specify both the cause and the action in the\n\
template for %1$s.\n"
$ errupdate/etcpy.c
$ %1$s will be expanded to a descriptive label

CAT_UPD_LE_ONECAUSE    "\
0315-053 Specify at least one cause and action in the template\n\
for %1$s.\n"
$ errupdate/etcpy.c
$ %1$s will be expanded to a descriptive label

CAT_UPD_LENDET         "\
0315-054 The length, %1$d, of a Detail Data field in the template\n\
for %2$s is too long.  The maximum length allowed is %3$d.\n" 
$ errupdate/etcpy.c
$ %1$d will be expanded to a the length of the detail data field 
$ %2$s will be expanded to a descriptive label

CAT_UPD_PP1  "\
0315-055 Bad value %1$s in the template for %2$s.\n\
         Acceptable values are:\n"
$ errupdate/parseutil.c
$ %1$s will be expanded to the bad value in the template.
$ %2$s will be expanded to a descriptive label
$ After this message will be a list of possible values.

CAT_UPD_PP2  "\
0315-056 Message identifier 0x%1$04X in the template for %2$s\n\
is out of range (0000 - FFFF).\n"
$ 0x%1$04X will be expanded to a message id
$ %2$s will be expanded to a descriptive label

CAT_UPD_PASS1DUPC  "\
0315-057 The Error Identifier %1$08x for the template %2$s is\n\
already defined for a template with the Label %3$s.\n"
$ errupdate/pass1.c
$ %1$08X will be expanded to a message id 
$ %2$s will be expanded to a descriptive label
$ %3$s will be expanded to a descriptive label

CAT_UPD_PASS1IL    "\
0315-058 The Label %1$s is already defined in the input file.\n" 
$ %1$d will be expanded to a descriptive label 

CAT_UPD_PASS1DUPL  "\
0315-059 The Label %1$s is already defined in the template file.\n" 
$ %1$s will be expanded to a descriptive label

CAT_UPD_PASS1UPD   "\
0315-060 The Error Identifier %1$08x is not defined in the template file.\n" 
$ %1$08x will be expanded to an error id 

CAT_UPD_PASS1DUPLW "\
0315-061 The Error Identifier %1$08x is already defined in the template file.\n"
$ %1$08x will be expanded to an error id

CAT_UPD_BADC         "\
0315-062 Bad character %1$c in %2$s in the template for %3$s.\n"  
$ errupdate/lex.c
$ %1$c will be expanded to the bad character
$ %2$s will be the string containing the bad character
$ %3$s will be expanded to a descriptive label

CAT_UPD_YY1  "\
0315-063 Supply a label after the '+' symbol on an add.\n" 
$ errupdate/parse.y

CAT_UPD_YY2  "\
0315-064 Supply an error id after the '=' symbol on an update.\n"

CAT_UPD_YY3  "\
0315-065 Supply an error id after the '-' symbol on a delete.\n"

CAT_UPD_YY4  "\
0315-066 You may only change the Log, Report, or Alert fields of an\n\
error record template on an update.\n"

CAT_UPD_YY5  "\
0315-067 The format for this line should be KEYWORD = VALUE.\n"

CAT_UPD_YY6  "\
0315-068 There is a colon missing after a label or an error id.\n"

CAT_UPD_YY7  "\
0315-069 At least one number in the list for this KEYWORD is either\n\
out of range or is not a valid hexadecimal number.\n" 

CAT_UPD_YY8  "\
0315-070 The format for the Detail_Data field is incorrect.  It should\n\
be in the form Detail_Data = LENGTH, ID, ENCODING.\n" 

CAT_UPD_YY9  "\
0315-071 The keyword '%1$s' is not a valid error record template keyword.\n"
$ %1$s will be expanded to the bad keyword

CAT_UPD_OVF   "\
0315-072 Yacc symbol table overflow"
$ sym.c

CAT_UPD_WMAG  "\
0315-073 Error reading yacc temporary file"

CAT_INS_TOTAL "\
0315-080 %1$d total errors. No modifications made to message catalog\n"
$ errinstall/main.c
$ %1$d will be expanded to a number of errors

CAT_INS_USAGEMSG "\
Usage:\n\
errmsg -c -w setlist -z filename idfile\n\
Add messages to the error logging message catalog\n\
%1$s according to the input file 'idfile'.\n\
\n\
-c            check flag. Scan file for syntax errors.\n\
-w set_list   Display messages in sets in 'set_list'\n\
              Valid codes are:\n\
              E     Error Description message set\n\
              P     Probable Cause message set\n\
              U     User Cause message set\n\
              I     Install Cause message set\n\
              F     Failure Cause message set\n\
              R     Recommended Action message set\n\
              D     Detailed Data Id message set\n\
              ALL   All\n\
              The set_list consists of codes separated by commas\n\
-z filename   Uses the error logging message catalog specified by the filename\n\
			  parameter.\n\
If no idfile is specified, the input is taken from stdin.\n\n"
$ %1$s will be expanded to the name of the message catalog.

CAT_INS_USAGE_INST  "\
Usage:\n\
errinstall -fcq -z filename idfile\n\
Install messages in the error logging message catalog\n\
%1$s according to 'idfile'.\n\
\n\
-f            force flag. Overwrite duplicate Message IDs.\n\
-c            check flag. Scan file for syntax errors.\n\
-q            quiet flag. Do not produce an undo file.\n\
-z filename   Uses the error logging message catalog specified by the filename\n\
			  parameter.\n\
If no idfile is specified, the input is taken from stdin.\n\n"
$ %1$s will be expanded to the name of the message catalog.

CAT_INS_NOSET  "\
0315-083 Must specify a specific message set.\n\
         Valid entries are:\n\
         SET E\n\
         SET P\n\
         SET U\n\
         SET I\n\
         SET F\n\
         SET R\n\
         SET D\n"
$ errinstall/pass1.c

CAT_INS_MSGTOOLONG "\
0315-085 The length, %1$d, of message %2$s is longer than the permitted\n\
value %3$d.\n" 
$ %1$d will be expanded to the message length 
$ %2$s will be expanded to a message string
$ %3$d will be expanded to the valid message length

CAT_INS_BADSET3  "\
0315-086 Bad message set identifier: %1$s\n" 
$ %1$s will be expanded to the bad set code

CAT_INS_PM  "\
0315-087 Must begin line with either '+' or '-'\n"

CAT_INS_BADFMT  "\
0315-088 Bad format for message id '%1$-.30s'"
$ %1$-30s will be expanded to the bad message id

CAT_INS_2DIGITS  "\
0315-089 Message IDs for the Detail Data Id message set must be\n\
either 2 or 4 digits long.\n" 

CAT_INS_4DIGITS  "\
0315-090 Message IDs must be 4 digits long.\n"

CAT_INS_EOL3  "\
0315-091 Text must be enclosed in quotes\n"

CAT_INS_OPEN1  "\
0315-092 Cannot write to message catalog %1$s\n\
         because there is no write permission\n"
$ pass2.c
$ %1$s will be expanded to the message catalog filename

CAT_INS_OPEN2  "\
0315-093 Cannot create the directory %1$s\n\
         for the message catalog %2$s\n"
$ %1$s will be expanded to a directory name containing the catalog
$ %2$s will be expanded to the message catalog name 

CAT_INS_EXIST "\
0315-094 The message id %1$04X is already defined in message set %2$s.\n" 
$ %1$04X will be expanded to a message id 
$ %2$s will be expanded to the name of a message set.

CAT_INS_OUTOFRANGE2 "\
0315-096 Skipping Message ID: - %1$.4X\n\
\
         For set %2$s you can only delete message ids in the range [0xE000 - 0xE7FF]\n"
$ errmsg/pass1.c
$ %1$.4X will be expanded to a message id 
$ %2$s will be expanded to a set id 

CAT_INS_NOCODEPOINTS2 "\
0315-097 The message %1$s cannot be added because all of the message\n\
identifiers in the set %2$s are in use.  Run errmsg -w %3$s to review\n\
this message set then delete any unused messages you have added to this\n\
set.  Do not delete any of the system's messages.  Then run this command\n\
again.\n" 
$ %1$s will be message text  
$ %2$s will be expanded to a set id
$ %3$s will be expanded to a set id

CAT_DEM_NO_RECORD "\
0315-099 No error records available.\n"
$ liberrlg/child.c

CAT_ERR_SESS	  "\
0315-100 The error log device driver, /dev/error, is already open.\n\
The error demon may already be active.\n"
$ errdemon/main.c

CAT_NO_TMPLT_FILE	"\
0315-101\n\
The error template repository file %1$s\n\
is either unreadable, or does not exist.\n"
$ liberrlg/udbutil.c errclear/main.c
$ %1$s will be expanded to the template file name.

CAT_LOG_BAD_MAGIC "\
0315-103 %1$s: invalid error log entry magic: %2$d at offset %3$d\n"
$ liberrlg/dyn_ent_util.c
$ %1$d will be expanded to the program name 
$ %2$d will be expanded to a the error log entry magic number 
$ %3$d will be expanded to the offset where the bad magic number is located

CAT_LOG_FILE_OPEN "\
0315-105 Cannot open logfile %1$s\n"
$ liberrlg/log.c
$ %1$s will be expanded to the error log filename

512          "YES"
513          "NO"
514          "Warning"
515          "Line"
$ $ This dummy message starts the numbering at 512.
$ Line is used as in "Line 23: syntax error", ie. line number of a program.

CAT_ERRLOGER_MSG_LONG "\
\n0315-107  The preceding message was truncated to 230 bytes\n"
$ errlogger/errlogger.c
$ Do not translate the word errlogger

CAT_CDPT_MALLOC "\
0315-108 Unable to initialize %1$s because there is not enough\n\
memory available.\n" 
$ liberrlg/codeptcat.c
$ %1$s will be expanded to a codepoint filename
$ Do not translate the words malloc or C_entry

CAT_LOGGER_USAGE "\
Usage: errlogger message\n"
$ errlogger/errlogger.c
$ Do no translate the word errlogger

CAT_LOG_CPY	"\
0315-121 Decreasing the error log file maximum size caused %1$s \n\tto be moved to %2$s.\n"
$ raslib/log.c
$ %1$s and %2$s will be expanded to filenames

CAT_ODM_LOCK "\
0315-122 Unable to lock ODM after waiting 5 seconds.\n\tRun this command again later."
$ raslib/threshold.c
$ do not translate ODM.

CAT_PUT_ATTR "\
0315-123 Unable to update the log file size in ODM object class SWservAt.\n"
$ raslib/log.c
$ do not translate ODM.

CAT_ODM_INIT "\
0315-124 Unable to initialize ODM.\n"
$ errdemon/main.c
$ do not translate ODM.

CAT_GET_ATTR_FSIZE_USE_HDR "\
0315-125 Unable to get the error log file size from the ODM object class SWservAt.\n\
Using the value %d from the header of errlog file %s for error log size.\n",
$ liberrlg/log.c
$ do not translate ODM.
$ do not translate SWservAt.

CAT_ODM_UNLOCK "\
0315-126 Unable to unlock ODM.\n"
$ raslib/threshold.c
$ do not translate ODM.

CAT_LOG_HDR_WRITE "\
0315-127 errdemon: Unable to update errlog header for error log file %1$s.\n"
$ raslib/log.c
$ %1$s is the name of the error log file

CAT_LOG_WRITE_ERR "\
0315-128  %1$d WRITE record errors occurred since last boot.\n"
$ raslib/log.c
$ %1$d will be the number of write errors

CAT_LOG_READ_ERR "\
0315-129  %1$d READ record errors occurred since last boot.\n"
$ raslib/log.c
$ %1$d will be the number of read errors

CAT_LOG_OVERFLOW "\
0315-130  %1$d OVERFLOW record errors occurred since last boot.\n"
$ raslib/log.c
$ %1$d will be the number of overflow errors

CAT_ERRPT_jANDk "\
0315-131 The flags j and k are mutually exclusive.\n"
$ errpt/main.c

CAT_LOG_INVALID "\
0315-132 The supplied error log is not valid: %1$s.\n"
$ raslib/log.c
$ %1$s will be the error log filename

CAT_CLR_USAGE "\
Usage:\n\
errclear -J err_label_list -K err_label_list -N resource_name_list\n\
         -R resource_type_list -S resource_class_list -T err_type_list\n\
         -d err_class_list -i filename -m machine_id -n node_id\n\
         -j id_list -k id_list -l seq_no_list -y filename number_of_days \n\
\n\
Delete error log entries in the specified list that are older than\n\
number_of_days specified.  Number_of_days refers to the number of twenty\n\
four hour periods from command invocation time.\n\
-J list       Select only error_labels     in 'list'.\n\
-K list       Select only error_labels not in 'list'.\n\
-N list       Select only resource_names   in 'list'.\n\
-S list       Select only resource_classes in 'list'.\n\
-R list       Select only resource_types   in 'list'.\n\
-T list       Select only error_types      in 'list'.\n\
-d list       Select only error_classes    in 'list'.\n\
-i filename   Uses the error log file specified by the filename parameter.\n\
-j list       Select only error_ids        in 'list'.\n\
-k list       Select only error_ids  not   in 'list'.\n\
-l list       Select sequence_numbers in 'list'.\n\
-m machine_id Delete entries for the machine id as output by uname -m.\n\
-n node_id    Delete entries for the node id    as output by uname -n.\n\
-y filename   Uses the error record template file specified by the filename\n\
              parameter.\n\
\n\
'list' is a list of entries separated by commas.\n\
error_type  = PERM,TEMP,PERF,PEND,UNKN,INFO\n\
error_class = H (HARDWARE), S (SOFTWARE), O (errlogger MESSAGES), U (UNDETERMINED)\n"
$ errclear/main.c

CAT_RPT_USAGE "\
Usage:   errpt -actg -s startdate -e enddate\n\
         -N resource_name_list -S resource_class_list -R resource_type_list\n\
         -T err_type_list -d err_class_list -j id_list -k id_list\n\
         -J label_list -K label_list -l seq_no_list -F flags_list \n\
		 -i filename -y filename -z filename\n\
\n\
Process error log entries from the supplied file(s).\n\
-i filename  Read error log entries from the supplied file.\n\
-y filename  Uses the error record template file specified by the filename\n\
			 parameter.\n\
-z filename  Uses the error logging message catalog specified by the filename\n\
			 parameter.\n\
\n\
Output formatted error log entries sorted chronologically.\n\
-a         Print a detailed listing. Default is a summary listing.\n\
-c         Concurrent mode. Display error log entries as they arrive.\n\
-t         Print error templates instead of error log entries.\n\
-g         Output raw ascii  error template structures.\n\
\n\
Error log entry qualifiers:\n\
-s startdate  Select entries posted later   than date. (MMddhhmmyy)\n\
-e enddate    Select entries posted earlier than date. (MMddhhmmyy)\n\
-N list       Select resource_names   in 'list'.\n\
-S list       Select resource_classes in 'list'.\n\
-R list       Select resource_types   in 'list'.\n\
-T list       Select error_types      in 'list'.\n\
-d list       Select error_classes    in 'list'.\n\
-j list       Select error_ids        in 'list'.\n\
-k list       Select error_ids  not   in 'list'.\n\
-J list       Select error_labels     in 'list'.\n\
-K list       Select error_labels not in 'list'.\n\
-l list       Select sequence_numbers in 'list'.\n\
-F list       Select templates according to the value of the\n\
	      Alert, Log, or Report field.\n\
-m machine_id Select entries for the machine id as output by uname -m.\n\
-n node_id    Select entries for the node id    as output by uname -n.\n\
\n\
'list' is a list of entries separated by commas.\n\
error_type  = PERM,TEMP,PERF,PEND,UNKN,INFO\n\
error_class = H (HARDWARE), S (SOFTWARE), O (errlogger MESSAGES), U (UNDETERMINED)\n");
$ errpt/main.c

CAT_CLR_NONNUM "\
0315-135 Non-numeric character '%1$c' in '%2$s'.\n"
$ errclear/main.c
$ %1$c will be a character in the user input string %2$s.

CAT_CLR_NO_DAYS "\
0315-136 Number of days is required, and must be zero or greater.\n"
$ errclear/main.c

CAT_UPD_RSVD_CDPT "\
0315-137 The template for %1$s was not added because the Alert field\n\
is set to TRUE and the template contains the message id %2$s which is\n\
not permitted in an Alert.\n\
Possible solutions are:\n\
1.  Change the message id to one that is permitted in Alerts.\n\
2.  Change the Alert field to FALSE.\n\
3.  Run the errupdate command with the -p option to override this\n\
    restriction.  If you do this, you must also customize NetView to\n\
    recognize this message id when it receives this Alert.\n"	
$ errupdate/pass1.c
$ %1$s is the error template label
$ %2$s is a message id

CAT_UPD_RSVD_CDPT_DETAIL "\
0315-138 For alertable template, \"%1$s\", \"%2$s\" must be\n\
in the range:\n\t%3$4.4x to %4$4.4x or equal to 8001.\n\
The supplied message id %5$4.4x is NOT in range.\n"
$ errupdate/alertable.c
$ %1$s is error template label
$ %2$s is the codepoint field name
$ %3$4.4x and %4$4.4x are the valid range numbers 
$ %5$4.4x is a message id

CAT_UPD_RSVD_CDPT_GENERIC "\
0315-169 For alertable template, \"%1$s\", \"%2$s\" must be\n\
in the range:\n\t%3$4.4x to %4$4.4x or %5$4.4x to %6$4.4x\n\
The supplied message id %7$4.4x is NOT in range.\n"
$ errupdate/alertable.c
$ %1$s error template label
$ %2$s codepoint field name
$ %3$4.4x, %4$4.4x, %5$4.4x, and %6$4.4x are valid ranges 
$ %7$4.4x is a message id

CAT_UPD_NOT_ALERTABLE_INFO "\
0315-139 The template for %1$s has the Err_Type field set to INFO\n\
and the Alert field set to TRUE.  Error Type INFO is not permitted\n\
in an Alert.\n\
Possible solutions are:\n\
1.  Change the Err_type field to one that is permitted in an\n\
    Alert (UNKN, TEMP, PERM, PEND, or PERF).\n\
2.  Change the Alert field to FALSE.\n"
$ errupdate/alertable.c
$ do not translate the string "INFO"
$ %1$s is the error template label

CAT_UPD_NOT_ALERTABLE_OP "\
0315-140 The template for %1$s has the Class field set to O and\n\
the Alert field set to TRUE.  Error Class OPERATOR is not permitted in\n\
an Alert.\n\
Possible solutions are:\n\
1.  Change the Class field to one that is permitted in an Alert\n\
    (H, S, or U).\n\
2.  Change the Alert field to FALSE.\n"
$ errupdate/alertable.c
$ do not translate the string "Operator"
$ %1$s is the error template label

CAT_LOG_BAD_SIZE "\
0315-141 %1$s: invalid error log entry length: %2$d at offset: %3$d\n"
$ liberrlg/dyn_ent_util.c
$ %1$s is program name
$ %2$d is the invalid length value
$ %3$d is the offset where the invalid value is found 

CAT_LOG_BAD_LE_MEMBER "\
0315-142 %1$s: invalid %2$s encountered in error log entry Sequence Number %3$d.\n"
$ liberrlg/dyn_ent_util.c
$ %1$s is program name
$ %2$s is a invalid string
$ %3$s is the sequence number where the invalid string is found 

CAT_ERRPT_JANDK "\
0315-143 The flags J and K are mutually exclusive.\n"
$ errpt/main.c

CAT_LOG_BAD_LENGTHS "\
0315-144 %1$s: invalid error log entry lengths: length: %2$d length2: %3$d at offset: %4$d\n"
$ liberrlg/dyn_ent_util.c
$ %1$s is program name
$ %2$d and %3$d are error log entry lengths 
$ %4$d is the offset where these are found
$ do not translate "length" and "length2".

CAT_LOG_BAD_SEQ "\
0315-145 %1$s: invalid error log entry Sequence Number: %2$d at offset: %3$d\n"
$ liberrlg/dyn_ent_util.c
$ %1$s is program name
$ %2$d is an invalid sequence number
$ %3$d is the offset where this sequence id is found 
$ do not translate "sequence_id"

CAT_LOG_BAD_TIME "\
0315-146 %1$s: invalid error log entry Date/Time: %2$d at offset: %3$d\n"
$ liberrlg/dyn_ent_util.c
$ %1$s is program name
$ %2$d is the invalid error log entry timestamp
$ %3$d is the offset where this timestamp is found 

CAT_BAD_TMPLT_ENTRY "\
0315-147 The error template for Error Id %1$08d contains an invalid %2$s.\n"
$ liberrlg/tmplt.c
$ %1$08d will be expanded to the error id 
$ %2$s is an invalid string

CAT_LE_BAD_CODE "\
0315-148 %1$s: invalid log entry bad code: %2$d\n"
$ liberrlg/dyn_ent_util.c
$ %1$s is program name
$ %2$d is the bad code in the error log entry 

CAT_LOG_BAD_MACH "\
0315-149 %1$s: invalid error log entry Machine Id at offset: %2$d\n"
$ liberrlg/dyn_ent_util.c
$ %1$s is program name
$ %2$d the offset where the invalid machineid is found 

CAT_LOG_BAD_NODE "\
0315-150 %1$s: invalid error log entry Node Id at offset: %2$d\n"
$ liberrlg/dyn_ent_util.c
$ %1$s is program name
$ %2$d is the offset where the invalid nodeid is found 

CAT_DEAD_ERRC_IO "\
0315-151 Invalid error buffer pointer(s) encountered in dump image %1$s.\n\
No error entries processed.\n"
$ errdead/main.c
$ %1$s will be the user supplied dumpfile.

CAT_DEAD_ERRC_IO_ERR_COUNT "\
0315-152 Invalid error count encountered in dump image %1$s.\n"
$ errdead/main.c
$ %1$s will be the user supplied dumpfile.

CAT_INST_CODEPOINT_W "\
0315-170 Cannot create the error message file with the specified path %1$s.\n"
$ errinstall/main.c
$ %1$s will be a Codeptfile name.
$ %2$s will be a system error message.

CAT_TMPLT_UNCOMPRESS "\
0315-155 Cannot uncompress template file %1$s.\n\
This could be because you don't have permissions, that the\n\
filesystem containing the template file is full, or a fork or\n\
exec system call failed.\n"
$ %1$s is the template filename

CAT_TMPLT_COMPRESS "\
0315-158 Cannot uncompress template file %1$s.\n\
This could be because you don't have permissions, that the\n\
filesystem containing the template file is full, or a fork or\n\
exec system call failed.\n"
$ %1$s is the template filename

CAT_PATH_GETATTR "\
0315-159 Unable to get %1$s attribute from ODM object class SWservAt.\n"
$ %1$s is the error log filename attribute 

CAT_ODM_TERM "\
0315-160 Unable to terminate ODM.\n"

CAT_INVALID_PATH "\
0315-161 The path %1$s does not exist.\n\
Using default logfile %2$s from ODM.\n"
$ %1$s is a pathname to the given error log filename
$ %2$s is a error log filename

CAT_INVALID_LOG "\
0315-162 Cannot initialize error log file %1$s.\n\
The error log file does not have the correct permissions, or is not a\n\
valid error log file.  Using default error log file %2$s from ODM.\n"
$ %1$s is an error log filename
$ %2$s is an error log filename

CAT_PATH_PUTATTR "\
0315-163 Unable to update %1$s attribute in ODM object class SWservAt.\n"
$ %1$s is the error log filename attribute

CAT_CLR_MAXDAYS "\
0315-026 The maximum number of days that can be specified is %1$d.\n\
NOTE: The errlog file has not been changed.\n"
$ %1$d is the maximum number of days that can be cleared from the
$ errlog file based on Jan. 1, 1970.

CAT_BUF_BADSIZE "\
0315-164 The value %1$d is not a valid error log memory buffer size value.\n\
This value must be a minimum of %2$d bytes.\n"
$ %1$d is the invalid error log memory buffer size 
$ %2$d is the minimum size for the error log memory buffer

CAT_BUF_LESS "\
0315-165 The value %1$d for the error log memory buffer size will not take affect\n\
until the next system reboot.\n"
$ %1$d is a error log memory buffer size

CAT_BUF_NOMEM "\
0315-111 Cannot set error log memory buffer size to %1$d, because there is\n\
not enough memory available.\n"
$ %1$d is the error log memory buffer size

CAT_BUF_GETATTR "\
0315-166 Unable to get the error log memory buffer size from the ODM object\n\
class SWservAt.\n"

CAT_BUF_PUTATTR "\
0315-167 Unable to put the error log memory buffer size into the ODM object\n\
class SWservAt.\n"

CAT_SHOW_ATTR "\
Error Log Attributes\n--------------------------------------------\n\
Log File	  \t%1$s\n\
Log Size	  \t%2$d bytes\n\
Memory Buffer Size\t%3$d bytes\n"
$ %1$s is the error log filename
$ %2$s is the error log maximum size
$ %3$d is the error log memory buffer size

CAT_UPD_POSS_CLASS "\
\t S \n \t O \n \t H \n \t U\n"
$ do not translate the above message characters S, O, H, U

CAT_UPD_POSS_TYPE "\
\t PERM \n \t TEMP \n \t PERF \n \t PEND \n \t UNKN\n \t INFO\n"
$ do not translate PERM, TEMP, PERF, PEND, UNKN, INFO

CAT_UPD_POSS_ALERT "\
\t TRUE \n \t FALSE \n"
$ do not translate TRUE or FALSE

CAT_LOG_CANT_PROCESS "\
0315-171 Unable to process the error log file %1$s.\n"
$ liberrlg/log.c

CAT_BUF_OVERFLOW "\
0315-172 The value specified for the error log memory buffer size is too large.\n\
The maximum value allowed is %1$d bytes.\n"
$ liberrlg/buffer.c
$ %1$d is the maximum error log memory buffer size 

CAT_INVALID_NUMBER "\
0516-173 The value specified %s is not valid for flag %c.\n\
         No processing was performed.\n"
$ errclear/main.c errpt/main.c
$ %c is the command line flag
$ %s is thea command line argument for that flag

CAT_MALLOC_ERROR "\
0315-174 Unable to report symptom data, since there is not enough\n\
memory at this time.\n"

CAT_DEM_BUF_RND "\
0315-175 The error log memory buffer size you supplied will be rounded up\n\
to a multiple of 4096 bytes.\n"

CAT_INVALID_S_E_TIME "\
0315-176 Invalid start or end time supplied: %s\n"
$ liberrlg/lstchk.c
$ %s is the command line argument for either -s or -e.

CAT_SW_ATTR_BAD "\
0315-177 The ODM object class SWservAT attribute %1$s contained an unacceptable value: %2$s\n"
$ liberrlg/log.c
$ do not translate ODM
$ do not translate SWservAT
$ %1$s sill contain the ODM attribute name
$ %2$s sill contain the ODM attribute value

CAT_GET_ATTR_FSIZE "\
0315-178 Unable to get the log file size from ODM object class SWservAt.\n"

CAT_INST_CODEPOINT_W2 "\
0315-179 The file %1$s is not a valid error message file.\n"
$ errmsg/main.c
$ %1$s will be replaced with name of the file.

CAT_LOG_READ_EOF "\
0315-180 logread: UNEXPECTED EOF\n"

CAT_FSIZE_USE_FILE "\
The supplied errlog header contains a file size different from ODM.\n\
Updating ODM to use the supplied header file size %d.\n"
$ liberrlg/log.c
$ do not translate ODM
$ do not translate errlog

CAT_BAD_ERR_CLASS "\
0315-181 Bad value supplied for error class: %s\n\
Rerun the command with a valid error class.\n"

CAT_BAD_ERR_TYPE "\
0315-182 Bad value supplied for error type: %s\n\
Rerun the command with a valid error type.\n"

CAT_CONVERT_USAGE "\
Usage:   convert_errlog -i Filename -o Filename\n\
\n\
-i Filename	Specifies the name of the V3 error log file.\n\
-o Filename	Specifies the name of the V4 error log file.\n\
\n\
If no option is specified, the default for input and output\n\
files is /var/adm/ras/errlog\n"

CAT_BAD_ERR_KEYWORD "\
0315-179 Bad value supplied for error template keyword:  %s\n\
Valid keyword values are:  Alert, Log, Report\n\
Rerun the command with a valid error template keyword.\n"

CAT_BAD_ERR_KEYWORD_VALUE "\
0315-180 Bad value supplied for error template keyword value:  %s\n\
Valid error template values for keywords Alert, Log, and Report\n\
are '0' and '1'.  Rerun the command with a valid error template\n\
keyword value.\n"

CAT_BAD_TMPLT_WRITE "\
0315-183 Cannot update the error template database %s.\n\
A write to the error template database failed.\n%s\n\
Possible Causes:\n\
1.  The filesystem containing the error template database\n\
    is full.\n\
Run the errupdate command with the '.undo' file(s) to clean up\n\
the error template database.  Increase the filesystem, and then\n\
rerun the errupdate command to add the templates.\n"

CAT_UPD_POSS_ENCODE "\
ALPHA\nDEC\nHEX\n"

CAT_UPD_YACC_SYNTAX "syntax error"

CAT_UPD_YACC_LINE "line"

CAT_UPD_YACC_OFLW "yacc stack overflow"

CAT_UPD_YACC_BACKUP "syntax error - cannot backup"

$ $ set 2 is for shell scripts such as /usr/lib/ras/notifymeth
$set 2
1 "A condition has been detected that requires immediate attention."
2 "If you have installed the Software Error Logging and Dump Service"
3 "A recovered error has been detected on the device"	
4 "The device may need cleaning or have defective media."
5 "If you have installed the Software Error Logging and Dump Service"
6 "Run diagnostics to determine the nature of the problem."
7 "If problem persists run diagnostics on the device."
8 "A memory error was detected and"
9 "has been logged in the system error log."
10 "A Checkstop occurred and"
11 "If Checkstop occurs repeatedly, contact your service representative."
12 "Aids package, bos.sysmgt.serv_aid, then run the command below for" 
13 "more detailed information.  Otherwise, move your system error log to"
14 "a machine that has this package installed, and run the command below."
15 "Aids package, bos.sysmgt.serv_aid, then run the command below to" 
16 "examine the error entry.  Otherwise, move your system error log to"
17 "a machine that has this package installed, and run the command below."
18 "Tape drive usage for %s has exceeded the recommended cleaning"
19 "interval without being cleaned.  Use approved cleaning materials to"
20 "clean the tape drive."
