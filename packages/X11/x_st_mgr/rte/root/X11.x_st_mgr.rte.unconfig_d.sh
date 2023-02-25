#!/bin/bsh
# @(#)57	1.2  src/packages/X11/x_st_mgr/rte/root/X11.x_st_mgr.rte.unconfig_d.sh, pkgxstmgr, pkg411, GOLD410 6/30/94 11:22:02
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
# Handle the /etc part of the config files

if [ -s /etc/$programname/"$programname"d.cf ]
then
        /usr/sbin/inuumsg $MSG_CONFIG
else
	if [ ! -f /etc/$programname/"$programname"d.cf ]
	then
        	rm -f /etc/$programname/"$programname"d.cf
        	touch /etc/$programname/"$programname"d.cf
        	chmod 600 /etc/$programname/"$programname"d.cf
	fi
fi
if [ ! -f /etc/$programname/"$programname"d.tmty ]
then
       	rm -f /etc/$programname/"$programname"d.tmty
       	touch /etc/$programname/"$programname"d.tmty
       	chmod 600 /etc/$programname/"$programname"d.tmty
fi

# Handle the /usr part of the config files

if cmp -s /etc/$programname/"$programname"d.cf \
	  /usr/lpp/$programname/bin/"$programname"d.cf 2>/dev/null 1>/dev/null
then
:
else
        rm -f /usr/lpp/$programname/bin/"$programname"d.cf
       	ln -s /etc/$programname/"$programname"d.cf \
       	    /usr/lpp/$programname/bin/"$programname"d.cf 2>/dev/null 1>/dev/null
fi
if cmp -s /etc/$programname/"$programname"d.tmty \
	  /usr/lpp/$programname/bin/"$programname"d.tmty 2>/dev/null 1>/dev/null
then
:
else
        rm -f /usr/lpp/$programname/bin/"$programname"d.tmty
       	ln -s /etc/$programname/"$programname"d.tmty \
       	    /usr/lpp/$programname/bin/"$programname"d.tmty 2>/dev/null 1>/dev/null
fi

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
# Remove the font from the x_st_mgrd.cf if it is Rom14.  This will allow the
# Aixterm app-defaults file to take over and provide a standard font in the
# correct locale.
#

sed -e "s/-fn 'Rom14'//" </etc/x_st_mgr/x_st_mgrd.cf >/tmp/$$tmptmp
mv /tmp/$$tmptmp /etc/x_st_mgr/x_st_mgrd.cf
sed -e "s/-fn Rom14 //" </etc/x_st_mgr/x_st_mgrd.cf >/tmp/$$tmptmp
mv /tmp/$$tmptmp /etc/x_st_mgr/x_st_mgrd.cf


rm -f /tmp/inetd.conf
rm -f /tmp/services
exit 0
