# @(#)68        1.1  src/packages/printers/lex4076-2c/rte/packdep.mk, pkgdevprint, pkg41J, 9512A_all 3/15/95 19:15:04
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

USR_LIBLPP_LIST		+= printers.lex4076-2c.rte.pre_d

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.lex4076-2c.diag.add%printers.lex4076-2c.rte \
			   printer.lex4076-2c.add%printers.lex4076-2c.rte
