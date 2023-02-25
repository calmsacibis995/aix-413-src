#! /bin/bsh
# @(#)75	1.13.1.3  src/bos/usr/sbin/lvm/highcmd/chpv.sh, cmdlvm, bos411, 9428A410j 5/3/94 14:11:47
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: chpv.sh
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

# FILE DESCRIPTION: High-level logical volume shell command to change
#                   the characteristics of a physical volume.
#
#
# RETURN VALUE DESCRIPTION:
#                             0     Successful
#                             1     Unsuccessful
#                             2     Partially successful
#                                   (at least one PV)
#
# EXTERNAL PROCEDURES CALLED: getlvodm, lchangepv
#
#
# GLOBAL VARIABLES: none
#

######################### check_input #########################################
#
# NAME: check_input()
#
# DESCRIPTION: Checks validity of input values.  Will output error message and
#              exit if input is not valid.
#
# INPUT:
#        $1 : Option flag  (ie. -a for allocation option)
#        $2 : Option value (ie. y for allocation allowed)
#        $3 : $4 : ... : Same as $1 $2 pair
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
#
check_input()
{
while [ "$#" -ne 0 ]    # While there are still arguments to process
do

   case "$1" in

      -a) if [ "$2" != y  -a  "$2" != n ]
	  then            #Invalid input value specified for allocation
	     dspmsg -s 1 cmdlvm.cat 674 "`lvmmsg 674`\n" chpv >& 2
	     exit
	  fi
	  shift;  shift  ;;    #shift past this option flag/value pair

      -r) if [ "$2" != r  -a  "$2" != a ]
	  then             #Invalid input value specfied for availability
	    dspmsg -s 1 cmdlvm.cat 670 "`lvmmsg 670`\n" chpv >& 2
	    exit
	  fi
	  shift;  shift ;;     #shift past this option flag/value pair

   esac

done   #end - while there are still arguments to process

}

########################### errhandler #######################################
#
# NAME: errhandler()
#
# DESCRIPTION: Displays fatal error message and sets up exit status for
#              error return from chpv.
#
# INPUT:
#        none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
#
errhandler()
{
   dspmsg -s 1 cmdlvm.cat 722 "`lvmmsg 722`\n" chpv $name >& 2
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
# OUTPUT: Exit with exit code
#
#
cleanup()
{
   rm -f /tmp/pv$$map
   exit $EXIT_CODE
}

######################### any_open_lv #######################################
#
# NAME: any_open_lv()
#
# DESCRIPTION: Checks for any open logical volumes on the given
# physical volume.
#
# INPUT:
#        none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
#
any_open_lv()
{
      OPENED=0
      lquerypv -p $PVID -g $VGID -d > /tmp/pv$$map   #Get the map for this pv
      if [ $? != 0 ]                            #Test for error return from 
      then                                      #lquerypv 
         OPENED=2
	 return
      fi

      # Extract list of lvids from map and store in LVLIST string variable

      LVLIST=`cat /tmp/pv$$map|sed '/ 0 /d' | sed 's/:/ /' | sed 's/  */ /g' \
              | cut -d" " -f5 | sort | uniq 2>/dev/null`

      for LVID in $LVLIST      # Go through each logical volume
      do                       # Check state to see if it's open
	LVSTATE=`lquerylv -L $LVID -o`
	if [ $LVSTATE != 2 ]
	then
	   OPENED=1
	    break
	fi
      done

}

############################## main ############################################
#  Change physical volume
#  Input:
#       Command line options and arguments:
#       chpv { -a | -v }  pvname...
#  Output:
#       Error Messages (Standard error)
#
hash errhandler getlvodm lchangepv
EXIT_CODE=1           #Initialize exit code. This will be reset to null before
		      #exiting only if chpv completes successfully.

#
# Trap on exit/interrupt/break to clean up
#

trap 'cleanup' 0 1 2 15

#
# Parse command line options

set -- `getopt a:v: $*`

if [ $? != 0 ]                      # Determine if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 720 "`lvmmsg 720`\n" chpv >& 2
   exit
fi

AFLAG= ; VFLAG= ; AVAL= ; VVAL= ;

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
	   -a) AFLAG='-a';  AVAL=$2;  shift;  shift;;   #Allocation state
	   -v) VFLAG='-r';  VVAL=$2;  shift;  shift;;   #Availability state
   esac
done   #end - while there is a command line option

if [ -z "$AFLAG"  -a  -z "$VFLAG" ]
then
   dspmsg -s 1 cmdlvm.cat 720 "`lvmmsg 720`\n" chpv >& 2
   exit
fi

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

if [ -n "$1" ]  #if pvname argument on command line
then
   while [ -n "$1" ]
   do               #for each pvname on the command line
      PVNAME="$PVNAME$1 "     #save pvnames in string variable pvname
      shift                   #skip to next command line argument

   done            #end - for each pvname on the command line
else
   dspmsg -s 1 cmdlvm.cat 604 "`lvmmsg 604`\n" chpv >& 2
   dspmsg -s 1 cmdlvm.cat 720 "`lvmmsg 720`\n" chpv >& 2
   exit
fi

#
# Check for invalid input
#

check_input $AFLAG $AVAL $VFLAG $VVAL
#
# Convert option values for following call to lchangelv
#

if [ -n "$AFLAG" ]      #allocation option specified?
then
   if [ "$AVAL" = y ]   #Allocation specified as yes?
   then                 #Set AVAL to 8 to indicate
      AVAL=8            #allocation allowed to lchangepv
   else
      AVAL=4            #no allocation to lchangepv
   fi
fi

if [ -n "$VFLAG" ]      #Availability option specified?
then
   if [ "$VVAL" = r ]   #PV removed?
   then                 #Set VVAL to 1 to indicate removed
      VVAL=1            #to lchangepv.
   else                 #Else available
      VVAL=2            #Set VVAL to 2 for lchangepv.
   fi
fi

#
# Loop through pvnames updating PV attributes.
# pvname(s) specified on command line -- call getlvodm
# to get the vgid and pvid for each pvname.
#

for name in $PVNAME
do
    VGID=`getlvodm -j $name`   #get the vgid
    if [ $? != 0 ]             #check for error return from getlvodm
    then
       errhandler
       continue
    fi
    PVID=`getlvodm -p $name`   #get the pvid
    if [ $? != 0 ]             #check for error return from getlvodm
    then
       errhandler
       continue
    fi

    any_open_lv                 # check for any opened lv's
				# if any open lv's, print warning msg
				# but continue with lchangepv
    if [ $OPENED != 0 ]
    then
	#
	# if we're trying to mark this pv as removed,
	# fail if it has open logical volumes.
	#
	if [ -n "$VFLAG" -a "$VVAL" = "1" ]
	then
		dspmsg -s 1 cmdlvm.cat 724 "`lvmmsg 724`\n" chpv $name >& 2
       		errhandler
		continue
	fi

	dspmsg -s 1 cmdlvm.cat 1010 "`lvmmsg 1010`\n" chpv $name >& 2
    fi

#
#   Call lchangepv to change the physical volume attributes in LVM.
#
    lchangepv -g $VGID -p $PVID $AFLAG $AVAL $VFLAG $VVAL
    if [ $? != 0 ]             #check for error return from lchangepv
    then
       errhandler
       continue
    fi
done

if [ "$EXIT_CODE" != 2 ]
then
   EXIT_CODE=0       #Reset exit code to indicate successful completion of
		     #chpv
fi
exit                 #trap will handle cleanup.
