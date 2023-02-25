#!/bin/bsh
# @(#)01        1.23  src/nfs/usr/sbin/sysmgt/mkmaster.sh, cmdnfs, nfs411, GOLD410 5/23/94 11:30:37
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

# Local functions.
# Translate commas into spaces
tr_comma_space() { /usr/bin/tr -s ',' '\040' ; }

# This awk script is used as a front end to the 'tr_comma_colon' function.
# The basic affect of this awk script is to strip leading and trailing
# spaces.
awk_strip_blanks() { \
	/bin/awk \
	    ' BEGIN { ORS=" " ; } ; \
	    {for (i=1; i <= NF; i++) \
		if (i == NF) \
		    { ORS="\n" ; print $i ; } \
		else \
		    { print $i ; } \
	    } ' ; }


# Variables used just within this script
YPINIT="/usr/sbin/ypinit"
COMMAND_NAME=`/usr/bin/basename $0`
RC_NFS="/etc/rc.nfs"
TMP_RC_NFS="/tmp/tmp.rc.nfs"
SED_CMD="/bin/sed"
RC_TCPIP="/etc/rc.tcpip"
TMP_RC_TCPIP="/tmp/tmp.rc.tcpip"
MKITAB="/usr/sbin/mkitab"
MAKEDBM="/usr/sbin/makedbm"
sflag="F"

USAGE="
usage:\n${COMMAND_NAME} [-s slave1[,slave2]...]
\t[-O|-o] [-E|-e] [-P|-p] [-U|-u] [-C|-c]
\t[-I|-B|-N]\n"

if [ -z "`/usr/bin/domainname`" ] ;then
        dspmsg cmdnfs.cat -s 37 43 "%s: The local host's domain name has not been set.  Please set it.\n" ${COMMAND_NAME}
        exit 1
fi

# Remove any spaces that are in the middle of parameters
#New stuff
PARMS=""
ORIG_PARMS="$@"
XLAT=""
argnum=1
export argnum
while [ $# -ge $argnum ]; do
	PARMNUM="\$${argnum}"
	PARM=`eval echo $PARMNUM`
	if [ -z "$PARM" ]
	then
		PARM="__NULL__"
		XLAT=1
	fi
	PARMS="$PARMS $PARM"
	argnum=`expr $argnum + 1`
done
if [ -n "$XLAT" ]
then
	set -- $PARMS
fi

# end of New stuff
PARMS=""
while [ -n "$1" ] ; do
    PARM=`echo "$1\c" | awk_strip_blanks | /usr/bin/tr -s ' ' ','`
    PARMS="${PARMS} ${PARM}"
    shift
done
set -- ${PARMS}

set -- `getopt s:OoEePpUuCcIBN $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME}
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-s")			#List of the slave servers
	    sflag="T"
	    if [ -n "${SLAVES}" ] ; then dspmsg cmdnfs.cat -s 37 6 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    if [ "__NULL__" = "$2" ] ; then SLAVES=`hostname`; else SLAVES=$2; fi
	    shift; shift
	    ;;
	"-o")			# Should we overwrite existing maps
	    if [ -n "${OVER_WRITE}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    OVER_WRITE="no"
	    shift
	    ;;
	"-O")			# Should we overwrite existing maps
	    if [ -n "${OVER_WRITE}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    OVER_WRITE="yes"
	    shift
	    ;;
	"-e")			# Exit on errors ?
	    if [ -n "${EXIT_ON_ERROR}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    EXIT_ON_ERROR="no"
	    shift
	    ;;
	"-E")			# Exit on errors ?
	    if [ -n "${EXIT_ON_ERROR}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    EXIT_ON_ERROR="yes"
	    shift
	    ;;
	"-p")			# Should yppasswdd be started?
	    if [ -n "${YPPASSWDD}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    YPPASSWDD="no"
	    shift
	    ;;
	"-P")			# Should yppasswdd be started?
	    if [ -n "${YPPASSWDD}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    YPPASSWDD="yes"
	    shift
	    ;;
	"-u")			# Should ypupdated be started?
	    if [ -n "${YPUPDATED}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    YPUPDATED="no"
	    shift
	    ;;
	"-U")			# Should ypupdated be started?
	    if [ -n "${YPUPDATED}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    YPUPDATED="yes"
	    shift
	    ;;
	"-c")			# Should ypbind be started?
	    if [ -n "${YPBIND}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    YPBIND="no"
	    shift
	    ;;
	"-C")			# Should ypbind be started?
	    if [ -n "${YPBIND}" ] ; then dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
	    YPBIND="yes"
	    shift
	    ;;
	"-I")			#WHEN should this  take place (IPL)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 6 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-N")			#WHEN should this  take place (NOW)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 6 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-B")			#WHEN should this  take place (BOTH)
	    if [ -n "${WHEN}" ] ; then dspmsg cmdnfs.cat -s 37 6 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    WHEN="B"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 6 "$USAGE" ${COMMAND_NAME} 
	    exit 1
	    ;;
    esac
done

# check to make sure that there are not any extra parameters
shift		# get rid of the -- parameter
if [ -n "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 6 "${USAGE}" ${COMMAND_NAME}
    exit 1
fi

# Add on this host as one of the slave servers
if [ -z "${SLAVES}" ] ; then
    SLAVES=`hostname`
else
    SLAVES=`hostname`,${SLAVES}
fi

# change the list of comma separated hostnames into a list of 
# space separated hostnames
SLAVES=`echo $SLAVES | tr_comma_space`

# Set the defaults for this command
OVER_WRITE=${OVER_WRITE:-"no"}
EXIT_ON_ERROR=${EXIT_ON_ERROR:-"yes"}
YPPASSWDD=${YPPASSWDD:-"no"}
YPUPDATED=${YPUPDATED:-"no"}
YPBIND=${YPBIND:-"yes"}
WHEN=${WHEN:-"B"}

# Build parameter list for ypinit
# Make sure we invoke it with the '-q' flag for quiet (no prompting)
PARMS="-q -m"
if [ "${OVER_WRITE}" = "yes" ] ; then
    PARMS="${PARMS} -o"
fi
if [ "${EXIT_ON_ERROR}" = "no" ] ; then
    PARMS="${PARMS} -n"
fi

# check if -s option is specified, if not and if yp servers (master and slaves)
# are existed, be sure to add those before call ypinit, otherwise yp servers
# will be deleted
if [ "${sflag}" = "F" ] ; then
    # get master server's hostname
    master=`ypwhich | awk '{if($1 == "ypwhich:") print $2; else print $1}' `
    ${MAKEDBM} -u /var/yp/`domainname`/ypservers > /tmp/mkmaster.tmp 2>/dev/null
    slaves=`/bin/awk '
	BEGIN { i = 0 }
	/YP_LAST_MODIFIED.*/    {break}
	/YP_MASTER_NAME.*/      { master = $2 ; break }
	{
	    slave[i] = $1
	    i = i + 1
	}
	END {
	    for (j = 0; j < i; j++) {
		if (slave[j] != master) {
			printf "\t%s\n", slave[j]
		}
	    }
	}' < /tmp/mkmaster.tmp `

    rm -f /tmp/mkmaster.tmp

    if [ -z "${SLAVES}" -a -n "${master}" ]; then
	SLAVES="${master}"
    fi
    SLAVES="${SLAVES} ${slaves}"
    
fi

# Call ypinit with the correct parameters
${YPINIT} ${PARMS} ${SLAVES}
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
		\?-x.*/usr/lib/netsvc/yp/ypserv?,/fi/s/#//" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
		exit 1
	fi
	# copy the file back to /etc
	/bin/cp ${TMP_RC_NFS} ${RC_NFS} > /dev/null 2>&1
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	fi

    if [ "$YPPASSWDD" = "yes" ] ; then
	# Uncomment the starting of the yppasswdd daemon
	${SED_CMD} "\?DIR?,/fi/s/#//" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
		exit 1
	fi
	# copy the file back to /etc
	/bin/cp ${TMP_RC_NFS} ${RC_NFS} > /dev/null 2>&1
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	fi
    else
	# comment the starting of the yppasswdd daemon
	${SED_CMD} "\?DIR?,/fi/s/#//
		\?DIR?,/^fi/s/^/#/"	${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
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
    if [ "$YPUPDATED" = "yes" ] ; then
	# Uncomment the starting of the ypupdated daemon
	${SED_CMD} "\?-x.*/usr/lib/netsvc/yp/rpc.ypupdated?,/fi/s/#//" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
		exit 1
	fi
	# copy the file back to /etc
	/bin/cp ${TMP_RC_NFS} ${RC_NFS} > /dev/null 2>&1
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	fi
    else
	# comment the starting of the ypupdated daemon
	${SED_CMD} "\?-x.*/usr/lib/netsvc/yp/rpc.ypupdated?,/fi/s/#//
		\?-x.*/usr/lib/netsvc/yp/rpc.ypupdated?,/fi/s/^/#/" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
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
    if [ "${YPBIND}" = "yes" ] ; then
	# Uncomment the starting of the ypbind daemon
	${SED_CMD} "\?-x.*/usr/lib/netsvc/yp/ypbind?,/fi/s/#//" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
		exit 1
	fi
	# copy the file back to /etc
	/bin/cp ${TMP_RC_NFS} ${RC_NFS} > /dev/null 2>&1
	if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 14 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	fi
    else
	# comment the starting of the ypbind daemon
	${SED_CMD} "\?-x.*/usr/lib/netsvc/yp/ypbind?,/fi/s/#//
		\?-x.*/usr/lib/netsvc/yp/ypbind?,/fi/s/^/#/" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
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
fi

if [ "${WHEN}" = "N" -o "${WHEN}" = "B" ] ; then
    # Start the portmapper and refresh inetd
    startsrc -s portmap
    /bin/refresh -s inetd
    if [ "${COMMAND}" = "chmaster" ] ; then
	stopsrc -s ypserv
    fi
    startsrc -s ypserv
    if [ "$YPPASSWDD" = "yes" ] ; then
	if [ "${COMMAND}" = "chmaster" ] ; then
	    stopsrc -s yppasswdd
	fi
	startsrc -s yppasswdd
    else
	stopsrc -s yppasswdd
    fi
    if [ "$YPUPDATED" = "yes" ] ; then
	if [ "${COMMAND}" = "chmaster" ] ; then
	    stopsrc -s ypupdated
	fi
	startsrc -s ypupdated
    else
	stopsrc -s ypupdated
    fi
    if [ "${YPBIND}" = "yes" ] ; then
	if [ "${COMMAND}" = "chmaster" ] ; then
	    stopsrc -s ypbind
	fi
	startsrc -s ypbind
    else
	stopsrc -s ypbind
    fi
fi
    
exit 0
