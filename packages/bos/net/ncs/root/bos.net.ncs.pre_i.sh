#!/bin/ksh
# @(#)26	1.3  src/packages/bos/net/ncs/root/bos.net.ncs.pre_i.sh, pkgncs, pkg411, GOLD410 4/14/94 09:32:07
#
#   COMPONENT_NAME: pkgncs
#
#   FUNCTIONS: Chksrc
#
#   ORIGINS: 27,84
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# 

# Function to check for the indicated srcmstr state.
# $1 is checked for state $2.
Chksrc() {
	# Save LANG and restore when done.
	USER_LANG=$LANG
	LANG=C
	checkrc=0
	lssrc -a | grep $1 | grep $2 >/dev/null || checkrc=1
	LANG=$USER_LANG
	return $checkrc
}

rc=0

# test for NCS present
if [ -d /etc/ncs ]
	then
        inuumsg 110
fi

# stop currently active brokers
stopsrc -cs glbd 2>&1 >/dev/null
stopsrc -cs nrglbd 2>&1 >/dev/null
stopsrc -cs llbd 2>&1 >/dev/null

# Remove llbd and nrglbd SRC subsystems
# Include error checking to make sure the subsystems are not active before removing them,
# and exit if active or rmssys fails.

export LANG

if Chksrc nrglbd inoperative

	then
	rmssys -s nrglbd || (inuumsg 111 nrglbd ; rc=1) || rc=1

	elif 
	Chksrc nrglbd active

	then
	inuumsg 112 nrglbd 
	rc=1
fi

if Chksrc glbd inoperative

	then
	rmssys -s glbd || (inuumsg 111 glbd ; rc=1) || rc=1

	elif 
	Chksrc glbd active

	then
	inuumsg 112 glbd 
	rc=1
fi

if Chksrc llbd inoperative

        then
        rmssys -s llbd || (inuumsg 111 llbd ; rc=1) || rc=1

        elif 
	Chksrc llbd active

        then
        inuumsg 112 llbd
        rc=1
fi


# Testing for RLM installation

if [ -d /usr/lpp/rlm ]
	then
        inuumsg 113

        stopsrc -cs rlmd 2>&1 >/dev/null

	if Chksrc rlmd inoperative

		then
		rmssys -s rlmd || (inuumsg 111 rlmd ; rc=1) || rc=1

		elif
		Chksrc rlmd active

		then
                inuumsg 112 rlmd
		rc=1
	fi

fi

exit $rc
