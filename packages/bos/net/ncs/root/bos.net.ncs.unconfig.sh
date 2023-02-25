#!/bin/bsh
# @(#)28	1.2  src/packages/bos/net/ncs/root/bos.net.ncs.unconfig.sh, pkgncs, pkg411, GOLD410 4/14/94 09:43:01
# 
#
# COMPONENT_NAME: pkgncs
#
# FUNCTIONS:  
#
# ORIGINS: 27, 84
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
			RC=1
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
			RC=1

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
			RC=1
		fi
	fi 
}

# Set RC for an exit status of 0
RC=0

# Stop the subsystems
stopsrc -s nrglbd 2>&1 >/dev/null
stopsrc -s glbd 2>&1 >/dev/null
stopsrc -s llbd 2>&1 >/dev/null

# Remove daemons started at boot time from /etc/inittab
Rmitab rcncs

# Remove subsystems
Rmssys llbd
Rmssys glbd
Rmssys nrglbd

exit $RC
