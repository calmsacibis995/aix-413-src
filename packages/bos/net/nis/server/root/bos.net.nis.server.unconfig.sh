# @(#)20        1.1  src/packages/bos/net/nis/server/root/bos.net.nis.server.unconfig.sh, pkgnis, pkg411, GOLD410 4/18/94 10:04:39
#
# COMPONENT_NAME: pkgnis
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
# NAME:	 bos.net.nis.server.unconfig.sh
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
/bin/rmssys -s ypserv
/bin/rmssys -s ypupdated
/bin/rmssys -s yppasswdd 

exit 0
