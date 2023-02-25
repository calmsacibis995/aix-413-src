# @(#)18	1.2  src/packages/devices/mca/deff/sdlc/root/devices.mca.deff.sdlc.trc.S, pkgdevcommo, pkg411, GOLD410 1/26/94 08:29:39
#
#   COMPONENT_NAME: (MPAINST) MP/A SDLC DEVICE DRIVER INSTALLATION
#
#   FUNCTIONS: 
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
* /etc/trcfmt
+ 22c 1.0 "MPADD" \
   {{ $loc = X1 }} R1 $DD1 \
        $loc, \
        0x21 { "ID:" O1 A4.8     \
               "d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4 }, \
        0xC1 { "MPA RECV DATA" $DD2 \
		"byte1: " X4 "byte2: " X4 }, \
        0xC2 { "MPA XMIT DATA" $DD2 \
		"byte1: " X4 "byte2: " X4 } 
