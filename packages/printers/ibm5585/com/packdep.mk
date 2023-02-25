# @(#)69        1.8  src/packages/printers/ibm5585/com/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 11:51:33
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

USR_LIBLPP_LIST		+= printers.ibm5585.com.pre_d

USR_ODMADD_LIST		+= printer.ibm5585.diag.add%printers.ibm5585.com \
			   printer.ibm5585.add%printers.ibm5585.com
