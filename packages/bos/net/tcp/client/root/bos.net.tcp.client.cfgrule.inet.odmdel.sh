# @(#)36	1.3  src/packages/bos/net/tcp/client/root/bos.net.tcp.client.cfgrule.inet.odmdel.sh, pkgtcpip, pkg411, GOLD410 6/20/94 15:01:50
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
if [ "$INUSAVE" = "1" ] ; then
odmget  -q " phase = '2' AND seq = '30' AND rule LIKE '/usr/lib/methods/definet*' " Config_Rules >> $SAVEDIR/bos.net.tcp.client.cfgrule.inet.rodmadd
chmod a+x $SAVEDIR/bos.net.tcp.client.cfgrule.inet.rodmadd 
fi
echo "Updating ODM data with bos.net.tcp.client.cfgrule.inet.odmdel ... "
odmdelete -o Config_Rules -q " phase = '2' AND seq = '30' AND rule LIKE '/usr/lib/methods/definet*' " >/dev/null
