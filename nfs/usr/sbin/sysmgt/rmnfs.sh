#!/bin/bsh
# @(#)08	1.20  src/nfs/usr/sbin/sysmgt/rmnfs.sh, cmdnfs, nfs411, 9440A411a 10/3/94 18:23:42
# COMPONENT_NAME: (CMDNFS) Network File System Commands
#
# FUNCTIONS: rmnfs
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
SED_CMD="/bin/sed"
RC_TCPIP="/etc/rc.tcpip"
INETD_CONF="/etc/inetd.conf"
TMP_INETD_CONF="/tmp/tmp.inetd.conf$$"
TMP_OUT_AWK="/tmp/tmp.out.awk$$"

USAGE="
usage:  ${COMMAND_NAME}	[{-I|-B|-N}]\n"

stop_daemons()
{
        if [ -f /usr/sbin/biod ]; then
                stopsrc -f -s biod
        fi
        if [ -f /usr/sbin/nfsd ]; then
                stopsrc -f -s nfsd
        fi
        if [ -f /usr/sbin/rpc.mountd ]; then
                stopsrc -f -s rpc.mountd
        fi
        if [ -f /usr/sbin/rpc.lockd ]; then
                stopsrc -f -s rpc.lockd
        fi
        if [ -f /usr/sbin/rpc.statd ]; then
                stopsrc -f -s rpc.statd
        fi
        if [ -f /usr/bin/domainname ]; then
                /usr/bin/domainname ""
        fi
        if [ -f /usr/lib/netsvc/yp/ypserv ]; then
                stopsrc -f -s ypserv
        fi
        if [ -f /usr/lib/netsvc/yp/ypbind ]; then
                stopsrc -f -s ypbind
        fi
        if [ -f /usr/lib/netsvc/yp/rpc.yppasswdd ]; then
                stopsrc -f -s rpc.yppasswdd
        fi
        if [ -f /usr/lib/netsvc/yp/rpc.ypupdated ]; then
                stopsrc -f -s rpc.ypupdated
        fi
        if [ -f /usr/sbin/keyserv ]; then
                stopsrc -f -s keyserv
        fi
}

set -- `/bin/getopt IBN $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 5 "${USAGE}" ${COMMAND_NAME}
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-I")			#WHEN should this change take place (IPL)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 5 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-N")			#WHEN should this change take place (NOW)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 5 "$USAGE" ${COMMAND_NAME}; exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-B")			#WHEN should this take place (BOTH)
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
    "N")	# stop nfs without changing INITTAB
	# Stop NFS
	TEMP=""

	# Call function to stop daemon process etc...
	stop_daemons

	TEMP=`/bin/lssrc -g keyserv | /bin/awk '{ if($3 == "inoperative") { print "OK"} }'`
	if [ "${TEMP}" = "OK" ] ; then
		exit 0
	else 
		exit 1
	fi
	;;
    "I"|"B")	# add the entry to inittab (if it doesn't exist)
	/usr/sbin/rmitab rcnfs
	if [ $? -ne 0 ] ; then
		dspmsg cmdnfs.cat -s 37 30 "${COMMAND_NAME}: error in removing \"rcnfs\" entry from inittab\n" ${COMMAND_NAME}
	fi
	
	# comment the entries in /etc/inetd.conf that are sunrpc daemons
	${SED_CMD} "\?^rexd.*[	, ]sunrpc_tcp.*wait.*root?s/^/#/
		\?^rstatd.*[	, ]sunrpc_udp.*wait.*root?s/^/#/
		\?^rusersd.*[	, ]sunrpc_udp.*wait.*root?s/^/#/
		\?^rwalld.*[	, ]sunrpc_udp.*wait.*root?s/^/#/
		\?^sprayd.*[	, ]sunrpc_udp.*wait.*root?s/^/#/
		\?^pcnfsd.*[	, ]sunrpc_udp.*wait.*root?s/^/#/" ${INETD_CONF} > ${TMP_INETD_CONF} 2>/dev/null
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${INETD_CONF}\n" ${COMMAND_NAME} ${INETD_CONF}
		/bin/rm ${TMP_INETD_CONF} > /dev/null 2>&1
	fi

	# copy the file back to /etc
	/bin/cp ${TMP_INETD_CONF} ${INETD_CONF} > /dev/null 2>&1
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${INETD_CONF}\n" ${COMMAND_NAME} ${INET_CONF}
		/bin/rm ${TMP_INETD_CONF} > /dev/null 2>&1
	fi
	
	# Remove the temporary file
	/bin/rm ${TMP_INETD_CONF} > /dev/null 2>&1

	# Check to see if we need to stop nfs
	if [ "${WHEN}" = "B" ] ; then
	    /bin/refresh -s inetd
	    # Call function to stop daemon process etc...
	    stop_daemons
	fi
	;;
esac
exit $?


