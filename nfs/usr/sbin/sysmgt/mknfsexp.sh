#!/bin/bsh
#
# @(#)03	1.23  src/nfs/usr/sbin/sysmgt/mknfsexp.sh, cmdnfs, nfs41J, 9514A_all 4/3/95 17:34:18
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
# Translate commas or spaces to a single colon
tr_comma_colon() { /usr/bin/tr -d " " | /usr/bin/tr -s ',' ':' ; }

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

# Function to take the output from the 'lsnfsexp' command and fill in the
# blank fields so that later we can tell whether an export had a
# particular option specified.

awk_fill_null_fields() { \
	/bin/awk \
		'BEGIN {FS=":"} ; {\
		if ($1 == "") { printf "_null " }else{printf "%s ", $1} ;\
		if ($2 == "") { printf "_null " }else{printf "%s ", $2} ;\
		if ($3 == "") { printf "_null " }else{printf "%s ", $3} ;\
		if ($4 == "") { printf "_null " }else{printf "%s ", $4} ;\
		if ($5 == "") { printf "_null " }else{printf "%s ", $5} ;\
		if ($6 == "") { printf "_null " }else{printf "%s ", $6} ;\
		if ($7 == "") { printf "_null " }else{printf "%s ", $7} ;\
		if ($8 == "") { printf "_null " }else{printf "%s\n", $8} \
		} ' ; }
		

# Variables used just within this script
AWK=/bin/awk
LSNFSEXP=/usr/sbin/lsnfsexp
EXPORTS_FILE=/etc/exports
XTAB_FILE=/etc/xtab
EXPORTFS_CMD=/usr/sbin/exportfs
COMMAND_NAME=`basename $0`
TMP_FILE=/tmp/${COMMAND_NAME}.$$

USAGE="
Usage:\n${COMMAND_NAME}	-d directory [-f exports_file]\n
\t[-t {rw|ro}{|rm -h hostname[,hostname]...}] \n
\t[-a uid] \n
\t[-r hostname[,hostname]... \n
\t[-c hostname[,hostname]... \n
\t[-s|-n] [-I|-B|-N]\n"

TAB=" "

# Remove any spaces that are in the middle of parameters
PARMS=""
num_parms=$#
while [ $num_parms != 0 ] ; do
   if [ -z "$1" ] ; then
	PARMS="${PARMS} _NULL_PARM"
   else
      PARM=`echo "$1\c" | awk_strip_blanks | /usr/bin/tr -s ' ' ','`
      PARMS="${PARMS} ${PARM}"
   fi
   shift
   num_parms=`expr $num_parms - 1`
done
set -- ${PARMS}

# Parse the command line arguments to find out what directory
# is to be exported and how it is to be exported.

set -- `getopt d:f:t:h:a:r:c:snIBN  $*  2>/dev/null` 
if [ $? != 0 ] ; then         # Test for syntax error
    dspmsg cmdnfs.cat -s 37 7 "${USAGE}" ${COMMAND_NAME} 1>&2
    exit 1
fi

while [ $1 != -- ]
do
    case "$1" in
	"-d")			#directory to export
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1 ; fi
	    if [ -n "${_DIRECTORY}" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1 ; fi
	    _DIRECTORY=$2
	    shift; shift
	    ;;
	"-t")			#read-write, read-only, read-mostly?
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1 ; fi
	    if [ -n "${TYPE_EXPORT}" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE" ${COMMAND_NAME} 1>&2 ; exit 1 ; fi
	    TYPE_EXPORT=$2
	    shift; shift
	    ;;
	"-h")			#List of hosts for read-mostly
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1 ; fi
	    if [ -n "${RM_HOSTLIST}" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE" ${COMMAND_NAME} 1>&2 ; exit 1 ; fi
	    if [ "$2" = "_NULL_PARM" ] ; then
 		   RM_HOSTLIST=""
	    else
		RM_HOSTLIST=$2
	    fi
	    if [ "${RM_HOSTLIST}" != "" ] ; then
	       # Convert the list of hosts into a colon separated list
	       RM_HOSTLIST=` { echo "$2" | tr_comma_colon ; }`
	    fi
	    shift; shift
	    ;;
	"-a")			#Anonymous uid option
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1 ; fi
	    if [ -n "${ANON_UID}" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1 ; fi
	    ANON_UID=$2
	    shift; shift
	    ;;
	"-r")			#Hosts allowed root access
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1 ; fi
	    if [ -n "${ROOT_HOSTLIST}" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE" ${COMMAND_NAME}  1>&2 ; exit 1 ; fi
	    if [ "$2" = "_NULL_PARM" ] ; then
	       if [ "${COMMAND_NAME}" = "chnfsexp" ] ; then
 		   ROOT_HOSTLIST="_NULL_PARM"
	       else
	          dspmsg cmdnfs.cat -s 37 7 "$USAGE" ${COMMAND_NAME}  1>&2 ;
		  exit 1 ;
	       fi
	    fi
	    ROOT_HOSTLIST=$2
	    if [ "${ROOT_HOSTLIST}" != "_NULL_PARM" ] ; then
	       # Convert the list of hosts into a colon separated list
	       ROOT_HOSTLIST=` { echo "$2" | tr_comma_colon ; } `
	    fi
	    shift; shift
	    ;;
	"-c")			#Hosts allowed client access
	    if [ -z "$2" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE" ${COMMAND_NAME}  1>&2 ; exit 1 ; fi
	    if [ -n "${CLIENT_HOSTLIST}" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE" ${COMMAND_NAME}  1>&2 ; exit 1 ; fi
	    if [ "$2" = "_NULL_PARM" ] ; then
	       if [ "${COMMAND_NAME}" = "chnfsexp" ] ; then
 		   CLIENT_HOSTLIST="_NULL_PARM"
	       else
	          dspmsg cmdnfs.cat -s 37 7 "$USAGE" ${COMMAND_NAME}  1>&2 ;
		  exit 1 ;
	       fi
	    fi
	    CLIENT_HOSTLIST=$2
	    if [ "${CLIENT_HOSTLIST}" != "_NULL_PARM" ] ; then
	       # Convert the list of hosts into a colon separated list
	       CLIENT_HOSTLIST=` { echo "$2" | tr_comma_colon ; } `
            fi
	    shift; shift
	    ;;
	# work through the single letter options.  
	# we have to cover all of the combitnations possible
	"-s")
	    if [ -n "${SECURE}" ] ; then  dspmsg cmdnfs.cat -s 37 7 "${USAGE}" ${COMMAND_NAME}  1>&2 ; exit 1 ; fi
	    SECURE="yes"
	    shift
	    ;;
	"-n")
	    if [ -n "${SECURE}" ] ; then  dspmsg cmdnfs.cat -s 37 7 "${USAGE}" ${COMMAND_NAME}  1>&2 ; exit 1 ; fi
	    SECURE="no"
	    shift
	    ;;
	"-I")
	    if [ -n "${WHEN}" ] ; then	dspmsg cmdnfs.cat -s 37 7 "${USAGE}" ${COMMAND_NAME}  1>&2 ;	exit 1 ; fi
	    WHEN="I"
	    shift
	    ;;
	"-B")
	    if [ -n "${WHEN}" ] ; then	dspmsg cmdnfs.cat -s 37 7 "${USAGE}" ${COMMAND_NAME}  1>&2 ;	exit 1 ; fi
	    WHEN="B"
	    shift
	    ;;
	"-N")
	    if [ -n "${WHEN}" ] ; then	dspmsg cmdnfs.cat -s 37 7 "${USAGE}" ${COMMAND_NAME}  1>&2 ;	exit 1 ; fi
	    WHEN="N"
	    shift
	    ;;
	"-f")
	    EXPORTS_FILE=$2
	    shift
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 7 "$USAGE" ${COMMAND_NAME}  1>&2 
	    exit 1
	    ;;
    esac
done

# check to make sure that there are not any extra parameters
shift		# get rid of the -- parameter
if [ -n "$1" ] ; then	# something extra here.
    dspmsg cmdnfs.cat -s 37 7 "${USAGE}" ${COMMAND_NAME} 1>&2
    exit 1
fi

if [ -z ${EXPORTS_FILE} ] ; then
    dspmsg cmdnfs.cat -s 37 7 "${USAGE}" ${COMMAND_NAME} 1>&2
    exit 1
fi

# All done with parsing the parameters. Now move on to building
# the line that will be added to the exports file

# We have to do a little sanity checking along the way to make
# sure that the user specified all the right parameters in the right order.

# Check to make sure that a directory was specified
if [ -z "${_DIRECTORY}" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1 ; fi

# Get the old export information so that the a change of the
# export will have the correct semantics.
if [ "${COMMAND_NAME}" = "chnfsexp" ] ; then
	# Get the current export information and fill in the empty spaces
	_EXPORT_PARMS=`${LSNFSEXP} -c -f ${EXPORTS_FILE} ${_DIRECTORY} | \
		/bin/awk '{if (NR != 1) { print $0 }}' | \
		awk_fill_null_fields`

	# There may not be a export of the directory is to be changed.
	if [ -z "${_EXPORT_PARMS}" ] ; then
		break
	fi

	set -- ${_EXPORT_PARMS}

	# Remove our temporary file
	/bin/rm -rf ${TMP_FILE}

	if [ "$2" != "_null" ] ; then	# Check the export's old type(rw/rm/ro)
		OLD_TYPE_EXPORT=$2		
	fi
	if [ "$3" != "_null" ] ; then	# Check the old anonymous uid
		OLD_ANON_UID=$3
	fi
	if [ "$4" != "_null" ] ; then	# Check the read mostly host list
		OLD_RM_HOSTLIST=` { echo "$4" | tr_comma_colon ; } `
	fi
	if [ "$5" != "_null" ] ; then	# Check the root access list
		OLD_ROOT_HOSTLIST=` { echo "$5" | tr_comma_colon ; } `
	fi
	if [ "$6" != "_null" ] ; then	# Check the client access list
		OLD_CLIENT_HOSTLIST=` { echo "$6" | tr_comma_colon ; } `
	fi
	if [ "$7" != "_null" ] ; then	# Check the secure option
		if [ "$7" = "-n" ] ; then
			OLD_SECURE="no"
		else
			OLD_SECURE="yes"
		fi
	fi
fi

# Set up the defaults for the user if they did not specify these.
ANON_UID=${ANON_UID:-"${OLD_ANON_UID:-"-2"}"}
WHEN=${WHEN:-"B"}
SECURE=${SECURE:-"${OLD_SECURE:-"no"}"}

# If there was no 'type' specified for this export and 
# there was an old type, then set the 'type' to the old type.
if [ -z "$TYPE_EXPORT" -a -n "$OLD_TYPE_EXPORT" ] ; then
	TYPE_EXPORT=$OLD_TYPE_EXPORT
fi
# If the type of export was not specified and a read-mostly host
# list was specified then the user made an error.
if [ -z "$TYPE_EXPORT" ] ; then 
    if [ -n "$RM_HOSTLIST" ] ; then
	dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1
    fi

else	# Check the type of export and start building the exports line

    case "$TYPE_EXPORT" in
	"rw")		# nothing to be done to the exports line
	    if [ -n "$RM_HOSTLIST" ] ; then
		dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1
	    fi
	    ;;
	"ro")		# add this as an option to the exports line
	    if [ -n "$RM_HOSTLIST" ] ; then
		dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1
	    fi
	    EXPORTS_LINE_TO_ADD="ro"
	    ;;
	"rm") # Check the old read-mostly list to see if it should be used
	    if [ -z "$RM_HOSTLIST" -a -n "$OLD_RM_HOSTLIST" ] ; then
		RM_HOSTLIST=$OLD_RM_HOSTLIST
	    fi
	    if [ -z "$RM_HOSTLIST" ] ; then
		dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1
	    else
		EXPORTS_LINE_TO_ADD="rw=${RM_HOSTLIST}"
	    fi
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1
	    ;;
    esac
fi

# Add on the anonymous uid value to the exports line
# First check and see if it is a number?
ANON_CHECK=` { echo "${ANON_UID}" | /usr/bin/tr -s '0987654321-' '###########' ;  } `
if [ "${ANON_CHECK}" != "#" ] ; then dspmsg cmdnfs.cat -s 37 7 "$USAGE"  ${COMMAND_NAME} 1>&2 ; exit 1 ; fi

# Check to see if it is something other than the default
if [ "${ANON_UID}" != "-2" ] ; then 
    if [ -z "$EXPORTS_LINE_TO_ADD" ] ; then
	EXPORTS_LINE_TO_ADD="anon=${ANON_UID}"
    else
	EXPORTS_LINE_TO_ADD="${EXPORTS_LINE_TO_ADD},anon=${ANON_UID}"
    fi
fi

# If the root hostlist is not specified and there was an old one
# present for the directory, then use the old list.
if [ -z "$ROOT_HOSTLIST" -a -n "$OLD_ROOT_HOSTLIST" ] ; then
	ROOT_HOSTLIST=$OLD_ROOT_HOSTLIST
fi
# If a host list was specified for root access, add
# the list to the line to be added to the exports file
if [ -n "$ROOT_HOSTLIST" ] ; then
   if [ "${ROOT_HOSTLIST}" != "_NULL_PARM" ] ; then
      if [ -z "$EXPORTS_LINE_TO_ADD" ] ; then
	EXPORTS_LINE_TO_ADD="root=${ROOT_HOSTLIST}"
      else
	EXPORTS_LINE_TO_ADD="${EXPORTS_LINE_TO_ADD},root=${ROOT_HOSTLIST}"
      fi
   fi
fi

# If the client hostlist is not specified and there was an old one
# present for the directory, then use the old list.
if [ -z "$CLIENT_HOSTLIST" -a -n "$OLD_CLIENT_HOSTLIST" ] ; then
	CLIENT_HOSTLIST=$OLD_CLIENT_HOSTLIST
fi
# If a host list was specified for client access, add the
# list to the line to be added to the exports file
if [ -n "$CLIENT_HOSTLIST" ] ; then
   if [ "${CLIENT_HOSTLIST}" != "_NULL_PARM" ] ; then
      if [ -z "$EXPORTS_LINE_TO_ADD" ] ; then
	  EXPORTS_LINE_TO_ADD="access=${CLIENT_HOSTLIST}"
      else
	  EXPORTS_LINE_TO_ADD="${EXPORTS_LINE_TO_ADD},access=${CLIENT_HOSTLIST}"
      fi
   fi
fi

# If the secure option was specified, add it to the 
# line to be added to the exports file
if [ ${SECURE} = "yes" ] ; then
    if [ -z "$EXPORTS_LINE_TO_ADD" ] ; then
	EXPORTS_LINE_TO_ADD="secure"
    else
	EXPORTS_LINE_TO_ADD="${EXPORTS_LINE_TO_ADD},secure"
    fi
fi

# Find out how this command was invoked and check to see that the
# user is not trying to export a directory that is already exported
# or trying to change an export that does not exist

case "${COMMAND_NAME}" in
    "mknfsexp")
        if [ ! -d  ${_DIRECTORY} -a ! -f ${_DIRECTORY}  ] ; then
                dspmsg cmdnfs.cat -s 37 41 "%s: %s: No such file or directory.\n" ${COMMAND_NAME} ${_DIRECTORY} 1>&2
                exit 1;
        fi
	${LSNFSEXP} -c -f ${EXPORTS_FILE} ${_DIRECTORY} | \
		grep "${_DIRECTORY}:" > /dev/null  2>&1
	if [ $? = 0 ] ; then
	    dspmsg cmdnfs.cat -s 37 25 "mknfsexp: an export for ${_DIRECTORY} already exists\n" ${_DIRECTORY} 1>&2
	    exit 1
	else	# Check the /etc/exports file for the directory
	    ${AWK} '{print $1}' ${EXPORTS_FILE} > /tmp/mknfsexp.$$ 2>&1
	    while [ 1 ]
	    do
		read inline
		if [ $? -ne 0 ] ; then 
			exit 0
		fi
		if [ "$inline" = "$_DIRECTORY" ] ; then 
		    dspmsg cmdnfs.cat -s 37 25 "mknfsexp: an export for ${_DIRECTORY} already exists\n" ${_DIRECTORY} 1>&2
		    exit 1
		fi
	    done < /tmp/mknfsexp.$$
	    if [ $? -ne 0 ] ; then
		/bin/rm -rf /tmp/mknfsexp.$$ >/dev/null 2>&1
		exit 1
	    fi
	    /bin/rm -rf /tmp/mknfsexp.$$ >/dev/null 2>&1
	fi
	;;
    "chnfsexp")
	${LSNFSEXP} -c -f ${EXPORTS_FILE} ${_DIRECTORY} | \
		grep "${_DIRECTORY}" > /dev/null 2>&1
	if [ $? != 0 ] ; then
	    ${AWK} '{print $1}' ${EXPORTS_FILE} > /tmp/chnfsexp.$$ 2>&1
	    while [ 1 ]
	    do
		read inline
		if [ $? -ne 0 ] ; then 
			exit 1
		fi
		if [ "$inline" = "$_DIRECTORY" ] ; then 
		    exit 0
		fi
	    done < /tmp/chnfsexp.$$
	    if [ $? -ne 0 ] ; then
		dspmsg cmdnfs.cat -s 37 26 "chnfsexp: an export for ${_DIRECTORY} does not exist\n" ${_DIRECTORY} 1>&2
		/bin/rm -rf /tmp/chnfsexp.$$ >/dev/null 2>&1
		exit 1
	    fi
	    /bin/rm -rf /tmp/chnfsexp.$$ >/dev/null 2>&1
	fi
	;;
esac

# Done with the parsing and sanity checking.  Move onto actually
# exporting the directory
echo "${_DIRECTORY}${TAB}${EXPORTS_LINE_TO_ADD}"

TEMP_AWK=/tmp/_awk.script.$$		# for temp awk script
TEMP_EXPORTS=/tmp/_exports.$$		# where do we put the exports file
					# while we are working on it?

# Check to see if we should start the nfsd or the rpc.mountd
# so that the user will not have to worry about that.
if [ ! -f ${EXPORTS_FILE} ] ; then
	/bin/startsrc -s nfsd
	if [ $? != 0 ] ; then
		exit 1
	fi
	/bin/startsrc -s rpc.mountd
	if [ $? != 0 ] ; then
		exit 1
	fi
	/usr/bin/ps -ef | /usr/bin/grep -v grep | /usr/bin/grep rpc.lockd > /dev/null 2>&1
	if [ $? != 0 ] ; then
		/usr/bin/startsrc -s rpc.statd
		/usr/bin/startsrc -s rpc.lockd
	fi
fi

# We have to decide when the export is to take affect.
# If is to be done on IPL or now and on IPL, then add the
# exports line to the exports file.

case "${WHEN}" in
    "N")	# execute exportfs with the correct parameters
	PARAM=${EXPORTS_LINE_TO_ADD:+"-o ${EXPORTS_LINE_TO_ADD}"}
	CMD="${EXPORTFS_CMD} -f ${EXPORTS_FILE} -i -v ${PARAM} ${_DIRECTORY}"
	${CMD}
	;;
    "I"|"B")	# change the exports file so that the export will last.
	# If the exports file exists and is readable, we will scan the file.
	# If there exists and export for the directory we are about to export,
	# remove it from the file and add the specification we have ready.

	if [ -r ${EXPORTS_FILE} ] ; then
	    # Build awk script to copy all entries except the one that matches
	    # with the _DIRECTORY that is to be added to the exports file.
            echo " { if (\"${_DIRECTORY}\" != \$1) print \$0 } " > ${TEMP_AWK}
            { /bin/awk -f ${TEMP_AWK} < ${EXPORTS_FILE} > ${TEMP_EXPORTS} ; }

            # Check to make sure that the awk script ran without errors
            if [ $? != 0 ] ; then 
		dspmsg cmdnfs.cat -s 37 33 "Unable to change ${EXPORTS_FILE}. Aborting action\n" ${EXPORTS_FILE} 1>&2
		exit 1
	    fi

            # Copy the new version of the exports file back to the original
	    cp ${TEMP_EXPORTS} ${EXPORTS_FILE}
	    if [ $? != 0 ] ; then 
		dspmsg cmdnfs.cat -s 37 33 "Unable to change ${EXPORTS_FILE}. Aborting action\n" ${EXPORTS_FILE} 1>&2
		exit 1
	    fi

	    # Add the prepared line to the end of the exports file
	    if [ -z "${EXPORTS_LINE_TO_ADD}" ] ; then
		echo "${_DIRECTORY}" >> "${EXPORTS_FILE}"
	    else
	        echo "${_DIRECTORY}${TAB}-${EXPORTS_LINE_TO_ADD}" >> ${EXPORTS_FILE}
	    fi

	    # Remove the temporary files used
	    /bin/rm ${TEMP_AWK} ${TEMP_EXPORTS}

	else		# file didn't exist; create it.
	    echo "${_DIRECTORY}${TAB}-${EXPORTS_LINE_TO_ADD}" >> ${EXPORTS_FILE}
	fi

	# Check to see if the directory is to be exported now.
	if [ "${WHEN}" = "B" ] ; then
	    ${EXPORTFS_CMD} -f ${EXPORTS_FILE} -v ${_DIRECTORY}

	    # Check to make sure that everything is okay.
	    if [ "${COMMAND_NAME}" = "mknfsexp" -a $? -ne 0 ] ; then

	     # Build awk script to copy all entries except the one to remove
	     echo " { if (\"${_DIRECTORY}\" != \$1) print \$0 } " > ${TEMP_AWK}
             { /bin/awk -f ${TEMP_AWK} < ${EXPORTS_FILE} > ${TEMP_EXPORTS} ; }

             # Check to make sure that the awk script ran without errors
             if [ $? != 0 ] ; then 
	         # Remove the temporary files used
	         /bin/rm ${TEMP_AWK} ${TEMP_EXPORTS}
		 exit 1
	     fi

             # Copy the new version of the exports file back to the original
	     cp ${TEMP_EXPORTS} ${EXPORTS_FILE}
	     if [ $? != 0 ] ; then 
	         # Remove the temporary files used
	         /bin/rm ${TEMP_AWK} ${TEMP_EXPORTS}
		 exit 1
	     fi

	     # Remove the temporary files used
	     /bin/rm ${TEMP_AWK} ${TEMP_EXPORTS}

	     # Export did not complete.
	     exit 1

	    fi
	fi
	;;
esac

