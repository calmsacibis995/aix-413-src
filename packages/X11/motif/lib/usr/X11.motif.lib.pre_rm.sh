#!/bin/ksh
#  @(#)66        1.1  src/packages/X11/motif/lib/usr/X11.motif.lib.pre_rm.sh, pkgx, pkg411, 9439C411a 9/29/94 08:15:33
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.motif.lib (usr) pre_rm installation configuration
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
#  Save compatibility library shr.o members for migration,
#  if necessary
#----------------------------------------------------------------
OPTION=X11.motif.lib
grep $OPTION $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  ODMDIR=/usr/lib/objrepos odmget -q"lpp_name=X11.compat.lib.Motif10 and state=5" product | grep "X11.compat.lib.Motif10" > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    mkdir -p /usr/lpp/X11/lib/R5/Motif1.2/compat_libMrm.a > /dev/null 2>&1 || exit 1
    ar x /usr/lpp/X11/lib/R5/Motif1.2/libMrm.a /usr/lpp/X11/lib/R5/Motif1.2/compat_libMrm.a/shr.o > /dev/null 2>&1 || exit 1
    mkdir -p /usr/lpp/X11/lib/R5/Motif1.2/compat_libXm.a > /dev/null 2>&1 || exit 1
    ar x /usr/lpp/X11/lib/R5/Motif1.2/libXm.a /usr/lpp/X11/lib/R5/Motif1.2/compat_libXm.a/shr.o > /dev/null 2>&1 || exit 1
    mkdir -p /usr/lpp/X11/lib/R5/Motif1.2/compat_libUil.a > /dev/null 2>&1 || exit 1
    ar x /usr/lpp/X11/lib/R5/Motif1.2/libUil.a /usr/lpp/X11/lib/R5/Motif1.2/compat_libUil.a/shr.o > /dev/null 2>&1 || exit 1
  fi
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
