#!/bin/ksh
#  @(#)97        1.15  src/packages/X11/samples/apps/clients/usr/X11.samples.apps.clients.post_i.sh, pkgx, pkg411, 9438B411a 9/20/94 11:57:39
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.apps.clients (usr) post installation configuration
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
#  Determine migration level cfgfiles required
#----------------------------------------------------------------
OPTION=X11.samples.apps.clients
grep "X11dev.src 1.2.0.0" $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  mv $OPTION.R4cfgfiles $OPTION.cfgfiles
else
  grep $OPTION $OPTION.installed_list > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    mv $OPTION.41cfgfiles $OPTION.cfgfiles
  fi
fi

#----------------------------------------------------------------
#  Add copyright information to necessary config files during
#  migration.  This will prevent a user_merge requirement.
#----------------------------------------------------------------
grep "X11dev.src 1.2.3.0" $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
set -A mig_src	\
"/usr/lpp/X11/Xamples/clients/bitmap/Bitmap-co.ad"	\
"/usr/lpp/X11/Xamples/clients/bitmap/Bitmap.ad"	\
"/usr/lpp/X11/Xamples/clients/editres/Edit-col.ad"	\
"/usr/lpp/X11/Xamples/clients/editres/Editres.ad"	\
"/usr/lpp/X11/Xamples/clients/oclock/Clock-col.ad"	\
"/usr/lpp/X11/Xamples/clients/viewres/Viewres.ad"	\
"/usr/lpp/X11/Xamples/clients/xcalc/XCalc-col.ad"	\
"/usr/lpp/X11/Xamples/clients/xcalc/XCalc.ad"	\
"/usr/lpp/X11/Xamples/clients/xclipboard/XClipboard.ad"	\
"/usr/lpp/X11/Xamples/clients/xconsole/XConsole.ad"	\
"/usr/lpp/X11/Xamples/clients/xditview/Xdit-chrtr.ad"	\
"/usr/lpp/X11/Xamples/clients/xditview/Xditview.ad"	\
"/usr/lpp/X11/Xamples/clients/xedit/Xedit.ad"	\
"/usr/lpp/X11/Xamples/clients/xfd/Xfd.ad"	\
"/usr/lpp/X11/Xamples/clients/xfontsel/XFontSel.ad"	\
"/usr/lpp/X11/Xamples/clients/xload/XLoad.ad"	\
"/usr/lpp/X11/Xamples/clients/xlogo/XLogo.ad"	\
"/usr/lpp/X11/Xamples/clients/xlogo/XLogo-co.ad"	\
"/usr/lpp/X11/Xamples/clients/xmag/Xmag.ad"	\
"/usr/lpp/X11/Xamples/clients/xman/Xman.ad"	\
"/usr/lpp/X11/Xamples/clients/xmh/Xmh.ad"

i=0
while ((i<${#mig_src[*]})) ; do
if [ -f $MIGSAVE${mig_src[i]} ]; then
mv $MIGSAVE${mig_src[i]} $MIGSAVE${mig_src[i]}.orig
echo "\
!
! COMPONENT_NAME: X11
!
! FUNCTIONS: app-defaults file
!
! ORIGINS: 16, 27
!
! (C) COPYRIGHT International Business Machines Corp. 1987,1994
! All Rights Reserved
! Licensed Materials - Property of IBM
!
! US Government Users Restricted Rights - Use, duplication or
! disclosure restricted by GSA ADP Schedule Contract with IBM Corp."  > $MIGSAVE${mig_src[i]}
cat $MIGSAVE${mig_src[i]}.orig >> $MIGSAVE${mig_src[i]}
rm -f $MIGSAVE${mig_src[i]}.orig
fi
((i=i+1))
done
fi

#----------------------------------------------------------------
#   Add customer courtesy links for sample clients
#     Add symbolic links to /usr/lpp/X11/bin
#     Add symbolic links to /usr/lpp/X11/include/X11/bitmaps
#     Add symbolic links to /usr/lpp/X11/lib/X11/app-defaults
#       * ad_src and ad_tgt arrays must match 1-to-1
#----------------------------------------------------------------
X11bin="/usr/lpp/X11/bin"
samplesbin="/usr/lpp/X11/Xamples/bin"
clients="appres atobm bitmap bmtoa editres listres oclock twm viewres	\
		 xbiff xcalc xclipboard xconsole xcrtca xditview xdpyinfo xedit	\
		 xfd xfontsel xkill xload xlogo xlsatoms xlsclients xmag xman	\
		 xmh xprop xrefresh xsccd xstdcmap xwininfo"

for client in $clients; do
  rm -f $X11bin/$client || exit 1
  ln -sf $samplesbin/$client $X11bin/$client || exit 1
done

X11bitmapdir="/usr/lpp/X11/include/X11/bitmaps"
bitmapdir="/usr/lpp/X11/Xamples/clients/bitmap"
bitmaps="Dashes Down Excl FlipHoriz FlipVert Fold Left Right RotateLeft	\
		 RotateRight Stipple Term Up"

for bitmap in $bitmaps; do
  rm -f $X11bitmapdir/$bitmap || exit 1
  ln -sf $bitmapdir/$bitmap $X11bitmapdir/$bitmap || exit 1
done

set -A ad_src	\
"/usr/lpp/X11/Xamples/clients/appres/appres.man"	\
"/usr/lpp/X11/Xamples/clients/bitmap/bitmap.man"	\
"/usr/lpp/X11/Xamples/clients/editres/editres.man"	\
"/usr/lpp/X11/Xamples/clients/listres/listres.man"	\
"/usr/lpp/X11/Xamples/clients/oclock/oclock.man"	\
"/usr/lpp/X11/Xamples/clients/viewres/viewres.man"	\
"/usr/lpp/X11/Xamples/clients/xbiff/xbiff.man"	\
"/usr/lpp/X11/Xamples/clients/xcalc/xcalc.man"	\
"/usr/lpp/X11/Xamples/clients/xclipboard/xclipboard.man"	\
"/usr/lpp/X11/Xamples/clients/xclipboard/xcutsel.man"	\
"/usr/lpp/X11/Xamples/clients/xconsole/xconsole.man"	\
"/usr/lpp/X11/Xamples/clients/xcrtca/xcrtca.man"	\
"/usr/lpp/X11/Xamples/clients/xcrtca/xsccd.man"	\
"/usr/lpp/X11/Xamples/clients/xditview/xditview.man"	\
"/usr/lpp/X11/Xamples/clients/xdpr/xdpr.man"	\
"/usr/lpp/X11/Xamples/clients/xdpyinfo/xdpyinfo.man"	\
"/usr/lpp/X11/Xamples/clients/xedit/xedit.man"	\
"/usr/lpp/X11/Xamples/clients/xfd/xfd.man"	\
"/usr/lpp/X11/Xamples/clients/xfontsel/xfontsel.man"	\
"/usr/lpp/X11/Xamples/clients/xkill/xkill.man"	\
"/usr/lpp/X11/Xamples/clients/xload/xload.man"	\
"/usr/lpp/X11/Xamples/clients/xlogo/xlogo.man"	\
"/usr/lpp/X11/Xamples/clients/xlsatoms/xlsatoms.man"	\
"/usr/lpp/X11/Xamples/clients/xlsclients/xlscli.man"	\
"/usr/lpp/X11/Xamples/clients/xmag/xmag.man"	\
"/usr/lpp/X11/Xamples/clients/xman/xman.man"	\
"/usr/lpp/X11/Xamples/clients/xmh/xmh.man"	\
"/usr/lpp/X11/Xamples/clients/xprop/xprop.man"	\
"/usr/lpp/X11/Xamples/clients/xrefresh/xrefresh.man"	\
"/usr/lpp/X11/Xamples/clients/xstdcmap/xstdcmap.man"	\
"/usr/lpp/X11/Xamples/clients/xwininfo/xwininfo.man"	\
"/usr/lpp/X11/Xamples/clients/twm/twm.man"	\
"/usr/lpp/X11/Xamples/clients/bitmap/Bitmap-co.ad"	\
"/usr/lpp/X11/Xamples/clients/bitmap/Bitmap.ad"	\
"/usr/lpp/X11/Xamples/clients/editres/Edit-col.ad"	\
"/usr/lpp/X11/Xamples/clients/editres/Editres.ad"	\
"/usr/lpp/X11/Xamples/clients/twm/system.twmrc"	\
"/usr/lpp/X11/Xamples/clients/viewres/Viewres.ad"	\
"/usr/lpp/X11/Xamples/clients/xcalc/XCalc-col.ad"	\
"/usr/lpp/X11/Xamples/clients/xcalc/XCalc.ad"	\
"/usr/lpp/X11/Xamples/clients/xclipboard/XClipboard.ad"	\
"/usr/lpp/X11/Xamples/clients/xconsole/XConsole.ad"	\
"/usr/lpp/X11/Xamples/clients/oclock/Clock-col.ad"	\
"/usr/lpp/X11/Xamples/clients/xfd/Xfd.ad"	\
"/usr/lpp/X11/Xamples/clients/xedit/Xedit.ad"	\
"/usr/lpp/X11/Xamples/clients/xfontsel/XFontSel.ad"	\
"/usr/lpp/X11/Xamples/clients/xload/XLoad.ad"	\
"/usr/lpp/X11/Xamples/clients/xlogo/XLogo.ad"	\
"/usr/lpp/X11/Xamples/clients/xlogo/XLogo-co.ad"	\
"/usr/lpp/X11/Xamples/clients/xmag/Xmag.ad"	\
"/usr/lpp/X11/Xamples/clients/xman/Xman.ad"	\
"/usr/lpp/X11/Xamples/clients/xmh/Xmh.ad"	\
"/usr/lpp/X11/Xamples/clients/xmh/black6"	\
"/usr/lpp/X11/Xamples/clients/xmh/box6"	\
"/usr/lpp/X11/Xamples/clients/xditview/Xdit-chrtr.ad"	\
"/usr/lpp/X11/Xamples/clients/xditview/Xditview.ad"	\
"/usr/lpp/X11/Xamples/clients/xditview/ldblarrow"	\
"/usr/lpp/X11/Xamples/clients/xditview/rdblarrow"

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
while ((i<${#ad_src[*]})) ; do
  rm -f ${ad_tgt[i]} || exit 1
  ln -sf ${ad_src[i]} ${ad_tgt[i]} || exit 1
  ((i=i+1))
done
#----------------------------------------------------------------
#  integrate X examples into Dt
#----------------------------------------------------------------

/etc/dtappintegrate -s /usr/lpp/X11/Xamples -l C > /dev/null 2>&1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
