# @(#)31        1.10  src/packages/devices/sio/sa/rte/packdep.mk, pkgdevtty, pkg411, GOLD410 4/21/94 08:39:58
#
# COMPONENT_NAME: pkgdevtty
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
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

INFO_FILES              += devices.sio.sa.rte.prereq

USR_LIBLPP_LIST		+= devices.sio.sa.rte.namelist.S \
                           devices.sio.sa.rte.pre_d

USR_ODMADD_LIST		+= adapter.sio.s1a.add%devices.sio.sa.rte \
			   adapter.sio.s2a.add%devices.sio.sa.rte \
			   adapter.sio.s3a.add%devices.sio.sa.rte 
