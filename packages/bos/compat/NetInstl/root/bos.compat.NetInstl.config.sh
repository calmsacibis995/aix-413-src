#!/bin/ksh
# @(#) 36 1.3 src/packages/bos/compat/NetInstl/root/bos.compat.NetInstl.config.sh, pkgnetinstl, pkg411, GOLD410 4/21/94 15:49:38
#
# COMPONENT_NAME: pkgnetinstl
#
# FUNCTIONS: 
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

# mkinst
#  - a korn shell script to edit /etc/services and /etc/inetd.conf
#    to setup a 4.1 server to support 3.2 clients
#
# prerequisits: BOSnet, NFS
PASSWD=/etc/passwd
CONFIG=/etc/services

#Call chsubserver to add the service
chservices -a -v instsrv -p tcp -n 1234
if [ $? -ne 0 ]
then
    exit 1
fi

chsubserver -a -v instsrv -t stream -p tcp -w nowait -u netinst -g /u/netinst/bin/instsrv -r inetd instsrv -r /tmp/netinstalllog /u/netinst/scripts 
if [ $? -ne 0 ]
then
    exit 1
fi
# Add the netinst user, if necessary
netinst_user=$(grep netinst $PASSWD)
if [[ -z $netinst_user ]]
then
    mkuser netinst
    if [ $? -ne 0 ]
    then
	exit 1
    fi
fi

# copy the files to the netinst user directory
cd /usr/lpp/bosinst
if [ $? -ne 0 ]
then
    exit 1
fi
find bin db scripts -print | cpio -dumpv /u/netinst
if [ $? -ne 0 ]
then
    exit 1
fi
chmod -R 500 /u/netinst/*
if [ $? -ne 0 ]
then
    exit 1
fi
chown -R netinst.staff /u/netinst/*
if [ $? -ne 0 ]
then
    exit 1
fi
