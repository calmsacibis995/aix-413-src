#!/usr/bin/ksh
# @(#)58        1.3  src/packages/bos/powermgt/rte/root/bos.powermgt.rte.unconfig.sh, pkgpwrmgt, pkg41J, 9510A_all 3/8/95 05:07:16
# COMPONENT_NAME: pkgpwrmgt
#
# FUNCTIONS: unconfig.sh
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME:	 bos.powermgt.rte.unconfig.sh
#
# FUNCTION: script called from instal to remove PM daemon from /etc/inittab.
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
# 

#
# check if the machine has PM controller
#
/usr/bin/checkpmc
if [ $? -ne 0 ]
then
        exit 0          # do nothing
fi

#
# if PM pahse 1 was installed, the Config_Rule needs to be recovered. 
#
if [ -f /etc/objrepos/pmmd_start ]
then
	cat > /tmp/unconfig.pkgpwrmgt.$$ << EOF
Config_Rules:
	phase = 2
	seq = 7
	boot_mask =0
	rule = /etc/methods/startpm
EOF
	odmadd /tmp/unconfig.pkgpwrmgt.$$ > /dev/null 2>&1
	rm -f /tmp/unconfig.pkgpwrmgt.$$
fi

#
# if PM pahse 1 was not installed, the pmd in inttab entry needs to be removed.
#
if [ ! -f /etc/objrepos/pmmd_start ]
then
	/usr/sbin/rmitab pmd > /dev/null 2>&1
fi

#
# remove the hibernation logical volume
#
/usr/sbin/rmlv -f pmhibernation

exit $?
