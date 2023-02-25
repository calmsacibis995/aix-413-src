#!/bin/bsh
#
# @(#)07	1.5  src/nfs/usr/sbin/sysmgt/rmkeyserv.sh, cmdnfs, nfs411, GOLD410 3/2/94 17:38:20
# COMPONENT_NAME: (CMDNFS) Network File System Commands
#
# FUNCTIONS:
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

COMMAND_NAME=`basename $0`
SED_CMD="/bin/sed"
RC_NFS="/etc/rc.nfs"
TMP_RC_NFS="/tmp/tmp.rc.nfs"

USAGE="
usage:  ${COMMAND_NAME} [-I|-B|-N]\n"

set -- `getopt IBN $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 5 "${USAGE}" ${COMMAND_NAME}
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-I")			#WHEN should this  take place (IPL)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 5 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-N")			#WHEN should this  take place (NOW)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 5 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-B")			#WHEN should this  take place (BOTH)
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

# Set the defaults for this command
WHEN=${WHEN:-"B"}


if [ "${WHEN}" = "I" -o "${WHEN}" = "B" ] ; then
    	${SED_CMD} "\?-x.*/usr/sbin/keyserv?,/fi/s/#//
		\?-x.*/usr/sbin/keyserv?,/fi/s/^/#/"  ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null

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
    stopsrc -s keyserv
fi
    
