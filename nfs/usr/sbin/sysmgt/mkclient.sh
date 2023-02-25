#!/bin/bsh
#
# @(#)99	1.13  src/nfs/usr/sbin/sysmgt/mkclient.sh, cmdnfs, nfs411, GOLD410 3/2/94 17:37:12
#
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
RC_TCPIP="/etc/rc.tcpip"
TMP_RC_TCPIP="/tmp/tmp.rc.tcpip"
MKITAB="/usr/sbin/mkitab"

USAGE="
usage:  ${COMMAND_NAME} [-I|-B|-N]\n"

if [ -z "`/usr/bin/domainname`" ] ;then
	dspmsg cmdnfs.cat -s 37 43 "^G%s: The local host's domain name has not been set.  Please set it.\n" ${COMMAND_NAME}
	exit 1
fi 

set -- `getopt IBN $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 5 "${USAGE}" $COMMAND_NAME
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-I")			#WHEN should this  take place (IPL)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 5 "$USAGE" $COMMAND_NAME ; exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-N")			#WHEN should this  take place (NOW)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 5 "$USAGE" $COMMAND_NAME ; exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-B")			#WHEN should this  take place (BOTH)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 5 "$USAGE" $COMMAND_NAME ; exit 1 ; fi
	    WHEN="B"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 5 "$USAGE" $COMMAND_NAME 
	    exit 1
	    ;;
    esac
done

# check to make sure that there are not any extra parameters
shift		# get rid of the -- parameter
if [ -n "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 5 "${USAGE}" $COMMAND_NAME
    exit 1
fi

# Set the defaults for this command
WHEN=${WHEN:-"B"}


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
	# Uncomment the starting of the ypbind daemon
    	${SED_CMD} "/starting NIS services:/s/#//
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

	# Add the plus entries to the /etc/passwd and /etc/group file
	# We check both files to see if they have plus entries.  If the 
	# file has a plus entry we do not add the plus entry to that file. 
	# Adding extra plus entries wastes system resources when looking up
	# passwords and groups.  It also can cause a user to be in too
	# many groups causing the "user in too many groups" message to be 
	# displayed when logging in.
	# The format of the next few lines should not be changed.

	/bin/grep "+::0:0:::" /etc/passwd > /dev/null 2>&1
	if [ $? != 0 ] ; then
	cat >> /etc/passwd << END
+::0:0:::
END
	fi

	/bin/grep "+:" /etc/group > /dev/null 2>&1
	if [ $? != 0 ] ; then
	cat >> /etc/group << END
+:
END
	
	fi
fi

if [ "${WHEN}" = "N" -o "${WHEN}" = "B" ] ; then

# The format of the following lines that edit /etc/passwd and /etc/group
# should NOT be altered

	/bin/grep "+::0:0:::" /etc/passwd > /dev/null 2>&1
	if [ $? != 0 ] ; then
	cat >> /etc/passwd << END
+::0:0:::
END
	fi

	/bin/grep "+:" /etc/group > /dev/null 2>&1
	if [ $? != 0 ] ; then
	cat >> /etc/group << END
+:
END
	
	fi
    # Start the portmapper and refresh inetd
    startsrc -s portmap
    /bin/refresh -s inetd
    startsrc -s ypbind
fi
    
