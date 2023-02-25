#!/bin/ksh
# @(#)04  1.7  src/bos/usr/sbin/fbcheck/fbcheck.sh, bosboot, bos411, 9439C411e 9/30/94 12:55:45
#
#   COMPONENT_NAME: BOSBOOT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#------------------------------- fbcheck ---------------------------------------
#
# NAME: fbcheck
#
# FUNCTION:
# if the /etc/firstboot script exists, this program renames it and executes it
# the /etc/firstboot script is the mechanism which is used during the
#	system's first boot to change a client's environment
#
# For Online software license display, this program will invoke the view and sign of 
# the license agreement if the current license has been signed with a 0000. This is used
# to allow the system to be set up preserving the capability for the end customer to view and
# sign the license.
#
# EXECUTION ENVIRONMENT:
# this file should ALWAYS have an entry in the /etc/inittab file, so that
# changes made with the	customization commands will be made the next time
# the client boots
#
# NOTES:
# refer to the corresponding man page or use InfoExplorer to obtain
#	detailed information about this command
#
#

FBNAME=/etc/fb_`date +"%H_%M_%m_%d"`

if [ -s "/etc/firstboot" ]
then
	# make sure it's executable
	chmod +x /etc/firstboot

	# rename this file so that it's not executed again
	mv /etc/firstboot $FBNAME

	# execute this shell script
	$FBNAME
fi
if [ -s "/.license.sig.tmp"  ] ; then
	NAME=`cat /.license.sig.tmp`
	if [ "$NAME" = "0000" ] ; then
		> /var/adm/.license.sig
		export SOFTLIC_ONLY=1
		/usr/sbin/install_assist
	fi
fi
