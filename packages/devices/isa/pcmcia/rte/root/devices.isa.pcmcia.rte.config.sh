#!/usr/bin/ksh
# @(#)08	1.2  src/packages/devices/isa/pcmcia/rte/root/devices.isa.pcmcia.rte.config.sh, pkgrspc, pkg41J, 9512A_all 3/16/95 17:17:05
#
#   COMPONENT_NAME: PKGRSPC
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PLATFORM=`/usr/sbin/bootinfo -T`
if [ "$PLATFORM" != "rspc" ]
then
	exit 0		# do nothing
fi

/usr/sbin/lsitab acfgd > /dev/null 2>&1
if [ $? -eq 1 ]
then
	/usr/sbin/mkitab "acfgd:2:wait:/usr/sbin/lacfgd > /dev/console 2>&1 # Start auto-config daemon"
	if [ $? -eq 1 ]
	then
		echo "/usr/sbin/mkitab failed to add acfgd to /etc/inittab"
		exit 1
	fi
fi

exit 0
