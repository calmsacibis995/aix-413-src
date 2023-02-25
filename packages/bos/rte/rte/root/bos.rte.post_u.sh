#!/usr/bin/ksh
# @(#)49        1.2  src/packages/bos/rte/rte/root/bos.rte.post_u.sh, pkgbossub, pkg41J, 9524E_all 6/13/95 08:56:08
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
# NAME:	 bos.rte.post_u
#                                                                    
# FUNCTION: script called from update to make changes to the updated files
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  

#
# Defect : 180161
# Reason : Create the lost+found files for /home and /var
#

if [ ! -d /home/lost+found ]
then
	cd /home
	mklost+found > /dev/null
fi
if [ ! -d /var/lost+found ]
then
	cd /var
	mklost+found > /dev/null
fi
exit 0
