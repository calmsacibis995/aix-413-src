#!/usr/bin/bsh
# @(#)28	1.26  src/bos/usr/sbin/skulker/skulker.sh, cmdcntl, bos411, 9428A410j 7/1/94 16:08:55
#
# COMPONENT_NAME: (CMDCNTL) system control commands
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

date=`/usr/bin/date`
uname=`/usr/bin/uname -nm`
if msg=`/usr/bin/dspmsg skulker.cat 1 '\n%1$s started at %2$s on %3$s\n' $0 "$date" "$uname"`
then :
else msg='/usr/bin/echo "\n$0 started at $date on $uname\n"'; fi
eval /usr/bin/echo $msg

# Uncomment the NATIVE entry that is appropriate for your system.
# For Distributed environments, '/native' is a path to the local filesystems;
# '/' is sufficient for standalone systems.
# NATIVE=/native/
NATIVE=/

# get rid of old primary.outputs that got lost
if [ -d ${NATIVE}var/spool/qdaemon ]
then
  /usr/bin/find ${NATIVE}var/spool/qdaemon -mtime +4 -type f -exec /usr/bin/rm -f {} \;
fi

# get rid of old qdir files
if [ -d ${NATIVE}var/spool/lpd/qdir ]
then
  /usr/bin/find ${NATIVE}var/spool/lpd/qdir -mtime +4 -type f -exec /usr/bin/rm -f {} \;
fi

# get rid of files that get left in the mail queues
if [ -d ${NATIVE}var/spool/qftp ]
then
  /usr/bin/find ${NATIVE}var/spool/qftp \( -name 'tmp*' -o -name '[0-9]*' \) -mtime +2 -exec /usr/bin/rm -f {} \;
fi

# Check if regular files are in ${NATIVE}tmp.
# If there are no files in tmp, do not invoke xargs.
# If xargs received no input (except for EOF),
# it would execute its command once.
# In that case, $0 would expand to "sh"
# and find would produce an error message.

expr 0 = `/usr/bin/ls -1Aq ${NATIVE}tmp | \
        /usr/bin/grep -v "\`/usr/bin/echo '[ \t]'\`" | /usr/bin/wc -l` > /dev/null
case $? in
  1 )
	ESCAPED_NATIVE=`echo "${NATIVE}" | /usr/bin/sed "s!/!\\\\\\\\/!g"`
# get rid of all ordinary files in the ${NATIVE}tmp directory older than 24 
# hours and not accessed or modified in the past 24 hours.
#
#    first line   finds names of all ordinary files in ${NATIVE}tmp
#                 lists them one per line, with control chars changed to ?
#    second line  filters out dangerous characters that might terminate
#                 xarg's argument (space and tab).
#    last lines   runs find on groups to find non-current files and rm's them
#                 the "-type f" redundant, unless the egrep list is flakey.
#
        /usr/bin/ls -1Aq ${NATIVE}tmp | \
        /usr/bin/grep -v "`/usr/bin/echo '[ \t]'`" | \
        /usr/bin/sed "s/^/${ESCAPED_NATIVE}tmp\//" | \
        /usr/bin/xargs -e /usr/bin/sh -c  \
	    '`/usr/bin/find ${0} ${@}  -atime +0 -mtime +0 -type f -exec /usr/bin/rm -f {} \; `'
                ;;
esac

# clean out ${NATIVE}var/tmp
if [ -d ${NATIVE}var/tmp ]
then
  /usr/bin/find ${NATIVE}var/tmp -atime +0 -mtime +0 -type f -exec /usr/bin/rm {} \;
fi

# get rid of news items older than 45 days
if [ -d ${NATIVE}var/news ]
then
  /usr/bin/find ${NATIVE}var/news -mtime +44 -type f -exec /usr/bin/rm {} \;
fi

# get rid of *.bak, .*.bak, a.out, core, proof, galley, ...*, ed.hup files
# that are more than a day old.
# proof and galley files must not be owner-writable
#
# Use the -xdev flag to prevent find from traversing a filesystem
# on a different device, this will prevent it from searching nfs
# mounted filesystems.  You may want to add filesystems here
# that you want to be cleaned up.
/usr/bin/find $NATIVE ${NATIVE}usr ${NATIVE}var ${NATIVE}tmp ${NATIVE}home \
    \(  \(  \( -name "*.bak" -o -name core     -o -name a.out -o    \
	       -name "...*"  -o -name ".*.bak" -o -name ed.hup \)   \
	    -atime +0 -mtime +0 -type f                             \
	\)                                                          \
	-o                                                          \
	\(  \( -name proof -o -name galley \)                       \
	    -atime +0 -mtime +0 -type f ! -perm -0200               \
	\)                                                          \
    \) -xdev -exec /usr/bin/rm  -f {} \;

# get rid of anything in a .putdir directory more than a day old.
for i in `/usr/bin/find $NATIVE ${NATIVE}usr ${NATIVE}var ${NATIVE}tmp \
	${NATIVE}home -xdev -type d -name ".putdir" -print`
do
	/usr/bin/find $i -mtime +0 -type f -exec /usr/bin/rm {} \;
done

date=`/usr/bin/date`
if msg=`/usr/bin/dspmsg skulker.cat 2 '\n%1$s finished at %2$s on %3$s\n' $0 "$date" "$uname"`
then :
else msg='/usr/bin/echo "\n$0 finished at $date on $uname\n"'; fi
eval /usr/bin/echo $msg
