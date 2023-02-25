#!/bin/bsh
# @(#)05        1.3  src/packages/devices/mca/deff/sdlc/root/devices.mca.deff.sdlc.unconfig.sh, pkgdevcommo, pkg411, 9436D411a 9/7/94 09:14:08
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
#��������������������������������������������������������������������������Ŀ
#�                                                                          �
#� LOGIC                                                                    �
#�                                                                          �
#�       Read Environment variable "INU_INSTSCRIPT"                         �
#�       IF "INU_INSTSCRIPT" = reject or deinstal                           �
#�               remove all CuAt attributes for this device                 �
#�                                                                          �
#����������������������������������������������������������������������������

if [ "$INU_INSTSCRIPT" = "reject" -o "$INU_INSTSCRIPT" = "deinstal" ]
then
	#�������������������������������������������������������������������Ŀ
	#� search for CuDv devices with unique_type = driver/mpaa/mpa        �
	#� issue an odmdelete command on all entries for those CuDv devices  �
	#���������������������������������������������������������������������
	NAME=`odmget -q "PdDvLn=driver/mpaa/mpa" CuDv | fgrep "name" | \
	    awk -F"\"" '{ print $2}'`

	for DEVICE in $NAME
	do
	    odmdelete -q name=$DEVICE -o CuAt >/dev/null 2>&1
	    odmdelete -q name=$DEVICE -o CuDv >/dev/null 2>&1
	done
fi
exit 0
