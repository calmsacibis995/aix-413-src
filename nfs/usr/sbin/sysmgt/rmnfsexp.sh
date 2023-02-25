#!/bin/bsh
#
# @(#)09	1.6  src/nfs/usr/sbin/sysmgt/rmnfsexp.sh, cmdnfs, nfs411, GOLD410 2/3/94 16:02:35
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

# Variables used just within this script
EXPORTS_FILE=/etc/exports
XTAB_FILE=/etc/xtab
EXPORTFS_CMD=/usr/sbin/exportfs
COMMAND_NAME=`basename $0`

USAGE="usage: ${COMMAND_NAME} -d directory [-f exports_file] [-B|-I|-N]\n"

# Parse the command line arguments to find out what directory
# is to be exported and how it is to be exported.

while [ -n "$1" ]
do
    case "$1" in
	"-d")			#directory to export
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 10 "$USAGE"; exit 1 ; fi
	    if [ -n "${DIRECTORY}" ] ; then dspmsg cmdnfs.cat -s 37 10 "$USAGE"; exit 1 ; fi
	    DIRECTORY=$2
	    shift; shift
	    ;;
	"-f")			#exports file to use
	    EXPORTS_FILE=$2
	    shift; shift
	    ;;
	"-I")
	    if [ -n "${WHEN}" ] ; then
		dspmsg cmdnfs.cat -s 37 10 "${USAGE}"
		exit 1
	    fi
	    WHEN="I"
	    shift
	    ;;
	"-B")
	    if [ -n "${WHEN}" ] ; then
		dspmsg cmdnfs.cat -s 37 10 "${USAGE}"
		exit 1
	    fi
	    WHEN="B"
	    shift
	    ;;
	"-N")
	    if [ -n "${WHEN}" ] ; then
		dspmsg cmdnfs.cat -s 37 10 "${USAGE}"
		exit 1
	    fi
	    WHEN="N"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 10 "$USAGE"
	    exit 1
	    ;;
    esac
done
if [ -z ${EXPORTS_FILE} ] ; then
	dspmsg cmdnfs.cat -s 37 10 "${USAGE}"
	exit 1
fi	

# All done with parsing the parameters.
# Next comes the sanity checking.

# Check to make sure that a directory was specified
if [ -z "$DIRECTORY" ] ; then dspmsg cmdnfs.cat -s 37 10 "$USAGE"; exit 1 ; fi

# If the user did not specify when the removal was to take place,
# take the default.  Remove now and for ever more.
if [ -z "${WHEN}" ] ; then
    WHEN="B"
fi


# Check to see that the user is trying to remove a directory
# from the NFS exports list that is actually on that list.

grep "${DIRECTORY}" ${XTAB_FILE} > /dev/null  2>&1
if [ $? != 0 ] ; then
    grep "${DIRECTORY}" ${EXPORTS_FILE} > /dev/null 2>&1
    if [ $? != 0 ] ; then
	dspmsg cmdnfs.cat -s 37 31 "rmnfsexp: ${DIRECTORY} is not currently exported and is not in ${EXPORTS_FILE}\n" ${DIRECTORY} ${EXPORTS_FILE}
    fi
fi

TEMP_AWK=/tmp/_awk.script.$$		# for temp awk script
TEMP_EXPORTS=/tmp/_exports.$$		# where do we put the exports file
					# while we are working on it?

# We have to decide when the removal is to take affect.
# If is to be done on IPL or now and on IPL, then delete the
# exports line to the exports file.

case "${WHEN}" in
    "N")	# execute exportfs with the correct parameters
	grep "${DIRECTORY}" ${XTAB_FILE} > /dev/null  2>&1
	if [ $? != 0 ] ; then
	    dspmsg cmdnfs.cat -s 37 32 "${COMMAND_NAME}: ${DIRECTORY} not currently exported.\n" ${COMMAND_NAME} ${DIRECTORY}
	else
	    ${EXPORTFS_CMD} -f ${EXPORTS_FILE} -v -u ${DIRECTORY}
	fi
	;;

    "I"|"B")	# change the exports file so that the export will last.
	# If the exports file exists and is readable, we will scan the file.
	# If there exists and export for the directory we are about to export,
	# remove it from the file and add the specification we have ready.

	if [ -r ${EXPORTS_FILE} ] ; then
	    # Build awk script to copy all entries except the one that matches
	    # with the directory that is to be removed from the exports file.
            echo " { if (\"${DIRECTORY}\" != \$1) print \$0 } " > ${TEMP_AWK}
            { awk -f ${TEMP_AWK} < ${EXPORTS_FILE} > ${TEMP_EXPORTS} ; }

            # Check to make sure that the awk script ran without errors
            if [ $? != 0 ] ; then 
		dspmsg cmdnfs.cat -s 37 33 "Unable to change ${EXPORTS_FILE}. Aborting action\n" ${EXPORTS_FILE}
		exit 1
	    fi

            # Copy the new version of the exports file back to the original
	    cp ${TEMP_EXPORTS} ${EXPORTS_FILE}
	    if [ $? != 0 ] ; then 
		dspmsg cmdnfs.cat -s 37 33 "Unable to change ${EXPORTS_FILE}. Aborting action\n" ${EXPORTS_FILE}
		exit 1
	    fi

	    # Remove the temporary files used
	    rm ${TEMP_AWK} ${TEMP_EXPORTS}

	fi

	# Check to see if the directory is to be exported now.
	if [ "${WHEN}" = "B" ] ; then
	    grep "${DIRECTORY}" ${XTAB_FILE} > /dev/null  2>&1
	    if [ $? = 0 ] ; then
		${EXPORTFS_CMD} -f ${EXPORTS_FILE} -v -u ${DIRECTORY}
	    fi
	fi
	;;
esac


