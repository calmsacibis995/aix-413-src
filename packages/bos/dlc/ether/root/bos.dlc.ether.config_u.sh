#!/bin/bsh
# static char sccsid[] = "@(#)85  1.2  src/packages/bos/dlc/ether/root/bos.dlc.ether.config_u.sh, pkgdlc, pkg411, GOLD410 4/18/94 14:31:44";
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
#   NAME: bos.dlc.ether.config_u.sh
#
#   RETURN VALUE DESCRIPTION:
#         return mkdev or rmdev return code
#

############################################################
#  configure Ethernet DLC                                  #
############################################################

# assumes INUCLIENTS will be non-zero length if installing on diskless client.

rc=0

if [ "$INUCLIENTS" = "" ]
then
	if test -r /dev/dlcether
	then
		rmdev -l dlcether > /dev/null 2>/dev/null

		if [ $? = 0 ]
		then
			mkdev -l dlcether
		fi
	rc=$?
	fi

else
#	indicate that the diskless clients will have to be rebooted
	inuumsg 108
fi
    
# pass the rmdev or mkdev return code to the caller
exit $rc
