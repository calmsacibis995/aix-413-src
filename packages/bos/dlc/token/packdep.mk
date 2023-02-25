# @(#)98        1.13  src/packages/bos/dlc/token/packdep.mk, pkgdlc, pkg411, GOLD410 4/26/94 10:10:18
#
# COMPONENT_NAME: pkgdlc
#
# FUNCTIONS: packaging definition
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
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= bos.dlc.token.namelist \
                           bos.dlc.token.pre_d

ROOT_LIBLPP_LIST	+= bos.dlc.token.config \
                           bos.dlc.token.unconfig

USR_LIBLPP_UPDT_LIST	+=
ROOT_LIBLPP_UPDT_LIST	+= bos.dlc.token.config_u

USR_ODMADD_LIST		+= dlctoken.add%bos.dlc.token
ROOT_ODMADD_LIST	+= dlctoken_root.add%bos.dlc.token

INFO_FILES		+= bos.dlc.token.prereq	\
			   bos.dlc.token.supersede
