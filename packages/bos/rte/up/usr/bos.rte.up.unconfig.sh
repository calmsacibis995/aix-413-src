# @(#)62        1.2  src/packages/bos/rte/up/usr/bos.rte.up.unconfig.sh, pkgbos, pkg411, GOLD410 8/23/93 15:47:38
#
#   COMPONENT_NAME: pkgbos
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# The bootinfo -z command will return 0 if UP system or 1 if MP system
# The links should never be removed if this is a UP system

RV=`bootinfo -z`
if [ $RV -eq 0 ]
then
	exit 1
fi
