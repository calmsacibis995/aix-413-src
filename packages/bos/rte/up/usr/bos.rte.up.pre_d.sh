# @(#)28        1.2  src/packages/bos/rte/up/usr/bos.rte.up.pre_d.sh, pkgbos, pkg411, GOLD410 3/31/94 10:51:19
#
#   COMPONENT_NAME: pkgbos
#
#   FUNCTIONS: bos.rte.up.pre_d
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# The bootinfo -z command will return 0 if UP system or 1 if MP system
# This option can only be deinstalled if this is a MP system.

RV=`bootinfo -z`
if [ "$RV" -eq 1 ]
then
	exit 0
fi

# Else in all other cases and values of RV the deinstall should be stopped
# to ensure that the customer does not end up with an unusable system.

exit 1
