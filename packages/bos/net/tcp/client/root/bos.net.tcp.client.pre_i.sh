#  @(#)53        1.1  src/packages/bos/net/tcp/client/root/bos.net.tcp.client.pre_i.sh, pkgtcpip, pkg411, GOLD410 4/22/94 13:11:26
#
# COMPONENT_NAME: pkgtcpip
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
#
# NAME:  root/bos.net.tcp.client.pre_i
#
# FUNCTION: script called from instal
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   

#
# This deletes the SRC subsystems that were added
# with our config script ( or a previous install)

#============================================================#
#
# Delete the src stuff
#
    rmssys -s portmap >/dev/null 2>&1

    exit 0
