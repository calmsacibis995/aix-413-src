#!/usr/bin/ksh
# @(#)75	1.5  src/packages/bos/net/uucp/root/bos.net.uucp.unconfig.sh, pkguucp, pkg41J, 9515A_all 4/7/95 15:34:09
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
FN=bos.net.uucp.unconfig
cp -p $SECFILE $SECCOPY
if [ $? -ne 0 ] ; then
	# Don't try to change login.cfg if you can't even copy it.
	echo "$FN: Cannot copy $SECFILE.  Exiting with failure."
	exit 1
fi
if grep "shells =.*" $SECFILE | grep -v "\*.*shells =.*" | grep "/usr/sbin/uucp/uucico" > /dev/null ; then
	UUCICO_THERE=1
else
	UUCICO_THERE=0
	rm -f $SECCOPY
fi

if [ $UUCICO_THERE -eq 1 ] ; then
	ENTRYCOUNT=`grep "shells =.*" $SECFILE | grep -v "\*.*shells =.*" | grep "/usr/sbin/uucp/uucico" | wc -l`
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
				sub(",/usr/sbin/uucp/uucico", "") ; \
				print $0 \
			} \
			else { print $0 } \
		} \
	}' $SECCOPY > $UPDATED

	if [ $? -ne 0 ] ; then
		# Awk messed up.  Don't overwrite login.cfg
		rm -f $SECCOPY $UPDATED
		echo "$FN: Unsuccessful in removing the /usr/sbin/uucp/uucico login shell from $SECFILE.  Exiting with failure."
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

exit 0
