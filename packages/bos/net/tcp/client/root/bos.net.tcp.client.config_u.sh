#!/usr/bin/ksh
# @(#)05	1.4  src/packages/bos/net/tcp/client/root/bos.net.tcp.client.config_u.sh, pkgtcpip, pkg41J, 9512A_all 3/9/95 11:21:55
# COMPONENT_NAME: pkgtcpip
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME:	 bos.net.tcp.client.config_u.sh
#                                                                    
# FUNCTION: script called from instal to do special tcpip configuration
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  
/usr/bin/rmssys -s iptrace > /dev/null 2>&1
/usr/bin/mkssys -s iptrace -p /usr/sbin/iptrace -u 0 -G tcpip

SECFILE=/etc/security/login.cfg

chown root.security $SECFILE
if [ $? -ne 0 ] ; then
	# File chown failed
	echo "chown root.security $SECFILE failed"
	exit 1
fi
exit 0
