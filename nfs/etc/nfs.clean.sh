#!/bin/ksh
# @(#)75	1.8  src/nfs/etc/nfs.clean.sh, cmdnfs, nfs411, GOLD410 2/3/94 16:23:48
# COMPONENT_NAME: CMDNFS nfs.clean
# 
# FUNCTIONS: 
#
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# module name - nfs.clean
#     removes NFS/NIS daemons
#
trap "exec > /dev/null ; exec 2> /dev/null" 13

GETOPT_CMD="/bin/getopt"
DFLAG="R"
NIS_FLAG="N"
USAGE="Usage: nfs.clean [-d][-y]\n"
set -- `${GETOPT_CMD} dy $* 2>/dev/null`
if [ $? != 0 ] ; then
	dspmsg cmdnfs.cat -s 7 2 "${USAGE}"
	exit 1
fi

while [ $1 != -- ]
do
   case "$1" in
	"-d")
	    DFLAG="D"
	    shift
	    ;;
	"-y")
	    NIS_FLAG="Y"
	    shift
	    ;;
         *)
	   dspmsg cmdnfs.cat -s 7 2 "${USAGE}"
	   exit 1
	   ;;
   esac
done
# check to make sure that there are no extra parameters
shift	# get rid of the -- parameter
if [ -n "$1" ] ; then  # something extra
	dspmsg cmdnfs.cat -s 7 2 "${USAGE}"
	exit 1
fi

dspmsg cmdnfs.cat -s 7 1 'Stopping NFS/NIS Daemons\n'


# Stop nfsd daemon
if [ -f /usr/sbin/nfsd ]; then
	stopsrc -f -s nfsd
fi

# Stop biod daemon if the "-d" flag was not set
if [ "${DFLAG}" = "R" ] ; then
   if [ -f /usr/sbin/biod ]; then
	stopsrc -f -s biod
   fi
fi
# Stop rpc.lockd daemon if the "-d" flag was not set
if [ "${DFLAG}" = "R" ] ; then
   if [ -f /usr/sbin/rpc.lockd ]; then
	stopsrc -f -s rpc.lockd
  fi
fi

# Stop rpc.statd daemon if the "-d" flag was not set
if [ "${DFLAG}" = "R" ] ; then
   if [ -f /usr/sbin/rpc.statd ]; then
	stopsrc -f -s rpc.statd
   fi
fi

# Stop rpc.mountd daemon
if [ -f /usr/sbin/rpc.mountd ]; then
	stopsrc -f -s rpc.mountd
fi

# Stop ypserv daemon
if [ "${DFLAG}" = "R" -a "${NIS_FLAG}" = "N" ] ; then
   if [ -f /usr/lib/netsvc/yp/ypserv ]; then
	stopsrc -f -s ypserv
   fi
fi

# Stop ypbind daemon
if [ "${DFLAG}" = "R" -a "${NIS_FLAG}" = "N" ] ; then
   if [ -f /usr/lib/netsvc/yp/ypbind ]; then
	stopsrc -f -s ypbind
   fi

   #
   # since we're stopping ypbind, perhaps we should
   # also turn the domainname off?
   #
   if [ -f /usr/bin/domainname ]; then
	domainname ""
   fi
fi

# Stop yppasswdd daemon
if [ -f /usr/lib/netsvc/yp/rpc.yppasswdd ]; then
	stopsrc -f -s yppasswdd
fi

# Stop ypupdated daemon
if [ -f /usr/lib/netsvc/yp/rpc.ypupdated ]; then
	stopsrc -f -s ypupdated
fi

exit 0
