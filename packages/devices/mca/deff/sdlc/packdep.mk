# @(#)17        1.8  src/packages/devices/mca/deff/sdlc/packdep.mk, pkgdevcommo, pkg411, 9439B411a 9/28/94 17:44:42
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

USR_LIBLPP_LIST		= devices.mca.deff.sdlc.namelist

ROOT_LIBLPP_LIST	+= devices.mca.deff.sdlc.unconfig \
			   devices.mca.deff.sdlc.trc      \
			   devices.mca.deff.sdlc.err

USR_ODMADD_LIST		+= mpa.add%devices.mca.deff.sdlc \
			   sm_mpa.add%devices.mca.deff.sdlc

ROOT_ODMADD_LIST	+=

INFO_FILES		+= devices.mca.deff.sdlc.prereq \
			   devices.mca.deff.sdlc.insize
