#! /bin/sh
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
# Create a new root directory from the prototype directory
#
#

HOME=/; export HOME

CMDNAME=$0

HOSTNAME=$1
ARCH=$2
YP=$3
ROOTPATH=$4
HOMEPATH=$5
EXECPATH=$6
SERVER=$7
DOMAIN=$8
MYPATH="/usr/lpp/nfs/sun_diskless"
PATH=${MYPATH}:/bin:/usr/bin:/etc:/usr/sbin:/usr/ucb

if [ $# -lt 8 ]; then
	echo "${CMDNAME}: incorrect number of arguments."
	echo "usage: $0 hostname archname yptype rootpath homepath execpath server domain"
	exit 1
fi

if [ ! -d $MYPATH/proto ]; then
	echo "${CMDNAME}: No proto directory"
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
# copy the proto directory
#
mkdir ${WHERE}${ROOTPATH} ${WHERE}${ROOTPATH}/$HOSTNAME 2>/dev/null
cd $MYPATH/proto
#tar cf - . | (cd ${WHERE}${ROOTPATH}/$HOSTNAME; tar xpf - )
find . -print | cpio -pdl ${ROOTPATH}/$HOSTNAME > /dev/null
#
# make devices
#
cd ${WHERE}${ROOTPATH}/$HOSTNAME/dev
# Take care of the differences in the awk command

# This is for the difference in the number of bits used for major
# and minor numbers for devices.  This difference is between 3.1 and
# Suns
cat > ./mknod << ENDFILE
#!/bin/bsh
minornumber=\`expr 256 \\* \$3 + \$4\`
/usr/sbin/mknod \$1 \$2 0 \$minornumber
ENDFILE

chmod 755 ./mknod

# Go and make the devices for the client.

#    ./MAKEDEV.tmp2 std pty0 pty1 pty2 win0 win1 win2 sd0 xy0 st0
${MYPATH}/MAKEDEV std pty0 pty1 pty2 win0 win1 win2 xy0 

mkdir ${WHERE}${ROOTPATH}/$HOSTNAME/home 2>/dev/null
mkdir ${WHERE}${ROOTPATH}/$HOSTNAME/home/$SERVER 2>/dev/null
#
# Fix files in /etc
#
cd ${WHERE}${ROOTPATH}/$HOSTNAME/etc
if [ ! -L ./rmt ] ; then
	ln -s /usr/etc/rmt .
fi

ed - ${WHERE}${ROOTPATH}/$HOSTNAME/etc/rc.boot <<END
/hostname=/s/^.*$/hostname=$HOSTNAME/
w
q
END
if [ "${EXECPATH}" = "/usr" ]; then
ed - ${WHERE}${ROOTPATH}/$HOSTNAME/etc/fstab 1>/dev/null <<END
g,/rootdir,s,,${ROOTPATH}/$HOSTNAME,g
g,/execdir,s,,${EXECPATH},g
g,/homedir,s,,${HOMEPATH},g
$
a
$SERVER:${EXECPATH}/kvm/${ARCH} /usr/kvm nfs ro 0 0
.
g,share,d
g,server,s,,$SERVER,g
w
q
END
else
ed - ${WHERE}${ROOTPATH}/$HOSTNAME/etc/fstab 1>/dev/null <<END 
g,/rootdir,s,,${ROOTPATH}/$HOSTNAME,g
g,/execdir,s,,${EXECPATH}/$ARCH,g
g,/homedir,s,,${HOMEPATH},g
$
a
$SERVER:${EXECPATH}/kvm/${ARCH} /usr/kvm nfs ro 0 0
.
g,share,d
g,server,s,,$SERVER,g
w
q
END
fi
if [ "$YP" != "none" ]; then
ed - ${WHERE}${ROOTPATH}/$HOSTNAME/etc/rc.local <<END
/domainname/s/ [^ ]*\$/ $DOMAIN/
w
q
END
if [ "$YP" = "client" ]; then
ed - ${WHERE}${ROOTPATH}/$HOSTNAME/etc/rc.local <<END
/ypserv/s/^/#/
/ypserv/s/^/#/
/fi/s/^/#/
w
q
END
fi
elif [ "$YP" = "none" ]; then
ed - ${WHERE}${ROOTPATH}/$HOSTNAME/etc/rc.local <<END
/ypbind/s/^/#/
/ypbind/s/^/#/
/fi/s/^/#/
w
/ypserv/s/^/#/
/ypserv/s/^/#/
/fi/s/^/#/
w
q
END
fi
if [ "$DOMAIN" != "noname" -a "$DOMAIN" != "" ]; then
	ypmatch $SERVER $HOSTNAME hosts | sed -e 's/   *$//' >> ${WHERE}${ROOTPATH}/$HOSTNAME/etc/hosts
else
        grep $SERVER ${WHERE}/etc/hosts | sed -e 's/   *$//' >> ${WHERE}${ROOTPATH}/$HOSTNAME/etc/hosts
	grep $HOSTNAME ${WHERE}/etc/hosts | sed -e 's/   *$//' >> ${WHERE}${ROOTPATH}/$HOSTNAME/etc/hosts
fi
