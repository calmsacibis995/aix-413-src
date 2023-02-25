#!/bin/bsh
# src/nfs/usr/sbin/yp/ypxfr_1perhour.sh, cmdnfs, nfs411, GOLD410 2/3/94 15:28:02
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
#     (#)ypxfr_1perhour.sh	1.1 88/03/07 4.0NFSSRC SMI
#
# ypxfr_1perhour.sh - Do hourly yp map check/updates
#

PATH=/bin:/usr/bin:/usr/sbin:/usr/lib/netsvc/yp:$PATH
export PATH

# set -xv
ypxfr passwd.byname
ypxfr passwd.byuid 
