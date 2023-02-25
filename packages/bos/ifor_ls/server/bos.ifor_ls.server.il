# @(#)02	1.6  src/packages/bos/ifor_ls/server/bos.ifor_ls.server.il, pkgnetls, pkg41J, 9520A_all 5/8/95 08:59:53
#
#   COMPONENT_NAME: pkgnetls
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LPP_NAME: bos.ifor_ls.server
#
D 2 2 755       /usr/lib/netls/include ifor_ls.server
D 2 2 755       /usr/lib/netls/ark/include ifor_ls.server
F 2 2 444       /usr/lib/netls/ark/include/log.h ifor_ls.server
S 2 2 777       /usr/lib/netls/include/log.h /usr/lib/netls/ark/include/log.h ifor_ls.server
F 2 2 500       /usr/lib/netls/conf/netls_config ifor_ls.server
F 2 2 500       /usr/lib/netls/conf/auto_config ifor_ls.server
F 2 2 500	/usr/lib/netls/ark/bin/srv_start ifor_ls.server
S 2 2 777	/usr/lib/netls/bin/srv_start /usr/lib/netls/ark/bin/srv_start ifor_ls.server
F 2 2 500	/usr/lib/netls/ark/bin/srv_stop ifor_ls.server
S 2 2 777	/usr/lib/netls/bin/srv_stop /usr/lib/netls/ark/bin/srv_stop ifor_ls.server
F 2 2 500	/usr/lib/netls/ark/bin/netlsd ifor_ls.server
F 2 2 444	/usr/lib/netls/ark/cat1/netlsd.1 ifor_ls.server
S 2 2 777	/usr/lib/netls/cat1/netlsd.1 /usr/lib/netls/ark/cat1/netlsd.1 ifor_ls.server
f 0 0 644       /etc/rc.netls ifor_ls.server
S 2 2 777	/usr/lib/netls/conf/rc.netls /etc/rc.netls ifor_ls.server
S 2 2 777	/usr/lib/netls/conf/rc.ncs /etc/rc.ncs ifor_ls.server

D 2 2 755       /usr/lib/netls/adk/bin ifor_ls.server
F 2 2 500	/usr/lib/netls/adk/bin/ls_dpass ifor_ls.server
S 2 2 777	/usr/lib/netls/bin/ls_dpass /usr/lib/netls/adk/bin/ls_dpass ifor_ls.server
F 2 2 500	/usr/lib/netls/adk/bin/_ls_dpass ifor_ls.server
S 2 2 777	/usr/lib/netls/bin/_ls_dpass /usr/lib/netls/adk/bin/_ls_dpass ifor_ls.server
D 2 2 755       /usr/lib/netls/adk/bin/help ifor_ls.server
F 2 2 400       /usr/lib/netls/adk/bin/help/lspass.hlp ifor_ls.server
S 2 2 777       /usr/lib/netls/bin/help/lspass.hlp /usr/lib/netls/adk/bin/help/lspass.hlp ifor_ls.server

D 2 2 755       /usr/lib/netls/adk/X11 ifor_ls.server
D 2 2 755       /usr/lib/netls/adk/X11/app-defaults ifor_ls.server
F 2 2 700       /usr/lib/netls/adk/X11/app-defaults/XLs_DPass ifor_ls.server
S 2 2 777       /usr/lpp/X11/lib/X11/app-defaults/XLs_DPass /usr/lib/netls/adk/X11/app-defaults/XLs_DPass ifor_ls.server

D 2 2 755       /usr/lib/netls/adk/cat1 ifor_ls.server
F 2 2 444	/usr/lib/netls/adk/cat1/ls_dpass.1.z ifor_ls.server
S 2 2 777	/usr/lib/netls/cat1/ls_dpass.1.z /usr/lib/netls/adk/cat1/ls_dpass.1.z ifor_ls.server
