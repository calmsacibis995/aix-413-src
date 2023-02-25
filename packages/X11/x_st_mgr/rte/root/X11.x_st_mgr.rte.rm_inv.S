# @(#)74	1.1  src/packages/X11/x_st_mgr/rte/root/X11.x_st_mgr.rte.rm_inv.S, pkgxstmgr, pkg411, GOLD410 7/26/94 17:46:42
#
#   COMPONENT_NAME: PKGXSTMGR
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
# Below are files that were removed in the move from X11R4 to X11R5 and since
# X11R4 is not available in 4.1 they also need to go.
/usr/lpp/x_st_mgr/bin/bootfile:
	symlinks=/etc/x_st_mgr/bootfile
/etc/x_st_mgr/bootfile.cf:
#
/usr/lpp/x_st_mgr/bin/bootfile1:
	symlinks=/etc/x_st_mgr/bootfile1
/etc/x_st_mgr/bootfile1.cf:
#
/usr/lpp/x_st_mgr/bin/bootfile2:
	symlinks=/etc/x_st_mgr/bootfile2
/etc/x_st_mgr/bootfile2.cf:
#
#  For space issues we are going to move the actual binaries for download onto
# the Xstation 140 and 150 from the / file system to the /usr file system.  The
# old files need to be removed.
#
/etc/x_st_mgr/xs150/upd/xst.abs.Z:
/etc/x_st_mgr/xs150/upd/post:
/etc/x_st_mgr/xs150/upd/nl_BE.km.Z:
/etc/x_st_mgr/xs150/upd/it_IT.km.Z:
/etc/x_st_mgr/xs150/upd/fr_FR.km.Z:
/usr/lpp/x_st_mgr/bin/X150serv:
	symlink=/etc/x_st_mgr/xs150/upd/file.list
/etc/x_st_mgr/xs150/upd/es_ES.km.Z:
/etc/x_st_mgr/xs150/upd/en_US.km.Z:
/etc/x_st_mgr/xs150/upd/de_DE.km.Z:
/etc/x_st_mgr/xs150/upd/Spanish.msg:
/etc/x_st_mgr/xs150/upd/Italian.msg:
/etc/x_st_mgr/xs150/upd/It_IT.km.Z:
/etc/x_st_mgr/xs150/upd/German.msg:
/etc/x_st_mgr/xs150/upd/French.msg:
/etc/x_st_mgr/xs150/upd/Fr_FR.km.Z:
/etc/x_st_mgr/xs150/upd/English.msg:
/etc/x_st_mgr/xs150/upd/Belgian_Dutch.msg:
/etc/x_st_mgr/xs150/upd:
#
/etc/x_st_mgr/xs140/upd/xst.abs.Z:
/etc/x_st_mgr/xs140/upd/post:
/etc/x_st_mgr/xs140/upd/nl_BE.km.Z:
/etc/x_st_mgr/xs140/upd/it_IT.km.Z:
/etc/x_st_mgr/xs140/upd/fr_FR.km.Z:
/usr/lpp/x_st_mgr/bin/X140serv:
	symlink=/etc/x_st_mgr/xs140/upd/file.list
/etc/x_st_mgr/xs140/upd/es_ES.km.Z:
/etc/x_st_mgr/xs140/upd/en_US.km.Z:
/etc/x_st_mgr/xs140/upd/de_DE.km.Z:
/etc/x_st_mgr/xs140/upd/Spanish.msg:
/etc/x_st_mgr/xs140/upd/Italian.msg:
/etc/x_st_mgr/xs140/upd/It_IT.km.Z:
/etc/x_st_mgr/xs140/upd/German.msg:
/etc/x_st_mgr/xs140/upd/French.msg:
/etc/x_st_mgr/xs140/upd/Fr_FR.km.Z:
/etc/x_st_mgr/xs140/upd/English.msg:
/etc/x_st_mgr/xs140/upd/Belgian_Dutch.msg:
/etc/x_st_mgr/xs140/upd:
