#  @(#)17        1.2  src/packages/devices/mca/deff/sdlc/usr/devices.mca.deff.sdlc.insize.S, pkgdevcommo, pkg411, GOLD410 1/26/94 08:34:08
#
#   COMPONENT_NAME: pkgdevcommo
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: mpa.sdlc.obj.insize.S
#                                                                    
# FUNCTION: provides size for filesystem increases needed at install time
#
#   /usr	- the mpa.sdlc.obj.undo.err, mpa.sdlc.obj.undo.trc and 
#		  mpa.sdlc.obj.undo.codepoint files are created dynamically
#		  in the /usr/lpp/mpa directory. 
# 
# directory sign perm_size temp_size 
/usr/lpp/mpa  +128    
