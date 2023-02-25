#!/bin/ksh
# @(#)56	1.6  src/nfs/usr/sbin/sysmgt/mkhanfs.sh, cmdnfs, nfs411, GOLD410 10/3/91 17:26:55
# COMPONENT_NAME: (CMDNFS) Network File System Commands
#
# FUNCTIONS: mkhanfs
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
RM_CMD="/bin/rm"
CAT_CMD="/bin/cat"
CP_CMD="/bin/cp"
TR_CMD="/bin/tr"
GETOPT_CMD="getopts"
CFG_FILE="/etc/hanfs"
RC_NFS="/etc/rc.nfs"
TMP_RC_NFS="/tmp/tmp.rc.nfs$$"
STARTSRC="/bin/startsrc"

USAGE="
Usage:\n\
${COMMAND_NAME} -T Twin_primary_hostname\n\
\t-t twin_secondary_hostname\n\
\t-a twin_alternate_hostname\n\
\t-N primary_net_adapter\n\
\t-n secondary_net_adapter\n\
\t-D primary_disk_list\n\
\t-d secondary_disk_list\n\
\t -u udp_port \n\
\t -l logfile \n\
\t -e exports_file \n\
\t -w timeout_value \n\
\t -g grace_period \n\
\t -p number_of_pings \n"
 
#set default values
OPTu=255			#udp port
OPTl="/dev/null"		#file to log HA-NFS events
OPTe="/etc/exports.hanfs"	#exports file for hanfs
OPTw=30				#seconds to wait before suspecting a failure
OPTg=120			#time before heartbeats start
OPTp=10				#number of pings before assuming failure

# Parse the command line arguments 

	while getopts :t:T:a:N:n:D:d:u:w:p:l:e:g: flag
	do
		case $flag in
			t)
				OPTt=$OPTARG
				;;
			T)
				OPTT=$OPTARG
				;;	
			a)
				OPTa=$OPTARG
				;;	
			N)
				OPTN=$OPTARG
				;;	
			n)
				OPTn=$OPTARG
				;;	
			D)
				OPTD=$OPTARG
				;;	
			d)
				OPTd=$OPTARG
				;;	
			u)
				OPTu=$OPTARG
				;;	
			w)
				OPTw=$OPTARG
				;;	
			p)
				OPTp=$OPTARG
				;;	
			l)
				OPTl=$OPTARG
				;;	
			e)
				OPTe=$OPTARG
				;;	
			g)
				OPTg=$OPTARG
				;;	
			:)
				print -u2 "$0: $OPTARG requires a value:"
	    			dspmsg cmdnfs.cat -s 52 7 "$USAGE" ${COMMAND_NAME}
				exit 1
				;;	
			\?)
				print -u2 "$0: Unknown option $OPTARG"
	    			dspmsg cmdnfs.cat -s 52 7 "$USAGE" ${COMMAND_NAME}
	    			exit 1
	    			;;
		esac
	done

# check to make sure that there are not any extra parameters
# if [ -n "$1" ] ; then	# something extra here.
#    dspmsg cmdnfs.cat -s 52 7 "${USAGE}" ${COMMAND_NAME}
#    exit 1
#fi

HOST=`/bin/hostname | /bin/sed 's/\..*//'`
if [ -z "$HOST" ] ; then 
    dspmsg cmdnfs.cat -s 52 4 "The hostname is not set\n" ${COMMAND_NAME} "hostname"
    exit 1 
fi
	

# Build the configuration file from the input parameters

${CAT_CMD} > $CFG_FILE << ENDSTR
${HOST}:
	ptwin 		= ${OPTT}
	stwin 		= ${OPTt}
	ttwin 		= ${OPTa}
	pnalocn 	= ${OPTN}
	snalocn 	= ${OPTn}
	port 		= ${OPTu}
	timeout 	= ${OPTw}
	numpings 	= ${OPTp}
	logfile 	= ${OPTl}
	exportfile 	= ${OPTe}
	graceperiod 	= ${OPTg}
ENDSTR

integer pri=0
for i in $OPTD
do
${CAT_CMD} >> $CFG_FILE << ENDSTR
	pdalocn$pri  	= $i
ENDSTR
	pri=pri+1
done

integer sec=0
for i in $OPTd
do
${CAT_CMD} >> $CFG_FILE << ENDSTR
	sdalocn$sec  	= $i
ENDSTR
	sec=sec+1
done

# uncomment the code in the /etc/rc.nfs file for ha-nfs
${SED_CMD} "\?startsrc -s hanfsd?,/fi/s/#//" ${RC_NFS} > ${TMP_RC_NFS} 2>/dev/null
if [ $? != 0 ] ; then
	dspmsg cmdnfs.cat -s 52 6 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	${RM_CMD} ${TMP_RC_NFS} > /dev/null 2>&1
	exit 1
fi
# copy the file back to /etc
${CP_CMD} ${TMP_RC_NFS} ${RC_NFS} > /dev/null 2>&1
if [ $? != 0 ] ; then
	dspmsg cmdnfs.cat -s 52 6 "${COMMAND_NAME}: error in updating ${RC_NFS}\n" ${COMMAND_NAME} ${RC_NFS}
	${RM_CMD} ${TMP_RC_NFS} > /dev/null 2>&1
	exit 1
fi

#remove the temporary file
${RM_CMD} ${TMP_RC_NFS} > /dev/null 2>&1

exit $?

