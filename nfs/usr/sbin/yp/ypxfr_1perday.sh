#!/bin/bsh
# src/nfs/usr/sbin/yp/ypxfr_1perday.sh, cmdnfs, nfs411, GOLD410 2/3/94 15:22:58
# 
# COMPONENT_NAME: (CMDNFS) Network File System Commands
# 
# FUNCTIONS: 
#
# ORIGINS: 24 
#
# Copyright (c) 1988 Sun Microsystems, Inc.
#
#
#    (#)ypxfr_1perday.sh	1.1 88/03/07 4.0NFSSRC SMI
#
# ypxfr_1perday.sh - Do daily yp map check/updates
#

PATH=/bin:/usr/bin:/usr/sbin:/usr/lib/netsvc/yp:$PATH
export PATH

# set -xv
ypxfr group.byname
ypxfr group.bygid 
ypxfr protocols.byname
ypxfr protocols.bynumber
ypxfr networks.byname
ypxfr networks.byaddr
ypxfr services.byname
ypxfr ypservers
