# @(#)94        1.6  src/packages/printers/ibm4019_AR/rte/packdep.mk, pkgdevprint, pkg411, 9432A411a 8/5/94 11:40:15
#
# COMPONENT_NAME: pkgdevprint
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

USR_LIBLPP_LIST		+= printers.ibm4019_AR.rte.pre_d \
			   printers.ibm4019_AR.rte.namelist \
			   printers.ibm4019_AR.rte.rm_inv

USR_ODMADD_LIST		+= 

INFO_FILES		+= printers.ibm4019_AR.rte.prereq
