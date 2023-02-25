#!/bin/bsh
#
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
CP_CMD="/bin/cp"
RM_CMD="/bin/rm"
RC_NFS="/etc/rc.nfs"
TMP_RC_NFS="/tmp/tmp.rc.nfs$$"
CFG_FILE="/etc/hanfs"
STOPSRC="/bin/stopsrc"

USAGE="
usage:  ${COMMAND_NAME}	[{-b|-r}]\n"

set -- `/bin/getopt br $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 52 2 "${USAGE}" ${COMMAND_NAME}
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-r")			#WHEN should this change take place (IPL)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 52 2 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="r"
	    shift
	    ;;
	"-b")			#WHEN should this take place (BOTH)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 52 2 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="b"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 52 2 "$USAGE" ${COMMAND_NAME}
	    exit 1
	    ;;
    esac
done

# check to make sure that there are not any extra parameters
shift		# get rid of the -- parameter
if [ -n "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 52 2 "${USAGE}" ${COMMAND_NAME}
    exit 1
fi

# Set the default for WHEN the 'operation' is to take place
# Set WHEN only if it has not been specified.
WHEN=${WHEN:-"b"}

# comment the entries in rc.nfs
${SED_CMD} "\?-x.*/usr/etc/hanfsd?,/fi/s/#//
	\?-x.*/usr/etc/hanfsd?,/fi/s/^/#/" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
	
if [ $? != 0 ] ; then
	dspmsg cmdnfs.cat -s 52 6 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	${RM_CMD} ${TMP_RC_TCPIP} > /dev/null 2>&1
fi

# copy the file back to /etc
${CP_CMD} ${TMP_RC_NFS} ${RC_NFS} > /dev/null 2>&1
if [ $? != 0 ] ; then
	dspmsg cmdnfs.cat -s 52 6 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	${RM_CMD} ${TMP_RC_NFS} > /dev/null 2>&1
fi

# Remove the temp file
${RM_CMD} ${TMP_RC_NFS} > /dev/null 2>&1

# Check to see if we need to stop hanfs
if [ "${WHEN}" = "b" ] ; then
	${STOPSRC} -g hanfsd
	# Remove the configuration file
	${RM_CMD} ${CFG_FILE} > /dev/null 2>&1
fi
exit $?




