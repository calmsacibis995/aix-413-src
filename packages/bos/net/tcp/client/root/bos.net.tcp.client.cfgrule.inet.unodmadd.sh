# @(#)37	1.2  src/packages/bos/net/tcp/client/root/bos.net.tcp.client.cfgrule.inet.unodmadd.sh, pkgtcpip, pkg411, GOLD410 6/9/94 13:08:05
#
#   COMPONENT_NAME: pkgtcpip
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
echo "Deinstalling ODM data with bos.net.tcp.client.cfgrule.inet.unodmadd ... "
odmdelete -o Config_Rules -q " phase = '2' AND seq = '30' AND rule LIKE '/usr/lib/methods/definet*' " >/dev/null
