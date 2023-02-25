# @(#)46	1.5  src/packages/devices/mca/8ef4/com/packdep.mk, pkgdevcommo, pkg411, GOLD410 4/9/94 16:07:34
#
# COMPONENT_NAME: pkgdevcommo
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1993,1994
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that	need to	be added to 
#		the respective liblpp.a	other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For	example, any
#		config scripts would be	included here.
#		The variable may be left blank if there	are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= devices.mca.8ef4.com.namelist \
			   devices.mca.8ef4.com.rm_inv	 \
			   devices.mca.8ef4.com.mig.odmdel
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= sm_fddi.add%devices.mca.8ef4.com
ROOT_ODMADD_LIST	+=

INFO_FILES		+= devices.mca.8ef4.com.supersede
