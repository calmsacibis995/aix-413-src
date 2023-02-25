#!/bin/bsh
#
# @(#)98	1.9  src/nfs/usr/sbin/sysmgt/lsmaster.sh, cmdnfs, nfs411, GOLD410 6/9/94 15:32:14
# COMPONENT_NAME: (CMDNFS) Network File System Commands
#
# FUNCTIONS:
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

# Set the default printing format
FORMAT="colon"
MAKEDBM="/usr/sbin/makedbm"
TEMPFILE=/tmp/lsmaster.$$

COMMAND_NAME=`basename $0`

USAGE="
usage:  ${COMMAND_NAME}	[ -c | -l ]\n"


case "$#" in
	"0")
		FORMAT="colon"
		;;
	"1")
		if [ "$1" = "-c" ] ; then
			FORMAT="colon"
		elif [ "$1" = "-l" ] ; then
			FORMAT="list"
		else
			dspmsg cmdnfs.cat -s 37 3 "$USAGE"
			exit
		fi
		;;
	*)
		dspmsg cmdnfs.cat -s 37 3 "$USAGE"
		exit
		;;
esac

# Print the header
if [ "${FORMAT}" = "colon" ] ; then
	echo "#currentdomain:slaves:domains:yppasswddrunning:ypupdatedrunning:ypbindrunning"
	echo "`domainname`:\c"
else
	dspmsg cmdnfs.cat -s 37 15 "Slave servers for the domain `domainname`\n" `domainname`
fi


# Find out what slaves are served by this master
${MAKEDBM} -u /var/yp/`domainname`/ypservers > ${TEMPFILE} 2>/dev/null
if [ $? = 0 ] ; then
	if [ "${FORMAT}" = "colon" ] ; then
		/bin/awk '
			BEGIN			{ i = 0 }
			/YP_LAST_MODIFIED.*/	{break}
			/YP_MASTER_NAME.*/	{ master = $2 ; break }
			{
				slave[i] = $1
				i = i + 1
			}
			END	{
					for (j = 0; j < i; j++) {
					    if (slave[j] != master) {
						  printf "%s ", slave[j]
				}
			}
		}
                ' < ${TEMPFILE}
	else
		/bin/awk '
			BEGIN			{ i = 0 }
			/YP_LAST_MODIFIED.*/	{break}
			/YP_MASTER_NAME.*/	{ master = $2 ; break }
			{
				slave[i] = $1
				i = i + 1
			}
			END	{
					for (j = 0; j < i; j++) {
					    if (slave[j] != master) {
						  printf "\t%s\n", slave[j]
				}
			}
		}
                ' < ${TEMPFILE}
	fi
fi
/bin/rm ${TEMPFILE}

# Find out what domains this master is serving
if [ "${FORMAT}" = "colon" ] ; then
	echo ":\c"
else
	dspmsg cmdnfs.cat -s 37 16 "\nDomains that are being served\n"
fi

if [ "${FORMAT}" = "colon" ] ; then
	find /var/yp -type d -exec basename {} \; | \
	/bin/awk '{if (NR > 1 && $1 != "yp" && $1 != "binding" ) printf "%s ", $1}'
else
	find /var/yp -type d -exec basename {} \; | \
	/bin/awk '{if (NR > 1 && $1 != "yp" && $1 != "binding" ) printf "\t%s\n", $1}'
fi

# Find out what daemons are going to be started when the
# /etc/rc.nfs file is executed.
RC_NFS="/etc/rc.nfs"
EGREP_CMD="/bin/egrep"

# Is the ypserv daemon going to be started?

${EGREP_CMD} "#.*start.*ypserv" ${RC_NFS} > /dev/null 2>&1
if [ $? = 0 ] ; then
	YPSERV_STARTED="no"
else
	YPSERV_STARTED="yes"
fi

# Is the ypbind daemon started?

${EGREP_CMD} "#.*start.*ypbind" ${RC_NFS} > /dev/null 2>&1
if [ $? = 0 ] ; then
	YPBIND_STARTED="no"
else
	YPBIND_STARTED="yes"
fi

# Is the ypupdated daemon started?

${EGREP_CMD} "#.*start.*ypupdated" ${RC_NFS} > /dev/null 2>&1
if [ $? = 0 ] ; then
	YPUPDATED_STARTED="no"
else
	YPUPDATED_STARTED="yes"
fi

${EGREP_CMD} "#.*start.*yppasswdd" ${RC_NFS} > /dev/null 2>&1
if [ $? = 0 ] ; then
	YPPASSWDD_STARTED="no"
else
	YPPASSWDD_STARTED="yes"
fi

if [ "${FORMAT}" = "list" ] ; then
	dspmsg cmdnfs.cat -s 37 17 "\nThese NIS daemons will be started.\n"
	if [ "${YPSERV_STARTED}" = "yes" ] ; then
		dspmsg cmdnfs.cat -s 37 18 "\typserv will be started on system restart\n"
	fi
	if [ "${YPBIND_STARTED}" = "yes" ] ; then
		dspmsg cmdnfs.cat -s 37 19 "\typbind will be started on system restart\n"
	fi
	if [ "${YPPASSWDD_STARTED}" = "yes" ] ; then
		dspmsg cmdnfs.cat -s 37 20 "\typpasswdd will be started on system restart\n"
	fi
	if [ "${YPUPDATED_STARTED}" = "yes" ] ; then
		dspmsg cmdnfs.cat -s 37 21 "\typupdated will be started on system restart\n"
	fi
else
	echo ":\c"
	if [ "${YPPASSWDD_STARTED}" = "yes" ] ; then
		echo "-P:\c"
	else
		echo "-p:\c"
	fi
	if [ "${YPUPDATED_STARTED}" = "yes" ] ; then
		echo "-U:\c"
	else
		echo "-u:\c"
	fi
	if [ "${YPBIND_STARTED}" = "yes" ] ; then
		echo "-C"
	else
		echo "-c"
	fi
fi
