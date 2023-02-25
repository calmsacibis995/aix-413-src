# @(#)18        1.3  src/packages/bos/net/nfs/client/root/bos.net.nfs.client.unconfig.sh, pkgnfs, pkg411, GOLD410 6/30/94 22:29:16
#
# COMPONENT_NAME: pkgnfs
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994.
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME:	 root/bos.net.nfs.client.unconfig.sh
#                                                                    
# FUNCTION: script called from instal to do special tcpip configuration
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  
# ============================================================#
#
# This removes the ODM stuff that we need for SRC
#
# ============================================================#

/usr/sbin/rmnfs -B > /dev/null 2>&1

/bin/rmssys -s biod
/bin/rmssys -s nfsd
/bin/rmssys -s rpc.lockd 
/bin/rmssys -s rpc.mountd 
/bin/rmssys -s rpc.statd 

exit 0
