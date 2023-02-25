# static char sccsid[] = "@(#)07	1.2  src/packages/devices/isa/cxia/rte/packdep.mk, pkgdevtty, pkg41J, 174218.pkg 3/23/95 17:18:43";
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
OBJCLASSDB               = ${ODE_TOOLS}/usr/lib/uniqueobjclassdb

USR_LIBLPP_LIST		+= devices.isa.cxia.rte.namelist \
			   devices.isa.cxia.rte.pre_d

ROOT_LIBLPP_LIST	+= devices.isa.cxia.rte.err \
			   devices.isa.cxia.rte.trc

USR_ODMADD_LIST		+= pcxr.add%devices.isa.cxia.rte \
			   com_isa_tty.add%devices.isa.cxia.rte \
			   sm_pcxr.add%devices.isa.cxia.rte 

INFO_FILES		+= devices.isa.cxia.rte.prereq
