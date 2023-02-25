# @(#)97        1.7  src/packages/devices/mca/8ef2/com/packdep.mk, pkgdevcommo, pkg411, GOLD410 7/5/94 14:54:05
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
# *_LIBLPP_LIST contains the list of files that need to be added to 
#               the respective liblpp.a other than the copyright, .al,
#               .tcb, .size, and .inventory files.  For example, any
#               config scripts would be included here.
#               The variable may be left blank if there are no additional
#               files, but the option will exist.
#

USR_LIBLPP_LIST         += devices.mca.8ef2.com.namelist

ROOT_LIBLPP_LIST	+= devices.mca.8ef2.com.err \
			   devices.mca.8ef2.com.trc

USR_ODMADD_LIST         +=
