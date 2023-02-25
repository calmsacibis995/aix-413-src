#!/bin/bsh
#  @(#)51        1.5  src/packages/bos/net/nfs/client/root/bos.net.nfs.client.config.sh, pkgnfs, pkg411, GOLD410 4/22/94 13:15:50
#
# COMPONENT_NAME: pkgnfs
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
#
# NAME:  root/bos.net.nfs.client.config
#
# FUNCTION: script called from instal to do special nfs configuration 
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   
/bin/mkssys -s biod -G nfs -p /usr/sbin/biod \
	-u 0 -i /dev/null -o /dev/console -e /dev/console \
	-a 6 -R -d -Q -S -n 30 -f 31 -E 20 -w 20

/bin/mkssys -s nfsd -G nfs -p /usr/sbin/nfsd \
	-u 0 -i /dev/null -o /dev/console -e /dev/console \
	-a 8 -R -d -Q -S -n 30 -f 31 -E 20 -w 20

/bin/mkssys -s rpc.statd -G nfs -p /usr/sbin/rpc.statd \
	-u 0 -t statd -i /dev/null -o /dev/console -e /dev/console \
	-R -d -Q -S -n 30 -f 31 -E 20 -w 20

/bin/mkssys -s rpc.lockd -G nfs -p /usr/sbin/rpc.lockd \
	-u 0 -t lockd -i /dev/null -o /dev/console -e /dev/console \
	-R -d -Q -S -n 30 -f 31 -E 20 -w 20

/bin/mkssys -s rpc.mountd -G nfs -p /usr/sbin/rpc.mountd -t mountd \
	-u 0 -i /dev/null -o /dev/console -e /dev/console \
	-R -d -Q -S -n 30 -f 31 -E 20 -w 20

/usr/sbin/mknfs -I >/dev/null 2>&1  

exit 0
