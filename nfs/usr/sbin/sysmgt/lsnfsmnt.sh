#!/bin/bsh
# @(#)96	1.16  src/nfs/usr/sbin/sysmgt/lsnfsmnt.sh, cmdnfs, nfs411, GOLD410 6/14/94 08:29:22
#
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

# Variables used just within this script
TMP_FILE=/tmp/lsnfsmnt.$$
TMP_FILE_2=/tmp/lsnfsmnt_2.$$
COMMAND_NAME=`basename $0`
TR=/usr/bin/tr
LSFS=/usr/sbin/lsfs
EGREP=/bin/egrep
AWK=/bin/awk

USAGE="usage: ${COMMAND_NAME}  [-c | -l | -p ] [directory]\n"

set -- `getopt plc $*  2>/dev/null`

if [ $? != 0 ] ; then
	dspmsg cmdnfs.cat -s 37 4 "${USAGE}" $COMMAND_NAME
	exit 1
fi

# Function that will take an "lsfs -c" output and convert colons to
# pipes except where a dos like pathname exists (ie: Device=c:\games\)
local_lsfs()
{
	if [ -n "${DIRECTORY}" ]; then
	  ${LSFS} -c "${DIRECTORY}" | ${AWK} 'BEGIN {FS=":"}
	  {
	    if (NF == 10)
	      {printf "%s|%s:%s|%s|%s|%s|%s|%s|%s\n",$1,$2,$3,$4,$5,$6,$7,$8,$9}
	    else
	      {printf "%s|%s|%s|%s|%s|%s|%s|%s\n",$1,$2,$3,$4,$5,$6,$7,$8}
	  }'
	else
	  ${LSFS} -c | ${AWK} 'BEGIN {FS=":"}
	  {
	    if (NF == 10)
	      {printf "%s|%s:%s|%s|%s|%s|%s|%s|%s\n",$1,$2,$3,$4,$5,$6,$7,$8,$9}
	    else
	      {printf "%s|%s|%s|%s|%s|%s|%s|%s\n",$1,$2,$3,$4,$5,$6,$7,$8}
	  }'
	fi
}

# Find out how we are supposed to print the information to the user.

while [ $1 != -- ]
do
    case "$1" in
	"-p")			#
	    if [ -n "${FORMAT}" ] ; then dspmsg cmdnfs.cat -s 37 4 "$USAGE" $COMMAND_NAME; exit 1 ; fi
	    FORMAT="pipe"
	    FSEP="|"
	    shift
	    ;;
	"-c")			#
	    if [ -n "${FORMAT}" ] ; then dspmsg cmdnfs.cat -s 37 4 "$USAGE" $COMMAND_NAME; exit 1 ; fi
	    FORMAT="colon"
	    FSEP=":"
	    shift
	    ;;
	"-l")			#
	    if [ -n "${FORMAT}" ] ; then dspmsg cmdnfs.cat -s 37 4 "$USAGE" $COMMAND_NAME; exit 1 ; fi
	    FORMAT="list"
	    shift
	    ;;
	*)
	    dspmsg cmdnfs.cat -s 37 4 "${USAGE}" $COMMAND_NAME
	    exit 1
	    shift
	    ;;
    esac
done

shift 		# get rid of the -- parameter
if [ $# -gt 1 ] ; then
	dspmsg cmdnfs.cat -s 37 4 "${USAGE}" $COMMAND_NAME
	exit 1
elif [ $# -eq 1 ] ; then
	DIRECTORY=$1
fi


# Find the information that we are supposed to print to the user.

if [ -n "$DIRECTORY" ] ; then
	if [ "${FORMAT}" = "colon" -o "${FORMAT}" = "pipe" ] ; then
		local_lsfs | ${AWK} 'BEGIN{ FS="|" } ; { if ($3 == "nfs" ) print ($0)}' | sed s/'\\'/\\\\\\\\/g > ${TMP_FILE} 2>/dev/null
                if [ ! -s ${TMP_FILE} ] ; then
		    /bin/rm ${TMP_FILE} > /dev/null 2>&1
		    exit 1
		fi
	else
		# There is nothing to do if the request is for list format
		
		${LSFS} -v nfs ${DIRECTORY} > ${TMP_FILE} 2> /dev/null
		if [ $? -eq 0 ] ; then
			cat -q ${TMP_FILE} | ${AWK} -v VAR=${DIRECTORY} '{ if ($3 == VAR) { print $0 }}' > ${TMP_FILE_2}
                	if [ ! -s ${TMP_FILE_2} ] ; then
				dspmsg cmdnfs.cat -s 52 4 "%s: %s not found.\n" $COMMAND_NAME ${DIRECTORY}
		    		RC=1
			else
				cat -q ${TMP_FILE} | ${AWK} '{ if (NR == 1) { print $0 }}'
				cat -q ${TMP_FILE_2}
				RC=0
			fi
		else
			dspmsg cmdnfs.cat -s 52 4 "%s: %s not found.\n" $COMMAND_NAME ${DIRECTORY}
			RC=1
		fi

		/bin/rm ${TMP_FILE} > /dev/null 2>&1
		/bin/rm ${TMP_FILE_2} > /dev/null 2>&1
		exit ${RC}

	fi
else
	if [ "${FORMAT}" = "colon" -o "${FORMAT}" = "pipe" ] ; then
		local_lsfs | ${AWK} 'BEGIN {FS="|"};{ if (NR != 1 && $3 == "nfs"q) {print $0}}' | sed s/'\\'/\\\\\\\\/g > ${TMP_FILE} 2>/dev/null
	else
		# There is nothing to do if the request is for list format
		${LSFS} | ${AWK} '{ if ((NR == 1) || ($4 == "nfs")) { print $0}}'
		exit $?
	fi
fi
# We have to do a little of our own formatting here.
# The rest of the script expects something for each of the options
# listed by the lsfs command.  This awk script fills in any null options.

${AWK} 'BEGIN {FS="|"} ; {\
	if ($1 == "") { printf "_null " }else{printf "%s ", $1} ;\
	if ($2 == "") { printf "_null " }else{printf "%s ", $2} ;\
	if ($3 == "") { printf "_null " }else{printf "%s ", $3} ;\
	if ($4 == "") { printf "_null " }else{printf "%s ", $4} ;\
	if ($5 == "") { printf "_null " }else{printf "%s ", $5} ;\
	if ($6 == "") { printf "_null " }else{printf "%s ", $6} ;\
	if ($7 == "") { printf "_null " }else{printf "%s ", $7} ;\
	if ($8 == "") { printf "_null " }else{printf "%s\n", $8} }'\
	${TMP_FILE} | sed s/'\\'/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\/g > ${TMP_FILE_2}

# Process the listed mount information so that we
# can correctly print the options for the user.
echo "#mountpoint${FSEP}remotepath${FSEP}nodename${FSEP}type${FSEP}readtype${FSEP}when${FSEP}softhard${FSEP}secure${FSEP}intr${FSEP}retry${FSEP}rsize${FSEP}wsize${FSEP}timeo${FSEP}port${FSEP}acregmin${FSEP}acregmax${FSEP}acdirmin${FSEP}acdirmax${FSEP}actimeo${FSEP}biods${FSEP}suid${FSEP}dev${FSEP}shortdev${FSEP}auto"

while [ 1 ]
do

	# Set up the defaults for the export
	mountpoint=
 	device=
	vfs=
	nodename=
	type=
	size=
	options=
	automount=

	# Get the export line that is in the temporary file.
	read inputline
	if [ $? != 0 ] ; then
		exit 0
	fi

	# set the arguments so that we can treat them like command parms
	set -- $inputline  2>/dev/null

	mountpoint=$1
 	device=$2
	vfs=$3
	nodename=$4
	if [ "$5" = "_null" ] ; then  type="" ; else type=$5 ; fi
	size=$6
	options=$7
	automount=$8
        if [ "${automount}" = "yes" ] ; then
           automount="-A"
        else
           automount="-a"
        fi
	# Defaults for all of the options that may be specified.
	read_only_read_write="rw"
	foreground_background="fg"
	soft_hard="-H"
	secure="-n"
	interrupts="-e"
	retry=
	rsize=
	wsize=
	timeo=
	port=
	acregmin=
	acregmax=
	acdirmin=
	acdirmax=
	actimeo=
	biods=
	suid="-Y"
	dev="-Z"
	shortdev="-X"

	# Parse the options list so that they may be printed correctly
	for option in `echo $options | ${TR} ',' ' '`
	do
	    case "$option" in
		rw)
			read_only_read_write=$option
			;;
		ro)
			read_only_read_write=$option
			;;
		fg)
			foreground_background=$option
			;;
		bg)
			foreground_background=$option
			;;
		soft)
			soft_hard="-S"
			;;
		hard)
			soft_hard="-H"
			;;
		secure)
			secure="-s"
			;;
		intr)
			interrupts="-E"
			;;
		retry*)
			retry=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		rsize*)
			rsize=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		wsize*)
			wsize=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		timeo*)
			timeo=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		port*)
			port=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		acregmin*)
			acregmin=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		acregmax*)
			acregmax=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		acdirmin*)
			acdirmin=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		acdirmax*)
			acdirmax=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		actimeo*)
			actimeo=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		biods*)
			biods=`echo $option | ${TR} '=' ' ' |${AWK} '{print $2}'`
			;;
		nosuid)
			suid="-y"
			;;
		nodev)
			dev="-z"
			;;
		shortdev)
			shortdev="-x"
			;;
		*)
			;;
	    esac
	done

	# SMIT-ESCape a colon if it exists in remote device name
	device=`echo $device | sed s/:/#!:/g` 

	echo "$mountpoint$FSEP$device$FSEP$nodename$FSEP$type$FSEP$read_only_read_write$FSEP$foreground_background$FSEP$soft_hard$FSEP$secure$FSEP$interrupts$FSEP$retry$FSEP$rsize$FSEP$wsize$FSEP$timeo$FSEP$port$FSEP$acregmin$FSEP$acregmax$FSEP$acdirmin$FSEP$acdirmax$FSEP$actimeo$FSEP$biods$FSEP$suid$FSEP$dev$FSEP$shortdev$FSEP$automount"

done < 	${TMP_FILE_2}

rm -rf ${TMP_FILE} > /dev/null 2>&1 
rm -rf ${TMP_FILE_2} > /dev/null 2>&1 

exit 0




