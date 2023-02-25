#!/bin/bsh
# static char sccsid[] = "@(#)99  1.2  src/packages/bos/dlc/sdlc/root/bos.dlc.sdlc.config.sh, pkgdlc, pkg411, GOLD410 4/18/94 14:35:27";
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
#   NAME: bos.dlc.sdlc.config.sh
#
#   RETURN VALUE DESCRIPTION:
#         return mkdev return code
#
#
############################################################
#  configure SDLC DLC                                      #
############################################################

# assumes INUCLIENTS will be non-zero length if installing on diskless client.

rc=0

if [ "$INUCLIENTS" = "" ]
then
#	make the dlc available
	if test -r  /dev/dlcsdlc
	then
		mkdev -l dlcsdlc

	else
		mkdev -c dlc -s dlc -t sdlc

	fi
	rc=$?

else
#       indicate that the diskless clients will have to be rebooted
        inuumsg 108
fi

# pass the mkdev return code to the caller
exit $rc
