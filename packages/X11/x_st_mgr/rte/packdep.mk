# @(#)54	1.5  src/packages/X11/x_st_mgr/rte/packdep.mk, pkgxstmgr, pkg41J 8/2/94 14:28:25
#
#   COMPONENT_NAME: PKGXSTMGR
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
USR_ODMADD_LIST		+= dev.add%X11.x_st_mgr.rte \
	x_add_fs_fpe_one.add%X11.x_st_mgr.rte \
	x_add_nfs_fpe_one.add%X11.x_st_mgr.rte \
	x_add_trm.add%X11.x_st_mgr.rte \
	x_add_trm_120.add%X11.x_st_mgr.rte \
	x_add_trm_130.add%X11.x_st_mgr.rte \
	x_add_trm_140.add%X11.x_st_mgr.rte \
	x_add_trm_150.add%X11.x_st_mgr.rte \
	x_add_xst_fpe_one.add%X11.x_st_mgr.rte \
	x_chg_def_set.add%X11.x_st_mgr.rte \
	x_chg_net.add%X11.x_st_mgr.rte \
	x_chg_net.add%X11.x_st_mgr.rte \
	x_chg_trm.add%X11.x_st_mgr.rte \
	x_chg_trm_120.add%X11.x_st_mgr.rte \
	x_chg_trm_130.add%X11.x_st_mgr.rte \
	x_chg_trm_140.add%X11.x_st_mgr.rte \
	x_chg_trm_150.add%X11.x_st_mgr.rte \
	x_config.add%X11.x_st_mgr.rte \
	x_def_net.add%X11.x_st_mgr.rte \
	x_fpe.add%X11.x_st_mgr.rte \
	x_ls_fpe.add%X11.x_st_mgr.rte \
	x_ls_net.add%X11.x_st_mgr.rte \
	x_ls_trm.add%X11.x_st_mgr.rte \
	x_rest_def.add%X11.x_st_mgr.rte \
	x_rm_fpe.add%X11.x_st_mgr.rte \
	x_rm_net.add%X11.x_st_mgr.rte \
	x_rm_trm.add%X11.x_st_mgr.rte \
	x_add_trm_160.add%X11.x_st_mgr.rte \
	x_chg_trm_160.add%X11.x_st_mgr.rte 


USR_LIBLPP_LIST	+= X11.x_st_mgr.rte.namelist \
		X11.x_st_mgr.rte.pre_i \
		X11.x_st_mgr.rte.rm_inv

ROOT_LIBLPP_LIST += X11.x_st_mgr.rte.post_i \
		X11.x_st_mgr.rte.unconfig \
		X11.x_st_mgr.rte.rm_inv \
		X11.x_st_mgr.rte.cfgfiles \
		X11.x_st_mgr.rte.namelist

INFO_FILES	+= X11.x_st_mgr.rte.insize \
		X11.x_st_mgr.rte.prereq \
		X11.x_st_mgr.rte.supersede \
		X11.x_st_mgr.rte.inv_u
