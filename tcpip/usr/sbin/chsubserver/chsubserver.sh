#! /bin/bsh
# @(#)48	1.2  src/tcpip/usr/sbin/chsubserver/chsubserver.sh, tcpinet, tcpip411, GOLD410 4/11/94 10:52:18
# 
# COMPONENT_NAME: TCPIP chsubserver
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
# chsubserver - 
#	starts TCP/IP services in inetd.conf or whatever similar
#	configuration file is given.
####################################################################
set -f			# this is so that the case statement will work

COMMAND_NAME=`/usr/bin/basename $0`
SED_CMD="/usr/bin/sed"
INETD_CONF="/etc/inetd.conf"
TMP_INETD_CONF="/tmp/tmp.inetd.conf$$"
RM_CMD="/usr/bin/rm"
CP_CMD="/usr/bin/cp"
CAT_CMD="/usr/bin/cat"
ECHO_CMD="/usr/bin/echo"
PS_CMD="/usr/bin/ps -e"
AWK_CMD="/usr/bin/awk"
GREP_CMD="/usr/bin/egrep"
XARGS_CMD="/usr/bin/xargs"
KILL_CMD="/usr/bin/kill -HUP"
SERVICE=
PROTOCOL=
SOCKET=
WAIT=
USER=
PROG=
ARGS=
REFRESH=
ACTION=
NEW_SERVICE=
NEW_PROTOCOL=
NEW_SOCKET=
NEW_WAIT=
NEW_USER=
NEW_PROG=
NEW_ARGS=

USAGE="
usage: ${COMMAND_NAME} [-a|-d|-c] -v service_name -p protocol [-t socket_type ] [-w ] wait_or_nowait ] [-u user ] [-g program ] [-V new_service_name ] [-P new_protocol ] [-T new_socket_type ] [-W new_wait_or_nowait ] [-U new_user ] [-G new_program ] [ -r server ] [-C config_file ] [ program_and_args ] ]\n"

# Parse the command line arguments to find out what service
# is to be uncommented and if a configuration file is specified.
# The -r server tells us to send a SIGHUP to the server.
set -- `/bin/getopt adcv:p:t:w:u:g:V:P:T:W:U:G:r:C: $*  2>/dev/null` 
if [ $? != 0 ] ; then          # Test for syntax error
    dspmsg chsubserver.cat -s 1 1 "${USAGE}" $COMMAND_NAME
    exit 1
fi

while [ $1 != -- ]
do
    case $1 in
	-a|-d|-c)		# Action is to add, delete or change
	    if [ -n "${ACTION}" ] ; then dspmsg chsubserver.cat -s 1 1 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
	    ACTION=`expr "$1" : "-\(.\)"`
	    shift		# Shift past the flag
	    ;;
	-v)			# Then we need to get the SERVICE
	    SERVICE=$2		# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and SERVICE
	    ;;
	-p)			# Then we need to get the PROTOCOL
	    PROTOCOL=$2		# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and PROTOCOL
	    ;;
	-t)			# Then we need to get the SOCKET
	    SOCKET=$2		# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and SOCKET
	    ;;
	-w)			# Then we need to get the WAIT
	    WAIT=$2		# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and WAIT
	    ;;
	-u)			# Then we need to get the USER
	    USER=$2		# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and USER
	    ;;
	-g)			# Then we need to get the PROG
	    PROG=$2		# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and PROG
	    ;;
	-V)			# Then we need to get the NEW_SERVICE
	    NEW_SERVICE=$2	# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and NEW_SERVICE
	    ;;
	-P)			# Then we need to get the NEW_PROTOCOL
	    NEW_PROTOCOL=$2	# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and NEW_PROTOCOL
	    ;;
	-T)			# Then we need to get the NEW_SOCKET
	    NEW_SOCKET=$2	# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and NEW_SOCKET
	    ;;
	-W)			# Then we need to get the NEW_WAIT
	    NEW_WAIT=$2		# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and NEW_WAIT
	    ;;
	-U)			# Then we need to get the NEW_USER
	    NEW_USER=$2		# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and NEW_USER
	    ;;
	-G)			# Then we need to get the NEW_PROG
	    NEW_PROG=$2		# to add to the inetd.conf line
	    shift; shift   	# Shift past the flag and NEW_PROG
	    ;;
	-r)			# Then we need to tell the next parameter
	    REFRESH=$2		# of the change to the configuration file.
	    shift; shift   	# Shift past the flag and server
	    ;;
	-C)			# Then we need to tell the next parameter
	    INETD_CONF=$2	# of the change to the configuration file.
	    shift; shift   	# Shift past the flag and server
	    ;;
	*)
	    dspmsg chsubserver.cat -s 1 1 "$USAGE" ${COMMAND_NAME}
	    exit 1
	    ;;
    esac
done
shift                   	# Shift past the "--" from getopt
 
if [ -z "${SERVICE}" ] ; then          # Test for syntax error
    dspmsg chsubserver.cat -s 1 1 "${USAGE}" $COMMAND_NAME
    exit 1
fi
 
if [ -z "${PROTOCOL}" ] ; then          # Test for syntax error
    dspmsg chsubserver.cat -s 1 1 "${USAGE}" $COMMAND_NAME
    exit 1
fi

i=$#
while [ $i -ne 0 ]
do
        ARGS=`${ECHO_CMD} ${ARGS} $1`
        shift
        i=`expr $i - 1`
done


# Set the default for ACTION to add.
# Set ACTION only if it has not been specified.
ACTION=${ACTION:-"a"}

case "${ACTION}" in
    "a")
	${GREP_CMD} "${SERVICE}.*.${PROTOCOL}.*.${PROG}.*.${ARGS}" ${INETD_CONF} >/dev/null 2>&1
	if [ $? -eq 1 ] ; then		# Then service_name is NOT in inetd.conf
					# add it to the end of inetd.conf
		${CP_CMD} ${INETD_CONF} ${TMP_INETD_CONF} > /dev/null 2>&1
		${ECHO_CMD} "${SERVICE}\t${SOCKET}\t${PROTOCOL}\t${WAIT}\t${USER}\t${PROG} ${ARGS}" >> ${TMP_INETD_CONF}
	else				# Then services_name is in inetd.conf
					# Uncomment the service_name line
					# in /etc/inetd.conf
		${SED_CMD} "\?${SERVICE}.*.${PROTOCOL}.*.${PROG}?s/#//
		\?${SERVICE}.*.${PROTOCOL}.*.${PROG}?s/#//" ${INETD_CONF} > ${TMP_INETD_CONF} 2>/dev/null
	fi
	;;
    "d")
	${GREP_CMD} "${SERVICE}.*.${PROTOCOL}.*.${PROG}.*.${ARGS}" ${INETD_CONF} >/dev/null 2>&1
	if [ $? -eq 0 ] ; then		# Then services_name is in inetd.conf
					# Uncomment the service_name line
					# in /etc/inetd.conf
		${SED_CMD} "\?${SERVICE}.*.${PROTOCOL}.*.${PROG}?s/#//
		\?${SERVICE}.*.${PROTOCOL}.*.${PROG}?s/^/#/" ${INETD_CONF} > ${TMP_INETD_CONF} 2>/dev/null
	fi
	;;
    "c")
	${GREP_CMD} "^${SERVICE}.*.${PROTOCOL}" ${INETD_CONF} >/dev/null 2>&1
	if [ $? -eq 1 ] ; then		# Then service_name is NOT in inetd.conf
		dspmsg chsubserver.cat -s 1 3 "${COMMAND_NAME}: ${SERVICE} not in ${INETD_CONF}\n" ${COMMAND_NAME} ${SERVICE} ${INETD_CONF}
		exit 1
	else				# Then services_name is in inetd.conf
					# so we need to delete the old line
					# and add the new one.
		if [ -z "${SOCKET}" -a -z "${NEW_SOCKET}" ] ; then
		    # If not set then get the old value from the config file
		    SOCKET=`${GREP_CMD} "^${SERVICE}" ${INETD_CONF} | ${AWK_CMD} '{ print $2 }'`
		fi
		if [ -z "${WAIT}" -a -z "${NEW_WAIT}" ] ; then
		    # If not set then get the old value from the config file
		    WAIT=`${GREP_CMD} "^${SERVICE}" ${INETD_CONF} | ${AWK_CMD} '{ print $4 }'`
		fi
		if [ -z "${USER}" -a -z "${NEW_USER}" ] ; then
		    # If not set then get the old value from the config file
		    USER=`${GREP_CMD} "^${SERVICE}" ${INETD_CONF} | ${AWK_CMD} '{ print $5 }'`
		fi
		if [ -z "${PROG}" -a -z "${NEW_PROG}" ] ; then 
		    # If not set then get the old value from the config file
		    PROG=`${GREP_CMD} "^${SERVICE}" ${INETD_CONF} | ${AWK_CMD} '{ print $6 }'`
		fi
		if [ -z "${ARGS}" ] ; then 
		    # If not set then get the old value from the config file
		    ARGS=`${GREP_CMD} "^${SERVICE}" ${INETD_CONF} | ${AWK_CMD} '{ for (i = 7; i <= NF; i++); print $i }'`
		fi
		${CAT_CMD} ${INETD_CONF} | ${SED_CMD} "/^${SERVICE}.*.${PROTOCOL}/d" > ${TMP_INETD_CONF} 2>/dev/null
		${ECHO_CMD} "${NEW_SERVICE:-${SERVICE}}\t${NEW_SOCKET:-${SOCKET}}\t${NEW_PROTOCOL:-${PROTOCOL}}\t${NEW_WAIT:-${WAIT}}\t${NEW_USER:-${USER}}\t${NEW_PROG:-${PROG}} ${ARGS}" >> ${TMP_INETD_CONF}
		if [ $? != 0 ] ; then
			dspmsg chsubserver.cat -s 1 2 "${COMMAND_NAME}: error in updating ${INETD_CONF}\n" ${COMMAND_NAME} ${INETD_CONF}
			exit 1
		fi
	fi
	;;
esac

#copy the file back to /etc
${CP_CMD} ${TMP_INETD_CONF} ${INETD_CONF} > /dev/null 2>&1
if [ $? != 0 ] ; then
	dspmsg chsubserver.cat -s 1 2 "${COMMAND_NAME}: error in updating ${INETD_CONF}\n" ${COMMAND_NAME} ${INETD_CONF}
	${RM_CMD} -f ${TMP_INETD_CONF} > /dev/null 2>&1
	exit 1	
fi

if [ $? != 0 ] ; then
	dspmsg chsubserver.cat -s 1 2 "${COMMAND_NAME}: error in updating ${INETD_CONF}\n" ${COMMAND_NAME} ${INETD_CONF}
	${RM_CMD} ${TMP_INETD_CONF} > /dev/null 2>&1
	exit 1
fi

# Remove the temporary file
${RM_CMD} ${TMP_INETD_CONF} > /dev/null 2>&1

# Send signal to server, if specified
if [ -n "${REFRESH}" ] ; then
	${PS_CMD}|${GREP_CMD} ${REFRESH}|${AWK_CMD} '{ print $1 }'|${XARGS_CMD} ${KILL_CMD} > /dev/null 2>&1
	exit $?
fi
