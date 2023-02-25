#!/bin/ksh
#  @(#)56        1.1  src/packages/devices/serial/gio/rte/root/devices.serial.gio.rte.post_i.sh, pkgdevgraphics, pkg41J, 9518A_all 5/1/95 13:15:26
#
#   COMPONENT_NAME: pkgdevgraphics
#
#   FUNCTIONS: devices.serial.gio.rte (root) post installation configuration
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#    Convert native serial Dial/LPFKeys devices, if possible.
#    Return code is intentionally ignored because customer may
#    have serial devices opened (e.g. X-server running) when this
#    install script is called. Conversion script may be called
#    at a later time when all accessing programs have been closed.
#----------------------------------------------------------------
/usr/lib/methods/convert_sgio

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
