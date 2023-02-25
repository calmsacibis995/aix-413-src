#!/bin/bsh
# @(#)88        1.1  src/packages/bos/dlc/sdlc/usr/bos.dlc.sdlc.insize.S, pkgdlc, pkg41J, 9517B_all 4/21/95 13:27:39
#
#   COMPONENT_NAME: PKGDLC
#
#   FUNCTIONS: provides the size for filesystem expansion when the
#              error log undo file is created.
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   NAME: bos.dlc.sdlc.insize.S
#
#########################################################################
#
# directory sign(+/-) perm_size temp_size
/lpp/bos.dlc  +128
