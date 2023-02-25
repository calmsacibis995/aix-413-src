# @(#)17        1.24  src/bos/usr/sbin/install/installp/Makefile, cmdinstl, bos411, 9428A410j 3/31/94 20:22:19
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= installp
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= installp
IDIR		= /usr/sbin/

INCFLAGS	= -I../messages -I../include

OFILES		= \
	installp.o installp_eval_size.o inu_apply_opts.o inu_args_func.o \
	inu_bld_commit_sop.o inu_bld_root_sop.o inu_bld_sop.o \
	inu_cleanup_opts.o inu_commit_opts.o inuconvert.o \
	inu_get_31cntl_files.o inu_get_cntl_files.o inu_get_min_space.o \
	inu_get_savdir.o inu_grep_option_toc.o inu_instl_args.o inu_level.o \
	inu_ls_apars.o inu_ls_instr.o inu_ls_toc.o inu_mk_opt_fil.o \
	inu_putenv.o inu_quit.o inu_reject_opts.o inu_run_sysck.o \
	inu_set_cur_oper.o inu_set_env_vars.o inu_set_libdir.o \
	inu_set_savdir.o inu_set_vpdtree.o inu_sig_abort.o \
	inu_summary.o inu_valid_args.o inu_vldt_vpd.o inu_vpd_lpp_prod.o \
	inu_vpd_history.o inu_vpd_set_states.o inu_do_status.o \
	inu_swvpd_check.o inu_remove_opts.o srchvpd.o inu_bosboot.o \
        inu_pseudo_ckreq.o inu_sort_sop.o \
        inu_process_opts.o inu_exec_installp.o inu_bld_sop_utils.o \
        inu_pif_summary.o inu_summary_log.o inu_deinstall_opts.o \
        rm_biron_files.o inu_preview.o inu_section_hdrs.o \
        installp_utils.o inu_print_fail_stats.o getspace_migdir.o \
        inu_toc_vpd.o instl_msg.o

LIBS		= -lckprereq -linulib -lodm -lswvpd -ls -lPW -lcfg

.include <${RULES_MK}>
