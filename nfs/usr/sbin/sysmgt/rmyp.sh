#!/bin/bsh
#
# @(#)11	1.11  src/nfs/usr/sbin/sysmgt/rmyp.sh, cmdnfs, nfs411, GOLD410 3/2/94 11:07:27
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
TMP_RC_NFS="/tmp/rc.nfs$$"


USAGE="
usage:  ${COMMAND_NAME} {-s | -c}\n"

set -- `getopt sc $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 12 "${USAGE}"
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-s")			# Remove the server definition
	    if [ -n "${WHICH}" ] ; then dspmsg cmdnfs.cat -s 37 12 "${USAGE}"; exit 1 ; fi
	    WHICH="server"
	    shift
	    ;;
	"-c")			# Remove the client definition
	    if [ -n "${WHICH}" ] ; then dspmsg cmdnfs.cat -s 37 12 "${USAGE}"; exit 1 ; fi
	    WHICH="client"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 12 "$USAGE"
	    exit 1
	    ;;
    esac
done

# check to make sure that there are not any extra parameters
shift		# get rid of the -- parameter
if [ -n "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 12 "${USAGE}"
    exit 1
fi

if [ -z "${WHICH}" ] ; then
    dspmsg cmdnfs.cat -s 37 12 "${USAGE}"
    exit 1
fi


if [ "${WHICH}" = "client" ] ; then
   	# comment the starting of the ypbind daemon
    	${SED_CMD} "/starting NIS services:/s/#//
		/starting NIS services:/s/^/#/
		\?-x.*/usr/lib/netsvc/yp/ypbind?,/fi/s/#//
		\?-x.*/usr/lib/netsvc/yp/ypbind?,/fi/s/^/#/
		\?/usr/bin/domainname?,/^[#]*fi/s/#//
		\?/usr/bin/domainname?,/^fi/s/^/#/" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
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

	# Remove all +::0:0::: entries from /etc/passwd.
	# There should be only one but this makes sure that all are removed.
 	/bin/grep "+::0:0:::" /etc/passwd > /dev/null 2>&1
	if [ $? = 0 ] ; then
		/bin/ed /etc/passwd << ENDEDIT > /dev/null 2>&1
g/+::0:0:::/d
w
q
ENDEDIT
	fi
	# Remove all +: entries from /etc/group.
	# There should be only one but this makes sure that all are removed.
 	/bin/grep "+:" /etc/group > /dev/null 2>&1
	if [ $? = 0 ] ; then
		/bin/ed /etc/group << ENDEDIT > /dev/null 2>&1
g/+:/d
w
q
ENDEDIT
	fi

	# Try the SRC stop just to make sure.
	stopsrc -s ypbind

	domainname ""

	/usr/bin/rm -f ${TMP_RC_NFS} 2>/dev/null
	exit $?
fi

if [ "${WHICH}" = "server" ] ; then
	# comment the line that says yp is being started
	# and the lines that starts the ypserv
	# and the lines that starts the yppasswdd daemon
	# and the lines that starts the ypupdated daemon
	# and the lines that starts the ypbind daemon
    	${SED_CMD} "/starting NIS services:/s/#//
		/starting NIS services:/s/^/#/
		\?-x.*/usr/lib/netsvc/yp/ypserv?,/fi/s/#//
		\?-x.*/usr/lib/netsvc/yp/ypserv?,/fi/s/^/#/
		\?DIR?,/fi/s/#//
		\?DIR?,/fi/s/^/#/
		\?-x.*/usr/lib/netsvc/yp/rpc.ypupdated?,/fi/s/#//
		\?-x.*/usr/lib/netsvc/yp/rpc.ypupdated?,/^fi/s/^/#/
		\?-x.*/usr/lib/netsvc/yp/rpc.yppasswd?,/fi/s/#//
		\?-x.*/usr/lib/netsvc/yp/rpc.yppasswd?,/^fi/s/^/#/
		\?-x.*/usr/lib/netsvc/yp/ypbind?,/fi/s/#//
		\?-x.*/usr/lib/netsvc/yp/ypbind?,/fi/s/^/#/
		\?/usr/bin/domainname?,/^[#]*fi/s/#//
		\?/usr/bin/domainname?,/^fi/s/^/#/" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null

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

 	/bin/grep "+::0:0:::" /etc/passwd > /dev/null 2>&1
	if [ $? = 0 ] ; then
		/bin/ed /etc/passwd << ENDEDIT > /dev/null 2>&1
g/+::0:0:::/d
w
q
ENDEDIT
	fi
 	/bin/grep "+:" /etc/group > /dev/null 2>&1
	if [ $? = 0 ] ; then
		/bin/ed /etc/group << ENDEDIT > /dev/null 2>&1
g/+:/d
w
q
ENDEDIT
	fi

	#
	# remove map files (in /var/yp/'domain') if they exist
	#
	DOMAIN=`domainname`
	if [ -n "$DOMAIN" -a -d /var/yp/$DOMAIN ]
	then
		/bin/rm -rf /var/yp/$DOMAIN
	fi

	stopsrc -g yp

	domainname ""

	/usr/bin/rm -f ${TMP_RC_NFS} 2>/dev/null
	exit $?
fi
