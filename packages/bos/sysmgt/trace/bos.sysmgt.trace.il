# @(#)86        1.13  src/packages/bos/sysmgt/trace/bos.sysmgt.trace.il, pkgtrc, pkg411, 9440E411c 10/11/94 15:01:01
#
# COMPONENT_NAME: pkgtrc
#
# FUNCTIONS:  inslist
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# -------------------------------------------------------------------------
#
# LPP_NAME: bos.sysmgt.trace
#
F 0 2 555	/usr/bin/trcnm
F 2 2 555	/usr/bin/trcrpt
F 0 0 550	/usr/bin/tracesoff
F 0 0 550	/usr/bin/traceson
FT 0 3 4555	/usr/lib/trcload
F 2 2 555	/usr/lib/drivers/trcdd
D 0 3 755	/usr/lib/ras
F 2 2 444	/usr/lib/ras/trclist
F 2 2 555	/usr/lib/ras/trace_smutil
F 2 2 555	/usr/sbin/trace
S 2 2 777	/usr/bin/trace /usr/sbin/trace
S 2 2 777	/usr/bin/trcoff /usr/sbin/trace
S 2 2 777	/usr/bin/trcon /usr/sbin/trace
S 2 2 777	/usr/bin/trcstop /usr/sbin/trace
S 2 2 777	/usr/bin/trcdead /usr/sbin/trace
D 2 2 755	/usr/lpp/bos.sysmgt
F 2 2 555	/usr/lpp/bos.sysmgt/handfmt
