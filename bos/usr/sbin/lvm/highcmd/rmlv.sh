#! /bin/bsh
# @(#)90	1.15.3.9  src/bos/usr/sbin/lvm/highcmd/rmlv.sh, cmdlvm, bos41J, 9521A_all 5/23/95 15:03:37
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: rmlv.sh
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
# FILE NAME: rmlv
#
# FILE DESCRIPTION: High-level logical volume shell command for removing
#                   a specified logical volume.  See AIX Commands Reference 
#                   manual for additional information.
#
# RETURN VALUE DESCRIPTION: 
#	0	Successful
#	1	Unsuccessful
#	2	Unsuccessful
#
# EXTERNAL PROCEDURES CALLED: getlvodm, ldeletelv, lquerylv, lreducelv
#	putlvodm, lvrelminor
#

########################### nonfatal_errhdlr #################################
#
# NAME: nonfatal_errhdlr()
#
# DESCRIPTION: Displays nonfatal error message, sets up exit status for an
#              unsuccessful return, and unlocks volume group, if locked. 
#
# RETURN VALUE:
#	0	Success
#
nonfatal_errhdlr()
{

	dspmsg -s 1 cmdlvm.cat 912 "`lvmmsg 912`\n" rmlv ${LVNAME:-${LVID:-$LVDESCRIPT}} >& 2

	#Unlock volume group if it was locked in this command
	if test $LOCKVG -eq 1
	then                               
		putlvodm -K $VGID
	fi

	# Set unsuccessful exit status for later use in cleanup()
	EXIT_CODE=2

}

############################ cleanup ###########################################
#
# NAME: cleanup()
#
# DESCRIPTION: Called from trap command to clean up environment and exit.
# 
# INPUT: none
#
# OUTPUT: Error Messages (Standard Error)
#
# NOTE: This function does not return
#
cleanup()
{
	trap '' 0 1 2 15 
	rm -f  $TMPFILE1 # remove temporary files
	test -n "$TMPFILE2" && rm -f  $TMPFILE2   # remove temporary files
	
	if test $LOCKVG -eq 1
	then                               # unlock volume group if it was locked
		putlvodm -K $VGID > /dev/null 2>&1
	fi
	
	getlvodm -R
	if [ $? -eq 0 ]
	then
	   savebase		     #save the database to the ipl_device
    fi
	exit $EXIT_CODE

} 

Usage()
{
	dspmsg -s 1 cmdlvm.cat 910 "`lvmmsg 910`\n" rmlv >& 2
}

######################### check_for_dump_device ###############################
#
# NAME: check_for_dump_device()
#
# DESCRIPTION: function determines if lv is defined as primary or secondary
#              dump device.
#
# INPUT: LVID
#
# OUTPUT: LVNAME
#
# RETURN VALUE DESCRIPTION:     0 if not a dump device
#                               1 if primary dump device
#                               2 if secondary dump device
#
#
check_for_dump_device()
{
	LVNAME=`getlvodm -a $1`

	if [ "`sysdumpdev -l | grep primary | awk '{ print $2 }'`" = "/dev/$LVNAME" ]
	then
		# this is the primary dump device
		return 1
	fi

	if [ "`sysdumpdev -l | grep secondary | awk '{ print $2 }'`" = "/dev/$LVNAME" ]
	then
		# this is the secondary dump device
		return 2
	fi

	# else this is not a dump device
	return 0
}

############################## Main ############################################
#  Remove logical volume
#  Input:
#       Command line options and arguments:  
#       rmlv [-f] [-p] lvdescript...
#
#  Output:
#       Error Messages (Standard Error)
#
#  Note on Error Handling:  If a fatal error occurs while removing any one
#                           lvdescript logical volume, rmlv will not terminate
#                           unless it is the only logical volume being removed
#                           or it is the last in a list of more than one.  
#                           The value returned from rmlv will be 1 if a
#                           fatal error occurs at any point.
#

hash  nonfatal_errhdlr getlvodm rm echo


EXIT_CODE=1         #Set exit status default to unsuccessful.
LOCKVG=0

# Trap on exit/interrupt/break to clean up 
trap 'cleanup' 0 1 2 15

#
# Parse command line options
#

set -- `getopt fp: $*`    #Break cmd line options into separate tokens.

if [ $? != 0 ]           #Determine if there is a syntax error
then
	Usage
	exit
fi

FFLAG=  ; PVNAME=  ; PFLAG=

while  [ "$1"  !=  -- ]               # While there is a command line option
do
   case $1 in
           -f) FFLAG=1;  shift;;
	   -p) PVNAME=$2; PFLAG=1; shift; shift;;
   esac
done
if test -n "$PVNAME" 
then
	PVID=`getlvodm -p $PVNAME`
fi

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

# must have at least one lv to delete
if [ $# -eq 0 ]
then
	dspmsg -s 1 cmdlvm.cat 602 "`lvmmsg 602`\n" rmlv >& 2
	Usage
	exit 
fi

#save lvdescripts in string variable lvdesc
lvdesc="$*"

#
# The following for loop will process one lvdescript at a time.  If a fatal
# error occurs during removal of any one lvdescript, the command will not
# abort.  Instead, it will continue on to the next lvdescript in the list.
# 

for LVDESCRIPT in $lvdesc
do
   # First, check to see if the disk named even holds a copy of
   # the lv that they want to remove.
   if [ -n "$PVNAME" ]
   then
        LV_BY_ID=`getlvodm -l $LVDESCRIPT`
        DISK_CHECK=`lquerylv -L $LV_BY_ID -r | grep "^$PVID "`
        if [ -z "$DISK_CHECK" ]
        then
		dspmsg -s 1 cmdlvm.cat 681 "`lvmmsg 681`\n" rmlv >& 2
                dspmsg -s 1 cmdlvm.cat 912 "`lvmmsg 912`\n" rmlvcopy $LVDESCRIPT >& 2
                exit
        fi
   fi
  
   if [ 0 -eq "$FFLAG" ]     #If a forced removal (-f option) is not specified
   then
      #
      # This is not a forced removal.  Warn the user about the consequences
      # of this command and allow user to terminate removal of this lv
      # at this point.

      LVNM=`getlvodm -a $LVDESCRIPT 2> /dev/null`

      if test -n "$PVNAME"
      then
	dspmsg -s 1 cmdlvm.cat 914 "`lvmmsg 914`\n" rmlv ${LVNM:-$LVDESCRIPT} $PVNAME 
      else
      	dspmsg -s 1 cmdlvm.cat 913 "`lvmmsg 913`\n" rmlv ${LVNM:-$LVDESCRIPT}
      fi

      dspmsg -s 1 cmdlvm.cat 632 "`lvmmsg 632`\n" rmlv
      read RESPONSE
      tstresp $RESPONSE

      if test $? -ne  1
      then             #Negative response - terminate command
	 EXIT_CODE=0   #Set up exit code for a successful exit
	 continue
      fi

   fi                     #end if -f option specified

   LVID=`getlvodm -l $LVDESCRIPT`   #Get the lvid for the logical volume
                                    #manager cmd calls that are to follow
   if [ $? != 0 ]                   #Check for error return from getlvodm
   then                             #error return - nonfatal error
      nonfatal_errhdlr              #Display error msg and set $EXIT_CODE to 2
      continue                      #Continue with next lvdescript in list
   fi   

   check_for_dump_device $LVID
   case $? in
#        1) dspmsg -s 1 cmdlvm.cat 917 "`lvmmsg 917`\n" rmlv $LVNAME "primary"
1) echo "0516-917 rmlv: Warning, cannot remove logical volume $LVNAME."
echo "This logical volume is also used as the primary dump device."
echo "Reset the dump device and retry the command."
		exit
                ;;
#        2) dspmsg -s 1 cmdlvm.cat 917 "`lvmmsg 917`\n" rmlv $LVNAME "secondary"
2) echo "0516-917 rmlv: Warning, cannot remove logical volume $LVNAME."
echo "This logical volume is also used as the secondary dump device."
echo "Reset the dump device and retry the command."
		exit
                ;;
        esac


   LVNAME=`getlvodm -e $LVID`       # Lvname will later be required to delete  
                                    # the special device files.  It will also
                                    # be used in error messages.
   if [ $? != 0 ]
   then 
	LVNAME=$LVID
   fi 

   #
   # Lock the volume group  if  it is not already locked
   #

   if [ "$rmlv_NOLOCK" != 1 ]              #If not locked before  lock the volume group
   then

      VGNAME=`getlvodm -b $LVID`
      if [ $? != 0 ]                #Check for error return from getlvodm
      then                          #Error return - nonfatal error
	 nonfatal_errhdlr           #Display error msg and set $EXIT_CODE to 2
	 continue                   #Continue with next lvdescript in list
      fi

      VGID=`getlvodm -v $VGNAME`
      if [ $? != 0 ]                #Check for error return from getlvodm
      then                          #Error return - nonfatal error
	 nonfatal_errhdlr           #Display error msg and set $EXIT_CODE to 2
	 continue                   #Continue with next lvdescript in list
      fi

	putlvodm -k $VGID               #lock the volume group
      	if [ $? != 0 ]                #Check for error return from putlvodm
      	then                          #Error return - nonfatal error
 		nonfatal_errhdlr           #Display error msg and set $EXIT_CODE to 2
        	continue                   #Continue with next lvdescript in list
     	fi
      	LOCKVG=1

   fi

   # Do a check here to be sure the logical volume is not open.  
   # ldeletelv will not allow the lv to be deleted, but we don't
   # want to remove any partitions if it is (like a paging space)

   LVSTATE=`lquerylv -L $LVID -o`
   if [ $? != 0 ]            #Check for error return from lquerylv
   then                      #error return - nonfatal error
      nonfatal_errhdlr       #Display error msg and set $EXIT_CODE to 2
      continue               #Continue with next lvdescript in list
   fi

   if [ $LVSTATE != 2 ]
   then
      dspmsg -s 1 cmdlvm.cat 1008 "`lvmmsg 1008`\n" rmlv $LVNAME >& 2
      exit
   fi
 
   #
   # create/preallocate temp file.  we do this to ensure that /tmp
   # has enough space to hold our temp file.  we realize that there's
   # a teensy-weensy window where /tmp could fill up between the time
   # the ">" # shell operation creates the file and the time we write to
   # it, but at least it's a small window...
   #

   # first, determine how many lps this lv has
   #
   LPS=`lquerylv -L $LVID -c`
   if [ $? != 0 ]            #Check for error return from lquerylv 
   then                      #error return - nonfatal error
      nonfatal_errhdlr       #Display error msg and set $EXIT_CODE to 2
      continue               #Continue with next lvdescript in list
   fi 

   #
   # then compute how much temp space we'll need (we know
   # that lquerylv -m outputs 28 chars for each lp)
   #
   TMPSIZ=`expr $LPS '*' 28`

   #
   # now, get the tmp file
   #
   TMPFILE1=`lmktemp /tmp/lvmap$$ $TMPSIZ`
   if [ $? != 0 ]            #Check for error return from lmktemp 
   then                      #error return - nonfatal error
      nonfatal_errhdlr       #Display error msg and set $EXIT_CODE to 2
      continue               #Continue with next lvdescript in list
   fi 

   #
   # Call lquerylv using d, and l flags to get the logical partition
   # map - in long format - for the logical volume, placing it in file
   # lvmap$$
   #
 
   lquerylv -L $LVID -r > $TMPFILE1
   if [ $? != 0 ]            #Check for error return from lquerylv 
   then                      #error return - nonfatal error
      nonfatal_errhdlr       #Display error msg and set $EXIT_CODE to 2
      continue               #Continue with next lvdescript in list
   fi 

   # get the map's partition count for lreducelv
   set `wc $TMPFILE1`
   ptncount=$1

   DELETELV=1;
   if test -n "$PVNAME"
   then
	#
	# get the new tmp file
	#
        TMPFILE2=`lmktemp /tmp/tempa$$ $TMPSIZ`
        if [ $? != 0 ]            #Check for error return from lmktemp 
        then                      #error return - nonfatal error
           nonfatal_errhdlr       #Display error msg and set $EXIT_CODE to 2
           continue               #Continue with next lvdescript in list
        fi 

	svcount=$ptncount
   	# get partition map for the portion of the logical volume that
	# resides on the specified phyisical volume
	grep $PVID $TMPFILE1 > $TMPFILE2
	mv $TMPFILE2 $TMPFILE1
   	# get the map's partition count for lreducelv
   	set `wc $TMPFILE1`
        ptncount=$1
	# we will only want to delete the logical volume 
	# if all partitions will be removed
	if test $ptncount -ne $svcount
	then
		DELETELV=0;
	else
		DELETELV=1;
	fi
   fi

   if [ $ptncount != 0 ]       #only call reducelv if the logical
   then                        #partion map is not empty.
 
      #
      # Call lreducelv to reduce the number of allocated logical
      # partitions in the logical volume by $ptncount - given lvmap$$ 
      #  
      lreducelv -l $LVID -s $ptncount $TMPFILE1
      if [ $? != 0 ]          #Check for error return from lreducelv 
      then                    #error return - nonfatal error
	 nonfatal_errhdlr     #Display error msg and set $EXIT_CODE to 2
         continue             #Continue with next lvdescript in list
      fi

   fi

   # we only want to remove the logical volume if we removed all the pp's from
   # that logical volume
   if test $DELETELV -eq 0
   then
		# if we remove exactly one copy of lv, then make sure...
        # update number of copies in database
        if [ $ptncount -eq `lquerylv -L $LVID -c` ]   
        then
            COPY=`getlvodm -c $LVID | awk '{print $6}'`
            if [ $COPY -ge 1 ]
            then
                putlvodm -c `expr $COPY - 1` $LVID
            fi
        fi
     	if test $LOCKVG -eq 1
    	then                               # unlock volume group if it was locked
	       putlvodm -K $VGID
       	      LOCKVG=0
        fi
	    continue
   fi


   #
   # ignore some signals here in an attempt to keep odm in
   # sync with the rest of the lvm world....
   #
   trap '' 0 1 2 15 

   # 
   # Delete logical volume $LVID from the parent volume group
   #

   ldeletelv -l $LVID
   if [ $? != 0 ]           #Check for error return from ldeletelv
   then                     #error return - nonfatal error
      nonfatal_errhdlr      #Display error msg and set $EXIT_CODE to 2
      continue              #Continue with next lvdescript in list
   fi 

   lvrelminor $LVNAME    #release the logical volume minor number

   #
   # Call putlvodm to delete this logical volume data from the
   # system management database.
   #

   putlvodm -L $LVID

   if [ $? != 0 ]     #Check for error return from putlvodm
   then               #error return - warning 
      dspmsg -s 1 cmdlvm.cat 1027 "`lvmmsg 1027`\n" rmlv $LVNAME >& 2
   fi


   if test $LOCKVG -eq 1
   then                               # unlock volume group if it was locked
      putlvodm -K $VGID
      LOCKVG=0
   fi

   #
   # restore signal catching here...
   #
   trap 'cleanup' 0 1 2 15

   dspmsg -s 1 cmdlvm.cat 918 "`lvmmsg 918`\n" rmlv $LVNAME

done        #end for loop (for LVDESCRIPT in $lvdesc)

#
# If $EXIT_CODE is 2, a fatal error occurred at some point.  Leave it set
# to two for an unsuccessful exit.  If it is still set to the default value
# of one, there have been no errors  - change to zero for successful exit. 
#

if [ "$EXIT_CODE" != 2 ]   
then                     
   EXIT_CODE=0
fi

exit
