#!/bin/bsh
# 
# COMPONENT_NAME: (CMDNFS) Network File System Commands
# 
# FUNCTIONS: 
#
# ORIGINS: 24 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#       Copyright (c) 1987 by Sun Microsystems, Inc.
#
# IP to hex converter
# Handles all three classes of addresses (A, B & C)
#
# Method:
#	echo argument into stdin of awk
#	use awk to separate fields and provide input to dc
#	use dc to calculate the IP address value and print it out in hex
#	use awk to pad to 8 bytes, if necessary
#
HOME=/; export HOME
PATH=/bin:/usr/bin:/etc:/usr/sbin:/usr/lpp/nfs/sun_diskless:/usr/ucb

echo $1 | \
awk 'BEGIN {  FS="."; OFS=" "; print "16 o"; } ; { \
	print $1; \
	if (NF == 4) { print "256 *", $2, "+ 256 *", $3, "+ 256 *", $4; } \
	if (NF == 3) { print "256 *", "      256 *", $2, "+ 256 *", $3; } \
	if (NF == 2) { print "256 *", "      256 *", "      256 *", $2; } \
	print "+ p"; }' | \
dc | \
awk 'BEGIN { ORS=""; } ; { \
	for (i = length($1); i < 8; i++) { print "0"; }; print $1, "\n" }'
