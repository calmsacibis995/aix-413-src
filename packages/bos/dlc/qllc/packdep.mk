# @(#)96        1.15  src/packages/bos/dlc/qllc/packdep.mk, pkgdlc, pkg41J, 9517B_all 4/21/95 13:11:49
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

USR_LIBLPP_LIST		+= bos.dlc.qllc.namelist \
                           bos.dlc.qllc.pre_d 

ROOT_LIBLPP_LIST	+= bos.dlc.qllc.config \
                           bos.dlc.qllc.post_i \
                           bos.dlc.qllc.unpost_i \
                           bos.dlc.qllc.unconfig  

USR_LIBLPP_UPDT_LIST	+=
ROOT_LIBLPP_UPDT_LIST	+= bos.dlc.qllc.config_u \
                           bos.dlc.qllc.post_u \
                           bos.dlc.qllc.unpost_u

USR_ODMADD_LIST		+= dlcqllc.add%bos.dlc.qllc
ROOT_ODMADD_LIST	+= dlcqllc_root.add%bos.dlc.qllc

INFO_FILES		+= bos.dlc.qllc.prereq \
			   bos.dlc.qllc.insize \
			   bos.dlc.qllc.supersede
