#! /bin/bsh
# @(#)57	1.2.2.3  src/bos/usr/sbin/lvm/highcmd/cfgvg.sh, cmdlvm, bos411, 9435C411a 9/1/94 13:55:07
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: cfgvg
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
## [End of PROLOG]

#
# FILE NAME: cfgvg
#
# FILE DESCRIPTION: Called by /etc/rc to varyon volume groups
#                   that have the auto-varyon flag set
#
# EXTERNAL PROCEDURES CALLED:
#	lsvg,sort,uniq
#	getlvodm
#	varyonvg

# get the list of vg's varied on.
vgvaryedon=`lsvg -o`

# remove the vg's already varied on
vglist=`lsvg | grep -v "$vgvaryedon"`
for name in $vglist
do
    AUTO_ON=`getlvodm -u $name`
    if [ $? = 0 -a "$AUTO_ON" = y ]
    then
        varyonvg  $name 
    fi
done

# removing kmid file from machines migrating from 3.2 to 4.1
rm -f /etc/vg/lvdd_kmid >/dev/null 2>&1
