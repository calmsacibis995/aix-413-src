#!/usr/bin/ksh
# @(#)49        1.2  src/packages/bos/rte/install/usr/bos.rte.install.unpost_u.sh, pkgbossub, pkg41J, 9525D_all 6/20/95 11:36:46
#
# COMPONENT_NAME: pkgbossub
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME:	 bos.rte.install.unpost_u
#                                                                    
# FUNCTION: script called from reject to undo actions taken in the post_u script
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  

#
# Defect : 179940
# Reason : Creation of link to the DCE-Client bundle if it is not already created
#

if [ -f /usr/sys/inst.data/sys_bundles/DCE-Client.bnd ]
then
	rm /usr/sys/inst.data/sys_bundles/DCE-Client.bnd

	if [ $? -ne 0 ]
	then
		exit 1
	fi
fi
exit 0
