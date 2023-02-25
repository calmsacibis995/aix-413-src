# @(#)35        1.2  src/packages/X11/Dt/bitmaps/usr/X11.Dt.bitmaps.insize.S, pkgxcde, pkg41J, 9517A_all 4/14/95 13:06:31
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: insize
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
#
# The calculation for the amount of space in /usr/lpp/X11.Dt
# is for the maximum size of the X11.Dt.bitmaps.cfgfiles
# that will be built at install time by
# the pre_rm script for the X11.Dt.bitmaps fileset in the
# /usr/lpp/X11.Dt directory.
# This file is removed after the install process is complete
# so temporary space should be reserved.
# This calculation assumes a maximum of 1171 icon files from
# 4.1.1/4.1.2 and a maximum path name of 45 characters +
# length of " user_merge".
# Size = 1171 * (45 + 11) = 65576
# Since files are allocated in multiples of 4096, number of
# 512-byte blocks = 136
/usr/lpp/X11.Dt +0 136
