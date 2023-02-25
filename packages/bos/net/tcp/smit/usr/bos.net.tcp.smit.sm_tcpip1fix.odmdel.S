# @(#)77        1.1  src/packages/bos/net/tcp/smit/usr/bos.net.tcp.smit.sm_tcpip1fix.odmdel.S, pkgtcpip, pkg411, 9440G411b 10/13/94 18:58:13
#
# COMPONENT_NAME: pkgtcpip
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
ODMDIR=/usr/lib/objrepos odmdelete -o sm_menu_opt -q "id=configtcp and id_seq_num=050 and next_id=servernet" > /dev/null 2>&1
ODMDIR=/usr/lib/objrepos odmdelete -o sm_menu_opt -q "id=tcpipen and id_seq_num=050 and next_id=servernet" > /dev/null 2>&1
ODMDIR=/usr/lib/objrepos odmdelete -o sm_menu_opt -q "id=tcpiptr and id_seq_num=050 and next_id=servernet" > /dev/null 2>&1
ODMDIR=/usr/lib/objrepos odmdelete -o sm_menu_opt -q "id=tcpipso and id_seq_num=050 and next_id=servernet" > /dev/null 2>&1
ODMDIR=/usr/lib/objrepos odmdelete -o sm_menu_opt -q "id=tcpipfi and id_seq_num=050 and next_id=servernet" > /dev/null 2>&1
