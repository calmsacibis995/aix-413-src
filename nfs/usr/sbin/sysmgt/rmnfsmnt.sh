#!/bin/bsh
#
# @(#)10	1.12  src/nfs/usr/sbin/sysmgt/rmnfsmnt.sh, cmdnfs, nfs41J, 9525E_all 6/21/95 09:46:44
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
UNMOUNT_CMD="/usr/sbin/umount"
COMMAND_NAME=`basename $0`

RMFS_CMD="/usr/sbin/rmfs"
LSFS_CMD="/usr/sbin/lsfs -c"
MOUNT_CMD="/usr/sbin/mount"

USAGE="
usage:  ${COMMAND_NAME}	-f path_name [{-I|-B|-N}]\n"

# Parse the command line arguments to find out what directory
# is to be removed from mounting.

set -- `getopt f:IBN $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 11 "${USAGE}"
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-f")			#path name to mount point
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 11 "$USAGE"; exit 1 ; fi
	    if [ -n "${PATH_NAME}" ] ; then dspmsg cmdnfs.cat -s 37 11 "$USAGE"; exit 1 ; fi
	    PATH_NAME=$2
	    shift; shift
	    ;;
	"-I")			#WHEN should this mount take place (IPL)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 11 "$USAGE"; exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-N")			#WHEN should this mount take place (NOW)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 11 "$USAGE"; exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-B")			#WHEN should this mount take place (BOTH)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 11 "$USAGE"; exit 1 ; fi
	    WHEN="B"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 11 "$USAGE"
	    exit 1
	    ;;
    esac
done

# check to make sure that there are not any extra parameters
shift		# get rid of the -- parameter
if [ -n "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 11 "${USAGE}"
    exit 1
fi

# All done with parsing the parameters. Now move on to removing
# the mount.

# We have to do a little sanity checking along the way to make
# sure that the user specified all the right parameters in the right order.

# Was a local path name specified?
if [ -z "${PATH_NAME}" ] ; then dspmsg cmdnfs.cat -s 37 11 "${USAGE}"; exit 1 ; fi

# Set the default for WHEN the removal is to take place.
# Set WHEN only if it has not been specified.
WHEN=${WHEN:-"B"}


# Done with the parsing and sanity checking.  Move onto actually
# removing the mount entry in /etc/filesystems (if we have to) and 
# executing the unmount command (if we have to)

# Verify that this is an NFS filesystem before processing
if [ ${WHEN} = "I" -o ${WHEN} = "B" ] ; then
	${LSFS_CMD} ${PATH_NAME} 2>&1  | /usr/bin/awk -F':' '{print $3}' | /usr/bin/grep "nfs" > /dev/null 2>&1

	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 42 "%s: Error. %s is not an NFS file system.\n" ${COMMAND_NAME} ${PATH_NAME}
		exit 1
	fi
fi

# Check to see if the directory is to be exported now.
if [ ${WHEN} = "N" -o ${WHEN} = "B" ] ; then
        # execute the mount command with the correct parameters
	# Determine if the file system is mounted currently
	${MOUNT_CMD} | /usr/bin/awk '{print $3}' | /usr/bin/grep "${PATH_NAME}" > /dev/null 2>&1

	if [ $? = 0 ] ; then
	    ${UNMOUNT_CMD} ${PATH_NAME}
	    if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 34 "rmnfsmnt: Error in unmounting ${PATH_NAME}\n" ${PATH_NAME}
		exit 1
	    fi
	else 
		if [ ${WHEN} = "N" ]; then
			dspmsg cmdnfs.cat -s 37 35 "rmnfsmnt: ${PATH_NAME} not currently mounted\n" ${PATH_NAME}
			exit 1
		else
			dspmsg cmdnfs.cat -s 37 35 "rmnfsmnt: ${PATH_NAME} not currently mounted\n" ${PATH_NAME}
		fi
	fi
fi

if [ ${WHEN} = "I" -o ${WHEN} = "B" ] ; then
	# We have to modify /etc/filesystems

	${RMFS_CMD} ${PATH_NAME}		# Remove the entry

	# Check to make sure our new file was created properly
	if [ $? != 0 ] ; then
	    exit 1
	fi 
fi

