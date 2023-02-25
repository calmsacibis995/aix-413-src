#! /bin/bsh
# @(#)77	1.7  src/bos/usr/sbin/lvm/highcmd/varyoffvg.sh, cmdlvm, bos411, 9428A410j 6/15/90 21:56:59
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: varyoffvg.sh
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
# FILE NAME: varyoffvg
#
# FILE DESCRIPTION: High-level logical volume shell command for making
#                   a volume group, and all associated physical and logical
#                   volumes, unavailable for use.
#
#
# RETURN VALUE DESCRIPTION: 
#                             0     Successful
#                             1     Unsuccessful (fatal error)
#                             
# 
#
# EXTERNAL PROCEDURES CALLED: getlvodm, lvaryoffvg, 
#                             putlvodm
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
      dspmsg -s 1 cmdlvm.cat 942 "`lvmmsg 942`\n" varyoffvg $VGNAME >& 2
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
# NOTE: This function will not return. 
#	
#
cleanup()
{
   trap '' 0 1 2 15

   if [ "$LOCKEDVG" -eq 1 ]
   then
      putlvodm -K $VGID >/dev/null 2>&1     # unlock volume group
   fi

   getlvodm -R 
   if [ $? -eq 0 ]
     then
	savebase	#save the database to the ipl_device
   fi

   exit $EXIT_CODE 
}

############################## main ############################################
#  Vary off volume group - Make a volume group unavailable for use
#  Input:
#       Command line arguments:  	
#       varyoffvg [-s] vgname
#  Output:
#       Error Messages (Standard error)	
#
hash test_return getlvodm
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if mklv completes successfully.
LOCKEDVG=0

#
# Trap on exit/interrupt/break to clean up 
#

trap 'cleanup' 0 1 2 15

PROG=`basename $0`      # just get the basename of the command

#
# Parse command line options
#

set -- `getopt s $*`

if [ $? != 0 ]                      # Determine if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 940 "`lvmmsg 940`\n" varyoffvg >& 2
   exit
fi

SFLAG= 

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
	   -s) SFLAG='-f';   shift;;   #vary off in system management mode
   esac                   #convert option for following call to lvaryoffvg
done   #end - while there is a command line option

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

if [ -n "$1" ]  #check for vgname on command line
then
   VGNAME=$1
else
   dspmsg -s 1 cmdlvm.cat 606 "`lvmmsg 606`\n" varyoffvg >& 2
   dspmsg -s 1 cmdlvm.cat 940 "`lvmmsg 940`\n" varyoffvg >& 2
   exit
fi              #end - if vgname on command line

# Call getlvodm, passing vgname to get vgid. (Will be needed in later call
# to lvaryoffvg, and putlvodm)

VGID=`getlvodm -v $VGNAME`      #get logical volume group id from odm
test_return $?                  #check for error return and terminate if error

#
# Lock the volume group
#

putlvodm -k $VGID               # lock the volume group
test_return $?
LOCKEDVG=1

# Call low-level command lvaryoffvg to make volume group unavailable for use

lvaryoffvg -g $VGID $SFLAG
test_return $?           # Check for error return and terminate if error

putlvodm -q 0 $VGID      # VG is varied off, update ODM

EXIT_CODE=0       #Reset exit code to indicate successful completion of varyoffvg

exit              #trap will handle cleanup.

