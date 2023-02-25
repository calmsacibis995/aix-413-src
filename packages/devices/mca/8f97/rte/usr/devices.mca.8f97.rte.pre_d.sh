#!/bin/ksh
# @(#)11 1.1  src/packages/devices/mca/8f97/rte/usr/devices.mca.8f97.rte.pre_d.sh, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:30:57
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    devices.mca.8f97.rte.pre_d.sh			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    This is called before a deinstall. We return 1 if	#
#		    the deinstall cannot be allowed to proceed.		#
#									#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when combined	#
#			 with the aggregate modules for	this project)	#
#									#
#   (C)	Copyright International	Business Machines Corp.	1995		#
#   All	rights reserved.						#
#									#
#########################################################################

uniquetypes="adapter/mca/ssa"
for type in $uniquetypes
do
    rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
    if [ -n "$rc" ]
    then
	exit 1
    fi
done

exit 0
