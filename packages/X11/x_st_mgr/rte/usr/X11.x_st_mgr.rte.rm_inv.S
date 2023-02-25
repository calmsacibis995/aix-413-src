# @(#)66	1.2  src/packages/X11/x_st_mgr/rte/usr/X11.x_st_mgr.rte.rm_inv.S, pkgxstmgr, pkg411, GOLD410 7/26/94 17:40:55
#
#   COMPONENT_NAME: PKGXSTMGR
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#  The first section is all of those files that we shipped a very long time ago
# or that should never have been shipped (SMIT files).
/usr/lpp/x_st_mgr/bin/xconfig:
/usr/lpp/x_st_mgr/bin/x_add_t_2.sm:
/usr/lpp/x_st_mgr/bin/x_add_t_3.sm:
/usr/lpp/x_st_mgr/bin/x_chg_t_2.sm:
/usr/lpp/x_st_mgr/bin/x_chg_t_3.sm:
/usr/lpp/x_st_mgr/bin/dev.sm:
/usr/lpp/x_st_mgr/bin/x_add_fs_fpe_one.sm:
/usr/lpp/x_st_mgr/bin/x_add_nfs_fpe_one.sm:
/usr/lpp/x_st_mgr/bin/x_add_trm.sm:
/usr/lpp/x_st_mgr/bin/x_add_trm_120.sm:
/usr/lpp/x_st_mgr/bin/x_add_trm_130.sm:
/usr/lpp/x_st_mgr/bin/x_add_trm_150.sm:
/usr/lpp/x_st_mgr/bin/x_add_xst_fpe_one.sm:
/usr/lpp/x_st_mgr/bin/x_chg_net.sm:
/usr/lpp/x_st_mgr/bin/x_chg_trm.sm:
/usr/lpp/x_st_mgr/bin/x_chg_trm_120.sm:
/usr/lpp/x_st_mgr/bin/x_chg_trm_130.sm:
/usr/lpp/x_st_mgr/bin/x_chg_trm_150.sm:
/usr/lpp/x_st_mgr/bin/x_config.sm:
/usr/lpp/x_st_mgr/bin/x_def_net.sm:
/usr/lpp/x_st_mgr/bin/x_fpe.sm:
/usr/lpp/x_st_mgr/bin/x_ls_fpe.sm:
/usr/lpp/x_st_mgr/bin/x_ls_net.sm:
/usr/lpp/x_st_mgr/bin/x_ls_trm.sm:
/usr/lpp/x_st_mgr/bin/x_rm_fpe.sm:
/usr/lpp/x_st_mgr/bin/x_rm_net.sm:
/usr/lpp/x_st_mgr/bin/x_rm_trm.sm:
#
# Below are files that were removed in the move from X11R4 to X11R5 and since
# X11R4 is not available in 4.1 they also need to go.
/usr/lpp/x_st_mgr/bin/bootfile:
	symlinks=/etc/x_st_mgr/bootfile
/usr/lpp/x_st_mgr/bin/x11xor.out:
#
/usr/lpp/x_st_mgr/bin/bootfile1:
	symlinks=/etc/x_st_mgr/bootfile1
/usr/lpp/x_st_mgr/bin/x11xor1.out:
#
/usr/lpp/x_st_mgr/bin/bootfile2:
	symlinks=/etc/x_st_mgr/bootfile2
/usr/lpp/x_st_mgr/bin/x11xor2.out:
#
#  For space issues we are going to move the actual binaries for download onto
# the Xstation 140 and 150 from the / file system to the /usr file system.  The
# old files need to be removed.
#
/usr/lpp/x_st_mgr/bin/X150serv:
	symlink=/etc/x_st_mgr/xs150/upd/file.list
/usr/lpp/x_st_mgr/bin/X140serv:
	symlink=/etc/x_st_mgr/xs140/upd/file.list
#
#  Next remove the files that are in the compat option.  If the user wants them
#they can install the compat option.
/usr/lpp/x_st_mgr/bin/xset:
/usr/lpp/x_st_mgr/bin/start_server:
/usr/lpp/x_st_mgr/bin/conf_tab:
