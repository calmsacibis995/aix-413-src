#!/usr/bin/ksh
#
# @(#)75        1.2  src/rspc/usr/bin/mkpmhlv/mkpmhlv.sh, pwrcmd, rspc41J, 9521A_all 5/22/95 08:50:43
#
# COMPONENT_NAME: (PWRCMD) Power Management Commands
#
# FUNCTIONS: mkpmhlv
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# Make a PM hibernation volume
#  or
# Display PM hibernation volume information
#

PATH=$PATH:/usr/sbin:/usr/bin

#
# find boot physical volume
#
find_boot()
{
	for pv in `/usr/sbin/getlvodm -C`
	do
		/usr/sbin/lspv -M $pv 2> /dev/null | \
				/usr/bin/grep -q hd5 > /dev/null 2>&1
		if [ $? -eq 0 ]
		then
			echo $pv
		fi
	done
}

#
# check flag
#
if [ $# -eq 0 ]
then
	/usr/sbin/lslv pmhibernation
	exit $?
elif [ $# -ne 1 -o "$1" != "-m" ]
then
	echo Usage: mkpmhlv [-m]
	exit 1
fi

#
# calculate the number of physical partitions
#
bootpv=`find_boot`
ppsize=`/usr/sbin/bootinfo -P 0 -s $bootpv`
mem=`/usr/sbin/bootinfo -r`
mem=`/usr/bin/expr $mem / 1024`
ppnum=`/usr/bin/expr $mem / $ppsize`

#
# get current pmhibernation volume size
#
current=`/usr/sbin/lslv -l pmhibernation 2> /dev/null | \
	/usr/bin/grep $bootpv | \
	/usr/bin/awk '{print $2}' | /usr/bin/awk 'BEGIN {FS=":"} {print $1}'`

#
# there is no PM hibernation volume.
#
if [ "$current" = "" ]
then
	echo /usr/sbin/mklv -y pmhibernation -t pmhibernation -b n -r n \
rootvg $ppnum $bootpv
	/usr/sbin/mklv -y pmhibernation -t pmhibernation -b n -r n \
rootvg $ppnum $bootpv
	exit $?
fi

#
# extend the current PM hibernation volume.
#
if [ $current -lt $ppnum ]
then
	num=`/usr/bin/expr $ppnum - $current`
	echo /usr/sbin/extendlv pmhibernation $num
	/usr/sbin/extendlv pmhibernation $num
	exit $?
fi

exit 0
