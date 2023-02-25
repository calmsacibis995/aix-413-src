$ @(#)26  1.22.4.2  src/bos/usr/sbin/install/inuumsg/inuumsg.msg, cmdinstl, bos411, 9435C411a 8/26/94 11:01:02
$
$ COMPONENT_NAME: (CMDINSTL) LPP Install
$
$ FUNCTIONS: Message catalog (inuumsg)
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set MS_INUUMSG

$
$ requested by em78
$
INU_NOFILE "0503-201 Cannot read the input file %s.\n\
\tCheck path name and permissions.\n"
$ %s is replaced by the file name

INU_UNKCMD "0503-202 %s is not a known command.\n\
\tUse local problem reporting procedures.\n"
$ media may be faulty or machine may be bad, or many other faults...
$ %s is replaced by the specified command 

INU_CKPRQ "0503-203 A program prerequisite was not found on the system.\n"
$ Software that is required for a particular program to run is
$ not present on the system.

INU_NOWHERE "0503-204 The specified country %s in the LPP option\n\
\tfile is not correct.\n"
$ %s is replaced by the country name 

INU_NOSIZE "0503-205 Cannot get the size and checksum for file %s.\n\
\tUse local problem reporting procedures.\n"
$ %s is replaced by the file name 

INU_NOPRO "0503-206 Cannot get information about the system profile %s.\n\
\tUse local problem reporting procedures.\n"
$ %s is replaced by the system profile name 

INU_1PLACE "0503-207 Select one country per installation only.\n"

INU_8 "0503-208\n"

INU_9 "0503-209\n"

$
$ requested by whip
$
$ LPP is the acronym for licensed program product
$ HCON is the acronym for workstation host interface program  
$

INU_HCONACTIVE "0503-210 The HCON server is active.\n\
\tStop the HCON server before installing or updating HCON.\n"
$ HCON is the acronym for workstation host interface program  

INU_BADSER "0503-211 The status of HCON is not correct.\n\
\tStop the HCON server before installing or updating HCON.\n"
$ HCON is the acronym for workstation host interface program  

INU_CONFIG "0503-212 The default configuration failed.\n\
\tThe installation is stopped. Correct the configuration and try again.\n"

INU_FORK "0503-213 Cannot create another process.\n\
\tTry again later or\n\
\tuse local problem reporting procedures.\n"

INU_EXEC "0503-214 The system call execl for %s failed.\n\
\tUse local problem reporting procedures.\n"
$ %s will be replaced with a string describing a system call or function

INU_TRYINGTO "0503-215 The installation or update process failed\n\
\ttrying to %s.\n"
$ %s will be replaced with a string such as:
$ save the old files
$ restore the new files
$ create an error log
$ create a trace log
$ create an event log
$ create a subsystem
$ start the HCON subsystem

INU_NOIBMC "0503-216 WARNING: I.B.M. C compiler not installed.\n"

INU_NOMRI "0503-217 WARNING: M.R.I. for HCON not installed.\n"

INU_CLEANUP "0503-218 Failure occured while\n\
\tattempting to cleanup %s.\n"

INU_19 "0503-219\n"

$
$ HCON is the acronym for workstation host interface program  

$
$ requested by SNA
$

INU_PARM "0503-220 One or more parameters not passed from installp\n\
\tto instal script.\n"

INU_FILE "0503-221 Parameter file from installp nonexistent.\n"

INU_SNACTIVE "0503-222 SNA is currently running.  SNA must be stopped\n\
\tbefore installing or updating sna or lu0.\n"

INU_23 "0503-223\n"

INU_INUSAVE "0503-224 The program /etc/inusave failed.\n"

INU_INUREST "0503-225 The program /etc/inurest failed.\n"

INU_ERRUPDATE "0503-226 The program /etc/errupdate failed.\n"

INU_NONFATAL "0503-227 A non-fatal error occurred when the\n\
\tprogram /usr/bin/peu failed.  The installation will continue, but\n\
\tthe command /usr/bin/peu must be run successfully before SNA services\n\
\tcan be used.\n"

INU_MKSYS "0503-228 mksys failed.\n"

INU_MKSERVER "0503-229 mkserver failed.\n"

INU_NOTBOTH "0503-230 You are unable to install the AIX VS COBOL Run Time\n\
\tEnvironment because it has already been installed\n\
\tas part of the AIX VS COBOL compiler.\n"

$
$ requested by X
$

INU_X_EXT_LINK_FAIL "X failed to link with extensions. Relinking without extensions...\n"
INU_X_SERVER_LINKING "X server is linking...\n"
INU_X_LINK_FAIL "X Failed to link successfully\n"
INU_X_CLIENT_LINKING "X11 clients are linking...\n"

$
$ requested by diagnostics
$

INU_DIAGREST "0503-235 The installation or update process has failed\n\
\ttrying to restore the System Diagnostics.\n"

$
$ requested by tcpip
$

INU_PROGFAIL "0503-236 The program %s failed.\n"
$ The %s parameter will be replaced by the name of the failed command.
$ This is a generic message that should follow error messages from the
$ command in question, so any further specificity should be the responsibility
$ of the failed command.


$
$ requested by DOSSRV
$
INU_DOSSVR_REQFILE "0503-237 Installation cannot continue because\n\
\tthe following file is missing: %s\n"

INU_DOSSVR_RUNNING "0503-238 Dos Server is currently running.\n\
\tIt must be stopped before a new installation can be performed.\n"


$
$ NEW STANDARD MESSAGE FOR CKPREREQ, INURECV, INUSAVE, INSTUPDT
$ Blank messages are reserverd for latter use
$

INU_STD_CKPREREQ "0503-239 The program prerequisite %s \n\
\twas not found on the system.\n"

INU_STD_INURECV "0503-240 An error was received while recovering saved files.\n\
\t/etc/inurecv return code: %s.\n"

INU_STD_INUSAVE "0503-241 An error was received while saving files.\n\
\t/etc/inusave return code: %s.\n"

INU_STD_INUREST "0503-242 An error was received while restoring files from the media.\n\
\t/etc/inurest return code: %s.\n"

INU_STD_CKP_COND "0503-243 Option %s requires at least one \n\
\tof the following programs to be installed on the system:\n\
\t\t%s.\n"

INU_STD_A "0503-244 No Message Defined\n"
INU_STD_B "0503-245 No Message Defined\n"
INU_STD_C "0503-246 No Message Defined\n"
INU_STD_D "0503-247 No Message Defined\n"
INU_STD_E "0503-248 No Message Defined\n"
INU_STD_F "0503-249 No Message Defined\n"


$
$ Messages for Net Work Management LPP install
$
INU_NTWK_50  "0503-250 No Message Defined\n"
INU_NTWK_51  "0503-251 No Message Defined\n"
INU_NTWK_52  "0503-252 No Message Defined\n"

INU_NTWK_AM_ACT     "0503-253 Alert Manager is active, installation has been canceled. \n"
$ Do not translate : Alert Manager

INU_NTWK_RPT_ACT    "0503-254 Alert Report is active, installation has been canceled. \n"
$ Do not translate : Alert Report

INU_NTWK_55  "0503-255 No Message Defined\n"
INU_NTWK_56  "0503-256 No Message Defined\n"
INU_NTWK_57  "0503-257 No Message Defined\n"
INU_NTWK_58  "0503-258 No Message Defined\n"
INU_NTWK_59  "0503-259 No Message Defined\n"

INU_NTWK_AM_DATA    "0503-260 Cannot create the Alert Manager data bases, installation\n\
\thas been canceled. \n"
$ Do not translate : Alert Manager


INU_NTWK_ADD_TCP    "0503-261 The installation of Alert Manager failed.\n\
\tUnable to add 'amrcv  1700/tcp' to /etc/services. \n"
$ Do not translate : Alert Manager

INU_NTWK_62  "0503-262 No Message Defined\n"
INU_NTWK_63  "0503-263 No Message Defined\n"
INU_NTWK_64  "0503-264 No Message Defined\n"
INU_NTWK_65  "0503-265 No Message Defined\n"
INU_NTWK_66  "0503-266 No Message Defined\n"
INU_NTWK_67  "0503-267 No Message Defined\n"
INU_NTWK_68  "0503-268 No Message Defined\n"
INU_NTWK_69  "0503-269 No Message Defined\n"
INU_NTWK_70  "0503-270 No Message Defined\n"
INU_NTWK_71  "0503-271 No Message Defined\n"
INU_NTWK_72  "0503-272 No Message Defined\n"

$ Requested by DOSSERV
$
INU_DOSSRV_1 "1390-186 X11 or TCP/IP not installed.  Error Installing %s.\n"
INU_DOSSRV_2 "1390-187 Save of existing files was unsuccessful.\n"
INU_DOSSRV_3 "1390-188 Error installing %s.\n"
INU_DOSSRV_4 "1390-189 Files installed in /usr/lpp/%s.\n"
INU_DOSSRV_5 "1390-190 Adding bootps file to /etc/services directory.\n"
INU_DOSSRV_6 "1390-191 Adding bootpc file to /etc/services directory.\n"
INU_DOSSRV_7 "1390-192 Warning - TFTP is not in %s.\n"
INU_DOSSRV_8 "1390-193 Warning - you must add a nobody account.\n"
INU_DOSSRV_9 "1390-196 Default port number %s in use, trying another .\n"
INU_DOSSRV_10 "1390-195 Error, no %s file.\n"
INU_DOSSRV_11 "1390-197 Removing bootps in %s.\n"
INU_DOSSRV_12 "1390-198 Removing x_st_mgr to %s.\n"
INU_DOSSRV_13 "1390-199 Error, unable to remove /usr/lpp/%s.\n"
INU_DOSSRV_14 "1390-200 Files removed in /usr/lpp/%s.\n"
INU_DOSSRV_15 "1390-201 Configuration files exist, saving previous versions.\n"

$ 
$ PCSIM messages
$ 
INU_PCSIM_ROS "0503-288 An error was received while reading the Read Only Storage on\n\
\tthe processor card.\n"

INU_PCSIM_README "0503-289 Please read the README file in /usr/lpp/pcsim/samples.\n"

INU_PCSIM_UNCMPRS "0503-290 An error was received while uncompressing files.\n"

INU_PCSIM_BIND "0503-291 PCSIM was unable to bind PCsimExt.o with X for optimum\n\
\tperformance of the simulator in VGA mode.\n"

INU_REBOOT "0503-292 This update will not fully take effect until after a\n\
\tsystem reboot.\n"

$
$ Messages for txtfmt.ts.obj (Transcript)
$

INU_TS_1 "lpp.reject: Restoring original font families.\n"

INU_TS_2 "lpp.reject: This will take a while, please wait.\n"
INU_TS_3 "ts.config: Attempting to rebuild installed font families:\n"
INU_TS_4 " Times(default)\n"
$ do not translate Times(default)
$
$ The above 2 messages comprise a message pair which will tell the
$ user what font families are to be built. This is dependent on what
$ is installed on the target system. When inuumsg is incorporated
$ into ts.config.sh, the code should be broken apart into 3 pieces
$ as follows:
$ CURRENT CODE in ts.config.sh
$	print -r "Attempting to rebuild installed font families:"
$	print -r "`echo ${fontFamilies} | sed 's/Family//g'` Times(default)"
$ NEW CODE (to replace the current code)
$ 	inuumsg <MSG-NUMBER-1>	# msg = Attempting to rebuild...families:
$	print -rn "`echo ${fontFamilies} | sed 's/Family//g'`"
$	inuumsg <MSG-NUMBER-2>	# msg = TIMES(default)

INU_TS_5 "ts.config: Update of font families has failed in setup phase.\n"
INU_TS_6 "ts.config: Installing font families.\n"
INU_TS_7 "ts.config: This will take a few minutes, please wait.\n"
INU_TS_8 "ts.config: Update of font families has failed during build phase.\n"
INU_TS_9 "ts.config: Font installation successful!"
INU_FATALI "The installation or updating script is unable to continue\n"
INU_INUCP "0503-293 The program /etc/inucp failed.\n"
INU_FATALC "The cleanup or reject script is unable to continue.\n"

$
$ requested by SNA
$

INU_MGRTSNAPRFL "\
0503-293 The migration of the SNA profiles failed;  the installation will\n\
\tcontinue.  After the SNA installation has completed successfully,\n\
\tthe old SNA profiles can be found in the file \n\
\t/tmp/.SNAmigration.\n"

$ added to support diskless

INU_DISKLESSBOOT "\
Something which has just been installed on your system has caused the network\n\
boot image to be updated.  A new network boot image has been created and\n\
replaced the /usr/lib/boot/net.image file.  If you have already created a\n\
diskless environment and you are serving your /usr filesystem as a SPOT, then\n\
you will need to copy the new network boot image into the boot directory\n\
associated with this SPOT.  To determine the location of the boot directory,\n\
execute the following command :\n\n\
          lsspot -L <SPOTname> (where SPOTname represents the name which was\n\
                                used when the SPOT was created)\n\n\
The boot directory associated with this SPOT will be displayed.  You should\n\
now copy the new network boot image from /usr/lib/boot/net.image to the boot\n\
directory, using a target filename of <SPOTname>.  Be sure to copy this file,\n\
not link it : linking it could result in tftp errors when a diskless client\n\
attempts to access it.\n\n\
If you have other SPOTs (ie., SPOTs other than /usr on this machine), then it\n\
is recommended that you use the instlclient command to install the changes\n\
into those SPOTs also.  When the instlclient command is used, you will not\n\
have to copy the new network boot image, as instlclient will do this for you.\n"

INU_DISKLESSBOOT2 "Creating a network boot image for diskless clients.\n"

INU_DISKLESSBOOT3 "This update will not fully take effect until after all\n\
\tdiskless clients are rebooted.\n"

INU_DISKLESSBOOT4 "By rejecting this ptf the /usr/lib/boot/net.image is now\n\
out of date!\n\
If this SPOT is used by diskless clients then a new boot image will need to\n\
be created by using the following command:\n\
\tbosboot -a -d /dev/network_device\n\
where network_device is one of the following:\n\
\t/dev/tok0\t/dev/tok1\t/dev/tok2\t/dev/tok3\n\
\t/dev/ent0\t/dev/ent1\t/dev/ent2\t/dev/ent3\n\
After creating the new /usr/lib/boot/net.image you will need to copy it\n\
into the boot directory associated with this SPOT.  To determine the location\n\
of the boot directory, execute the following command :\n\n\
          lsspot -L <SPOTname> (where SPOTname represents the name which was\n\
                                used when the SPOT was created)\n\n\
The boot directory associated with this SPOT will be displayed.  You should\n\
now copy the new network boot image from /usr/lib/boot/net.image to the boot\n\
directory, using a target filename of <SPOTname>.  Be sure to copy this file,\n\
not link it : linking it could result in tftp errors when a diskless client\n\
attempts to access it.\n\n"

$ Support NetLS install

INU_NETLS_GLOBAL "\n\n0503-300   WARNING\n\n\
                NCS 1.1 is not equipped to handle replicatable global\n\
        brokers, you must have version 1.5.1. Since NCS is part of the\n\
        base operating system, it cannot simply be removed. To ensure\n\
        NetLS will install properly we must stop the llbd and nrglbd\n\
        daemons and remove them from the SRC subsystems database. \n\n "

INU_NET_RMSYS "\n0503-301  rmssys failed to remove %s.  Cannot continue!"

INU_NET_STOPSRC "\n0503-302  stopsrc failed to stop %s.  Cannot continue!"

INU_NET_RLM "\
\n0503-303  RLM is currently installed. Must stop rlmd daemon and remove\n\
the SRC subsystem database entry. \n"

INU_NET_NODELOCK "\n\n0503-306   WARNING\n\n\
        The information contained in /usr/lpp/rlm/db/nodelock must be\n\
        place in /usr/lib/netls/conf/nodelock once NetLS is completely and\n\
        properly installed. \n\n "

INU_NET_FAIL "\n0503-307  %s failed on %s.\n"

INU_NET_RMENTRY "\n  Removing entry %s from inittab.\n"

INU_NET_NOTRM "\n0503-308  rmittab failed!  /etc/inittab entry for %s was\n\
not removed\n"

$ deinstal script messages

INU_DEINST_EXEC_ERROR "\n  WARNING: %s: Problem executing %s.\n\
        Continuing... \n\n "


$ inurid script messages

INU_BIRON_LOCK "\
inurid:  Cannot remove the %s lock file.\n"

INU_BIRON_ALREADY_RUN "\
inurid:  The inurid command is already running.  If it really is\n\
\tnot, remove the %s file.\n"

INU_BIRON_CANT_CR "\
inurid:  Cannot create the %s lock file.\n"

INU_BIRON_VPD_ACCESS "\
inurid:  Failed while attempting to access the SWVPD database.\n"

INU_BIRON_SPOT_EXISTS "\
inurid:  Cannot execute because a /usr diskless SPOT exists.\n"

INU_BIRON_MUT_EX_FLAGS "\
inurid:  The -q and -r flags are mutually exclusive.\n"

INU_BIRON_Q_ONLY_ONCE "\
inurid:  The -q flag may only be given once.\n"

INU_BIRON_R_ONLY_ONCE "\
inurid:  The -r flag may only be given once.\n"

INU_BIRON_CANT_CD "\
inurid:  Cannot change directory to %s.\n"

INU_BIRON_IR_FILES "\
inurid:  Failed cleaning up the inst_root files.\n"

INU_BIRON_USAGE "\
Usage:    inurid {-q | -r}\n\
            -q Query if the BIRON variable is set or not\n\
               returns 1 if BIRON is set to yes and\n\
                       0 if BIRON is set to no\n\
            -r Set the BIRON variable to yes and remove\n\
               appropriate files\n\n\
WARNING:  Use of this script will prevent your system from\n\
          being used as a /usr SPOT or /usr server.\n\n\
          This script will not change the BIRON variable\n\
          to yes if a /usr SPOT already exists.\n"

INU_BIRON_INSTALLP_RUNNING "\
inurid:  The installp command is currently running.  The inurid command\n\
\tcannot execute while the installp command is running.\n"

$
$ requested by dsmit
$
INU_SUN_INSTALL_MSG "\
    _____________________________________________________________________ \n\
    NOTE:  Refer to /usr/share/DSMIT/SunOS_4.1.3/inst.images/README for \n\
    further installation instructions.\n\
    _____________________________________________________________________ \n"

INU_HP_INSTALL_MSG "\
    _____________________________________________________________________ \n\
    NOTE:  Refer to /usr/share/DSMIT/HP-UX_9.0/inst.images/README for \n\
    further installation instructions.\n\
    _____________________________________________________________________ \n"


$
$ requested by default migration processing
$
INU_MIG_USAGE "\
Usage: migrate_cfg config_list save_directory \n\
    or migrate_cfg config_list #with MIGSAVE=name of save directory \n\
"
$ Do not translate migrate_cfg.
$ config_list is the name of a file containing a list of user configurable
$ files to be processed by the migration script.

INU_MIG_FAIL "\
migrate_cfg: Unable to migrate %s.  Saved version remains in $MIGSAVE/%s.\n"
$ Do not translate migrate_cfg.
$ %s is replaced by the file name of file being migrated.  "Migrated" means
$ that the file is being processed to work with another level of the product.

INU_MIG_NOCFG "\
migrate_cfg: Error: Unable to read configuration file %s. \n"
$ Do not translate migrate_cfg.
$ %s is replaced by the name of a file name containing a list of user
$ configurable files to be processed by the migration script.

INU_MIG_NONDIR "\
migrate_cfg: Error: Save directory %s is not a writeable directory. \n"
$ Do not translate migrate_cfg.
$ %s is the name of a directory 

$
$  messages for sm_inst.sh
$

INU_SM_INST_COMMON_HEADER_STRING "\
#---------------------------------------------------------------------\n\
# + = No license password required \n\
# $ = License found\n\
# ! = No license found\n\
# \n\
# The \"all_licensed\" filter will include filesets with \"+\" and \"$\".\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_MAINT_ENH_HEADER_STRING "\
#---------------------------------------------------------------------\n\
# %s\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_NO_PRODUCTS_ON_MEDIA_STRING "\
#---------------------------------------------------------------------\n\
# No installable software products were found on the media.\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_NO_ENH_ON_MEDIA_STRING "\
#---------------------------------------------------------------------\n\
# No enhancements were found on the media.\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_NO_MAINT_ON_MEDIA_STRING "\
#---------------------------------------------------------------------\n\
# No maintenance levels were found on the media.\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_NO_FILESET_UPDATES_ON_MEDIA_STRING "\
#---------------------------------------------------------------------\n\
# No fileset updates were found on the media.\n\
#---------------------------------------------------------------------\n"

$
$  messages for rminstal.sh
$

INU_PRE_RM_FAIL  "Failure occurred during pre_rm.\n"

$
$  messages for diagnostics unconfig script.
$

INU_DIAG_MKITAB_FAIL "\nWARNING: mkitab failed! Unable to add 'diagd' entry\n\
to /etc/inittab\n"

INU_DIAG_RMITAB_FAIL "\nWARNING: rmitab failed! Unable to remove 'diagd' entry\n\
from /etc/inittab\n"
$
$ Do not translate /etc/inittab, diagd, mkitab and rmitab.
$

INU_SCRIPT_FAILED "%s:  Failed while executing the %s script.\n\n"

$
$  messages for rminstal.sh
$

INU_SAVECONFIG_FAIL "Failure occurred while saving config files.\n"

INU_RMI_FAIL "Failure occurred during rminstal.\n"

$
$  message for inuumsg.sh
$

INU_MSG_NOT_FOUND "\
0503-273 inuumsg:  Message number %d not found.\n"


$
$  further messages for sm_inst.sh including entries for bundles / filters
$

INU_SM_INST_NO_BUNDLE "Bundle %s does not exist.\n"
$ %s is replaced by the bundle name

INU_SM_INST_NO_NETLS_INFO_FOR_ALL_LICENSED "\
Unable to obtain NetLS information to determine ALL_LICENSED options.\n"

INU_SM_INST_MISSING_BUNDLE_OPTIONS_STRING "\
#---------------------------------------------------------------------\n\
# The following options in bundle %s were not found on the media.\n\
#---------------------------------------------------------------------\n"
$ %s is replaced by the bundle name

INU_SM_INST_INVALID_DEVICE "Invalid device specified.\n"

INU_SM_INST_NO_DEVICE "Input device or directory does not exist.\n"

INU_SM_INST_CANNOT_READ_DEVICE "\
Cannot read input device, directory or table of contents file.\n"

INU_SM_INST_NO_SYS_BUNDLES_DIR "\
System bundles directory %s does not exist.\n"
$ %s is replace by the directory name /usr/sys/inst.data/sys_bundles

INU_SM_INST_NO_BUNDLE_OPTIONS "\
Bundle %s does not contain valid packages or filesets.\n"
$ %s is replaced by the bundle name

INU_SM_INST_THIS_TAKES_AWHILE "\
This command can take a significant amount of time to run.\n"

$
$  message for DPS LPP installation
$
INU_DPS "\
The Display Postscript LPP will not install because a valid \n\
iFOR/LS license is not available.  Please obtain a valid \n\
iFOR/LS license for the DPS LPP before installing the DPS LPP.\n"

INU_SM_INST_BUNDLE_HEADER_STRING "\
#---------------------------------------------------------------------\n\
# + = No license password required \n\
# $ = License found\n\
# ! = No license found\n\
# \n\
# Only filesets with \"+\" and \"$\" will be selected.\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_NO_BUNDLE_MATCHES_ON_MEDIA_STRING "\
#---------------------------------------------------------------------\n\
# No filesets on the media matched the bundle contents.\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_NO_FILTER_MATCHES_ON_MEDIA_STRING "\
#---------------------------------------------------------------------\n\
# No filesets on the media matched the filter criteria.\n\
#---------------------------------------------------------------------\n"

$
$ Messages for oslevel
$

INU_OSLEVEL_USAGE_STRING "\
Usage: oslevel [ -l <level> | -g | -q ]\n\
     -l : List filesets at levels earlier than maintenance level\n\
          specified by the <level> parameter\n\
     -g : List filesets at levels later than most recent\n\
          complete maintenance level\n\
     -q : List names of known maintenance levels which may be\n\
          specified with the -l flag\n\n\
  Output indicates that base system software is entirely at\n\
  or above a particular maintenance level. Corresponding output\n\
  would be 4.1.1.0 first AIX 4.1 maintenance level.\n\n\
  The additional options may be specified to determine which\n\
  filesets differ from the maintenance level.\n"

INU_OSLEVEL_TOO_MANY_LEVELS_ERROR "\
Error: Only specify one of [ -l <level> | -g | -q ]\n\n"

INU_OSLEVEL_WRONG_VERSION_ERROR "\
Error: oslevel cannot be used on AIX Version 3\n"

INU_OSLEVEL_NO_MLINFO "\
No maintenance level information is available.\n"

INU_OSLEVEL_INVALID_ML_ERROR "\
Error: Maintenance level %s is invalid or unknown.\n"
$ %s is replaced by the maintenance level name

INU_OSLEVEL_LIST_ML_HEADING "\
Known Maintenance Levels\n\
------------------------\n"

INU_OSLEVEL_NO_ML_FOUND "\
There are no complete maintenence levels installed.\n"

INU_OSLEVEL_LIST_FILESETS_HEADING "\
Fileset                                 Actual Level        Maintenance Level\n\
-----------------------------------------------------------------------------\n"

$
$ Messages for sm_inst.sh
$

INU_SM_INST_HDR_BFF_LIST "\
#  Package Name                     Level\n\
#---------------------------------------------------\n"

INU_SM_INST_MAINT_LEVEL_HEADER_STRING "\
#---------------------------------------------------------------------\n\
# Maintenance Levels\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_ENHANCEMENT_HEADER_STRING "\
#---------------------------------------------------------------------\n\
# Enhancements\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_UPDATES_HEADER_STRING "\
#---------------------------------------------------------------------\n\
# Fileset Updates\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_INVALID_KEYWORD "\
%s is not a valid keyword.\n"
$ %s is replaced by the invalid keyword

INU_SM_INST_FAILED_NETLS_INFO_FOR_ALL_LICENSED "\
#---------------------------------------------------------------------\n\
# Unable to obtain NetLS information to determine ALL_LICENSED options.\n\
# All options which require a license will be treated as unlicensed.\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_NO_UPDATE_ALL_FILTER_MATCHES_ON_MEDIA_STRING "\
#---------------------------------------------------------------------\n\
# No filesets on the media could be used to update the currently\n\
# installed software.\n\
#\n\
# Either the software is already at the same level as on the media, or\n\
# the media contains only filesets which are not currently installed.\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_ALREADY_INSTALLED "\
 @  Currently installed \n"
$
$ The symbol @ will be placed next to the filesets in the listing
$ that are already installed.
$

INU_SM_INST_COMMON_HEADER "\
#---------------------------------------------------------------------\n\
# + = No license password required\n\
# $ = License found\n\
# ! = No license found\n\
# @ = Currently installed\n\
# \n\
# The \"all_licensed\" filter will include filesets with \"+\" and \"$\".\n\
#---------------------------------------------------------------------\n"

INU_SM_INST_BUNDLE_HEADER "\
#---------------------------------------------------------------------\n\
# + = No license password required\n\
# $ = License found\n\
# ! = No license found\n\
# @ = Currently installed\n\
# \n\
# Only filesets with \"+\" and \"$\" will be selected.\n\
#---------------------------------------------------------------------\n"
$
