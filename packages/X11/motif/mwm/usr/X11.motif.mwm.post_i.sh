#!/bin/ksh
#  @(#)17        1.3  src/packages/X11/motif/mwm/usr/X11.motif.mwm.post_i.sh, pkgx, pkg411, 9437C411a 9/13/94 17:27:17
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.motif.mwm (usr) post installation configuration
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
OPTION=X11.motif.mwm
grep "X11rte.obj" $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then

  # migration is from 3.2; cfgfiles must be handled manually
  XLIBDIR=/usr/lpp/X11/lib/X11
  if [ -f $XLIBDIR/system.mwmrc -a ! -L $XLIBDIR/system.mwmrc ]; then

    # Motif 1.1.4 has not relocated
    mkdir -p ${MIGSAVE}${XLIBDIR}/app-defaults
    cp $XLIBDIR/system.mwmrc ${MIGSAVE}${XLIBDIR} > /dev/null 2>&1
    cp $XLIBDIR/app-defaults/Mwm ${MIGSAVE}${XLIBDIR}/app-defaults > /dev/null 2>&1
  else
    # Motif 1.1.4 has been relocated.  Save config files if they exist...

    # Motif 1.1.4 must be user merged.
    M114DIR=/usr/lpp/X11/Motif1.1.4/app-defaults
    mkdir -p ${MIGSAVE}${M114DIR}
    cp $M114DIR/Mwm ${MIGSAVE}${M114DIR} > /dev/null 2>&1
    cp $M114DIR/system.mwmrc ${MIGSAVE}${M114DIR} > /dev/null 2>&1

    # Motif 1.2 will be preserved.
    mkdir -p ${MIGSAVE}/usr/lpp/X11/defaults/Motif1.2
    cp /usr/lpp/X11/Motif1.2/app-defaults/Mwm ${MIGSAVE}/usr/lpp/X11/defaults/Motif1.2 > /dev/null 2>&1
    cp /usr/lpp/X11/Motif1.2/app-defaults/system.mwmrc ${MIGSAVE}/usr/lpp/X11/defaults/Motif1.2 > /dev/null 2>&1
  fi
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
