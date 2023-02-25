# @(#)64        1.2  src/packages/bos/sysmgt/smit/softlic/root/bos.sysmgt.smit.softlic.post_i.sh, pkgsoftlic, pkg411, 9439A411a 9/27/94 09:43:09
#
# COMPONENT_NAME: pkgsoftlic
#
# FUNCTIONS: post_i
#                                                                    
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
if [ ! -f "/var/adm/.license.sig" ]
then
	touch /var/adm/.license.sig
fi
exit 0
