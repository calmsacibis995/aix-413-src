#! /bin/bsh
# @(#)76	1.7.3.2  src/bos/usr/sbin/lvm/highcmd/chvg.sh, cmdlvm, bos411, 9428A410j 3/3/93 13:10:36
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: chvg.sh
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
#                   the characteristics of a volume group.
#
#
# RETURN VALUE DESCRIPTION:
#                             0     Successful
#                             1     Unsuccessful
#                             2     Partially successful
#                                   (at least one VG)
#
# EXTERNAL PROCEDURES CALLED: getlvodm, putlvodm
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
#        $1 : Option flag  (ie. -a for auto_on option)
#        $2 : Option value (ie. y to automatically vary on the volume group)
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

      -o) if [ "$2" != y  -a  "$2" != n ]
	  then            #Invalid input value specified for auto_on
	     dspmsg -s 1 cmdlvm.cat 678 "`lvmmsg 678`\n" chvg >& 2
	     exit
	  fi
	  shift;  shift  ;;    #shift past this option flag/value pair

   esac

done   #end - while there are still arguments to process

}

########################### errhandler #######################################
#
# NAME: errhandler()
#
# DESCRIPTION: Displays fatal error message and sets up exit status for
#              error return from chvg.
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
   dspmsg -s 1 cmdlvm.cat 732 "`lvmmsg 732`\n" chvg $name >& 2
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
	getlvodm -R 
	if [ $? -eq 0 ]
	then
		savebase	#save the database to the ipl_device
	fi

   exit $EXIT_CODE
}

############################## main ############################################
#  Change volume group
#  Input:
#       Command line options and arguments:
#       chvg [-a] VGName...
#  Output:
#       Error Messages (Standard error)
#
hash errhandler getlvodm putlvodm
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
		      #successful exit
#
# Trap on exit/interrupt/break to clean up
#

trap 'cleanup' 0 1 2 15

#
# Parse command line options

set -- `getopt a:Q:u $*`

if [ $? != 0 ]                      # Determine if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 1028 "`lvmmsg 1028`\n" chvg >& 2
   exit
fi

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
	   -a) AFLAG='-o';  AVAL=$2;  shift;  shift;;   #Auto-on state
	   -Q) QFLAG='-Q';  QVAL=$2;  shift;  shift;;   #quorum
	   -u) KFLAG='-K';  shift;;   			#unlock vg
   esac
done   #end - while there is a command line option

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

if [ -n "$1" ]  #if vgname argument on command line
then
   while [ -n "$1" ]
   do               #for each vgname on the command line

      VGNAME="$VGNAME$1 "     #save vgnames in string variable vgname
      shift                   #skip to next command line argument

   done            #end - for each vgname on the command line
else
   dspmsg -s 1 cmdlvm.cat 606 "`lvmmsg 606`\n" chvg >& 2
   dspmsg -s 1 cmdlvm.cat 1028 "`lvmmsg 1028`\n" chvg >& 2
   exit
fi

#
# Check for invalid input
#

check_input $AFLAG $AVAL

#
# Loop through vgnames updating attributes.
# vgname(s) are specified on command line -- Call getlvodm
# to get the vgid for each vgname.
#
for name in $VGNAME
do
    VGID=`getlvodm -v $name`   #get the vgid
    if [ $? != 0 ]             #check for error return from getlvodm
    then
       errhandler
       continue
    fi

#
#   Update ODM with new volume group data.
#
    putlvodm $AFLAG $AVAL $QFLAG $QVAL $KFLAG $VGID
    if [ $? != 0 ]             #check for error return from putlvodm
    then
       errhandler
       continue
    fi
done     #end - for each vgname

if [ "$EXIT_CODE" != 2 ]
then
   EXIT_CODE=0       #Reset exit code to indicate successful completion of
		     #chvg
fi
exit                 #trap will handle cleanup.
