/* @(#)46  1.41.1.63  src/bos/usr/sbin/install/messages/installpmsg.h, cmdinstl, bos41J, 9510A_all 2/27/95 12:14:55 */

#ifndef _H_INSTALLPMSG
#define _H_INSTALLPMSG
/*
 * COMPONENT_NAME: (CMDINSTL) command install
 *
 * FUNCTIONS:  This is the header for default installp messages for CMDINSTL
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "commonmsg.h"

/*-----------------------------------------------------------------------
** INUCONVERT
** The following messages are used by the 3.1-to-3.2 conversion routines.
**-----------------------------------------------------------------------*/

#define ICV_CONV_D "\
%s:  An error occurred while running the %s \n\
\tconversion shell script.\n\
\tThe return code is %d.\n"

#define ICV_GETWD_D "\
%s:  An error occurred while running the getwd function.\n\
\tUse local problem reporting procedures.\n"

#define ICV_MISS_FILE_D "\
%s:  One or more root inventory files exist for the fileset\n\
\t%s,\n\
\tbut the file %s is missing.\n"

#define ICV_MKDIR_D "\
%s:  Cannot make the directory %s \n\
\tspecified in the inventory file %s.\n\
\tCheck path name and permissions.\n"

#define ICV_MRKBRK_READ_D "\
%s:  An error occurred while trying to mark the fileset\n\
\t%s broken.\n\
\tThe Software Vital Product Data database could not be read. \n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

#define ICV_MRKBRK_UPDT_D "\
%s:  An error occurred while trying to mark the fileset\n\
\t%s broken.\n\
\tThe Software Vital Product Data database could not be updated. \n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

#define ICV_MV_FAIL_D "\
%s:  An error occurred while processing the \n\
\tinventory file.\n\
\tThe file move of %s to %s failed.\n\
\tThe return code is %d.\n\
\tThe inventory file is %s.\n\
\tCheck path name and permissions.\n"

#define ICV_MV_NOEXP_D "\
%s:  An error occurred while processing the \n\
\tinventory file.\n\
\tAttempted to move %s to %s\n\
\tbut the destination filesystem was not large enough \n\
\tand could not be expanded.\n\
\tThe return code is %d.\n\
\tThe inventory file is %s.\n"

#define ICV_NODEL_SWVPD_D "\
%s:  Cannot delete %s\n\
\tfrom the Software Vital Product Data database.\n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

#define ICV_NOFILE_INVEN_D "\
%s:  An error occurred while processing the \n\
\tinventory file.\n\
\tNeither the newfile %s\n\
\tnor the oldpath %s exists.\n\
\tThe inventory file is %s.\n"

#define ICV_NOINFO_SWVPD_D "\
%s:  Cannot get information about %s\n\
\tfrom the Software Vital Product Data database.\n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

#define ICV_OLDPATH_D "\
%s:  An error occurred while reading the oldpath attribute.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

#define ICV_OLDP_DIR_D "\
%s:  An error occurred while processing the \n\
\tinventory file.\n\
\tThe stanza %s incorrectly specifies that\n\
\toldpath %s is a directory.\n\
\tThe inventory file is %s.\n"

#define ICV_OLDP_FIL_D "\
%s:  An error occurred while processing the \n\
\tinventory file.\n\
\tThe stanza %s incorrectly specifies that\n\
\toldpath %s is a file.\n\
\tThe inventory file is %s.\n"

#define ICV_OLDP_NOINF_D "\
%s:  An error occurred while processing the \n\
\tinventory file.\n\
\tCannot get information about the oldpath %s.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

#define ICV_RETURN_DIR_D "\
%s:  Cannot return to directory %s.\n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

#define ICV_RMDIR_D "\
%s:  Cannot remove the directory %s. \n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n\
\tThe directory must be empty to be removed.\n\
\tCheck path name and permissions.\n"

#define ICV_RMV_LINK_D "\
%s:  An error occurred while processing the \n\
\tinventory file.\n\
\tThe link %s could not be removed. \n\
\tThe return code is %d.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n\
\tCheck path name and permissions.\n"

#define ICV_STANZA_OLDPATH_D "\
%s:  The inventory file is not formatted correctly.\n\
\tThe oldpath %s is not an absolute path.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

#define ICV_STANZA_PATH_D "\
%s:  The inventory file is not formatted correctly.\n\
\tThe stanza name %s is not an absolute path name. \n\
\tThe inventory file is %s.\n"

#define ICV_STANZA_TYPE_D "\
%s:  The inventory file is not formatted correctly.\n\
\tThe type attribute is %s.\n\
\tIt must be either a file name or directory name.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

#define ICV_TYPE_D "\
%s:  An error occurred while reading the type attribute.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

#define ICV_UNARC_D "\
%s:  The file %s could not be unarchived.\n\
\tThe return code is %d.\n"

#define ICV_VPD_PATH_D "\
%s:  A path cannot be set to the /usr/lib/objrepos directory.\n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"


/*-----------------------------------------------------------------------
** INSTALLP MESSAGES
** The following messages are used by the installp command
**-----------------------------------------------------------------------*/

#define INP_ACTION_D "\
installp: Only one of the following flags may be used on \n\
\tthe command line at a time: \n\
\t\t-a, -ac, -c, -r, -u, -C, -A, -i, -l,or -s.\n"

#define INP_ALL_EX_D "\
installp: The -%c flag and a list of installable filesets\n\
\tare mutually exclusive.\n"

#define INP_AN_INVAL_D "\
installp: The NO SAVE (-N) option is invalid without \n\
\tthe COMMIT (-c) option. \n"

#define INP_APAR_INFO_D "\
\tFix information for %s\n"

#define INP_APPLY_BAD_SAVDIR_SYM_LINK_D "\
installp:  Using the existing save directory for the update\n\
\t%s.\n"

#define INP_APPLY_DANGLING_SAVDIR_SYM_LINK_D "\
installp: Cannot access the save directory,\n\
\t%s,\n\
\tfrom the symbolic link,\n\
\t%s.\n"

#define INP_A_ASSUMED_D "\
installp: No action was indicated. \n \
\tThe -a (apply) flag is being assumed. \n"

#define INP_BADNAM_D "\
installp:  %s is an invalid fileset name.\n\
\tUse the -l flag to see a list of available software.\n"

#define INP_BAD_RESTORE_D "\
installp: An error occurred while running the restore command.\n\
\tUse local problem reporting procedures.\n"

#define INP_BAD_USE_D "\
installp: The -%s flag is incompatible as given.\n"

#define INP_BEGIN_COPYRIGHT_D "\n\
. . . . . << Copyright notice for %s >> . . . . . . .\n"

#define INP_END_COPYRIGHT_D "\
. . . . . << End of copyright notice for %s >>. . . . \n\n"

#define INP_BOSBOOT_FAILED_D "\
%s:  An error occurred during bosboot processing.\n\
\tPlease correct the problem and rerun %s.\n"

#define INP_BOSBOOT_PERFORM_COMPLETE_D "\
%s:  bosboot process completed.\n"

#define INP_BOSBOOT_PERFORM_D "\
%s:  bosboot process starting...\n"

#define INP_BOSBOOT_VERIFY_COMPLETE_D "\
%s:  bosboot verification completed.\n"

#define INP_BOSBOOT_VERIFY_D "\
%s:  bosboot verification starting...\n"

#define INP_CANT_SYM_LINK_D "\
installp: Cannot create a symbolic link\n\
\tnamed %s\n\
\tto file or directory %s.\n\
\tCheck path name and permissions.\n"

#define INP_CLEANING_D "\n\
installp: Cleaning up software for:\n"

#define INP_CLEANUP_COMMIT_D "\n\
installp: Cleaning up software by finishing the commit operation for:\n"

#define INP_CLEANUP_NOSTAT_D "\
installp: The status file was not found. \n\
\tThis could be caused by an internal error within the \n\
\tcleanup script or could be caused by using Ctrl-c to \n\
\tinterrupt the installation processing.\n"

#define INP_CLEANUP_NOTFND_D "\
installp:  The cleanup script, %s,\n\
\tcould not be found for fileset %s.\n\
\tThe fileset has been marked as BROKEN since cleanup could not\n\
\tbe performed.\n"

#define INP_COMMIT_DANGLING_SAVDIR_SYM_LINK_D "\
installp:  Unable to access the save directory,\n\
\t%s,\n\
\tfrom the symbolic link,\n\
\t%s.\n\
\tCommitting without deleting saved files.\n"

#define INP_COMMIT_ERROR_D "\n\
installp:  Due to errors unable to commit fileset\n\
\t%s.\n"

#define INP_DEINST_NOSTAT_D "\
installp: The status file was not found. \n\
\tThis could be caused by an internal error within the \n\
\tdeinstall script or could be caused by using Ctrl-c to \n\
\tinterrupt the installation processing.\n"

#define INP_EXCLUS_D "\
installp: The -%s and -%s flags are mutually exclusive.\n"

#define INP_EXCLUS_F_G_D "\
installp: The \"AUTOMATICALLY install requisite software\" option \n\
\tand the \"OVERWRITE same or newer versions\" option from the \n\
\tprevious menu may not be selected together.\n"

#define INP_EXCL_ALL_D "\
installp: The -%s flag and the keyword \"all\" are mutually \n\
\texclusive.\n"

#define INP_EXEC_FAIL_D "\
\ninstallp: The reinvocation of the installp command has failed.\n\
\tUse local problem reporting procedures.\n"

#define INP_FAIL_CR_SUM_LOG_D "\
installp: Could not create the %s\n\
\tinstallp summary log file.\n"

#define INP_INSTRUCTIONS_D "\
\tInstructions for %s\nfrom file: %s\n"

#define INP_INTRN_FLNOTFND_D "\
installp: The status file was not found. \n\
\tThis could be caused by an internal error within the \n\
\tinstallation scripts or could be caused by using Ctrl-c \n\
\tto interrupt the installation processing.\n"

#define INP_INTRN_INVSTAT_D "\
installp: Invalid status character found in the status file.\n\
\tThis could be caused by an internal error within the \n\
\tinstallation scripts or could be caused by using Ctrl-c \n\
\tto interrupt the installation processing.\n"

#define INP_INTRN_MISSVPDI_D "\
installp: An internal error has occurred. Expected \n\
\tinformation was not found in the Software Vital Product Data.\n"

#define INP_INTRN_OPTNOTFND_D "\
installp: Fileset %s was not found in the status file. \n\
\tThis could be caused by an internal error within the \n\
\tinstallation scripts or could be caused by using Ctrl-c \n\
\tto interrupt the installation processing.\n"

#define INP_INUPDT_MSG_D "\
instupdt: The use of the -%s flag implies the use of the \n\
\t-%s flag(s). Please reissue the command or refer to the \n\
\tdocumentation on the installp command.\n"

#define INP_INURID_RUNNING_D "\
installp: The installp command cannot execute while the \n\
\tinurid command is running. If the inurid command is not really \n\
\trunning, remove the \"/usr/lpp/inurid_LOCK\" file and rerun the command.\n"

#define INP_INVAL_D "\
installp: A %s is an invalid argument to the -%s flag.\n"

#define INP_INVAL_LPP_D "\
installp: %s is an invalid fileset name.\n"

#define INP_INVAL_SCED_OPT_D "\
installp: Invalid superseding fileset name. The superseding\n\
\tfileset name %s\n\
\tmust match the current fileset.\n"

#define INP_INVAL_SCED_SCED_D "\
installp: Invalid superseding fileset update. The superseding\n\
\tupdate %s\n\
\tis more than %d bytes.\n\
\tThe superseding update may only be %d bytes.\n\n"

#define INP_LEVEL_D "\
installp: %s is invalid.\n \
\tAll levels must be preceded by the name of a fileset at \n\
\tthat level.\n"

#define INP_LINE_D "\
Usage installp:  Valid Flags are AaBbCcDdeFfgIilLNOpqrstuVvXz where:\n\n\
       -A  List fixes                     -l  List software on media\n\
       -a  Apply software                 -L  List software (colon separated)\n\
       -B  Consider updates only          -N  Do not save replaced files\n\
       -b  No bosboot                     -O  {[r][s][u]}  root, share, usr\n\
       -C  Cleanup install failure        -p  Preview\n\
       -c  Commit update(s)               -q  Quiet (no prompt)\n\
       -D  Delete install image           -r  Reject update(s)\n\
       -d  {install device}               -s  List applied software updates\n\
       -e  {log file}                     -t  {alternate save directory}\n\
       -F  Force installation             -u  Deinstall (Remove) fileset(s)\n\
       -f  {file containing fileset(s)}   -V  {verbosity level [2-4]}\n\
       -g  Include requisite software     -v  Verify installation\n\
       -I  Consider base levels only      -X  Expand filesystem if necessary\n\
       -i  List supplemental information  -z  {media block size}\n\n"

#define INP_LISTHEAD_UL_D "\
===================================================================="

#define INP_LIST_UL_D "\
--------------------------------------------------------------------"

#define INP_LOCKPENDING_D "\
installp:  Either there is an installp process currently \n\
\trunning, or there is a previously failed installation. \n\
\tWait for the process to complete or run installp -C to \n\
\tcleanup a failed installation.\n"

#define INP_LPPALL_D "\
%c Description: Will include ALL the images for %s.\n"

#define INP_LPP_ACTION_D "\
I/U"

#define INP_LPP_CONTENT2_D "\
Content"

#define INP_LPP_LABEL_D "\
Fileset Name"

#define INP_LPP_LEVEL_D "\
Level"

#define INP_LPP_QUISCENT2_D "\
Q"

#define INP_MISS_NETLS_PID_D "\
installp: The product ID data for fileset\n \
\t%s is missing from the iFOR/LS\n\
\tinformation within the table of contents on the installation media.\n"

#define INP_MISS_NETLS_PVER_D "\
installp: The product version data for fileset\n \
\t%s is missing from the iFOR/LS\n \
\tinformation within the table of contents on the installation media.\n"

#define INP_MUST_HAVE_D "\
installp: The -%s flag is only valid when the -%s flag is \n\
\talso used.\n"

#define INP_NEEDCLEANUP_D "\
installp: There are incomplete installation operations \n\
\ton the following filesets.  Run installp -C to clean up \n\
\tthe previously failed installations before continuing.\n\n"

#define INP_NOARG_D "\
installp: The -%s flag requires an argument.\n"

#define INP_NODEV_D "\
installp: Device %s could not be accessed.\n\
\tSpecify a valid device name.\n"

#define INP_NOLIBLPP_D "\
installp: Cannot install the specified\n\
\tfileset %s.\n\
\tCould not access the installation control files.\n\
\tUse local problem reporting procedures.\n"

#define INP_NOLIST_D "\
installp: This function of installp requires a list of \n\
\tinstallable filesets to be provided.\n\n"

#define INP_NOTHING_TO_CLEAN_ALL_D "\n\
installp: No filesets were found in the Software Vital\n\
\tProduct Database that could be cleaned up.\n"

#define INP_NOTINSTL_D "\
installp: %s is not a standard calling routine.\n"

#define INP_NOT_DISKFILE_D "\
installp: The -D flag is not valid because the installation\n\
\timage file is not on the file system.\n"

#define INP_NO_DEINST_PTF_D "\n\
installp: %s is NOT a valid\n\
\tbase level fileset.  You may only specify base level filesets\n\
\t(no updates) to be deinstalled.\n\n"

#define INP_NO_DEINST_SCRIPT_D "\n\
installp: Unable to access the default deinstall script, \n\
\t%s. Deinstallation attempt CANCELLED.\n\n"

#define INP_NO_INFO_D "\
\tThere was no information for the following:\n"

#define INP_NO_ROOT_TO_APPLY_D "\n\
installp: No filesets were found in the Software Vital\n\
\tProduct Database that could be applied to the \"root\"\n\
\tpart of %s.\n"

#define INP_ONE_D "\
installp: The -%s flag may only be used once on the command \n\
\tline.\n"

#define INP_ONE_TDIR_D "\
installp: The alternate save directory\n\
\tis %s.\n\
\tCommit or reject all fileset updates before changing the alternate\n\
\tsave directory to %s.\n"

#define INP_PERFORMING_OPER_D "\n\
installp: %s software for:\n"

#define INP_PERFORMING_OPER_PART_D "\n\
installp: %s software for the \"%s\" part of fileset\n\
\t%s.\n"

#define INP_OPER_FAIL_D "\n\
installp: The %s operation has failed for the following \n\
\tfilesets:\n"

#define INP_OPER_FAIL_PART_D "\n\
installp:  The %s operation has failed for the \"%s\" part \n\
\tof the following filesets:\n"

#define INP_OPER_SUCC_D "\
installp: The %s operation was successful for the following \n\
\tfilesets:\n"

#define INP_OPER_SUCC_PART_D "\
installp: The %s operation was successful for the \"%s\" part \n\
\tof the following filesets:\n"

#define INP_PART_COMMIT_D "\
installp: Warning. Some of the saved files may not have been\n\
\tcleaned up during the commit operation of %s.\n\
\tThe save directory is %s.\n"

#define INP_PATH_TOO_BIG_D "\
installp: The save directory name for the symbolic link is too long.\n"

#define INP_PREINST_FAIL_D "\
installp: After completion of pre-installation processing, \n\
\tthere are no valid filesets left to process.  No processing\n\
\thas occurred.\n"

#define INP_PTF_CANT_SEDE_SELF_D "\
installp: Fatal error: %s\n\
\terroneously supersedes itself. Processing cannot continue.\n"

#define INP_REINVOKE_D "\
\ninstallp: The installp command has been updated. Reinvoking installp\n\
\tto process the remaining items.\n"

#define INP_REJECT_DANGLING_SAVDIR_SYM_LINK_D "\
installp: The reject has been CANCELLED.\n\
\tProcessing cannot continue.\n\
\tA directory for saved files cannot be accessed.\n\
\tA symbolic link, %s,\n\
\tsuggests that a save directory is\n\
\t %s,\n\
\tand that the alternate save directory is\n\
\t %s.\n\
\tPlease check that the alternate save directory\n\
\tis mounted or available.\n\
\tA save directory is not accessible for\n\
\tthe following filesets:\n"

#define INP_REJECT_NOSTAT_D "\
installp: The status file was not found. \n\
\tThis could be caused by an internal error within the \n\
\treject script or could be caused by using Ctrl_c to \n\
\tinterrupt the installation processing.\n"

#define INP_REM_XFLAG_D "\
installp: Warning. The functionality for the -x flag has been \n\
\tremoved due to its destructive nature. The installp command \n\
\twill continue.\n"

#define INP_SHARE_READONLY_D "\
installp: \"share\" part is on a read only file system.\n\
\tSpecify parts with the -O[r][u] option.\n"

#define INP_SMIT_STATUS_HEAD_D "\
%sName                                Level\n"

#define INP_STATUS_HEAD_D "\
%sName                                Part    Level                   State\n"

#define INP_STATUS_HEAD_LINE_D "\
%s---------------------------------------------------------------------------\n"

#define INP_STATUS_D "\
%sInstallp Status\n"

#define INP_STATUS_LINE_D "\
%s---------------\n"

#define INP_SYSCK_ERROR_D "\
installp: The program \"sysck\" detected verification errors with\n\
\tfileset %s.\n\
\tThe fileset may still be usable but will be marked as BROKEN.\n"

#define INP_TFLAG_LOCK_D "\
installp: The alternate save directory %s\n\
\tis not available. If the alternate save\n\
\tdirectory is available, then delete\n\
\t%s/%s\n\
\tor specify a different alternate save directory.\n\
\tIf the file is on a remote filesystem,\n\
\tverify that the remote server allows root access to\n\
\tthe remote directory for this host.\n"

#define INP_USAGE_A_D "\
Usage installp:  Install software:  Apply Only (-a):\n\
      installp -a [-e LogFile] [-V Number] [-d Device] [-bBDIpqvX]\n\
               [-F | -g] [-O{[r][s][u]}] [-t SaveDirectory] [-z BlockSize] \n\
               { Fileset [Level] ... | -f ListFile | all }\n\n"

#define INP_USAGE_A_C_D "\
Usage installp:  Install Software:  Apply and Commit (-ac):\n\
      installp -ac[N] [-e LogFile] [-V Number] [-d Device] [-bBDIpqvX]\n\
               [-F | -g] [-O{[r][s][u]}] [-t SaveDirectory] [-z BlockSize] \n\
               { FileSet [Level] ... | -f ListFile | all }\n\n"

#define INP_USAGE_A_AC_D "\
Usage installp:  Install Software:  Apply Only (-a) or Apply/Commit (-ac):\n\
      installp [-a | -ac[N]] [-e LogFile] [-V Number] [-d Device]\n\
               [-bBDIpqvX] [-F | -g] [-O{[r][s][u]}] [-t SaveDirectory]\n\
               [-z BlockSize] { FileSet [Level] ... | -f ListFile | all }\n\n"

#define INP_USAGE_AL_D "\
Usage installp:  List Fixes (-A):\n\
      installp -A [-e LogFile] [-d Device] [-BIq] [-O{[s][u]}]\n\
               [-z BlockSize] { FileSet [Level] ... | -f ListFile | all }\n\n"

#define INP_USAGE_A_I_D "\
Usage installp:  List Fixes (-A) or Supplemental Information (-i):\n\
      installp {-A | -i} [-e LogFile] [-d Device] [-BIq] [-O{[s][u]}]\n\
               [-z BlockSize] { FileSet [Level] ... | -f ListFile | all }\n\n"

#define INP_USAGE_CL_D "\
Usage installp:  Clean Up Install Failure (-C):\n\
      installp -C [-e LogFile] [-b] \n\n"

#define INP_USAGE_CO_D "\
Usage installp:  Commit Applied Software Updates (-c):\n\
      installp -c [-e LogFile] [-V Number] [-bgpvX] [-O{[r][s][u]}]\n\
               { FileSet [Level] ... | -f ListFile | all }\n\n"

#define INP_USAGE_C_R_U_D "\
Usage installp:  Commit (-c), Reject (-r), or Deinstall (-u) Software:\n\
      installp {-c | -r | -u} [-e LogFile] [-V Number] [-bgpvX]\n\
               [-O{[r][s][u]}] { FileSet [Level] ... | -f ListFile | all }\n\
      Note:  The keyword \"all\" is not valid with the -r or -u flag.\n\n"

#define INP_USAGE_I_D "\
Usage installp:  List Supplemental Information (-i):\n\
      installp -i [-e LogFile] [-d Device] [-BIq] [-O{[s][u]}]\n\
               [-z BlockSize] { FileSet [Level] ... | -f ListFile | all }\n\n"

#define INP_USAGE_L_D "\
Usage installp:  List Software on Media (-l or -L):\n\
      installp [-l | -L ] [-e LogFile] [-d Device] [-BIq] [-O{[s][u]}]\n\
               [-z BlockSize]\n\n"

#define INP_USAGE_R_D "\
Usage installp:  Reject Applied Software Updates (-r):\n\
      installp -r [-e LogFile] [-V Number] [-bgpvX]\n\
               [-O{[r][s][u]}] { FileSet [Level] ... | -f ListFile }\n\n"

#define INP_USAGE_S_D "\
Usage installp:  List Applied Software Updates (-s):\n\
      installp -s [-e LogFile] [-O{[r][s][u]}] \n\
               [ FileSet [Level] ... | -f ListFile | all ]\n\n"

#define INP_USAGE_T_D "\
Usage installp:  Specifying alternate save directory with the -t flag:\n\
      installp [-a | -ac[N]] [-t SaveDirectory] [-e LogFile] [-V Number]\n\
               [-d Device] [-bBDIpqvX] [-F | -g] [-O{[r][s][u]}]\n\
               [-z BlockSize] { FileSet [Level] ... | -f ListFile | all }\n\n"

#define INP_USAGE_U_D "\
Usage installp:  Deinstall (Remove) Software:\n\
      installp -u [-e LogFile] [-V Number] [-bgpvX] [-O{[r][s][u]}]\n\
               { FileSet [Level] ... | -f ListFile }\n\n"

#define INP_USR_READONLY_D "\
installp: \"usr\" part is on a read only file system.\n\
\tSpecify parts with the -O[r][s] option.\n"

#define INP_PREV_INST_NOT_D "\
installp PREVIEW:  installation will not actually occur.\n"

#define INP_PREV_COMMIT_NOT_D "\
installp PREVIEW:  commit operation will not actually occur.\n"

#define INP_PREV_REJECT_NOT_D "\
installp PREVIEW:  reject operation will not actually occur.\n"

#define INP_PREV_DEINST_NOT_D "\
installp PREVIEW:  deinstall operation will not actually occur.\n"

#define INP_VERIF_INST_D "\
installp:  Pre-installation verification may take several minutes.\n"

#define INP_VERIF_COMMIT_D "\
installp:  Pre-commit verification may take several minutes.\n"

#define INP_VERIF_REJECT_D "\
installp:  Pre-reject verification may take several minutes.\n"

#define INP_VERIF_DEINST_D "\
installp:  Pre-deinstall verification may take several minutes.\n"

#define INP_PREV_END_D "\
End of installp PREVIEW.  "

#define INP_PREV_NO_APPLY_D "\
No apply operation has actually occurred.\n"

#define INP_PREV_NO_COMMIT_D "\
No commit operation has actually occurred.\n"

#define INP_PREV_NO_REJECT_D "\
No reject operation has actually occurred.\n"

#define INP_PREV_NO_DEINST_D "\
No deinstall operation has actually occurred.\n"

#define INP_PIV_D "\
\t\t    Pre-installation Verification...\n"

#define INP_PCV_D "\
\t\t\tPre-commit Verification...\n"

#define INP_PRV_D "\
\t\t\tPre-reject Verification...\n"

#define INP_PDV_D "\
\t\t    Pre-deinstall Verification...\n"

#define INP_INST_SW_D "\
\t\t\t Installing Software...\n"

#define INP_COMMIT_SW_D "\
\t\t\t  Committing Software...\n"

#define INP_REJ_SW_D "\
\t\t\t  Rejecting Software...\n"

#define INP_DEINST_SW_D "\
\t\t\t   Deinstalling Software...\n"

#define INP_PIP_D "\
\t\t     Post-installation Processing...\n"

#define INP_PCP_D "\
\t\t        Post-commit Processing...\n"

#define INP_PRP_D "\
\t\t        Post-reject Processing...\n"

#define INP_PDP_D "\
\t\t       Post-deinstall Processing...\n"

#define INP_SUMMARIES_D "\
\t\t\t\tSummaries:\n"

#define INP_CANT_REJ_COMMITEDS_D "\
Committed: cannot be rejected\n"

#define INP_INCOST_UR_PARTS_D "\
Inconsistent usr and root parts\n"

#define INP_NO_SW_PRODS_IN_VPD_D "\
installp:  No filesets were found in the Software\n\
\tVital Product Database in the APPLIED state.\n"

#define INP_PREV_AND_MAJOR_FLAG_D "\
installp:  The -p flag (preview operation) must be specified with\n\
\tone of the -a (apply), -c (commit), -r (reject), or\n\
\t-u (deinstall).\n\n"

#define INP_ONE_MAJ_PREV_FLAG_D "\
installp:  The -p flag (preview operation) must be specified with\n\
\tonly one of the following flags: -a (apply), -c (commit),\n\
\t-r (reject), or -u (deinstall).\n\n"

#define INP_INCMPLT_SUMMARY_D "\
installp:  This may result in the installation summary \n\
\tbeing incomplete.\n"

#define INP_FINIS_PROC_ALL_PKGS_D "\
Finished processing all filesets."

#define INP_PROC_XOFY_D "\
Filesets processed:  %d of %d"

#define INP_TT_HMS_D "\
  (Total time:  %d hrs %d mins %d secs).\n"

#define INP_TT_MS_D "\
  (Total time:  %d mins %d secs).\n"

#define INP_TT_S_D "\
  (Total time:  %d secs).\n"

#define INP_RESOURCES_TITLE_D "\
RESOURCES\n\
---------\n"

#define INP_NO_RESOURCE_EST_D "\
  Resource requirements could not be estimated.  It appears that size\n\
  information was not provided for the filesets being installed.\n\n"

#define INP_RESOURCES_HDR_D "\
  Estimated system resource requirements for filesets being installed:\n\
		(All sizes are in 512-byte blocks)\n"

#define INP_RESOURCES_COL_HDR_D "\
      Filesystem                     Needed Space             Free Space\n"

#define INP_TOTAL_RESOURCE_D "\
      TOTAL: %34d %22d\n\n"

#define INP_PAGE_SPACE_HDR_D "\
  Paging Space Requirements:    %10d\n\n"

#define INP_NEEDED_SPACE_D "\
  NOTE:  \"Needed Space\" values are calculated from data available prior\n\
  to installation.  These are the estimated resources required for the\n\
  entire operation.  Further resource checks will be made during\n\
  installation to verify that these initial estimates are sufficient.\n"

#define INP_USE_X_D "\
  Use the option to extend filesystems (-X flag) to ensure that any\n\
  additional resources which may be needed are automatically allocated.\n\n"

#define INP_BOSBOOT_DONTBOOT_D "\
%s: An error occurred during bosboot processing.\n\
\tDo NOT reboot your machine until you have successfully run\n\
\tthe installp -C command to cleanup.\n"

#define INP_BOSBOOT_WASRUN_D "\
installp: The bosboot command has successfully rebuilt the boot image.\n\
\tIt may be necessary to reboot your system before the changes\n\
\twill take effect.\n"

#define INP_APPLY_FAIL_PART_D "\n\
installp:  The installation has FAILED for the \"%s\" part \n\
\tof the following filesets:\n"

#define INP_REJECT_FAIL_PART_D "\n\
installp:  The reject operation has FAILED for the \"%s\" part \n\
\tof the following filesets (the software has been marked \n\
\tBROKEN and may need to be reinstalled):\n"

#define INP_CLEAN_FAIL_PART_D "\n\
installp:  The cleanup operation has FAILED for the \"%s\" part \n\
\tof the following filesets:\n"

#define INP_DEINSTALL_FAIL_PART_D "\n\
installp:  The deinstall operation has FAILED for the \"%s\" part\n\
\tof the following filesets:\n"

#define INP_NO_CMD_LINE_IN_NIM_D "\
installp:  The installp command cannot be run directly from\n\
\tthe command line in the Network Installation Management (NIM)\n\
\tenvironment.  Please use the NIM commands.\n"

#define INP_VERBOSITY_USAGE_D "\
installp:  The -V flag (detail/verbosity specifier for installp \n\
\toutput) may only be used in combination with one of the following: \n\
\t-a (apply), -c (commit), -ac (apply and commit), -r (reject), or \n\
\t-u (deinstall).\n\n"

#define INP_FORCE_APPLY_USR_D "\
installp:  You cannot force re-apply (-aF) the USR part without also \n\
specifying the ROOT part.\n"

#define INP_41_UPDT_HAS_SEDES_D "\
installp:  The 4.1-formatted fileset update %s\n\
\tincorrectly contains supersedes information.\n"

#define INP_NO_SEDES_IN_41_UPDT_D "\
installp:  4.1-formatted fileset updates cannot contain\n\
\tsupersedes information.\n"

#define INP_NO_FIX_ID_D "\
installp:  Base level fileset %s %d.%d.%d.%d.%s\n\
\tincorrectly contains a PTF ID field.\n"

#define INP_NO_FIX_IN_41_BASE_D "\
installp:  Base level fileset %s %d.%d.%d.%d.%s\n\
\tincorrectly contains a nonzero fix value.\n"

#define INP_FIX_MUST_BE_0_D "\
installp:  All 4.1-formatted base level filesets must contain a\n\
\tfix value of zero.\n"

#define INP_41_UPDT_HAS_FIX_ID_D "\
installp:  Fileset update %s %d.%d.%d.%d.%s\n\
\tincorrectly contains a PTF ID field.\n"

#define INP_NO_FIX_ID_IN_41_UPDT_D "\
installp:  4.1-formatted fileset updates cannot contain a PTF ID\n\
\tfield.\n"

#define INP_41_UPDT_HAS_ZERO_MF_D "\
installp:  The 4.1-formatted fileset update\n\
\t%s %d.%d.%d.%d.%s\n\
\tincorrectly contains zero modification and zero fix values.\n"

#define INP_NO_ZERO_MOD_AND_FIX_D "\
installp:  4.1-formatted fileset updates cannot contain zero\n\
\tmodification and zero fix values.\n"

#define INP_SEDES_PARSING_D "\n\
installp:  Error parsing supersedes information for\n\
\t%s.\n"

#define INP_BAD_LEVEL_IN_SEDES_D "\n\
installp:  An invalid level of %s was specified in the\n\
\tsupersedes information for %s.\n"

#define INP_BAD_NAME_IN_SEDES_D "\n\
installp:  An invalid fileset name was found in the supersedes\n\
\tinformation for %s.\n"

#define INP_LEVEL_TOO_HIGH_D "\
installp:  The level field specified in the supersedes information\n\
\tof a base level fileset must not be greater than the level of the base level\n\
\tfileset itself.\n"

#define INP_SEDES_NAME_MUST_MATCH_D "\n\
installp:  Any fileset name specified in the supersedes\n\
\tinformation of a base level fileset must match the name of the \n\
\tbase level fileset itself.\n"

#define INP_DEINST_3_1_D "\
installp:  %s is a 3.1-formatted fileset.\n\
\tDeinstallation of this fileset requires the use of the FORCE \n\
\toption (-F flag).\n\n"

#define INP_DEINST_3_2_D "\
installp:  The fileset, %s, \n\
\twas originally installed on an AIX 3.2 system.  Deinstallation\n\
\tof this fileset requires the use of the FORCE option (-F flag).\n\n"

#define INP_DEINST_MIGRATING_D "\
installp:  The fileset, %s, \n\
\twas originally installed on an AIX 3.2 system.  It has only been\n\
\tpartially migrated from 3.2 to 4.1 and, therefore, cannot be\n\
\tdeinstalled.\n\n"

#define INP_DEINST_COMPAT_D "\
installp:  The fileset, %s,\n\
\tis a compatibility entry used by installp and, therefore, cannot be\n\
\tdeinstalled.\n\n"

#define INP_REBOOT_USR_D "\n\
installp:  * * *  A T T E N T I O N ! ! ! \n\
\tSoftware changes processed during this session require this system \n\
\tand any of its diskless/dataless clients to be rebooted in order \n\
\tfor the changes to be made effective.\n"

#define INP_REBOOT_ROOT_D "\n\
installp:  * * *  A T T E N T I O N ! ! ! \n\
\tSoftware changes processed during this session require the system \n\
\tto be rebooted in order for the changes to be made effective.\n"

#define INP_STAT_HDR_D "\
FILESET STATISTICS \n\
------------------\n"

#define INP_NUM_SELECTED_D "\
%5d  Selected to be installed, of which:\n"

#define INP_NUM_PASSED_PIV_D "\
%9d  Passed pre-installation verification\n"

#define INP_NUM_FAILED_PIV_D "\
%9d  FAILED pre-installation verification\n"

#define INP_NUM_SUBSTITUTED_D "\
%9d  Replaced by superseding updates\n"

#define INP_NUM_ALREADY_INST_D "\
%9d  Already installed (directly or via superseding filesets)\n"

#define INP_ADDTL_REQS_D "\
%5d  Additional requisites to be automatically installed\n"

#define INP_ADDTL_MAND_D "\
%5d  Unselected mandatory filesets that FAILED pre-installation verification\n"

#define INP_TOT_TB_INST_D "\
%5d  Total to be installed\n"

#define INP_TOT_SEL_REJ_D "\
%5d  Selected to be rejected, of which:\n"

#define INP_TOT_PASS_PRV_D "\
%9d  Passed pre-reject verification\n"

#define INP_TOT_FAIL_PRV_D "\
%9d  FAILED pre-reject verification\n"

#define INP_ADTL_DEPEND_D "\
%5d  Additional dependents to be automatically rejected\n"

#define INP_TOT_TBR_D "\
%5d  Total to be rejected\n"

#define INP_TOT_SEL_TBD_D "\
%5d  Selected to be deinstalled, of which:\n"

#define INP_TOT_PASS_PDV_D "\
%9d  Passed pre-deinstall verification\n"

#define INP_TOT_FAIL_PDV_D "\
%9d  FAILED pre-deinstall verification\n"

#define INP_ADTL_DEP_DEINST_D "\
%5d  Additional dependents to be automatically deinstalled\n"

#define INP_TOT_TBD_D "\
%5d  Total to be deinstalled\n"

#define INP_TOT_SELECTED_TBC_D "\
%5d  Selected to be committed, of which:\n"

#define INP_NUM_ALREADY_COMMITTED_D "\
%9d  Already in the committed state\n"

#define INP_ADTL_REQ_COMMIT_D "\
%5d  Additional requisites to be automatically committed\n"

#define INP_TOT_TBC_D "\
%5d  Total to be committed\n"

#define INP_NUM_DEFERRED_D "\
%9d  Deferred (see *NOTE below)\n"

#define INP_DEFERREDS_NOTE_D "\
*NOTE  The deferred filesets mentioned above will be processed after the\n\
       installp update and its requisites are successfully installed.\n"

#define INP_PIFWS_HDR1_D "\
Pre-installation Failure/Warning Summary\n\
----------------------------------------\n"

#define INP_PIFWS_HDR2_D "\
Name                      Level           Pre-installation Failure/Warning\n\
--------------------------------------------------------\
-----------------------\n"

#define INP_PIF_NOM_TRY_SEDES_D "\
Not found; try %s\n"

#define INP_PIF_USR_NOT_INSTALLED_D "\
USR not instld; try %s\n"

#define INP_PIF_ALREADY_INST_D "\
Already installed\n"

#define INP_PIF_ALREADY_COMMIT_D "\
Already COMMITTED\n"

#define INP_PIF_FAILURE_D "\
Pre-installation failure\n"

#define INP_PIF_NO_USR_MEANS_NO_ROOT_D  "\
No root part or usr not installed\n"

#define INP_PIF_FORCE_APPLY_PTF_D "\
Cannot force apply fileset updates\n"

#define INP_PIF_FORCE_DEINST_D "\
Must use -F flag (force) to deinstall\n"

#define INP_PIF_PART_MIG_DEINST_D "\
Partially migrated, not deinstallable\n"

#define INP_PIF_COMPAT_DEINST_D "\
Compatibility, not deinstallable\n"

#define INP_PIF_ALREADY_INSTALLED_D "\
Already installed\n"

#define INP_PIF_ALREADY_SUPERSEDED_D "\
Already superseded by %s\n"

#define INP_PIF_BASE_MUST_BE_COMMITTED_D "\
Must commit before reinstalling\n"

#define INP_PIF_BROKEN_NEEDS_COMMIT_D "\
BROKEN, requires the -acgN flags\n"

#define INP_PIF_MUST_APPLY_ROOT_TOO_D "\
Must apply both usr and root parts\n"

#define INP_PIF_NOT_FOUND_ON_MEDIA_D "\
Not found on the installation media\n"

#define INP_PIF_NO_DEINST_BOS_D "\
bos.rte is not deinstallable\n"

#define INP_PIF_NUTTIN_TO_APPLY_D "\
Nothing by this name to apply\n"

#define INP_PIF_NUTTIN_TO_COMMIT_D "\
Must be in APPLIED state to commit\n"

#define INP_PIF_NUTTIN_TO_DEINSTL_D "\
Nothing by this name to deinstall\n"

#define INP_PIF_NUTTIN_TO_REJECT_D "\
Must be in APPLIED state to reject\n"

#define INP_PIF_OTHER_BROKENS_NEED_COMMIT_D "\
First apply BROKENs for this fileset\n"

#define INP_PIF_REQUISITE_FAILURE_D "\
Requisite failure\n"

#define INP_PIF_TO_BE_SUPERSEDED_D "\
To be superseded by %s\n"

#define INP_PIF_FAILED_PRE_D_D "\
Failed pre-deinstallation check\n"

#define INP_PIF_SUPS_A_BROKEN_FILESET_D "\
Supersedes a BROKEN fileset\n"

#define INP_PIW_SEE_WARNING_SECTION_D "\
See WARNINGS section above\n"

#define INP_SUMM_LINE_D "\
Installation Summary\n\
--------------------\n"

#define INP_SUM_HEAD_D "\
Name                        Level           Part        Event       Result\n\
--------------------------------------------------------\
-----------------------\n"


#define INP_CFGFILES_STORED_D "\
Some configuration files could not be automatically merged into the system \n\
during the installation.  The previous versions of these files have been \n\
saved in a configuration directory as listed below.  Compare the saved files \n\
and the newly installed files to determine if you need to recover \n\
configuration data.  Consult product documentation to determine how to \n\
merge the data. \n\
"

#define INP_CFGLIST_D "\
\n\
 Configuration files which were saved in %s:\n\
"

#define INUCP_INST_ROOT_D "\
\t\t\t Copying files from\n %s...\n"

#define INP_NIM_USR_SPOT_WARNING_D "\
Warning: the /usr filesystem on this machine has been converted into\n\
a Network Install Manager (NIM) SPOT.  If there are any diskless or\n\
dataless NIM clients using this SPOT, you should perform the following\n\
NIM operations when this invocation of installp finishes:\n\
\tnim -o sync_roots %s\n"

#define INP_OEM_MISMATCH_D "\
Incorrect system manufacturer\n"

#define INP_OEM_REPLACED_D "\
System manufacturer fileset required\n"

#define INP_OEM_BASELEVEL_D "\
System manufacturer fileset installed\n"

#endif /* _H_INSTALLPMSG */

