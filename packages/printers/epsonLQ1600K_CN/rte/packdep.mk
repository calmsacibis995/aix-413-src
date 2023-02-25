# @(#)98	1.2  src/packages/printers/epsonLQ1600K_CN/rte/packdep.mk, pkgdevprint, pkg41J, 9507B 2/1/95 11:02:14
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

USR_LIBLPP_LIST		+= printers.epsonLQ1600K_CN.rte.pre_d

INFO_FILES		+= printers.epsonLQ1600K_CN.rte.prereq

USR_ODMADD_LIST         += printer.epsonLQ1600K.diag.add%printers.epsonLQ1600K_CN.rte \
                           printer.epsonLQ1600K.add%printers.epsonLQ1600K_CN.rte

