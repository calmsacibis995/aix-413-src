#!/bin/ksh
# @(#)11        1.1  src/packages/bos/sysmgt/nim/master/usr/bos.sysmgt.nim.master.unpre_i.sh, pkgnim, pkg411, GOLD410 4/14/94 06:26:40
#
# COMPONENT_NAME: pkgnim
#
# FUNCTIONS:  .unpre_i
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
# -------------------------------------------------------------------------

#
# this script is run when the bos.sysmgt.nim.master fileset is deinstalled
# this script removes the master SMIT screens and replaces them with the
#		bos.sysmgt.nim.client screens, which were saved off by the
#		bos.sysmgt.nim.master.pre_i script
tmp_file=/usr/lpp/bos.sysmgt/nim.client.screens

#
# first, delete the master SMIT screens
#
ODMDIR=/usr/lib/objrepos odmdelete -q"next_id like 'nim*'" \
	-o sm_menu_opt >/dev/null
ODMDIR=/usr/lib/objrepos odmdelete -q"next_id like 'iplrom'" \
	-o sm_menu_opt >/dev/null
ODMDIR=/usr/lib/objrepos odmdelete -q"id like 'nim*'" \
	-o sm_cmd_hdr >/dev/null
ODMDIR=/usr/lib/objrepos odmdelete -q"id like 'iplrom*'" \
	-o sm_cmd_hdr >/dev/null
ODMDIR=/usr/lib/objrepos odmdelete -q"id like 'nim*'" \
	-o sm_cmd_opt >/dev/null
ODMDIR=/usr/lib/objrepos odmdelete -q"id like 'iplrom*'" \
	-o sm_cmd_opt >/dev/null
ODMDIR=/usr/lib/objrepos odmdelete -q"id like 'nim*'" \
	-o sm_name_hdr >/dev/null

#
# now, put the client screens back
#
if [[ -s $tmp_file ]]
then
	ODMDIR=/usr/lib/objrepos odmadd $tmp_file
	rm $tmp_file 2>/dev/null
fi

exit 0
