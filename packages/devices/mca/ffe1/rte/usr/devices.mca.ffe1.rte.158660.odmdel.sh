#!/bin/bsh
# @(#)88        1.1  src/packages/devices/mca/ffe1/rte/usr/devices.mca.ffe1.rte.158660.odmdel.sh, pkgdevtty, pkg41J, 9520A_all 4/27/95 16:17:37
#
# COMPONENT_NAME: pkgdevtty
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME:  
#
# FUNCTION: script called from install to remove tty attributes that
#           have moved to the parent adapter. 158660 
#
# RETURN VALUE DESCRIPTION: None
#                            
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'onstr' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'offstr' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'maxcps' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'maxchar' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'bufsize' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'forcedcd' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'fastcook' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'altpin' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'edelay' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = '2200flow' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = '2200print' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'fastbaud' " >/dev/null

