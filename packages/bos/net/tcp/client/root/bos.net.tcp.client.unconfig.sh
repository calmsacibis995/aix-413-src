#!/usr/bin/ksh
# @(#)15	1.5  src/packages/bos/net/tcp/client/root/bos.net.tcp.client.unconfig.sh, pkgtcpip, pkg41J, 9512A_all 3/7/95 14:20:21
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
# This removes the ODM stuff that we need for SRC
#
# ============================================================#
delete_db() 
{
	let rc=0
	odmdelete -o CuDv -q "name LIKE 'sl[0-9]*'" || let rc=1
	odmdelete -o CuAt -q "name LIKE 'sl[0-9]*'" || let rc=1
	odmdelete -o CuDep -q "name LIKE 'sl[0-9]*'" || let rc=1
	return $rc
}

cleanup()
{
	rm $OLDFILE $NEWFILE >/dev/null 2>&1
	exit $1
}

/bin/rmssys -s gated
/bin/rmssys -s named
/bin/rmssys -s routed
/bin/rmssys -s rwhod
/bin/rmssys -s iptrace
/bin/rmssys -s syslogd
/bin/rmssys -s timed
/bin/rmssys -s sendmail
/bin/rmssys -s snmpd
/bin/rmserver -t ftp
/bin/rmserver -t uucp
/bin/rmserver -t telnet
/bin/rmserver -t shell
/bin/rmserver -t login
/bin/rmserver -t exec
/bin/rmserver -t finger
/bin/rmserver -t tftp
/bin/rmserver -t ntalk
/bin/rmserver -t echo
/bin/rmserver -t discard
/bin/rmserver -t chargen
/bin/rmserver -t daytime
/bin/rmserver -t time
/bin/rmserver -t comsat
/bin/rmserver -t bootps
/bin/rmserver -t systat
/bin/rmserver -t netstat
/bin/rmssys -s inetd

/usr/sbin/rmitab rctcpip

# Remove sliplogin from the list of valid login shells.
SECFILE=/etc/security/login.cfg
SECCOPY=${INUTEMPDIR}/cfgcpy.${$}
UPDATED=${INUTEMPDIR}/updated.${$}
FN=bos.net.tcp.client.unconfig
cp -p $SECFILE $SECCOPY
if [ $? -ne 0 ] ; then
	# Don't try to change login.cfg if you can't even copy it.
	echo "$FN: Cannot copy $SECFILE.  Exiting with failure."
	exit 1
fi
if grep "shells =.*" $SECFILE | grep -v "\*.*shells =.*" | grep "/usr/sbin/sliplogin" > /dev/null ; then
	SLIPLOGIN_THERE=1
else
	SLIPLOGIN_THERE=0
	rm -f $SECCOPY
fi

if [ $SLIPLOGIN_THERE -eq 1 ] ; then
	ENTRYCOUNT=`grep "shells =.*" $SECFILE | grep -v "\*.*shells =.*" | grep "/usr/sbin/sliplogin" | wc -l`
	if [ $ENTRYCOUNT -gt 1 ] ; then
		# Multiple "shells =" entries.  Better safe than sorry.
		echo "$FN: Located multiple \"shells =\" entries in $SECFILE.  Exiting with failure."
		exit 1
	fi

	awk '{ \
		matched=match($0,"shells =.*"); \
		if (matched==0) {print $0} \
		else { \
			matched=match($0,/\*.*shells =.*/); \
			if (matched==0) { \
				sub(",/usr/sbin/sliplogin", "") ; \
				print $0 \
			} \
			else { print $0 } \
		} \
	}' $SECCOPY > $UPDATED

	if [ $? -ne 0 ] ; then
		# Awk messed up.  Don't overwrite login.cfg
		rm -f $SECCOPY $UPDATED
		echo "$FN: Unsuccessful in removing the /usr/sbin/sliplogin login shell from $SECFILE.  Exiting with failure."
		exit 1
	fi
	mv $UPDATED $SECFILE
	chmod 660 $SECFILE && chown root.security $SECFILE
	if [ $? -ne 0 ] ; then
		# File overwrite messed up.  Try to put the original back.
		mv $SECCOPY $SECFILE
		chmod 660 $SECFILE ; chown root.security $SECFILE
		rm -f $UPDATED
		echo "$FN: Unsuccessful in modifying $SECFILE file. Exiting with failure."
		exit 1
	fi
	chmod 660 $SECFILE && chown root.security $SECFILE
	if [ $? -ne 0 ] ; then
		echo "$FN: Could not perform chmod on $SECFILE file. Exiting with failure."
		exit 1
	fi
	rm $SECCOPY
fi

#------------------------------------------------
# Perform any slip interface deletion if needed.-
#------------------------------------------------
#kill all the running slattach/sliplogin commands.
for pid in `ps -ef | grep "slattach[\t ]" | awk '{ print $2 }'` ; do
	kill -15 $pid
done
for pid in `ps -ef | grep sliplogin | grep -v grep | awk '{ print $2 }'` ; do
	kill -15 $pid
done

OLDFILE=${INUTEMPDIR}/slip_old.add
NEWFILE=${INUTEMPDIR}/slip_new.add

if [ -f $OLDFILE ] # OLDFILE there, we need to clean up for failed install
then
	if [ -s $OLDFILE ] 
	then 
		delete_db || cleanup 1
		odmadd $OLDFILE
		if [[ $? -ne 0 ]] 
		then
			delete_db
			cleanup 1
		fi
	fi
	cleanup 0
fi

if [ -f $NEWFILE ] # NEWFILE there, install failed; but did not do any damage
then
	cleanup 0
fi

let unit=0
# If we reach here then we must be invoked from reject process.
for i in `lsdev -C -s SL -t sl | awk '{print $1}'`
do
	if [[ $unit > 7 ]] #Old slip supported only 8 devices
	then
		continue;
	fi

	let RC=0
	odmget -q "name='$i'" CuDv | sed -e "s/$i/sl$unit/" >> $NEWFILE
	let RC=$RC+$?
	odmget -q "name='$i'" CuAt | sed -e "s/$i/sl$unit/" >> $NEWFILE
	let RC=$RC+$?
	odmget -q "name='$i'" CuDep | sed -e "s/$i/sl$unit/" >> $NEWFILE
	let RC=$RC+$?

	if [[ $RC > 0 ]] then
		delete_db
		cleanup 1
	fi

	let unit=$unit+1
done

if [ -s $NEWFILE ] 
then 
	delete_db  || cleanup 1
	odmadd $NEWFILE || delete_db
fi
cleanup 0
