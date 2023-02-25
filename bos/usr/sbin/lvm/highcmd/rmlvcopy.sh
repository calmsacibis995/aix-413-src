#! /bin/bsh
# @(#)39	1.13.2.7  src/bos/usr/sbin/lvm/highcmd/rmlvcopy.sh, cmdlvm, bos411, 9428A410j 4/13/94 23:54:11
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: rmlvcopy.sh
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
# FILE NAME: rmlvcopy
#
# FILE DESCRIPTION: High-level shell command that deallocates physical
#		    data copies from each logical partition a logical
#                   volume. 
#
#
# RETURN VALUE DESCRIPTION: 
#                             0     Successful
#                             1     Unsuccessful
# 
#
# EXTERNAL PROCEDURES CALLED: getlvodm, putlvodm, putlvcb
#                             lquerylv, allocp, lreducelv
#
#
# GLOBAL VARIABLES: none
#

########################### test_return #######################################
#
# NAME: test_return()
#
# DESCRIPTION: Tests function return code. Will exit and output error message
#              if bad. 
#
# INPUT: 
#        $1 : Function return code
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if input value is invalid. (ie. it will
#       exit with exit status of 1. )
#	
test_return()
{
   if [ "$1" != 0 ]             
   then                # Unsuccessful - give general error message and exit ...
      dspmsg -s 1 cmdlvm.cat 922 "`lvmmsg 922`\n" rmlvcopy $LVDESC >& 2
      exit 
   fi
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

   # unlock the volume group if the VGID was assigned 

   if [ -n "$LOCKED" ]
      then
         putlvodm -K $VGID
   fi

   rm -f /tmp/allocp_out$$ /tmp/allocp_out1$$ /tmp/pvmap$$ /tmp/lvid$$  \
              /tmp/getchar$$ /tmp/temp$$

   exit $EXIT_CODE 
}

############################## main ############################################
#  Deallocate physical data copies from each logical partition in the logical volume.
#  Input:
#       Command line options and arguments:  	
#        rmlvcopy lvdescrip copynum {pvdesc(s)}
#  Output:
#       Error Messages (Standard error)	
#

hash test_return getlvodm putlvodm lquerylv allocp lreducelv cat wc dspmsg getopt

PVDESC=

EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if rmlvcopy completes successfully.
#
# Trap on exit/interrupt/break to clean up 
#

trap 'cleanup' 0 1 2 15

#
# Parse command line arguments
#

if [ -n "$1" ]  	#check for lvdescript on command line
then
   LVDESC=$1      	#save lvdesc in string variable lvdesc
   shift 		#skip to next copynum is there is one
   if [ -n "$1" ]  	#check for copynum
   then
        if [ "$1" -lt 1 -o "$1" -gt 2 ]		#check validity of copynum value
	then
	   dspmsg -s 1 cmdlvm.cat 697 "`lvmmsg 697`\n" rmlvcopy >& 2
	   exit
	fi
	COPYNUM=$1				#set copynum to second argument
   else
	dspmsg -s 1 cmdlvm.cat 697 "`lvmmsg 697`\n" rmlvcopy >& 2
	exit
   fi
   shift
   if [ -n "$1" ]  	#check for optional pvdescript(s) on command line
   then
	PFLAG='-p'
	while [ "$#" -gt 0 ]
	do
        # First, check to see if the disk named even holds a copy of
        # the lv that they want to remove.
                DISK_NAME=$1
		PVID=`getlvodm -p $DISK_NAME`
        	LV_BY_ID=`getlvodm -l $LVDESC`
        	DISK_CHECK=`lquerylv -L $LV_BY_ID -r | grep "^$PVID "`
                if [ -z "$DISK_CHECK" ]
                then
   			dspmsg -s 1 cmdlvm.cat 681 "`lvmmsg 681`\n" rmlvcopy >& 2
                        dspmsg -s 1 cmdlvm.cat 922 "`lvmmsg 922`\n" rmlvcopy $LVDESC >& 2
                        exit
                fi
                PVIDTMP=`getlvodm -p $DISK_NAME`
		test_return $?
		PVDESC="$PVIDTMP,$PVDESC"
		shift 
	done
   fi 

else
   dspmsg -s 1 cmdlvm.cat 602 "`lvmmsg 602`\n" rmlvcopy >& 2
   dspmsg -s 1 cmdlvm.cat 920 "`lvmmsg 920`\n" rmlvcopy >& 2
   exit
fi              #end - if lvdesc on command line

#Pass LVDESC to getlvodm to get the LVID (logical volume identifier)
LVID=`getlvodm -l $LVDESC`   	#get logical volume id for the lvdesc for the lock
test_return $?			#test return error from getlvodm
 
#cut the VGID out of the LVID by using the '.' as a delimiter
echo $LVID > /tmp/lvid$$
VGID=`cut -d"." -f1 /tmp/lvid$$`

#Pass the VGID to putlvodm to lock the volume group 
putlvodm -k $VGID
test_return $?			#test return error from putlvodm
LOCKED=y			#if a successful lock then set variable in case of cleanup

#
# Call lqueryvg to get the list of all PVIDs in the volume group,
# Use the cut command to retrieve only the pvids from lqueryvg's
# output tuples, assigning the output of cut to string variable pvlist.
#

lqueryvg -g $VGID -P > /tmp/temp$$   #Store lqueryvg output in temporary file
test_return $?                  #Check for error return from lqueryvg
    
pvlist=`cat /tmp/temp$$ | cut -d" " -f1`   #extract pvids

#
# For each pvid in $pvlist get the pv map and append to file /tmp/pvmap$$. 
#

> /tmp/pvmap$$
for PVID in $pvlist
   do                         #for each pvid in the list

	#
	# get physical volume map - store in /tmp/pvmap$$
	#

	lquerypv -g $VGID -p $PVID -d -t >> /tmp/pvmap$$
         
	if [ $? != 0 ]       #error return from lquerypv
	then
	  PVNAME=`getlvodm -g $PVID`   #get pvname for error message
	  dspmsg -s 1 cmdlvm.cat 848 "`lvmmsg 848`\n" rmlvcopy ${PVNAME:=$PVID} >& 2
	  exit
	fi
	
done   #end - for each pvid

#call lquerylv to get the map of the logical volume for allocp input
lquerylv -L $LVID -dtl >> /tmp/pvmap$$
test_return $?			#test for error return from lquerylv

#Pass the LVID to getlvodm to get the allocation characteristics for this logical volume
#The characteristics will be used in the call to allocp 
getlvodm -c $LVID > /tmp/getchar$$
test_return $?

#set the strict state and intra-policy characteristics to use for allocp
STRICT=`cut -d" " -f6 /tmp/getchar$$`
INTRA=`cut -d" " -f3 /tmp/getchar$$`

#check the strict state value from getlvodm and if it is 'n' then
#set the '-k' flag for allocp non-strict policy
if [ "$STRICT" = n ]
  then
    SFLAG='-k'
fi

#call allocp with the map as input and characteristics set
#the temporary file data is being used as a dummy replace later with
cat /tmp/pvmap$$ | 	\
	allocp -i $LVID	\
	-t u           	\
	-c $COPYNUM    	\
	-a $INTRA     	\
	$PFLAG $PVDESC 	\
			> /tmp/allocp_out1$$

cat /tmp/allocp_out1$$ | sed '/0000000000000000/D' > /tmp/allocp_out$$

test_return $?			#test for error return from allocp

#set number of partitions to delete to number of lines from allocp output
#the number of lines value is five spaces over in the output of wc
set `wc /tmp/allocp_out$$`
LVNUM=$1

#check for a null file from the output from allocp which means there is no 
#reducing needed for this logical volume
if [ "$LVNUM" != 0 ]
then
	#send deallocation map from allocp to lreducelv  
	lreducelv -l $LVID -s $LVNUM /tmp/allocp_out$$ 
	test_return $?			#test for error return from lreducelv

	#call putlvodm to update copynum for the logical volume
	putlvodm -c $COPYNUM $LVID
	if [ $? != 0 ]
	then 
	  dspmsg -s 1 cmdlvm.cat 1023 "`lvmmsg 1023`\n" rmlvcopy >& 2
	  exit
	fi

	LVNAME=`getlvodm -a $LVDESC`
	test_return $?
   	putlvcb -c $COPYNUM $LVNAME
   	if [ $? != 0 ]   #error return from putlvcb 
   	then
		dspmsg -s 1 cmdlvm.cat 622 "`lvmmsg 622`\n" rmlvcopy >& 2
		exit
   	fi 

else
	dspmsg -s 1 cmdlvm.cat 921 "`lvmmsg 921`\n" rmlvcopy $COPYNUM >& 2
	exit
fi

#cleanup will be called here automatically to remove temporary files 
#and unlock the volume group

#set exit code to 0 for a successful completion of command
EXIT_CODE=0
exit             #trap will handle cleanup

