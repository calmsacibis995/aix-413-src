# @(#)14        1.8  src/packages/devices/sio/ppa/rte/packdep.mk, pkgdevbase, pkg411, GOLD410 6/28/94 08:18:25
#
# COMPONENT_NAME: pkgdevbase
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

USR_LIBLPP_LIST		+= devices.sio.ppa.rte.namelist \
						devices.sio.ppa.rte.pre_d

USR_ODMADD_LIST		+= adapter.sio.ppa_iod.add%devices.sio.ppa.rte \
                           adapter.sio.ppa.add%devices.sio.ppa.rte

INFO_FILES		+= devices.sio.ppa.rte.prereq
