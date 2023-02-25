#!/bin/bsh
# @(#)60        1.8  src/nfs/usr/sbin/netdisk/INSTALL.sh, cmdnfs, nfs411, GOLD410 3/8/94 16:44:14
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
# INSTALL:  script to add diskless architecture support and add clients
#
#  	Copyright (c) 1987 by Sun Microsystems, Inc.
#

trap unmountall 2 3 9

unmountall () {
        if [ -n "$CDFS_PATH" ] ; then
                unmount $CDFS_PATH 2>&1 > /dev/null
                rmfs $CDFS_PATH 2>&1 > /dev/null
                rmdir $CDFS_PATH 2>&1 > /dev/null
        fi
        exit
}

HOME=/; export HOME
PATH=/bin:/usr/bin:/usr/sbin:/etc:/usr/lpp/nfs/sun_diskless:/usr/ucb

CMDNAME=$0
RELEASE="1.0"
HOSTS="/etc/hosts"
BOOTPARAMS="/etc/bootparams"
YPMASTERPATH="/var/yp"

# If $TAPE starts with /dev/ then $TAPEDEV will be that name.  Otherwise
# tape device name $TAPEDEV constructed from ${NRTAPE}${TAPE}${DEVNO}
#       NRTAPE  is the prefix (including rooted path name)
#       TAPE    is the driver type taken from input prompt
#       DEVNO   is the suffix
# The device name should be the non-rewinding tape device to use.
NRTAPE="/dev/nr"
DEVNO="0"

# Specify default values for various paths here
ROOTPATHDEF=/export/root
SWAPPATHDEF=/export/swap
DUMPPATHDEF=none
HOMEPATHDEF=/home
EXECDEF=""
SWAPDEF=""
CLIENTYPDEF=""
UPGRADE="no"
case $# in
1)
        PVAL=${1}; shift
        case $PVAL in
        "upgrade" )
                UPGRADE="yes"
                echo "UPGRADE INSTALL"
                ;;
        *)
                ;;
        esac
        ;;
*)
        ;;
esac


echo
echo "	>>>	Sun Diskless Client Installation Tool	<<<"
echo "	>>>		(for non-Sun Servers)		<<<"
echo "	>>>		     Release $RELEASE		<<<"
#
# Get system hostname and YP domain name
#
HOST=`hostname`
DOMAIN=`domainname`
#
# Specify tape drive type : local or remote
# If remote, specify tape host name
#
while true; do
        echo
        echo "Enter tape drive type ? [local | remote]: \c"
        read DRIVE;
        case "$DRIVE" in
        "local"|"l" )
		TAPEHOST=""
                break ;;
        "remote"|"r" )
		while true; do
			echo;echo "Enter host of remote drive ? \c"
			read TAPEHOST;
			case "$TAPEHOST" in
			"" )
				echo; echo "${CMDNAME}: invalid tape host."
				;;
			* )
				if [ "$DOMAIN" = "noname" -o "$DOMAIN" = "" ];
				then
					host $TAPEHOST > /dev/null
				else
					ypmatch $TAPEHOST hosts >/dev/null
				fi
				case $? in
				1)
					echo
					echo "${CMDNAME}: invalid tapehost."
					;;
				*)
					break ;;
				esac ;;
			esac
		done
		break ;;
	* )
		echo "${CMDNAME}: invalid tape drive type \"${DRIVE}\"." ;;
	esac
done
#
# Specify tape type : ar, st, mt, xt or /dev/something
#
while true; do 
	echo
	echo \
"Enter tape type ? [ar[08] | st[08] | mt[08] | xt[08] | cd? | /dev/???]: \c"
	read TAPE;
	case "$TAPE" in
	ar | ar0 | nrar0 )
		TAPE=ar
		break ;;
	ar8 | nrar8 )
		TAPE=ar
		DEVNO=8
		break ;;
	st | st0 | nrst0 )
		TAPE=st
		break ;;
	st8 | nrst8 )
		TAPE=st
		DEVNO=8
		break ;;
       	/dev/* )
		break ;;
	mt | mt0 | nrmt0 | xt | xt0 | nrxt0 )
		TAPE=mt
		break ;;
	mt8 | nrmt8 | xt8 | nrxt8 )
		TAPE=mt
		DEVNO=8
		break ;;
        cd*)
                if [  $DRIVE = "r" ] || [ $DRIVE = "remote"  ] ; then
			echo " "
			echo "An install from CD-ROM on a remote host"
			echo "is not supported.  Please use an alternate"
			echo "media or local install."
			echo " "
			exit 1
		fi
                CDFS_PATH="/tmp/cdfs_$$"
                crfs -v cdrfs -p ro -d"$TAPE" -m"$CDFS_PATH" -A'no'
                if [ $? != 0 ] ; then
                        echo "${CMDNAME}: invalid device for cdrom file system"
                else
                        mount $CDFS_PATH
                        if [ $? != 0 ] ; then
                                echo "$CMDNAME: error in mounting cdrom file sys
tem"
                                rmfs $CDFS_PATH
                        else
                                TAPEHOST="$CDFS_PATH"
                                DEVNO=""
                                NRTAPE=""
                                break
                        fi
                fi
                ;;

       	* )
               	echo "${CMDNAME}: invalid tape type \"${TAPE}\"." ;;
	esac
done

#
# the statment of TAPEDEV=$TAPE was used instead
#
case $TAPE in
/dev/* )
	TAPEDEV=$TAPE
	;;
* )
	TAPEDEV=${NRTAPE}${TAPE}${DEVNO}
	;;
esac
#
# Create a list of architecture types.  Valid types are determined
# from the release tapes.
#
ARCHLIST=""
PATHLIST=""
while true; do
	while true; do
		echo "
Enter next architecture type to load
    [ sun2 | sun3 | ... | continue | done ]: \c"
		read ARCH;
		for i in ${ARCHLIST}; do
			case "$i" in
			"$ARCH")
				echo "\"${ARCH}\" already specified."
				break ;;
			esac
		done
		case "$i" in
		"$ARCH")
			;;
		*)
			break ;;
		esac
	done
	case "$ARCH" in
	"" | "execs" | "clients" | "share" | "noshare" )
		echo "${CMDNAME}: invalid architecture \"${ARCH}\"."
		;;
	"done" )
		break ;;
	"continue" | "c" )
		# continue with setup_exec with data already gathered
		ARCH=continue
		case "$ARCHLIST" in
		"")
			if [ -f /tmp/ARCHLIST ]; then
			    ARCHLIST=`cat /tmp/ARCHLIST`
			    case "$ARCHLIST" in
			    "")
				echo "No current architectures to use."
				;;
			    *)
				if [ -f /tmp/PATHLIST ]; then
				    PATHLIST=`cat /tmp/PATHLIST`
				    case "$PATHLIST" in
				    "")
					;;
				    *)
					echo "Continuing with architecture(s)\c"
					for i in $ARCHLIST
					do
					    case $i in
					    execs | clients | share | noshare )
						;;
					    * )
						echo " $i \c"
						;;
					    esac
					done
					echo ""
                                        if [ -f /tmp/UPGRADE ]; then
                                                UPGRADE="yes"
                                        fi
					break ;;
				    esac
				fi
				echo "No current pathlist to use."
				;;
			    esac
			else
			    echo "No current architectures to use."
			fi ;;
		*)
			echo "
Cannot continue after specifying additional architectures."
			;;
		esac ;;
	* )
		while true; do
			echo
			echo "Enter pathname for $ARCH executables ? \c"
			read EXEC;
			case "$EXEC" in
			"" )
				case "$EXECDEF" in
				"")
					;;
				*)
					EXEC=$EXECDEF
					echo "Using default \"$EXEC\""
					;;
				esac ;;
			esac
			case "$EXEC" in
			"" )
				;;
			*/$ARCH | */$ARCH/* )
				echo \
"Please give pathname without \"$ARCH\" at end."
				;;
			/* )
			    rm -f /tmp/*${ARCH}
			    if [ -d ${EXEC}/${ARCH}/bin ]; then
				echo "
${CMDNAME}: exec tree ${EXEC}/${ARCH} appears to already exist.
  You may select one of the following options:
	ignore	continue as if this architecture had not been specified
	remove	remove existing exec tree, then continue
	use	continue, loading any new optional software specified
        upgrade continue, loading all new files unconditionally
	clients	continue with $ARCH clients, but use existing exec tree
			? \c"
			        while true; do
				    read ans
				    case "$ans" in
				    "ignore" | "use" | "clients" | "upgrade" )
					break ;;
				    "remove" )
					echo "Removing ${EXEC}/${ARCH}..."
					rm -rf ${EXEC}/${ARCH}
					if [ -d ${EXEC}/${ARCH} ]; then
					    echo \
"  ${EXEC}/${ARCH} not removed; architecture ${ARCH} ignored"
					    ans=ignore
					else
					    ans=use
					fi
					break ;;
				    * )
					echo \
"Please enter \"ignore\" \"remove\" \"use\" \"upgrade\" or \"clients\" : \c"
					;;
				    esac
			        done
			    else
				ans=use
			    fi
			    # ans is one of ignore, use, upgrade or clients
			    case "$ans" in
			    "upgrade" | "use" )
                                if [ ${ans} = "upgrade" ]; then
                                        UPGRADE="yes"
                                fi
				STATUS=error
				opt_software ${ARCH} ${TAPEDEV} ${TAPEHOST}
				case $? in
				0 )
					ARCHLIST="${ARCHLIST} ${ARCH}"
					STATUS=ok
				esac
				case $STATUS in
				error)
					echo "
${CMDNAME}: error in opt_software; $ARCH architecture entry ignored."
					;;
				esac
				;;
			    "clients" )
				EXECDEF=$EXEC
				PATHLIST="${PATHLIST} ${EXEC}"
				ARCHLIST="${ARCHLIST} clients ${ARCH}"
				;;
			    * )
				# ignore
				echo "$ARCH architecture entry ignored."
				;;
			    esac
			    break
			    ;;
					
			* )
			    echo "Exec pathname must start with \"/\""
			    ;;
			esac
		done
		;;
	esac
done
rm -f /tmp/PATHLIST /tmp/ARCHLIST
echo "$ARCHLIST" > /tmp/ARCHLIST
echo "$PATHLIST" > /tmp/PATHLIST
if [ ${UPGRADE} = "yes" ]; then
        touch /tmp/UPGRADE
else
        rm -f /tmp/UPGRADE
fi

#
# Gather client info
# $RESET on input causes current client info to be reset and ignored
#
RESET="^"
case $ARCH in
continue)
	# already have client info.  Skip to setup_exec.
	;;
*)
    #  get client info for all ARCHSTATES (execs and clients)
    for ARCH in ${ARCHLIST}; do
	case $ARCH in
	share | noshare | execs | clients )
	    # ignore keywords
	    ;;
	* )
	    while true; do
		case "$STATE" in
		reset)
			echo ">> $CLIENT entries ignored"
			;;
		esac
		STATE=ok
		echo
		echo "Enter a $ARCH client name ? [ name | done ]: \c"
		read CLIENT;
		case "$CLIENT" in
		"" )
			;;
		"none" | "done" )
			break ;;
		* )
			#
			# Verify that host and ether entries are present
			#
			echo
			echo "Verifying ip address..."
			rm -f /tmp/ipaddr
			if [ "$DOMAIN" = "noname" -o "$DOMAIN" = "" ]; then
                                host $CLIENT | awk '{ print $3 }' > /tmp/ipaddr
			else
				ypmatch $CLIENT hosts | awk '{ print $1 }' \
				  > /tmp/ipaddr
			fi
			IPADDR=`cat /tmp/ipaddr`
			rm -f /tmp/ipaddr
			case "$IPADDR" in
			"")
				echo "
${CMDNAME}: can't find ip address in /etc/hosts for \"${CLIENT}\""
				echo
				STATE=reset
				;;
			*)
				echo "  $IPADDR $CLIENT"
				;;
			esac
			#
			# Verify ethernet address
			#
			echo
			echo "Verifying ethernet address..."
			rm -f /tmp/etheraddr
			if [ "$DOMAIN" = "noname" -o "$DOMAIN" = "" ]; then
				grep -w $CLIENT /etc/ethers | grep -v "^#" \
				  | awk '{ print $1 }' > /tmp/etheraddr
			else
				ypmatch $CLIENT ethers | awk '{ print $1 }' \
				  > /tmp/etheraddr
			fi
			ETHERADDR=`cat /tmp/etheraddr`
			rm -f /tmp/etheraddr
			case "$ETHERADDR" in
			"")
				echo "
${CMDNAME}: can't find ethernet address in /etc/ethers for \"${CLIENT}\""
				echo
				STATE=reset
				;;
			*)
				echo "  $ETHERADDR $CLIENT"
				;;
			esac

			case $STATE in
			ok)
			    while true; do
				echo
				echo \
"Enter yp type of $CLIENT ? [ master | slave | client | none ]: "
				read CLIENTYP;
				case "$CLIENTYP" in
				"")
					case "$CLIENTYPDEF" in
					"")
						;;
					*)
						CLIENTYP=$CLIENTYPDEF
						echo "Using default \"$CLIENTYP\""
						break ;;
					esac ;;
				"master" | "slave" | "client" | "none" )
					CLIENTYPDEF=$CLIENTYP
					break ;;
				"$RESET")
					STATE=reset
					break ;;
				* )
					;;
				esac
			    done ;;
			esac
			case $STATE in
			ok)
			    while true; do
				echo
				echo "Enter swap size of $CLIENT ? \c"
				read SWAP;
				case "$SWAP" in
				"$RESET")
					STATE=reset
					break ;;
				[0-9]* )
					SWAPDEF=$SWAP
					break ;;
				*)
					if [ "$SWAP" = "" -a "$SWAPDEF" != "" ]; then
						SWAP=$SWAPDEF
						echo "Using default \"$SWAP\""
						break
					fi
					echo "
Swap size must start with a digit (\"$SWAP\" is not valid)
Examples: 16M or 16m ==> 16 * 1048576 bytes
	  16K or 16k ==> 16 * 1000 bytes
	  16B or 16b ==> 16 * 512 bytes
	  16         ==> 16 bytes )"
					;;
				esac
			    done ;;
			esac
			case $STATE in
			ok)
			    while true; do
				echo
				echo "Enter root pathname of $CLIENT ? \c"
				read ROOTPATH;
				case "$ROOTPATH" in
				"" )
					case "$ROOTPATHDEF" in
					"")
						;;
					*)
						ROOTPATH=$ROOTPATHDEF
						echo "Using default \"$ROOTPATH\""
						break ;;
					esac ;;
				"$RESET")
					STATE=reset
					break ;;
				*/$CLIENT )
					echo \
"Please give pathname without \"$CLIENT\" at end."
					;;
				/* )
					ROOTPATHDEF=$ROOTPATH
					break ;;
				* )
					echo \
"Root pathname must start with \"/\""
					;;
				esac
			    done ;;
			esac
			case $STATE in
			ok)
			    while true; do
				echo
				echo  "Enter swap pathname of $CLIENT ? \c"
				read SWAPPATH;
				case "$SWAPPATH" in
				"" )
					case "$SWAPPATHDEF" in
					"")
						;;
					*)
						SWAPPATH=$SWAPPATHDEF
						echo "Using default \"$SWAPPATH\""
						break ;;
					esac ;;
				"$RESET")
					STATE=reset
					break ;;
				*/$CLIENT )
					echo \
"Please give pathname without \"$CLIENT\" at end."
					;;
				/* )
					SWAPPATHDEF=$SWAPPATH
					break ;;
				* )
					echo \
"Swap pathname must start with \"/\""
					;;
				esac
			    done ;;
			esac
			case $STATE in
			ok)
			    while true; do
				echo
				echo  "Enter dump pathname of $CLIENT (or \"none\") ? \c"
				read DUMPPATH;
				case "$DUMPPATH" in
				"" )
					case "$DUMPPATHDEF" in
					"")
						;;
					*)
						DUMPPATH=$DUMPPATHDEF
						echo "Using default \"$DUMPPATH\""
						break ;;
					esac ;;
				"$RESET")
					STATE=reset
					break ;;
				*/$CLIENT )
					echo \
"Please give pathname without \"$CLIENT\" at end."
					;;
				/* | none )
					DUMPPATHDEF=$DUMPPATH
					break ;;
				* )
					echo \
"Dump pathname must start with \"/\""
					;;
				esac
			    done ;;
			esac
			case $STATE in
			ok)
			    while true; do
				echo
				echo  "Enter home pathname of $CLIENT (or \"none\") ? \c"
				read HOMEPATH;
				case "$HOMEPATH" in
				"" )
					case "$HOMEPATHDEF" in
					"")
						;;
					*)
						HOMEPATH=$HOMEPATHDEF
						echo "Using default \"$HOMEPATH\""
						break ;;
					esac ;;
				"$RESET")
					STATE=reset
					break ;;
				/* | none )
					HOMEPATHDEF=$HOMEPATH
					break ;;
				* )
					echo \
"Home pathname must start with \"/\""
					;;
				esac
			    done ;;
			esac
			case $STATE in
			ok)
			    while true; do
				echo
				echo  "Information for $CLIENT ok ? [y/n] : "
				read ans
				case "$ans" in
				"y" | "yes" )
					break ;;
				"n" | "no" )
					STATE=reset
					break ;;
				* )
					echo "Please answer yes or no."
					;;
				esac
			    done ;;
			esac
			case $STATE in
			ok)
			    echo "$CLIENT"	>> /tmp/CLIENT.${ARCH}
			    echo "$IPADDR"	>> /tmp/CLIENTIP.${ARCH}
			    echo "$ETHERADDR"	>> /tmp/CLIENTETHER.${ARCH}
			    echo "$CLIENTYP"	>> /tmp/CLIENTYP.${ARCH}
			    echo "$SWAP"	>> /tmp/SWAP.${ARCH}
			    echo "$ROOTPATH"	>> /tmp/ROOTPATH.${ARCH}
			    echo "$SWAPPATH"	>> /tmp/SWAPPATH.${ARCH}
			    echo "$DUMPPATH"	>> /tmp/DUMPPATH.${ARCH}
			    echo "$HOMEPATH"	>> /tmp/HOMEPATH.${ARCH}
			    ;;
			esac
			;;
		esac
	    done
	    ;;
	esac
    done
    ;;
esac
#
# Prompt user attention last time before starting to build
#
while true; do
        echo;echo  "Are you ready to start the installation ? [y/n] : "
        read READY;
        case "${READY}" in
        "y" | "yes" )
                break ;;
        "n" | "no" )
                echo "
Installation procedure terminates.
You may continue by running this script and entering \"continue\" at the
first prompt for architecture type."
                unmountall ;;
        * )
                echo;echo "Please answer only yes or no."
        esac
done

#
# Installation starts
#
ARCHSTATE=execs
for CURRENTARCH in ${ARCHLIST}; do
  case $CURRENTARCH in
  execs | clients | share | noshare )
    ARCHSTATE=$CURRENTARCH
    ;;
  * )
    echo
    echo "Beginning Installation for the ${CURRENTARCH} architecture."
    #
    # get path and read in release tape
    #
EXECPATH=${EXEC}
    case "$ARCHSTATE" in
    execs | share | noshare )
            setup_exec $CURRENTARCH $EXECPATH $DRIVE $TAPEHOST $TAPEDEV
            STATUS=$?
	    ;;
    * )
            # do not setup these execs
            echo
            echo "[${CURRENTARCH} executables already installed]"
            STATUS=0
            ;;
    esac
    
    case ${STATUS} in
    0)
	echo
	echo "Starting installation of ${CURRENTARCH} clients..."
	echo
	if [ -f /tmp/CLIENT.${CURRENTARCH} ]; then
		#
		# Read these files just once.
		# The 'set' hacks are used to avoid using 'expr'
		# counters, which are not built-in and take awhile
		# to do as the loops get bigger.
		# 
		IPLIST=`cat /tmp/CLIENTIP.${CURRENTARCH}`
		ETHERLIST=`cat /tmp/CLIENTETHER.${CURRENTARCH}`
		YPLIST=`cat /tmp/CLIENTYP.${CURRENTARCH}`
		SWAPLIST=`cat /tmp/SWAP.${CURRENTARCH}`
		ROOTPATHLIST=`cat /tmp/ROOTPATH.${CURRENTARCH}`
		SWAPPATHLIST=`cat /tmp/SWAPPATH.${CURRENTARCH}`
		DUMPPATHLIST=`cat /tmp/DUMPPATH.${CURRENTARCH}`
		HOMEPATHLIST=`cat /tmp/HOMEPATH.${CURRENTARCH}`
		for i in `cat /tmp/CLIENT.${CURRENTARCH}`; do
			set $IPLIST
			IP=$1
			shift
			IPLIST="$*"

			set $ETHERLIST
			ETHER=$1
			shift
			ETHERLIST="$*"

			set $YPLIST
			YP=$1
			shift
			YPLIST="$*"

			set $SWAPLIST
			SWAP=$1
			shift
			SWAPLIST="$*"

			set $ROOTPATHLIST
			ROOTPATH=$1
			shift
			ROOTPATHLIST="$*"

			set $SWAPPATHLIST
			SWAPPATH=$1
			shift
			SWAPPATHLIST="$*"

			set $DUMPPATHLIST
			DUMPPATH=$1
			shift
			DUMPPATHLIST="$*"

			set $HOMEPATHLIST
			HOMEPATH=$1
			shift
			HOMEPATHLIST="$*"
			#
			# Create client
			#
			setup_client add $i $YP $SWAP \
				$ROOTPATH $SWAPPATH $DUMPPATH \
				$HOMEPATH $EXECPATH $CURRENTARCH
			case $? in
			0)
				DIDACLIENT=yes
				;;
			*)
				echo "
ATTENTION: $CURRENTARCH client $i not installed."
				echo
				;;
			esac
		done
	fi
	;;
    *)
	echo
	echo "ATTENTION: exec installation for $CURRENTARCH failed."
	echo "           $CURRENTARCH clients not installed."
	;;
    esac
    ;;
  esac
done
case "$DIDACLIENT" in
yes)
	if [ "$DOMAIN" != "noname" -a "$DOMAIN" != "" -a -f ${YPMASTERPATH}/bootparams.time ]; then
		echo
		echo "Updating bootparams YP map..."
		cd ${YPMASTERPATH}
		make bootparams
	elif [ "$DOMAIN" != "noname" -a "$DOMAIN" != "" -a ! -f ${YPMASTERPATH}/bootparams.time ]; then
		if [ ! -f ${YPMASTERPATH}/bootparams.time- ]; then
			echo "
ATTENTION: /etc/bootparams on the yp master needs to be updated."
		fi
	fi ;;
esac
echo
case "$ARCHLIST" in
"")
	echo "Nothing to install!"
	;;
*)
	echo "Diskless Client Installation Completed."
	;;
esac
unmountall
exit 0
