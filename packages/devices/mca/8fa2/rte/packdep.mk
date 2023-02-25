# @(#)49  1.3  src/packages/devices/mca/8fa2/rte/packdep.mk, pkgdevcommo, pkg412, 9445C412a 11/7/94 13:48:39
#
# COMPONENT_NAME: pkgdevcommo
#
# FUNCTIONS: packaging definitions
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
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts or prereq files would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST         += devices.mca.8fa2.rte.pre_d  

ROOT_LIBLPP_LIST        += devices.mca.8fa2.rte.err    \
			   devices.mca.8fa2.rte.trc

USR_ODMADD_LIST         += mps_tok.add%devices.mca.8fa2.rte	\
			   sm_mps_tok.add%devices.mca.8fa2.rte
ROOT_ODMADD_LIST        +=

INFO_FILES              += devices.mca.8fa2.rte.prereq
