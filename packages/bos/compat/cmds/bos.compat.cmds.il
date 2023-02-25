# @(#)70	1.11  src/packages/bos/compat/cmds/bos.compat.cmds.il, pkgcompat, pkg411, GOLD410 5/23/94 11:13:22
#
# COMPONENT_NAME: pkgcompat
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
# LPP_NAME: bos.compat.cmds
#
F 2 2 555	/usr/bin/confer
H 2 2 777	/usr/bin/joinconf /usr/bin/confer
S 2 2 777	/usr/bin/copy /usr/bin/cp
V 2 2 555	/usr/bin/del
F 2 2 555	/usr/bin/li
H 2 2 777	/usr/bin/di /usr/bin/li
F 2 2 555	/usr/bin/msgs
S 2 2 777	/usr/ucb/more /usr/bin/more
S 2 2 777	/usr/bin/page /usr/bin/more
S 2 2 777	/usr/bin/move /usr/bin/mv
S 2 2 777	/usr/bin/delete /usr/bin/rm
S 2 2 777       /usr/ucb/sccs /usr/bin/sccs
F 0 7 500	/usr/bin/virscan
D 0 7 750	/usr/lib/security/scan
F 0 7 640	/usr/lib/security/scan/648clean.lst
F 0 7 640	/usr/lib/security/scan/darclean.lst
F 0 7 640	/usr/lib/security/scan/ghoclean.lst
F 0 7 640	/usr/lib/security/scan/vacclean.lst
F 0 7 640	/usr/lib/security/scan/virsig.lst
F 0 7 640	/usr/lib/security/scan/xa1clean.lst
s 0 0 777	/etc/updatep /usr/sbin/installp
s 0 0 777	/etc/instupdt /usr/sbin/installp
