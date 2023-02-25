#!/bin/bsh
#
# @(#)03      1.22  src/nfs/usr/sbin/sysmgt/lsnfsexp.sh, cmdnfs, nfs411, 9432B411a 8/9/94 09:36:03
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
EXPORTED=/tmp/exports.$$
TMP_FILE=/tmp/lsnfsexp.$$
COMMAND_NAME=`basename $0`
TR=/usr/bin/tr
EXPORTFS=/usr/sbin/exportfs
DIFF=/bin/diff
AWK=/bin/awk
CAT=/usr/bin/cat
SORT=/usr/bin/sort
CUT=/usr/bin/cut
DIR_OUTPUT=no
LIST_ALL="false"

USAGE="usage: ${COMMAND_NAME} [-f exports_file ] [-c | -l ] [directory]\n"

set -- `getopt f:lca $*  2>/dev/null`

if [ $? != 0 ] ; then
	dspmsg cmdnfs.cat -s 37 37 "${USAGE}" $COMMAND_NAME
	exit 1
fi

# Find out how we are supposed to print the information to the user.

while [ $1 != -- ]
do
    case "$1" in
	"-c")			#
	    if [ -n "${FORMAT}" ] ; then dspmsg cmdnfs.cat -s 37 37 "$USAGE" $COMMAND_NAME ; exit 1 ; fi
	    FORMAT="colon"
	    shift
	    ;;
	"-l")			#
	    if [ -n "${FORMAT}" ] ; then dspmsg cmdnfs.cat -s 37 37 "$USAGE" $COMMAND_NAME ; exit 1 ; fi
	    FORMAT="list"
	    shift
	    ;;
	"-f")			#
	    EXPORTS_FILE=$2
	    shift
	    shift
	    ;;
	"-a")
	    LIST_ALL="true"
	    shift 
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 37 "${USAGE}" $COMMAND_NAME
	    exit 1
	    shift
	    ;;
    esac
done

shift 		# get rid of the -- parameter
if [ $# -gt 1 ] ; then
	dspmsg cmdnfs.cat -s 37 37 "${USAGE}" $COMMAND_NAME
	exit 1
elif [ $# -eq 1 ] ; then
	_DIRECTORY=$1
fi

if [ -z ${EXPORTS_FILE} ] ; then
  	dspmsg cmdnfs.cat -s 37 37 "${USAGE}" $COMMAND_NAME
  	exit 1
fi	

# create a temporary file which includes all the valid entries
# in either /etc/exports or /etc/xtab files.
cat -q ${EXPORTS_FILE} ${XTAB_FILE} | ${AWK} '
BEGIN {
	i = 0
}
# main of the awk script
$1 !~ /^#/ {line[i] = $0; entry[i] = $1; i++}
END {
	j = 0
	k = 0
	for(j = 0; j < i; j++)
		if(entry[j] != "#")
			for(k = j + 1; k < i; k++)
			{
				if(entry[j] == entry[k])
					{entry[k] = "#"}
			}
	for(j = 0; j < i; j++)
		if(entry[j] != "#")
			print line[j]
}' - > ${EXPORTED}

if [ ! -r ${EXPORTED} ]; then
	exit 1
fi

# Find the information that we are supposed to print to the user.

if [ -n "$_DIRECTORY" ] ; then
 	if [ -d "$_DIRECTORY" ] || [ -f "$_DIRECTORY" ] ; then
	cat > ${TMP_FILE} << END_INPUT
END_INPUT
	while [ 1 ]
	do
	    read export_directory excess_parameters
	    if [ $? != 0 ] ; then
		break
	    fi
	    if [ "$export_directory" = "$_DIRECTORY" ] ; then
		echo "$export_directory $excess_parameters" >> ${TMP_FILE}
	    fi
	done < ${EXPORTED}
	else
		if [ "${FORMAT}" = "colon" ] ; then
			echo "#directory:type:anonuid:mostly:root:access:secure"
		fi
  	   	dspmsg cmdnfs.cat -s 37 41 "%s: %s: No such file or directory.\n" $COMMAND_NAME $_DIRECTORY
  	   exit 1
	fi
else
	${EXPORTFS} -f ${EXPORTS_FILE} > ${EXPORTED} 2>/dev/null
	if [ ${LIST_ALL} = "true" ] ; then
		${CAT} -q /etc/exports >> ${EXPORTED}
	fi
	if [ -r ${EXPORTED} ] ; then
		${CUT} -d" " -f1 ${EXPORTED} | ${SORT} | uniq > ${TMP_FILE}
	else
		exit 1
	fi
fi

if [ "${FORMAT}" = "colon" ] ; then
	echo "#directory:type:anonuid:mostly:root:access:secure"
else
	egrep -e '^\/' ${TMP_FILE} 2>/dev/null
	rm -rf ${TMP_FILE} >/dev/null 2>&1 
	rm -rf ${EXPORTED} >/dev/null 2>&1 
	exit 0
fi

exec < ${TMP_FILE}

while [ 1 ]
do

	# Set up the defaults for the export
	directory=
	type="rw"
	anonuid="-2"
	readmostly=
	root=
	access=
	secure="-n"

	# Get the export line that is in the temporary file.
	read inputline
	if [ $? != 0 ] ; then
		break
	fi

	# Set the default STATE
	STATE=directory

	# Parse the export line so that we can print it correctly
	for token in `echo $inputline | ${TR} '=,' '  '`
	do
	    case "$STATE" in
	    "directory")
		directory=$token
		STATE=parameters
		STATE_PARAMS=unknown
		FIRST_OPTION=1;
		;;
	    "parameters")
		if [ $FIRST_OPTION -eq 1 ]
		then
			export token
			token=`(IFS=-;echo $token)`
			FIRST_OPTION=0
		fi
		case "$STATE_PARAMS" in
		"oldaccess")	# only hosts listed
			HOSTS_LIST="$HOSTS_LIST:$token"
			STATE_PARAMS=unknown
			;;
		"access")	# 'access' parameter used for host access
			access=$token
			STATE_PARAMS=unknown
			;;
		"type")		# rw or ro?
			STATE_PARAMS=unknown
			;;
		"readmostly")	# Those hosts allowed read access
			type="rm"		# To match SMIT menus
			readmostly=$token
			STATE_PARAMS=unknown
			;;
		"secure")	# Exported in secure mode?
			STATE_PARAMS=unknown
			;;
		"anonuid")	# Anonymous uid
			anonuid=$token
			STATE_PARAMS=unknown
			;;
		"root")		# Hosts allowed 'root' access
			root=$token
			STATE_PARAMS=unknown
			;;
		*)		# Unknown state
			# Find out what parameters state we should be in
			case "$token" in
			"anon")
				STATE_PARAMS=anonuid
				;;
			"rw")
				STATE_PARAMS=readmostly
				;;
			"ro")
				type="ro"
				STATE_PARAMS=unknown
				;;
			"secure")
				secure="-s"
				STATE_PARAMS=unknown
				;;
			"access")
				STATE_PARAMS=access
				;;
			"root")
				STATE_PARAMS=root
				;;
			*)
				STATE_PARAMS=oldaccess
				HOSTS_LIST=$token
				;;
			esac
			;;
		esac
		
		;;
	    *)
		;;
	    esac
	done

	# Replace the colons with commas
	readmostly=`echo $readmostly | ${TR} ':' ','`
	root=`echo $root | ${TR} ':' ','`
	access=`echo $access | ${TR} ':' ','`

	# Print the line 
	case "$directory" in
	   /*)		# a valid directory
	          echo "$directory:$type:$anonuid:$readmostly:$root:$access:$secure"
		  DIR_OUTPUT="yes"
		  ;;
	     *)		# anything else... like nothing exported message
		  ;;
        esac
	
done 

rm -rf ${TMP_FILE} > /dev/null 2>&1 
rm -rf ${EXPORTED} > /dev/null 2>&1

if [ "$DIR_OUTPUT" = "yes" ]; then
	exit 0
else
        dspmsg cmdnfs.cat -s 37 32 "%s: 1831-361 %s not currently exported.\n" ${COMMAND_NAME} ${_DIRECTORY}
	exit 1
fi
