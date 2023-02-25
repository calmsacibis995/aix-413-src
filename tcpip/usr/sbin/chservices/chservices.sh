#! /bin/bsh
# @(#)45	1.4  src/tcpip/usr/sbin/chservices/chservices.sh, tcpinet, tcpip41J, 9518A_all 4/25/95 10:35:49
# 
# COMPONENT_NAME: TCPIP chservices
# 
# FUNCTIONS: 
#
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
####################################################################
# chservices - 
#	adds or deletes TCP/IP services in /etc/services or whatever
#	configuration file is given.
####################################################################
set -f			# this is so that the case statement will work

COMMAND_NAME=`/usr/bin/basename $0`
SERVICES="/etc/services"
TMP_SERVICES="/tmp/tmp.services$$"
GREP_CMD="/usr/bin/egrep"
ECHO_CMD="/usr/bin/echo"
SED_CMD="/usr/bin/sed"
CP_CMD="/usr/bin/cp"
RM_CMD="/usr/bin/rm"
CAT_CMD="/usr/bin/cat"
ACTION=
SERVICE_NAME=
PROTOCOL=
PORT=
NEW_SERVICE_NAME=
NEW_PROTOCOL=
NEW_PORT=
ALIASES=

USAGE="
usage: ${COMMAND_NAME} [-a|-d|-c] -v service_name -p protocol -n port [-V new_service_name] [-P new_protocol] [-N new_port] [-u \"aliases [aliases]...\"]\n"

# Parse the command line arguments to find out what service
# is to be uncommented, commented, or added and if a configuration
# file is specified.
# The -u is followed by a list of alias names in quotes
# and it must be the last flag used on the command line.

set -- `/bin/getopt adcv:p:n:V:P:N:u: $*  2>/dev/null` 
if [ $? != 0 ] ; then          # Test for syntax error
    dspmsg chservices.cat -s 1 1 "${USAGE}" $COMMAND_NAME
    exit 1
fi

while [ $1 != -- ]
do
    case $1 in
	-a|-d|-c)		# Action is to add or delete the service
	    if [ -n "${ACTION}" ] ; then dspmsg chservices.cat -s 1 1 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    ACTION=`expr "$1" : "-\(.\)"`
	    shift		# Shift past the flag
	    ;;
	-v)			# Then we need to get the SERVICE_NAME
	    SERVICE_NAME=$2	# to add to the services line
	    shift; shift   	# Shift past the flag and SERVICE_NAME
	    ;;
	-p)			# Then we need to get the PROTOCOL
	    PROTOCOL=$2		# to add to the services line
	    shift; shift   	# Shift past the flag and PROTOCOL
	    ;;
	-n)			# Then we need to get the PORT
	    PORT=$2		# to add to the services line
	    shift; shift   	# Shift past the flag and PORT
	    ;;
	-V)			# Then we need to get the NEW_SERVICE_NAME
	    NEW_SERVICE_NAME=$2	# to add to the services line
	    shift; shift   	# Shift past the flag and NEW_SERVICE_NAME
	    ;;
	-P)			# Then we need to get the NEW_PROTOCOL
	    NEW_PROTOCOL=$2	# to add to the services line
	    shift; shift   	# Shift past the flag and NEW_PROTOCOL
	    ;;
	-N)			# Then we need to get the NEW_PORT
	    NEW_PORT=$2		# to add to the services line
	    shift; shift   	# Shift past the flag and NEW_PORT
	    ;;
	-u)			# Then we need to get the list of ALIASES
	    ALIASES=$2		# to add to the end of the services line
	    shift; shift   	# Shift past the flag and ALIASES
	    ;;
	*)
	    dspmsg chservices.cat -s 1 1 "$USAGE" ${COMMAND_NAME}
	    exit 1
	    ;;
    esac
done
shift                   	# Shift past the "--" from getopt
 
if [ -z "${SERVICE_NAME}" ] ; then          # Test for syntax error
    dspmsg chservices.cat -s 1 1 "${USAGE}" $COMMAND_NAME
    exit 1
fi

if [ -z "${PROTOCOL}" ] ; then          # Test for syntax error
    dspmsg chservices.cat -s 1 1 "${USAGE}" $COMMAND_NAME
    exit 1
fi

if [ -z "${PORT}" ] ; then          # Test for syntax error
    if [ -z "${NEW_PORT}" ] ; then          # Test for syntax error
        dspmsg chservices.cat -s 1 1 "${USAGE}" $COMMAND_NAME
        exit 1
    else
	if [ ${ACTION} -eq "c" ] ; then
		PORT=${NEW_PORT}
	fi
    fi
fi

i=$#
while [ $i -ne 0 ]
do
	ALIASES=`${ECHO_CMD} ${ALIASES} $1`
	shift
        i=`expr $i - 1`
done

# Set the default for ACTION to add.
# Set ACTION only if it has not been specified.
ACTION=${ACTION:-"a"}

case "${ACTION}" in
    "a")
	${GREP_CMD} "${SERVICE_NAME}.*.${PORT}/${PROTOCOL}" ${SERVICES} 2>/dev/null
	if [ $? -eq 1 ] ; then		# Then service_name is NOT in SERVICES
					# add it to the end of SERVICES
		${ECHO_CMD} "${SERVICE_NAME}\t\t${PORT}/${PROTOCOL}\t${ALIASES}" >> ${SERVICES}
	else				# Then services_name is in SERVICES
					# uncomment line.
		${SED_CMD} "\?${SERVICE_NAME}.*.${PORT}/${PORTOCOL}?s/^#//
		\?${SERVICE_NAME}.*.${PORT}/${PORTOCOL}?s/$/ ${ALIASES}/" ${SERVICES} > ${TMP_SERVICES} 2>/dev/null
		if [ $? != 0 ] ; then
			dspmsg chservices.cat -s 1 2 "${COMMAND_NAME}: error in updating ${SERVICES}\n" ${COMMAND_NAME} ${SERVICES}
			exit 1
		fi
		#copy the file back to /etc
		${CP_CMD} ${TMP_SERVICES} ${SERVICES} > /dev/null 2>&1
		if [ $? != 0 ] ; then
			dspmsg chservices.cat -s 1 2 "${COMMAND_NAME}: error in updating ${SERVICES}\n" ${COMMAND_NAME} ${SERVICES}
			${RM_CMD} -f ${TMP_SERVICES} > /dev/null 2>&1
			exit 1	
		fi
	fi
	;;
    "d")
	${GREP_CMD} "${SERVICE_NAME}.*.${PORT}/${PROTOCOL}" ${SERVICES} 2>/dev/null
	if [ $? -eq 1 ] ; then		# Then service_name is NOT in SERVICES
					# add it to the end of SERVICES
		${ECHO_CMD} "#${SERVICE_NAME}\t\t${PORT}/${PROTOCOL}\t${ALIASES}" >> ${SERVICES}
	else				# Then services_name is in SERVICES
					# comment the line.
		${SED_CMD} "\?^${SERVICE_NAME}.*.${PORT}/${PROTOCOL}?s/^/#/" ${SERVICES} > ${TMP_SERVICES} 2>/dev/null
		if [ $? != 0 ] ; then
			dspmsg chservices.cat -s 1 2 "${COMMAND_NAME}: error in updating ${SERVICES}\n" ${COMMAND_NAME} ${SERVICES}
			exit 1
		fi
		#copy the file back to /etc
		${CP_CMD} ${TMP_SERVICES} ${SERVICES} > /dev/null 2>&1
		if [ $? != 0 ] ; then
			dspmsg chservices.cat -s 1 2 "${COMMAND_NAME}: error in updating ${SERVICES}\n" ${COMMAND_NAME} ${SERVICES}
			${RM_CMD} -f ${TMP_SERVICES} > /dev/null 2>&1
			exit 1	
		fi
	fi
	;;
    "c")
	${GREP_CMD} "^${SERVICE_NAME}" ${SERVICES} 2>/dev/null
	if [ $? -eq 1 ] ; then		# Then service_name is NOT in SERVICES
					# add it to the end of SERVICES
		dspmsg chservices.cat -s 1 3 "${COMMAND_NAME}: ${SERVICE_NAME} not in ${SERVICES}\n" ${COMMAND_NAME} ${SERVICE_NAME} ${SERVICES}
		exit 1
	else				# Then services_name is in SERVICES
					# so we need to delete the old line
					# and add the new one.
		${SED_CMD} "\?^${SERVICE_NAME}.*.${PORT}\/${PROTOCOL}?d" ${SERVICES} > ${TMP_SERVICES} 2>/dev/null
		${ECHO_CMD} "${NEW_SERVICE_NAME:-${SERVICE_NAME}}\t\t${NEW_PORT:-${PORT}}/${NEW_PROTOCOL:-${PROTOCOL}}\t${ALIASES}" >> ${TMP_SERVICES}
		if [ $? != 0 ] ; then
			dspmsg chservices.cat -s 1 2 "${COMMAND_NAME}: error in updating ${SERVICES}\n" ${COMMAND_NAME} ${SERVICES}
			exit 1
		fi
		#copy the file back to /etc
		${CP_CMD} ${TMP_SERVICES} ${SERVICES} > /dev/null 2>&1
		if [ $? != 0 ] ; then
			dspmsg chservices.cat -s 1 2 "${COMMAND_NAME}: error in updating ${SERVICES}\n" ${COMMAND_NAME} ${SERVICES}
			${RM_CMD} -f ${TMP_SERVICES} > /dev/null 2>&1
			exit 1	
		fi
	fi
	;;
esac

if [ $? != 0 ] ; then
	dspmsg chservices.cat -s 1 2 "${COMMAND_NAME}: error in updating ${SERVICES}\n" ${COMMAND_NAME} ${SERVICES}
	exit 1
fi

# Remove the temporary file
${RM_CMD} -f ${TMP_SERVICES} > /dev/null 2>&1
