#! /bin/ksh
# @(#)05	1.5  src/tcpip/usr/samples/tcpip/onhost/onhost.make, tcpip, tcpip411, GOLD410 2/13/94 15:01:53
#
#  COMPONENT_NAME: TCPIP onhost.make
#
#  FUNCTIONS:
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1986, 1988, 1989
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES
#
# INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
# EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
# WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
# LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
# OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
# IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
# DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
# DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
# CORRECTION.
#
#  RISC System/6000 is a trademark of International Business Machines
#   Corporation.
#
#
# Determine system type, then make hostconnect and onhost, version 1.1
# AIXwhat is onhost.make 1.7 PASC 1.7
#
# This script uses onhost.mk to make hostconnect and onhost for this system.
#
ALL='hostconnect onhost'
CC=cc
LIBS=''
TARGET=''
#
if [ $# -gt 0 ] ; then
TARGET=$1
fi
#
#	Discover which kind of system we are on
#	and invoke make with appropriate options.
#
#	System must have tcp/ip (AF_INET) sockets.
#	The main decision is System V or BSD
#
#	The following flags are used in hostconnect and onhost.
#	-DAIXV3 for AIX v3
#	-DBSD42LIB for BSD 4.2 and up
#	-DLDSF enables VM connection feature available only on AIX 370
#	-DSHORT truncates alias names for short file directory systems
#	-DSIGVEC select the enhanced signal capability
#	-DSYSVLIB for System V
#	-DTNOLD generates tn -p option for pre-AIX/RT 2.2.1 version of tn
#
#	The following code tries to determine what system we are on but
#	the tests are not foolproof.
#
if [ -f /etc/site ] ; then
	if u370 ; then
		echo make hostconnect and onhost for AIX/370
		DFLAGS='-DSYSVLIB -DLDSF -DSIGVEC'
#
	elif i386; then
		echo make hostconnect and onhost for AIX/386
		DFLAGS='-DSYSVLIB -DSIGVEC'
	fi
#
elif [ -s /vrm ] ; then
	echo make hostconnect and onhost for AIX/RT
	DFLAGS='-DSYSVLIB -DSIGVEC -DSHORT'
	if [ ! -s /usr/bin/telnet ] # AIX 2.2.1 adds telnet command
	then
		echo prior to release 2.2.1
		DFLAGS='-DSYSVLIB -DSIGVEC -DTNOLD -DSHORT'
		LIBS='-lsock'
	fi
#
elif [ -d /etc/objrepos ] ; then
	echo make hostconnect and onhost for AIX v3
	ALL='msgcat hostconnect onhost'
	DFLAGS='-DAIXV4 -DAIXV3 -DSYSVLIB -DSIGVEC -D_NO_PROTO'
#
elif [ -f /usr/include/sgtty.h ] ; then
	echo make hostconnect and onhost for ACIS 4.3/RT
	DFLAGS='-DBSD42LIB -DSIGVEC'
else
	echo The tests as coded can not determine the kind of system so
	echo onhost.make must be modified.
	exit
fi
#
make $TARGET -f onhost.mk "CC=$CC" "DFLAGS=$DFLAGS" "LIBS=$LIBS" "ALL=$ALL"
