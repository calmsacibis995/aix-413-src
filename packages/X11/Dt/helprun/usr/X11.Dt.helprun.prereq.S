# @(#)40        1.8  src/packages/X11/Dt/helprun/usr/X11.Dt.helprun.prereq.S, pkgxcde, pkg41J, 9520A_all 5/15/95 11:50:45
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: prereq
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#[type] [option] [expression] {o [expression]
#       [type] - *prereq, *coreq, *ifreq
#       [option] - full name of option with requisite
#       [expression]    "=","<",">"
#               v - version
#               r - release
#               m - modification number
#               f - fix number
#               p - update of PTF number
#       {o [expression]} - or condition for the option
#
*coreq X11.base.rte 4.1.0.0
*coreq X11.base.lib 4.1.0.0
*coreq X11.motif.lib 4.1.3.0
*coreq X11.Dt.lib 4.1.3.0
*coreq X11.Dt.helpmin 4.1.3.0
