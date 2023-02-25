#  @(#)69        1.1  src/packages/bos/net/nis/client/root/bos.net.nis.client.pre_i.sh, pkgnis, pkg411, GOLD410 1/12/93 16:16:08
#
# COMPONENT_NAME: pkgnis
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993.
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
#
# NAME:  root/bos.net.nis.client.pre_i
#
# FUNCTION: script called from instal
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   

#
# This deletes the ODM stuff for SRC that we added
# with our config script ( or a previous install)

#============================================================#
#
# Delete the src stuff
#
    rmssys -s keyserv >/dev/null 2>&1
    rmssys -s ypbind >/dev/null 2>&1

    exit 0
