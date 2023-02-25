#!/bin/sh
# @(#)74	1.6  src/tenplus/termcap/keymaps.sh, tenplus, tenplus411, GOLD410 3/23/93 12:14:44

# COMPONENT_NAME: (INED)  INed Editor
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

file=/usr/lib/INed/keys.map

readfile -g -0 -m Hkeys.map.cat 1 $file | \
	sed "s/^type = line, name = 'Name':$//" | \
	egrep -v ":$|NULL pointer" | sed "s/^'//" | sed "s/'$//" 

