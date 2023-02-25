#!/bin/bsh
# src/nfs/usr/sbin/yp/ypxfr_2perday.sh, cmdnfs, nfs411, GOLD410 2/3/94 15:29:16
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
#    (#)ypxfr_2perday.sh	1.1 88/03/07 4.0NFSSRC SMI
#
# ypxfr_2perday.sh - Do twice-daily yp map check/updates
#

PATH=/bin:/usr/bin:/usr/sbin:/usr/lib/netsvc/yp:$PATH
export PATH

# set -xv
ypxfr hosts.byname
ypxfr hosts.byaddr
ypxfr ethers.byaddr
ypxfr ethers.byname
ypxfr netgroup
ypxfr netgroup.byuser
ypxfr netgroup.byhost
ypxfr mail.aliases 
