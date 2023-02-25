#!/bin/ksh
#  @(#)42        1.3  src/packages/X11/apps/custom/usr/X11.apps.custom.pre_i.sh, pkgx, pkg411, GOLD410 6/15/94 10:08:58
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.apps.custom (usr) pre installation configuration
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
#  Remove /usr/lpp/X11/lib/X11/app-custom directory if it exists
#  as a true directory.  It is becoming a symlink if 4.1.  This
#  is necessary for a migration installation.
#----------------------------------------------------------------
XCUSTOMDIR=/usr/lpp/X11/lib/X11/app-custom
CUSTOMDIR=/usr/lpp/X11/custom/lib/app-custom

# If directory exists and is not a symlink already...
if [ -d $XCUSTOMDIR -a ! -L $XCUSTOMDIR ]; then
  for f in $XCUSTOMDIR/* $XCUSTOMDIR/.*; do
    # If customer placed any files in directory (bypass "." and "..")...
    if [ ! -L $f -a "$f" != "$XCUSTOMDIR/." -a "$f" != "$XCUSTOMDIR/.." -a "$f" != "$XCUSTOMDIR/*" ]; then
      # ...Move them to standard custom directory
      mv $f $CUSTOMDIR >/dev/null 2>&1 || cp -rp $f $CUSTOMDIR >/dev/null 2>&1 || exit 1
    fi
  done
  rm -rf $XCUSTOMDIR || exit 1
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
