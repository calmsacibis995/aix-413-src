#!/bin/bsh
# @(#)10	1.1  src/packages/bos/ifor_ls/server/root/bos.ifor_ls.server.unconfig.sh, pkgnetls, pkg411, GOLD410 2/28/94 15:50:09
#
#   COMPONENT_NAME: pkgnetls
#
#   FUNCTIONS: Rmitab
#		Rmssys
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#check to see if use implemented daemon startup at boot time
Rmitab() {
	for i in `lsitab -a | grep $1 | cut -f1 -d":"`
	do
		inuumsg 116 $i
		rmitab $i 2>/dev/null
		if [ $? -ne 0 ]
	        then
			inuumsg 117 $i
			rc=1
		fi
	done
}

#Remove subsystem
Rmssys() {
	# Save LANG and restore when done.
	USER_LANG=$LANG
	LANG=C
	#Check to see if not stopped
	lssrc -s $1 |fgrep inoperative >/dev/null 
	if [ $? -ne 0 ]
	then
		#Check to see if it's not stopped because it doesn't exist.
		lssrc -s $1 >/dev/null
		if [ $? -eq 0 ]
		then
			LANG=$USER_LANG
			inuumsg 115 stopsrc $1
			rc=1

		fi
	fi
	LANG=$USER_LANG
	rmssys -s $1 2>&1 >/dev/null
	if [ $? -ne 0 ]
	then
		lssrc -s $1 >/dev/null
		if [ $? -eq 0 ]
		then
			inuumsg 115 rmssys $1
			rc=1
		fi
	fi 
}

# Set rc for an exit status of 0
rc=0

# Stop the netls subsystem
stopsrc -s netlsd 2>&1 >/dev/null

# Remove daemons started at boot time from /etc/inittab
Rmitab netlsd

# Remove netls subsystems
Rmssys netlsd

exit $rc
