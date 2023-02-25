#!/bin/ksh
# @(#) 37 1.3 src/packages/bos/compat/NetInstl/root/bos.compat.NetInstl.unconfig.sh, pkgnetinstl, pkg411, 9432B411a 8/10/94 17:58:05
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

# unmkinst
#  - a korn shell script to edit /etc/services and /etc/inetd.conf
#    to remove setup of a 4.1 server to support 3.2 clients
#
# prerequisits: BOSnet, NFS

# Files used
PASSWD=/etc/passwd

# Call chsubserver to remove the service
chsubserver -d -v instsrv -p tcp -r inetd
if [ $? -ne 0 ]
then
    exit 1
fi
# Call services to remove the service
chservices -d -v instsrv -p tcp -n 1234
if [ $? -ne 0 ]
then
    exit 1
fi

# remove the netinst user, if necessary
netinst_user=$(grep netinst $PASSWD)
if [[ -n $netinst_user ]]
then
    rmuser netinst
    if [ $? -ne 0 ]
    then
	exit 1
    fi
fi

# remove the files in the netinst user directory
rm -rf /u/netinst
if [ $? -ne 0 ]
then
    exit 1
fi
