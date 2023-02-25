# @(#)13	1.2  src/packages/printers/starAR2463_CN/rte/packdep.mk, pkgdevprint, pkg41J, 9507B 2/1/95 11:01:40
#
#   COMPONENT_NAME: pkgdevprint
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

USR_LIBLPP_LIST		+= printers.starAR2463_CN.rte.pre_d

INFO_FILES		+= printers.starAR2463_CN.rte.prereq

USR_ODMADD_LIST         += printer.starAR2463.diag.add%printers.starAR2463_CN.rte \
                           printer.starAR2463.add%printers.starAR2463_CN.rte

