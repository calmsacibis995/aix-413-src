/* @(#)20  1.2.1.9  src/bos/usr/sbin/install/sysck/sysmsg.h, cmdinstl, bos411, 9428A410j 6/6/94 12:10:25 */
/*
 *
 * COMPONENT_NAME: cmdinstl
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <limits.h>
#include <nl_types.h>
#include <locale.h>
#include "sysck_msg.h"

#define MF_SYSCK "sysck.cat"
/*
 * Definitions for default message strings for sysck programs
 */

#define DEF_Usage \
"Usage: sysck -i | -u [ -R RootPath ][ -Nv ][ -s savefile ]\n\
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

#define DEF_True_Or_False      "%1$s: \
3001-001 The value for %2$s must be true or false.\n"

#define DEF_No_Value           "%1$s: \
3001-002 A value may not be specified for %2$s.\n"

#define DEF_Needs_A_Value      "%1$s: \
3001-003 A value must be specified for %2$s for entry %3$s.\n"

#define DEF_Needs_An_Attribute "%1$s: \
3001-004 The attribute %2$s must be specified for\n\
\tfile %3$s.\n"

#define DEF_Duplicate_Stanza   "%1$s: \
3001-005 The entry for %2$s\n\
\tappears more than once in the database.\n"

#define DEF_Duplicate_Object   "%1$s: \
3001-006 The database entries for %2$s\n\
\tand %3$s refer to the same file.\n"

#define DEF_Linked_Directory   "%1$s: \
3001-007 The entry for the directory %2$s may not\n\
\tspecify hard links.\n"

#define DEF_Illegal_Attribute  "%1$s: \
3001-008 The attribute %2$s is not valid for the file \n\
\t%3$s.\n"

#define DEF_Illegal_Root       "%1$s: \
3001-009 The Root Path specified, %2$s is not valid.\n"

#define DEF_Invalid_Value      "%1$s: \
3001-010 The attribute %2$s has a value %3$s which is not valid.\n"

/*
 * Error messages for program execution errors
 */

#define DEF_No_Program         "%1$s: \
3001-011 The program %2$s cannot be executed.\n"

#define DEF_Program_Error      "%1$s: \
3001-012 The program %2$s encountered an error during execution.\n"

#define DEF_Verify_Failed      "%1$s: \
3001-013 The program %2$s was not successful\n\
\tverifying the file \n\
\t%3$s.\n"

#define DEF_Illegal_Entry      "%1$s: \
3001-014 The entry for %2$s is not valid.\n"

#define DEF_Illegal_Type       "%1$s: \
3001-015 The entry for %2$s has a type %3$s\n\
\twhich is not valid.\n"

#define DEF_Missing_Type       "%1$s: \
3001-016 File %2$s is missing the type attribute.\n"

#define DEF_Installp_Failed    "%1$s: \
3001-017 Errors were detected validating the files\n\
\tfor package %2$s.\n"

#define DEF_Input_File_Error   "%1$s: \
3001-018 An error was encountered reading the input file.\n"

#define DEF_Last_Stanza        "%1$s: \
3001-019 The last valid stanza read was %2$s.\n"

#define DEF_No_Last_Stanza     "%1$s: \
3001-020 No valid stanzas were read.\n"

/*
 * Error messages for file related errors
 */

#define DEF_Unknown_Type       "%1$s: \
3001-021 The file %2$s \n\
\tis of an unknown type.\n"

#define DEF_No_Such_File       "%1$s: \
3001-022 The file %2$s \n\
\twas not found.\n"

#define DEF_Donot_Know_How     "%1$s: \
3001-023 The file %2$s \n\
does not exist and the entry has no source attribute.\n"

#define DEF_Wrong_File_Type    "%1$s: \
3001-024 The file %2$s \n\
\tis the wrong file type.\n"

#define DEF_Wrong_Checksum     "%1$s: \
3001-025 The file %2$s \n\
\thas the wrong checksum value.\n"

#define DEF_Wrong_Size         "%1$s: \
3001-026 The file %2$s \n\
\thas the wrong size.\n"

#define DEF_No_Such_Source     "%1$s: \
3001-027 The file %2$s \n\
\thas a source file\n\
\t%3$s that does not exist.\n"

#define DEF_Absolute_File       "%1$s: \
3001-028 The file %2$s \n\
\tmust be an absolute path name.\n"

#define DEF_Absolute_Link       "%1$s: \
3001-029 The link %2$s \n\
\tmust be an absolute path name.\n"

#define DEF_Absolute_Program    "%1$s: \
3001-030 The program %2$s \n\
\tmust be an absolute path name.\n"

#define DEF_Absolute_Source     "%1$s: \
3001-031 The file %2$s \n\
\tmust have an absolute path name\n\
\tfor the source attribute %3$s.\n"

#define DEF_Copy_Failed         "%1$s: \
3001-032 The copy from file %2$s\n\
\tto %3$s was not successful.\n"

#define DEF_Create_Failed       "%1$s: \
3001-033 The creation of file %2$s \n\
\twas not successful.\n"

#define DEF_Chmod_Failed        "%1$s: \
3001-034 Setting permissions on the file %2$s\n\
\twas not successful.\n"

#define DEF_Install_Failed      "%1$s: \
3001-035 Errors were detected validating file \n\
\t%2$s.\n"

#define DEF_VPD_Two_Owners      "%1$s: \
3001-036 WARNING:  File %2$s\n\
\tis also owned by fileset %3$s.\n"

/*
 * Other Errors
 */

#define DEF_Unknown_User        "%1$s: \
3001-037 The name %2$s is not a known user for file \n\
\t%3$s.\n"

#define DEF_Unknown_Group       "%1$s: \
3001-038 The name %2$s is not a known group for entry \n\
\t%3$s.\n"

#define DEF_Unknown_Mode        "%1$s: \
3001-039 The file %2$s \n\
\thas a file mode or flag %3$s which is not valid.\n"

#define DEF_Out_Of_Memory       "%1$s: \
3001-040 A request for additional memory failed.\n"

#define DEF_No_Install_File     "%1$s: \
3001-041 Cannot open the installation file \n\
\t%2$s.\n"

#define DEF_VPD_Open_Failed     "%1$s: \
3001-042 Cannot open the vital product database.\n"

#define DEF_VPD_Add_Failed      "%1$s: \
3001-043 Cannot add the vital product data for \n\
\t%2$s.\n"

#define DEF_Can_Not_Open        "%1$s: \
3001-044 Cannot open inventory save file %2$s.\n"

#define DEF_No_LPP_for_Inv_Rec  "%1$s: \
3001-045 WARNING:  A file which is being installed already has an\n\
\tentry in the inventory database but is not owned by any installed\n\
\tfileset.  The file is:\n\
\t%2$s\n\
\tDeleting the existing inventory entry and continuing.\n\n"

#define DEF_VPD_Open_Failed2     "%1$s: \
3001-046 Cannot open the %2$s vital product database.\n"

/*
 * Message macro
 */

#define MSG_S(a,b,c,d,e,f) \
fprintf(stderr,catgets(catd,MSGS_SYSCK,a,b),sysck_pn,c,d,e,f)

nl_catd  catd;    /* Message Catalog descriptor */
char *sysck_pn;   /* Sysck program name */

