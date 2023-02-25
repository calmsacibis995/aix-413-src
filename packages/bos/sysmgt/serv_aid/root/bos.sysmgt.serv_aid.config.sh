# @(#)03        1.10  src/packages/bos/sysmgt/serv_aid/root/bos.sysmgt.serv_aid.config.sh, pkgserv_aid, pkg41B, 9504A 12/13/94 14:03:19 
# 
# COMPONENT_NAME: pkgserv_aid
#
# FUNCTIONS: config  
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Get the copy of the current cron for root
crontab -l > $INUTEMPDIR/root.cron

# Add the /usr/bin/errclear entries to the file
fgrep /usr/bin/errclear /usr/lpp/bos.sysmgt/serv_aid/bos.sysmgt.serv_aid.cron >>$INUTEMPDIR/root.cron

# Submit the new cron file for root
crontab $INUTEMPDIR/root.cron 1>/dev/null 2>/dev/null

# Save the return code
RC=$?

# if RC is not 0, assuming cron daemon has not been started.
# copy the root.cron file to /var/spool/cron/crontabs/root 

if [ $RC -ne 0 ] 
then 
   cp -f $INUTEMPDIR/root.cron  /var/spool/cron/crontabs/root
fi
 
# remove the working file
rm -f $INUTEMPDIR/root.cron

# run logsymptom at the end of inittab
id=logsymp
itab="$id:2:once:/usr/lib/ras/logsymptom # for system dumps"

# Note:  If inittab processing fails we don't want to stop the install.
# See if already there (forced reinstall)
lsitab $id >/dev/null
if [ $? = 0 ]
then	# it is there
	chitab "$itab"
else	# It wasn't there, add at the end
	mkitab "$itab"
fi

if [ -n "$INUCLIENTS" ]
then
	exit 0
fi

#
# Increase the size of /var if the primary dump device is
# paging and if the rootvg has enough space. 
#

# First, find out if the primary dump device type is paging:

PRIMARY=`/usr/bin/dspmsg -s 1 cmddump.cat 3 "primary"`
A=`sysdumpdev -l | grep $PRIMARY`
DMPDEV=`echo $A | cut -f2 -d' '`

# Make sure dump device is a logical volume and not a remote dump device.
#
START_DMPDEV=`echo $DMPDEV | cut -c1-5`
if test $START_DMPDEV != /dev/
then
	exit 0
fi

# Do an lslv on the device name and find out if its type is 'paging'.
# If it isn't, we don't want to try to expand /var, so we will just exit.
#
DMPDEV=`echo $DMPDEV | cut -c6-`
LSLV_TYPE=`lslv $DMPDEV | grep TYPE`
DMPDEV_TYPE=`echo $LSLV_TYPE | cut -f2 -d' '`
if test $DMPDEV_TYPE != paging
then
	exit 0
fi

#Get the size of the /var filesystem
#
LANG=C
A=`df -k | grep " /var$"`
VAR_SIZE=`echo $A | cut -f2 -d' '`

if [ -z "$VAR_SIZE" ]
then
	exit 0
fi

# Divide by 1024 for comparison to rootvg that will follow.
#
VAR_SIZE=`expr $VAR_SIZE / 1024`

# If /var is already 12 meg or greater, don't have to continue.
#
if [ "$VAR_SIZE" -ge 12 ]
then
        exit 0
fi

# Calculate how much more is needed to expand /var to 12 Meg.
#
REQ_EXTRA=`expr 12 - $VAR_SIZE`

# If /var is currently smaller than 12 Meg., and if there is at least
# 600 Meg. and enough space to expand /var to 12 meg in the rootvg,
# we will try to expand /var to 12 Meg.
# (ie., if ( (sizeof_var < 12) && (sizeof_rootvg >= 600)
#          && ( (freespace in rootvg) >= (12 - sizeof_var) ) )
#
# Get the size of the rootvg first.  If it is 600Mb or greater, find
# out if there is enough free space to expand /var to 12Mb.

D=`lsvg rootvg | grep 'TOTAL PPs' | cut -f3 -d:`
TOTALPPS=`echo $D | cut -f1 -d' '`

if [ -z "$TOTALPPS" ]
then
	exit 0
fi

E=`lsvg rootvg | grep 'PP SIZE' | cut -f3 -d:`
VG_PPSIZE=`echo $E | cut -f1 -d' '`

if [ -z "$VG_PPSIZE" ]
then
	exit 0
fi

ROOTVG_SIZE=`expr $TOTALPPS \* $VG_PPSIZE`
 
# Proceed only if rootvg has 600 Meg. or more ...	
#
if [ "$ROOTVG_SIZE" -ge 600 ]
then

	F=`lsvg rootvg | grep 'FREE PPs' | cut -f3 -d:`
	ROOTVG_FREE=`echo $F | cut -f1 -d' '`

	if [ -z "$ROOTVG_FREE" ]
	then
		exit 0
	fi

	ROOTVG_FREE=`expr $VG_PPSIZE \* $ROOTVG_FREE`
  
# Proceed only if rootvg has enough free space to expand /var to 12 Meg.
#
	if [ "$ROOTVG_FREE" -ge "$REQ_EXTRA" ]
	then
		chfs -a size=24576 /var > /dev/null
	fi

fi


