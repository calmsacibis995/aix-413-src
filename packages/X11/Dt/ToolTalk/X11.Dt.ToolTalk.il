# @(#)71        1.11  src/packages/X11/Dt/ToolTalk/X11.Dt.ToolTalk.il, pkgxcde, pkg41J, 9514A_all 4/4/95 18:45:24
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# X11.Dt.ToolTalk
#
#
# directories
#
D 2 2 755	/usr/dt
D 2 2 755	/usr/dt/bin
D 2 2 755	/usr/dt/lib
D 2 2 755	/usr/dt/install
D 2 2 755	/usr/lib
D 2 2 755	/usr/dt/appconfig
D 2 2 755	/usr/dt/appconfig/tttypes
#
# files for configuring tooltalk daemons
#
F 2 2 555	/usr/dt/install/configTT
#
# files from CDE-TT.udb
#
#---
F 2 2 555 	 /usr/dt/bin/rpc.ttdbserver
F 2 2 555 	 /usr/dt/bin/tt_type_comp 
F 2 2 555 	 /usr/dt/bin/ttcp 
F 2 2 555 	 /usr/dt/bin/ttdbck 
F 2 2 555 	 /usr/dt/bin/ttmv 
F 2 2 555 	 /usr/dt/bin/ttrm 
S 2 2 777	/usr/dt/bin/ttrmdir  /usr/dt/bin/ttrm
F 2 2 555 	 /usr/dt/bin/ttsession 
F 2 2 555 	 /usr/dt/bin/tttrace 
F 2 2 555 	 /usr/dt/bin/tttar 
F 2 2 555 	 /usr/dt/bin/ttsnoop 
V 2 2 555 	 /usr/dt/lib/libtt.a 
S 2 2 777	 /usr/lib/libtt.a /usr/dt/lib/libtt.a
F 2 2 444	/usr/dt/appconfig/tttypes/types.xdr
