/* @(#)45  1.23.2.35  src/bos/usr/sbin/install/messages/commonmsg.h, cmdinstl, bos41B, 412_41B_sync 1/10/95 18:30:37 */

/*
*
* COMPONENT_NAME: (CMDINSTL) High Level Install Command
*
* FUNCTIONS: Message catalog for CMDINSTL
*
* ORIGINS: 27
*
* (C) COPYRIGHT International Business Machines Corp. 1991,1994
* All Rights Reserved
* Licensed Materials - Property of IBM
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

#ifndef _H_COMMONMSG
#define _H_COMMONMSG

/*------------------------------------------------*
** These are the messages for the bffcreate command.
**------------------------------------------------*/

#define BFF_NOLPPS_D "\
bffcreate: No package names were specified on the command line.\n"

#define BFF_FAILED_CREATE_D "\
bffcreate: Failed creating the backup format file for package\n\
\t%s.\n"

#define BFF_USAGE_D "\
Usage: bffcreate [-qvX] [-d device] [-t savedir] [-w workdir]\n\
                 {-l | package1 [level1][, package2 [level2], ...] | all}\n\
       where\n\
           -d  <device>   input (d)evice containing source media\n\
                                 default is /dev/rfd0\n\
           -l             (l)ist media contents\n\
           -q             (q)uiet mode, don't needlessly prompt for device\n\
           -t  <savedir>  (t)arget directory in which images will be created\n\
                                 default is /usr/sys/inst.images\n\
           -v             (v)erbose mode, echoes image name\n\
           -w  <workdir>  (w)ork directory used for temporary work space\n\
                                 default is /tmp\n\
           -X             autoe(X)pand\n\n\
           package        is the package name\n\
           level          is of the form vv.rr.mmmm.ffff.ppppppppp\n\
                                        (version.release.mod.fix.ptf_id)\n"

#define BFF_NO_SIZE_IN_TOC_D "\
bffcreate:  Warning: important size information is missing from the table\n\
\tof contents file.  Consequently, there may not be enough free\n\
\tfile system space to successfully create the backup format file\n\
\timages.  Continuing anyway ...\n\n"

#define BFF_NO_LPPS_W_L_FLAG_D  "\
bffcreate:  A package list is not permitted with the -l flag.\n"

#define BFF_INVALID_LEVEL_D  "\
bffcreate:  An invalid level was given for package\n\
\t%s: %s\n\
\tALL levels must be in one of the following formats: \n\
\t    vv.rr.mmmm.ffff   OR   vv.rr.mmmm.fffff.ppppppppp\n"

#define BFF_PKG_NOT_FOUND_D  "\
bffcreate:  The specified software package %s\n\
\twas not found on device %s\n"

#define BFF_PKG_W_LEVEL_NOT_FOUND_D "\
bffcreate:  The specified software package %s %s\n\
\twas not found on device %s\n"

#define BFF_PKG_LIST_HDR_D "\
   Package Name                   Level                     Content\n"

#define BFF_NO_ALL_W_PKG_LIST_D  "\
bffcreate:  The \"all\" keyword cannot be given with a package list.\n"


/*--------------------------------------------*
** These are the messages for the inucp command.
**--------------------------------------------*/

#define INUCP_WRG_FMT_D "\
inucp: Warning. inucp expects the listfile to contain relative path names \n\
\tto the file. inucp will attempt to complete anyway.\n"

#define INUCP_USAGE_D "\
Usage: inucp -s start_dir [-e final_dir] listfile package\n\
\twhere \n\
\t\tstart_dir is the directory from which files \n\
\t\t\twill be copied.\n\
\t\tfinal_dir is the directory to which files \n\
\t\t\twill be copied.\n\
\t\tlistfile is a file containing relative paths of files\n\
\t\t\tto be copied. \n\
\t\tpackage is the name of the software package to be copied.\n"


/*-----------------------------------------------*
** These are the messages for the inurest command.
**-----------------------------------------------*/

#define RST_USAGE_D "\
Usage: inurest [-d device] [-q] listfile package \n\
\twhere \n\
\t\tdevice is the input device.\n\
\t\tlistfile is a file containing relative paths of files to\n\
\t\t\tbe restored.\n\
\t\tpackage is the name of the software package to be restored.\n"

#define RST_INFORM_WAIT_D "\
Restoring files, please wait.\n"

#define RST_PROGRESS_1_D "\
%d files out of %d restored\n"

#define RST_PROGRESS_2_D "\
%d files restored.\n"

#define RST_FAILURE_D "\
inurest: Error in restoring files\n"

#define RST_FAILURE_TAPE_D "\
The restore failure may be due to a tape error or SCSI bus reset.\n\
Please check your physical SCSI configuration, the tape, the tape drive,\n\
and try again.\n"


/*----------------------------------------------*
** These are the messages for the inusave command.
**----------------------------------------------*/

#define SAV_DELVPD_D "\
inusave: Warning. Unable to delete an entry from the Software \n\
\tVital Product Data database.\n"

#define SAV_VPDNF_D "\
inusave: Software Vital Product Data information was not\n\
\tfound for apply list entry %s.\n"

#define SAV_USAGE_D "\
Usage: inusave listfile package \n\
\twhere \n\
\t\tlistfile is a file containing relative paths of files to \n\
\t\t\tbe saved.\n\
\t\tpackage is the name of the software package to be saved.\n"


/*----------------------------------------------*
** These are the messages for the inurecv command.
**----------------------------------------------*/

#define RCV_USAGE_D "\
Usage: inurecv package [filesets_file]\n\
\twhere \n\
\t\tpackage is name of the software package to be recovered.\n\
\t\tfilesets_file is the full path name of a file that specifies\n\
\t\t\twhich filesets within the package to recover.\n"


/*-----------------------------------------------------*
** These are the messages that are common among routines.
**-----------------------------------------------------*/

#define CMN_MALLOC_FAILED_D "\
%s: Received an error while trying to allocate space \n\
\tusing malloc. Use local problem reporting procedures.\n"

#define CMN_CANT_OPEN_D "\
%s: Cannot access or open file %s. \n\
\tCheck path name and permissions.\n"

#define CMN_NO_MK_TMPDIR_D "\
%s: Cannot create a directory in the /tmp directory.\n\
\tCheck permissions or available space.\n"

#define CMN_NO_MK_DIR_D "\
%s: Cannot create the directory %s.\n\
\tCheck path name and permissions.\n"

#define CMN_REMOTE_FS_D "\
%s:  There is not enough free disk space in a filesystem.\n\
\tThe remote filesystem %s:%s,\n\
\tmounted over %s, \n\
\tneeds an additional %d 512-byte blocks.\n\
\tThis filesystem cannot be extended automatically by the installation\n\
\tprogram.  Make more space available then retry the installation.\n\n"

#define CMN_READONLY_FS_D "\
%s: The file system %s is mounted read only.\n"

#define CMN_CANT_CHDIR_D "\
%s: Cannot change to directory %s.\n\
\tCheck path name and permissions.\n"

#define CMN_SET_ENV_VAR_D "\
%s: Environment variable %s could not be set.\n"

#define CMN_CPFAIL_D "\
%s: The copy of file %s\n\
\tto %s failed.\n"

#define CMN_ARFAIL_D "\
%s: The archive of a constituent member into \n\
\t file %s failed.\n"

#define CMN_BAD_FL_FORMAT_D "\
%s: File %s is not in the correct format.\n"

#define CMN_NOSORT_D "\
%s: The %s file could not be sorted.\n"

#define CMN_RENAME_D "\
%s: Warning. File %s\n\
\tcould not be renamed to %s.\n"

#define CMN_TOO_FW_D "\
%s: Not enough parameters were passed on the \n\
\tcommand line.\n"

#define CMN_TOO_MN_D "\
%s: Too many parameters were passed on the \n\
\tcommand line.\n"

#define CMN_BADMV_D "\
%s: File %s\n\
\tcould not be moved to %s.\n"

#define CMN_SYSCOM_D "\
%s: Failure on system call to execute command %s.\n"

#define CMN_CANT_CREATE_D "\
%s: Cannot create file %s.\n\
\tCheck path name and permissions.\n"

#define CMN_NOSPACE_D "\
%s:  There is not enough free disk space in filesystem\n\
\t%s (%d more 512-byte blocks are required).\n\
\tMake more disk space available or use the option to automatically\n\
\textend filesystems (-X flag).\n\n"

#define CMN_CANT_EXPAND_D "\
%s:  There is not enough free disk space in filesystem\n\
\t%s (%d more 512-byte blocks are required).\n\
\tAn attempt to extend this filesystem was unsuccessful.\n\
\tMake more space available then retry this operation.\n\n"

#define CMN_INTERNAL_D "\
%s: An internal error has occurred. \n\
\tUse local problem reporting procedures.\n"

#define CMN_BAD_TOC_D "\
%s: The format of the toc file is invalid.\n"

#define CMN_ODMDIR_D "\
%s: Warning. The directory for the Object Data Manager object \n\
\trepository was not specified.\n"

#define CMN_NOSVF_D "\
%s: File %s, specified in the list of saved files,\n\
\tdoes not exist and could not be recovered.\n"

#define CMN_ADDVPD_D "\
%s: Unable to add an entry to the Software Vital \n\
\tProduct Data database.\n"

#define CMN_BAD_DEV_D "\
%s: The specified device %s\n\
\tis not a valid device or file.\n"

#define CMN_LEVEL_NUM_D "\
%s: %s is an invalid level. \n\
\tALL levels must be in one of the following formats: \n\
\t\tvv.rr.mmmm.ffff OR vv.rr.mmmm.fffff.ppppppppp\n"

#define CMN_NO_SPACE_D "\
%s: There is not enough space on the system to continue.\n"

#define CMN_NOT_ENOUGH_D "\
%s: Warning. Continuing even though there may not be enough \n\
\tspace to complete the operation.\n"

#define CMN_INSRT_VOL_D "\
%s: Please mount volume %d on %s. \n\
\tPress the enter key to continue.\n"

#define CMN_VPDERR_D "\
%s: A system error occurred while attempting \n\
\tto access the Software Vital Product Data.\n\
\tUse local problem reporting procedures.\n"

#define CMN_IVPDERR_D "\
%s: An internal error occurred while attempting \n\
\tto access the Software Vital Product Data.\n\
\tUse local problem reporting procedures.\n"

#define CMN_REALLOC_FAILED_D "\
%s: Received an error while trying to reallocate space.\n\
\tUse local problem reporting procedures.\n"

#define CMN_CREATE_TOC_D "\
%s: An error occurred while attempting to build the \n\
\ttable of contents file in the %s directory.\n"

#define CMN_READ_ERROR_D "\
%s: An error occurred while attempting to read from \n\
\tthe device %s.\n"

#define CMN_NOT_DIR_D "\
%s: The %s option requires a directory \n\
\t%s is a file. \n"

#define CMN_PGSPACE_MEG_D "\
%s: The page space could not be increased by %d megabytes.\n"

#define CMN_BD_CNTFLD_D "\
%s: Incorrect content field of \'%c\' in the file lpp_name for \n\
\tsoftware package \'%s\'. Assuming \'%c\' and continuing.\n"

#define CMN_INV_DSKT_D "\
%s: Please mount volume %d and restart the command.\n"

#define CMN_MISS_PROD_NAME_D "\
%s: The package name field required in a 4.1-formatted package\n\
\tis missing from the lpp_name file.\n"

#define CMN_INVALID_OPT_NAME_D "\
%s: The fileset name %s does not\n\
\tbegin with its package name of %s.\n"

#define CMN_EMPTY_PREREQ_FILE_D "\
%s: The prereq file is empty for the fileset\n\
\tmaintenance level %s.\n"

#define CMN_DEV_INFORM_WAIT_D "\
%s: Obtaining device information, please wait.\n"

#define CMN_DEV_INFORM_CORRECTION_D "\
%s: Using device %s instead of %s\n"

#define CMN_MIXT_LEVELS_D "\
%s: Installable filesets %s and\n \
\t%s have different levels.\n \
\tAll filesets within the same package must have identical\n \
\tlevels (version, release, modification, and fix fields).  (For\n\
\t3.2-formatted update packages, the PTF ID must also be identical.)\n"

#define CMN_MSG_SIZE_FILE_D "\
%s: The package containing installable fileset\n\
\t%s does not have a size file for the fileset.\n \
\tThe size file is required for 4.1-formatted packages.\n"

#define CMN_ERR_SYSCALL_D "\
%s: %s system call failed\n"

#define CMN_ERR_FUNCCALL_D "\
%s: %s function call failed\n"

#define CMN_PLEASE_WAIT_D "\
Please wait...\n"

#define CMN_VERIF_SELS_D "\
Verifying selections..."

#define CMN_DONE_D "\
done\n"

#define CMN_NO_EXPAND_ALLOWED_D "\
%s:  There is not enough free disk space in filesystem\n\
\t%s (%d more 512-byte blocks are required).\n\
\tThis filesystem cannot be extended automatically by the installation\n\
\tprogram.  Make more space available then retry the installation.\n\n"

/*---------------------------------------------*
** These are the messages for the inutoc command.
**---------------------------------------------*/

#define INUTOC_BADIR_D "\
inutoc: %s either does not exist or is not a directory.\n"

#define INUTOC_SYSTIME_D "\
inutoc: Warning. Could not read the system time. \n\
\tThe toc header will contain 000000000000 in the timestamp \n\
\tlocation.\n"

#define INUTOC_USAGE_D "\
Usage: inutoc [dirname]\n\
\twhere dirname is the name of the directory for which a \n\
\t./.toc file will be created.\n"

#define INUTOC_UNKNOWN_TYPE_D "\
%s: Warning. The installation package image in %s\n\
\thas an unrecognized package type of \"%s\". This may indicate a\n\
\tbad image or a new package type requiring a newer version of the\n\
\tinstallation commands.\n\
\tThe installation image was added to the .toc file anyway.\n"


/*----------------------------------------------*
** These are the messages for the inurdsd command.
**----------------------------------------------*/

#define RSD_USAGE_D "\
Usage: inurdsd [-d dev] [-v v_n] [-s v_i] [-b blkn] [-n cnt] [-t n]\n\
\twhere \n\
\t\tdev is the name of the input device (default is /dev/rfd0). \n\
\t\tv_n is the volume of where to start reading (default is 1). \n\
\t\tv_i is the volume ID string mmddhhMMssyy (default is none).\n\
\t\tblkn is the first block number to read (default is 1).\n\
\t\tcnt is the number of characters to read (default is 9999999).\n\
\t\tn is the trace code digit, from 0 to 9 (default is 0).\n"

/*
 *  These are the messages for the migration commands.
 */

#define ADEL_EXISTS_D "\
%s: %s already exists\n"

#define ADEL_ODMDIR_D "\
%s: ODMDIR not set\n"

#define ADEL_ODMDIR_USR_SHARE_D "\
%s: ODMDIR should be set to USR or SHARE instead of %s\n"

#define ADEL_USAGE_D "\
\tUsage: %s product.option v.r.m.f\n"

#define ADEL_CREATE_REC_D "\
%s: Can not create product record\n"

#define ADEL_NOT_COMPAT_ENTRY_D "\
%s: %s is not a compatability entry, or is missing\n"

#define CONVERT_USAGE_D "\
\tUsage: %s <32_vpd_path> <41_vpd_path>\n"

#define CONVERT_ADDVPD_D "\
%s: Could not merge %s database\n"
   
#define CONVERT_MIGRATE_USAGE_D "\
\tUsage: %s <32_vpd_path> \n"

/*----------------------------------------------------------*
 * These are the messages for the get_license_info command.
 *----------------------------------------------------------*/

#define GLI_USAGE_D "\
 Usage: get_license_info [-l][-f file]\n\
\tThis lists iFOR/LS-enabled products which have been registered\n\
\twith iFOR/LS server(s) on a network.\n\n\
 -f logs status of execution to a file (for communication with caller).\n\
 -l lists vendor names, vendor id's, product name, product id's and\n\
    product versions in a colon separated format.\n\
 default:\n\
    lists vendor id's, product id's and product version in a colon\n\
    separated format.\n"

#define GLI_NO_STATUS_FILE_D "\
get_license_info: No name for the status file was entered.\n"

#define GLI_INVALID_ARGUMENT_D "\
get_license_info: Invalid argument.\n"

#define GLI_LS_TV_ERROR_D "\
get_license_info: ls_tv did not return successfully.\n"

#define GLI_NO_SERVER_NAMES_FOUND_D "\
get_license_info: Could not get any server names from ls_tv.\n"

#define GLI_LS_ADMIN_FAILED_D "\
get_license_info: ls_admin failed as no vendor names were obtained.\n"

#define GLI_NO_VENDORS_FOUND_D "\
get_license_info: Could not get any vendors from the server.\n"

/*----------------------------------------------------------*
 * These are the messages for the instfix command.
 *----------------------------------------------------------*/

#define ISF_VPD_ERROR_D "\
%s: Unknown VPD error occurred while processing the fix database.\n"

#define ISF_NO_KEYWORD_DATA_D "\
%s: There are no filesets on the media for %s.\n"

#define ISF_NO_KEYWORD_LIST_D "\
%s: There are no filesets on the media for the requested Fix IDs.\n"

#define ISF_PROC_QUERY_D "\
\n\
%s: Searching for installed filesets associated with %s.\n\
\n"

#define ISF_FILESET_NOT_FOUND_D "\
Fileset %s is not applied on the system.\n"

#define ISF_FILESET_FOUND_D"\
Fileset %s is applied on the system.\n"

#define ISF_ALL_FILESETS_FOUND_D "\
All filesets for %s were found.\n"

#define ISF_NOT_ALL_FILESETS_FOUND_D "\
Not all filesets for %s were found.\n"

#define ISF_NO_RELEVANT_FILESETS_D "\
No filesets which have fixes for %s are currently installed.\n"

#define ISF_NO_FIX_DATA_D "\
There was no data for %s in the fix database.\n"

#define ISF_HELP_D_OLD "\
Usage: instfix [-T] [-s string] [ -k keyword | -f file ] [-p]\n\
          [-i[a]] [-d device] \n\
\n\
Function: Installs filesets associated with keywords or fixes.\n\
\n\
\t-T Display entire Table of Contents from the media.\n\
\t-s Search for and Display Table of Contents entries containing a string.\n\
\t-k Install filesets for a keyword or fix.\n\
\t-f Install filesets for multiple keywords or fixes using an input file.\n\
\t\tNote that the output of the -T option produces a suitable input\n\
\t\tfile format. -f- will result in instfix using standard input.\n\
\t-i Use with -k or -f option to display whether fixes or \n\
\t   keywords are installed.  This option is for information only.\n\
\t   Installation is not attempted when this option is used.\n\
\t-a Use only with -i to optionally display the symptom text associated \n\
\t   with a fix.\n\
\t-d Specify the input device (required for all but -i).\n\
\t-p Used with -k or -f to Print (Preview) filesets associated with \n\
\t   keywords.  This options is for information only.  Installation is \n\
\t   not attempted when this options is used.\n\
\n\
Examples:\n\
\n\
instfix -k IX38794 -d /dev/rmt0.1\n\
\twill install all filesets associated with fix IX38794 from the\n\
\ttape mounted on /dev/rmt0.1\n\
\n\
instfix -s SCSI -d /dev/rmt0.1\n\
\twill list all keyword entries on the tape containing the string \"SCSI\".\n\
\n\
instfix -i -k \"IX38794 IX48523\"\n\
\twill inform the user on whether fixes IX38794 and IX48523 are installed.\n\
\n\
\tNote: Available fixes are stored in the ODM fix database.\n"

#define ISF_ABSTRACT_D "\
%s Abstract: %s \n\n"

#define ISF_SYMPTOM_D "\
Symptom Text: \n%s \n\n"

#define ISF_HELP_D "\
Usage: instfix [-T] [-s string] [ -k keyword | -f file ] [-p] [-d device]\n\
          [-i [-c] [-q] [-t type] [-v] [-F]] [-a] \n\
\n\
Function: Installs or queries filesets associated with keywords or fixes.\n\
\n\
\t-T Display fix information for complete fixes present on the media.\n\
\t-s Search for and display fixes on media containing a specified string.\n\
\t-k Install filesets for a keyword or fix.\n\
\t-f Input file containing keywords or fixes. Use '-' for standard input.\n\
\t\t The -T option produces a suitable input file format for -f.\n\
\t-p Use with -k or -f to print filesets associated with keywords.\n\
\t   Installation is not attempted when -p is used.\n\
\t-d Input device (required for all but -i and -a).\n\n\
\t-i Use with -k or -f option to display whether specified fixes or \n\
\t   keywords are installed.  Installation is not attempted.\n\
\t   If neither -k nor -i is specified, all known fixes are displayed.\n\
\t-c Colon-separated output for use with -i. Output includes keyword\n\
\t   name, fileset name, required level, installed level, status, and\n\
\t   abstract.  Status values are < (down level), = (correct level),\n\
\t   + (superseded), and ! (not installed).\n\
\t-q Quiet option for use with -i.  If -c is specified, no heading is \n\
\t   displayed.  Otherwise, no output is displayed.\n\
\t-t Use with -i option to limit search to a given type.  Currently\n\
\t   valid types are 'f' (fix) and 'p' (preventive maintenance).\n\
\t-v Verbose option for use with -i.  Gives information about each\n\
\t   fileset associated with a fix or keyword.\n\
\t-F Returns failure unless all filesets associated with the fix\n\
\t   are installed.\n\
\t-a Display the symptom text (can be combined with -i, -k, or -f).\n"

#define ISF_C_OUTPUT_HDR_D "\
#Keyword:Fileset:ReqLevel:InstLevel:Status:Abstract\n"

/**********************/

#define USR2_STR_D    "usr"
#define BOTH_STR_D    "usr,root"
#define SHARE2_STR_D  "share"

#endif /* _H_COMMONMSG */



