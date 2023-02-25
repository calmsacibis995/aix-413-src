#!/bin/bsh
# @(#)08	1.2  src/packages/bos/ifor_ls/server/root/bos.ifor_ls.server.config.sh, pkgnetls, pkg411, GOLD410 4/14/94 09:49:02
#
#   COMPONENT_NAME: pkgnetls
#
#   FUNCTIONS: Mkssys
#		Rmssys
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

Mkssys(){
	mkssys -s $1 -p$2 -G $3 -u 0 -S -n15 -f9
	if [ $? -ne 0 ]
	then
		inuumsg 115 mkssys $1 
                rc=1
        fi
}

Rmssys(){
	rmssys -s $1 2>&1 >/dev/null
}

rc=0

Rmssys netlsd
Mkssys netlsd /usr/lib/netls/ark/bin/netlsd netlsd

exit $rc
