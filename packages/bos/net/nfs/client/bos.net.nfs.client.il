# @(#)54	1.17  src/packages/bos/net/nfs/client/bos.net.nfs.client.il, pkgnfs, pkg411, 9438B411a 9/21/94 14:57:18
#
# COMPONENT_NAME: (pkgnfs)
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
# LPP_NAME: bos.net.nfs.client
#
D 0 0 755	/usr/etc 
v 0 0 755	/etc/nfs.clean 
v 0 0 755	/etc/rc.nfs 
d 2 2 755       /sbin 
d 0 0 775       /sbin/helpers 
f 0 0 555       /sbin/helpers/nfsmnthelp 
F 0 0 555	/usr/bin/on 
F 0 0 555	/usr/bin/rup 
S 0 0 777	/usr/ucb/rup /usr/bin/rup 
F 0 0 555	/usr/bin/rusers 
S 0 0 777	/usr/ucb/rusers /usr/bin/rusers 
F 0 0 555	/usr/bin/showmount 
S 0 0 777	/usr/etc/showmount /usr/bin/showmount 
F 0 0 555       /usr/lib/drivers/nfs_kdes_null.ext 
F 0 0 555       /usr/lib/drivers/nfs_krpc.ext 
F 0 0 555       /usr/lib/drivers/nfs_clnt.ext 
F 0 0 555       /usr/lib/drivers/nfs_kacl.ext 
F 0 0 555       /usr/lib/drivers/nfs_serv.ext 
F 0 0 555       /usr/lib/drivers/nfs_clnt.load 
F 0 0 555       /usr/lib/drivers/nfs_serv.load 
F 0 0 755       /usr/lib/drivers/swapnfsdd 
F 0 0 500	/usr/lib/methods/cfgswpnfs 
D 0 0 755	/usr/lib/netsvc/rusers 
D 0 0 755	/usr/lib/netsvc/rwall 
D 0 0 755	/usr/lib/netsvc/spray 
F 0 0 555	/usr/lib/netsvc/rusers/rpc.rusersd 
S 0 0 777	/usr/etc/rpc.rusersd /usr/lib/netsvc/rusers/rpc.rusersd 
F 0 0 555	/usr/lib/netsvc/rwall/rpc.rwalld 
S 0 0 777	/usr/etc/rpc.rwalld /usr/lib/netsvc/rwall/rpc.rwalld 
F 0 0 555	/usr/lib/netsvc/spray/rpc.sprayd 
S 0 0 777	/usr/etc/rpc.sprayd /usr/lib/netsvc/spray/rpc.sprayd 
F 0 0 555	/usr/sbin/automount 
S 0 0 777	/usr/etc/automount /usr/sbin/automount 
F 0 0 555	/usr/sbin/biod 
S 0 0 777	/usr/etc/biod /usr/sbin/biod 
F 0 0 555	/usr/sbin/exportfs 
S 0 0 777	/usr/etc/exportfs /usr/sbin/exportfs 
F 0 0 555	/usr/sbin/gfsinstall 
s 0 0 777	/etc/gfsinstall /usr/sbin/gfsinstall 
F 0 0 555	/usr/sbin/nfsd 
S 0 0 777	/usr/etc/nfsd /usr/sbin/nfsd 
F 0 0 555	/usr/sbin/nfso 
S 0 0 777	/usr/etc/nfso /usr/sbin/nfso 
FT 0 0 4555	/usr/sbin/nfsstat 
ST 0 0 777	/usr/etc/nfsstat /usr/sbin/nfsstat 
F 0 0 555       /usr/sbin/rpc.lockd 
F 0 0 555	/usr/sbin/rpc.mountd 
S 0 0 777	/usr/etc/rpc.mountd /usr/sbin/rpc.mountd 
F 0 0 555	/usr/sbin/rpc.pcnfsd 
s 0 0 777	/etc/rpc.pcnfsd /usr/sbin/rpc.pcnfsd 
F 0 0 555	/usr/sbin/rpc.rexd 
S 0 0 777	/usr/etc/rpc.rexd /usr/sbin/rpc.rexd 
F 0 0 555	/usr/sbin/rpc.rstatd 
S 0 0 777	/usr/etc/rpc.rstatd /usr/sbin/rpc.rstatd 
F 0 0 555       /usr/sbin/rpc.statd 
F 0 0 555	/usr/sbin/rwall 
S 0 0 777	/usr/etc/rwall /usr/sbin/rwall 
F 0 0 555	/usr/sbin/spray 
S 0 0 777	/usr/etc/spray /usr/sbin/spray 
V 0 0 555	/usr/sbin/chnfs 
S 0 0 777	/usr/etc/chnfs /usr/sbin/chnfs 
V 0 0 555	/usr/sbin/lsnfsexp 
S 0 0 777	/usr/etc/lsnfsexp /usr/sbin/lsnfsexp 
V 0 0 555	/usr/sbin/lsnfsmnt 
S 0 0 777	/usr/etc/lsnfsmnt /usr/sbin/lsnfsmnt 
V 0 0 555	/usr/sbin/mknfs 
S 0 0 777	/usr/etc/mknfs /usr/sbin/mknfs 
V 0 0 555	/usr/sbin/mknfsexp 
S 0 0 777	/usr/etc/mknfsexp /usr/sbin/mknfsexp 
S 0 0 777	/usr/etc/chnfsexp /usr/sbin/mknfsexp 
S 0 0 777	/usr/sbin/chnfsexp /usr/sbin/mknfsexp 
V 0 0 555	/usr/sbin/mknfsmnt 
S 0 0 777	/usr/etc/mknfsmnt /usr/sbin/mknfsmnt 
S 0 0 777	/usr/etc/chnfsmnt /usr/sbin/mknfsmnt 
S 0 0 777	/usr/sbin/chnfsmnt /usr/sbin/mknfsmnt 
V 0 0 555	/usr/sbin/rmnfs 
S 0 0 777	/usr/etc/rmnfs /usr/sbin/rmnfs 
V 0 0 555	/usr/sbin/rmnfsexp 
S 0 0 777	/usr/etc/rmnfsexp /usr/sbin/rmnfsexp 
V 0 0 555	/usr/sbin/rmnfsmnt 
S 0 0 777	/usr/etc/rmnfsmnt /usr/sbin/rmnfsmnt 
