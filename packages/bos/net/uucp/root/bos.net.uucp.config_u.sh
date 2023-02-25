#!/usr/bin/ksh
# @(#)03	1.1  src/packages/bos/net/uucp/root/bos.net.uucp.config_u.sh, pkguucp, pkg412, 9446B 11/15/94 16:41:07
#
#   COMPONENT_NAME: pkguucp
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
SECFILE=/etc/security/login.cfg

chown root.security $SECFILE
if [ $? -ne 0 ] ; then
	# File chown failed
	echo "chown root.security $SECFILE failed"
	exit 1
fi
exit 0
