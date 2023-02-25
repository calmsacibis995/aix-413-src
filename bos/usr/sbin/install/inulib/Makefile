# @(#)18        1.23  src/bos/usr/sbin/install/inulib/Makefile, cmdinstl, bos41J, 9510A_all 2/27/95 12:15:01
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# - - - - - - - - - -  - - - - - - - - - -  - - - - - - - - - -  - - - - - -
#                         *****  NOTE *****
#  src/bldenv/inutoc has a dependency on this library.  So, if you change
#  this Makefile, ensure that src/bldenv/inutoc builds okay.
# - - - - - - - - - -  - - - - - - - - - -  - - - - - - - - - -  - - - - - -


VPATH		= ../messages

LIBRARIES	= libinulib.a
CATFILES	= cmdinstl_e.cat
HFILES		=
EXPLIB_TARGETS	= export_libinulib.a

IDIR		= /usr/ccs/lib/

INCFLAGS	=-I../messages -I../include

OFILES		= \
	inu_level_compare.o inu_level_convert.o	decapitate.o \
	enough_space.o expand_lv.o find_lfs.o find_pfs.o \
	getbasename.o get_stat.o inu_archive.o \
	inu_cat.o inu_cp.o inu_dev_ctl.o inu_expand_ps.o \
	inu_fcopy.o inu_fget_file_sizes.o inu_fget_fs_sizes.o \
	inu_get_fs_sizes.o inu_get_lppname.o inu_lock.o inu_mk_dir.o \
	inu_mk_mstrs.o inu_msg.o inu_mv.o inu_obj_mgmt.o inu_position_tape.o \
	inu_read_file.o inu_restore.o inu_rm.o inu_sop_func.o inu_toc.o \
	inurddk.o in_al.o is_file.o mk_fs_list.o get_lppname.o \
	inufile_eval_size.o inu_rm_save_dir.o inu_signal.o \
	inu_rcvr_or_rm.o \
        inu_do_exec.o file_query.o vappend_file.o inu_tree_string.o \
        inu_unpad.o inu_deinst_chk.o get_fsname.o get_migdir_prefix.o \
        inu_tape_lock.o  inu_init_strings.o inu_oem_chk.o

.include <${RULES_MK}>
