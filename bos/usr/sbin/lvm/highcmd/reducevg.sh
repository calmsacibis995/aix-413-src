#! /bin/bsh
# @(#)79	1.13.1.1  src/bos/usr/sbin/lvm/highcmd/reducevg.sh, cmdlvm, bos411, 9428A410j 5/22/92 15:27:34
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: reducevg
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
# FILE NAME: reducevg
#
# FILE DESCRIPTION: High-level logical volume shell command for reducing
#                   the size of a logical volume group by removing specified
#                   physical volumes. 
#
#
# RETURN VALUE DESCRIPTION: 
#                             0     Successful
#                             1     Unsuccessful
# 
#
# EXTERNAL PROCEDURES CALLED: getlvodm, ldeletepv, lquerylv, lquerypv,
#                             lreducelv, putlvodm, rmlv
#
#
# GLOBAL VARIABLES: none
#

########################## test_return #######################################
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
#       exit with an error status. )
#	
test_return()
{
   if [ "$1" != 0 ]             
   then                # Unsuccessful - give general error message and exit .
      dspmsg -s 1 cmdlvm.cat 882 "`lvmmsg 882`\n"  reducevg >& 2
      exit 
   fi
}

######################## nonfatal_errhdlr ###################################
#
# NAME: nonfatal_errhdlr()
#
# DESCRIPTION: Outputs warning message about a physical volume that could
#               not be deleted and sets exit code to 2. 
#
# INPUT: 
#        $1 : Physical volume name
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
#	
nonfatal_errhdlr()
{
      dspmsg -s 1 cmdlvm.cat 884 "`lvmmsg 884`\n" reducevg $1 >& 2
      EXIT_CODE=2      # Set exit code to 2 to indicate partial error (command
                       # not terminated).              
}

########################### cleanup ########################################
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
#
cleanup()
{
   trap '' 0 1 2 15

   rm -f /tmp/pvmap$$              # remove temporary files

   # unlock the volume group?
   if test $LOCKEDVG -eq 1
   then
	putlvodm -K $VGID > /dev/null 2>&1
   fi

   savebase		     #save the database to the ipl_device

   exit $EXIT_CODE 
}

############################## main ########################################
#  Reduce Volume Group
#  Input:
#       Command line options and arguments:  
#       reducevg [-d] [-f] vgname pvdesc...	
#  Output:
#       Error Messages (Standard error)	
#

hash test_return getlvodm putlvodm

EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if reducevg completes successfully.

VG_DELETED=2          #Initialize constant to value that will be returned 
                      #from ldeletepv if a physical volume was successfully
                      #deleted and the volume group was removed.
LOCKEDVG=0
#
# Trap on exit/interrupt/break to clean up 
#

trap 'cleanup' 0 1 2 15   

#
# Parse command line options
#

set -- `getopt df $*`   
		
if [ $? != 0 ]                      # Determine if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 880 "`lvmmsg 880`\n"  reducevg >& 2
   exit  
fi

DFLAG= ; FFLAG= 

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
           -d) DFLAG='-d';  shift;;
           -f) FFLAG='-f';  shift;;  
  esac
done   #end - while there is a command line option

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

if [ -n "$1" ]  #if vgname argument on command line
then
   VGNAME=$1
else  
   dspmsg -s 1 cmdlvm.cat 606 "`lvmmsg 606`\n"  reducevg >& 2
   dspmsg -s 1 cmdlvm.cat 880 "`lvmmsg 880`\n"  reducevg >& 2
   exit 
fi

if [ -n "$2" ]  #check for 1st pvdesc argument on command line
then
   PVLIST=$2
else
   dspmsg -s 1 cmdlvm.cat 604 "`lvmmsg 604`\n"  reducevg >& 2
   dspmsg -s 1 cmdlvm.cat 880 "`lvmmsg 880`\n"  reducevg >& 2
   exit  
fi

shift; shift  # skip past vgname and and pvdesc arguments

while [ -n "$1" ] 
do               #for each additional pvdesc on the command line 

   PVLIST="$PVLIST $1"     #save pvdescs in string variable PVLIST
   shift                   #skip to next command line argument

done            #end - for each pvdesc on the command line

# Call getlvodm passing vgname to get vgid.  The vgid will be necessary
# for later call to ldeletepv -- so if ODM is down and we can't get
# the vgid, terminate reducevg command now. 

VGID=`getlvodm -v $VGNAME`
test_return $?

lqueryvg -g "$VGID" >/dev/null  # is the vg varied on. If not, exit.
test_return $?

putlvodm -k $VGID    # Lock the volume group
test_return $?       # Test for error return.  If volume
LOCKEDVG=1           # group can't be locked, terminate command.

# PVDESC can be either a pvname or a pvid  
# pvdesc short for pvdescriptor
# either of these values can be used in reducevg

for PVDESC in $PVLIST   #for each pvdesc specified on the command line
do

   # Call getlvodm passing pvdesc to get pvid. The pvid will be necessary
   # for later low-level command calls -- so if ODM is down and we can't
   # get the pvid, treat as a non-fatal error (call nonfatal_errhdlr to
   # output warning message and set exit code to 2) and continue attempt
   # to reduce volume group by the remaining physical volumes specified 
   # on the command line. 

   PVID=`getlvodm -p $PVDESC`   # Call getlvodm, passing pvdesc to get
                                # pvid. This will automatically return if
				# pvid is given.
   if [ $? != 0 ]               # If error return from getlvodm, issue
   then                         # warning message and set error code to
      nonfatal_errhdlr $PVDESC  # indicate a partial error. Then continue
      continue                  # to next pv. (Cannot operate w/o pvid.)
   fi
   NUM=0			# $NUM is # of allocated partions to delete
				# assume 0
   if [ -n "$DFLAG" ]    #if -d option specified, find the # of partions
   then                  #from all logical volumes residing on this pv 
      NUM=`lquerypv -p $PVID -g $VGID -n`
   fi

   if [ "$NUM" != 0 ]		# if any partions to delete, call rmlv
   then
      lquerypv -p $PVID -g $VGID -d > /tmp/pvmap$$   #Get the map for this pv
      if [ $? != 0 ]                            #Test for error return from 
      then                                      #lquerypv 
         nonfatal_errhdlr $PVDESC     
         continue                               #Continue to next pv if error.
      fi

      # Extract list of lvids from map and store in LVLIST string variable

      LVLIST=`cat /tmp/pvmap$$| sed '/ 0 /d' | sed 's/:/ /' | sed 's/  */ /g' \
              | cut -d" " -f5 | sort | uniq`

      # Extract the first lvid from $LVLIST.  Compare the first part of the
      # lvid to the vgid (they should match) to make sure that this physical
      # volume belongs to the specified volume group. If it doesn't, just
      # skip to the next pv, as later call to ldeletepv will require both vgid
      # and pvid - and it won't let you delete the pv if it doesn't belong 
      # to the vg.

      #LVID_VGID=`echo $LVLIST | cut -f1 -d" " | cut -f1 -d"."` 
      #if [ "$LVID_VGID" != "$VGID" ]
      #then
	#dspmsg -s 1 cmdlvm.cat 886
	# "`lvmmsg 886`\n" reducevg $PVDESC $VGNAME >& 2
        # EXIT_CODE=2
        # continue
      #fi


      for LVID in $LVLIST      # Go through each logical volume
      do                       # call rmlv to remove 

	 rmlv_NOLOCK=1
	 export rmlv_NOLOCK
         rmlv $FFLAG -p $PVDESC $LVID

         if [ $? != 0 ]     
         then              	
            DEALLOC_FAIL=TRUE  # Set flag to indicate failure to deallocate
                               # a logicl volume
	     unset rmlv_NOLOCK
            break              # Break out of loop through logical volumes
         fi 
	 unset rmlv_NOLOCK
 
      done

   fi # If -d option specified

   if [ "$DEALLOC_FAIL" = TRUE ]  # If a failure occurred while trying to 
   then                           # deallocate partitions of one of the
      nonfatal_errhdlr $PVDESC    # logical volumes, continue on to next
      continue
   fi

   ldeletepv -g $VGID -p $PVID    # Delete the physical volume
   RC=$?                          # Save return code in $RC as it will
                                  # be tested for a couple of return values.

   # If this was the last pv in the vg, the vg has been removed.
   if [ "$RC" = "$VG_DELETED" ]   # Compare return code against constant value
   then                           # representing return value for physical
                                  # volume successfully deleted and vg removed

      lvrelmajor $VGNAME          # Release the major number for the vg
       if [ $? != 0 ]
       then
	 dspmsg -s 1 cmdlvm.cat 894 "`lvmmsg 894`\n" reducevg $VGNAME >& 2
       fi

      putlvodm -V $VGID           # Update ODM 
      LOCKVG=0			  # When the VGID is bing deleted
				  # don't unlock. The lock will
				  # go away when the process dies.
      if [ $? != 0 ]
      then
	 dspmsg -s 1 cmdlvm.cat 894 "`lvmmsg 894`\n" reducevg $VGNAME >& 2
      fi

      lvrelminor $VGNAME    #release the logical volume minor number
      if [ $? != 0 ]
      then
	   dspmsg -s 1 cmdlvm.cat 894 "`lvmmsg 894`\n" reducevg $VGNAME >& 2
      fi	
 
       putlvodm -P $PVID           # Remove physical volume's data from odm.
       if [ $? != 0 ]              # Inform user if odm couldn't be updated
       then
 	 dspmsg -s 1 cmdlvm.cat 896 "`lvmmsg 896`\n" reducevg $PVDESC >& 2
       fi

   elif [ "$RC" = 0 ]             # If return code=zero, the physical volume
   then                           # was successfully deleted (only pv, not vg
      putlvodm -P $PVID           # Remove physical volume's data from odm.
      if [ $? != 0 ]              # Inform user if odm couldn't be updated
      then
	 dspmsg -s 1 cmdlvm.cat 896 "`lvmmsg 896`\n" reducevg $PVDESC >& 2
      fi

   else                        # If return code was not VG_DELETED or 0, print
                               # warning message that this pv wasn't deleted
      nonfatal_errhdlr $PVDESC # and set error code to partial error
       
   fi

done  # for each PVDESC

if [ "$EXIT_CODE" = 1 ]      # If there was no occurrence of a partial error,
then                         # set exit code to 0 to indicate successful
  EXIT_CODE=0                # completion of reducevg.
fi

exit

