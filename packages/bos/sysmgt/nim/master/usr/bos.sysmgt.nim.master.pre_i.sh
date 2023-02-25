#!/bin/ksh
# @(#)12        1.2  src/packages/bos/sysmgt/nim/master/usr/bos.sysmgt.nim.master.pre_i.sh, pkgnim, pkg411, GOLD410 4/18/94 15:03:39
#
# COMPONENT_NAME: pkgnim
#
# FUNCTIONS:  .pre_i
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
# this script is run when the bos.sysmgt.nim.master fileset is installed
# 		and is used to remove the bos.sysmgt.nim.client SMIT screens
# these screens are mutually exclusive with the master screens and will already
#		be present if the bos.sysmgt.nim.client fileset was previously installed
# NOTE that the bos.sysmgt.nim.client screens are saved off in a file named
#		"/usr/lpp/bos.sysmgt/nim.client.screens"; this file will be used by the
#		bos.sysmgt.nim.master.unpre_i script to restore the client screens when
#		the master package is deinstalled
#
tmp_file=/usr/lpp/bos.sysmgt/nim.client.screens
client_odmadd=/usr/lpp/bos.sysmgt/bos.sysmgt.nim.client.sm_nim_client.odmadd

#
# first, save the client screens
#
ODMDIR=/usr/lib/objrepos odmget -q"next_id like 'nim*'" \
	sm_menu_opt > $tmp_file
ODMDIR=/usr/lib/objrepos odmget -q"next_id like 'iplrom'" \
	sm_menu_opt >> $tmp_file
ODMDIR=/usr/lib/objrepos odmget -q"id like 'nim*'" \
	sm_cmd_hdr >> $tmp_file
ODMDIR=/usr/lib/objrepos odmget -q"id like 'iplrom*'" \
	sm_cmd_hdr >> $tmp_file
ODMDIR=/usr/lib/objrepos odmget -q"id like 'nim*'" \
	sm_cmd_opt >> $tmp_file
ODMDIR=/usr/lib/objrepos odmget -q"id like 'iplrom*'" \
	sm_cmd_opt >> $tmp_file
ODMDIR=/usr/lib/objrepos odmget -q"id like 'nim*'" \
	sm_name_hdr >> $tmp_file

#
# now, delete them the NIM client smit screens so there won't be conflicts
#		when the master screens get installed
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
# now, check to see if the bos.sysmgt.nim.client.sm_nim_client.odmadd file
#		is around; if so, this indicates that the client and master filesets are
#		being installed at the same time and that the client screens haven't
#		been added yet
[[ -s $client_odmadd ]] && mv $client_odmadd $tmp_file

exit 0
