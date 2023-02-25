#!/bin/ksh
# @(#)95	1.10  src/packages/bos/net/tcp/client/root/bos.net.tcp.client.post_i.sh, pkgtcpip, pkg411, GOLD410 7/7/94 16:29:27
#
#   COMPONENT_NAME: pkgtcp
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
#

#
# When installing 410 -> 410, we need to move the old inetd.conf file back
# into /etc.  If installing 325 -> 410, leave the inetd.conf file in place
# of the saved files so that the user can deal with it.  This is done so 
# that this will work correctly on diskless stations.
#
# The following files need to be preserve so that the scripts will
# work properly for migration.  preserve will cause the new file to be
# erased while the old file is returned to its proper place.  This is ok,
# because the scripts will specifically save off the files.
# 
# /etc/inetd.conf
# /etc/rc.tcpip
# /etc/rc.net
# /etc/rc.bsdnet
# /etc/3270.keys
# /etc/3270keys.hft
# /etc/map3270
# /etc/services
# /etc/bootptab
# /etc/rpc
#
# If it is a 3.2 -> 4.1 migration, the old file will be saved off with 
# the new file and a merged version will be placed out in the proper place.
#
# If it is a 4.1 -> 4.1 migration, the new file will be saved off and the 
# old will be placed out in the proper place.  The old file will be saved off
# in the same location as well.
#
# In either case, the old file will be saved as filename.old under the 
# proper path and the new file will be saved as filename.new in the same
# directory.
#

if [ -n "$MIGSAVE" ] && [ -n "$INSTALLED_LIST" ]
then
    grep -q bos.net.tcp.client $INSTALLED_LIST
    rc2=$?

    if [ $rc2 -eq 0 ]
    then
	#
	# This is a migration for 4.1 -> 4.1 or beyond.
	# Do nothing
	# Preserve will handle it.
	# Exit now, before we start 320 -> 410 migration.
	exit 0
    fi

    ##
    ## Make sure the /$MIGSAVE/etc exists
    ##
    mkdir -p $MIGSAVE/etc > /dev/null 2>&1 

    ## 
    ## This is the inetd.conf 320 -> 410 migration section.
    ##
    ## Current Assumptions:
    ##     Willing to lose comments in old inetd.conf file.
    ##
    ##
if [ -f $MIGSAVE/etc/inetd.conf ]
then
    OUTPUTFILE=$MIGSAVE/etc/inetd.conf
    TMPFILE=$MIGSAVE/etc/inetd.conf.1t$$
    TMPFILE1=$MIGSAVE/etc/inetd.conf.2t$$
    OLDFILE=$MIGSAVE/etc/inetd.conf
    NEWFILE=/etc/inetd.conf
    OLDSAVEFILE=$MIGSAVE/etc/inetd.conf.old
    NEWSAVEFILE=$MIGSAVE/etc/inetd.conf.new

    ## 
    ## Set up initial file place and save off copies.
    ##
    cp -p $OLDFILE $TMPFILE
    cp -p $OLDFILE $OLDSAVEFILE
    cp -p $NEWFILE $NEWSAVEFILE
    cp -p $NEWFILE $OUTPUTFILE

    ##
    ## These commands remove comments from the copy of the old file
    ##
    fgrep -v "# " $TMPFILE > $TMPFILE1
    fgrep -v "## " $TMPFILE1 > $TMPFILE
    egrep -vx -e "[#]" $TMPFILE > $TMPFILE1
    fgrep -v "#	" $TMPFILE1 > $TMPFILE

    ##
    ## This loop removes lines from the copy of the old file that are common
    ## with the new file.
    ##
    exec 3< $NEWFILE
    while read -u3 -r line
    do
	fgrep -v "$line" $TMPFILE > $TMPFILE1
	mv $TMPFILE1 $TMPFILE
    done
    exec 3<&-

    ##
    ## At this point, TMPFILE has the old entries that are commented out and not
    ## commented out in new file, the old entries that are not commented out and
    ## are commented out in the new file, new entries that are not in the new
    ## file, and entries that are changed from one release to another.
    ##
    ## OUTPUTFILE has the new file in it.
    ##

    exec 3< $TMPFILE

    while read -u3 -r line
    do
	SERVICE=`echo $line | awk '{ print $1 }' | awk -F# '{ print $1 }'`

	if [ -z "$SERVICE" ]
	then
	    SERVICE=`echo $line | awk '{print $1}' | awk -F# '{print $2}'`
	    COMMENT="YES"
	else
	    COMMENT="NO"
	fi

	## SOCKET=`echo $line | awk '{ print $2 }'`
	PROTOCOL=`echo $line | awk '{ print $3 }'`
	WAIT=`echo $line | awk '{ print $4 }'`
	USER=`echo $line | awk '{ print $5 }'`
	## PROGRAM=`echo $line | awk '{ print $6 }'`
	ARGS=`echo $line | awk '{ for (i = 7; i <= NF; i++) print $i }'`
	line2=`egrep "^[#]?$SERVICE[ 	]" $OUTPUTFILE | fgrep $PROTOCOL`
	COUNT=`egrep "^[#]?$SERVICE[ 	]" $OUTPUTFILE | fgrep $PROTOCOL |wc -l`

	## 
	## This may not be need anymore, the only thing it does it remove
	## duplicates.
	##
	## fgrep -v "$line" $TMPFILE > $TMPFILE1
	## mv $TMPFILE1 $TMPFILE
	## exec 3<&-
	## exec 3< $TMPFILE
	##

	if [ $COUNT -eq 0 ]
	then
	    echo "$line" >> $OUTPUTFILE
	else
	    ## At this point, we have a service and a protocol unique 
	    ## entry in both the new and the old tmp files.  The new file
	    ## entry needs to be broken down into pieces and compared.
	    ## Everything should be compared and the corresponding final 
	    ## variable should be set.
	
	    SERVICE2=`echo $line2 | awk '{print $1}' | awk -F# '{print $1}'`

	    if [ -z "$SERVICE2" ]
	    then
		SERVICE2=`echo $line2 | awk '{print $1}' | awk -F# '{print $2}'`
		COMMENT2="YES"
	    else
		COMMENT2="NO"
	    fi

	    SOCKET2=`echo $line2 | awk '{ print $2 }'`
	    ## PROTOCOL2=`echo $line2 | awk '{ print $3 }'`
	    ## WAIT2=`echo $line2 | awk '{ print $4 }'`
	    ## USER2=`echo $line2 | awk '{ print $5 }'`
	    PROGRAM2=`echo $line2 | awk '{ print $6 }'`
	    ## ARGS2=`echo $line2 | awk '{for (i = 7; i <= NF; i++) print $i}'`

	    ##
	    ## Assumptions: 
	    ##    1.  If it is uncommented in the new file and not in the
	    ##        old file, the new file has precedence.  If it is 
	    ##        uncommented in the old file and commented in the
	    ##        new file, then the line should be uncommented.
	    ##        If commenting scheme matches, leave alone.
	    ##
	    ##    2.  Always use the new version of the socket type
	    ##
	    ##    3.  Always use the WAIT from the old file
	    ##
	    ##    4.  Always use the User from the old file
	    ##
	    ##    5.  Always use the Program from the new file
	    ##
	    ##    6.  Always use the Args from the old file
	    ##
	    if [ "$COMMENT" = "$COMMENT2" ]
	    then
		QCOMMENT=$COMMENT
	    else
		QCOMMENT="NO"
	    fi

	    FSOCKET=$SOCKET2
	    FWAIT=$WAIT
	    FUSER=$USER
	    FPROGRAM=$PROGRAM2
	    FARGS=$ARGS

	    ## Make $line3
	    if [ "$QCOMMENT" = "NO" ]
	    then 
		FCOMMENT=""
	    else
		FCOMMENT="#"
	    fi

	    line3=`echo $FCOMMENT$SERVICE"	"$FSOCKET"	"$PROTOCOL"	"$FWAIT"	"$FUSER"	"$FPROGRAM"	"$FARGS`

	    sed "s:$line2:$line3:g" $OUTPUTFILE > $TMPFILE1
	    mv $TMPFILE1 $OUTPUTFILE
	fi
    done

    exec 3<&-

    rm -f $TMPFILE $TMPFILE1

fi
    ##
    ## End of inetd.conf
    ##


    ## 
    ## This is the rc.tcpip merge section
    ##
    ##
    ## This script will only change the things that I know will be different.
    ## That is to say only our 410 changes will be done.
    ## In fact, this script doesn't need the 410 file.  It is saved off only.
    ##
if [ -f $MIGSAVE/etc/rc.tcpip ]
then
    OUTPUTFILE=$MIGSAVE/etc/rc.tcpip
    TMPFILE1=$MIGSAVE/etc/rc.tcpip.1t$$
    OLDFILE=$MIGSAVE/etc/rc.tcpip
    NEWFILE=/etc/rc.tcpip
    NEWSAVEFILE=$MIGSAVE/etc/rc.tcpip.new
    OLDSAVEFILE=$MIGSAVE/etc/rc.tcpip.old

    ##
    ## Save off old file and save off new file
    ##
    cp -p $OLDFILE $OLDSAVEFILE
    cp -p $NEWFILE $NEWSAVEFILE
    
    ##
    ## Change the path names.
    ## 
    sed "s/\/etc\/inetd/\/usr\/sbin\/inetd/g" $OUTPUTFILE > $TMPFILE1
    sed "s/\/etc\/named/\/usr\/sbin\/named/g" $TMPFILE1 > $OUTPUTFILE
    sed "s/\/etc\/timed/\/usr\/sbin\/timed/g" $OUTPUTFILE > $TMPFILE1
    sed "s/\/etc\/rwhod/\/usr\/sbin\/rwhod/g" $TMPFILE1 > $OUTPUTFILE
    sed "s/\/etc\/gated/\/usr\/sbin\/gated/g" $OUTPUTFILE > $TMPFILE1
    sed "s/\/etc\/routed/\/usr\/sbin\/routed/g" $TMPFILE1 > $OUTPUTFILE
    sed "s/\/etc\/syslogd/\/usr\/sbin\/syslogd/g" $OUTPUTFILE > $TMPFILE1
    sed "s/\/usr\/lpd\/lpd/\/usr\/sbin\/lpd/g" $TMPFILE1 > $OUTPUTFILE
    sed "s/\/usr\/etc\/portmap/\/usr\/sbin\/portmap/g" $OUTPUTFILE > $TMPFILE1
    mv $TMPFILE1 $OUTPUTFILE
    
    ##
    ## Replace the nfs portmap code.
    ##
    sed "s/mount | grep ' \/usr \*nfs' 2>&1 > \/dev\/null/USR_NFS=\`mount | awk '{if(\$3==\"\/usr\") print \$4}'\`/g" $OUTPUTFILE > $TMPFILE1
    sed "s/if \[ \"\$?\" -ne 0 \]/if \[ \"\$USR_NFS\" != \"nfs\" \]/g" $TMPFILE1 > $OUTPUTFILE

    ## 
    ## Make sure that portmap is uncommented
    ##
    sed "s/#start \/usr\/sbin\/portmap/start \/usr\/sbin\/portmap/g" $OUTPUTFILE > $TMPFILE1
    mv $TMPFILE1 $OUTPUTFILE

    rm -f $TMPFILE1 $TMPFILE

    ##
    ## End of rc.tcpip merge section
    ##
fi


    ## 
    ## This is the rc.net section
    ##
    ## Only do this on a 325 -> 410 install.
    ##
    ##
if [ -f $MIGSAVE/etc/rc.net ]
then
    OUTPUTFILE=$MIGSAVE/etc/rc.net
    TMPFILE=$MIGSAVE/etc/rc.net.1t$$
    TMPFILE1=$MIGSAVE/etc/rc.net.2t$
    OLDFILE=$MIGSAVE/etc/rc.net
    NEWFILE=/etc/rc.net
    OLDSAVE=$MIGSAVE/etc/rc.net.old
    NEWSAVE=$MIGSAVE/etc/rc.net.new

    ##
    ## Save off old file and save off new file
    ##
    cp -p $NEWFILE $NEWSAVE
    cp -p $OLDFILE $OLDSAVE

    ##
    ## Delete the old x25 and serial pieces and add the new x25 and serial code
    ##
    START_LINE=`grep -n "#  Special X25 and SLIP handling" $OUTPUTFILE | awk -F: '{ print $1 }'`
    STOP_LINE=`grep -n "# Configure the Internet protocol kernel extension (netinet):" $OUTPUTFILE | awk -F: '{ print $1 }'`

    if [ $START_LINE -gt 0 -a $STOP_LINE -gt $START_LINE ]; then
	    STOP_LINE=`echo $STOP_LINE"d"`
	    sed "$START_LINE,$STOP_LINE" $OUTPUTFILE > $TMPFILE1
    fi
    mv $TMPFILE1 $OUTPUTFILE

    ##
    ## Make $OUTPUTFILE look like $NEWFILE, except don't del or change things.
    ##
    diff -be $OUTPUTFILE $NEWFILE > $TMPFILE
    egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)d" $TMPFILE > $TMPFILE1

    exec 3< $TMPFILE1
    STOPPED="NO"
    rm -f $TMPFILE
    while read -u3 -r line
    do
	RC=`echo "$line" | egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)c"`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="YES"
	fi

	if [ "$STOPPED" = "NO" ]
	then
	    echo "$line" >> $TMPFILE
	fi

	RC=`echo "$line" | egrep -xv "\."`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="NO"
	fi
    done
    exec 3<&-

    if [ -f $TMPFILE ]
    then
	(cat $TMPFILE ; echo '1,$p') | ed - $OUTPUTFILE > $TMPFILE1
	mv $TMPFILE1 $OUTPUTFILE
    fi

    ##
    ## change the xt references to xs, in ifconfigs
    ##
    sed "s/ifconfig xt/ifconfig xs/g" $OUTPUTFILE > $TMPFILE1
    sed "s/ xt=/ xs=/g" $TMPFILE1 > $OUTPUTFILE

    rm -f $TMPFILE $TMPFILE1

fi
    ##
    ## This is the end of the rc.net section
    ##

    ##
    ## This is the rc.bsdnet section
    ##
    ##
    ## Only do this on a 325 -> 410 install.
    ##
if [ -f $MIGSAVE/etc/rc.bsdnet ]
then
    OUTPUTFILE=$MIGSAVE/etc/rc.bsdnet
    TMPFILE=$MIGSAVE/etc/rc.bsdnet.1t$$
    TMPFILE1=$MIGSAVE/etc/rc.bsdnet.2t$$
    OLDFILE=$MIGSAVE/etc/rc.bsdnet
    NEWFILE=/etc/rc.bsdnet
    NEWSAVE=$MIGSAVE/etc/rc.bsdnet.new
    OLDSAVE=$MIGSAVE/etc/rc.bsdnet.old
    
    ##
    ## Save off old file and save off new file
    ##
    cp -p $OLDFILE $OLDSAVE
    cp -p $NEWFILE $NEWSAVE

    ##
    ## Make $OUTPUTFILE look like $NEWFILE, except don't del or change things.
    ##
    diff -be $OUTPUTFILE $NEWFILE > $TMPFILE
    egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)d" $TMPFILE > $TMPFILE1
    
    exec 3< $TMPFILE1
    STOPPED="NO"
    rm -rf $TMPFILE
    while read -u3 -r line
    do
	RC=`echo "$line" | egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)c"`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="YES"
	fi

	if [ "$STOPPED" = "NO" ]
	then
	    echo "$line" >> $TMPFILE
	fi

	RC=`echo "$line" | egrep -xv "\."`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="NO"
	fi
    done
    exec 3<&-

    if [ -f $TMPFILE ]
    then
	(cat $TMPFILE ; echo '1,$p') | ed - $OUTPUTFILE > $TMPFILE1
	mv $TMPFILE1 $OUTPUTFILE
    fi

    ##
    ## change the xt references to xs, in ifconfigs
    ##
    sed "s/ifconfig xt/ifconfig xs/g" $OUTPUTFILE > $TMPFILE1
    sed "s/ xt=/ xs=/g" $TMPFILE1 > $OUTPUTFILE

    rm -f $TMPFILE $TMPFILE1
fi
    ## 
    ## The end of the rc.bsdnet section
    ##

    ##
    ## This is the bootptab section
    ##
    ## Only do this on a 325 -> 410 install.
    ##
    ##
if [ -f $MIGSAVE/etc/bootptab ] 
then
    OUTPUTFILE=$MIGSAVE/etc/bootptab
    TMPFILE=$MIGSAVE/etc/bootptab.1t$$
    TMPFILE1=$MIGSAVE/etc/bootptab.2t$$
    OLDFILE=$MIGSAVE/etc/bootptab
    NEWFILE=/etc/bootptab
    NEWSAVE=$MIGSAVE/etc/bootptab.new
    OLDSAVE=$MIGSAVE/etc/bootptab.old

    ##
    ## Save off old file and save off new file
    ##
    cp -p $OLDFILE $OLDSAVE
    cp -p $NEWFILE $NEWSAVE
    cp -p $NEWFILE $OUTPUTFILE

    ##
    ## Make $OUTPUTFILE look like $NEWFILE, except don't del or change things.
    ##
    diff -be $OUTPUTFILE $OLDSAVE > $TMPFILE
    egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)d" $TMPFILE > $TMPFILE1

    exec 3< $TMPFILE1
    STOPPED="NO"
    rm -f $TMPFILE
    while read -u3 -r line
    do
	RC=`echo "$line" | egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)c"`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="YES"
	fi

	if [ "$STOPPED" = "NO" ]
	then
	    echo "$line" >> $TMPFILE
	fi

	RC=`echo "$line" | egrep -xv "\."`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="NO"
	fi
    done
    exec 3<&-

    if [ -f $TMPFILE ]
    then
	(cat $TMPFILE ; echo '1,$p') | ed - $OUTPUTFILE > $TMPFILE1
	mv $TMPFILE1 $OUTPUTFILE
    fi

    rm -f $TMPFILE $TMPFILE1
fi
    ## 
    ## This is the end of the bootptab section
    ##

    ##
    ## This is the beginning of the 3270.keys section
    ##
    ##
    ## Only do this on a 325 -> 410 install.
    ##
    ##
if [ -f $MIGSAVE/etc/3270.keys ] 
then
    OUTPUTFILE=$MIGSAVE/etc/3270.keys
    TMPFILE=$MIGSAVE/etc/3270.keys.1t$$
    TMPFILE1=$MIGSAVE/etc/3270.keys.2t$$
    OLDFILE=$MIGSAVE/etc/3270.keys
    NEWFILE=/etc/3270.keys
    NEWSAVE=$MIGSAVE/etc/3270.keys.new
    OLDSAVE=$MIGSAVE/etc/3270.keys.old

    ##
    ## Save off old file and save off new file
    ##
    cp -p $NEWFILE $NEWSAVE
    cp -p $OLDFILE $OLDSAVE

    ##
    ## Add lft stanza
    ##
    sed "s/hft-old/hft-old or lft/" $OUTPUTFILE > $TMPFILE
    mv $TMPFILE $OUTPUTFILE

    ##
    ## Make $OUTPUTFILE look like $NEWFILE, except don't del or change things.
    ##
    diff -be $OUTPUTFILE $NEWFILE > $TMPFILE
    egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)d" $TMPFILE > $TMPFILE1

    exec 3< $TMPFILE1
    STOPPED="NO"
    rm -f $TMPFILE
    while read -u3 -r line
    do
	RC=`echo "$line" | egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)c"`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="YES"
	fi

	if [ "$STOPPED" = "NO" ]
	then
	    echo "$line" >> $TMPFILE
	fi

	RC=`echo "$line" | egrep -xv "\."`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="NO"
	fi
    done
    exec 3<&-

    if [ -f $TMPFILE ]
    then
	(cat $TMPFILE ; echo '1,$p') | ed - $OUTPUTFILE > $TMPFILE1
	mv $TMPFILE1 $OUTPUTFILE
    fi

    rm -f $TMPFILE $TMPFILE1
fi
    ##
    ## This is the end of the 3270.keys section
    ##

    ##
    ## This is the beginning of the 3270keys.hft section
    ##
    ## Only do this on a 325 -> 410 install.
    ##
if [ -f $MIGSAVE/etc/3270keys.hft ]
then
    OUTPUTFILE=$MIGSAVE/etc/3270keys.hft
    TMPFILE=$MIGSAVE/etc/3270keys.hft.1t$$
    TMPFILE1=$MIGSAVE/etc/3270keys.hft.2t$$
    OLDFILE=$MIGSAVE/etc/3270keys.hft
    NEWFILE=/etc/3270keys.hft
    NEWSAVE=$MIGSAVE/etc/3270keys.hft.new
    OLDSAVE=$MIGSAVE/etc/3270keys.hft.old

    ##
    ## Save off old file and save off new file
    ##
    cp -p $OLDFILE $OLDSAVE
    cp -p $NEWFILE $NEWSAVE


    ##
    ## Make $OUTPUTFILE look like $NEWFILE, except don't del or change things.
    ##
    diff -be $OUTPUTFILE $NEWFILE > $TMPFILE
    egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)d" $TMPFILE > $TMPFILE1

    exec 3< $TMPFILE1
    STOPPED="NO"
    rm -f $TMPFILE
    while read -u3 -r line
    do
	RC=`echo "$line" | egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)c"`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="YES"
	fi

	if [ "$STOPPED" = "NO" ]
	then
	    echo "$line" >> $TMPFILE
	fi

	RC=`echo "$line" | egrep -xv "\."`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="NO"
	fi
    done
    exec 3<&-

    if [ -f $TMPFILE ]
    then
	(cat $TMPFILE ; echo '1,$p') | ed - $OUTPUTFILE > $TMPFILE1
	mv $TMPFILE1 $OUTPUTFILE
    fi

    rm -f $TMPFILE $TMPFILE1
fi
    ##
    ## This is the end of the 3270keys.hft section
    ##

    ##
    ## This is the beginning of the map3270 section
    ##
    ## Only do this on a 325 -> 410 install.
    ##
if [ -f $MIGSAVE/etc/map3270 ]
then
    OUTPUTFILE=$MIGSAVE/etc/map3270
    TMPFILE=$MIGSAVE/etc/map3270.1t$$
    TMPFILE1=$MIGSAVE/etc/map3270.2t$$
    OLDFILE=$MIGSAVE/etc/map3270
    NEWFILE=/etc/map3270
    NEWSAVE=$MIGSAVE/etc/map3270.new
    OLDSAVE=$MIGSAVE/etc/map3270.old

    ##
    ## Save off old file and save off new file
    ##
    cp -p $OLDFILE $OLDSAVE
    cp -p $NEWFILE $NEWSAVE

    ##
    ## Add lft stanza
    ##
    sed "s/hft-old/hft-old | lft/" $OUTPUTFILE > $TMPFILE
    mv $TMPFILE $OUTPUTFILE

    ##
    ## Make $OUTPUTFILE look like $NEWFILE, except don't del or change things.
    ##
    diff -be $OUTPUTFILE $NEWFILE > $TMPFILE
    egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)d" $TMPFILE > $TMPFILE1

    exec 3< $TMPFILE1
    STOPPED="NO"
    rm -f $TMPFILE
    while read -u3 -r line
    do
	RC=`echo "$line" | egrep -xv "([1-9][0-9]*|[1-9][0-9]*,[1-9][0-9]*)c"`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="YES"
	fi

	if [ "$STOPPED" = "NO" ]
	then
	    echo "$line" >> $TMPFILE
	fi

	RC=`echo "$line" | egrep -xv "\."`
	RC1=$?
	if [ "$RC" = "" -a "$RC1" = "1" ]
	then
	    STOPPED="NO"
	fi
    done
    exec 3<&-

    if [ -f $TMPFILE ]
    then
	(cat $TMPFILE ; echo '1,$p') | ed - $OUTPUTFILE > $TMPFILE1
	mv $TMPFILE1 $OUTPUTFILE
    fi

    rm -f $TMPFILE $TMPFILE1
fi
    ##
    ## This is the end of the map3270 section
    ##

    ##
    ## This is the beginning of the services section
    ##
    ## Only do this on a 325 -> 410 install.
    ##
if [ -f $MIGSAVE/etc/services ] 
then
	OUTPUTFILE=$MIGSAVE/etc/services
	TMPFILE1=$MIGSAVE/etc/services.1t$$
	TMPFILE2=$MIGSAVE/etc/services.2t$$
	TMPFILE3=$MIGSAVE/etc/services.3t$$
	TMPFILE4=$MIGSAVE/etc/services.4t$$
	OLDFILE=$MIGSAVE/etc/services
	NEWFILE=/etc/services
	NEWSAVE=$MIGSAVE/etc/services.new
	OLDSAVE=$MIGSAVE/etc/services.old

	cp -p $OLDFILE $OLDSAVE
	cp -p $NEWFILE $NEWSAVE
	cp -p $NEWFILE $OUTPUTFILE

	egrep -v '^#' $OLDSAVE > $TMPFILE1

	rm -f $TMPFILE2
	rm -f $TMPFILE3
	exec 3< $TMPFILE1
	while read -u3 -r line
	do
		PROTPORT=`echo "$line" | awk '{ print $2 }'`
		grep -q "$PROTPORT" $OUTPUTFILE
		if [ $? -ne 0 ]
		then
			echo "$line" >> $TMPFILE2
		else
			NAME=`echo "$line" | awk '{ print $1 }'`
			grep "$PROTPORT" $OUTPUTFILE | grep -q "$NAME"
			if [ $? -ne 0 ]
			then
				line2=`grep "$PROTPORT" $OUTPUTFILE`
				grep -v "$line2" $OUTPUTFILE > $TMPFILE4
				mv $TMPFILE4 $OUTPUTFILE
				echo "#$line2" >> $TMPFILE3
				echo "$line" >> $TMPFILE3
			fi
		fi
	done
	exec 3<&-

	if [ -f $TMPFILE2 ]
	then
		cat $TMPFILE2 >> $OUTPUTFILE
	fi

	if [ -f $TMPFILE3 ]
	then
		cat $TMPFILE3 >> $OUTPUTFILE
	fi

	rm -f $TMPFILE1 $TMPFILE2 $TMPFILE3 $TMPFILE4
fi
    ##
    ## This is the end of the services section
    ##

    ##
    ## This is the beginning of the rpc section
    ##
    ## Only do this on a 325 -> 410 install.
    ##
if [ -f $MIGSAVE/etc/rpc ] 
then
	OUTPUTFILE=$MIGSAVE/etc/rpc
	TMPFILE1=$MIGSAVE/etc/rpc.1t$$
	TMPFILE2=$MIGSAVE/etc/rpc.2t$$
	TMPFILE3=$MIGSAVE/etc/rpc.3t$$
	TMPFILE4=$MIGSAVE/etc/rpc.4t$$
	OLDFILE=$MIGSAVE/etc/rpc
	NEWFILE=/etc/rpc
	NEWSAVE=$MIGSAVE/etc/rpc.new
	OLDSAVE=$MIGSAVE/etc/rpc.old

	cp -p $OLDFILE $OLDSAVE
	cp -p $NEWFILE $NEWSAVE
	cp -p $NEWFILE $OUTPUTFILE

	egrep -v '^#' $OLDSAVE > $TMPFILE1

	rm -f $TMPFILE2
	rm -f $TMPFILE3
	exec 3< $TMPFILE1
	while read -u3 -r line
	do
		PROTPORT=`echo "$line" | awk '{ print $2 }'`
		grep -q "$PROTPORT" $OUTPUTFILE
		if [ $? -ne 0 ]
		then
			echo "$line" >> $TMPFILE2
		else
			NAME=`echo "$line" | awk '{ print $1 }'`
			grep "$PROTPORT" $OUTPUTFILE | grep -q "$NAME"
			if [ $? -ne 0 ]
			then
				line2=`grep "$PROTPORT" $OUTPUTFILE`
				grep -v "$line2" $OUTPUTFILE > $TMPFILE4
				mv $TMPFILE4 $OUTPUTFILE
				echo "#$line2" >> $TMPFILE3
				echo "$line" >> $TMPFILE3
			fi
		fi
	done
	exec 3<&-

	if [ -f $TMPFILE2 ]
	then
		cat $TMPFILE2 >> $OUTPUTFILE
	fi

	if [ -f $TMPFILE3 ]
	then
		cat $TMPFILE3 >> $OUTPUTFILE
	fi

	rm -f $TMPFILE1 $TMPFILE2 $TMPFILE3 $TMPFILE4
fi
    ##
    ## This is the end of the rpc section
    ##
fi

exit 0
