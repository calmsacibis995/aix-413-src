#!/bin/bsh
#  @(#)68        1.2  src/packages/bos/net/nis/client/root/bos.net.nis.client.config.sh, pkgnis, pkg411, GOLD410 2/4/94 15:06:22
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
# NAME:  root/bos.net.nis.client.config
#
# FUNCTION: script called from instal to do special nfs configuration 
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   
/bin/mkssys -s keyserv -G keyserv -p /usr/sbin/keyserv \
	-u 0 -i /dev/null -o /dev/console -e /dev/console \
	-O -d -Q -S -n 30 -f 31 -E 20 -w 20
/bin/mkssys -s ypbind -G yp -p /usr/lib/netsvc/yp/ypbind \
	-u 0 -i /dev/null -o /dev/console -e /dev/console \
	-R -d -Q -S -n 30 -f 31 -E 20 -w 20

exit 0
