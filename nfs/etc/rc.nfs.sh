#!/bin/bsh
# @(#)47	1.34  src/nfs/etc/rc.nfs.sh, cmdnfs, nfs411, GOLD410 5/31/94 08:05:21
# COMPONENT_NAME: (CMDNFS) Network File System Commands
# 
# FUNCTIONS: 
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# start() has the following logic
# 1) if srcmstr is running, use it to start the daemon
# 2) if srcmstr is NOT running, lookup the daemon path and arguments from
# srcmstr's config database in odm (SRCsubsys).
# 3) if the config info can not be found, attempt to start the daemon
# from the default parameters passed into start().
#
start()
{

daemon=$1 					    # Subsystem name
default_path=$2					    # full path w/ cmdname
shift;shift
default_arg=$*					    # default arguments

#
# get the path to the daemon from the SRC ODM config info
#
daemon_path=`odmget -q subsysname=$daemon SRCsubsys 2>/dev/null | \
      awk ' $1 == "path" { print $NF }' 2>/dev/null | sed 's/"//g' `

#
# if daemon_path not set (length zero) then try synonymname
#
if [ -z "$daemon_path" ] ; then 	
	daemon_path=`odmget -q synonym=$daemon SRCsubsys 2>/dev/null | \
      		awk ' $1 == "path" { print $NF }' 2>/dev/null | sed 's/"//g' `
fi

#
# get the arguments to the daemon from the SRC ODM config info
#
cmdargs=`odmget -q subsysname=$daemon SRCsubsys 2>/dev/null | \
      awk '$1  == "cmdargs" { print $NF }' 2>/dev/null | sed 's/"//g' `


if [ -n "$src_running" -a -n "$daemon_path" ] ; then 	
	#
	#if srcmstr is running and there is an entry in SRCsubsys - use src
	#
	startsrc -s $daemon 
else					#if srcmstr not running, start manually
	if [ -n "$daemon_path" ] ; then
		if [ -n "$cmdargs" ] ; then
			$daemon_path $cmdargs  &		# issue cmd
		else
			$daemon_path $default_arg  & 	# issue cmd
		fi
	else
		$default_path $default_arg &  		#issue cmd
	fi
	
fi

} 	# end start()

#
# determine of srcmstr is running 
#
if [ -n "`ps -e | awk '$NF == "srcmstr" {print $1} '`" ] ; then
       src_running=1
else 
       src_running=""
fi


# Check the mount of /.  If it is remote, do not start statd,lockd.
REMOTE_ROOT="N"
/usr/sbin/mount | /usr/bin/grep ' / *jfs ' 2>&1 > /dev/null
if [ "$?" != 0 ]
then
	REMOTE_ROOT="Y"
fi

# Check the mount of /usr.  If it is remote, do not start statd.
REMOTE_USR="N"
/usr/sbin/mount | /usr/bin/grep ' /usr *jfs ' 2>&1 > /dev/null
if [ "$?" != 0 ]
then
	REMOTE_USR="Y"
fi

# Uncomment the following lines  and change the domain
# name to define your domain (domain must be defined
# before starting NIS).
#if [ -x /usr/bin/domainname ]; then
#	/usr/bin/domainname ibm
#fi

#
# Clear all servers' rmtab files in case we went down abnormally.
#
if [ -s /sbin/helpers/nfsmnthelp ]; then
	/sbin/helpers/nfsmnthelp B 0
fi

#dspmsg cmdnfs.cat -s 8 2 "starting NIS services:\n"
#if [ -x /usr/lib/netsvc/yp/ypserv -a -d /var/yp/`domainname` ]; then
#	start ypserv /usr/lib/netsvc/yp/ypserv
#fi

#if [ -x /usr/lib/netsvc/yp/ypbind ]; then
#	start ypbind /usr/lib/netsvc/yp/ypbind
#fi

#if [ -x /usr/sbin/keyserv ]; then
#	start keyserv /usr/sbin/keyserv
#fi

#if [ -x /usr/lib/netsvc/yp/rpc.ypupdated -a -d /var/yp/`domainname` ]; then
#	start ypupdated /usr/lib/netsvc/yp/rpc.ypupdated
#fi

dspmsg cmdnfs.cat -s 8 1 "starting nfs services:\n"
if [ -x /usr/sbin/biod ]; then
	start biod /usr/sbin/biod 8
fi

#
# If nfs daemon is executable and /etc/exports, become nfs server.
#
if [ -x /usr/sbin/nfsd -a -f /etc/exports ]; then
	> /etc/xtab
	/usr/sbin/exportfs -a
	start nfsd /usr/sbin/nfsd 8
	start rpc.mountd /usr/sbin/rpc.mountd
fi

#
# start up status monitor and locking daemon if present
#
if [ -x /usr/sbin/rpc.statd -a $REMOTE_ROOT = "N" -a $REMOTE_USR = "N" ]; then
	start rpc.statd /usr/sbin/rpc.statd
fi

if [ -x /usr/sbin/rpc.lockd -a $REMOTE_ROOT = "N" ]; then
	start rpc.lockd /usr/sbin/rpc.lockd
fi

#
#Uncomment the following lines to start up the NIS 
#yppasswd daemon.
#DIR=/etc
#if [ -x /usr/lib/netsvc/yp/rpc.yppasswdd -a -f $DIR/passwd ]; then
#	start rpc.yppasswdd /usr/lib/netsvc/yp/rpc.yppasswdd /etc/passwd -m
#fi

