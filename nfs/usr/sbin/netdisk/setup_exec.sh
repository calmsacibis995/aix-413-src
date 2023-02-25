#! /bin/sh
# @(#)66        1.4  com/cmd/usr.etc/netdisk/setupexec.sh, cmdnfs, nfs320, 91453 20 3/21/90 15:28:43
#
# COMPONENT_NAME: (CMDNFS) Network File System Commands
#
# FUNCTIONS:
#
# ORIGINS: 24 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#       Copyright (c) 1987 by Sun Microsystems, Inc.
#
#  setup_exec : script written to set up exec on a server.
#
#  usage: setup_exec archname path tapetype tapedev tapehost
#
#	@(#)setup_exec	1.3 90/07/25 4.1NFSSRC; from 1.29 88/03/11 D/NFS
#
#  	Copyright (c) 1987 by Sun Microsystems, Inc.
#

HOME=/; export HOME
PATH=/bin:/usr/bin:/etc:/usr/sbin:/usr/ucb:/usr/lpp/nfs/sun_diskless

CMDNAME=$0
MYPATH="/usr/lpp/nfs/sun_diskless"
DOVERIFY=yes

USAGE="usage: ${CMDNAME} arch execpath tapetype tapehost tapedev
where:
	arch        = \"sun2\" or \"sun3\" ....
        execpath    = pathname of the directory where exec exists
        tapetype    = \"local\" or \"remote\"
	tapehost    = name of the host which has the tape drive
        tapedev     = \"ar\" or \"st\" or \"mt\" or \"xt\"
"

#
# Verify number of arguments
#
if [ $# -lt 4 -o $# -gt 5 ]; then
	echo "${CMDNAME}: incorrect number of arguments."
        echo "${USAGE}"
        exit 1
fi
#
# miniroot or multiuser mode
#
if [ -f /.MINIROOT ]; then
	WHERE="/a"
else
	WHERE=""
fi
#
# Verify architecture name
#
ARCH=${1}; shift
case "$ARCH" in
"sun2" | "sun3" | "sun4"  | "sun4c")
	break ;;
* )
	echo "${CMDNAME}: invalid architecture type \"${ARCH}\"."
	exit 1 ;;
esac
#
# Path name for exec/arch
#
EXECPATH=${1}; shift
#
# Check tape drive type : local or remote
# If remote, check tape host and ethernet type : ec, ie or le
#
DRIVE=${1}; shift
case "$DRIVE" in
"local" )
	break ;;
"remote" )
	TAPEHOST=${1}; shift
	REMOTE="rsh $TAPEHOST -n"
	break ;;
* )
	echo "${CMDNAME}: invalid tape drive type \"${DRIVE}\"."
	exit 1 ;;
esac
#
# Check tape type : ar, st, mt or xt
#
TAPE=${1}
case "$TAPE" in
"ar" | "st" )
	BS=126
        TAPE=/dev/nr${TAPE}8
	break ;;
"mt" | "xt" )
	BS=20
        TAPE=/dev/nrmt8
	break ;;
/dev/* )
	BS=20
        # TAPE = full path i.e. /dev/rmt0.1
        break ;;
* )
	if [ -n "${2}" ] ; then
		TAPEHOST=$TAPE
		CDFS_PATH=$TAPEHOST
		TAPE=${2}
	else
		echo "${CMDNAME}: invalid tape type \"${TAPE}\"."
		exit 1
	fi ;;
esac
#
# Preliminary work before installation starts
#

if [ "$DRIVE" = "remote" ]; then
	DOMAIN=`domainname`
        if [ "$DOMAIN" = "noname" ]; then
                host $TAPEHOST > /dev/null 2>&1
        else
                /usr/bin/ypmatch $TAPEHOST hosts > /dev/null 2>&1
        fi
	if [ "$?" != 0 ]; then
		ping $TAPEHOST 1 1 1>/dev/null 2>&1
		if [ $? -eq 1 ] ; then
                	echo "${CMDNAME}: can't reach tapehost \"${TAPEHOST}\" "
                	exit 1
		fi

        fi
fi
#
# Determine optional software
#
if [ ! -f /tmp/EXTRACTLIST.${ARCH} ]; then
	opt_software ${ARCH}
fi
EXTRACTLIST=""
if [ -f /tmp/EXTRACTLIST.${ARCH} ]; then
#MONTE
	grep ENTRY /tmp/EXTRACTLIST.${ARCH} | awk '{ print $2}' >>/tmp/EXTRACTLIST${ARCH}
	for OPT in `cat /tmp/EXTRACTLIST${ARCH}`; do
		if [ $OPT != use -a $OPT != root ]; then
			EXTRACTLIST="${EXTRACTLIST} ${OPT}"
		fi
	done
#	rm -f /tmp/EXTRACTLIST.${ARCH}
fi

echo
echo "Installation of ${ARCH} executable files begins :"
#
# Extract sys and usr from release tape 
#
mkdir ${WHERE}${EXECPATH} ${WHERE}${EXECPATH}/${ARCH} 2>/dev/null
cd ${WHERE}${EXECPATH}/${ARCH}
if [ ! -f /tmp/TOC ]; then
	if [ -z "$CDFS_PATH" ] ; then
		verify_tapevol_arch ${ARCH} 1 ${TAPE} ${TAPEHOST}
	fi
fi
#
# Extract software from release tape
# We go through EXTRACTLIST three times to make sure 'Sys'
# is loaded first, and 'usr' second, if they are being 
# loaded
#
DONEONCE=no
STATE=IDENT
LASTTAPE=none
LASTFILE=0
for OPT in `cat /tmp/EXTRACTLIST.${ARCH}`; do
	# Make sure we are in state ENTRY if $OPT is "ENTRY"
	# or state IDENT if $OPT is "IDENT"
	case $OPT in
	ENTRY)
		case $STATE in
		ENTRY)
			STATE=NAME
			continue ;;
		*)
			echo "
${CMDNAME}: /tmp/EXTRACTLIST.${ARCH} out of sync.
ATTENTION: ${ARCH} architecture not fully installed."
			exit 1 ;;
		esac ;;
	IDENT)
		case $STATE in
		IDENT)
			STATE=TITLE
			continue ;;
		*)
			echo "
${CMDNAME}: /tmp/EXTRACTLIST.${ARCH} out of sync.
ATTENTION: ${ARCH} architecture not fully installed."
			exit 1 ;;
		esac ;;
	esac
	#
	# Now assign the proper variable depending on the state.
	# If we are in states ENTRY or IDENT at this point, then we
	# are out of sync.
	#
	case $STATE in
	ENTRY | IDENT)
		echo "
${CMDNAME}: /tmp/EXTRACTLIST.${ARCH} out of sync.
ATTENTION: ${ARCH} architecture not fully installed."
		exit 1 ;;
	TITLE)
		# Distribution name
		TITLE=$OPT
		STATE=VERSION
		continue ;;
	VERSION)
		# Distribution version
		VERSION=$OPT
		STATE=ENTRY
		if [ $DONEONCE -eq no ]; then
		echo "
[Loading version ${VERSION} of ${ARCH} architecture.]"
		DONEONCE=yes
		fi
		continue ;;
	NAME)
		# Software name
		NAME=$OPT
		STATE=TAPENO
		continue ;;
	TAPENO)
		# Release tape volume number
		TAPENO=$OPT
		STATE=FILENO
		continue ;;
	FILENO)
		# File number on the tape (starting with 0)
		FILENO=$OPT
		STATE=WHERE
		continue ;;
	WHERE)
		# Pathname to load on
		WHERE=$OPT
		STATE=STATUS
		continue ;;
	STATUS)
		# "required" or "desirable" or "common" or "optional"
		STATUS=$OPT
		STATE=MOVABLE
		continue ;;
	MOVABLE)
		# "movable" or "not_movable"
		MOVABLE=$OPT
		STATE=SIZE
		continue ;;
	SIZE)
		# Size of this file
		SIZE=$OPT
		STATE=ENTRY
		;;
	esac
	# We have a complete entry, so process it
	case $WHERE in
	/ | /usr | appl | share)
		# go ahead and load in the EXEC directory
		LDDIR=${EXECPATH}/${ARCH}
		;;
	impl)
		# load in kvm area
		LDDIR=${EXECPATH}/kvm/${ARCH}
		make_dirs ${LDDIR} 755
		;;
	/usr/*)
		# translate pathname into EXEC directory
		LDDIR=`echo $WHERE | sed -e "s,^/usr/,,"`
		case "$LDDIR" in
		"")
			LDDIR=${EXECPATH/${ARCH}}
			;;
		*)
			LDDIR=${EXECPATH}/${ARCH}/${LDDIR}
			make_dirs ${LDDIR} 755
			;;
		esac ;;
	*)
		#
		# Try to relocate the software into the
		# ARCH directory if we can
		#
		case $MOVABLE in
		movable)
			LDDIR=${EXECPATH}/${ARCH}
			;;
		*)
			echo "
${CMDNAME}: Can't relocate loadpoint directory \"$WHERE\"
into ${EXECPATH}.
ATTENTION: $STATUS \"$NAME\" software not installed."
			continue ;;
		esac ;;
	esac
	if [ ! -d $LDDIR ]; then
		echo "
${CMDNAME}: Directory \"${LDDIR}\" was not created.
ATTENTION: $STATUS \"$NAME\" software not installed."
		continue
	fi
	cd ${LDDIR}
	#
	# Only verify the tape if the current tape volume is different than
	# the previous one, or if the current file number is not greater
	# than the last one.
	# Also, always do verify if DOVERIFY is anything but "no" (see comments
	# at top of file).
	#
	case $NAME in
	Sys )
		if [ $DOVERIFY != no -o $TAPENO != $LASTTAPE -o $FILENO -lt $LASTFILE ]
		then
			if [ -z "$CDFS_PATH" ] ; then
			verify_tapevol_arch ${ARCH} ${TAPENO} ${TAPE} ${TAPEHOST}
			fi
			SKIP=$FILENO
		else
			# Same tape, so just skip to the correct file
			SKIP=`expr $FILENO - $LASTFILE`
		fi

		if [ -z "$CDFS_PATH" ] ; then
		${MYPATH}/extracting ${TAPE} ${SKIP} ${BS} ${NAME} ${TAPEHOST}
		else
		NAME=`echo $NAME | /usr/ucb/tr A-Z a-z`
		${MYPATH}/extracting ${TAPE} ${NAME} ${TAPEHOST} ${WHERE} ${ARCH}
		fi
		LASTTAPE=$TAPENO
#		Since we loaded, i.e advanced the tape, it is now
#		at the next FINENO position on the tape
		LASTFILE=`expr $FILENO + 1`
		;;
	*)
		;;
	esac
done
STATE=IDENT
LASTTAPE=none
LASTFILE=0
for OPT in `cat /tmp/EXTRACTLIST.${ARCH}`; do
	# Make sure we are in state ENTRY if $OPT is "ENTRY"
	# or state IDENT if $OPT is "IDENT"
	case $OPT in
	ENTRY)
		case $STATE in
		ENTRY)
			STATE=NAME
			continue ;;
		*)
			echo "
${CMDNAME}: /tmp/EXTRACTLIST.${ARCH} out of sync.
ATTENTION: ${ARCH} architecture not fully installed."
			exit 1 ;;
		esac ;;
	IDENT)
		case $STATE in
		IDENT)
			STATE=TITLE
			continue ;;
		*)
			echo "
${CMDNAME}: /tmp/EXTRACTLIST.${ARCH} out of sync.
ATTENTION: ${ARCH} architecture not fully installed."
			exit 1 ;;
		esac ;;
	esac
	#
	# Now assign the proper variable depending on the state.
	# If we are in states ENTRY or IDENT at this point, then we
	# are out of sync.
	#
	case $STATE in
# This will not be needed for now
#	ENTRY | IDENT)
#		echo "
#${CMDNAME}: /tmp/EXTRACTLIST.${ARCH} out of sync.
#ATTENTION: ${ARCH} architecture not fully installed."
#		exit 1 ;;
	TITLE)
		# Distribution name
		TITLE=$OPT
		STATE=VERSION
		continue ;;
	VERSION)
		# Distribution version
		VERSION=$OPT
		STATE=ENTRY
		if [ "$DONEONCE" -eq "no" ]; then
		echo "
[Loading version ${VERSION} of ${ARCH} architecture.]"
		DONEONCE=yes
		fi
		continue ;;
	NAME)
		# Software name
		NAME=$OPT
		STATE=TAPENO
		continue ;;
	TAPENO)
		# Release tape volume number
		TAPENO=$OPT
		STATE=FILENO
		continue ;;
	FILENO)
		# File number on the tape (starting with 0)
		FILENO=$OPT
		STATE=WHERE
		continue ;;
	WHERE)
		# Pathname to load on
		WHERE=$OPT
		STATE=STATUS
		continue ;;
	STATUS)
		# "required" or "desirable" or "common" or "optional"
		STATUS=$OPT
		STATE=MOVABLE
		continue ;;
	MOVABLE)
		# "movable" or "not_movable"
		MOVABLE=$OPT
		STATE=SIZE
		continue ;;
	SIZE)
		# Size of this file
		SIZE=$OPT
		STATE=ENTRY
		;;
        *)
                continue;;   # related to the commenting out of ENTRY|IDENT
	esac
	# We have a complete entry, so process it
	case $WHERE in
	/ | /usr | appl | share)
		# go ahead and load in the EXEC directory
		LDDIR=${EXECPATH}/${ARCH}
		;;
	impl)
		# load in kvm area
		LDDIR=${EXECPATH}/kvm/${ARCH}
		make_dirs ${LDDIR} 755
		;;
	/usr/*)
		# translate pathname into EXEC directory
		LDDIR=`echo $WHERE | sed -e "s,^/usr/,,"`
		case "$LDDIR" in
		"")
			LDDIR=${EXECPATH/${ARCH}}
			;;
		*)
			LDDIR=${EXECPATH}/${ARCH}/${LDDIR}
			make_dirs ${LDDIR} 755
			;;
		esac ;;
	*)
		#
		# Try to relocate the software into the
		# ARCH directory if we can
		#
		case $MOVABLE in
		movable)
			LDDIR=${EXECPATH}/${ARCH}
			;;
		*)
			echo "
${CMDNAME}: Can't relocate loadpoint directory \"$WHERE\"
into ${EXECPATH}.
ATTENTION: $STATUS \"$NAME\" software not installed."
			continue ;;
		esac ;;
	esac
	if [ ! -d $LDDIR ]; then
		echo "
${CMDNAME}: Directory \"${LDDIR}\" was not created.
ATTENTION: $STATUS \"$NAME\" software not installed."
		continue
	fi
	cd ${LDDIR}
	#
	# Only verify the tape if the current tape volume is different than
	# the previous one, or if the current file number is not greater
	# than the last one.
	# Also, always do verify if DOVERIFY is anything but "no" (see comments
	# at top of file).
	#
#MONTE
	case $NAME in
	usr )
		if [ $DOVERIFY != no -o $TAPENO != $LASTTAPE -o $FILENO -lt $LASTFILE ]
		then
			if [ -z "$CDFS_PATH" ] ; then
			verify_tapevol_arch ${ARCH} ${TAPENO} ${TAPE} ${TAPEHOST}
			fi
			SKIP=$FILENO
		else
			# Same tape, so just skip to the correct file
			SKIP=`expr $FILENO - $LASTFILE`
		fi
#MONTE
		if [ -z "$CDFS_PATH" ] ; then
		${MYPATH}/extracting ${TAPE} ${SKIP} ${BS} ${NAME} ${TAPEHOST}
		else
		NAME=`echo $NAME | /usr/ucb/tr A-Z a-z`
		${MYPATH}/extracting ${TAPE} ${NAME} ${TAPEHOST} ${WHERE} ${ARCH}
		fi
		LASTTAPE=$TAPENO
		LASTFILE=`expr $FILENO + 1`
		;;
	*)
		;;
	esac
done
STATE=IDENT
LASTTAPE=none
LASTFILE=0
for OPT in `cat /tmp/EXTRACTLIST.${ARCH}`; do
	# Make sure we are in state ENTRY if $OPT is "ENTRY"
	# or state IDENT if $OPT is "IDENT"
	case $OPT in
	ENTRY)
		case $STATE in
		ENTRY)
			STATE=NAME
			continue ;;
		*)
			echo "
${CMDNAME}: /tmp/EXTRACTLIST.${ARCH} out of sync.
ATTENTION: ${ARCH} architecture not fully installed."
			exit 1 ;;
		esac ;;
	IDENT)
		case $STATE in
		IDENT)
			STATE=TITLE
			continue ;;
		*)
			echo "
${CMDNAME}: /tmp/EXTRACTLIST.${ARCH} out of sync.
ATTENTION: ${ARCH} architecture not fully installed."
			exit 1 ;;
		esac ;;
	esac
	#
	# Now assign the proper variable depending on the state.
	# If we are in states ENTRY or IDENT at this point, then we
	# are out of sync.
	#
	case $STATE in
	ENTRY | IDENT)
		echo "
${CMDNAME}: /tmp/EXTRACTLIST.${ARCH} out of sync.
ATTENTION: ${ARCH} architecture not fully installed."
		exit 1 ;;
	TITLE)
		# Distribution name
		TITLE=$OPT
		STATE=VERSION
		continue ;;
	VERSION)
		# Distribution version
		VERSION=$OPT
		STATE=ENTRY
		if [ "$DONEONCE" -eq "no" ]; then
		echo "
[Loading version ${VERSION} of ${ARCH} architecture.]"
		DONEONCE=yes
		fi
		continue ;;
	NAME)
		# Software name
		NAME=$OPT
		STATE=TAPENO
		continue ;;
	TAPENO)
		# Release tape volume number
		TAPENO=$OPT
		STATE=FILENO
		continue ;;
	FILENO)
		# File number on the tape (starting with 0)
		FILENO=$OPT
		STATE=WHERE
		continue ;;
	WHERE)
		# Pathname to load on
		WHERE=$OPT
		STATE=STATUS
		continue ;;
	STATUS)
		# "required" or "desirable" or "common" or "optional"
		STATUS=$OPT
		STATE=MOVABLE
		continue ;;
	MOVABLE)
		# "movable" or "not_movable"
		MOVABLE=$OPT
		STATE=SIZE
		continue ;;
	SIZE)
		# Size of this file
		SIZE=$OPT
		STATE=ENTRY
		;;
	esac
	# We have a complete entry, so process it
	case $WHERE in
	/ | /usr | appl | share)
		# go ahead and load in the EXEC directory
		LDDIR=${EXECPATH}/${ARCH}
		;;
	impl)
		# load in kvm area
		LDDIR=${EXECPATH}/kvm/${ARCH}
		make_dirs ${LDDIR} 755
		;;
	/usr/*)
		# translate pathname into EXEC directory
		LDDIR=`echo $WHERE | sed -e "s,^/usr/,,"`
		case "$LDDIR" in
		"")
			LDDIR=${EXECPATH/${ARCH}}
			;;
		*)
			LDDIR=${EXECPATH}/${ARCH}/${LDDIR}
			make_dirs ${LDDIR} 755
			;;
		esac ;;
	*)
		#
		# Try to relocate the software into the
		# ARCH directory if we can
		#
		case $MOVABLE in
		movable)
			LDDIR=${EXECPATH}/${ARCH}
			;;
		*)
			echo "
${CMDNAME}: Can't relocate loadpoint directory \"$WHERE\"
into ${EXECPATH}.
ATTENTION: $STATUS \"$NAME\" software not installed."
			continue ;;
		esac ;;
	esac
	if [ ! -d $LDDIR ]; then
		echo "
${CMDNAME}: Directory \"${LDDIR}\" was not created.
ATTENTION: $STATUS \"$NAME\" software not installed."
		continue
	fi
	cd ${LDDIR}
	#
	# Only verify the tape if the current tape volume is different than
	# the previous one, or if the current file number is not greater
	# than the last one.
	# Also, always do verify if DOVERIFY is anything but "no" (see comments
	# at top of file).
	#
	case $NAME in
	Sys | usr )
		;;
	* )
		if [ $DOVERIFY != no -o $TAPENO != $LASTTAPE -o $FILENO -lt $LASTFILE ]
		then
			if [ -z "$CDFS_PATH" ] ; then
			verify_tapevol_arch ${ARCH} ${TAPENO} ${TAPE} ${TAPEHOST}
			fi
			SKIP=$FILENO
		else
			# Same tape, so just skip to the correct file
			SKIP=`expr $FILENO - $LASTFILE`
		fi
		case $NAME in 
		Kvm )
			make_dirs ${LDDIR}/boot 775
			make_dirs ${LDDIR}/mdec 775
			make_dirs ${LDDIR}/stand 775
			make_dirs ${EXECPATH}/${ARCH}/kvm 775
			;;
		root )
		#
		# Only install the root prototype if it doesn't
		# already exist.
		#
		if [ ! -d $MYPATH/proto/etc ]; then
			LDDIR=${MYPATH}/proto
			rm -rf ${LDDIR}
			mkdir ${LDDIR}
			chmod 755 ${LDDIR}
			cd ${LDDIR}
			echo
			echo "Loading prototype root tree..."
		else
                    if [ -f /tmp/UPGRADE ]; then
		    	LDDIR=${MYPATH}/proto
			make_dirs ${LDDIR} 775
			cd ${LDDIR}
			echo
			echo "Upgrading prototype root tree..."
		    else
			# already exists, so skip it
			echo "root prototype being used as it exists"
			continue
		    fi
		fi ;;
		* )
			;;
		esac
		if [ -z "$CDFS_PATH" ] ; then
		${MYPATH}/extracting ${TAPE} ${SKIP} ${BS} ${NAME} ${TAPEHOST}
		else
		NAME=`echo $NAME | /usr/ucb/tr A-Z a-z`
		${MYPATH}/extracting ${TAPE} ${NAME} ${TAPEHOST} ${WHERE} ${ARCH}
		fi
		LASTTAPE=$TAPENO
		LASTFILE=`expr $FILENO + 1`
		;;
	esac
done
rm -f /tmp/EXTRACTLIST.${ARCH}
#
# Update exports on server
#
grep ${WHERE}${EXECPATH}/$ARCH ${WHERE}/etc/exports >/dev/null 2>&1
if [ "$?" = 1 ]; then
	echo
	echo "Updating /etc/exports."
	echo "#" >> ${WHERE}/etc/exports
	echo "${WHERE}${EXECPATH}/$ARCH" >> ${WHERE}/etc/exports
	if [ -f ${WHERE}/usr/sbin/exportfs ]; then
		${WHERE}/usr/sbin/exportfs -a
		if [ "$?" != 0 ]; then
			echo
			echo "ATTENTION: /etc/exports needs attention !"
			echo "ATTENTION: fix /etc/exports and rerun exportfs !"
		fi
	else
		echo
		echo "ATTENTION: /usr/sbin/exportfs does not exist !"
	fi
fi

echo
echo "Installation of ${ARCH} executable files completed."
exit 0
