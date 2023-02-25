#!/bin/ksh
#  @(#)01        1.1  src/packages/X11/base/lib/usr/X11.base.lib.post_i.sh, pkgx, pkg411, 9439C411a 9/29/94 14:03:35
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.base.lib (usr) post installation configuration
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#  Archive compatibility shr.o members if necessary
#----------------------------------------------------------------
if [ -f /usr/lpp/X11/lib/R5/compat_libX11.a/shr.o ]; then
  ar r /usr/lpp/X11/lib/R5/libX11.a /usr/lpp/X11/lib/R5/compat_libX11.a/shr.o > /dev/null 2>&1 || exit 1
fi
if [ -f /usr/lpp/X11/lib/R5/compat_libXt.a/shr.o ]; then
  ar r /usr/lpp/X11/lib/R5/libXt.a /usr/lpp/X11/lib/R5/compat_libXt.a/shr.o > /dev/null 2>&1 || exit 1
fi
if [ -f /usr/lpp/X11/lib/R5/compat_liboldX.a/shr.o ]; then
  ar r /usr/lpp/X11/lib/R5/liboldX.a /usr/lpp/X11/lib/R5/compat_liboldX.a/shr.o > /dev/null 2>&1 || exit 1
fi

# Save the remove until the end just in case there is a archive failure
# prior to this point.
rm -rf /usr/lpp/X11/lib/R5/compat_libX11.a
rm -rf /usr/lpp/X11/lib/R5/compat_libXt.a
rm -rf /usr/lpp/X11/lib/R5/compat_liboldX.a

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
