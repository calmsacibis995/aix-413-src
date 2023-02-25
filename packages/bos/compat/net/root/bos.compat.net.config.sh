#!/bin/ksh
# @(#)40	1.1  src/packages/bos/compat/net/root/bos.compat.net.config.sh, pkgcompat, pkg411, GOLD410 3/30/94 18:48:07
#
#   COMPONENT_NAME: pkgcompat
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
# NAME:	 bos.compat.net.config.sh
#                                                                    
# FUNCTION: script called from instal to do special compat
# 	    tcpip configuration
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  
# ============================================================#
#
# This Creates the InetServ ODM object database
#
# ============================================================#

/usr/sbin/inetimp

exit 0
