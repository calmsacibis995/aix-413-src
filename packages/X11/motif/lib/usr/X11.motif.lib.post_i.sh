#!/bin/ksh
#  @(#)00        1.1  src/packages/X11/motif/lib/usr/X11.motif.lib.post_i.sh, pkgx, pkg411, 9439C411a 9/29/94 14:03:30
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.motif.lib (usr) post installation configuration
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
if [ -f /usr/lpp/X11/lib/R5/Motif1.2/compat_libMrm.a/shr.o ]; then
  ar r /usr/lpp/X11/lib/R5/Motif1.2/libMrm.a /usr/lpp/X11/lib/R5/Motif1.2/compat_libMrm.a/shr.o > /dev/null 2>&1 || exit 1
fi
if [ -f /usr/lpp/X11/lib/R5/Motif1.2/compat_libXm.a/shr.o ]; then
  ar r /usr/lpp/X11/lib/R5/Motif1.2/libXm.a /usr/lpp/X11/lib/R5/Motif1.2/compat_libXm.a/shr.o > /dev/null 2>&1 || exit 1
fi
if [ -f /usr/lpp/X11/lib/R5/Motif1.2/compat_libUil.a/shr.o ]; then
  ar r /usr/lpp/X11/lib/R5/Motif1.2/libUil.a /usr/lpp/X11/lib/R5/Motif1.2/compat_libUil.a/shr.o > /dev/null 2>&1 || exit 1
fi

# Save the remove until the end just in case there is a archive failure
# prior to this point.
rm -rf /usr/lpp/X11/lib/R5/Motif1.2/compat_libMrm.a
rm -rf /usr/lpp/X11/lib/R5/Motif1.2/compat_libXm.a
rm -rf /usr/lpp/X11/lib/R5/Motif1.2/compat_libUil.a

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
