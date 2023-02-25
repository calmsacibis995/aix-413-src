#!/usr/bin/ksh
# @(#)76	1.5  src/packages/bos/net/uucp/root/bos.net.uucp.config.sh, pkguucp, pkg41J, 9515A_all 4/7/95 15:33:04
#
#   COMPONENT_NAME: pkguucp
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
SECFILE=/etc/security/login.cfg
SECCOPY=${INUTEMPDIR}/cfgcpy.${$}
UPDATED=${INUTEMPDIR}/updated.${$}
FN=bos.net.uucp.config
cp -p $SECFILE $SECCOPY
if [ $? -ne 0 ] ; then
	# Don't try to change login.cfg if you can't even copy it.
	echo "$FN: Cannot copy $SECFILE.  Not adding uucico login shell."
	echo "	Proceeding with installation..."
	# No harm done. Exit with successful return code.
	exit 0
fi

if grep "shells =.*" $SECFILE | grep -v "\*.*shells =.*" | grep "/usr/sbin/uucp/uucico" > /dev/null ; then
	# It's already there.  No need for any change.
	rm -f $SECCOPY
	UUCICO_THERE=1
else
	UUCICO_THERE=0
fi

if [ $UUCICO_THERE -eq 0 ] ; then
	ENTRYCOUNT=`grep "shells =.*" $SECFILE | grep -v "\*.*shells =.*" | wc -l`
	if [ $ENTRYCOUNT -ne 1 ] ; then
		# Multiple or zero "shells =" entries.  Better safe than sorry.
		echo "$FN: Located ${ENTRYCOUNT} \"shells =\" entries in"
		echo "	$SECFILE instead of one and only one.  Not adding"
		echo "	uucico as a login shell.  Proceeding with "
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
				print $0 ",/usr/sbin/uucp/uucico" \
			} \
			else {print $0} \
		} \
	}' $SECCOPY > $UPDATED

	if [ $? -ne 0 ] ; then
		# Awk messed up.  Don't overwrite the original login.cfg
		rm -f $SECCOPY $UPDATED
		echo "$FN: Unsuccessful in adding /usr/sbin/uucp/uucico as a"
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

lsuser ALL | grep "shell.*uucico" > /dev/null || \
lsuser ALL | grep "nuucp" > /dev/null
if [ $? -ne 0 ] ; then
	# Add a login uucp user.
	mkuser -a gecos="uucp login user" pgrp=uucp \
		  home=/var/spool/uucppublic \
		  shell=/usr/sbin/uucp/uucico nuucp
	if [ $? -ne 0 ] ; then
		echo "$FN: Attempt to add user nuucp failed."
		echo "	Exiting with failure."
		exit 1
	fi
fi
exit 0
