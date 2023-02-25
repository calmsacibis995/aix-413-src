#!/usr/bin/ksh
# @(#)57        1.7  src/packages/bos/powermgt/rte/root/bos.powermgt.rte.config.sh, pkgpwrmgt, pkg41J, 9523C_all 6/9/95 13:53:32
# COMPONENT_NAME: pkgpwrmgt
#
# FUNCTIONS: config.sh
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
# NAME:	 bos.powermgt.rte.config.sh
#
# FUNCTION: script called from instal to add PM daemon to /etc/inittab.
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
# 

#
# delete old Config_Rule for phase 1
#
odmdelete -q 'rule=/etc/methods/startpm' -o Config_Rules > /dev/null 2>&1

#
# check if the machine has PM controller
#
/usr/bin/checkpmc
if [ $? -ne 0 ]
then
	exit 0		# do nothing
fi

#
# add pmd entry (for PM daemon) to /etc/inittab
#
RC=0
/usr/sbin/lsitab pmd > /dev/null 2>&1
if [ $? -eq 1 ]
then
	#
	# get PCMCIA auto config daemon entry. It must be after the pmd entry.
	#
	ACFGD_ENTRY=`/usr/sbin/lsitab acfgd`
	if [ $? -eq 1 ]
	then
		/usr/sbin/mkitab "pmd:2:wait:/usr/bin/pmd > \
/dev/console 2>&1 # Start PM daemon"
		RC=$?
	else
		/usr/sbin/rmitab acfgd > /dev/null 2>&1
		/usr/sbin/mkitab "pmd:2:wait:/usr/bin/pmd > \
/dev/console 2>&1 # Start PM daemon"
		RC=$?
		/usr/sbin/mkitab "$ACFGD_ENTRY" > /dev/null 2>&1
	fi
	if [ $RC -eq 1 ]
	then
		echo "/usr/sbin/mkitab failed to add pmd to /etc/inittab"
		exit 1
	fi
fi

#
# create a hibernation logical volume
#
/usr/bin/mkpmhlv -m

exit 0
