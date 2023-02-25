#!/bin/bsh
# @(#)04        1.3  src/packages/devices/mca/deff/rte/root/devices.mca.deff.rte.unconfig.sh, pkgdevcommo, pkg411, 9436D411a 9/7/94 09:14:05
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
#旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
#�                                                                          �
#� LOGIC                                                                    �
#�                                                                          �
#�       Read Environment variable "INU_INSTSCRIPT"                         �
#�       IF "INU_INSTSCRIPT" = reject or deinstal                           �
#�               remove all CuAt attributes for this device                 �
#�                                                                          �
#읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸

if [ "$INU_INSTSCRIPT" = "reject" -o "$INU_INSTSCRIPT" = "deinstal" ]
then
	#旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
	#� search for CuDv devices with unique_type = adapter/mca/mpaa       �
	#� issue an odmdelete command on all entries for those CuDv devices  �
	#읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
	NAME=`odmget -q "PdDvLn=adapter/mca/mpaa" CuDv | fgrep "name" | \
	    awk -F"\"" '{ print $2}'`

	for DEVICE in $NAME
	do
	    odmdelete -q name=$DEVICE -o CuAt >/dev/null 2>&1
	    odmdelete -q name=$DEVICE -o CuDv >/dev/null 2>&1
	done
fi
exit 0
