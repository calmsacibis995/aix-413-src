#!/bin/bsh
# @(#)97	1.6  src/nfs/usr/sbin/sysmgt/chnfs.sh, cmdnfs, nfs411, GOLD410 3/7/91 17:59:44
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

COMMAND_NAME=`/bin/basename $0`

USAGE="
Usage:  ${COMMAND_NAME}	-b num_biods -n num_nfsds  [{-I|-B|-N}]\n"

ERRORMSG="
ERROR: parameter values can not be less than or equal to 0\n"

# Parse the command line arguments to find out what directory
# is to be removed from mounting.

set -- `/bin/getopt b:n:IBN $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 1 "${USAGE}"
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-b")			# Number of biod daemons to be started.
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 1 "$USAGE" ; exit 1 ; fi	    
	    if [ $2 -le 0 ] ; then dspmsg cmdnfs.cat -s 37 36 "$ERRORMSG" ; exit 1 ; fi
	    if [ -n "${NUM_BIODS}" ] ; then dspmsg cmdnfs.cat -s 37 1 "$USAGE" ; exit 1 ; fi
	    NUM_BIODS=$2
	    shift; shift
	    ;;
	"-n")			# Number of nfsd daemons to be started
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 1 "$USAGE" ; exit 1 ; fi	    
	    if [ $2 -le 0 ] ; then dspmsg cmdnfs.cat -s 37 36 "$ERRORMSG" ; exit 1 ; fi
	    if [ -n "${NUM_NFSDS}" ] ; then dspmsg cmdnfs.cat -s 37 1 "$USAGE" ; exit 1 ; fi
	    NUM_NFSDS=$2
	    shift; shift
	    ;;
	"-I")			#WHEN should this mount take place (IPL)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 1 "$USAGE" ; exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-N")			#WHEN should this mount take place (NOW)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 1 "$USAGE" ; exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-B")			#WHEN should this mount take place (BOTH)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 1 "$USAGE" ; exit 1 ; fi
	    WHEN="B"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 1 "$USAGE" 
	    exit 1
	    ;;
    esac
done

# check to make sure that there are not any extra parameters
shift		# get rid of the -- parameter
if [ -n "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 1 "${USAGE}"
    exit 1
fi

# Check the usage of the command.
if [ -z "${NUM_BIODS}" -a -z "${NUM_NFSDS}" ] ; then
	dspmsg cmdnfs.cat -s 37 1 "${USAGE}"
	exit 1
fi

# Set the default for WHEN the 'operation' is to take place
# Set WHEN only if it has not been specified.
WHEN=${WHEN:-"B"}

# Change the number of daemons on the system is the user requested it.

if [ -n "${NUM_BIODS}" ] ; then
    NUM_CHECK=`{ echo "${NUM_BIODS}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" ] ; then dspmsg cmdnfs.cat -s 37 1 "$USAGE" ; exit 1 ; fi

    if [ "${WHEN}" = "B" -o "${WHEN}" = "N" ] ; then
	/bin/stopsrc -s biod
    fi

    if [ "${WHEN}" = "N" ] ; then
	# Find out how many we have now
	SAVED_NUM_BIODS=`/usr/bin/odmget -q subsysname=biod SRCsubsys | /bin/awk '/cmdargs/ { print $3 }' | /usr/bin/tr -s '\042' '\040' `

    fi

    # Install the objects for biod's
    /bin/chssys  -s biod -a "${NUM_BIODS}"

    if [ "${WHEN}" = "B" -o "${WHEN}" = "N" ] ; then
	/bin/startsrc -s biod
    fi

    if [ "${WHEN}" = "N" ] ; then
	/bin/chssys -s biod -a ${SAVED_NUM_BIODS}
    fi

fi

if [ -n "${NUM_NFSDS}" ] ; then
    NUM_CHECK=`{ echo "${NUM_NFSDS}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" ] ; then dspmsg cmdnfs.cat -s 37 1 "$USAGE" ; exit 1 ; fi

    if [ "${WHEN}" = "B" -o "${WHEN}" = "N" ] ; then
	/bin/stopsrc -s nfsd
    fi

    if [ "${WHEN}" = "N" ] ; then
	# Find out how many we have now
	SAVED_NUM_NFSDS=`/usr/bin/odmget -q subsysname=nfsd SRCsubsys | /bin/awk '/cmdargs/ { print $3 }' | /usr/bin/tr -s '\042' '\040'`

    fi

    # Install the objects for nfsd's
    
    /bin/chssys  -s nfsd -a "${NUM_NFSDS}"

    if [ "${WHEN}" = "B" -o "${WHEN}" = "N" ] ; then
	/bin/startsrc -s nfsd
    fi

    if [ "${WHEN}" = "N" ] ; then
	/bin/chssys -s nfsd -a ${SAVED_NUM_NFSDS}
    fi

fi

exit $?
