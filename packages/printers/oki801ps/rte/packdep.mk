# @(#)95        1.7  src/packages/printers/oki801ps/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 12:04:50
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

USR_LIBLPP_LIST		+= printers.oki801ps.rte.pre_d \
			   printers.oki801ps.rte.namelist \
			   printers.oki801ps.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.oki801ps.diag.add%printers.oki801ps.rte \
			   printer.oki801ps.add%printers.oki801ps.rte

INFO_FILES		+= printers.oki801ps.rte.prereq
