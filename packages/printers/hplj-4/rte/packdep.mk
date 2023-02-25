# @(#)98        1.10  src/packages/printers/hplj-4/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 11:52:15
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

USR_LIBLPP_LIST		+= printers.hplj-4.rte.pre_d \
			   printers.hplj-4.rte.namelist \
			   printers.hplj-4.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.hplj-4.diag.add%printers.hplj-4.rte \
			   printer.hplj-4.add%printers.hplj-4.rte
