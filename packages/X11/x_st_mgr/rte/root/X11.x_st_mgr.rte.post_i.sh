#!/bin/bsh
# @(#)64	1.5  src/packages/X11/x_st_mgr/rte/root/X11.x_st_mgr.rte.post_i.sh, pkgxstmgr, pkg411, GOLD410 7/26/94 18:39:14
# COMPONENT_NAME:  pkgxstmgr
#
# FUNCTIONS:
#
# ORIGINS: 27
# (C) COPYRIGHT International Business Machines Corp. 1990
# All Rights Reserved
# Licensed Material - Property of IBM
# (C) COPYRIGHT Advanced Graphics Engineering 1990
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME:  X11.x_st_mgr.post_i.sh
#
# FUNCTION:  configures inetd.conf and rc.tcpip files
#            called by instal.sh
# fix the /etc/services file and the /etc/inetd.conf files.
# Changes for 4.1 marked with #4.1


programname=x_st_mgr
servicesfile=/etc/services
inetdconf=/etc/inetd.conf
rctcpip=/etc/rc.tcpip
MSG_BOOTPS=77
MSG_BOOTPC=78
WARN_TFTP=79
WARN_ACCNT=80
ERR_RCTCPIP=82
MSG_CONFIG=87
#

cp $servicesfile /tmp/services #4.1
cp $inetdconf /tmp/inetd.conf #4.1
chservices -a -v bootps -p udp -n 67 #4.1
chservices -a -v bootpc -p udp -n 68 #4.1
chsubserver -a -v tftp -p udp #4.1

# Create empty files.
touch /etc/x_st_mgr/x_st_mgrd.tmty
touch /etc/x_st_mgr/x_st_mgrd.cf

egrep "^nobody" /etc/passwd > /dev/null
if test $? != 0
 then
# The message is: Warning - you must add a nobody account.
    /usr/sbin/inuumsg $WARN_ACCNT
fi
#
# add file server to /etc/rc.tcpip
#
fsport=9000
egrep -v "$programname"d $servicesfile | grep "$fsport"/tcp > /dev/null
while test $? = 0
    do
    fsport=`expr $fsport + 1` > /dev/null
    egrep -v "$programname"d $servicesfile | grep "$fsport"/tcp > /dev/null
done
chservices -a -v x_st_mgrd -p tcp -n $fsport
if [ -r $rctcpip ]
then
        egrep "x_st_mgrd" $rctcpip > /dev/null
        if test $? = "0"
                then
                cp $rctcpip /tmp/$$.tmp
                sed -e \\?/usr/lpp/$programname/bin/x_st_mgrd?d /tmp/$$.tmp > $rctcpip
		chmod ug+x $rctcpip
                rm /tmp/$$.tmp
        fi
        echo "/usr/lpp/$programname/bin/x_st_mgrd -b /usr/lpp/$programname/bin/x_st_mgrd.cf -s $programname"d >> $rctcpip
else
        /usr/sbin/inuumsg $ERR_RCTCPIP $rctcpip
#       echo Error, no $rctcpip file.
	cp /tmp/services $servicesfile
	cp /tmp/inetd.conf $inetdconf
        exit -1
fi
chsubserver -a -v bootps -p udp #4.1

# modify /etc/bootptab for the bootfile name changes and the new .cf files
# first save a copy
cp /etc/bootptab /tmp/bootptab
# print warning if "bootfile" is in use
egrep "bf=bootfile:" /etc/bootptab > /dev/null
if test $? = "0"
        then
        echo "WARNING: All XStations booted from this host must have a minimum"
	echo "         of 2 MB DRAM.  To support Multi-byte Character Sets,"
	echo "         DRAM must be 2 MB plus the size of the base MBCS fonts."
fi
awk '
BEGIN { FS=":"} 
/^x_st/ {if (/hd=\/usr\/lpp\/x_st_mgr\/bin:/)
	    {if (/bf=bootfile2/) 
                {suffix = substr($1, index($1, ".")+1)
		 system("cp /usr/lpp/x_st_mgr/bin/bootfile4.cf /etc/x_st_mgr/" suffix ".cf")
		 system("ln -s /usr/lpp/x_st_mgr/bin/bootfile4 /etc/x_st_mgr/" suffix )
		 sub("hd=/usr/lpp/x_st_mgr/bin","hd=/etc/x_st_mgr")
		 sub("bf=bootfile2","bf="suffix)
		 print $0}
	    else
	    if (/bf=bootfile1/)
                {suffix = substr($1, index($1, ".")+1)
		 system("cp /usr/lpp/x_st_mgr/bin/bootfile3.cf /etc/x_st_mgr/" suffix ".cf")
		 system("ln -s /usr/lpp/x_st_mgr/bin/bootfile3 /etc/x_st_mgr/" suffix )
		 sub("hd=/usr/lpp/x_st_mgr/bin","hd=/etc/x_st_mgr")
		 sub("bf=bootfile1","bf="suffix)
		 print $0}
	    else
	    if (/bf=bootfile:/)
                {suffix = substr($1, index($1, ".")+1)
		 system("cp /usr/lpp/x_st_mgr/bin/bootfile3.cf /etc/x_st_mgr/" suffix ".cf")
		 system("ln -s /usr/lpp/x_st_mgr/bin/bootfile3 /etc/x_st_mgr/" suffix )
		 sub("hd=/usr/lpp/x_st_mgr/bin","hd=/etc/x_st_mgr")
		 sub("bf=bootfile","bf="suffix)
		 print $0}
	    else print $0
	    }
	 else print $0} 
$0 !~ /^x_st/
' /etc/bootptab >/tmp/$$tmptmp
mv /tmp/$$tmptmp /etc/bootptab

rm -f /tmp/inetd.conf
rm -f /tmp/services

# change the x_st_mgrd port to the value determined for the port above.
sed -e "/^x_st_mgr/s/T170=1b58/T170=$fsport/" < /etc/bootptab > /tmp/$$tmptmp
mv /tmp/$$tmptmp /etc/bootptab

#
#  Place a dt flag on the end of each network type that is in /etc/booptab that
#does not contain a bt flag.
#
awk '/^x_st_mgr/ && !/:bt:/ {print $0"dt:"}\
     !/^x_st_mgr/ || /:bt:/ {print}' /etc/bootptab >/tmp/$$tmptmp
mv /tmp/$$tmptmp /etc/bootptab

#
# Add the font /usr/lib/X11/fonts/ibm850 to the font paths of all users.
#
awk 'BEGIN {FS = ":"} /^x_st_mgr/ { system("/usr/lpp/x_st_mgr/bin/x_add_nfs_fpe /usr/lib/X11/fonts/ibm850 tftp 1 " $1) }' /etc/bootptab 

exit 0
