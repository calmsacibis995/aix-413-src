# @(#)25	1.6  src/packages/devices/mca/8fc3/rte/devices.mca.8fc3.rte.il, pkgdevcommo, pkg411, GOLD410 1/28/94 17:46:23
#
# COMPONENT_NAME: pkgdevcommo
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
# LPP_NAME: devices.mca.8fc3.rte
#
f 0 0 755	/etc/Force_esca dd-mca
f 0 0 644	/etc/objrepos/Connection dd-mca
f 0 0 644	/etc/objrepos/CuSubChannel dd-mca
f 0 0 644	/etc/objrepos/Fibre dd-mca
f 0 0 755	/etc/rc.VPD dd-mca
F 0 0 755	/usr/bin/ConsTerm dd-mca
F 0 0 755	/usr/bin/DefIfes dd-mca
F 0 0 755	/usr/bin/DoAdapForm dd-mca
F 0 0 755	/usr/bin/DoCond dd-mca
S 0 0 777	/usr/bin/addCond /usr/bin/DoCond dd-mca
S 0 0 777	/usr/bin/chgCond /usr/bin/DoCond dd-mca
F 0 0 755	/usr/bin/DoFibd dd-mca
S 0 0 777	/usr/bin/addFibd /usr/bin/DoFibd dd-mca
S 0 0 777	/usr/bin/chgFibd /usr/bin/DoFibd dd-mca
F 0 0 755	/usr/bin/DoIfesForm dd-mca
F 0 0 755	/usr/bin/DoIfesd dd-mca
F 0 0 755	/usr/bin/DoOdmForm dd-mca
F 0 0 755	/usr/bin/DoRAS dd-mca
F 0 0 755	/usr/bin/DoSubd dd-mca
S 0 0 777	/usr/bin/addSubd /usr/bin/DoSubd dd-mca
S 0 0 777	/usr/bin/chgSubd /usr/bin/DoSubd dd-mca
F 0 0 755	/usr/bin/DontMsg dd-mca
F 0 0 755	/usr/bin/ForceOff dd-mca
F 0 0 755	/usr/bin/ListOpens dd-mca
F 0 0 755	/usr/bin/ListRAS dd-mca
F 0 0 755	/usr/bin/OffEscon dd-mca
F 0 0 755	/usr/bin/OnEscon dd-mca
F 0 0 755	/usr/bin/StStSub dd-mca
F 0 0 755	/usr/bin/StartStop dd-mca
F 0 0 755	/usr/bin/SubAll_addr dd-mca
F 0 0 755	/usr/bin/SubState dd-mca
F 0 0 755	/usr/bin/Subd_munge dd-mca
F 0 0 755	/usr/bin/TakeOff dd-mca
F 0 0 755	/usr/bin/ToHex dd-mca
F 0 0 755	/usr/bin/adap_munge dd-mca
F 0 0 755	/usr/bin/addr_munge dd-mca
F 0 0 755	/usr/bin/chgesca dd-mca
F 0 0 755	/usr/bin/if_es_list dd-mca
S 0 0 777	/usr/bin/list_def_if_es /usr/bin/if_es_list dd-mca
S 0 0 777	/usr/bin/list_ndef_if_es /usr/bin/if_es_list dd-mca
F 0 0 755	/usr/bin/list_SubSets dd-mca
F 0 0 755	/usr/bin/machVPD dd-mca
S 0 0 777	/usr/bin/cook_machVPD /usr/bin/machVPD dd-mca
S 0 0 777	/usr/bin/set_machVPD /usr/bin/machVPD dd-mca
S 0 0 777	/usr/bin/list_machVPD /usr/bin/machVPD dd-mca
F 0 0 755	/usr/bin/readlog dd-mca
F 0 0 755	/usr/bin/tuneesca dd-mca
F 0 0 644	/usr/include/sys/escadd.h dd-mca
F 0 0 644	/usr/include/sys/escadefs.h dd-mca
F 0 0 644	/usr/include/sys/escamagic.h dd-mca
F 0 0 644	/usr/include/sys/escauser.h dd-mca
F 0 0 644	/usr/include/sys/load_i960.h dd-mca
F 0 0 644	/usr/include/sys/mt3ipca.h dd-mca
F 0 0 644	/usr/lib/drivers/escadd dd-mca
F 0 0 644	/usr/lib/drivers/if_es dd-mca
F 0 0 500	/usr/lib/methods/cfgescon dd-mca
F 0 0 500	/usr/lib/methods/ucfgescon dd-mca
F 0 0 500	/usr/lib/methods/undefescon dd-mca

# The following files should probably be shipped in devices.mca.8fc3.diag
#F 0 0 544	/usr/lpp/diagnostics/bin/uescon
#F 0 0 555	/usr/lpp/diagnostics/catalog/default/dnsca.cat
#F 0 0 555	/usr/lpp/diagnostics/catalog/default/uescon.cat
#F 0 0 544	/usr/lpp/diagnostics/da/dnsca

# The following files should probably be shipped in bos.msg.en_US.dev.mca.8fc3.rte
#F 0 0 644	/usr/lpp/msg/En_US/cfgescon.cat
#F 0 0 644	/usr/lpp/msg/En_US/esca.cat
#F 0 0 644	/usr/lpp/msg/En_US/esca_help.cat
#F 0 0 644	/usr/lpp/msg/En_US/escon.cat
