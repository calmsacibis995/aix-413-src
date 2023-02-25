/* @(#)78       1.7  src/bos/usr/sbin/lvm/cmdlvm_def.h, cmdlvm, bos411, 9428A410j 6/15/93 13:59:39 */

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define MSG_MISSING_LVID \
"0516-601 %s: Specify logical volume identifier.\n"
#define MSG_MISSING_LVID2 \
"0516-601 %s: Specify logical volume identifier.\n"
#define MSG_BAD_LVID \
"0516-602 %s: Logical volume identifier is not correct.\n"
#define MSG_MISSING_VGID \
"0516-603 %s: Specifiy volume group identifier.\n"
#define MSG_MISSING_PVID \
"0516-604 %s: Specifiy physical volume identifier.\n"
#define MSG_MISSING_LVNM \
"0516-605 %s: Specify logical volume name.\n"
#define MSG_MISSING_MINOR \
"0516-606 %s: Specify logical volume minor number.\n"
#define MSG_MISSING_MAJ \
"0516-608 %s: Specify volume group major number.\n"
#define MSG_MISSING_PVNM \
"0516-609 %s: Specify physical volume name.\n"
#define MSG_MISSING_MAXLV \
"0516-610 %s: Specify maximum number of logical volumes.\n"
#define MSG_MISSING_DASZ \
"0516-611 %s: Specify size of volume group descriptor area.\n"
#define MSG_MISSING_PPSZ \
"0516-612 %s: Specify size of physical partitions.\n"
#define MSG_MISSING_MID \
"0516-613 %s: Specify logical volume device driver's kernal module identifier.\n"
#define MSG_MISSING_FN \
"0516-614 %s: Specify file name.\n"
#define MSG_MISSING_MAP \
"0516-615 %s: Cannot read input file map entries.\n"
#define MSG_EXPECT_MAP \
"0516-616 Expecting more entries in map file.\n"
#define MSG_EXPECT_INPT \
"0516-617 Expecting PV identifier, PP number, LP number in each map entries.\n"
#define MSG_RECEIVED \
"0516-618 Received: %s"
#define MSG_RDERR_MAP \
"0516-619 Error reading input maps\n."
#define MSG_MISSING_VGNAME \
"0516-620 %s: Missing volume group name (-a <VGname>).\n"
#define MSG_CHLV_USAGE \
"0516-651 Usage: %s -l <LVid> [ -s <MaxLVsize> ] [-n <LVname> ]\n\
\t[ -M <MirrorPolicy> ] [ -p <permissions> ]\n\
\t[ -r <relocation> ] [ -v <WriteVerify> ]\n"
#define MSG_CHLV_FAILED \
"0516-652 %s: The lvm_changelv system call failed(%1d).\n"
#define MSG_CHPV_USAGE \
"0516-661 Usage: %s -g <VGid> -p <PVid> [ -r <Rem/Ret> ]\n\t\t[ -a <Alloc/Noalloc> ]\n"
#define MSG_CHPV_FAILED \
"0516-662 %s: The lvm_changepv system call failed(%1d)\n"
#define MSG_CRLV_USAGE \
"0516-671 Usage: %s -N <LVname> -g <VGid> -n <Minor Number>\n\
\t[ -M <MirrorPolicy> ] [ -s <MaxLPts> ] [ -p <permissions> ]\n\
\t[ -r <relocation> ] [ -v <WriteVerify> ]\n"
#define MSG_CRLV_FAILED \
"0516-672 %s: The lvm_createlv system call failed(%1d).\n"
#define MSG_CRVG_USAGE \
"0516-681 Usage: %s -a <VGname> -V <VGmajor> -N <PVname> -n <Max LV>\n\
\t-D <Descriptor Size> -s <PP Size> -m <Module ID> [ -t ]\n"
#define MSG_CRVG_FAILED \
"0516-682 %s: The lvm_createvg system call failed(%1d).\n"
#define MSG_DLV_USAGE \
"0516-691 Usage: %s -l <LVid>\n"
#define MSG_DLV_FAILED \
"0516-692 %s: The lvm_deletelv system call failed(%1d).\n"
#define MSG_DPV_USAGE \
"0516-701 Usage: %s -g <VGid> -p <PVid>\n"
#define MSG_DPV_FAILED \
"0516-702 %s: The lvm_deletepv system call failed(%1d).\n"
#define MSG_DPV_SUCCESS \
"0516-703 %s: (%1d) All physical volumes have been deleted.\n"
#define MSG_EXLV_USAGE \
"0516-711 Usage: %s -l <LVid> -s <Size> <filename>\n"
#define MSG_EXLV_FAILED \
"0516-712 %s: The lvm_extendlv system call failed(%1d).\n"
#define MSG_MISSING_ESZ \
"0516-713 %s: Specify size to be extended.\n"
#define MSG_IPV_USAGE \
"0516-721 Usage: %s -N <PVname> -g <VGid>\n"
#define MSG_IPV_FAILED \
"0516-722 %s: The lvm_installpvsystem call failed(%1d).\n"
#define MSG_MPP_USAGE \
"0516-731 Usage: %s -g <VGid> -p <old PVid> -n <old PPnumber>\n\
\t-P <new PVid> -N <new PPnumber>\n"
#define MSG_MPP_FAILED \
"0516-732 %s: The lvm_migratepp system call failed(%1d).\n"
#define MSG_MISSING_OPVID \
"0516-733 %s: Specify old physical volume identifier.\n"
#define MSG_MISSING_OPPN \
"0516-734 %s: Specify old physical partition number.\n"
#define MSG_MISSING_NPVID \
"0516-735 %s: Specify new physical volume identifier.\n"
#define MSG_MISSING_NPPN \
"0516-736 %s: Specify new physical partition number.\n"
#define MSG_QLV_USAGE \
"0516-741 Usage: %s -L <LVid> [ -p <PVname> ] [ -NGnMScsPRvoadlArt ]\n"
#define MSG_QLV_FAILED \
"0516-742 %s: The lvm_querylv system call failed(%1d).\n"
#define MSG_QPV_USAGE \
"0516-751 Usage: %s -p <PVid> [ -g <VGid> | -N <PVname> ] [ -scPnaDdAt ]\n"
#define MSG_QPV_FAILED \
"0516-752 %s: The lvm_querypv system call failed(%1d).\n"
#define MSG_MISSING_VGPV \
"0516-753 %s: Specify volume group identifier or physical volume name.\n"
#define MSG_QVG_USAGE \
"0516-761 Usage: %s [ -g <VGid> | -p <PVname> ] [ -NsFncDaLPAvt ]\n"
#define MSG_QVG_FAILED \
"0516-762 %s: The lvm_queryvg system call failed(%1d).\n"
#define MSG_MISSING_VGPN \
"0516-753 %s: Specify volume group identifier or physical volume name.\n"
#define MSG_QVGS_USAGE \
"0516-771 Usage: %s -m <kmid> [ -NGAt ]\n"
#define MSG_QVGS_FAILED \
"0516-772 %s: The lvm_queryvgs system call failed(%1d).\n"
#define MSG_RLV_USAGE \
"0516-781 Usage: %s -l <LVid> -s <Size> <filename>\n"
#define MSG_RLV_FAILED \
"0516-782 %s: The lvm_reducelv system call failed(%1d).\n"
#define MSG_MISSING_RSZ \
"0516-783 %s: Specify size to be reduced.\n"
#define MSG_RSLP_USAGE \
"0516-791 Usage: %s -l <LVid> -n <LPnumber>\n"
#define MSG_RSLP_FAILED \
"0516-792 %s: The lvm_resynclp system call failed(%1d).\n"
#define MSG_MISSING_LPN \
"0516-793 %s: Specify logical partition number.\n"
#define MSG_RSLV_USAGE \
"0516-801 Usage: %s -l <LVid>\n"
#define MSG_RSLV_FAILED \
"0516-802 %s: The lvm_resynclv system call failed(%1d).\n"
#define MSG_RSPV_USAGE \
"0516-811 Usage: %s -g <VGid> -p <PVid>\n"
#define MSG_RSPV_FAILED \
"0516-812 %s: The lvm_resyncpv system call failed(%1d).\n"
#define MSG_VOFF_USAGE \
"0516-821 Usage: %s -g <VGid> [ -f ]\n "
#define MSG_VOFF_FAILED \
"0516-822 %s: The lvm_varyoffvg system call failed(%1d).\n"
#define MSG_VON_USAGE \
"0516-831 Usage: %s -a <VGname> -V <VGmajor> -g <VGid>\n\
\t -m <Module ID> [ -ornpt ] <filename>\n"
#define MSG_VON_FAILED \
"0516-832 %s: The lvm_varyonvg system call failed(%1d).\n"
#define MSG_VON_SUCCESS \
"0516-833 %s: The lvm_varyonvg system call succeeded(%1d).\n"
#define MSG_VGDELETED_LVM \
"%s: Volume group deleted since all physical volumes removed.\n"
#define MSG_EXTRAPVID_LVM \
"0516-841 %s: Physical volume is not member of volume group.\n"
#define MSG_OFFLINEPV_LVM \
"0516-842 %s: Physical volume is inactive.\n"
#define MSG_NOTSTALE_LVM \
"0516-843 %s: Logical parition is not stale.\n"
#define MSG_UNKNERR_LVM \
"0516-851 %s: System call returned an unknown error code(%1d).\n"
#define MSG_OFFLINE_LVM \
"0516-852 %s: Volume group is offline.\n"
#define MSG_LVOPEN_LVM \
"0516-853 %s: Logical volume must be closed.\n"
#define MSG_VGMEMBER_LVM \
"0516-854 %s: Physical volume belongs to another volume group.\n"
#define MSG_PARTFND_LVM \
"0516-855 %s: Cannot delete physical volume with allocated partitions.\n"
#define MSG_ONLINE_LVM \
"0516-856 %s: Cannot delete on-line volume group.\n"
#define MSG_VGFULL_LVM \
"0516-857 %s: Not enough space in volume group descriptor for additions.\n"
#define MSG_LVEXIST_LVM \
"0516-858 %s: Logical volume id already exists.\n"
#define MSG_INVAL_PARAM_LVM \
"0516-859 %s: Bad parameter or structure value.\n"
#define MSG_PVOPNERR_LVM \
"0516-860 %s: Cannot open physical volume.\n"
#define MSG_NOALLOCLP_LVM \
"0516-861 %s: Cannot add more than 3 copies to logical partition.\n"
#define MSG_MAPFOPN_LVM \
"0516-862 %s: Cannot open internal map file - check space in /etc.\n"
#define MSG_LPNUM_INVAL_LVM \
"0516-863 %s: Bad logical partition number.\n"
#define MSG_PPNUM_INVAL_LVM \
"0516-864 %s: Bad physical partition number.\n"
#define MSG_DALVOPN_LVM \
"0516-865 %s: Cannot access descriptor area device.\n"
#define MSG_MAPFSHMAT_LVM \
"0516-866 %s: The shmat system call failed.\n"
#define MSG_INVAL_MIN_NUM_LVM \
"0516-867 %s: Bad minor number used.\n"
#define MSG_PVDAREAD_LVM \
"0516-868 %s: Cannot access physical volume descriptor area.\n"
#define MSG_PVSTATE_INVAL_LVM \
"0516-869 %s: Physical volume has an invalid state.\n"
#define MSG_MAPFRDWR_LVM \
"0516-870 %s: The fsync system call failed.\n"
#define MSG_NODELLV_LVM \
"0516-871 %s: Cannot delete logical volume before all paritions removed.\n"
#define MSG_PVMAXERR_LVM \
"0516-872 %s: Maximum number of physical volumes already in use.\n"
#define MSG_VGDASPACE_LVM \
"0516-873 %s: No descriptor area space to add another physical volume.\n"
#define MSG_NOQUORUM_LVM \
"0516-874 %s: Volume group cannot be varied on without a quorum.\n"
#define MSG_MISSPVNAME_LVM \
"0516-875 %s: Volume group not varied on with missing physical volume name.\n"
#define MSG_MISSINGPV_LVM \
"0516-876 %s: Volume group not varied on with inactive physical volume.\n"
#define MSG_ALLOCERR_LVM \
"0516-877 %s: The malloc system call failed.\n"
#define MSG_CFGRECERR_LVM \
"0516-878 %s: Cannot read physical volume configuration record.\n"
#define MSG_LVMRECERR_LVM \
"0516-879 %s: Cannot read or write logical volume manager record.\n"
#define MSG_WRTDAERR_LVM \
"0516-880 %s: Cannot write the volume group descriptor.\n"
#define MSG_CFGDEVOPN_LVM \
"0516-881 %s: Cannot open configuration device driver.\n"
#define MSG_NOTVGMEM_LVM \
"0516-882 %s: Physical volume is not in volume group.\n"
#define MSG_NOPVVGDA_LVM \
"0516-883 %s: Physical volume does not have descriptor area.\n"
#define MSG_NOTSYNCED_LVM \
"0516-884 %s: Unable to completely resyncronize volume.\n"
#define MSG_PROGERR_LVM \
"0516-885 %s: Unaccountable internal error. Depending upon where\n\
this product was acquired, contact your service representative or\n\
an approved supplier.\n"
#define MSG_BADBBDIR_LVM \
"0516-886 %s: Bad block directory is bad.\n"
#define MSG_O1OF2BAD_LVM \
"0516-887 %s: One of two logical partition copies was not syncronized.\n"
#define MSG_O1OF3BAD_LVM \
"0516-888 %s: One of three logical partition copies was not syncronized.\n"
#define MSG_O2OF3BAD_LVM \
"0516-889 %s: Two of three logical partition copies was not syncronized.\n"
#define MSG_NOMIRRORS_LVM \
"0516-890 %s: Logical volume has no mirror copies to syncronize.\n"
#define MSG_INRESYNC_LVM \
"0516-891 %s: Cannot extend logical partitions being syncronized.\n"
#define MSG_INVLPRED_LVM \
"0516-892 %s: Cannot remove only non-stale copy of stale parition.\n"
#define MSG_INVPVDEV_LVM \
"0516-893 %s: Bad physical volume device.\n"
#define MSG_INVCONFIG_LVM \
"0516-894 %s: Bad device driver configuration.\n"
#define MSG_NOTCHARDEV_LVM \
"0516-895 %s: Device is block device; must be raw device.\n"
#define MSG_INV_DEVENT_LVM \
"0516-896 %s: Bad physical volume found in volume group.\n"
#define MSG_BELOW_QRMCNT_LVM \
"0516-897 %s: Cannot remove physical volume without losing quorum.\n"
