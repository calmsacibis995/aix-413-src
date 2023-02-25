#! /bin/bsh
# @(#)43	1.10  src/bos/usr/sbin/lvm/intercmd/synclvodm.sh, cmdlvm, bos411, 9428A410j 6/15/90 21:59:22
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: synclvodm.sh
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

#############################################################################
#
# FILE NAME: synclvodm
#
# FILE DESCRIPTION: High-level logical volume shell command for synchronizing
#                   or rebuilding the logical volume control block, the
#                   data base, and the volume group information in LVM.
#
#
# RETURN VALUE DESCRIPTION:
#                             0         Successful
#                             non-zero  Unsuccessful    1 fatal error
#                                                       2 non-fatal error
#
# EXTERNAL PROCEDURES CALLED: getlvodm, lqueryvg, updatevg, updatelv
#
#
# GLOBAL VARIABLES: none
#

# Requirement: Volume group is already varied on.
# Usage: synclvodm [-v] vgname [lvname...]

######################### extract_lvnames #############################
#
# NAME: extract_lvnames()
#
# DESCRIPTION: Extracts the lvnames from LVM data
#
# INPUT:
#        filename
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
#
#
extract_lvnames()
{
      file=$1
      rm -f /tmp/lvnamelist$$ 	# to make sure that the file does not exist.
                  		# Here a temp file is needed because the
                          	# variable lvname used inside the while loop
				# would disappear once the while loop finishes
				# execution. It was executed in a subshell.
				# Redirect standard input (exec < file)
				# then reassign it back to the terminal 
				# (exec < /dev/tty) caused some problems.
      if test -s $file
      then
	  cat $file |
	  while read LVID LVNAME STATE
	  do
              echo $LVNAME >> /tmp/lvnamelist$$   
	  done
          lvname=`cat /tmp/lvnamelist$$`
      else
	  dspmsg -s 1 cmdlvm.cat 552 "`lvmmsg 552`\n" $PROG $VGNAME >& 2
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

   rm -f /tmp/lvlist$$           # clean up
   rm -f /tmp/lvnamelist$$       # clean up
   exit $EXIT_CODE
}


############################## main ############################################
#  Synchronize LVM, control block and data base
#  Input:
#       Command line options and arguments:
#       synclvodm [-v] vgname [lvname...]
#  Output:
#       Error Messages (Standard error)
#
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
		      #exiting only if synclvodm completes successfully.

PROG=`basename $0`      # just get the basename of the command

#
# Trap on exit/interrupt/break to clean up
#

trap 'cleanup' 0 1 2 15


#
# Parse command line options
#

set -- `getopt v $*`

if [ $? != 0 ]                      # Determine if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 540 "`lvmmsg 540`\n" $PROG >& 2 # usage message
   exit
fi

VFLAG= 

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
	   -v) VFLAG='-v';  shift;;      # verbose mode
  esac
done   #end - while there is a command line option

#

#

shift     # skip past "--" from getopt

if test $# -gt 0  #if vgname argument on command line
then
   VGNAME=$1
   shift
else
   dspmsg -s 1 cmdlvm.cat 540 "`lvmmsg 540`\n" $PROG >& 2
   exit
fi

if test $# -ne 0         # check for lvnames on command line
then                     # lvnames are specified on the command line.
   lvname=$*
else                     # get lvnames from LVM
   VGID=`getlvodm -v $VGNAME`
   rc=$?
   if [ $rc -eq 2 -o $rc -eq 3 ]       # odm access error or object not found 
   then
      dspmsg -s 1 cmdlvm.cat 542 "`lvmmsg 542`\n" $PROG >& 2
      exit
   fi
   lqueryvg -g $VGID -L >/tmp/lvlist$$        #extract lvnames
   if [ $? -ne 0 ]
   then
      dspmsg -s 1 cmdlvm.cat 544 "`lvmmsg 544`\n" $PROG $VGNAME >& 2
      exit
   fi
   extract_lvnames /tmp/lvlist$$                # extract lvnames
fi

updatevg $VGNAME
rc=$?
if [ $rc -eq 1 ]
then
   dspmsg -s 1 cmdlvm.cat 546 "`lvmmsg 546`\n" $PROG $VGNAME >& 2
   exit              # fatal error
elif [ $rc -eq 2 ]
then
   dspmsg -s 1 cmdlvm.cat 548 "`lvmmsg 548`\n" $PROG $VGNAME >& 2
   EXIT_CODE=2       # non-fatal error
elif [ -n "$VFLAG" ] #verbose mode then also print the successful update msg
then
   dspmsg -s 1 cmdlvm.cat 550 "`lvmmsg 550`\n" $PROG >& 2
fi


for LVNAME in $lvname
do                              #for each lvid in the list
   updatelv $LVNAME $VGNAME
   if [ $? -ne 0 ]   # should not exit, go on and update as many LVs as we can
   then
      dspmsg -s 1 cmdlvm.cat 556 "`lvmmsg 556`\n" $PROG $LVNAME >& 2
      EXIT_CODE=2
   elif [ -n "$VFLAG" ]         #verbose mode then also print the successful
   then
      dspmsg -s 1 cmdlvm.cat 558 "`lvmmsg 558`\n" $PROG $LVNAME >& 2
   fi
done            # end for each LV

if [ "$EXIT_CODE" -ne 2 ]
then
   EXIT_CODE=0          # reset exit code to indicate successful completion
fi

exit
