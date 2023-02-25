#!/usr/bin/ksh
# @(#)55	1.1  src/packages/internet_server/httpd/root/internet_server.httpd.unconfig.sh, pkgweb, pkg41J, 9516B_all 4/19/95 18:19:07
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
# NAME:	 internet_server.httpd.sh
#                                                                    
# FUNCTION: script called from instal to do special httpd configuration
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
# ============================================================#
#
# This the deamon
#
# ============================================================#
stopsrc -c -s httpd > /dev/null 2>&1
if [ $? -ne 0 ]
then
	ps -eaf | grep httpd | awk ' { print $2 } ' | xargs kill -9 > /dev/null 2>&1
fi
# ============================================================#
#
# This removes the ODM stuff that we need for SRC
#
# ============================================================#

/usr/bin/rmnotify -n httpd
/usr/bin/rmssys -s httpd

# ============================================================#
#
# This removes the /etc/inittab entry for /etc/rc.httpd
#
# ============================================================#
/usr/sbin/rmitab rchttpd

exit 0
