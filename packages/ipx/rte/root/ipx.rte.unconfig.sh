#!/bin/ksh
# @(#)99 1.1  src/packages/ipx/rte/root/ipx.rte.unconfig.sh, pkgnetware, pkg411, 9438B411a 9/14/94 14:19:16
#
#   COMPONENT_NAME: NETW_INSTALL
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

lsitab "rcnetw" > /dev/null 2>&1
if [ $? -ne 0 ]    # The entry is not in the table.
then
	rmitab "rcnetw" 
fi

exit 0
