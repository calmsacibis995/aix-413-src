# @(#)05	1.11  src/packages/X11/vsm/rte/usr/X11.vsm.rte.prereq.S, pkgxvsm, pkg411, 9435A411a 8/26/94 17:01:56
#
#   COMPONENT_NAME: libvsm
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

*coreq X11.vsm.helps 4.1.0.0
*coreq X11.vsm.icons 4.1.0.0

# Need the Motif library. This should prereq most of the other X stuff
*prereq X11.motif.lib 4.1.0.0

# Need Msmit for installation of additional devices.
*coreq X11.apps.msmit 4.1.0.0

# Need helpview
*coreq X11.Dt.helprun 4.1.0.0

# Prereq the necessary fonts. These will generally be on the
# system already, but we depend on them.

*coreq X11.fnt.defaultFonts 4.1.0.0
*coreq X11.fnt.iso1 4.1.0.0


# Need the printer commands
#*coreq printers.rte 4.1.0.0

# Need the errlog command
#*coreq bos.sysmgt.serv_aid 4.1.0.0

# Need the mknfsmnt command
#*coreq bos.net.nfs.client 4.1.0.0




