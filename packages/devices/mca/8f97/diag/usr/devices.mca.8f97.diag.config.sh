#!/bin/ksh
# @(#)01 1.1  src/packages/devices/mca/8f97/diag/usr/devices.mca.8f97.diag.config.sh, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:29:44
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    devices.mca.8f97.diag.config			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    This is called after an installation has been done	#
#		    and we have the oportunity to do some configuration	#
#		    We add the diagnostic ODM stanzas if we have to.	#
#		    Note that they cannot be deleted!			#
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

OLDODMDIR=$ODMDIR
export ODMDIR=/usr/lib/objrepos

odmget -q "msgkey = 'USM'" DSMOptions | grep "SSAaids.cat" 1>/dev/null 2>&1
if [[ $? -ne 0 ]]
then
    /bin/odmadd /usr/lpp/devices.mca.8f97/SSAaids.add 1>/dev/null 2>/dev/null
fi

export ODMDIR=$OLDODMDIR

exit 0
