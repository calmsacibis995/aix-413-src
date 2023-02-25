#! /bin/bsh
# @(#)41	1.12  src/bos/usr/sbin/lvm/highcmd/syncvg.sh, cmdlvm, bos411, 9428A410j 10/23/90 14:12:23
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: syncvg.sh
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
# FILE NAME: syncvg
#
# FILE DESCRIPTION: High-level shell command to synchronizes all stale 
#                   partitions within either a volume group, logical volume,
#                   or physical volume. 
#
#
# RETURN VALUE DESCRIPTION: 
#                             0     Successful
#                             1     Fatal ERROR - Unsuccessful 
#			      2     Non-Fatal ERROR - Unsuccessful 
# 
#
# EXTERNAL PROCEDURES CALLED: getlvodm, lqueryvg, 
#                             lresyncpv, lresynclv
#
#
# GLOBAL VARIABLES: namelist
#

########################### syncvolgrp #######################################
#
# NAME: syncvolgrp
#
# DESCRIPTION: Syncronizes volume group.  Will output warning messages for any
#	       group that can not be syncronized.
#
# INPUT: none.
#
# OUTPUT: Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:      
#			    0 Successful
# NOTE: This function will not exit, it will only issue a warning message.
#
syncvolgrp()
{
#for each vgname get the vgid and pvids then call lresynpv
#to syncronize the volume group.  if a failure occurs print a
#warning message and continue to next volume group in namelist.
	for VGNAME in $namelist
	do
	   #for each vgname call getlvodm to get the vgid
	   VGID=`getlvodm -v $VGNAME`
	   if [ $? != 0 ]
	   then		
	      #Unsuccessful - give general error message for vg and continue
	      dspmsg -s 1 cmdlvm.cat 932 "`lvmmsg 932`\n" $PROG $VGNAME >& 2
	      EXIT_CODE=2
	    else
	       #then call lqueryvg to get the LVIDS for that volume group
	       lqueryvg -g $VGID -L > /tmp/pvinfo$$
	       if [ $? != 0 ]
	       then		
	         #Unsuccessful - give general error message for vg and continue
		 dspmsg -s 1 cmdlvm.cat 932 "`lvmmsg 932`\n" $PROG $VGNAME >& 2
		 EXIT_CODE=2
	       else
	   	   pvids=`cat /tmp/pvinfo$$ | cut -d" " -f1`
		   #while there is a pvid, call lresyncpv to syncronize the 
		   #stale partitions on that physical volume then get next pvid
		   for PVID in $pvids
		   do
		      lresynclv $FFLAG -l $PVID 	#syncronize LV if fails
		      if [ $? != 0 ]			#send warning msg 
							#and continue to next 
	              then				#and get next LVID
		dspmsg -s 1 cmdlvm.cat 932 "`lvmmsg 932`\n" $PROG $VGNAME >& 2
			EXIT_CODE=2
		      fi
		   done
	        fi
	    fi
	done

}
########################### synclogvol #######################################
#
# NAME: synclogvol
#
# DESCRIPTION: Syncronizes logical volume.  Will output warning messages for 
#	       any group that can not be syncronized.
#
# INPUT: none.
#
# OUTPUT: Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:      
#			    0 Successful
#
# NOTE: This function will not exit, it will only issue a warning message.
#
synclogvol()
{
	#for each lvdesc get the lvid then call lresynclv
	#to syncronize the logical volume.  if a failure occurs print a
	#warning message and continue to next logical volume in namelist
	for LVDESC in $namelist
	do 
	   #for each lvdesc call getlvodm to get the lvid
	   LVID=`getlvodm -l $LVDESC`
	   if [ $? != 0 ]	#if return code from getlvodm is not 0 then
	   then			#Unsuccessful - give error message for LV
	     dspmsg -s 1 cmdlvm.cat 934 "`lvmmsg 934`\n" $PROG $LVDESC >& 2
	     EXIT_CODE=2
	   else
	      lresynclv $FFLAG -l $LVID 	#syncronize LV and if it fails
	      if [ $? != 0 ]
	      then		#Unsuccessful - give error message for LV
		dspmsg -s 1 cmdlvm.cat 934 "`lvmmsg 934`\n" $PROG $LVDESC >& 2
		EXIT_CODE=2
	      fi
 
           fi
	done
}
########################### syncphyvol #######################################
#
# NAME: syncphyvol
#
# DESCRIPTION: Syncronizes physical volume.  Will output warning messages for 
#	       any group that can not be syncronized.
#
# INPUT: none.
#
# OUTPUT: Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:      
#			    0 Successful
#
# NOTE: This function will not exit, it will only issue a warning message.
#
syncphyvol()
{

	#for each pvname get the pvid then call lresyncpv
	#to syncronize the physical volume.  if a failure occurs print a
	#warning message and continue to the next physical volume in pvname
	for PVNAME in $namelist
	do
	    #for each lvdesc call getlvodm to get the lvid
	    PVID=`getlvodm -p $PVNAME`
	    if [ $? != 0 ]
	    then	#Unsuccessful - give error message for PV
	      dspmsg -s 1 cmdlvm.cat 936 "`lvmmsg 936`\n" $PROG $PVNAME >& 2
	      EXIT_CODE=2
	    else
	
		VGID=`getlvodm -j $PVNAME`	#call getlvodm to get the vgid 
		if [ $? != 0 ]			#for pvname
	        then			#Unsuccessful - give error msg for PV
		  dspmsg -s 1 cmdlvm.cat 936 "`lvmmsg 936`\n" $PROG $PVNAME >& 2
		  EXIT_CODE=2
		else
		   lresyncpv $FFLAG -g $VGID -p $PVID	#syncronize PV and if it fails 
       		   if [ $? != 0 ] 
	           then			#Unsuccessful - give error msg for PV
		  dspmsg -s 1 cmdlvm.cat 936 "`lvmmsg 936`\n" $PROG $PVNAME >& 2
		     EXIT_CODE=2
		   fi
		fi
	     fi
	done
}
############################ cleanup ###########################################
#
# NAME: cleanup()
#
# DESCRIPTION: Called from trap command to clean up environment and exit.
# 
# INPUT: none 
#
# OUTPUT:
#        Error messages  (Standard Error) 
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if input value is invalid. (ie. it will
#       exit with exit status of 1. )
#	
#
cleanup()
{
trap '' 0 1 2 15  

   rm -f /tmp/pvids$$ /tmp/pvinfo$$
   exit $EXIT_CODE 
}

############################## main ############################################
#  Synchronizes all stale partitions within either a volume group 'vgname',
#  logical volume 'lvdescript', or physical volume 'pvname'.
#  Input:
#       Command line options and arguments:  	
#        syncvg [-i] [-f] -v | -l | -p Name...  
#  Output:
#       Error Messages (Standard error)	
#
hash getlvodm lqueryvg lresynclv lresyncpv cat
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if syncvg completes successfully.

#
# Trap on exit/interrupt/break to clean up 
#

trap 'cleanup' 0 1 2 15  

#
# Parse command line options
#

PROG=$0		#set up program name for error messages

set -- `getopt fivlp $*`
		
if [ $? != 0 ]                      # Determine if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 1002 "`lvmmsg 1002`\n" $PROG >& 2
   exit  
fi
 
FFLAG= ;IFLAG= ; VFLAG= ; LFLAG= ; PFLAG= ; CASE= 

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
	   #specifies the names are read from standard in
	       -i) IFLAG='-i';  shift;;   		 
	   #synchronizes even the non-stale partitions
	       -f) FFLAG='-f';  shift;;   		 
       #synchronizes the indicated volume group   
           -v) VFLAG='-v';  shift;    CASE=y;;   
	   #synchronizes the indicated logical volume 
           -l) LFLAG='-l';  shift;    CASE=y;;   
	   #synchronizes the indicated physical volume
	       -p) PFLAG='-p';  shift;    CASE=y;;   
   esac
done   #end - while there is a command line option
 
if [ -z "$CASE" ]
then
   dspmsg -s 1 cmdlvm.cat 938 "`lvmmsg 938`\n" $PROG >& 2
   dspmsg -s 1 cmdlvm.cat 1002 "`lvmmsg 1002`\n" $PROG >& 2
   exit  
fi
	
if [ -n "$VFLAG" -a \( -n "$LFLAG" -o -n "$PFLAG" \) ]
then
   dspmsg -s 1 cmdlvm.cat 937 "`lvmmsg 937`\n" $PROG >& 2
   dspmsg -s 1 cmdlvm.cat 1002 "`lvmmsg 1002`\n" $PROG >& 2
   exit  
fi

if [ -n "$LFLAG" -a \( -n "$VFLAG" -o -n "$PFLAG" \) ]
then
   dspmsg -s 1 cmdlvm.cat 937 "`lvmmsg 937`\n" $PROG >& 2
   dspmsg -s 1 cmdlvm.cat 1002 "`lvmmsg 1002`\n" $PROG >& 2
   exit  
fi

if [ -n "$PFLAG" -a \( -n "$LFLAG" -o -n "$VFLAG" \) ]
then
   dspmsg -s 1 cmdlvm.cat 937 "`lvmmsg 937`\n" $PROG >& 2
   dspmsg -s 1 cmdlvm.cat 1002 "`lvmmsg 1002`\n" $PROG >& 2
   exit  
fi

#
# Parse command line arguments
#

#if -i option then read values in from standard in and save in name list
if [ -n "$IFLAG" ]
then
	while [ read LINE ]
	do
		namelist="$namelist $LINE "
	done

#else get arguments from command line and save in namelist
else
	shift	# skip past "--" from getopt

	if [ -n "$1" ]
	then
		namelist=$*
	else
	  dspmsg -s 1 cmdlvm.cat 618 "`lvmmsg 618`\n" $PROG >& 2
	  dspmsg -s 1 cmdlvm.cat 1002 "`lvmmsg 1002`\n" $PROG >& 2
	  exit
	fi
	
fi

if [ -n "$VFLAG" ]		#if sycronizing volume group(s)
then
	syncvolgrp		#syncronize the volume group(s)

elif [ -n "$LFLAG" ]		#if syncronizing logical volume(s)
then
	synclogvol		#syncronize the logical volume(s)

else	
	syncphyvol		#then sycronize the physical volume(s)

fi

#check exit_code to see if any unsuccessful attempts were made and if not
#set exit_code to successful
if [ "$EXIT_CODE" != 2 ]
then 
   EXIT_CODE=0      #Reset exit code to indicate successful completion of syncvg
fi

exit             #trap will handle cleanup.
