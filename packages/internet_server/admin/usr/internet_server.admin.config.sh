#!/usr/bin/ksh
# @(#)39	1.1  src/packages/internet_server/admin/usr/internet_server.admin.config.sh, pkgweb, pkg41J, 9516B_all 4/19/95 18:15:28
#
# COMPONENT_NAME: pkgweb
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME:	 internet_server.admin.config.sh
#                                                                    
# FUNCTION: script called from instal to do special
# 	internet_server configuration
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  
# ============================================================#
# fix_up html pages to point to this server.
# ============================================================#
#set -f			# this is so that the case statement will work

COMMAND_NAME=`/usr/bin/basename $0`
TMP_HTML="/tmp/tmp.HTML$$"

SED_CMD="/usr/bin/sed"
CP_CMD="/usr/bin/cp"
RM_CMD="/usr/bin/rm"

#USAGE="
#usage: ${COMMAND_NAME}"
#
#set -- `/bin/getopt h: $*  2>/dev/null` 
#if [ $? != 0 ] ; then          # Test for syntax error
#    dspmsg fix_up.cat -s 1 1 "${USAGE}" $COMMAND_NAME
#    exit 1
#fi
#
#while [ $1 != -- ]
#do
#    case $1 in
#	-a|-d|-c)		# Action is to add or delete the service
#	    if [ -n "${ACTION}" ] ; then dspmsg chservices.cat -s 1 1 "$USAGE" ${COMMAND_NAME} ; exit 1 ; fi
#	    ACTION=`expr "$1" : "-\(.\)"`
#	    shift		# Shift past the flag
#	    ;;
#    esac
#done
#shift                   	# Shift past the "--" from getopt

HOSTNAME=`hostname`
 
for i in `find /usr/lpp/internet/server_root/pub -name "*.html" -print` 
do
    ${SED_CMD} "\?t-negev?s/t-negev/${HOSTNAME}/" $i > ${TMP_HTML}
    if [ $? != 0 ] ; then
	    echo "error in updating $i\n"
	    exit 1
    fi
    #copy the file back to /usr/lpp/internet/server_root/pub
    ${CP_CMD} ${TMP_HTML} $i > /dev/null 2>&1
    if [ $? != 0 ] ; then
	    echo "error in updating $i\n"
	    ${RM_CMD} -f ${TMP_HTML} > /dev/null 2>&1
	    exit 1	
    fi
done

exit 0
