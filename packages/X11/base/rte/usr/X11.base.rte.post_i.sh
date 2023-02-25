#!/bin/ksh
#  @(#)96        1.8  src/packages/X11/base/rte/usr/X11.base.rte.post_i.sh, pkgx, pkg411, GOLD410 5/13/94 10:46:45
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.base.rte (usr) post installation configuration
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
#  Perform Client, Server and auto-install configuration
#----------------------------------------------------------------
BNDDIR=/usr/sys/inst.data/sys_bundles
if [ -f $BNDDIR/Client.def ]; then
  grep -s -q X11.apps.clients $BNDDIR/Client.def
  if [ $? -ne 0 ]; then
    cat $BNDDIR/GOS.client >> $BNDDIR/Client.def
    if [ $? -ne 0 ]; then
	  exit 1
    fi
  fi
fi
if [ -f $BNDDIR/Server.def ]; then
  grep -s -q X11.apps.clients $BNDDIR/Server.def
  if [ $? -ne 0 ]; then
    cat $BNDDIR/GOS.client $BNDDIR/GOS.server >> $BNDDIR/Server.def
    if [ $? -ne 0 ]; then
      exit 1
    fi
  fi
fi
if [ -f $BNDDIR/App-Dev.def ]; then
  grep -s -q X11.apps.clients $BNDDIR/App-Dev.def
  if [ $? -ne 0 ]; then
    cat $BNDDIR/GOS.client $BNDDIR/GOS.adt >> $BNDDIR/App-Dev.def
    if [ $? -ne 0 ]; then
      exit 1
    fi
  fi
fi
rm -f $BNDDIR/GOS.client
rm -f $BNDDIR/GOS.server
rm -f $BNDDIR/GOS.adt

#----------------------------------------------------------------
#  Determine migration level cfgfiles required
#----------------------------------------------------------------
OPTION=X11.base.rte
grep $OPTION $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  mv $OPTION.41cfgfiles $OPTION.cfgfiles
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
