#!/bin/bsh
# @(#)02	1.17  src/nfs/usr/sbin/sysmgt/mknfs.sh, cmdnfs, nfs411, 9440A411a 10/3/94 18:23:34
# COMPONENT_NAME: (CMDNFS) Network File System Commands
#
# FUNCTIONS: mknfs
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
set -f			# this is so that the case statement will work

COMMAND_NAME=`/bin/basename $0`
SED_CMD="/usr/bin/sed"
MKITAB="/usr/sbin/mkitab"
RC_TCPIP="/etc/rc.tcpip"
TMP_RC_TCPIP="/tmp/tmp.rc.tcpip$$"
INETD_CONF="/etc/inetd.conf"
TMP_INETD_CONF="/tmp/tmp.inetd.conf$$"

USAGE="
usage:  ${COMMAND_NAME}	[{-I|-B|-N}]\n"

# Parse the command line arguments to find out what directory
# is to be removed from mounting.

set -- `/usr/bin/getopt IBN $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 5 "${USAGE}" $COMMAND_NAME
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-I")			#WHEN should this mount take place (IPL)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 5 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-N")			#WHEN should this mount take place (NOW)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s47 5 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-B")			#WHEN should this mount take place (BOTH)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 5 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="B"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 5 "$USAGE" ${COMMAND_NAME}
	    exit 1
	    ;;
    esac
done

# check to make sure that there are not any extra parameters
shift		# get rid of the -- parameter
if [ -n "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 5 "${USAGE}" ${COMMAND_NAME}
    exit 1
fi

# Set the default for WHEN the 'operation' is to take place
# Set WHEN only if it has not been specified.
WHEN=${WHEN:-"B"}

case "${WHEN}" in
    "N")	# remove the entry in inittab and start the nfs group
	# Start NFS
	/usr/bin/startsrc -s portmap
	/etc/rc.nfs
	;;
    "I"|"B")	# add the entry to inittab (if it doesn't exist)
	/usr/sbin/lsitab rcnfs > /dev/null 2>&1 	# check to see if it is there
	if [ $? -ne 0 ] ; then
		$MKITAB -i rctcpip "rcnfs:2:wait:/etc/rc.nfs > /dev/console 2>&1 # Start NFS Daemons"
		if [ $? -ne 0 ] ; then
			dspmsg cmdnfs.cat -s 37 23 "${COMMAND_NAME}: error in adding entry to inittab\n" ${COMMAND_NAME}
			exit 1
		fi
	fi
	# Uncomment the starting of portmap in /etc/rc.tcpip
	${SED_CMD} "\?.*/usr/sbin/portmap?s/^#//" ${RC_TCPIP} > ${TMP_RC_TCPIP} 2>/dev/null
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_TCPIP}\n" ${COMMAND_NAME} ${RC_TCPIP}
		/bin/rm ${TMP_RC_TCPIP} > /dev/null 2>&1
		exit 1
	fi
	#copy the file back to /etc
	/bin/cp ${TMP_RC_TCPIP} ${RC_TCPIP} > /dev/null 2>&1
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_TCPIP}\n" ${COMMAND_NAME} ${RC_TCPIP}
		/bin/rm ${TMP_RC_TCPIP} > /dev/null 2>&1
		exit 1	
	fi
	
	# Remove the temporary file
	/bin/rm ${TMP_RC_TCPIP} > /dev/null 2>&1

	# Uncomment the starting of the rpc daemons in /etc/inetd.conf
        ${SED_CMD} "\?#rstatd.*[   , ]sunrpc_udp.*wait.*root?s/#//
                \?#rusersd.*[  , ]sunrpc_udp.*wait.*root?s/#//
                \?#rwalld.*[   , ]sunrpc_udp.*wait.*root?s/#//
                \?#sprayd.*[   , ]sunrpc_udp.*wait.*root?s/#//
                \?#pcnfsd.*[   , ]sunrpc_udp.*wait.*root?s/#//" ${INETD_CONF} > ${TMP_INETD_CONF} 2>/dev/null
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${INETD_CONF}\n" ${COMMAND_NAME} ${INETD_CONF}
		/bin/rm ${TMP_INETD_CONF} > /dev/null 2>&1
		exit 1
	fi
	#copy the file back to /etc
	/bin/cp ${TMP_INETD_CONF} ${INETD_CONF} > /dev/null 2>&1
	if [ $? != 0 ] ; then
	 	dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${INETD_CONF}\n" ${COMMAND_NAME} ${INETD_CONF}
		/bin/rm ${TMP_INETD_CONF} > /dev/null 2>&1
		exit 1	
	fi
	
	# Remove the temporary file
	/bin/rm ${TMP_INETD_CONF} > /dev/null 2>&1

	# Check to see if we need to start nfs
	if [ "${WHEN}" = "B" ] ; then
		/bin/startsrc -s portmap
		/etc/rc.nfs 
		/bin/refresh -s inetd
	fi
	;;
esac
exit $?
