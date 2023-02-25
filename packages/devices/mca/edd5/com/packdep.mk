# @(#)35        1.6 7/5/94 src/packages/devices/mca/edd5/com/packdep.mk, pkgdevgraphics, pkg411, GOLD410 17:24:38
#
# COMPONENT_NAME: pkgdevgraphics
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

USR_LIBLPP_LIST		+= devices.mca.edd5.com.pre_d \
			   devices.mca.edd5.com.cfginfo

# NOTE: When turning trace back on, remove these lines and
# uncomment the next line.  You will also have to edit the
# Makefile and the devices.mca.edd5.com.lp file since we
# are adding a root file.  Don't forget the Makefile above
# this level as well.
#ROOT_LIBLPP_LIST	+= devices.mca.edd5.com.trc

USR_ODMADD_LIST		+= gio.add%devices.mca.edd5.com \
			   sm_dials.add%devices.mca.edd5.com
ROOT_ODMADD_LIST	+=
