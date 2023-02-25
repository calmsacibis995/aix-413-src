# @(#)70        1.1  src/packages/X11/base/lib/usr/X11.base.lib.insize.S, pkgx, pkg411, 9439C411a 9/29/94 13:23:10
#
# COMPONENT_NAME: pkgx
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
# The following size requirements are for archiving the X11R3 compatibility
# libraries during a system migration, when necessary.  The size calculations
# are as follows.  Some padding has been included to address some growth in
# the libraries.  Accounting for growth in important since new install images
# could be generated for future releases.
#
# /usr/lpp/X11/lib/R5 - largest_compat_member (padded by 10%)
# /usr/lpp/X11/lib/R5/compat_lib - compat_member (padded by 10%)
#
/usr/lpp/X11/lib/R5 0 595
/usr/lpp/X11/lib/R5/compat_libX11.a 0 593
/usr/lpp/X11/lib/R5/compat_libXt.a 0 595
/usr/lpp/X11/lib/R5/compat_liboldX.a 0 43
