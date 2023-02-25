#!/bin/bsh
# @(#)04	1.18.1.3  src/nfs/usr/sbin/sysmgt/mknfsmnt.sh, cmdnfs, nfs41J, 9508A 2/13/95 16:34:56
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

# unset the following to avoid they pick up environment varialbe's value
unset PATH_NAME REMOTE_DIRECTORY REMOTE_HOST TYPE_READ_WRITE \
      MOUNT_TYPE_NAME WHERE_MOUNT SOFT_HARD SECURE WHEN INTR AUTOM \
      RETRY_TIMES BUFSIZ_READ TIMEOUT INET_PORT ACREGMIN ACREGMAX \
      ACDIRMIN ACDIRMAX ACTIMEO NUM_BIODS SHORTDEV SUID DEV

# Local functions.
# Translate commas or spaces to a single colon
tr_comma_colon() { tr -s ', ' '[:*]' ; }

# Function to take the output from the 'lsnfsmnt' command and fill in the
# blank fields so that later we can tell whether an export had a
# particular option specified.

awk_fill_null_fields() { \
	/bin/awk \
		'BEGIN {FS="|"} ; {\
		if ($1 == "") { printf "_null " }else{printf "%s ", $1} ;\
		if ($2 == "") { printf "_null " }else{printf "%s ", $2} ;\
		if ($3 == "") { printf "_null " }else{printf "%s ", $3} ;\
		if ($4 == "") { printf "_null " }else{printf "%s ", $4} ;\
		if ($5 == "") { printf "_null " }else{printf "%s ", $5} ;\
		if ($6 == "") { printf "_null " }else{printf "%s ", $6} ;\
		if ($7 == "") { printf "_null " }else{printf "%s ", $7} ;\
		if ($8 == "") { printf "_null " }else{printf "%s ", $8} ;\
		if ($9 == "") { printf "_null " }else{printf "%s ", $9} ;\
		if ($10 == "") { printf "_null " }else{printf "%s ", $10} ;\
		if ($11 == "") { printf "_null " }else{printf "%s ", $11} ;\
		if ($12 == "") { printf "_null " }else{printf "%s ", $12} ;\
		if ($13 == "") { printf "_null " }else{printf "%s ", $13} ;\
		if ($14 == "") { printf "_null " }else{printf "%s ", $14} ;\
		if ($15 == "") { printf "_null " }else{printf "%s ", $15} ;\
		if ($16 == "") { printf "_null " }else{printf "%s ", $16} ;\
		if ($17 == "") { printf "_null " }else{printf "%s ", $17} ;\
		if ($18 == "") { printf "_null " }else{printf "%s ", $18} ;\
		if ($19 == "") { printf "_null " }else{printf "%s ", $19} ;\
		if ($20 == "") { printf "_null " }else{printf "%s ", $20} ;\
		if ($21 == "") { printf "_null " }else{printf "%s ", $21} ;\
		if ($22 == "") { printf "_null " }else{printf "%s ", $22} ;\
		if ($23 == "") { printf "_null " }else{printf "%s ", $23} ;\
		if ($24 == "") { printf "_null " }else{printf "%s\n", $24} \
		} ' ; }

# Variables used just within this script
MOUNT_CMD="/usr/sbin/mount"
UNMOUNT_CMD="/usr/sbin/umount"
LSNFSMNT="/usr/sbin/lsnfsmnt"
COMMAND_NAME=`basename $0`

CRFS_CMD="/usr/sbin/crfs"
CHFS_CMD="/usr/sbin/chfs"

USAGE="
usage:\n${COMMAND_NAME}	-f path_name -d remote_directory -h remote_host
\t[-t {rw|ro}]
\t[-m mount_type_name]
\t[-w {fg|bg}] [{-X|-x}]
\t[{-S|-H}] [{-Y|-y}] [{-Z|-z}]]
\t[{-e|-E}] [{-a|-A}]
\t[{-s|-n}] [{-I|-B|-N}]
\t[-r times_to_retry] [-b read_buffer_size] [-c write_buffer_size]
\t[-o timeout] [-P port_number]
\t[-u acregmin] [-U acregmax] [-v acdirmin] [-V acdirmax] [-T actimeo]
\t[-p num_biods]\n"

# Parse the command line arguments to find out what directory
# is to be mounted, etc.

num_opt=1
tot_num_op=$#
while [ $num_opt -le $tot_num_op ]
do
    case "$1" in
	"-f")			#path name to mount point
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		PATH_NAME=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		PATH_NAME=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-d")			#remote directory for mounting
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		REMOTE_DIRECTORY=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		REMOTE_DIRECTORY=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-h")			#remote host of the directory
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		REMOTE_HOST=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		REMOTE_HOST=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-t")			#type of mount read-only or read-write
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		TYPE_READ_WRITE=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		TYPE_READ_WRITE=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-m")			#mount type name
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		MOUNT_TYPE_NAME=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		MOUNT_TYPE_NAME=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-w")			#where to try mount foreground | background
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		WHERE_MOUNT=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		WHERE_MOUNT=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-S")			#mount is to be a SOFT mount
	    SOFT_HARD="soft"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-H")			#mount is to be a HARD mount
	    SOFT_HARD="hard"	
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-s")			#Use the more secure protocol
	    SECURE="yes"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-n")			#Do NOT use the more secure protocol
	    SECURE="no"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-I")			#WHEN should this mount take place (IPL)
	    WHEN="I"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-N")			#WHEN should this mount take place (NOW)
	    WHEN="N"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-B")			#WHEN should this mount take place (BOTH)
	    WHEN="B"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-e")			#do not allow interrupts on this mount
	    INTR="no"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-E")			#allow interrupts on this mount
	    INTR="yes"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-a")			#do not mount the directory on re-boot
	    AUTOM="no"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-A")			#mount the directory on re-boot
	    AUTOM="yes"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-r")			#number of time to retry mounting
	    if [ "x`echo $2 | cut -c 1`" != 'x-' ]
	    then
		RETRY_TIMES=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		RETRY_TIMES=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-b")			#buffer size for READS
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		BUFSIZ_READ=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		BUFSIZ_READ=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-c")			#buffer size for WRITES
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		BUFSIZ_WRITE=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		BUFSIZ_WRITE=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-o")			#NFS timeout
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		TIMEOUT=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		TIMEOUT=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-P")			#Internet port number to try daemons
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		INET_PORT=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		INET_PORT=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-u")			#acregmin value
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		ACREGMIN=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		ACREGMIN=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-U")			#acregmax value
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		ACREGMAX=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		ACREGMAX=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-v")			#acdirmin value
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		ACDIRMIN=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		ACDIRMIN=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-V")			#acdirmax value
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		ACDIRMAX=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		ACDIRMAX=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-T")			#actimeo
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		ACTIMEO=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		ACTIMEO=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;
	"-p")			#number of biods to dedicate to this mount
	    if [ "x`echo $2 | cut -c 1`" != "x-" ]
	    then
		NUM_BIODS=$2
		num_opt=`expr $num_opt + 2`
		shift; shift
	    else
		NUM_BIODS=
		num_opt=`expr $num_opt + 1`
		shift
	    fi
	    ;;

	"-x")			#server does not support 32 bit device numbers
	    SHORTDEV="no"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-X")			#server does support 32 bit device numbers
	    SHORTDEV="yes"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-y")			#do not allow execution by root            
	    SUID="no"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-Y")			# allow execution by root            
	    SUID="yes"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-z")			#do not allow device acces                 
	    DEV="no"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	"-Z")			# not allow device acces                 
	    DEV="yes"
	    num_opt=`expr $num_opt + 1`
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 39 "$USAGE"  ${COMMAND_NAME}
	    exit 1
	    ;;
    esac
done

# We have to do a little sanity checking along the way to make
# sure that the user specified all the right parameters in the right order.

# Was a local path name specified?
if [ -z "${PATH_NAME}" ] ; then dspmsg cmdnfs.cat -s 37 39 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
# Was the remote directory specified?
if [ -z "${REMOTE_DIRECTORY}" ] ; then dspmsg cmdnfs.cat -s 37 39 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi
# Was the remote host specified?
if [ -z "${REMOTE_HOST}" ] ; then dspmsg cmdnfs.cat -s 37 39 "${USAGE}" ${COMMAND_NAME} ; exit 1 ; fi

# Check to see if this is a change. If it is, get the old specification
# so that we can combine with those that were specified on the
# command line
if [ "$COMMAND_NAME" = "chnfsmnt" ] ; then
	# Obtain the current mount information and fill in the empty spaces
	TMP_MNT_PARMS=`${LSNFSMNT} -p ${PATH_NAME} | \
		/bin/awk '{if (NR != 1) {print $0}}'`
	if [ "x$TMP_MNT_PARMS" = "x" ]
	then
		dspmsg cmdnfs.cat -s 37 42 "%s: Error. %s is not an NFS file system.\n" ${COMMAND_NAME} ${PATH_NAME}
		exit 1
	fi

	# Check to make sure that we got something from the ls command
	if [ -n "$_MOUNT_PARMS" ] ; then

		# Set the current parameter list so that we may use its functionality
		set -- ${_MOUNT_PARMS}


		shift
		if [ "$1" != "_null" ] ; then	# Is there an old remote directory path
			OLD_REMOTE_DIRECTORY=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# What is the old remote host
			OLD_REMOTE_HOST=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Did a 'type=' parameter field exist
			OLD_MOUNT_TYPE_NAME=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Was this read-write or read-only
			OLD_TYPE_READ_WRITE=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Where to mount fg/bg
			OLD_WHERE_MOUNT=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Should the mount be soft/hard
			if [ "$1" = "-S" ] ; then OLD_SOFT_HARD="soft" ; fi
			if [ "$1" = "-H" ] ; then OLD_SOFT_HARD="hard" ; fi
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Use the secure protocol for mount
			if [ "$1" = "-s" ] ; then OLD_SECURE="yes" ; fi
			if [ "$1" = "-n" ] ; then OLD_SECURE="no" ; fi
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Is the mount supposed to be intr.
			if [ "$1" = "-E" ] ; then OLD_INTR="yes" ; fi
			if [ "$1" = "-e" ] ; then OLD_INTR="no" ; fi
		fi
		shift
		if [ "$1" != "_null" ] ; then	# How many times to retry mount
			OLD_RETRY_TIMES=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Size of the read buffer
			OLD_BUFSIZ_READ=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Size of the write buffer
			OLD_BUFSIZ_WRITE=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Timeout to be used for mount
			OLD_TIMEOUT=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Internet port to use for mount
			OLD_INET_PORT=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Attribute cache timeout...
			OLD_ACREGMIN=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	#  Attribute cache timeout...
			OLD_ACREGMAX=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	#  Attribute cache timeout...
			OLD_ACDIRMIN=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	#  Attribute cache timeout...
			OLD_ACDIRMAX=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	#  Attribute cache timeout...
			OLD_ACTIMEO=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Number of biod daemons on mount
			OLD_NUM_BIODS=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Is suid execution allowed         
			OLD_SUID=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Is device access allowed
			OLD_DEV=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	# Does server support long devnos
			OLD_SHORTDEV=$1
		fi
		shift
		if [ "$1" != "_null" ] ; then	#  Automount value
			OLD_AUTOM=$1
		fi
		shift
	fi
fi

# Find out if a mount type name was specified and if it was not then
# check to see if there is an old one that should be used.
if [ -z "${MOUNT_TYPE_NAME}" -a -n "${OLD_MOUNT_TYPE_NAME}" ] ; then
	MOUNT_TYPE_NAME="${OLD_MOUNT_TYPE_NAME}"
fi

# Set up the defaults for the user if they did not specify these.
TYPE_READ_WRITE=${TYPE_READ_WRITE:-"${OLD_TYPE_READ_WRITE:-"rw"}"}
if [ "${TYPE_READ_WRITE}" != "rw" -a "${TYPE_READ_WRITE}" != "ro" ] ; then
    dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME}
    exit 1
fi

# Check the secure option to see if there is an old specification that
# should be used.
if [ -z "$SECURE" -a -n "$OLD_SECURE" ] ; then
	SECURE=$OLD_SECURE
fi
SECURE=${SECURE:-"no"}

# Check the Automount option to see if there is an old value that
# should be used

if [ -z "$AUTOM" -a -n "$OLD_AUTOM" ] ; then
	AUTOM=$OLD_AUTOM
fi
AUTOM=${AUTOM:-"no"}

# Check the nosuid, nodev, and shortdev options for old values and set defaults

if [ -z "$DEV" -a -n "$OLD_DEV" ] ; then
	DEV=$OLD_DEV
fi
DEV=${DEV:-"yes"}

if [ -z "$SHORTDEV" -a -n "$OLD_SHORTDEV" ] ; then
	SHORTDEV=$OLD_SHORTDEV
fi
SHORTDEV=${SHORTDEV:-"yes"}

if [ -z "$SUID" -a -n "$OLD_SUID" ] ; then
	SUID=$OLD_SUID
fi
SUID=${SUID:-"yes"}

WHEN=${WHEN:-"B"}
# Check all parameters that are supposed to be numbers
if [ -n "${RETRY_TIMES}" ] ; then
    NUM_CHECK=`{ echo "${RETRY_TIMES}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi
if [ -n "${BUFSIZ_READ}" ] ; then
    NUM_CHECK=`{ echo "${BUFSIZ_READ}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE"  ${COMMAND_NAME}; exit 1 ; fi
fi
if [ -n "${BUFSIZ_WRITE}" ] ; then
    NUM_CHECK=`{ echo "${BUFSIZ_WRITE}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi
if [ -n "${TIMEOUT}" ] ; then
    NUM_CHECK=`{ echo "${TIMEOUT}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi
if [ -n "${INET_PORT}" ] ; then
    NUM_CHECK=`{ echo "${INET_PORT}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi
if [ -n "${ACREGMIN}" ] ; then
    NUM_CHECK=`{ echo "${ACREGMIN}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi
if [ -n "${ACREGMAX}" ] ; then
    NUM_CHECK=`{ echo "${ACREGMAX}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi
if [ -n "${ACDIRMIN}" ] ; then
    NUM_CHECK=`{ echo "${ACDIRMIN}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi
if [ -n "${ACDIRMAX}" ] ; then
    NUM_CHECK=`{ echo "${ACDIRMAX}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi
if [ -n "${ACTIMEO}" ] ; then
    NUM_CHECK=`{ echo "${ACTIMEO}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi
if [ -n "${NUM_BIODS}" ] ; then
    NUM_CHECK=`{ echo "${NUM_BIODS}"|/usr/bin/tr -s '[0-9]' '[#*]' ; }`
    if [ "${NUM_CHECK}" != "#" -a "x${NUM_CHECK}" != "x" ] ; then dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
fi

# Finished with the parsing.  Build the options string.

# If specified read-only, put it in the options list.  otherwise, ignore.
if [ "${TYPE_READ_WRITE}" = "ro" ] ; then
    OPTIONS="${OPTIONS}${OPTIONS:+","}${TYPE_READ_WRITE}"
fi

# if WHERE_MOUNT is set then add its value to the OPTIONS list
if [ -z "$WHERE_MOUNT" -a -n "$OLD_WHERE_MOUNT" ] ; then
	WHERE_MOUNT=$OLD_WHERE_MOUNT
fi
if [ -n "$WHERE_MOUNT" ] ; then
	if [ "$WHERE_MOUNT" != "fg" -a "$WHERE_MOUNT" != "bg" ] ; then
		dspmsg cmdnfs.cat -s 37 39 "$USAGE" ${COMMAND_NAME} ; exit 1
	fi
fi

OPTIONS="${OPTIONS}${WHERE_MOUNT:+"${OPTIONS:+","}${WHERE_MOUNT}"}"

# if SOFT_HARD is set then its value to the OPTIONS list
# Check to see if there was an old specification that should be used.
if [ -z "$SOFT_HARD" -a -n "$OLD_SOFT_HARD" ] ; then
	SOFT_HARD=$OLD_SOFT_HARD
fi
OPTIONS="${OPTIONS}${SOFT_HARD:+"${OPTIONS:+","}${SOFT_HARD}"}"

# if the secure option is set to "yes" add it to the list
if [ "${SECURE}" = "yes" ] ; then
    OPTIONS="${OPTIONS}${OPTIONS:+","}secure"
fi

# check to see if interrupts should be allowed on hard mounts
if [ -z "${INTR}" ] ; then
    if [ -n "${OLD_INTR}" ] ; then
	INTR=$OLD_INTR
    else		# default to interruptable
	INTR="yes"
    fi
fi
if [ "${INTR}" = "yes" ] ; then
    OPTIONS="${OPTIONS}${OPTIONS:+","}intr"
fi

# RETRY specified? add it to the list
if [ -z "${RETRY_TIMES}" -o "x${RETRY_TIMES}" = "x" -a -n "${OLD_RETRY_TIMES}" ] ; then
	RETRY_TIMES=$OLD_RETRY_TIMES
fi
OPTIONS="${OPTIONS}${RETRY_TIMES:+"${OPTIONS:+","}retry=${RETRY_TIMES}"}"

# Buffer size for reads specified? add it to the list
if [ -z "${BUFSIZ_READ}" -o "x${BUFSIZ_READ}" = "x" -a -n "${OLD_BUFSIZ_READ}" ] ; then
	BUFSIZ_READ=$OLD_BUFSIZ_READ
fi
OPTIONS="${OPTIONS}${BUFSIZ_READ:+"${OPTIONS:+","}rsize=${BUFSIZ_READ}"}"

# Buffer size for writes specified? add it to the list
if [ -z "${BUFSIZ_WRITE}" -o "x${BUFSIZ_WRITE}" = "x" -a -n "${OLD_BUFSIZ_WRITE}" ] ; then
	BUFSIZ_WRITE=$OLD_BUFSIZ_WRITE
fi
OPTIONS="${OPTIONS}${BUFSIZ_WRITE:+"${OPTIONS:+","}wsize=${BUFSIZ_WRITE}"}"

# Timeout specified? Add it to the list
if [ -z "$TIMEOUT" -o "x$TIMEOUT" = "x" -a -n "$OLD_TIMEOUT" ] ; then
	TIMEOUT=$OLD_TIMEOUT
fi
OPTIONS="${OPTIONS}${TIMEOUT:+"${OPTIONS:+","}timeo=${TIMEOUT}"}"

# Internet port number specified?
if [ -z "$INET_PORT" -o "x$INET_PORT" = "x" -a -n "$OLD_INET_PORT" ] ; then
	INET_PORT=$OLD_INET_PORT
fi
OPTIONS="${OPTIONS}${INET_PORT:+"${OPTIONS:+","}port=${INET_PORT}"}"

# Check for all of the cache attributes
if [ -z "$ACREGMIN" -o "x$ACREGMIN" = "x" -a -n "$OLD_ACREGMIN" ] ; then
	ACREGMIN=$OLD_ACREGMIN
fi
OPTIONS="${OPTIONS}${ACREGMIN:+"${OPTIONS:+","}acregmin=${ACREGMIN}"}"

if [ -z "$ACREGMAX" -o "x$ACREGMAX" = "x" -a -n "$OLD_ACREGMAX" ] ; then
	ACREGMAX=$OLD_ACREGMAX
fi
OPTIONS="${OPTIONS}${ACREGMAX:+"${OPTIONS:+","}acregmax=${ACREGMAX}"}"

if [ -z "$ACDIRMIN" -o "x$ACDIRMIN" = "x" -a -n "$OLD_ACDIRMIN" ] ; then
	ACDIRMIN=$OLD_ACDIRMIN
fi
OPTIONS="${OPTIONS}${ACDIRMIN:+"${OPTIONS:+","}acdirmin=${ACDIRMIN}"}"

if [ -z "$ACDIRMAX" -o "x$ACDIRMAX" = "x" -a -n "$OLD_ACDIRMAX" ] ; then
	ACDIRMAX=$OLD_ACDIRMAX
fi
OPTIONS="${OPTIONS}${ACDIRMAX:+"${OPTIONS:+","}acdirmax=${ACDIRMAX}"}"

if [ -z "$ACTIMEO" -o "x$ACTIMEO" = "x" -a -n "$OLD_ACTIMEO" ]; then
	ACTIMEO=$OLD_ACTIMEO
fi
OPTIONS="${OPTIONS}${ACTIMEO:+"${OPTIONS:+","}actimeo=${ACTIMEO}"}"

if [ -z "$NUM_BIODS" -o "$NUM_BIODS" = "x" -a -n "$OLD_NUM_BIODS" ] ; then
	NUM_BIODS=$OLD_NUM_BIODS
fi
OPTIONS="${OPTIONS}${NUM_BIODS:+"${OPTIONS:+","}biods=${NUM_BIODS}"}"

if [ "$SHORTDEV" = "no" ] ; then 
   OPTIONS="${OPTIONS}${SHORTDEV:+"${OPTIONS:+","}shortdev"}"
fi
if [ "$DEV" = "no" ] ; then 
   OPTIONS="${OPTIONS}${DEV:+"${OPTIONS:+","}nodev"}"
fi
if [ "$SUID" = "no" ] ; then 
   OPTIONS="${OPTIONS}${SUID:+"${OPTIONS:+","}nosuid"}"
fi



# Done with the parsing and sanity checking.  Move onto actually
# adding mount entry in /etc/filesystems (if we have to) and 
# executing the mount command (if we have to)

if [ ${COMMAND_NAME} = "mknfsmnt" ] ; then
    if [ ${WHEN} = "N" ] ; then
	# execute the mount command with the correct parameters
	OPTIONS=${OPTIONS:+"-o ${OPTIONS}"}
	${MOUNT_CMD} -v nfs -n ${REMOTE_HOST} ${OPTIONS} ${REMOTE_DIRECTORY} ${PATH_NAME}
	exit $?		# return the return value from mount
    fi
    if [ ${WHEN} = "I" -o ${WHEN} = "B" ] ; then
	MOUNT_TYPE_NAME=${MOUNT_TYPE_NAME:+"-u ${MOUNT_TYPE_NAME}"}
	OPTIONS=${OPTIONS:-"bg"}
	REMOTE_DIRECTORY=`echo "${REMOTE_DIRECTORY}"| sed s/'\\\\'/'\\\\\\\\'/g`
	if [ "${AUTOM}" = "yes" ] ; then 
	    YESSTR=${AUTOM}
	    ${CRFS_CMD} -v nfs -m ${PATH_NAME} -d \"${REMOTE_DIRECTORY}\" \
		        -n ${REMOTE_HOST} ${MOUNT_TYPE_NAME}\
		        -A ${YESSTR} -a options=${OPTIONS}
        else
	    NOSTR=${AUTOM}
	    ${CRFS_CMD} -v nfs -m ${PATH_NAME} -d \"${REMOTE_DIRECTORY}\" \
		        -n ${REMOTE_HOST} ${MOUNT_TYPE_NAME}\
		        -A ${NOSTR} -a options=${OPTIONS}
        fi
           
	# Check to make sure that we created the file without failure.
	if [ $? != 0 ] ; then
		exit 1
	fi

	# Check to see if the directory is to be mounted now.
	if [ "${WHEN}" = "B" ] ; then
	    ${MOUNT_CMD} ${PATH_NAME}
	    exit $?
	fi
    fi
else		# if chnfsmnt was invoked
    if [ ${WHEN} = "I" -o ${WHEN} = "B" ] ; then
	MOUNT_TYPE_NAME=${MOUNT_TYPE_NAME:+"-u ${MOUNT_TYPE_NAME}"} 
	OPTIONS=${OPTIONS:+"-a options=${OPTIONS}"}
	REMOTE_DIRECTORY=`echo "${REMOTE_DIRECTORY}"| sed s/'\\\\'/'\\\\\\\\'/g`
	if [ "${AUTOM}" = "yes" ] ; then 
	    YESSTR=${AUTOM}
	    ${CHFS_CMD} -n ${REMOTE_HOST} ${MOUNT_TYPE_NAME} \
		        -A ${YESSTR} -a dev=\"${REMOTE_DIRECTORY}\" ${OPTIONS} \
		        ${PATH_NAME}
        else
	    NOSTR=${AUTOM}
	    ${CHFS_CMD} -n ${REMOTE_HOST} ${MOUNT_TYPE_NAME} \
		        -A ${NOSTR} -a dev=\"${REMOTE_DIRECTORY}\" ${OPTIONS} \
		        ${PATH_NAME}
        fi
	# Check to make sure our entry was updated correctly
	if [ $? != 0 ] ; then
	    exit 1
	fi 

    fi

    # Check to see if the directory is to be exported now.
    if [ ${WHEN} = "N" -o ${WHEN} = "B" ] ; then
        # execute the mount command with the correct parameters
	# Determine if the file system is mounted currently
	${MOUNT_CMD} | /bin/awk '{print $3}' | grep "${PATH_NAME}" > /dev/null 2>&1

	if [ $? = 0 ] ; then
	    ${UNMOUNT_CMD} ${PATH_NAME}
	    if [ $? != 0 ] ; then
		dspmsg cmdnfs.cat -s 37 28 "${COMMAND_NAME}: Error in unmounting ${PATH_NAME} for change\n" ${COMMAND_NAME} ${PATH_NAME}
		exit 1
	    fi
	fi

	# Mount the filesystem with the specified options
	if [ ${WHEN} = "N" ] ; then
	    OPTIONS=${OPTIONS:+"-o ${OPTIONS}"}
	    ${MOUNT_CMD} -v nfs -n ${REMOTE_HOST} ${OPTIONS} ${REMOTE_DIRECTORY} ${PATH_NAME}
	else
	    ${MOUNT_CMD} ${PATH_NAME}
	fi
	exit $?
    fi
fi
