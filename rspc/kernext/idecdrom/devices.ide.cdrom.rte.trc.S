# @(#)91        1.1  src/rspc/kernext/idecdrom/devices.ide.cdrom.rte.trc.S, idecd, rspc41J, 9510A 2/21/95 14:23:17
#
#   COMPONENT_NAME: IDECD
#
#   FUNCTIONS: none
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
* /etc/trcfmt
+2A2 1.0 "IDECDROMDD" \
	$HDU, \
	0x0F { strategy: bp=$D2 starttimer(0x2A2,$D2) }, \
	0x1E { `coalesce: (bp,ataidebuf)=($D2,$D3)` }, \
	0x1F { iodone: devtoname($D1) bp=$D2 buftofilename($D2) \
		endtimer(0x2A2,$D2) }, \
	0x17 { $D2, 0 { $SKIP } \
		bstart_entry: devtoname($D1) bp=$D2 buftofilename($D2) \
			pblock=$D4 bcount=$D5 bflags($D3) \
	}, \
	\*   { $DD1 }
