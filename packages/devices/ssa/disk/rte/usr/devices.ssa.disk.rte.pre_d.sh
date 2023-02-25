#!/bin/ksh
# @(#)29 1.1  src/packages/devices/ssa/disk/rte/usr/devices.ssa.disk.rte.pre_d.sh, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:33:03
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    devices.ssa.disk.rte.pre_d.sh			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    This is called before a deinstall. We return 1 if	#
#		    the deinstall cannot be allowed to proceed.		#
#									#
#   Licensed Materials - Property of IBM				#
#									#
#   (C) Copyright International Business Machines Corp. 1995.		#
#   All rights reserved.						#
#									#
#   US Government Users Restricted Rights - Use, duplication or		#
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.	#
#									#
#########################################################################

uniquetypes="adapter/mca/ssa driver/node/ssar disk/ssar/hdisk pdisk/ssar/1000mbC pdisk/ssar/2000mbC pdisk/ssar/4000mbC pdisk/ssar/1000mbF pdisk/ssar/2000mbF pdisk/ssar/4000mbF pdisk/ssar/ossadisk pdisk/ssar/2000mbA"

for type in $uniquetypes
do
    rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
    if [ -n "$rc" ]
    then
	exit 1
    fi
done

exit 0
