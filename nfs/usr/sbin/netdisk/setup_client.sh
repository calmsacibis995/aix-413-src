#! /bin/bsh
# @(#)77        1.4  com/cmd/usr.etc/netdisk/setupclnt.sh, cmdnfs, nfs320, 91453 20 3/21/90 15:25:05
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
#
#  setup_client : script written to add/remove a client
#
#

HOME=/; export HOME
PATH=/bin:/usr/bin:/etc:/usr/sbin:/usr/ucb:/usr/lpp/nfs/sun_diskless:/dev

CMDNAME=$0
MYPATH="/usr/lpp/nfs/sun_diskless"
TFTPBOOTPATH="/tftpboot"

USAGE="usage: 
${CMDNAME} op name yp size rootpath swappath dumppath homepath execpath arch
where:
	op	    = \"add\" or \"remove\"
	name	    = name of the client machine
	yp	    = \"master\" or \"slave\" or \"client\" or \"none\"
	size        = size for swap 
		      (e.g. 16M or 16m ==> 16 * 1048576 bytes
			    16K or 16k ==> 16 * 1000 bytes
			    16B or 16b ==> 16 * 512 bytes
			    16         ==> 16 bytes )
	rootpath    = pathname of nfsroot
        swappath    = pathname of nfsswap
        dumppath    = pathname of nfsdump
	homepath    = pathname of home
	execpath    = pathname of exec directory
	arch        = \"sun2\" or \"sun3\" .... 
"

#
# Verify number of arguments
#
if [ $# -ne 10 ]; then
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
DOMAIN=`domainname`
SERVER=`hostname`
# 
# Verify operation
#
OP=${1}; shift
case "$OP" in
"add" | "remove" )
        break ;;
*)
        echo "${CMDNAME}: invalid operation type \"${OP}\"."
        exit 1 ;;
esac
#
# Client name
#
NAME=${1}; shift
#
# Verify yp type
#
YP=${1}; shift
case "$YP" in
"master" | "slave" | "client" | "none" )
        break ;;
*)
        echo "${CMDNAME}: invalid yp type \"${YP}\"."
        exit 1 ;;
esac
#
# Number of bytes reserved for client's swap
#
SWAPSIZE=${1}; shift
#
# Pathnames
#
ROOTPATH=${1}; shift
if [ ! -d ${WHERE}$ROOTPATH -a "$ROOTPATH" != "none" ]; then
	mkdir ${WHERE}$ROOTPATH 2>/dev/null
	chmod 700 ${WHERE}$ROOTPATH
fi
SWAPPATH=${1}; shift
if [ ! -d ${WHERE}$SWAPPATH -a "$SWAPPATH" != "none" ]; then 
        mkdir ${WHERE}$SWAPPATH 2>/dev/null
	chmod 700 ${WHERE}$SWAPPATH
fi
DUMPPATH=${1}; shift
if [ ! -d ${WHERE}$DUMPPATH -a "$DUMPPATH" != "none" ]; then 
        mkdir ${WHERE}$DUMPPATH 2>/dev/null
fi
HOMEPATH=${1}; shift
if [ ! -d ${WHERE}$HOMEPATH -a "$HOMEPATH" != "none" ]; then
        mkdir ${WHERE}$HOMEPATH ${WHERE}$HOMEPATH/$SERVER 2>/dev/null
fi
EXECPATH=${1}; shift
#
# Verify architecture name, exec pathname and root pathname
#
ARCH=${1}
if [ ! -d $MYPATH/proto -a "$OP" = "add" ]; then
	while true; do
		echo; echo  "Enter tape drive : [ ar | st | mt | xt | cd? ] ? \c"
	 	read TAPE;
		case "$TAPE" in
		"ar" | "st" )
			BS=126
                        TAPE=/dev/nr${TAPE}0
			break ;;
		"mt" | "xt" )
			BS=20
			cd /dev
			rm -f *mt*
			MAKEDEV ${TAPE}0
                        TAPE=/dev/nrmt0
			break ;;
                /dev/* )
                        BS=20
                        #TAPE = full path i.e. /dev/rmt0.1
                        break ;;
		cd* )
			CDFS_PATH="/tmp/cdfs_$$"
			crfs -v cdrfs -p ro -d"$TAPE" -m"$CDFS_PATH" -A'no'
			if [ $? != 0 ] ; then
				echo "${CMDNAME}: invalid device for cdrom file system"
			else
				mount $CDFS_PATH
				if [ $? != 0 ] ; then
					echo "$CMDNAME: error in mounting cdrom file system"
					rmfs $CDFS_PATH
				else
					TAPEHOST="$CDFS_PATH"
					DEVNO=""
					NRTAPE=""
				fi
			fi
			break ;;
		* )
			echo; echo "${CMDNAME}: invalid tape device." ;;
		esac
	done	
	while true; do
                echo; echo  "Enter tape drive type : [ local | remote ] ? \c" 
                read TAPETYPE;
                case "$TAPETYPE" in
                "local" )    
                        break ;;
		"remote" )     
			while true; do
                		echo; echo  "Enter tape host ? \c"
                		read TAPEHOST;
                		case "$TAPEHOST" in
                		"" )
                        		echo; echo "${CMDNAME}: invalid tape host." ;;
				* )
					if [ "$DOMAIN" = "noname" -o "${WHERE}" != "" ]; then
						host $TAPEHOST >/dev/null
					else
						ypmatch $TAPEHOST hosts >/dev/null
					fi
					if [ $? = 1 ]; then
						echo; echo "${CMDNAME}: invalid tapehost."
					else
						break
					fi ;;
				esac
			done	
                        break ;;
                * )
                        echo; echo "${CMDNAME}: invalid tape device." ;;
                esac
        done
	mkdir $MYPATH/proto
        cd $MYPATH/proto
	if [ -z "$CDFS_PATH" ] ; then
		verify_tapevol_arch ${ARCH} 1 ${TAPE} ${TAPEHOST}
	fi
	TAPE_NUM=`cat /tmp/TOC | awk '$3 == "root" {print $1}'`
        NUM=`cat /tmp/TOC | awk '$3 == "root" {print $2}'`
        SKIP=$NUM
	if [ "$TAPE_NUM" != 1 -a -z "$CDFS_PATH" ]; then
        	verify_tapevol_arch ${CURRENTARCH} ${TAPE_NUM} ${TAPE} ${TAPEHOST}
	fi
	if [ -z "$CDFS_PATH" ] ; then
	        extracting ${TAPE} ${SKIP} ${BS} "root" ${TAPEHOST}
	else
	        extracting ${TAPE} "root" ${TAPEHOST} "/" ${ARCH}
	fi
fi
if [ -d ${WHERE}$ROOTPATH/$NAME -a "$OP" = "add" ]; then
	echo "${CMDNAME}: client \"${NAME}\" already exists."
	exit 1
elif [ ! -d ${WHERE}$ROOTPATH/$NAME -a "$OP" = "remove" ]; then
	echo "${CMDNAME}: client \"${NAME}\" does not exist."
        exit 1
fi
#
# Verify ip address
#
rm -f /tmp/ipaddr
if [ "$DOMAIN" = "noname" -o "$DOMAIN" = "" ]; then
        host $NAME | awk '{ print $3 }' >> /tmp/ipaddr
else
        ypmatch $NAME hosts | awk '{ print $1 }' >> /tmp/ipaddr
fi
IPADDR=`cat /tmp/ipaddr`
rm -f /tmp/ipaddr
if [ "$IPADDR" = "" ]; then
	echo "${CMDNAME}: can't find ip address in /etc/hosts for \"${NAME}\" !"
	exit 1
fi
#
# Convert ipaddr to hex
#
HEXADDR=`convert_to_hex $IPADDR`

#
#  Create the symlink name to the boot file.  Sun3 symlink names are
#  merely the 8 character uppercase IP address, while all others
#  have ".arch" appended, where "arch" is the uppercase architecture
#  name of the client.
#
case "$ARCH" in
sun3)
        BOOTSYMLINK=$HEXADDR
        ;;
sun386)
        BOOTSYMLINK=`echo "$HEXADDR." | /usr/ucb/tr -d " "`S386
        ;;
*)
        # Get rid of spaces in $HEXADDR and make $ARCH uppercase
        BOOTSYMLINK=`echo "$HEXADDR." | /usr/ucb/tr -d " "``echo "$ARCH" \
                | /usr/ucb/tr a-z A-Z`
        ;;
esac
#

#
# Verify ethernet address
#
rm -f /tmp/etheraddr
if [ "$DOMAIN" = "noname" -o "$DOMAIN" = "" ]; then
        grep -w $NAME ${WHERE}/etc/ethers | awk '{ print $1 }' >> /tmp/etheraddr
else
        ypmatch $NAME ethers | awk '{ print $1 }' >> /tmp/etheraddr
fi
ETHERADDR=`cat /tmp/etheraddr`
rm -f /tmp/etheraddr
if [ "$ETHERADDR" = "" ]; then 
        echo "${CMDNAME}: can't find address in /etc/ethers for \"${NAME}\" !"
        exit 1    
fi
echo
#
# Start add/remove a nfs client
#
if [ "$OP" = "add" ]; then
	echo "Start creating $ARCH client \"${NAME}\" :"
	echo
	# 
	# Update bootparams
	# 
	echo "Updating bootparams ..."
	if [ -f ${WHERE}/etc/bootparams ]; then
		grep $NAME ${WHERE}/etc/bootparams >/dev/null 2>&1
		if [ "$?" = 0 ]; then
			fix_bootparams remove $NAME $ROOTPATH $SWAPPATH $DUMPPATH
		fi
	fi
	fix_bootparams $OP $NAME $ROOTPATH $SWAPPATH $DUMPPATH
	if [ "$DOMAIN" != "noname" -a -f ${WHERE}/var/yp/bootparams.time ]; then
		cd ${WHERE}/var/yp
		make bootparams
	elif [ "$DOMAIN" != "noname" -a ! -f ${WHERE}/var/yp/bootparams.time ]; then
		if [ ! -f ${WHERE}/var/yp/bootparams.time- ]; then
			echo "ATTENTION: /etc/bootparams on the yp master needs to be updated."
		fi
	fi
	#
	# Setup root
	#
	echo
	echo "Creating root for client \"${NAME}\"."
	create_root $NAME $ARCH $YP $ROOTPATH $HOMEPATH $EXECPATH $SERVER "$DOMAIN"
	cd ${WHERE}${ROOTPATH}/${NAME}/etc
	#
	# cp ${MYPATH}/rc.local rc.local
	# use the rc.local setup in create_root
	# 
	cd ${WHERE}$EXECPATH/kvm/$ARCH/boot
	if [ ! -d ${WHERE}$ROOTPATH/${NAME}/single ]; then 
		mkdir ${WHERE}${ROOTPATH}/${NAME}/single
		chmod 755 ${WHERE}$ROOTPATH/${NAME}/single
	fi
	if [ ! -d ${WHERE}$ROOTPATH/${NAME}/usr ]; then 
		mkdir ${WHERE}${ROOTPATH}/${NAME}/usr
		chmod 755 ${WHERE}$ROOTPATH/${NAME}/usr
	fi
	tar cf - init mount ifconfig intr | (cd ${WHERE}${ROOTPATH}/${NAME}/single; tar xpf -)      
	tar cf - init mount ifconfig intr | (cd ${WHERE}${ROOTPATH}/${NAME}/sbin; tar xpf -)      
	cd ${WHERE}$EXECPATH/$ARCH/bin
	tar cf - sh hostname | (cd ${WHERE}${ROOTPATH}/${NAME}/single; tar xpf -)      
	tar cf - sh hostname | (cd ${WHERE}${ROOTPATH}/${NAME}/sbin; tar xpf -)      
	cp ${WHERE}$EXECPATH/kvm/$ARCH/stand/vmunix ${WHERE}${ROOTPATH}/${NAME}
	#
	# Setup swap
	#
	echo
	echo "Creating $SWAPSIZE bytes of swap for client \"${NAME}\"."
	cd ${WHERE}$SWAPPATH
	if [ -f ${WHERE}/${MYPATH}/mkfile ]; then
		${WHERE}/${MYPATH}/mkfile $SWAPSIZE $NAME
	else
		echo "ATTENTION: mkfile does not exist."
		echo "ATTENTION: $SWAPSIZE bytes of swap for \"${NAME}\" not created."
	fi
	#
	# Setup dump
	#
	echo
	echo "Creating dump for client \"${NAME}\"."
	if [ "$DUMPPATH" != "none" ]; then
		cd ${WHERE}$DUMPPATH
		${WHERE}/usr/bin/touch $NAME
		chmod 666 $NAME
	fi
        #
        # Setup ${TFTPBOOTPATH} directory
        #
        echo
        echo "Setting up ${TFTPBOOTPATH} directory."
        if [ ! -d ${TFTPBOOTPATH} ]; then
                ${MYPATH}make_dirs ${TFTPBOOTPATH} 755
        fi
        cd ${TFTPBOOTPATH}
        if [ -f /usr/etc/in.tftpd ]; then
                if [ ! -f in.tftpd ]; then
                        (cd /usr/etc; tar cf - in.tftpd) |  tar xpf -
                fi
                if [ ! -d tftpboot ]; then
                        ln -s . tftpboot
                fi
        fi
        #
        #  Create the symlink to the boot file.
        #
        case $ARCH in
        sun386 )
                BOOTARCH=S386
                ;;
        * )
                BOOTARCH=$ARCH
                ;;
        esac
        rm -f $BOOTSYMLINK
        if [ -f boot.${BOOTARCH} ]; then
                ln -s boot.${BOOTARCH} $BOOTSYMLINK
	elif [ -f ${WHERE}$EXECPATH/kvm/$ARCH/stand/boot.${ARCH} ]; then
	       cp ${WHERE}$EXECPATH/kvm/$ARCH/stand/boot.${ARCH} .
               ln -s boot.${BOOTARCH} $BOOTSYMLINK
	       ln -s boot.${ARCH} $HEXADDR
        # following elif left in from 3.1 AIX
        elif [ -f $EXECPATH/stand/boot.${BOOTARCH} ]; then
                cp $EXECPATH/stand/boot.${BOOTARCH} .
                ln -s boot.${BOOTARCH} $BOOTSYMLINK
        else
                echo "ATTENTION: ${TFTPBOOTPATH}/boot.${BOOTARCH} doesn't exist."
                echo "ATTENTION: $BOOTSYMLINK -> boot.${BOOTARCH} not created."
        fi

        case "$ARCH" in
        sun2)
                if [ ! -f sun2.bb ]; then
                        if [ -f $EXECPATH/stand/sun2.bb ]; then
                                cp $EXECPATH/stand/sun2.bb .
                        else
                                echo "  
ATTENTION: $EXECPATH/stand/sun2.bb doesn't exist.
ATTENTION: Can't create sun2 bootblock ${TFTPBOOTPATH}/sun2.bb"
                        fi
                fi ;;
        esac



	#
	# Fix /etc/exports on server
	#
	grep $NAME ${WHERE}/etc/exports >/dev/null
	if [ "$?" = 1 ]; then
		echo
		echo "Updating /etc/exports to export \"$NAME\" info."
		echo "#" >> ${WHERE}/etc/exports
		echo "$ROOTPATH/$NAME -root=$NAME,access=$NAME" >> ${WHERE}/etc/exports
		echo "$SWAPPATH/$NAME -root=$NAME,access=$NAME" >> ${WHERE}/etc/exports
		echo "$EXECPATH -root=$NAME,access=$NAME" >> ${WHERE}/etc/exports
		if [ "$DUMPPATH" != "none" ]; then
			echo "$DUMPPATH/$NAME -root=$NAME" >> ${WHERE}/etc/exports
		fi
		if [ -f ${WHERE}/usr/sbin/exportfs ]; then
			${WHERE}/usr/sbin/exportfs -a
			if [ "$?" != 0 ]; then
				echo "ATTENTION: /etc/exports needs attention !"
				echo "ATTENTION: fix /etc/exports and rerun exportfs !"
			fi
		else
			echo "ATTENTION: /usr/sbin/exportfs does not exist !"
		fi
	fi
	grep $HOMEPATH ${WHERE}/etc/exports >/dev/null
	if [ "$?" = 1 ]; then
                echo
                echo "Updating /etc/exports to export \"$HOMEPATH\"."
		echo "#" >> ${WHERE}/etc/exports
		echo "$HOMEPATH" >> /etc/exports
                if [ -f ${WHERE}/usr/sbin/exportfs ]; then
                        ${WHERE}/usr/sbin/exportfs -a
                        if [ "$?" != 0 ]; then
                               echo "ATTENTION: /etc/exports needs attention !"
                               echo "ATTENTION: fix /etc/exports and rerun exportfs !"
                        fi
                else
                        echo "ATTENTION: /usr/sbin/exportfs does not exist !"
                fi
        fi

	echo
	echo "Completed creating $ARCH client \"$NAME\"."
	if [ "$ARCH" = "sun2" ]; then
		echo
		echo "ATTENTION: /etc/ndbootd needs to be running on server before bring up \"$NAME\"."
	fi
elif [ "$OP" = "remove" ]; then
	echo "Start removing $ARCH client \"$NAME\" :"
	echo
	#
	# Update bootparams
	#
	if [ -f ${WHERE}/etc/bootparams ]; then
		grep $NAME ${WHERE}/etc/bootparams >/dev/null 2>&1
		if [ "$?" = 0 ]; then
			echo "Updating bootparams ..."
			fix_bootparams $OP $NAME $ROOTPATH $SWAPPATH $DUMPPATH
		fi
	fi
	if [ "$DOMAIN" != "noname" -a -f ${WHERE}/var/yp/bootparams.time ]; then
                cd ${WHERE}/var/yp
                make bootparams
	elif [ "$DOMAIN" != "noname" -a ! -f ${WHERE}/var/yp/bootparams.time ]; then
		if [ ! -f ${WHERE}/var/yp/bootparams.time- ]; then
                	echo "ATTENTION: /etc/bootparams on the yp master needs to be updated."
		fi
        fi
	# 
        # Clean nfsroot 
        # 
	if [ "$ROOTPATH" != "none" ]; then
		echo
		echo "Removing root for client \"${NAME}\"."
        	cd ${WHERE}$ROOTPATH
        	rm -rf $NAME
	fi
        #
        # Clean nfsswap
        #
	if [ "$SWAPPATH" != "none" ]; then
		echo
		echo "Removing swap for client \"${NAME}\"."
        	cd ${WHERE}$SWAPPATH
		rm -f $NAME
	fi
        #
        # Clean nfsdump
        #
	if [ "$DUMPPATH" != "none" ]; then
		echo
		echo "Removing dump for client \"${NAME}\"."
        	cd ${WHERE}$DUMPPATH
		rm -f $NAME
		cd ${WHERE}/tftpboot
		rm -f $HEXADDR
	fi
	#
	# Fix /etc/exports on server
	#
	grep ${NAME} ${WHERE}/etc/exports >/dev/null 2>&1
	if [ "$?" = 0 ]; then
ed - ${WHERE}/etc/exports <<END
g,$NAME,d
d
w
q
END
		if [ -f ${WHERE}/usr/sbin/exportfs ]; then
			exportfs -a
			if [ "$?" != 0 ]; then
				echo "ATTENTION: /etc/exports needs attention !"
				echo "ATTENTION: fix exports and rerun exportfs !"
			fi
		else
			echo "ATTENTION: exportfs does not exist !"
		fi
	fi

	echo
	echo "Completed removing $ARCH client \"$NAME\"."
else
	echo;echo "${CMDNAME}: invalid operation type \"$OP\"."
	exit 1
fi
exit 0
