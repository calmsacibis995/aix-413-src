#!/bin/bsh
#
# @(#)06	1.16  src/nfs/usr/sbin/sysmgt/mkslave.sh, cmdnfs, nfs411, GOLD410 5/10/94 11:13:57
#
# COMPONENT_NAME: (CMDNFS) Network File System Commands
#
# FUNCTIONS: tr_comma_colon()
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

# Variables used just within this script
YPINIT="/usr/sbin/ypinit"
COMMAND_NAME=`basename $0`
SED_CMD="/bin/sed"
RC_NFS="/etc/rc.nfs"
TMP_RC_NFS="/tmp/tmp.rc.nfs"
RC_TCPIP="/etc/rc.tcpip"
TMP_RC_TCPIP="/tmp/tmp.rc.tcpip"
CONTINUE="yes"
CFLAG="no"
MKITAB="/usr/sbin/mkitab"

USAGE="
usage:  ${COMMAND_NAME} [-O|-o] [-I|-B|-N] [-c|-C] master"

if [ -z "`/usr/bin/domainname`" ] ;then
        dspmsg cmdnfs.cat -s 37 43 "%s: The local host's domain name has not been set.  Please set it.\n" ${COMMAND_NAME}
        exit 1
fi

set -- `getopt CcOoIBN $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 40 "${USAGE}" ${COMMAND_NAME}
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-c")			# Should we continue on errors
	    if [ "${CFLAG}" = "yes" ] ; then dspmsg cmdnfs.cat -s 37 40 "${USAGE}" ${COMMAND_NAME}; exit 1 ; fi
	    CFLAG="yes"
	    CONTINUE="no"
	    shift
	    ;;
	"-C")			# Should we continue on errors
	    if [ "${CFLAG}" = "yes" ] ; then dspmsg cmdnfs.cat -s 37 40 "${USAGE}" ${COMMAND_NAME}; exit 1 ; fi
	    CFLAG="yes"
	    CONTINUE="yes"
	    shift
	    ;;
	"-o")			# Should we overwrite existing maps
	    if [ -n "${OVER_WRITE}" ] ; then dspmsg cmdnfs.cat -s 37 40 "${USAGE}" ${COMMAND_NAME}; exit 1 ; fi
	    OVER_WRITE="no"
	    shift
	    ;;
	"-O")			# Should we overwrite existing maps
	    if [ -n "${OVER_WRITE}" ] ; then dspmsg cmdnfs.cat -s 37 40 "${USAGE}" ${COMMAND_NAME}; exit 1 ; fi
	    OVER_WRITE="yes"
	    shift
	    ;;
	"-I")			#WHEN should this  take place (IPL)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 40 "$USAGE" ${COMMAND_NAME}; exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-N")			#WHEN should this  take place (NOW)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 40 "$USAGE" ${COMMAND_NAME}; exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-B")			#WHEN should this  take place (BOTH)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 40 "$USAGE" ${COMMAND_NAME}; exit 1 ; fi
	    WHEN="B"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 40 "$USAGE" ${COMMAND_NAME}
	    exit 1
	    ;;
    esac
done

# check to make sure that there are not any extra parameters
shift		# get rid of the -- parameter
if [ -z "$1" ] ; then	# we want something here.
    dspmsg cmdnfs.cat -s 37 40 "${USAGE}" ${COMMAND_NAME}
    exit 1
else
    MASTER="$1"
    shift
fi

if [ -n "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 40 "${USAGE}" ${COMMAND_NAME}
    exit 1
fi

OVER_WRITE=${OVER_WRITE:-"no"}
WHEN=${WHEN:-"B"}

# Build parameter list for ypinit
# Make sure we invoke it with the '-q' flag for quiet (no prompting)
PARMS="-q -s ${MASTER}"
if [ "${OVER_WRITE}" = "yes" ] ; then
    PARMS="${PARMS} -o"
fi

if [ "${CONTINUE}" = "yes" ] ; then
    PARMS="${PARMS} -n"
fi

# Make sure that the ypbind daemon is running so that the ypinit
# will procede correctly
/bin/startsrc -s ypbind

# Call ypinit with the correct parameters
${YPINIT} ${PARMS}
if [ $? != 0 ] ; then
    dspmsg cmdnfs.cat -s 37 22 "${COMMAND_NAME}: Exiting because of errors in ${YPINIT}\n" ${COMMAND_NAME} ${YPINIT}
    exit 1
fi

if [ "${WHEN}" = "I" -o "${WHEN}" = "B" ] ; then
        # Add  rc.nfs in /etc/inittab
	$MKITAB -i rctcpip "rcnfs:2:wait:/etc/rc.nfs > /dev/console 2>&1 # Start NFS Daemons"
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
	# Uncomment the line that says yp is being started
	# Uncomment the starting of the ypserv daemon
    	${SED_CMD} "/starting NIS services:/s/#//
		\?-x.*/usr/lib/netsvc/yp/ypserv?,\?fi?s/#//
		\?-x.*/usr/lib/netsvc/yp/ypbind?,\?fi?s/#//" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null

	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
		exit 1
	fi
	# copy the file back to /etc
	/bin/cp ${TMP_RC_NFS} ${RC_NFS} > /dev/null 2>&1
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	fi

fi

if [ "${WHEN}" = "N" -o "${WHEN}" = "B" ] ; then
    # Start the portmapper and refresh inetd
    startsrc -s portmap
    /bin/refresh -s inetd
    if [ "${COMMAND_NAME}" = "chslave" ] ; then
	/bin/stopsrc -s ypserv
	/bin/stopsrc -s ypbind
    fi
    /bin/startsrc -s ypserv
    /bin/startsrc -s ypbind
fi
    
