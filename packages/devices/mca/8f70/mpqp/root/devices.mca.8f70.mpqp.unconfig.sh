#!/bin/bsh
# @(#)94        1.3  src/packages/devices/mca/8f70/mpqp/root/devices.mca.8f70.mpqp.unconfig.sh, pkgdevcommo, pkg411, 9436D411a 9/7/94 09:14:03
#
#   COMPONENT_NAME: PKGDEVCOMMO
#
#   FUNCTIONS: unconfig.sh
#               Script called from reject or cleanup.
#               If script called by "reject" or "deinstall"
#               then remove any customized device information
#               for this device.
#               If script called by cleanup, do nothing.
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
#³                                                                          ³
#³ LOGIC                                                                    ³
#³                                                                          ³
#³       Read Environment variable "INU_INSTSCRIPT"                         ³
#³       IF "INU_INSTSCRIPT" = reject or deinstal                           ³
#³               remove all CuAt attributes for this device                 ³
#³                                                                          ³
#ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

if [ "$INU_INSTSCRIPT" = "reject" -o "$INU_INSTSCRIPT" = "deinstal" ]
then
	#ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	#³ search for all CuDv devices with unique_type = port/mpqp/mpqport  ³
	#³ issue an odmdelete command on all entries for those CuDv devices  ³
	#ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
	NAME=`odmget -q "PdDvLn=port/mpqp/mpqport" CuDv | fgrep "name" | \
	    awk -F"\"" '{ print $2}'`

	for DEVICE in $NAME
	do
	    odmdelete -q name=$DEVICE -o CuAt >/dev/null 2>&1
	    odmdelete -q name=$DEVICE -o CuDv >/dev/null 2>&1
	done
	#ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	#³ search for CuDv devices with unique_type = driver/portmaster/mpqp ³
	#³ issue an odmdelete command on all entries for those CuDv devices  ³
	#ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
	NAME=`odmget -q "PdDvLn=driver/portmaster/mpqp" CuDv | fgrep "name" | \
	    awk -F"\"" '{ print $2}'`

	for DEVICE in $NAME
	do
	    odmdelete -q name=$DEVICE -o CuAt >/dev/null 2>&1
	    odmdelete -q name=$DEVICE -o CuDv >/dev/null 2>&1
	done
fi
exit 0
