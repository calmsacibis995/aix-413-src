# @(#)79        1.2  src/packages/X11/Dt/helpinfo/usr/X11.Dt.helpinfo.prereq.S, pkgxcde, pkg41J, 9518A_a 5/2/95 14:37:15
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
#   (C) COPYRIGHT International Business Machines Corp. 1993
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
*coreq X11.Dt.helpmin 4.1.3.0
