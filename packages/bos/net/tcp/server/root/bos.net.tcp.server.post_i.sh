#!/bin/ksh
# @(#)97	1.2  src/packages/bos/net/tcp/server/root/bos.net.tcp.server.post_i.sh, pkgtcpip, pkg411, GOLD410 4/11/94 15:22:27
#
#   COMPONENT_NAME: pkgtcp
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
#


#
# When installing 325 -> 410, leave the /etc/gated.conf file in the save 
# directory.  When install 410 -> 410, move the /etc/gated.conf file from 
# the save directory to the /etc directory.  This is need because one cannot 
# write in the /usr file system when installing diskless.
#

if [ -n "$MIGSAVE" ] && [ -n "$INSTALLED_LIST" ]
then
    grep -q bos.net.tcp.server $INSTALLED_LIST
    rc2=$?

    if [ $rc2 -eq 0 ]
    then
# Move $MIGSAVE/etc/gated.conf back to /etc/gated.conf when 410 -> 410.
	mv $MIGSAVE/etc/gated.conf /etc/gated.conf 2>&1 > /dev/null
    fi
fi

exit 0
