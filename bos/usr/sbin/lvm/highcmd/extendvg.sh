#! /bin/ksh
# @(#)87	1.19.2.7  src/bos/usr/sbin/lvm/highcmd/extendvg.sh, cmdlvm, bos41J, 9520A_all 5/9/95 16:18:12
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: extendvg.sh
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
## [End of PROLOG]


# 
# FILE NAME: extendvg
#
# FILE DESCRIPTION: High-level logical volume shell command for adding
#                   specified physical volumes to a specified volume
#                   group.
#
# RETURN VALUE DESCRIPTION: 
#                             0     Successful
#                             1     Unsuccessful (fatal error)
#                             2     Partially successful (non-fatal error)
#                             
# 
#
# EXTERNAL PROCEDURES CALLED: getlvodm, linstallpv, 
#                             putlvodm
#
#
# GLOBAL VARIABLES: none
#

########################### test_return #####################################
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
   then                # Unsuccessful - give general error message and exit 
      dspmsg -s 1 cmdlvm.cat 792 "`lvmmsg 792`\n" extendvg $>& 2
      exit 1
   fi
}

################################# go_on #####################################
#
# NAME: go_on()
#
# DESCRIPTION: Will output warning message and continue with the next PV
#
# INPUT: 
#       None 
#
# OUTPUT:
#        Error messages (Standard Error)
#
#	
go_on()
{
   dspmsg -s 1 cmdlvm.cat 628 "`lvmmsg 628`\n" extendvg $PVNAME $VGNAME >& 2
   EXIT_CODE=2
}

############################ cleanup ####################################
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

   rm -f /tmp/pvinfo$$
   if [ $LOCKVG = 1 ]        # check for LOCKVG
   then

      putlvodm -K $VGID       # unlock volume group
   fi

   getlvodm -R 
   if [ $? -eq 0 ]
   then
	savebase	#save the database to the ipl_device
   fi

   exit $EXIT_CODE 
}

############################## main ######################################
#  Extend volume group
#  Input:
#       Command line arguments:  	
#       extendvg vgname pvname...
#  Output:
#       Error Messages (Standard error)	
#
hash test_return putlvodm  
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if extendvg completes successfully.

LOCKVG=0
PVOK=0

#
# Trap on exit/interrupt/break to clean up 
#
trap 'cleanup' 0 1 2 15 

# Parse command line options
set -- `getopt f $*`
if [ $? != 0 ]                # if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 790 "`lvmmsg 790`\n" extendvg >& 2
   exit
fi

FFLAG= 

while  [ $1  !=  -- ]         # While there is a command line option
do
   case $1 in
	   -f) FFLAG='-f';  shift;;   #force flag
   esac
done

# Parse command line arguments
shift     # skip past "--" from getopt

if [ -n "$1" ]     #if vgname argument on command line
then
   VGNAME=$1
else  
   dspmsg -s 1 cmdlvm.cat 606 "`lvmmsg 606`\n" extendvg $>& 2
   dspmsg -s 1 cmdlvm.cat 790 "`lvmmsg 790`\n" extendvg $>& 2
   exit 
fi
 
if [ -n "$2" ]     #if pvname  on command line, store in PVLIST variable   
then
   PVLIST=$2
else  
   dspmsg -s 1 cmdlvm.cat 604 "`lvmmsg 604`\n" extendvg $>& 2
   dspmsg -s 1 cmdlvm.cat 790 "`lvmmsg 790`\n" extendvg $>& 2
   exit 
fi

shift; shift           # skip past vgname and 1st pvname arguments 

while [ -n "$1" ] 
do                 #for each additional pvname on the command line 

   PVLIST="$PVLIST $1"     #save pvnames in string variable pvlist
   shift                   #skip to next command line argument

done               #end - for each pvname on the command line



# Call getlvodm, passing vgname to get vgid. (Will be needed in later call
# to linstallpv.)
  
VGID=`getlvodm -v $VGNAME`
test_return $?            # Check for error return and terminate if error

# Is the volume group already varied on? Exit if not.
lqueryvg -g "$VGID" >/dev/null
test_return $?

#
# Lock the volume group unless NOLOCK has been set
#

if [ "$extendvg_NOLOCK" != 1 ]
then
	putlvodm -k $VGID
	test_return $?           # Check for error and terminate if error
	LOCKVG=1
fi

# if input PVLIST has "omd" device, display an error message
if [ -x /usr/bin/odmget ]
then
    for PVNAME in $PVLIST
    do
	if [ ! -z "`odmget -q \"name=${PVNAME} and \
		PdDvLn LIKE rwoptical*\" CuDv 2>/dev/null`" ] # omd device found
	then
		dspmsg -s 1 cmdlvm.cat 1029 "`lvmmsg 1029`\n" $PROG $VGNAME >& 2
		exit
	fi
    done
fi

# if PVNAME in this vg has an "omd" device, display an error message.
if [ -x /usr/bin/odmget ]
then
    getlvodm -w $VGID >/tmp/pvinfo$$ 2>/dev/null
    while read PVID PVNAME
    do
	if [ ! -z "`odmget -q \"name=${PVNAME} and \
		PdDvLn LIKE rwoptical*\" CuDv 2>/dev/null`" ] # omd device found
	then
		exit 99
		break
	fi
    done < /tmp/pvinfo$$
    if [ $? = 99 ]
    then
	dspmsg -s 1 cmdlvm.cat 1029 "`lvmmsg 1029`\n" $PROG $VGNAME >& 2
	exit
    fi
fi

if [ -x /usr/sbin/lqueryvg ]
then
	PP_POWER=`lqueryvg -g $VGID -s`
	case $PP_POWER in
		20) SVAL=1;;
		21) SVAL=2;;
		22) SVAL=4;;
		23) SVAL=8;;
		24) SVAL=16;;
		25) SVAL=32;;
		26) SVAL=64;;
		27) SVAL=128;;
		28) SVAL=256;;
		*) dspmsg -s 1 cmdlvm.cat 682 "`lvmmsg 682`\n" extendvg >& 2; exit;;
	esac
fi

if [ -x /usr/sbin/bootinfo ]
then
	for PVNAME in $PVLIST
	do
        	DISK_SIZE=`/usr/sbin/bootinfo -s $PVNAME`
        	PP_COUNT=`expr $DISK_SIZE / $SVAL`
        	if [ "$PP_COUNT" -gt 1016 ]
        	then
#dspmsg -s 1 cmdlvm.cat 876 "`lvmmsg 876`\n" extendvg "$SVAL" "$PP_COUNT" "$PVNAME"
			echo "5016-876 extendvg: Warning, the Physical Partition Size of $SVAL"
			echo "requires the creation of $PP_COUNT partitions for $PVNAME."
			echo "The system limitation is 1016 physical partitions per disk."
			echo "extendvg will not work unless the original volume group"
			echo "is recreated at a larger Physical Partition size that can also handle the disk to be added."
                	dspmsg -s 1 cmdlvm.cat 862 "`lvmmsg 862`\n" extendvg >& 2 ;
                	exit
        	fi
	done
fi

for PVNAME in $PVLIST
do
   # 
   # Check for disk that is not configured
   #
   CONFIGPVS=`getlvodm -P | cut -d" " -f1` 
   test_return $?
   for TESTPV in $CONFIGPVS
   do
      if [ "$TESTPV" = "$PVNAME" ]
      then
         PVOK=1
      fi
   done
   if [ "$PVOK" = 0 ]
   then 
      dspmsg -s 1 cmdlvm.cat 794 "`lvmmsg 794`\n" extendvg $PVNAME >& 2
      exit 1
   fi
   PVOK=0
      
   #
   # Check for disk without PVID
   #
   PVID=`getlvodm -p $PVNAME`
   test_return $?
   if [ "$PVID" = 0000000000000000 ]
   then
      dspmsg -s 1 cmdlvm.cat 796 "`lvmmsg 796`\n" extendvg $PVNAME >& 2
      chdev -l $PVNAME -a pv=yes >/dev/null 
      test_return $?
   else
      #
      # Check for PV that is already in a VG
      #
      getlvodm -j $PVNAME >/dev/null 2>&1
      if [ $? = 0 ]
      then
	 dspmsg -s 1 cmdlvm.cat 696 "`lvmmsg 696`\n" extendvg $PVNAME >& 2
	 exit 1
      fi
   fi
done

for PVNAME in $PVLIST
do      
   # Call low-level command linstallpv to install the pv in the vg

   linstallpv -N $PVNAME -g $VGID $FFLAG
   rc=$?
   case $rc in
	# everything is okay, but we want to wipe clean any potential
	# old boot image from this disk
	0) if [ -x /usr/sbin/mkboot ]
	   then
		mkboot -c -d /dev/$PVNAME
	   fi;;

	#
	# test for PV already in a volume group
	# If it is in a volume group then retry with the force flag and
	# give the user the chose as to what to do.
	#
        3) dspmsg -s 1 cmdlvm.cat 631 "`lvmmsg 631`\n" extendvg $PVNAME
           dspmsg -s 1 cmdlvm.cat 632 "`lvmmsg 632`\n" extendvg
           read RESPONSE
           tstresp $RESPONSE
           if [ $? = 1 ]
           then                # positive response - retry
               linstallpv -N $PVNAME -g $VGID -f
	       if [ $? != 0 ]
	       then
                  go_on        # print error message and continue with next PV
                  continue
		else
			# everything is okay, but we want to wipe clean
			# any potential old boot image from this disk
			if [ -x /usr/sbin/mkboot ]
			then
				mkboot -c -d /dev/$PVNAME
			fi
	       fi
	   else
               go_on        # print error message and continue with next PV
               continue
	   fi;;

        *) go_on       # print error message and continue with next PV
           continue ;;
   esac
   
   # Call putlvodm to update vg data with pvid

   # get the PVID 
   PVID=`getlvodm -p $PVNAME`
   if [ $? != 0 ]          #if getpvid failed, output warning and continue
   then
     dspmsg -s 1 cmdlvm.cat 1025 "`lvmmsg 1025`\n" extendvg $PVNAME $VGNAME >& 2
   else
     putlvodm -p $VGID $PVID
     if [ $? != 0 ]   #if putlvodm failed, output warning and continue
     then
	dspmsg -s 1 cmdlvm.cat 1025 "`lvmmsg 1025`\n" extendvg $PVNAME $VGNAME >& 2
     fi
   fi
done   #for PVNAME in $PVLIST

if [ "$EXIT_CODE" -ne 2 ]
then
    EXIT_CODE=0       # set exit code to OK.
fi

exit              #trap will handle cleanup.

