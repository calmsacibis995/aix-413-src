# !/bin/bsh
# @(#)09	1.1  src/packages/bos/rcs/usr/bos.rcs.pre_rm.sh, pkgrcs, pkg411, GOLD410 6/10/94 08:43:41
#
# COMPONENT_NAME: pkgrcs
#
# FUNCTIONS:  pre_rm.sh
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
# Function: script to remove references to AIXServ
#
# LPP_NAME: bos.rcs
#
if [ -f /usr/bin/rmusrprof ] ; then
	cd /etc/objrepos
	/usr/bin/rmusrprof -a -ms
	exit $?
else
	exit 0
fi
