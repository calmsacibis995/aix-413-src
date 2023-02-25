# @(#)22        1.10  src/packages/bos/rcs/packdep.mk, pkgrcs, pkg411, GOLD410 7/5/94 15:23:26
#
# COMPONENT_NAME: pkgrcs
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
USR_LIBLPP_LIST		+= bos.rcs.namelist \
			   bos.rcs.pre_rm

ROOT_LIBLPP_LIST        += bos.rcs.cfgfiles \
			   bos.rcs.namelist \
			   bos.rcs.err

USR_ODMADD_LIST		+= rcs_sm_menu_opt.add%bos.rcs \
			   rcs_sm_name_hdr.add%bos.rcs \
			   rcs_sm_cmd_hdr.add%bos.rcs \
			   rcs_sm_cmd_opt.add%bos.rcs

INFO_FILES		+= bos.rcs.prereq \
			   bos.rcs.supersede
