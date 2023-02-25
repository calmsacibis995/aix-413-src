# @(#)88	1.1  src/packages/devices/sio/fda/com/packdep.mk, pkgdevbase, pkg411, GOLD410 6/28/94 08:05:01
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

USR_LIBLPP_LIST		+= devices.sio.fda.com.namelist \
						devices.sio.fda.com.pre_d
ROOT_LIBLPP_LIST    += 

USR_ODMADD_LIST		+= diskette.siofd.fd.add%devices.sio.fda.com \
                           sm_fd.add%devices.sio.fda.com
ROOT_ODMADD_LIST        +=

INFO_FILES              +=

