#!/bin/ksh
#  @(#)98        1.11  src/packages/X11/samples/apps/clients/usr/X11.samples.apps.clients.unpost_i.sh, pkgx, pkg41B, 412_41B_sync 12/21/94 10:52:50
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.apps.clients (usr) unpost installation configuration
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#   Remove customer courtesy links for sample clients
#     Remove symbolic links to /usr/lpp/X11/bin
#     Remove symbolic links to /usr/lpp/X11/include/X11/bitmaps
#     Remove symbolic links to /usr/lpp/X11/lib/X11/app-defaults
#----------------------------------------------------------------
X11bin="/usr/lpp/X11/bin"
clients="appres atobm bitmap bmtoa editres listres oclock twm viewres	\
		 xbiff xcalc xclipboard xconsole xcrtca xditview xdpyinfo xedit	\
		 xfd xfontsel xkill xload xlogo xlsatoms xlsclients xmag xman	\
		 xmh xprop xrefresh xsccd xstdcmap xwininfo"

for client in $clients; do
  rm -f $X11bin/$client
done

X11bitmapdir="/usr/lpp/X11/include/X11/bitmaps"
bitmaps="Dashes Down Excl FlipHoriz FlipVert Fold Left Right RotateLeft \
         RotateRight Stipple Term Up"

for bitmap in $bitmaps; do
  rm -f $X11bitmapdir/$bitmap
done

set -A ad_tgt	\
"/usr/lpp/X11/Xamples/man/appres.man"	\
"/usr/lpp/X11/Xamples/man/bitmap.man"	\
"/usr/lpp/X11/Xamples/man/editres.man"	\
"/usr/lpp/X11/Xamples/man/listres.man"	\
"/usr/lpp/X11/Xamples/man/oclock.man"	\
"/usr/lpp/X11/Xamples/man/viewres.man"	\
"/usr/lpp/X11/Xamples/man/xbiff.man"	\
"/usr/lpp/X11/Xamples/man/xcalc.man"	\
"/usr/lpp/X11/Xamples/man/xclipboard.man"	\
"/usr/lpp/X11/Xamples/man/xcutsel.man"	\
"/usr/lpp/X11/Xamples/man/xconsole.man"	\
"/usr/lpp/X11/Xamples/man/xcrtca.man"	\
"/usr/lpp/X11/Xamples/man/xsccd.man"	\
"/usr/lpp/X11/Xamples/man/xditview.man"	\
"/usr/lpp/X11/Xamples/man/xdpr.man"	\
"/usr/lpp/X11/Xamples/man/xdpyinfo.man"	\
"/usr/lpp/X11/Xamples/man/xedit.man"	\
"/usr/lpp/X11/Xamples/man/xfd.man"	\
"/usr/lpp/X11/Xamples/man/xfontsel.man"	\
"/usr/lpp/X11/Xamples/man/xkill.man"	\
"/usr/lpp/X11/Xamples/man/xload.man"	\
"/usr/lpp/X11/Xamples/man/xlogo.man"	\
"/usr/lpp/X11/Xamples/man/xlsatoms.man"	\
"/usr/lpp/X11/Xamples/man/xlscli.man"	\
"/usr/lpp/X11/Xamples/man/xmag.man"	\
"/usr/lpp/X11/Xamples/man/xman.man"	\
"/usr/lpp/X11/Xamples/man/xmh.man"	\
"/usr/lpp/X11/Xamples/man/xprop.man"	\
"/usr/lpp/X11/Xamples/man/xrefresh.man"	\
"/usr/lpp/X11/Xamples/man/xstdcmap.man"	\
"/usr/lpp/X11/Xamples/man/xwininfo.man"	\
"/usr/lpp/X11/Xamples/man/twm.man"	\
"/usr/lpp/X11/lib/X11/app-defaults/Bitmap-color"	\
"/usr/lpp/X11/lib/X11/app-defaults/Bitmap"	\
"/usr/lpp/X11/lib/X11/app-defaults/Editres-color"	\
"/usr/lpp/X11/lib/X11/app-defaults/Editres"	\
"/usr/lpp/X11/lib/X11/twm/system.twmrc"	\
"/usr/lpp/X11/lib/X11/app-defaults/Viewres"	\
"/usr/lpp/X11/lib/X11/app-defaults/XCalc-color"	\
"/usr/lpp/X11/lib/X11/app-defaults/XCalc"	\
"/usr/lpp/X11/lib/X11/app-defaults/XClipboard"	\
"/usr/lpp/X11/lib/X11/app-defaults/XConsole"	\
"/usr/lpp/X11/lib/X11/app-defaults/Clock-color"	\
"/usr/lpp/X11/lib/X11/app-defaults/Xfd"	\
"/usr/lpp/X11/lib/X11/app-defaults/Xedit"	\
"/usr/lpp/X11/lib/X11/app-defaults/XFontSel"	\
"/usr/lpp/X11/lib/X11/app-defaults/XLoad"	\
"/usr/lpp/X11/lib/X11/app-defaults/XLogo"	\
"/usr/lpp/X11/lib/X11/app-defaults/XLogo-color"	\
"/usr/lpp/X11/lib/X11/app-defaults/Xmag"	\
"/usr/lpp/X11/lib/X11/app-defaults/Xman"	\
"/usr/lpp/X11/lib/X11/app-defaults/Xmh"	\
"/usr/lpp/X11/include/X11/bitmaps/black6"	\
"/usr/lpp/X11/include/X11/bitmaps/box6"	\
"/usr/lpp/X11/lib/X11/app-defaults/Xditview-chrtr"	\
"/usr/lpp/X11/lib/X11/app-defaults/Xditview"	\
"/usr/lpp/X11/include/X11/bitmaps/ldblarrow"	\
"/usr/lpp/X11/include/X11/bitmaps/rdblarrow"

i=0
while ((i<${#ad_tgt[*]})) ; do
  rm -f ${ad_tgt[i]}
  ((i=i+1))
done

#----------------------------------------------------------------
#  remove X examples from Dt
#----------------------------------------------------------------

/etc/dtappintegrate -s /usr/lpp/X11/Xamples -l C -u > /dev/null 2>&1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
