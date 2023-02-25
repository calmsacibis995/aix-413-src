# @(#)67	1.13  src/packages/bos/net/nis/client/bos.net.nis.client.il, pkgnis, pkg41J, 9520A_all 5/17/95 12:03:10
#
# COMPONENT_NAME: (pkgnis)
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
# LPP_NAME: bos.net.nis.client
#
F 0 0 555	/usr/bin/chkey 
F 0 0 555	/usr/bin/domainname 
F 0 0 555	/usr/bin/keylogin 
F 0 0 555	/usr/bin/ypcat 
F 0 0 555	/usr/bin/ypmatch 
F 0 7 4555	/usr/bin/yppasswd 
F 0 0 555	/usr/bin/ypwhich 
D 0 0 755	/usr/etc 
D 0 0 755	/usr/etc/yp 
D 0 0 755	/usr/lib/netsvc/yp 
F 0 0 555	/usr/lib/netsvc/yp/ypbind 
S 0 0 777	/usr/etc/ypbind /usr/lib/netsvc/yp/ypbind 
FT 0 0 4555	/usr/sbin/keyenvoy 
ST 0 0 777	/usr/etc/keyenvoy /usr/sbin/keyenvoy 
F 0 0 555	/usr/sbin/keyserv 
S 0 0 777	/usr/etc/keyserv /usr/sbin/keyserv 
F 0 0 555	/usr/sbin/ypset 
S 0 0 777	/usr/etc/yp/ypset /usr/sbin/ypset 
d 0 0 755	/var/yp 
F 0 0 555       /usr/lib/security/NIS 
V 0 0 555	/usr/sbin/chypdom 
S 0 0 777	/usr/etc/yp/chypdom /usr/sbin/chypdom 
V 0 0 555	/usr/sbin/mkclient 
S 0 0 777	/usr/etc/yp/mkclient /usr/sbin/mkclient 
V 0 0 555	/usr/sbin/mkkeyserv 
S 0 0 777	/usr/etc/yp/mkkeyserv /usr/sbin/mkkeyserv 
F 0 0 555	/usr/sbin/rmkeyserv 
S 0 0 777	/usr/etc/yp/rmkeyserv /usr/sbin/rmkeyserv 
V 0 0 555       /usr/sbin/rmyp 
S 0 0 777	/usr/etc/yp/rmyp /usr/sbin/rmyp 
