# @(#)96        1.1  src/packages/bos/dlc/com_enet/packdep.mk, pkgdlc, pkg411, GOLD410 3/31/94 19:42:26
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
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= 

ROOT_LIBLPP_LIST	+= 

USR_LIBLPP_UPDT_LIST	+=

USR_ODMADD_LIST		+= dlccom_enet.add%bos.dlc.com_enet
ROOT_ODMADD_LIST        +=

INFO_FILES		+= bos.dlc.com_enet.prereq
