$ @(#)10        1.12.1.1  src/bos/usr/sbin/install/sysck/sysck.msg, cmdinstl, bos411, 9436C411a 9/7/94 10:01:08
$ COMPONENT_NAME: (CMDSADM) sysck - system configuration checker
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$quote "
$set MSGS_SYSCK

MSG_Usage "\
Usage: sysck -i | -u [ -R RootPath ][ -Nv ][ -s savefile ]\n\
              [ -O {[r][s][u]} ] -f <filename> <program name>\n\
\t-i Check installation and update SWVPD\n\
\t-u Delete file entries from SWVPD and remove soft and hard links\n\
\t-R Specify alternate root (default /)\n\
\t-N Specify that SWVPD not be changed\n\
\t-v Verify checksum\n\
\t-s Specify savefile be created, usually used with -u\n\
\t-O Data base(s) to be processed (default is /usr/lib/objrepos)\n\
\t   r = /etc/objrepos, s = /usr/share/lib/objrepos\n\
\t   u = /usr/lib/objrepos\n\
\t-f Stanza file\n\
\tprogram name Specifies installable software product or fileset\n"

MSG_True_Or_False      "%1$s: \
3001-001 The value for %2$s must be true or false.\n"

MSG_No_Value           "%1$s: \
3001-002 A value may not be specified for %2$s.\n"

MSG_Needs_A_Value      "%1$s: \
3001-003 A value must be specified for %2$s for entry %3$s.\n"

MSG_Needs_An_Attribute "%1$s: \
3001-004 The attribute %2$s must be specified for\n\
\tfile %3$s.\n"

MSG_Duplicate_Stanza   "%1$s: \
3001-005 The entry for %2$s\n\
\tappears more than once in the database.\n"

MSG_Duplicate_Object   "%1$s: \
3001-006 The database entries for %2$s\n\
\tand %3$s refer to the same file.\n"

MSG_Linked_Directory   "%1$s: \
3001-007 The entry for the directory %2$s may not\n\
\tspecify hard links.\n"

MSG_Illegal_Attribute  "%1$s: \
3001-008 The attribute %2$s is not valid for the file \n\
\t%3$s.\n"

MSG_Illegal_Root       "%1$s: \
3001-009 The Root Path specified, %2$s is not valid.\n"

MSG_Invalid_Value      "%1$s: \
3001-010 The attribute %2$s has a value %3$s which is not valid.\n"

$
$ Error messages for program execution errors
$

MSG_No_Program         "%1$s: \
3001-011 The program %2$s cannot be executed.\n"

MSG_Program_Error      "%1$s: \
3001-012 The program %2$s encountered an error during execution.\n"

MSG_Verify_Failed      "%1$s: \
3001-013 The program %2$s was not successful\n\
\tverifying the file %3$s.\n"

MSG_Illegal_Entry      "%1$s: \
3001-014 The entry for %2$s is not valid.\n"

MSG_Illegal_Type       "%1$s: \
3001-015 The entry for %2$s has a type %3$s\n\
\twhich is not valid.\n"

MSG_Missing_Type       "%1$s: \
3001-016 File %2$s is missing the type attribute.\n"

MSG_Installp_Failed    "%1$s: \
3001-017 Errors were detected validating the files\n\
\tfor package %2$s.\n"

MSG_Input_File_Error   "%1$s: \
3001-018 An error was encountered reading the input file.\n"

MSG_Last_Stanza        "%1$s: \
3001-019 The last valid stanza read was %2$s.\n"

MSG_No_Last_Stanza     "%1$s: \
3001-020 No valid stanzas were read.\n"

$
$ Error messages for file related errors
$

MSG_Unknown_Type       "%1$s: \
3001-021 The file %2$s \n\
\tis of an unknown type.\n"

MSG_No_Such_File       "%1$s: \
3001-022 The file %2$s \n\
\twas not found.\n"

MSG_Donot_Know_How     "%1$s: \
3001-023 The file %2$s \n\
\tdoes not exist and the entry has no source attribute.\n"

MSG_Wrong_File_Type    "%1$s: \
3001-024 The file %2$s \n\
\tis the wrong file type.\n"

MSG_Wrong_Checksum     "%1$s: \
3001-025 The file %2$s \n\
\thas the wrong checksum value.\n"

MSG_Wrong_Size         "%1$s: \
3001-026 The file %2$s \n\
has the wrong size.\n"

MSG_No_Such_Source     "%1$s: \
3001-027 The file %2$s \n\
\thas a source file %3$s that does not exist.\n"

MSG_Absolute_File       "%1$s: \
3001-028 The file %2$s \n\
\tmust be an absolute path name.\n"

MSG_Absolute_Link       "%1$s: \
3001-029 The link %2$s \n\
\tmust be an absolute path name.\n"

MSG_Absolute_Program    "%1$s: \
3001-030 The program %2$s must be an absolute path name.\n"

MSG_Absolute_Source     "%1$s: \
3001-031 The file %2$s \n\
\tmust have an absolute path name\n\
\tfor the source attribute %3$s.\n"

MSG_Copy_Failed         "%1$s: \
3001-032 The copy from file %2$s\n\
\tto %3$s was not successful.\n"

MSG_Create_Failed       "%1$s: \
3001-033 The creation of file %2$s was not successful.\n"

MSG_Chmod_Failed        "%1$s: \
3001-034 Setting permissions on the file \n\
\t%2$s was not successful.\n"

MSG_Install_Failed      "%1$s: \
3001-035 Errors were detected validating file %2$s.\n"

MSG_VPD_Two_Owners	"%1$s: \
3001-036 WARNING:  File %2$s\n\
\tis also owned by fileset %3$s.\n"

$
$ Other Errors
$

MSG_Unknown_User        "%1$s: \
3001-037 The name %2$s is not a known user for file %3$s.\n"

MSG_Unknown_Group       "%1$s: \
3001-038 The name %2$s is not a known group for entry %3$s.\n"

MSG_Unknown_Mode        "%1$s: \
3001-039 The file %2$s \n\
\thas a file mode or flag %3$s which is not valid.\n"

MSG_Out_Of_Memory       "%1$s: \
3001-040 A request for additional memory failed.\n"

MSG_No_Install_File     "%1$s: \
3001-041 Cannot open the installation file %2$s.\n"

MSG_VPD_Open_Failed     "%1$s: \
3001-042 Cannot open the vital product database for %2$s.\n"

MSG_VPD_Add_Failed      "%1$s: \
3001-043 Cannot add the vital product data for %2$s.\n"

MSG_Can_Not_Open        "%1$s: \
3001-044 Cannot open inventory save file %2$s.\n"

MSG_No_LPP_for_Inv_Rec  "%1$s: \
3001-045 WARNING:  A file which is being installed already has an\n\
\tentry in the inventory database but is not owned by any installed\n\
\tfileset.  The file is:\n\
\t%2$s\n\
\tDeleting the existing inventory entry and continuing.\n\n"

MSG_VPD_Open_Failed2     "%1$s: \
3001-046 Cannot open the %2$s vital product database.\n"
$ The second argument is the name of a vital product database file 
$ (either "inventory" or "lpp")
