#!/bin/bsh
# @(#)45	1.6  src/nfs/usr/sbin/sysmgt/chypdom.sh, cmdnfs, nfs411, GOLD410 3/14/94 14:04:47
# COMPONENT_NAME: (CMDNFS) Network File System Commands
#
# FUNCTIONS: chypdom
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
DOMAINNAME_CMD="/usr/bin/domainname"
RC_NFS="/etc/rc.nfs"
TMP_RC_NFS="/tmp/rc.nfs$$"
SED_CMD="/bin/sed"

USAGE="usage:  ${COMMAND_NAME} [{-I|-B|-N}] newdomainname\n"

set -- `/bin/getopt IBN $*  2>/dev/null` 

if [ -z "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 2 "${USAGE}"
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-I")			#WHEN should this name change take effect
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 2 "$USAGE"; exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-N")			#WHEN should this take place (NOW)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 2 "$USAGE"; exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-B")			#WHEN should this take place (BOTH)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 2 "$USAGE"; exit 1 ; fi
	    WHEN="B"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 2 "$USAGE"
	    exit 1
	    ;;
    esac
done

# Make sure that we have something to use for the new domainname
shift		# get rid of the -- parameter
if [ -z "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 2  "${USAGE}"
    exit 1
else
    DOMAINNAME=$1
fi

# check to make sure that there are not any extra parameters
if [ -n "$2" ] ; then
	dspmsg cmdnfs.cat -s 37 2 "${USAGE}"
	exit 1
fi

# Set the default for WHEN the 'operation' is to take place
# Set WHEN only if it has not been specified.
WHEN=${WHEN:-"B"}

case "${WHEN}" in
    "N")	# should we run the domainname command and forget about it?
	${DOMAINNAME_CMD} ${DOMAINNAME} > /dev/null 2>&1
	if [ $? != 0 ] ; then
	    dspmsg cmdnfs.cat -s 37 13 "${COMMAND_NAME}: error in updating the domainname of the system\n" ${COMMAND_NAME}
	    exit 1
	fi
	;;
    "I"|"B")	# add the entry to inittab (if it doesn't exist)

	# Make sure that the lines in the /etc/rc.nfs file that take care
	# of setting the domainname of the machine are uncommented.
	${SED_CMD} "\?/usr/bin/domainname?,\?^[#]*fi?s/#//" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
		/usr/bin/rm -f ${TMP_RC_NFS} 2>/dev/null
		exit 1
	fi
	# copy the file back to /etc
	/bin/cp ${TMP_RC_NFS} ${RC_NFS} > /dev/null 2>&1
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	fi

	# Now to the task of updating the name that is used in /etc/rc.nfs
	${SED_CMD} "s?^[^!-]*/usr/bin/domainname.*?	/usr/bin/domainname ${DOMAINNAME}?g" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
		/usr/bin/rm -f ${TMP_RC_NFS} 2>/dev/null
		exit 1
	fi
	# copy the file back to /etc
	/bin/cp ${TMP_RC_NFS} ${RC_NFS} > /dev/null 2>&1
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	fi
	
	# Are we supposed to update the domainname now?
	if [ "${WHEN}" = "B" ] ; then
		${DOMAINNAME_CMD} ${DOMAINNAME}
		if [ $? != 0 ] ; then
		    dspmsg cmdnfs.cat -s 37 13 "${COMMAND_NAME}: error in updating the domainname of the system\n" ${COMMAND_NAME}
		    /usr/bin/rm -f ${TMP_RC_NFS} 2>/dev/null
		    exit 1
		fi
	fi
	;;
esac

/usr/bin/rm -f ${TMP_RC_NFS} 2>/dev/null
exit $?





