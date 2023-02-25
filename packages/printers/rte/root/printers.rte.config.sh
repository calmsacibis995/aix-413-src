#!/bin/ksh
# @(#)88        1.4  src/packages/printers/rte/root/printers.rte.config.sh, pkgdevprtrte, pkg411, 9432A411a 8/5/94 11:39:23
#
# COMPONENT_NAME: pkgdevprtrte
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
#
# NAME:  root/printers.rte.config
#
# FUNCTION: script called from instal to do special printer backend
# configuration 
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   

set -o nounset
typeset -i		estatus=0
typeset -i		cstatus=0
typeset -r		titf=/var/tmp/inittab-$$
typeset -r		oldcusdir=/var/spool/lpd/pio/custom
typeset -r		newcusdir=/var/spool/lpd/pio/@local/custom
typeset -r		olddevdir=/var/spool/lpd/pio/dev
typeset -r		newdevdir=/var/spool/lpd/pio/@local/dev
typeset -r		newddidir=/var/spool/lpd/pio/@local/ddi
typeset -r		fontsdir=/usr/lib/lpd/pio/fonts
typeset			file

# Add an entry to /etc/inittab file for printer backend cleanup/initialization.
cp -p /etc/inittab $titf || cstatus=1
if lsitab piobe >/dev/null 2>&1
then
   chitab \
      'piobe:2:wait:/usr/lib/lpd/pio/etc/pioinit >/dev/null 2>&1  # pb cleanup'
else
   mkitab -i cron \
      'piobe:2:wait:/usr/lib/lpd/pio/etc/pioinit >/dev/null 2>&1  # pb cleanup'
fi
estatus=$?
if (( estatus == 0 ))
then
   rm -f $titf
else
   [[ $cstatus = 0 ]] && mv -f $titf /etc/inittab
   exit $estatus
fi

umask 002

# Create a symbolic link (with the host name) to /var/spool/lpd/pio/@local
# directory.  Also save the current host name in the file
# /var/spool/lpd/pio/@local/piocfg.
/usr/lib/lpd/pio/etc/piodmgr -c >/dev/null

# Convert old custom files (if any) and move them to new location.
[[ -d $oldcusdir ]] &&
{
   ls $oldcusdir|xargs -i /usr/lib/lpd/pio/etc/pioupdate -i $oldcusdir/{}
   ls $oldcusdir|xargs -i /usr/lib/lpd/pio/etc/piodigest $oldcusdir/{}
   ls $oldcusdir|xargs -i mv -f $oldcusdir/{} $newcusdir/.
   ls $newddidir|xargs -i chown root.printq $newddidir/{}
   ls $newddidir|xargs -i chmod 664 $newddidir/{}
   rm -rf $oldcusdir
}

# Move old dev files (if any) to new location.
[[ -d $olddevdir ]] &&
{
   ls $olddevdir|xargs -i mv -f $olddevdir/{} $newdevdir/.
   rm -rf $olddevdir
}

########	COMMENTED OUT AS DEEMED NOT TO BE NECESSARY AT THIS TIME
# Rename fonts directories to new printer types.
#[[ -d $fontsdir ]] &&
#(
#   cd $fontsdir 2>/dev/null
#   for file in $(ls 2>/dev/null)
#   do
#      [[ -d $file ]] &&
#      {
#	 [[ $file = 2380 ||
#	    $file = 2380-2 ||
#	    $file = 2381 ||
#	    $file = 2381-2 ||
#	    $file = 2390 ||
#	    $file = 2390-2 ||
#	    $file = 2391 ||
#	    $file = 2391-2 ||
#	    $file = 3812-2 ||
#	    $file = 3816 ||
#	    $file = 4019 ||
#	    $file = 4029 ||
#	    $file = 4037 ||
#	    $file = 4039 ||
#	    $file = 4070 ||
#	    $file = 4072 ||
#	    $file = 4076 ||
#	    $file = 4079 ||
#	    $file = 4201-2 ||
#	    $file = 4201-3 ||
#	    $file = 4202-2 ||
#	    $file = 4202-3 ||
#	    $file = 4207-2 ||
#	    $file = 4208-2 ||
#	    $file = 4212 ||
#	    $file = 4216-31 ||
#	    $file = 4224 ||
#	    $file = 4226 ||
#	    $file = 4234 ||
#	    $file = 5202 ||
#	    $file = 5204 ||
#	    $file = 6180 ||
#	    $file = 6182 ||
#	    $file = 6184 ||
#	    $file = 6185-1 ||
#	    $file = 6185-2 ||
#	    $file = 6186 ||
#	    $file = 6252 ||
#	    $file = 6262 ||
#	    $file = 7372 ]] &&
#	 {
#	    if [[ ! -d ibm$file ]]
#	    then
#	       mv $file ibm$file
#	    else
#	       mv $file/* ibm$file/. && rm -rf $file
#	    fi
#	 }
#      }
#   done
#)


exit 0
