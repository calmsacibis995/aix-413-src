#!/bin/ksh
#  @(#)60        1.1  src/packages/X11/base/lib/usr/X11.base.lib.pre_rm.sh, pkgx, pkg411, 9439C411a 9/28/94 17:12:26
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.base.lib (usr) pre_rm installation configuration
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
OPTION=X11.base.lib
grep $OPTION $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  ODMDIR=/usr/lib/objrepos odmget -q"lpp_name=X11.compat.lib.X11R3 and state=5" product | grep "X11.compat.lib.X11R3" > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    mkdir -p /usr/lpp/X11/lib/R5/compat_libX11.a > /dev/null 2>&1 || exit 1
    ar x /usr/lpp/X11/lib/R5/libX11.a /usr/lpp/X11/lib/R5/compat_libX11.a/shr.o > /dev/null 2>&1 || exit 1
    mkdir -p /usr/lpp/X11/lib/R5/compat_libXt.a > /dev/null 2>&1 || exit 1
    ar x /usr/lpp/X11/lib/R5/libXt.a /usr/lpp/X11/lib/R5/compat_libXt.a/shr.o > /dev/null 2>&1 || exit 1
    mkdir -p /usr/lpp/X11/lib/R5/compat_liboldX.a > /dev/null 2>&1 || exit 1
    ar x /usr/lpp/X11/lib/R5/liboldX.a /usr/lpp/X11/lib/R5/compat_liboldX.a/shr.o > /dev/null 2>&1 || exit 1
  fi
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
