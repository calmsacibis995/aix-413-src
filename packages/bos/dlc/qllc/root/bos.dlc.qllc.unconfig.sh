#!/bin/bsh
# static char sccsid[] = "@(#)97  1.3  src/packages/bos/dlc/qllc/root/bos.dlc.qllc.unconfig.sh, pkgdlc, pkg411, GOLD410 4/18/94 17:59:47";
#
#   COMPONENT_NAME: PKGDLC
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   NAME: bos.dlc.qllc.unconfig.sh
#
#   RETURN VALUE DESCRIPTION:
#         always returns 0
#

############################################################
#  unconfigure X.25 QLLC DLC                               #
#  Note - the pre_d script keeps a de-install from         #
#         removing a defined DLC (ie., the operator must   #
#         remove the DLC before de-installing it.  Install #
#         failures, on the other hand, that reach config,  #
#         are handled here, and the DLC is removed.        #
#                                                          #
#         Assumes INUCLIENTS will be non-zero length if    #
#         installing on diskless client.                   #
############################################################

if [ "$INUCLIENTS" = "" ]
then
#	remove the dlc
	rmdev -l dlcqllc -d > /dev/null 2>/dev/null
fi

exit 0
