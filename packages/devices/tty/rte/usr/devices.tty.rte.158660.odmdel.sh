#!/bin/bsh
# @(#)89        1.1  src/packages/devices/tty/rte/usr/devices.tty.rte.158660.odmdel.sh, pkgdevtty, pkg41J, 9520A_all 4/27/95 16:19:15
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
# NAME:  root/devices.mca.ffe1.rte.config
#
# FUNCTION: script called from install to remove tty attributes that
#           have moved to the parent adapter. 158660 
#
# RETURN VALUE DESCRIPTION: None
#                            
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'speed' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'rtrig' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'tbc16' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'tbc64' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'xprint_priority' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'xprint_on_str' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs232/tty' AND attribute = 'xprint_off_str' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs422/tty' AND attribute = 'speed' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs422/tty' AND attribute = 'rtrig' " >/dev/null
odmdelete -o PdAt -q " uniquetype = 'tty/rs422/tty' AND attribute = 'tbc16' " >/dev/null

