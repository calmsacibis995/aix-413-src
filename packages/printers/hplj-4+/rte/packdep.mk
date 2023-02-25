# @(#)34        1.1  src/packages/printers/hplj-4+/rte/packdep.mk, pkgdevprint, pkg41J, 9512A_all 3/15/95 19:14:02
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
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

USR_LIBLPP_LIST		+= printers.hplj-4+.rte.pre_d

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.hplj-4+.diag.add%printers.hplj-4+.rte \
			   printer.hplj-4+.add%printers.hplj-4+.rte
