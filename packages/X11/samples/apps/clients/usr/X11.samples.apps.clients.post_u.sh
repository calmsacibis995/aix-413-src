#!/bin/ksh
#  @(#)50        1.1  src/packages/X11/samples/apps/clients/usr/X11.samples.apps.clients.post_u.sh, pkgx, pkg41B, 412_41B_sync 12/21/94 11:19:14
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.apps.clients (usr) post update configuration
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
#
#  Need to remove the /usr/lpp/X11/lib/X11/app-defaults/XClock entry
#  from the X11.apps.samples.clients.unpost_i script.
#

cd /usr/lpp/X11.samples/deinstl
cp X11.samples.apps.clients.unpost_i /tmp/X11.samples.apps.clients.unpost_i
sed -e "/XClock/d" /tmp/X11.samples.apps.clients.unpost_i > X11.samples.apps.clients.unpost_i
rm /tmp/X11.samples.apps.clients.unpost_i

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
