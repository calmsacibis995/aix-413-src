#!/usr/bin/ksh
# @(#)51	1.1  src/packages/internet_server/httpd/root/internet_server.httpd.config.sh, pkgweb, pkg41J, 9516B_all 4/19/95 18:18:26
#
# COMPONENT_NAME: pkgweb
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
# NAME:	 internet_server.httpd.config.sh
#                                                                    
# FUNCTION: script called from instal to do special
# 	internet_server configuration
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  
# ============================================================#
#
# First stop any existing daemons in case a force overwrite install
#
# ============================================================#
stopsrc -c -s httpd > /dev/null 2>&1
if [ $? -ne 0 ]
then
	ps -eaf | grep /usr/sbin/httpd | awk ' { print $2 } ' | xargs kill -9 > /dev/null 2>&1
fi
# ============================================================#
#
# This adds the ODM stuff that we need for SRC
#
# ============================================================#
# If it is there already remove the old and add the new one
# so that the path is correct.
/usr/bin/rmssys -s httpd > /dev/null 2>&1
/usr/bin/rmnotify -n httpd > /dev/null 2>&1
/usr/bin/mkssys -s httpd -p /usr/sbin/httpd -u 0 -G tcpip
/usr/bin/mknotify -n httpd -m /etc/rc.httpd

/usr/sbin/lsitab rchttpd > /dev/null 2>&1
if [ $? -eq 1 ]
then
	/usr/sbin/mkitab -i rctcpip "rchttpd:2:wait:/etc/rc.httpd > /dev/console 2>&1 # Start HTTP daemon"
	if [ $? -eq 1 ]
	then
		echo "/usr/sbin/mkitab failed to add rchttpd to /etc/inittab"
		exit 1
	fi
fi

startsrc -s httpd
if [ $? -ne 0 ]
then
	echo "Unable to start httpd daemon"
fi

exit 0
