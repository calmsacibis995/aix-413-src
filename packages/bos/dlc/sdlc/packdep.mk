# @(#)97        1.15  src/packages/bos/dlc/sdlc/packdep.mk, pkgdlc, pkg41J, 9517B_all 4/21/95 13:10:05
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
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
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

USR_LIBLPP_LIST		+= bos.dlc.sdlc.namelist \
                           bos.dlc.sdlc.pre_d

ROOT_LIBLPP_LIST	+= bos.dlc.sdlc.config \
                           bos.dlc.sdlc.post_i \
                           bos.dlc.sdlc.unpost_i \
                           bos.dlc.sdlc.unconfig

USR_LIBLPP_UPDT_LIST	+=
ROOT_LIBLPP_UPDT_LIST	+= bos.dlc.sdlc.config_u \
                           bos.dlc.sdlc.post_u \
                           bos.dlc.sdlc.unpost_u

USR_ODMADD_LIST		+= dlcsdlc.add%bos.dlc.sdlc
ROOT_ODMADD_LIST	+= dlcsdlc_root.add%bos.dlc.sdlc

INFO_FILES		+= bos.dlc.sdlc.prereq \
			   bos.dlc.sdlc.insize \
			   bos.dlc.sdlc.supersede
