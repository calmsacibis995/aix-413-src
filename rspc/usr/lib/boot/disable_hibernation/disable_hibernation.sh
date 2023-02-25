#!/usr/bin/ksh
#
# @(#)66        1.1  src/rspc/usr/lib/boot/disable_hibernation/disable_hibernation.sh, pwrcmd, rspc41J, 9517B_all 4/9/95 23:04:08
#
# COMPONENT_NAME: (PWRCMD) Power Management Commands
#
# FUNCTIONS: disable_hibernation
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# disable_hibernation
#
if [ `basename $0` = "disable_hibernation" ]; then
	#
	# add pmmi_data
	#
	/usr/bin/odmadd <<- EOF
		pmmi_data:
			attribute=disable_hiber
			value1=1
			value2=1
	EOF
	if [ $? -ne 0 ]; then
		exit 1
	fi
#
# enable_hibernation
#
elif [ `basename $0` = "enable_hibernation" ]; then
	#
	# delete pmmi_data
	#
	/usr/bin/odmdelete -o pmmi_data -q 'attribute=disable_hiber' \
					> /dev/null 2>&1
	if [ $? -ne 0 ]; then
		exit 1
	fi
#
# error exit
#
else
	exit 1
fi
#
# execute bosboot
#
/usr/sbin/bosboot -ad /dev/ipldevice
