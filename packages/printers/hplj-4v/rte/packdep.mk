# @(#)29        1.1  src/packages/printers/hplj-4v/rte/packdep.mk, pkgdevprint, pkg41J, 9512A_all 3/15/95 19:13:54
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

USR_LIBLPP_LIST		+= printers.hplj-4v.rte.pre_d

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.hplj-4v.diag.add%printers.hplj-4v.rte \
			   printer.hplj-4v.add%printers.hplj-4v.rte
