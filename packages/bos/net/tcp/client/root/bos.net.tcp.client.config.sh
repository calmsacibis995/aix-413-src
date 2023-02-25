#!/usr/bin/ksh
# @(#)14	1.12  src/packages/bos/net/tcp/client/root/bos.net.tcp.client.config.sh, pkgtcpip, pkg41J, 9512A_all 3/9/95 11:07:57
# COMPONENT_NAME: pkgtcpip
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME:	 bos.net.tcp.client.config.sh
#                                                                    
# FUNCTION: script called from instal to do special tcpip configuration
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  
# ============================================================#
#
# This adds the ODM stuff that we need for SRC
#
# ============================================================#
# If it is there already remove the old and add the new one
# so that the path is correct.
/usr/bin/rmssys -s inetd > /dev/null 2>&1
/usr/bin/mkssys -s inetd -p /usr/sbin/inetd -u 0 -G tcpip
/usr/bin/rmssys -s gated > /dev/null 2>&1
/usr/bin/mkssys -s gated -p /usr/sbin/gated -u 0 -G tcpip
/usr/bin/rmssys -s named > /dev/null 2>&1
/usr/bin/mkssys -s named -p /usr/sbin/named -u 0 -G tcpip
/usr/bin/rmssys -s routed > /dev/null 2>&1
/usr/bin/mkssys -s routed -p /usr/sbin/routed -u 0 -G tcpip
/usr/bin/rmssys -s rwhod > /dev/null 2>&1
/usr/bin/mkssys -s rwhod -p /usr/sbin/rwhod -u 0 -G tcpip
/usr/bin/rmssys -s iptrace > /dev/null 2>&1
/usr/bin/mkssys -s iptrace -p /usr/sbin/iptrace -u 0 -G tcpip
/usr/bin/rmssys -s syslogd > /dev/null 2>&1
/usr/bin/mkssys -s syslogd -p /usr/sbin/syslogd -u 0 -G ras
/usr/bin/rmssys -s timed > /dev/null 2>&1
/usr/bin/mkssys -s timed -p /usr/sbin/timed -u 0 -G tcpip -S -n 1 -f 2 -a "-S"
/usr/bin/rmssys -s sendmail > /dev/null 2>&1
/usr/bin/mkssys -s sendmail -p /usr/lib/sendmail -u 0 -G mail
/usr/bin/rmssys -s snmpd > /dev/null 2>&1
/usr/bin/mkssys -s snmpd -p /usr/sbin/snmpd -u 0 -K -G tcpip
/usr/bin/rmssys -s portmap > /dev/null 2>&1
/usr/bin/mkssys -s portmap -G portmap -p /usr/sbin/portmap \
	-u 0 -i /dev/null -o /dev/console -e /dev/console \
	-O -d -Q -S -n 30 -f 31 -E 20 -w 20
/usr/bin/mkserver -s inetd -t ftp -c 21
/usr/bin/mkserver -s inetd -t uucp -c 540
/usr/bin/mkserver -s inetd -t telnet -c 23
/usr/bin/mkserver -s inetd -t shell -c 514
/usr/bin/mkserver -s inetd -t login -c 513
/usr/bin/mkserver -s inetd -t exec -c 512
/usr/bin/mkserver -s inetd -t finger -c 79
/usr/bin/mkserver -s inetd -t tftp -c 69
/usr/bin/mkserver -s inetd -t ntalk -c 517
/usr/bin/mkserver -s inetd -t echo -c 7
/usr/bin/mkserver -s inetd -t discard -c 9
/usr/bin/mkserver -s inetd -t chargen -c 19
/usr/bin/mkserver -s inetd -t daytime -c 13
/usr/bin/mkserver -s inetd -t time -c 37
/usr/bin/mkserver -s inetd -t comsat -c 1512
/usr/bin/mkserver -s inetd -t bootps -c 67
/usr/bin/mkserver -s inetd -t systat -c 11
/usr/bin/mkserver -s inetd -t netstat -c 15

/usr/sbin/lsitab rctcpip > /dev/null 2>&1
if [ $? -eq 1 ]
then
	/usr/sbin/mkitab -i srcmstr "rctcpip:2:wait:/etc/rc.tcpip > /dev/console 2>&1 # Start TCP/IP daemons"
	if [ $? -eq 1 ]
	then
		echo "/usr/sbin/mkitab failed to add rctcpip to /etc/inittab"
		exit 1
	fi
fi

#---------------------------------------
# Add sliplogin as a valid login shell -
#---------------------------------------
SECFILE=/etc/security/login.cfg
SECCOPY=${INUTEMPDIR}/cfgcpy.${$}
UPDATED=${INUTEMPDIR}/updated.${$}
FN=bos.net.tcp.client.config
cp -p $SECFILE $SECCOPY
if [ $? -ne 0 ] ; then
	# Don't try to change login.cfg if you can't even copy it.
	echo "$FN: Cannot copy $SECFILE.  Not adding sliplogin login shell."
	echo "	Proceeding with installation..."
	# No harm done. Exit with successful return code.
	exit 0
fi

if grep "shells =.*" $SECFILE | grep -v "\*.*shells =.*" | grep "/usr/sbin/sliplogin" > /dev/null ; then
	# It's already there.  No need for any change.
	rm -f $SECCOPY
	SLIPLOGIN_THERE=1
else
	SLIPLOGIN_THERE=0
fi

if [ $SLIPLOGIN_THERE -eq 0 ] ; then
	ENTRYCOUNT=`grep "shells =.*" $SECFILE | grep -v "\*.*shells =.*" | wc -l`
	if [ $ENTRYCOUNT -ne 1 ] ; then
		# Multiple or zero "shells =" entries.  Better safe than sorry.
		echo "$FN: Located ${ENTRYCOUNT} \"shells =\" entries in"
		echo "	$SECFILE instead of one and only one.  Not adding"
		echo "	sliplogin as a login shell.  Proceeding with "
		echo "	installation..."
		# No harm done. Exit with successful return code.
		exit 0
	fi

	awk '{ \
		matched=match($0,"shells =.*"); \
		if (matched==0) {print $0} \
		else { \
			matched=match($0,/\*.*shells =.*/); \
			if (matched==0) { \
				print $0 ",/usr/sbin/sliplogin" \
			} \
			else {print $0} \
		} \
	}' $SECCOPY > $UPDATED

	if [ $? -ne 0 ] ; then
		# Awk messed up.  Don't overwrite the original login.cfg
		rm -f $SECCOPY $UPDATED
		echo "$FN: Unsuccessful in adding /usr/sbin/sliplogin as a"
		echo "	valid login shell to $SECFILE.  Leaving it unmodified."
		echo "	proceeding with installation..."
		# No harm done.  Don't flag an error.
		exit 0
	fi
	mv $UPDATED $SECFILE && chmod 660 $SECFILE && chown root.security $SECFILE
	if [ $? -ne 0 ] ; then
		# File overwrite messed up.  Try to put the original back.
		rm -f $UPDATED
		mv $SECCOPY $SECFILE && chmod 660 $SECFILE && \
					 chown root.security $SECFILE
		if [ $? -eq 0 ] ; then
			echo "$FN: Unsuccessful in modifying $SECFILE file."
			echo "	Leaving it unmodified.  Proceeding with"
			echo "	installation..."
			# No harm done.  Don't flag an error.
			exit 0
		else
			echo "$FN: Unsuccessful in modifying $SECFILE file. "
			echo "	Exiting with failure."
			exit 1
		fi
	fi
	rm -f $SECCOPY
fi

#---------------------------------------
# Remove and re-add slip interfaces if -
# ttynames don't match interface names -
#---------------------------------------

#kill all the running slattach and sliplogin commands.
for pid in `ps -ef | grep "slattach[\t ]" | awk '{ print $2 }'`
do
	kill -15 $pid
done
for pid in `ps -ef | grep sliplogin | grep -v grep | awk '{ print $2 }'`
do
	kill -15 $pid
done

# add the existing device information and modified info in 
# temporary files. OLDFILE is used for cleanup and NEWFILE
# is used for adding the modified entries.

export ODMDIR=/etc/objrepos
OLDFILE=${INUTEMPDIR}/slip_old.add
NEWFILE=${INUTEMPDIR}/slip_new.add
> $OLDFILE
> $NEWFILE

NEED_REDONE=0
for i in `lsdev -C -s SL -t sl | awk '{print $1}'` ; do
	tty=`lsattr -E -F "value" -l $i -a ttyport`  
	if [[  "$tty" = tty[0-9]* ]] ; then
		port=`echo $tty | sed -e 's/tty\([0-9]*\)/\1/'`
		iface=`echo $i | sed -e 's/sl\([0-9]*\)/\1/'`
		if [ $port -ne $iface ] ; then 
			NEED_REDONE=1
		fi
	fi
done

if [ $NEED_REDONE -eq 1 ] ; then
	TTYS_DONE=""
	for i in `lsdev -C -s SL -t sl | awk '{print $1}'` ; do
		let RC=0
		tty=`lsattr -E -F "value" -l $i -a ttyport`  
		if [[  "$tty" = tty[0-9]* ]]	
		then 
			port=`echo $tty | sed -e 's/tty\([0-9]*\)/\1/'`
			odmget -q "name='$i'" CuDv >> $OLDFILE
			let RC=$RC+$?
			odmget -q "name='$i'" CuAt >> $OLDFILE
			let RC=$RC+$?
			odmget -q "name='$i'" CuDep >> $OLDFILE
			let RC=$RC+$?

			# Don't want multiple interfaces on the same tty.
			# weed them out if there are any like this in the
			# old configuration.
			echo "${TTYS_DONE}" | grep "${tty} " > /dev/null 2>&1
			if [ $? -ne 0 ] ; then
				odmget -q "name='$i'" CuDv | \
					sed -e "s/$i/sl$port/" >> $NEWFILE
				let RC=$RC+$?
				odmget -q "name='$i'" CuAt | \
					sed -e "s/$i/sl$port/" >> $NEWFILE
				let RC=$RC+$?
				odmget -q "name='$i'" CuDep | \
					sed -e "s/$i/sl$port/" >> $NEWFILE
				let RC=$RC+$?
				TTYS_DONE="${TTYS_DONE} ${tty} "
			fi
		fi
	
		if [[ $RC -gt 0 ]] then
			# No cleanup rqd, we did not change the databases yet.
			# Do not remove $NEWFILE. This tells unconfig to not do
			# any cleanup of the database.
			rm $OLDFILE 
			exit 1
		fi
	done

	# If anything failed before this, we do not need to cleanup.
	# If something fails after this we need to cleanup (as part of 
	# unconfig).  Cleanup procedure is deleting all the slip entries from 
	# the CuDv, CuAt and CuDep and then adding from the OLDFILE. If cleanup
	# fails, remove all the slip entries from the database and exit.
	let RC=0
	odmdelete -o CuDv -q "name LIKE 'sl[0-9]*'"
	let RC=$RC+$?
	odmdelete -o CuAt -q "name LIKE 'sl[0-9]*'"
	let RC=$RC+$?
	odmdelete -o CuDep -q "name LIKE 'sl[0-9]*'"
	let RC=$RC+$?
	if [[ $RC > 0 ]] 
	then
		rm $NEWFILE
		exit 1
	fi
	
	odmadd $NEWFILE 
	if [[ $? > 0 ]]
	then
		rm $NEWFILE
		exit 1
	fi
fi # End if NEED_REDONE == 1

rm $NEWFILE $OLDFILE >/dev/null 2>&1
exit 0
