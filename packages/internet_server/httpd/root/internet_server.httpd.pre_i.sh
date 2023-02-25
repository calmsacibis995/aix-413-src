#!/usr/bin/ksh
# @(#)54	1.1  src/packages/internet_server/httpd/root/internet_server.httpd.pre_i.sh, pkgweb, pkg41J, 9516B_all 4/19/95 18:18:57
#
# COMPONENT_NAME: pkgweb
#
# FUNCTIONS: none
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
# NAME:  root/internet_server.httpd.pre_i
#
# FUNCTION: script called from instal
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   
# ============================================================#
#
# This stops the deamon
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

/usr/bin/rmnotify -n httpd > /dev/null 2>&1
/usr/bin/rmssys -s httpd > /dev/null 2>&1

# ============================================================#
#
# This removes the /etc/inittab entry for /etc/rc.httpd
#
# ============================================================#
/usr/sbin/rmitab rchttpd > /dev/null 2>&1

exit 0
