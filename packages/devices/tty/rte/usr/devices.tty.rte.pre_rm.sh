#!/bin/ksh          
# @(#)50        1.3  src/packages/devices/tty/rte/usr/devices.tty.rte.pre_rm.sh, pkgdevtty, pkg411, 9438C411b 9/21/94 17:06:04
#
# COMPONENT_NAME: pkgdevtty
#
# FUNCTIONS:  pre_rm
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# -------------------------------------------------------------------------
#
# LPP_NAME: devices.tty.rte
#
ODMDIR=/usr/lib/objrepos odmdelete -q "loc0=/usr/lbin/tty/stty-rs" -o inventory >/dev/null 2>&1


APPLYLIST=/usr/lpp/devices.mca.edd0/deinstl/devices.mca.edd0.com.al
if [  -f "$APPLYLIST" ] ; then
	if egrep 'stty-rs' $APPLYLIST 1>/dev/null 2>&1
	then
		sed -e "/stty-rs/d" $APPLYLIST >./apply.tmp 2>&1
		mv ./apply.tmp $APPLYLIST
	fi
fi
    
