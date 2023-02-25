#!/usr/bin/ksh
# @(#)48        1.2  src/packages/bos/rte/install/usr/bos.rte.install.post_u.sh, pkgbossub, pkg41J, 9525D_all 6/20/95 11:36:43
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
# NAME:	 bos.rte.install.post_u
#                                                                    
# FUNCTION: script called from update to make changes to the updated files
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  

#
# Defect : 179940
# Reason : Creation of link to the DCE-Client bundle if it is not already created
#

if [ ! -f /usr/sys/inst.data/sys_bundles/DCE-Client.bnd ]
then
	ln -s /usr/sys/inst.data/sys_bundles/DCE-Client.def  /usr/sys/inst.data/sys_bundles/DCE-Client.bnd

	if [ $? -ne 0 ]
	then
		exit 1
	fi
fi
exit 0
