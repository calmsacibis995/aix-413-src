#!/bin/ksh
# @(#)86	1.4  src/packages/bos/sysmgt/nim/master/usr/bos.sysmgt.nim.master.unconfig.sh, pkgnim, pkg411, GOLD410  6/2/94  18:07:39
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: 
#		/src/packages/bos/sysmgt/nim/client/bos.sysmgt.nim.master.unconfig.sh
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# check for the NIM_BOOT_ENV variable: when set, it indicates what we're
#	executing from the network boot environment (so we don't want certain
#	things to happen)
if [[ -z "${NIM_BOOT_ENV}" ]]
then

	# remove the /etc/niminfo file
	rm /etc/niminfo 2>/dev/null

	# stop nim subservers
	/usr/bin/stopsrc -g nim 2>/dev/null

	# remove the nim subservers
	/usr/bin/rmssys -s nimesis 2>/dev/null
	/usr/bin/rmssys -s nimd 2>/dev/null

fi

# remove nim from /etc/inittab
/usr/sbin/rmitab nim 2>/dev/null

# if there was a previous niminfo file, mv it back
[[ -f /etc/niminfo.prev ]] && /usr/bin/mv /etc/niminfo.prev /etc/niminfo

# if there was a previous rhost file, mv it back
[[ -f /.rhosts.prev ]] && /usr/bin/mv /.rhosts.prev /.rhosts

if [[ -z "${NIM_BOOT_ENV}" ]]
then

	# if there was a previous inittab entry for nimclient, put it back
	itab="$(/usr/bin/cat /usr/lpp/bos.sysmgt/nim.client.itab)"
	[[ -n "$itab" ]] && /usr/sbin/mkitab -i cron "${itab}"
	rm /usr/lpp/bos.sysmgt/nim.client.itab 2>/dev/null

fi

exit 0

