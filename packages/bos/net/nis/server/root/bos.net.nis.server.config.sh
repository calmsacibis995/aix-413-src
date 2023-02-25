#!/bin/bsh
#  @(#)75        1.4  src/packages/bos/net/nis/server/root/bos.net.nis.server.config.sh, pkgnis, pkg411, GOLD410 2/15/94 14:21:51
#
# COMPONENT_NAME: pkgnis
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993.
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
#
# NAME:  root/bos.net.nis.server.config
#
# FUNCTION: script called from instal to do special nfs configuration 
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   

/bin/mkssys -s ypserv -G yp -p /usr/lib/netsvc/yp/ypserv \
	-u 0 -i /dev/null -o /dev/console -e /dev/console \
	-R -d -Q -S -n 30 -f 31 -E 20 -w 20
/bin/mkssys -s ypupdated -G yp -p /usr/lib/netsvc/yp/rpc.ypupdated \
	-u 0 -t rpc.ypupdated -i /dev/null -o /dev/console -e /dev/console \
	-R -d -Q -S -n 30 -f 31 -E 20 -w 20
/bin/mkssys -s yppasswdd -G yp -p /usr/lib/netsvc/yp/rpc.yppasswdd \
	-u 0 -t rpc.yppasswdd -i /dev/null -o /dev/console -e /dev/console \
	-a '/etc/passwd -m' -R -d -Q -S -n 30 -f 31 -E 20 -w 20

exit 0
