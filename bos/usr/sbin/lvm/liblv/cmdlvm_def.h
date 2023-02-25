/* @(#)77	1.31.2.20  src/bos/usr/sbin/lvm/liblv/cmdlvm_def.h, cmdlvm, bos411, 9430C411a 7/20/94 09:23:40 */
/*
 *   COMPONENT_NAME: CMDLVM
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define MSG_VGDELETED_LVM \
"\
%1$s: Volume Group deleted since it contains no physical volumes.\n"
#define MSG_BADVERSION_LVM \
"\
0516-002 %1$s: The volume group version is incompatible with this level\n\
\tof the operating system and cannot be activated.\n"
#define MSG_NOKMID_LVM \
"\
0516-003 %1$s: The kernel module id cannot be obtained.\n"
#define MSG_MAPFBSY_LVM \
"\
0516-004 %1$s: The mapped file is currently being used\n\
\tby another process.\n"
#define MSG_ALRDYMEM_LVM \
"\
0516-005 %1$s: The physical volume is already a member of the\n\
\trequested volume group.\n"
#define MSG_CHKVVGOUT_LVM \
"\
0516-006 %1$s: Check the status report. At least one\n\
\tof the physical volumes does not belong to the\n\
\tvolume group or at least one of the physical\n\
\tvolumes was removed, was not configured or could\n\
\tnot be opened. Run diagnostics.\n"
#define MSG_REMRET_INCOMP_LVM \
"\
0516-007 %1$s: The physical volume state was updated\n\
\tin the descriptor area but not in the kernel.\n"
#define MSG_UNKNERR_LVM \
"\
0516-008 %1$s: LVM system call returned an unknown\n\
\terror code (%2$s).\n"
#define MSG_VONOTHMAJ_LVM \
"\
0516-023 %1$s: The volume group is currently varied on\n\
\tat a different major number.\n"
#define MSG_MIGRATE_FAIL_LVM \
"\
0516-009 %1$s: The migrate failed due to an error during\n\
\tthe resync phase.\n"
#define MSG_OFFLINE_LVM \
"\
0516-010 %1$s: Volume group must be varied on; use varyonvg command.\n"
#define MSG_FORCEOFF_LVM \
"\
0516-011 %1$s: The volume group has been forcefully varied off\n\
\tdue to a loss of quorum.\n"
#define MSG_LVOPEN_LVM \
"\
0516-012 %1$s: Logical volume must be closed.  If the logical\n\
\tvolume contains a filesystem, the umount command will close\n\
\tthe LV device.\n"
#define MSG_NOVGDAS_LVM \
"\
0516-013 %1$s: The volume group cannot be varied on because\n\
\tthere are no good copies of the descriptor area.\n"
#define MSG_VGMEMBER_LVM \
"\
0516-014 %1$s: The physical volume appears to belong to another\n\
\tvolume group.\n"
#define MSG_PARTFND_LVM \
"\
0516-016 %1$s: Cannot delete physical volume with allocated\n\
\tpartitions. Use either migratepv to move the partitions or\n\
\treducevg with the -d option to delete the partitions.\n"
#define MSG_VGFULL_LVM \
"\
0516-018 %1$s: No more logical volumes can be created; the\n\
\tmaximum number has been reached. Use a different volume group.\n"
#define MSG_LVEXIST_LVM \
"\
0516-020 %1$s: Logical volume name or minor number already\n\
\tin use.\n"
#define MSG_INVAL_PARAM_LVM \
"\
0516-022 %1$s: Illegal parameter or structure value.\n"
#define MSG_PVOPNERR_LVM \
"\
0516-024 %1$s: Unable to open physical volume.\n\
\tEither PV was not configured or could not be opened. Run\n\
\tdiagnostics.\n"
#define MSG_NOALLOCLP_LVM \
"\
0516-026 %1$s: Cannot add more copies to a logical partition.\n"
#define MSG_MAPFOPN_LVM \
"\
0516-028 %1$s: Unable to open internal map file.\n"
#define MSG_MEMACTVVG_LVM \
"\
0516-029 %1$s: The Physical Volume is a member of a currently\n\
\tvaried on Volume Group and this cannot be overidden.\n"
#define MSG_LPNUM_INVAL_LVM \
"\
0516-030 %1$s: Illegal logical partition number.\n"
#define MSG_PPNUM_INVAL_LVM \
"\
0516-032 %1$s: Illegal physical partition number.\n"
#define MSG_DALVOPN_LVM \
"\
0516-034 %1$s: Unable to access volume group device. Execute\n\
\tvaryoffvg followed by varyonvg -m1 to build new environment.\n"
#define MSG_INVAL_MIN_NUM_LVM \
"\
0516-038 %1$s: Illegal minor number used.\n"
#define MSG_PVDAREAD_LVM \
"\
0516-040 %1$s: Unable to read the specified physical volume\n\
\tdescriptor area.\n"
#define MSG_PVSTATE_INVAL_LVM \
"\
0516-042 %1$s: Cannot allocate partitions on a physical volume\n\
\twhich has allocation disabled. Use the chpv command to enable\n\
\tallocation.\n"
#define MSG_MAPFRDWR_LVM \
"\
0516-044 %1$s: The write to internal map file failed. Confirm\n\
\tspace is available under the filesystem directory /etc then\n\
\texecute varyonvg again.\n"
#define MSG_NODELLV_LVM \
"\
0516-046 %1$s: Cannot delete logical volume before partitions removed.\n"
#define MSG_PVMAXERR_LVM \
"\
0516-048 %1$s: Maximum number of physical volumes allowed by LVM is\n\
\talready used. Add physical volume to a different volume group.\n"
#define MSG_VGDASPACE_LVM \
"\
0516-050 %1$s: Not enough descriptor area space left in this volume\n\
\tgroup. Either try adding a smaller PV or use another volume group.\n"
#define MSG_NOQUORUM_LVM \
"\
0516-052 %1$s: Volume group cannot be varied on without a\n\
\tquorum. More physical volumes in the group must be active.\n\
\tRun diagnostics on inactive PVs.\n"
#define MSG_MISSPVNAME_LVM \
"\
0516-054 %1$s: The volume group is not varied on because a\n\
\tphysical volume name is missing.\n"
#define MSG_MISSINGPV_LVM \
"\
0516-056 %1$s: The volume group is not varied on because a\n\
\tphysical volume is marked missing. Run diagnostics.\n"
#define MSG_ALLOCERR_LVM \
"\
0516-058 %1$s: Not enough memory available now. Try again later\n"
#define MSG_RDPVID_LVM \
"\
0516-059 %1$s: Unable to read physical volume ID or the physical\n\
\tvolume may not have an ID.\n"
#define MSG_LVMRECERR_LVM \
"\
0516-062 %1$s: Unable to read or write logical volume manager\n\
\trecord. PV may be permanently corrupted. Run diagnostics\n"
#define MSG_WRTDAERR_LVM \
"\
0516-064 %1$s: Unable to write the volume group descriptor area\n\
\tor status area. Check the state of the physical volume.\n\
Run diagnostics if necessary.\n"
#define MSG_NOTVGMEM_LVM \
"\
0516-066 %1$s: Physical volume is not a volume group member.\n\
\tCheck the physical volume name specified.\n"
#define MSG_NOTSYNCED_LVM \
"\
0516-068 %1$s: Unable to completely resynchronize volume. Run\n\
\tdiagnostics if neccessary.\n"
#define MSG_PROGERR_LVM \
"\
0516-070 %1$s: LVM system call found an unaccountable\n\
\tinternal error.\n"
#define MSG_BADBBDIR_LVM \
"\
0516-072 %1$s: Bad-block directory is corrupted. The physical\n\
\tvolume data has been permanently damaged. Run diagnostics.\n"
#define MSG_INRESYNC_LVM \
"\
0516-074 %1$s: Cannot extend logical partitions being\n\
\tsynchronized. Try again after synchronization is complete.\n"
#define MSG_INVLPRED_LVM \
"\
0516-076 %1$s: Cannot remove last good copy of stale partition.\n\
\tResynchronize the partitions with syncvg and try again.\n"
#define MSG_INVCONFIG_LVM \
"\
0516-078 %1$s: Incomplete device driver configuration. Probable\n\
\tcause is the special file for the VG is missing. Execute\n\
\tvaryoffvg followed by varyonvg -m1 to build correct environment.\n"
#define MSG_NOTCHARDEV_LVM \
"\
0516-080 %1$s: Device is block device; must be raw device. Execute\n\
\tvaryoffvg followed by varyonvg -m1 to build correct environment.\n"
#define MSG_INV_DEVENT_LVM \
"\
0516-082 %1$s: Unable to access a special device file.\n\
\tExecute varyoffvg followed by varyonvg -m1 to build correct\n\
\tenvironment.\n"
#define MSG_BELOW_QRMCNT_LVM \
"\
0516-084 %1$s: Cannot remove or delete physical volume\n\
\twithout losing quorum.\n"
#define MSG_VGDA_BB_LVM \
"\
0516-086 %1$s: Volume group descriptor area has a bad block.\n"
#define MSG_MISSING_LVID \
"\
0516-102 %1$s: LV identifier not entered.\n"
#define MSG_BAD_LVID \
"\
0516-104 %1$s: Incorrect LV identifier. Execute varyoffvg to close\n\
\tvolume, then open it again with varyonvg using -m1 option for\n\
\tpossible fix.\n"
#define MSG_MISSING_VGID \
"\
0516-106 %1$s: VG identifier not entered.\n"
#define MSG_MISSING_PVID \
"\
0516-108 %1$s: PV identifier not entered.\n"
#define MSG_MISSING_LVNM \
"\
0516-110 %1$s: LV name not entered.\n"
#define MSG_MISSING_MINOR \
"\
0516-112 %1$s: LV minor number not entered.\n"
#define MSG_MISSING_MAJ \
"\
0516-114 %1$s: VG major number not entered.\n"
#define MSG_MISSING_PVNM \
"\
0516-116 %1$s: PV name not entered.\n"
#define MSG_MISSING_MAXLV \
"\
0516-118 %1$s: Maximum number of LVs not entered.\n"
#define MSG_MISSING_DASZ \
"\
0516-120 %1$s: Size of VG descriptor area not entered.\n"
#define MSG_MISSING_PPSZ \
"\
0516-122 %1$s: Size of physical partitions not entered.\n"
#define MSG_MISSING_ESZ \
"\
0516-123 %1$s: Size not entered.\n"
#define MSG_MISSING_MID \
"\
0516-124 %1$s: Error getting Kernel module identifier.\n"
#define MSG_MISSING_FN \
"\
0516-126 %1$s: File name not entered.\n"
#define MSG_MISSING_MAP \
"\
0516-128 %1$s: Unable to read input file map entries.\n"
#define MSG_EXPECT_MAP \
"\
0516-130 %1$s: Incorrect input; expecting more entries in map file.\n"
#define MSG_EXPECT_INPT \
"\
0516-132 %1$s: Incorrect entry in partition map file.\n"
#define MSG_RECEIVED \
"\
0516-134 %1$s: Received: %2$s\n"
#define MSG_RDERR_MAP \
"\
0516-136 %1$s: Error reading input map.\n"
#define MSG_MISSING_VGNAME \
"\
0516-138 %1$s: VG name not entered.\n"
#define MSG_CHLV_USAGE \
"\
Usage: %1$s -l LVid [-s MaxPartitions] [-n LVname] [-M SchedulePolicy]\n\
\t[-p Permissions] [-r BadBlocks] [-v WriteVerify] [-w mirwrt_consist].\n"
#define MSG_CHPV_USAGE \
"\
Usage: %1$s -g VGid -p PVid [-r RemoveMode] [-a AllocateMode]\n"
#define MSG_CRLV_USAGE \
"\
Usage: %1$s -N LVname -g VGid -n MinorNumber [ -M MirrorPolicy]\n\
\t[-s MaxLPs] [-p Permissions] [ -r Badblocks] [-v WriteVerify]\n\
\t[-w mirwrt_consist]\n"
#define MSG_CRVG_USAGE \
"\
Usage: %1$s -a VGname -V MajorNumber -N PVname -n MaxLVs\n\
\t-D VGDescriptorSize -s PPSize [-f] [-t]\n"
#define MSG_DLV_USAGE \
"\
Usage: %1$s -l LVid\n"
#define MSG_DPV_USAGE \
"\
Usage: %1$s -g VGid -p PVid\n"
#define MSG_EXLV_USAGE \
"\
Usage: %1$s -l LVid -s Size Filename\n"
#define MSG_MISSING_SZ \
"\
0516-148 %1$s: Size not entered.\n"
#define MSG_IPV_USAGE \
"\
Usage: %1$s -N PVname -g VGid [-f]\n"
#define MSG_MPP_USAGE \
"\
Usage: %1$s -g VGid -p SourcePVid -n SourcePPnumber\n\
\t-P DestinationPVid -N DestinationPPnumber\n"
#define MSG_MISSING_OPVID \
"\
0516-152 %1$s: Source PV identifier not entered.\n"
#define MSG_MISSING_OPPN \
"\
0516-154 %1$s: Source physical partition number not entered.\n"
#define MSG_MISSING_NPVID \
"\
0516-156 %1$s: Destination PV identifier not entered.\n"
#define MSG_MISSING_NPPN \
"\
0516-158 %1$s: Destination physical partition number not entered.\n"
#define MSG_QLV_USAGE \
"\
Usage: %1$s -L LVid [-p PVname] [-NGnMScsPRvoadlArtwb].\n"
#define MSG_QPV_USAGE \
"\
Usage: %1$s -p PVid [-g VGid | -N PVname] [-scPnaDdAt]\n"
#define MSG_MISSING_VGPV \
"\
0516-162 %1$s: VG identifier or PV name not entered.\n"
#define MSG_QVG_USAGE \
"\
Usage: %1$s [-g VGid | -p PVname] [-NsFncDaLPAvt]\n"
#define MSG_QVGS_USAGE \
"\
Usage: %1$s [-NGAt]\n"
#define MSG_RLV_USAGE \
"\
Usage: %1$s -l LVid -s Size Filename\n"
#define MSG_RSLP_USAGE \
"\
Usage: %1$s -l LVid -n LPnumber\n"
#define MSG_MISSING_LPN \
"\
0516-170 %1$s: Logical partition number not entered.\n"
#define MSG_RSLV_USAGE \
"\
Usage: %1$s -l LVid\n"
#define MSG_RSPV_USAGE \
"\
Usage: %1$s -g VGid -p PVid\n"
#define MSG_VOFF_USAGE \
"\
Usage: %1$s -g VGid [-f]\n"
#define MSG_VON_USAGE \
"\
Usage: %1$s -a VGname -V MajorNumber -g VGid\n\
\t[-ornpft] Filename\n"
#define MSG_VON_SUCCESS \
"\
%1$s: Volume group successfully varied on.\n"
#define MSG_GETODM_USE \
"\
Usage: %1$s [-a LVdescript] [-B LVdesrcript] [-b LVid] [-c LVid]\n\
\t[-C] [-d VGdescript] [-e LVid] [-F] [-g PVid] [-h] [-j PVdescript]\n\
\t[-k] [-L VGdescript] [-l LVdescript] [-m LVid] [-p PVdescript]\n\
\t[-r LVid] [-s VGdescript] [-t VGid] [-u VGdescript] [-v VGdescript]\n\
\t[-w VGid] [-y LVid] [-G LVdescript]\n"
#define MSG_BAD_VGID \
"\
0516-302 %1$s: The identifier format is incorrect.\n"
#define MSG_ID_NOT_FOUND \
"\
0516-304 %1$s: Unable to find device id %2$s in the Device\n\
\tConfiguration Database.\n"
#define MSG_DEV_NOT_FOUND \
"\
0516-306 %1$s: Unable to find %2$s %3$s in the Device\n\
\tConfiguration Database.\n"
#define MSG_ODM_DOWN \
"\
0516-307 %1$s: Unable to access Device Configuration\n\
\tDatabase.\n"
#define MSG_CH_NOT_FOUND \
"\
0516-308 %1$s: Unable to find logical volume attribute %2$s\n\
\tin the Device Configuration Database.\n"
#define MSG_ATTR_NOT_FOUND \
"\
0516-310 %1$s: Unable to find attribute %2$s in the Device\n\
\tConfiguration Database. Execute varyoffvg followed by varyonvg\n\
\twith -m1 option to attempt to correct the LV database.\n"
#define MSG_LV_NOT_FOUND \
"\
0516-312 %1$s: Unable to find logical volume %2$s in the Device\n\
\tConfiguration Database.\n"
#define MSG_NO_FREE_PVS \
"\
0516-313 %1$s: No configured physical volumes are free.\n"
#define MSG_LV_NOT_FOUND2 \
"\
0516-314 %1$s: Unable to find logical volume %2$s in the Device\n\
\tConfiguration Database.\n"
#define MSG_VG_NOT_FOUND \
"\
0516-316 %1$s: Unable to find volume group %2$s in the Device\n\
\tConfiguration Database.\n"
#define MSG_NO_VGS_NOW \
"\
0516-318 %1$s: No volume groups found.\n"
#define MSG_PV_NOT_IN_VG \
"\
0516-320 %1$s: Physical volume %2$s is not assigned to\n\
\ta volume group.\n"
#define MSG_PV_NOT_CONF \
"\
0516-321 %1$s: Physical volume %2$s is not configured.\n"
#define MSG_PVS_NOT_FOUND \
"\
0516-322 %1$s: The Device Configuration Database is inconsistent.\n\
\tExecute varyoffvg followed by varyonvg with -m1 option to\n\
\tbuild new LV database.\n"
#define MSG_NO_PVS_ON_VG \
"\
0516-324 %1$s: Unable to find PV identifier for physical volume.\n\
\tThe Device Configuration Database is inconsistent for physical\n\
\tvolumes.\n"
#define MSG_PUTODM_USE \
"\
Usage:\n\
\t%1$s [-a IntraPolicy] [-B label] [-c Copies] [-e InterPolicy]\n\
\t\t[-L LVid] [-l LVname] [-n NewLVName] [-r Relocate] [-s Strict]\n\
\t\t[-t Type] [-u UpperBound] [-y Copyflag]\n\
\t\t[-z Size] [-S StripeSize] LVid\n\
\tputlvodm [-o Auto-on] [-k] [-K] [-q VGstate]\n\
\t\t[-v VGname -m majornum] [-V] VGid\n\
\tputlvodm [-p VGid] [-P] PVid\n"
#define MSG_MISSING_ID \
"\
0516-328 %1$s: Identifier not entered.\n"
#define MSG_BAD_FLAG \
"\
0516-330 %1$s: Do not use -L with any other flag.\n"
#define MSG_BAD_FLAG2 \
"\
0516-332 %1$s: Do not use -V with any other flag.\n"
#define MSG_MAJ_NUM_MISSING \
"\
0516-334 %1$s: Major number not entered.\n"
#define MSG_NAME_TRUNC \
"\
0516-335 %1$s: Warning, name has been truncated to 15 characters.\n"
#define MSG_LVNAME_USE \
"\
Usage: %1$s [-Y Prefix] [-n LVname] [Type]"
#define MSG_SEQ_ERROR \
"\
0516-336 %1$s: The gensequence function failed.\n"
#define MSG_VGNAME_USE \
"\
Usage: %1$s [-n VGname]\n"
#define MSG_GENMAJ_USE \
"\
Usage: %1$s VGname\n"
#define MSG_GENMAJ_ERROR \
"\
0516-342 %1$s: The genmajor function failed.\n"
#define MSG_GENMIN_USE \
"\
Usage: %1$s [-p PreferredNumber] MajorNumber NewDeviceName\n"
#define MSG_MISSING_PARMS \
"\
0516-346 %1$s: Major number and logical volume name not entered.\n"
#define MSG_MISSING_MAJNUM \
"\
0516-348 %1$s: Major number not entered.\n"
#define MSG_GENMINOR_ERR \
"\
0516-350 %1$s: The genminor function failed.\n"
#define MSG_RELSEQ_ERR \
"\
0516-352 %1$s: Warning, the reldevno function failed.\n"
#define MSG_RELMIN_USE \
"\
Usage: %1$s Name\n"
#define MSG_RELMINOR_ERR \
"\
0516-356 %1$s: Warning, the reldevno function failed.\n"
#define MSG_PREFIX_EXISTS \
"\
0516-358 %1$s: Illegal name; the prefix of the name is reserved.\n\
\tChoose a different name.\n"
#define MSG_NAME_EXISTS \
"\
0516-360 %1$s: The device name is already used; choose a\n\
\tdifferent name.\n"
#define MSG_LVODM_ERROR \
"\
0516-362 %1$s: Unknown Object Data Manager error: %2$d.\n"
#define MSG_ATTR_NOT_ADDED \
"\
0516-364 %1$s: Attribute %2$s not found in Device Configuration\n\
\tDatabase. The database is inconsistent.\n"
#define MSG_VG_LOCKED \
"\
0516-366 %1$s: Volume group %2$s is locked. Try again.\n"
#define MSG_REL_MIN_FAIL \
"\
0516-368 %1$s: The reldevno function failed; name not changed.\n"
#define MSG_GEN_MIN_FAIL \
"\
0516-370 %1$s: The genminor function failed; name not changed.\n"
#define MSG_GET_KMID_FAIL \
"\
0516-371 %1$s: The loadext function failed. Unable to get kernel\n\
\tmodule identifier.\n"
#define MSG_CHKMAJ_ERROR \
"\
0516-372 %1$s: The chkmajor function failed.\n"
#define MSG_CHKMAJ_PARMS \
"\
0516-373 %1$s: Major number and volume group name not entered.\n"
#define MSG_CHKMAJ_USE \
"\
Usage: %1$s Majornumber VGname\n"
#define MSG_LSTMAJ_ERROR \
"\
0516-374 %1$s: The lstmajor function failed.\n"
#define MSG_LSTMAJ_USE \
"\
Usage: %1$s\n"
#define MSG_ALLOCP_MAP_NOFREE \
"\
0516-400 %1$s: Not enough free partitions in map file.\n"
#define MSG_ALLOCP_SCANFAIL \
"\
0516-402 %1$s: Incorrect data encountered during scan function.\n"
#define MSG_ALLOCP_NORESOURCE \
"\
0516-404 %1$s: Not enough resources available to fulfill\n\
\tallocation. Either not enough free partitions or not enough\n\
\tphysical volumes to keep strictness. May be caused by no free\n\
\tpartitions in /tmp.  Try again with different allocation \n\
\tcharacteristics.\n"
#define MSG_ALLOCP_STRICTERROR \
"\
0516-406 %1$s: Unable to maintain strictness.\n"
#define MSG_ALLOCP_PARSELVID \
"\
0516-408 %1$s: Illegal logical volume identifier value.\n"
#define MSG_ALLOCP_PARSECOPY \
"\
0516-410 %1$s: Illegal Copy value.\n"
#define MSG_ALLOCP_PARSESIZE \
"\
0516-412 %1$s: Illegal Size value.\n"
#define MSG_ALLOCP_PARSEBOUND \
"\
0516-414 %1$s: Illegal UpperBound value.\n"
#define MSG_ALLOCP_PARMCOMB \
"\
0516-416 %1$s: Illegal combination of command line flags.\n"
#define MSG_ALLOCP_LINERROR \
"\
0516-418 %1$s: Illegal data on standard input map line %2$d.\n"
#define MSG_ALLOCP_INTRAERROR \
"\
0516-420 %1$s: Illegal InterPolicy value, defaulted to middle.\n"
#define MSG_ALLOCP_USAGE \
"\
0516-422 %1$s: [-i LVid] [-t Type] [-c Copies] [-s Size]\n\
\t[-k] [-u UpperBound>] [-e InterPolicy] [-a InterPolicy]\n"
#define MSG_ALLOCP_MAPFILELOW \
"\
0516-423 %1$s: Not enough entries in mapfile to fulfill allocation. \n"
#define MSG_500 \
"Usage: %1$s VGname\n"
#define MSG_502 \
"0516-502 %1$s: Unable to access volume group %2$s.\n"
#define MSG_504 \
"0516-504 %1$s: Unable to access device configuration database.\n"
#define MSG_506 \
"0516-506 %1$s: Unable to find any physical volumes.\n"
#define MSG_508 \
"0516-508 %1$s: Warning, unable to update device configuration\n\
\tdatabase for physical volume %2$s.\n"
#define MSG_510 \
"0516-510 %1$s: Physical volume not found for physical volume\n\
\tidentifier %2$s.\n"
#define MSG_520 \
"0516-520 %1$s: Partially successful with updating logical\n\
\tvolume %2$s.\n"
#define MSG_521 \
"0516-521 %1$s: Warning, using device configuration database\n\
\tdefault values for logical volume %2$s.\n"
#define MSG_522 \
"0516-522 %1$s: Unable to update logical volume %2$s.\n"
#define MSG_524 \
"Usage: %1$s LVname VGname\n"
#define MSG_526 \
"0516-526 %1$s: Logical volume and volume group names not entered.\n"
#define MSG_528 \
"0516-528 %1$s: Unable to access device configuration database.\n"
#define MSG_530 \
"0516-530 %1$s: Logical volume name %2$s changed to %3$s.\n"
#define MSG_532 \
"0516-532 %1$s: Logical volume %2$s not found in volume group %3$s.\n"
#define MSG_540 \
"Usage: %1$s [-v] VGname [LVname...]\n"
#define MSG_542 \
"0516-542 %1$s: Unable to continue. Command terminated.\n"
#define MSG_544 \
"0516-544 %1$s: Unable to access volume group %2$s.\n"
#define MSG_546 \
"0516-546 %1$s: Unable to update volume group %2$s.\n\
\tCommand terminated\n"
#define MSG_548 \
"0516-548 %1$s: Partially successful with updating volume\n\
\tgroup %2$s.\n"
#define MSG_550 \
"%1$s: Physical volume data updated.\n"
#define MSG_552 \
"%1$s: No logical volumes in volume group %2$s.\n"
#define MSG_556 \
"0516-556 %1$s: Unable to update logical volume %2$s.\n"
#define MSG_558 \
"%1$s: Logical volume %2$s updated.\n"
#define MSG_560 \
"Usage: %1$s {-d PVname | -i VGid} [-V MajorNumber] VGname\n"
#define MSG_562 \
"0516-562 %1$s: Unable to access physical volume %2$s\n"
#define MSG_564 \
"0516-564 %1$s: Unable to access device configuration database.\n"
#define MSG_566 \
"0516-566 %1$s: Unable to find configured physical volumes\n\
\tin device configuration database.\n"
#define MSG_567 \
"0516-567 %1$s: No configured physical volumes were found\n\
\tfor the volume group.\n"
#define MSG_568 \
"0516-568 %1$s: Unable to get major number.\n"
#define MSG_570 \
"0516-570 %1$s: Unable to get minor number.\n"
#define MSG_572 \
"0516-572 %1$s: Volume Group already exists under name %2$s.\n"
#define MSG_574 \
"0516-574 %1$s: Volume Group name %2$s is already used.\n"
#define MSG_576 \
"0516-576 %1$s: Unable to update device configuration database.\n"
#define MSG_602 \
"0516-602 %1$s: Logical volume name not entered.\n"
#define MSG_604 \
"0516-604 %1$s: Physical volume name not entered.\n"
#define MSG_606 \
"0516-606 %1$s: Volume group name not entered.\n"
#define MSG_608 \
"0516-608 %1$s: New logical volume name not entered.\n"
#define MSG_610 \
"0516-610 %1$s: Number of logical partitions not entered.\n"
#define MSG_612 \
"0516-612 %1$s: Source physical volume name not entered.\n"
#define MSG_613 \
"0516-613 %1$s: Source logical volume name not entered.\n"
#define MSG_614 \
"0516-614 %1$s: Destination physical volume name not entered.\n"
#define MSG_615 \
"0516-615 %1$s: Destination logical volume name not entered.\n"
#define MSG_616 \
"0516-616 %1$s: Number of logical volume copies not entered.\n"
#define MSG_618 \
"0516-618 %1$s: Name not entered.\n"
#define MSG_620 \
"0516-620 %1$s: Warning, cannot update device configuration\n\
\tdatabase. Execute varyonvg with -m1 option to synchronize the\n\
\tdatabase.\n"
#define MSG_621 \
"0516-621 %1$s: The Device Configuration Database is inconsistent.\n\
\tExecute exportvg %2$s followed by importvg to synchronize\n\
\tthe database.\n"
#define MSG_622 \
"0516-622 %1$s: Warning, cannot write lv control block data.\n"
#define MSG_624 \
"0516-624 %1$s: Warning, cannot update device configuration\n\
\tdatabase for volume group. Execute varyonvg with -m1 option to\n\
\tsynchronize the database.\n"
#define MSG_626 \
"0516-626 %1$s: Warning, cannot update device configuration\n\
\tdatabase for physical volume %2$s in volume group %3$s. Execute\n\
\tvaryonvg with -m1 option to synchronize the database.\n"
#define MSG_628 \
"0516-628 %1$s: Warning, cannot install physical volume %2$s\n\
\tin volume group %3$s.\n"
#define MSG_630 \
"0516-630 %1$s: Warning, cannot update device configuration\n\
\tdatabase for logical volume %2$s. Execute varyonvg with -m1\n\
\toption to synchronize the database.\n"
#define MSG_631 \
"0516-631 %1$s: Warning, all data belonging to physical\n\
\tvolume %2$s will be destroyed.\n"
#define MSG_632 \
"%1$s: Do you wish to continue? y(es) n(o)? "
#define MSG_633 \
"0516-633 %1$s: Logical partitions have stale copies.\n\
\tBefore %2$s either use syncvg to synchronize the stale\n\
\tcopies or use rmlvcopy to remove them.\n"
#define MSG_634 \
"0516-634 %1$s: /tmp directory does not have enough space,\n\
\tdelete some files and try again.\n"
#define MSG_635 \
"0516-635 %1$s: The logical volume manager mirror\n\
\tfunctions will be made available in the fourth quarter of\n\
\t1990 to current licensees via a Program Temporary Fix (PTF)\n\
\tdistribution.\n"
#define MSG_636 \
"0516-636 %1$s: The logical volume manager mirror\n\
\tfunctions will be made available in the fourth quarter of\n\
\t1990 to current licensees via a Program Temporary Fix (PTF)\n\
\tdistribution.\n"
#define MSG_637 \
"Volume group identifier"
#define MSG_638 \
"Auto varyon"
#define MSG_639 \
"Type description"
#define MSG_640 \
"Information label"
#define MSG_641 \
"Policy for allocation on physical volume"
#define MSG_642 \
"Number of physical partitions for each logical partition"
#define MSG_643 \
"Policy for allocation over multiple physical volumes"
#define MSG_644 \
"Bad block relocatable policy"
#define MSG_645 \
"Copies allocation policy"
#define MSG_646 \
"Number of physical volumes to allocate across"
#define MSG_647 \
"Number of logical partitions in logical volume"
#define MSG_648 \
"Volume group identifier . minor number"
#define MSG_649 \
"Logical volume type and prefix"
#define MSG_650 \
"Prefix used for unknown type"
#define MSG_651 \
"Default volume group prefix"
#define MSG_652 \
"0516-652 %1$s: The -a parameter for IntraPolicy must be m, im, c,\n\
\tie or e.\n"
#define MSG_654 \
"0516-654 %1$s: The -e parameter for InterPolicy must be x or m.\n"
#define MSG_656 \
"0516-656 %1$s: The -u parameter for Upperbound must be\n\
\tbetween 1 and 32.\n"
#define MSG_658 \
"0516-658 %1$s: The -s parameter for Strict must be y or n.\n"
#define MSG_660 \
"0516-660 %1$s: The -b parameter for BadBlocks must be y or n.\n"
#define MSG_662 \
"0516-662 %1$s: The -c parameter for Copies must be 1, 2, or 3.\n"
#define MSG_664 \
"0516-664 %1$s: The -d parameter for Schedule must be p or s.\n"
#define MSG_666 \
"0516-666 %1$s: The -p parameter for Permission must be w or r.\n"
#define MSG_668 \
"0516-668 %1$s: The -x parameter for Maximum must be between 1 and\n\
\t32,512. It cannot be smaller than the existing number of LP's.\n"
#define MSG_670 \
"0516-670 %1$s: The -v parameter for Availability must be a or r.\n"
#define MSG_672 \
"0516-672 %1$s: The -r parameter for Relocate must be y or n.\n"
#define MSG_674 \
"0516-674 %1$s: The -a parameter for Allocation must be y or n.\n"
#define MSG_676 \
"0516-676 %1$s: The -v parameter for Verify must be y or n.\n"
#define MSG_678 \
"0516-678 %1$s: The -a parameter for Auto-on must be y or n.\n"
#define MSG_680 \
"0516-680 %1$s: Unable to read Mapfile specified as the -m parameter.\n"
#define MSG_681 \
"0516-681 %1$s: Name provided is of illegal length or content.\n"
#define MSG_682 \
"0516-682 %1$s: The -s parameter for Size must be a power\n\
\tof 2 between 1 and 256.\n"
#define MSG_684 \
"0516-684 %1$s: The Copies parameter must be 1, 2, or 3.\n"
#define MSG_686 \
"0516-686 %1$s: The -m parameter for Synchronize must be 1 or 2.\n"
#define MSG_688 \
"0516-688 %1$s: The -y parameter for Name cannot be used\n\
\twith the -Y parameter for Prefix.\n"
#define MSG_690 \
"0516-690 %1$s: The -a, -e, -u, -s, and -c options cannot be\n\
\tused with the -m option.\n"
#define MSG_692 \
"0516-692 %1$s: The -f option can only be used with the -e option.\n"
#define MSG_693 \
"0516-693 %1$s: The -%2$s parameter is of illegal length or content.\n"
#define MSG_694 \
"0516-694 %1$s: The -v, -y, and -Y options cannot be used with\n\
\tthe -e option.\n"
#define MSG_695 \
"0516-695 %1$s: The -y option cannot be used with the -Y option.\n"
#define MSG_696 \
"0516-696 %1$s: Physical Volume %2$s belongs to another volume group.\n"
#define MSG_697 \
"0516-697 %1$s: Number of copies must be specified (1 or 2).\n"
#define MSG_698 \
"Volume group"
#define MSG_699 \
"Logical volume"
#define MSG_702 \
"Usage:\n\
\t%1$s -n NewLVname LVname\n\
\t%2$s [-a IntraPolicy] [-e InterPolicy] [-L Label] [-u UpperBound]\n\
\t\t[-s Strict] [-b BadBlocks] [-d Schedule] [-p Permission]\n\
\t\t[-r Relocate] [-t Type] [-v Verify] [-x MaxLPs] LVname...\n\
Changes the characteristics of a logical volume.\n"
#define MSG_704 \
"0516-704 %1$s: Unable to change logical volume %2$s.\n"
#define MSG_706 \
"0516-706 %1$s: Do not use -n flag with any other flag.\n"
#define MSG_708 \
"0516-708 %1$s: Cannot rename open logical volume. If LV\n\
\tcontains a filesystem, unmount to close logical volume %2$s.\n"
#define MSG_710 \
"0516-710 %1$s: mknod function failure. Cannot create new\n\
\tdevice special file %2$s.\n"
#define MSG_711 \
"0516-711 %1$s: Warning, unable to update logical volume\n\
\tcontrol block.\n"
#define MSG_712 \
"0516-712 %1$s: The chlv succeeded, however chfs must now be \n\
\trun on every filesystem which references the old log name %2$s.\n"
#define MSG_720 \
"Usage: %1$s { -a Allocation | -v Availability } PVname...\n\
Changes the characteristics of a physical volume.\n"
#define MSG_722 \
"0516-722 %1$s: Unable to change physical volume %2$s.\n"
#define MSG_724 \
"0516-724 %1$s: Unable to change physical volume %2$s.\n\
\tThere is an open logical volume.\n"
#define MSG_730 \
"Usage: %1$s [-a Auto on] VGname...\n\
Changes the characteristics of a volume group.\n"
#define MSG_732 \
"0516-732 %1$s: Unable to change volume group %2$s.\n"
#define MSG_740 \
"Usage: %1$s [-v VGname] [-y NewName | -Y Prefix] SourceLV\n\
       %1$s -e [-f] DestinationLV SourceLV.\n\
Copies a logical volume.\n"
#define MSG_741 \
"0516-741 %1$s: Source logical volume %2$s is open.\n\
\tUnmount file system on this logical volume.\n"
#define MSG_742 \
"0516-742 %1$s: Unable to copy logical volume %2$s.\n"
#define MSG_743 \
"0516-743 %1$s: Error occurred during data copy.\n"
#define MSG_744 \
"%1$s: All data contained on the destination logical\n\
\tvolume will be destroyed.\n"
#define MSG_746 \
"0516-746 %1$s: Destination logical volume must have\n\
\ttype set to copy.\n"
#define MSG_747 \
"%1$s: Use chlv command with -t flag to change the type to copy.\n"
#define MSG_748 \
"0516-748 %1$s: Warning, copying to a smaller logical volume -- \n\
\tmay lose data.  Also, could corrupt any existing filesystem.\n"
#define MSG_750 \
"0516-750 %1$s: Warning, cannot verify destination logical\n\
\tvolume is large enough; logical volume might be reduced; this\n\
\tcould corrupt any residing filesystem.\n"
#define MSG_752 \
"0516-752 %1$s: Warning, cannot update destination logical\n\
\tvolume's type in device configuration database.\n"
#define MSG_753 \
"0516-753 %1$s: Warning, cannot update destination logical\n\
\tvolume's label in device configuration database.\n"
#define MSG_754 \
"%1$s: Logical volume %2$s successfully copied to %3$s .\n"
#define MSG_760 \
"Usage: %1$s VGname\n\
Exports the definition of a volume group.\n"
#define MSG_762 \
"0516-762 %1$s: The root volume group cannot be exported.\n"
#define MSG_764 \
"0516-764 %1$s: The volume group must be varied off\n\
\tbefore exporting.\n"
#define MSG_766 \
"0516-766 %1$s: Warning, cannot remove logical volume %2$s\n\
\tfrom device configuration database.\n"
#define MSG_768 \
"0516-768 %1$s: Warning, cannot remove physical volume %2$s\n\
\tfrom the LVM device configuration database.\n"
#define MSG_770 \
"0516-770 %1$s: Warning, cannot remove the identifier for\n\
\tvolume group %2$s from the device configuration database.\n"
#define MSG_772 \
"0516-772 %1$s: Unable to export volume group %2$s\n"
#define MSG_774 \
"Usage: %1$s [-V MajorNumber] [-y VGname] [-f] PVname\n\
Imports the definition of a volume group.\n"
#define MSG_776 \
"0516-776 %1$s: Cannot import %2$s as %3$s.\n"
#define MSG_778 \
"0516-778 %1$s: Volume Group Name %2$s is already used.\n"
#define MSG_780 \
"0516-780 %1$s: Unable to import volume group from %2$s.\n"
#define MSG_782 \
"0516-782 %1$s: Partially successful importing of %2$s.\n"
#define MSG_785 \
"0516-785 %1$s: The Copies parameter must be 1 or 2.\n"
#define MSG_786 \
"Usage: %1$s [-a IntraPolicy] [-e InterPolicy] [-m MapFile]\n\
\t[-s Strict] [-u UpperBound] LVname NumberOfLPs [PVname...]\n\
Extends the size of a logical volume.\n"
#define MSG_787 \
"0516-787 %1$s: Maximum allocation for logical volume %2$s\n\
\tis %3$s.\n"
#define MSG_788 \
"0516-788 %1$s: Unable to extend logical volume.\n"
#define MSG_789 \
"0516-789 %1$s: Physical Volume %2$s is not in volume\n\
\tgroup %3$s.\n"
#define MSG_790 \
"Usage: %1$s [-f] VGname PVname...\n\
Extends a volume group by adding a physical volume.\n"
#define MSG_792 \
"0516-792 %1$s: Unable to extend volume group.\n"
#define MSG_794 \
"0516-794 %1$s: %2$s is not configured. Please configure\n\
\tthe disk before trying this command again.\n"
#define MSG_796 \
"0516-796 %1$s: Making %2$s a physical volume. Please wait.\n"
#define MSG_800 \
"Usage: %1$s [-i] [-l LVname] SourcePV DestinationPV...\n\
Migrates all logical partitions off a physical volume.\n"
#define MSG_804 \
"0516-804 %1$s: Source physical volume %2$s cannot be in\n\
\tdestination.\n"
#define MSG_806 \
"0516-806 %1$s: Destination physical volume %2$s belongs\n\
\tto wrong volume group.\n"
#define MSG_808 \
"0516-808 %1$s: Destination physical volume state does not\n\
\tallow allocation.\n"
#define MSG_810 \
"0516-810 %1$s: Not enough room on destination physical volumes.\n"
#define MSG_812 \
"0516-812 %1$s: Warning, migratepv did not completely succeed;\n\
\tall physical partitions have not been moved off the PV.\n"
#define MSG_814 \
"0516-814 %1$s: No other physical volumes are available for\n\
\tallocation in the volume group.\n"
#define MSG_816 \
"0516-816 %1$s: Unable to migrate %2$s:%3$s to %4$s:%5$s.\n"
#define       MSG_817 \
"0516-817 %1$s: Source physical volume %2$s state does not\n\
\tallow migration.\n"
#define MSG_818 \
"0516-818 %1$s: Striped logical volume %2$s will not be migrated.\n"
#define MSG_820 \
"Usage: %1$s [-a IntraPolicy] [-b BadBlocks] [-c Copies]\n\
\t[-d Schedule] [-e InterPolicy] [-i] [-L Label] [-m MapFile]\n\
\t[-r Relocate] [-s Strict][-t Type] [-u UpperBound]\n\
\t[-v Verify] [-x MaxLPs] [-y LVname] [-S StripeSize] [-Y Prefix]\n\
\tVGname NumberOfLPs [PVname...]\n\
Makes a logical volume.\n"
#define MSG_822 \
"0516-822 %1$s: Unable to create logical volume.\n"
#define MSG_823 \
"0516-823 %1$s: Physical Volume %2$s is not allocatable.\n"
#define MSG_824 \
"0516-824 %1$s: Unable to get allocation data for physical\n\
\tvolume %2$s.\n"
#define MSG_825 \
"0516-825 %1$s: With -i option the physical volume names\n\
\tmust come from standard input.\n"
#define MSG_828 \
"%1$s: Logical volume identifier %2$s generated.\n"
#define MSG_830 \
"0516-830 %1$s: Both volume group and number of partitions\n\
\tmust be entered.\n"
#define MSG_832 \
"0516-832 %1$s: Special file %2$s or %3$s\n\
\talready exists.\n"
#define MSG_834 \
"%1$s: Warning, physical volume %2$s is excluded since it may be\n\
\teither missing or removed.\n"
#define MSG_840 \
"Usage: %1$s [-a IntraPolicy] [-e InterPolicy]\n\
\t[-k] [-m MapFile] [-u UpperBound] [-s Strict]\n\
\tLVname LPcopies [PVname...]\n\
Makes logical partition copies for a logical volume.\n"
#define MSG_842 \
"0516-842 %1$s: Unable to make logical partition copies for\n\
\tlogical volume.\n"
#define MSG_844 \
"0516-844 %1$s: Logical partition synchronization failed.\n"
#define MSG_846 \
"%1$s: There are no physical volumes available for allocation.\n"
#define MSG_848 \
"0516-848 %1$s: Failure on physical volume %2$s, it may be missing\n\
\tor removed.\n"
#define MSG_850 \
"0516-850 %1$s: Physical volume %2$s is not available for\n\
\tallocation. Use chpv to change the allocation state.\n"
#define MSG_852 \
"0516-852 %1$s: Logical volume %2$s already has copies.\n"
#define MSG_860 \
"Usage: %1$s [-d MaxPVs] [-f] [-i] [-m MaxPVsize] [-n] [-s PPsize]\n\
\t[-V MajorNumber] [-y VGname] PVname...\n\
Makes a volume group.\n"
#define MSG_862 \
"0516-862 %1$s: Unable to create volume group.\n"
#define MSG_864 \
"%1$s: Volume group name %2$s generated.\n"
#define MSG_866 \
"%1$s: Volume group identifier %2$s generated.\n"
#define MSG_868 \
"0516-868 %1$s: The -d parameter for MaxPVs must be between 1 and 32.\n"
#define MSG_870 \
"%1$s: Volume group special file %2$s already exists.\n"
#define MSG_872 \
"%1$s: Warning, number of partitions %2$s given by the MaxPVsize and\n\
\tPPsize is larger than system limitation 1016.  1016 partitions per PV\n\
\tis used to calculate the LVM overhead space per PV.  Specify a larger\n\
\tpartition size in order to not waste space on MaxPVsize physical volume.\n"
#define MSG_874 \
"%1$s: Volume group name %2$s contains invalid characters.\n"
#define MSG_880 \
"Usage: %1$s [-d] [-f] VGname PVname...\n\
Reduces volume group size by removing a physical volume.\n"
#define MSG_882 \
"0516-882 %1$s: Unable to reduce volume group.\n"
#define MSG_884 \
"0516-884 %1$s: Unable to remove physical volume %2$s.\n"
#define MSG_886 \
"0516-886 %1$s: Physical volume %2$s does not belong to\n\
\tvolume group %3$s.\n"
#define MSG_888 \
"%1$s: Partitions from the following logical volumes will be deleted:\n"
#define MSG_894 \
"0516-894 %1$s: Warning, cannot remove volume group %2$s from\n\
\tdevice configuration database.\n"
#define MSG_896 \
"0516-896 %1$s: Warning, cannot remove physical volume %2$s from\n\
\tDevice Configuration Database.\n"
#define MSG_910 \
"Usage: %1$s [-f] [-p PVname] LVname...\n\
Removes a logical volume.\n"
#define MSG_912 \
"0516-912 %1$s: Unable to remove logical volume %2$s.\n"
#define MSG_913 \
"Warning, all data contained on logical volume %2$s will be destroyed.\n"
#define MSG_914 \
"0516-914 %1$s: Warning, all data belonging to logical volume\n\
\t%2$s on physical volume %3$s will be destroyed.\n"
#define MSG_916 \
"0516-916 %1$s: Warning, cannot remove logical volume %2$s from\n\
\tdevice configuration database. Execute varyoffvg followed by\n\
\tvaryonvg -m1 to synchronize the database.\n"
#define MSG_918 \
"%1$s: Logical volume %2$s is removed.\n"
#define MSG_920 \
"Usage: %1$s LVname LPcopies [PVname...]\n\
Leaves at most LVcopies number of physical partitions per logical partition\n"
#define MSG_921 \
"0516-921 %1$s: All logical partitions have less than or\n\
\tequal to %2$s number of copies.\n"
#define MSG_922 \
"0516-922 %1$s: Unable to remove logical partition copies from\n\
\tlogical volume %2$s.\n"
#define MSG_930 \
"Usage: %1$s [-i] {-l|-p|-v} Name\n\
Synchronize logical partition copies.\n"
#define MSG_932 \
"0516-932 %1$s: Unable to synchronize volume group %2$s.\n"
#define MSG_934 \
"0516-934 %1$s: Unable to synchronize logical volume %2$s.\n"
#define MSG_936 \
"0516-936 %1$s: Unable to synchronize physical volume %2$s.\n"
#define MSG_937 \
"0516-937 %1$s: Enter only one option.\n"
#define MSG_938 \
"0516-938 %1$s: One option must be entered.\n"
#define MSG_940 \
"Usage: %1$s [-s] VGname\n\
Varies a volume group off.\n"
#define MSG_942 \
"0516-942 %1$s: Unable to vary off volume group %2$s.\n"
#define MSG_950 \
"Usage: %1$s [-d] [-f] [-m Mode] [-n] [-s] VGname...\n\
Varies a volume group on.\n"
#define MSG_952 \
"0516-952 %1$s: Unable to vary on volume group %2$s.\n"
#define MSG_954 \
"%1$s: Volume group %2$s is varied on.\n"
#define MSG_956 \
"0516-956 %1$s: Synchronization of Device Configuration\n\
\tDatabase failed.\n"
#define MSG_957 \
"0516-957 %1$s: A physical volume is missing from the\n\
\tvolume group. Run diagnostics on the missing physical volume.\n\
\tIf you must activate the volume group to recover data,\n\
\tuse the varyonvg command with the -f flag.\n"
#define MSG_958 \
"0516-958 %1$s: The volume group is not available for normal\n\
\toperation. Run diagnostics on the missing physical volumes.\n\
\tIf you must activate the volume group to recover data,\n\
\tuse the varyonvg command with the -f flag.\n"
#define MSG_959 \
"0516-959 %1$s: /tmp directory does not have enough space,\n\
\tdelete some files to free up at least 72KB then execute varyoffvg\n\
\tfollowed by varyonvg.\n"
#define MSG_960 \
"Usage: %1$s [-i] VGname [LVname...]\n\
Reorganizes partition allocation within a volume group.\n"
#define MSG_962 \
"0516-962 %1$s: Logical volume %2$s migrated.\n"
#define MSG_964 \
"0516-964 %1$s: Unable to migrate logical volume %2$s.\n"
#define MSG_966 \
"0516-966 %1$s: Unable to create internal map.\n"
#define MSG_968 \
"0516-968 %1$s: Unable to reorganize volume group.\n"
#define MSG_970 \
"0516-970 %1$s: Logical volume %2$s belongs to a different\n\
\tvolume group\n"
#define MSG_972 \
"0516-972 %1$s: Logical volume %2$s is not relocatable\n"
#define MSG_982 \
"0516-982 %1$s: Physical volume %2$s status is missing,\n\
\tno allocate, or removed.\n"
#define MSG_984 \
"0516-984 %1$s: There are no physical volumes to reorganize.\n"
#define MSG_988 \
"0516-988 %1$s: Unable to move a physical partition.\n"
#define MSG_990 \
"0516-990 %1$s: Unable to create new allocation for\n\
\tlogical volume %2$s.\n"
#define MSG_LS_BADOPS \
"\
0516-992 %1$s: Illegal combination of command line options.\n"
#define MSG_LS_BADLP \
"\
0516-994 %1$s: Too many copies for logical partition %2$d.\n\
\tDepending upon where this product was acquired, contact a\n\
\tservice representative or the approved_supplier.\n"
#define MSG_LS_LVUSAGE \
"\
Usage: %1$s [-l | -m] [-n DescriptorPV] LVname\n\
       %2$s: [-n DescriptorPV] -p PVname [LVname]\n\
Lists the characteristics of a logical volume.\n"
#define MSG_LS_PVUSAGE \
"\
Usage: %1$s [-l | -p] [-n DescriptorPV] [-v VGid] [PVname]\n\
Lists the characteristics of a volume group's physical volume.\n"
#define MSG_LS_VGUSAGE \
"\
Usage: %1$s [-o] [-n PVname]\n\
       %2$s [-i] [-l | -p] VGname\n\
Lists the characteristics of a volume group.\n"
#define MSG_RESYNC_FAILED_LVM \
"\
0516-015 %1$s: The migrate was done but the partition\n\
\tcould not be resynced.\n"
#define MSG_EXTRAPP_LVM \
"\
0516-017 %1$s: The partition was moved to the new location\n\
\tbut the old partition could not be removed.\n"
#define MSG_LOSTPP_LVM \
"\
0516-019 %1$s: The migrate failed after the old partition\n\
\twas removed and it could not be recovered.\n"
#define MSG_MAJINUSE_LVM \
"\
0516-021 %1$s: The varyonvg failed because the volume group's\n\
\tmajor number was already used by another device.\n"
#define MSG_NLS_PVUSAGE \
"\
Usage: %1$s [-M | -l | -p]  [-n DescriptorPV] [-v VGid] [PVname]\n\
Lists the characteristics of a volume group's physical volume.\n"
#define MSG_NLS_VGUSAGE \
"\
Usage: %1$s [-o] [-n PVname]\n\
       %2$s [-i] [-M | -l | -p] VGname...\n\
Lists the characteristics of a volume group.\n"
#define MSG_1002 \
"Usage: %1$s [-i] [-f] {-l|-p|-v} Name\n\
Synchronize logical partition copies.\n"
#define MSG_1003 \
"Usage:\n\
\t%1$s -n NewLVname LVname\n\
\t%2$s [-a IntraPolicy] [-e InterPolicy] [-L Label] [-u UpperBound]\n\
\t\t[-s Strict] [-b BadBlocks] [-d Schedule] [-p Permission]\n\
\t\t[-r Relocate] [-t Type] [-w MirrorWriteConsistency]\n\
\t\t[-v Verify] [-x MaxLPs] LVname...\n\
Changes the characteristics of a logical volume.\n"
#define MSG_1004 \
"\
0516-677 %1$s: The -w parameter for Mirror-Write Consistency\n\
\tmust be y or n.\n"
#define MSG_1005 \
"\
0516-679 %1$s: The -w parameter for Mirror-Write Consistency\n\
\tof a paging Logical Volume must be n.\n"
#define MSG_1006 \
"\
Usage: %1$s [-a IntraPolicy] [-b BadBlocks] [-c Copies] [-d Schedule]\n\
\t[-e InterPolicy] [-i] [-L Label] [-m MapFile] [-r Relocate] [-s Strict]\n\
\t[-t Type] [-u UpperBound] [-v Verify] [-w MWC] [-x MaxLPs] [-y LVname]\n\
\t[-Y Prefix] [-S StripeSize] VGname NumberOfLPs [PVname...]\n\
Makes a logical volume.\n"
#define MSG_1008 \
"0516-1008 %1$s: Logical volume %2$s must be closed.  If the logical volume\n\
\tcontains a filesystem, the umount command will close the LV device.\n"
#define MSG_LMKTEMP_USAGE \
"\
516-1009 Usage: %1$s template [size]\n"
#define MSG_1010 \
"\
0516-1010 %1$s: Warning, the physical volume %2$s has open logical \n\
\tvolumes.  Continuing with change.\n"
#define MSG_1011 "0516-1011 %1$s: Logical volume %2$s is labeled as a boot logical volume.\n"
#define MSG_1012 "%1$s: boot logical volume %2$s migrated. Please remember to run\n\
\tbosboot, specifying %3$s as the target physical boot device.\n"
#define MSG_1013 "0516-1013 %1$s: No contiguous partitions found for boot logical volume %2$s.\n"
#define MSG_1014 "Usage: %1$s [-f] [-n] [-s] [-p] VGname\n\
Varies a volume group on.\n"

#define MSG_LVEXISTLVM "\
0516-020 %1$s: Logical volume name or minor number already\n\
\tin use.\n"
#define MSG_DALVOPNLVM "\
0516-034 %1$s: Unable to access volume group device. Execute\n\
\tredefinevg to build correct environment.\n"
#define MSG_INVCONFIGLVM "\
0516-078 %1$s: Incomplete device driver configuration. Probable\n\
\tcause is the special file for the VG is missing. Execute\n\
\tvaryoffvg followed by varyonvg to build correct environment.\n"
#define MSG_NOTCHARDEVLVM "\
0516-080 %1$s: Device is block device; must be raw device. Execute\n\
\tredefinevg to build correct environment.\n"
#define MSG_INVDEVENTLVM "\
0516-082 %1$s: Unable to access a special device file.\n\
\t Execute redefinevg and synclvodm to build correct environment.\n"
#define MSG_BADLVID "\
0516-104 %1$s: Incorrect LV identifier. The format must be \n\
\tVG_identifier.LV_minor_number\n"
#define MSG_ATTRNOTFOUND "\
0516-310 %1$s: Unable to find attribute %2$s in the Device\n\
\tConfiguration Database. Execute synclvodm to attempt to\n\
\tcorrect the database.\n"
#define MSG_1022 "\
0516-322 %1$s: The Device Configuration Database is inconsistent.\n\
\tExecute redefinevg to correct the database.\n"
#define MSG_1023 "0516-620 %1$s: Warning, cannot update device configuration\n\
\tdatabase. Execute synclvodm  to synchronize the database.\n"
#define MSG_1024 "0516-624 %1$s: Warning, cannot update device configuration\n\
\tdatabase for volume group. Execute redefinevg to synchronize the database.\n"
#define MSG_1025 "0516-626 %1$s: Warning, cannot update device configuration\n\
\tdatabase for physical volume %2$s in volume group %3$s. Execute\n\
\tredefinevg to synchronize the database with the physical volume\n\
\tdescriptor areas.\n"
#define MSG_1026 "0516-630 %1$s: Warning, cannot update the device configuration\n\
\tdatabase for logical volume %2$s. Execute synclvodm to \n\
\tupdate the database.\n"
#define MSG_1027 "0516-916 %1$s: Warning, can not remove logical volume %2$s from\n\
\tdevice configuration database.\n"
#define MSG_1028 "Usage: %1$s [-a Auto on] [-Q quorum] [-u] VGname...\n\
Changes the characteristics of a volume group.\n"
#define MSG_1029 "0516-1029 %1$s: Optical device found.  Only one optical device\n\
\tis supported per volume group and no other device is allowed in\n\
\tthe same volume group.  Command rejected. \n"
#define MSG_1030 "0516-1030 %1$s: Command not executed. \n"
#define MSG_1031 "0516-1031 %1$s: Please vary off and export the previous\n\
\tvolume group %2$s before execute this command. \n"
#define MSG_1032 \
"0516-1032 %1$s: The -e, -m, -s, -u, -w, -d and -c options cannot be\n\
\tused with striped logical volumes (-S option).\n"
#define MSG_1033 \
"0516-1033 %1$s: Physical volume(s) not specified.\n"
#define MSG_1034 \
"0516-1034 %1$s: Not enough physical partitions in physical volume %2$s.\n"
#define MSG_1035 \
"0516-1035 %1$s: The -e, -u, -s, and -m options cannot be\n\
\tused with striped logical volume.\n"
#define MSG_1036 \
"0516-1036 %1$s: Striped logical volume size can only be an even\n\
\tmultiple of the striping width.\n"
#define MSG_1037 \
"0516-1037 %1$s: Physical volumes or upperbound value not specified. \n"
#define MSG_1038 \
"0516-1038 %1$s: Not enough physical partitions for striped logical \n\
\tvolume %2$s.\n"
#define MSG_1039 \
"0516-1039 %1$s: The -S parameter for stripe size must be a power\n\
\tof 2. (ex: 4K, 8K, 16K, 32K, 64K or 128K) \n"
#define MSG_1040 \
"0516-1040 %1$s: Physical volumes cannot be specified when extending the\n\
striped logical volume.\n"
#define MSG_1041 \
"0516-1041 %1$s: Cannot create a striped logical volume as relocatable.\n"
#define MSG_1042 \
"0516-1042 %1$s: Cannot change a striped logical volume to relocatable.\n"
#define MSG_1043 \
"0516-1043 %1$s: There are not enough physical volumes to create a\n\
\tstriped logical volume.\n"
#define MSG_1044 \
"0516-1044 %1$s: Copies are not allowed on a striped logical volume.\n"
#define MSG_1046 \
"0516-1046 %1$s: The -e, -u, -s, -w, and -d options cannot be used\n\
\twith striped logical volumes.\n"
#define MSG_1048 \
"0516-1048 %1$s: Cannot create a striped logical volume as type 'boot'.\n"
#define MSG_1049 \
"0516-1049 %1$s: Cannot change a striped logical volume as type 'boot'.\n"
#define MSG_1100 "Striping Width"
#define MSG_1101 "Stripe Size"
#define MSG_PVINTRO "physical volume"
#define MSG_LVINTRO "logical volume"
#define MSG_VGINTRO "volume group"

