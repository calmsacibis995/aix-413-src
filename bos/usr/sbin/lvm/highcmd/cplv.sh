#! /bin/bsh
# @(#)04	1.13.1.4  src/bos/usr/sbin/lvm/highcmd/cplv.sh, cmdlvm, bos411, 9432A411a 8/8/94 14:44:33
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: cplv.sh
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

# FILE NAME: cplv
#
# FILE DESCRIPTION: High-level logical volume shell command for copying
#                   the contents of a logical volume to a new logical
#                   volume.
#
# RETURN VALUE DESCRIPTION:
#                             0     Successful
#                             1     Unsuccessful
#
# EXTERNAL PROCEDURES CALLED: copyrawlv, getlvodm, lquerylv, lqueryvg,
#                             mklv, putlvodm, rmlv
#
# GLOBAL VARIABLES: none

######################### check_input #######################################
#
# NAME: check_input()
#
# DESCRIPTION: Checks validity of argument and option combinations.
#        Will output error message exit if input is not valid.
#
# INPUT:
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
	# copying to an existing lv?
	if [ -n "$EFLAG" ]
	then
		# -y -v -Y and -e are mutually exclusive
		if [ -n "$YFLAG"  -o  -n "$yFLAG"  -o  -n "$VFLAG"  ]
		then
                        dspmsg -s 1 cmdlvm.cat 694 "`lvmmsg 694`\n" cplv >& 2
			Usage
			cleanup
		fi
	else
		# -f only valid with -e option.
		if [ -n "$FFLAG" ]
		then            
                        dspmsg -s 1 cmdlvm.cat 692 "`lvmmsg 692`\n" cplv >& 2
			Usage
			cleanup
		fi
		# -y and -Y are mutually exclusive
		if [ -n "$YFLAG"  -a  -n "$yFLAG" ]
		then                             
                        dspmsg -s 1 cmdlvm.cat 695 "`lvmmsg 695`\n" cplv >& 2
			Usage
			cleanup
		fi
		if [ -n "$YFLAG" ]
		then
			VALID_CHECK=`odmget -q "prefix=$YVAL" PdDv`
			if [ -n "$VALID_CHECK" ]
			then
				dspmsg -s 1 cmdlvm.cat 693 "`lvmmsg 693`\n" cplv "Y">& 2
				dspmsg -s 1 cmdlvm.cat 742 "`lvmmsg 742`\n" cplv $SLVDESC >& 2
				cleanup
			fi
		fi
	fi
}

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
	then               
		dspmsg -s 1 cmdlvm.cat 742 "`lvmmsg 742`\n" cplv $SLVDESC >& 2
		cleanup
	fi
}


########################### convert_ppsize #################################
#
# NAME: convert_dppsize()
#
# DESCRIPTION: Converts ppsize values from the low-level query routines
#              from power-of-2 value to the actual number of megabytes that
#              power-of-2 value is supposed to represent.
#
# INPUT:
#        none
#
# OUTPUT:
#        none
#
convert_ppsize()
{
	echo "2 $PPSIZE" | awk '{print $1 ^ $2}'
}


########################### get_dlvsize #####################################
#
# NAME: get_dlvsize()
#
# DESCRIPTION: Calculates the destination logical volume size.
#
# INPUT:
#        none
#
# OUTPUT:
#        none
#
#
#
get_dlvsize()
{
	# get real size of the source lv
	TMPSZ=`echo $SLVSIZE $SPPSIZE | awk '{print $1 * $2}'`

	# is only one destionation pp needed
	if [ $TMPSZ  -le  $DPPSIZE ]           
	then                                
		DLVSIZE=1                      
	else
		# does an even number of destionation pps go into source lv
		if test `echo $TMPSZ $DPPSIZE | awk '{print $1 % $2}'` -eq 0
		then
			DLVSIZE=`echo $TMPSZ $DPPSIZE | awk '{print $1/$2}'`
		else
			DLVSIZE=`echo $TMPSZ $DPPSIZE | awk '{print $1/$2+1}'`
		fi
	fi
}


########################### calc_lvsizes ####################################
#
# NAME: calc_lvsizes()
#
# DESCRIPTION: Determines the source and destination logical volume sizes
#              in number of physical partitions
#
# INPUT:
#        none
#
# OUTPUT:
#        none
#
#
#
calc_lvsizes()
{
	#This function will get source and destination lv sizes in number of
	#physical partitions.  First, physical partition sizes of the source
	#and destination volumes must be determined. If they differ and a new
	#lv is being created, size requirements for the new lv will have to be
	#adjusted.

	# Get the number of physical partitions in the source lv.
	SLVSIZE=`lquerylv -L $SLVID -c`
	test_return $?

	# Get the physical partition size of the source lv.
	PPSIZE=`lquerylv -L $SLVID -s`
	test_return $?

	# convert the partition size to the number of bytes
	SPPSIZE=`convert_ppsize`

	if [ -z "$EFLAG"  -a  -z "$VFLAG" ]
	then
	# new lv to be created in source lv's vg
		DLVSIZE=$SLVSIZE
		DPPSIZE=$SPPSIZE
	else
		if test "$DVGID" != "$SVGID"
		then
			#Get the physical partition size of destination lv.
			PPSIZE=`lqueryvg -g $DVGID -s`
			test_return $?
			# convert the partition size to the number of bytes
			DPPSIZE=`convert_ppsize`
		else
			DPPSIZE=$SPPSIZE
		fi

		# copying to an existing lv?
		if [ -n "$EFLAG" ]
		then
			# get the number of pps in the lv
			DLVSIZE=`lquerylv -L $DLVID -c`
		else
			# Do the ppsizes differ
			if [ "$SPPSIZE" != "$DPPSIZE" ]
			then
				# get the number of pps for the destination lv
				get_dlvsize
			else
				# physical partition sizes are the same
				DLVSIZE=$SLVSIZE
			fi
		fi
	fi  #end if -z $EFLAG and -z $VFLAG (dest lv or vg not specified)

}

getvglocks()
{
	# Lock the source lv's volume group.
	putlvodm -k $SVGID
	test_return $?
	LOCKSVG=1

	# User may have specified the same vgname as the source lv's vg.
	# If the source and dest vg is the same, we don't need to lock
	# the vg twice, which will cause a false putlvodm error.
	# Only lock if vgs are different.
	if [ "$SVGID" != "$DVGID" ]
	then
		putlvodm -k $DVGID
		test_return $?
		LOCKDVG=1
	fi
}

freevglocks()
{
	# is the source logical volume locked?
	if test $LOCKSVG -eq 1
	then
		# Unlock the source lv's volume group
		putlvodm -K $SVGID  > /dev/null 2>&1
		LOCKSVG=0
	fi

	# is the destination logical volume locked?
	if test $LOCKDVG -eq 1
	then
		# Unlock the destination lv's volume group
		putlvodm -K $DVGID > /dev/null 2>&1
		LOCKDVG=0
	fi
}

############################ cleanup ########################################
#
# NAME: cleanup()
#
# DESCRIPTION: Called from trap command to clean up environment and exit.
#	Will remove destination logical volume if it was created.
#
# INPUT: none
#
# OUTPUT:
#        none
#
#
# NOTE: This function will not return.
# rmlv options: 
#               -f tells rmlv not to ask for user confirmation.
#      This will also remove the logical volume special device files.
#
cleanup()
{
	trap '' 0 1 2 15 

	# did we create a new logical volume?
	if test $MADEDLV -ne 0
	then
		# remove the new lv
                rmlv_NOLOCK=1
		export rmlv_NOLOCK
		rmlv -f  $DLVNAME > /dev/null 2>&1
		unset rmlv_NOLOCK
	fi

	freevglocks

	getlvodm -R 
	if [ $? -eq 0 ]
	then
		savebase	#save the database to the ipl_device
	fi

	exit $EXIT_CODE
}

Usage()
{
	dspmsg -s 1 cmdlvm.cat 740 "`lvmmsg 740`\n" cplv >& 2
}

############################## main #########################################
#  Copy logical volume
#  Input:
#       Command line options and arguments:
#        1. cplv [-v vgname] [-Y lvprefix | -y lvname]  lvdescript
#        2. cplv [-e lvname [-f]] lvdescript "
#  Output:
#       Error Messages (Standard error)
#
hash test_return getlvodm
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if cplv completes successfully.
LOCKSVG=0
LOCKDVG=0
MADEDLV=0

#
# Trap on exit/interrupt/break to clean up
#

trap 'cleanup' 0 1 2 15 

#
# Parse command line options
#

set -- `getopt v:y:Y:e:f $*`

if [ $? != 0 ]                      # Determine if there is a syntax error.
then
	Usage
	cleanup
fi

EFLAG= ; DLVNAME= ; VFLAG= ; VVAL= ; yFLAG= ; YVAL= ; FFLAG= ; 

while  [ $1  !=  -- ]               # While there is a command line option
do
	case $1 in
		-e) EFLAG='-e';  DLVNAME=$2;shift;shift;;    #existing lvname
		-v) VFLAG='-v';  VVAL=$2;  shift;  shift;;   #volume group
		-y) yFLAG='-y';  yVAL=$2;  shift;  shift;;   #lvname
		-Y) YFLAG='-Y';  YVAL=$2;  shift;  shift;;   #lvname prefix
		-f) FFLAG='-f';  shift;;		#no user confirmation
	esac
done   #end - while there is a command line option

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

if test $# -ne 0  #if source lvdescript argument on command line
then
   	SLVDESC=$1
else
	dspmsg -s 1 cmdlvm.cat 613 "`lvmmsg 613`\n" cplv >& 2
	Usage
   	cleanup
fi

#
# Check for invalid option combinations
#

check_input

SLVID=`getlvodm -l $SLVDESC`    #Get the source lv's id for low-level query.
test_return $?

SLVNAME=`getlvodm -a $SLVDESC`   #Get the source lv's name for copyrawlv
test_return $?

# test to see if the source logical volume is open
lquerylv -L $SLVID > /dev/null 2 >& 1
if [ $? = 1 ]
then
      dspmsg -s 1 cmdlvm.cat 741 "`lvmmsg 741 `\n" cplv $SLVNAME >& 2
      exit
fi
    
SVGNAME=`getlvodm -b $SLVID`     #Source vgname will be required later for
test_return $?                   #comparison with dest. vgname to make sure
                                 #that a lock on the same volume group is
                                 #not attempted twice.

SVGID=`getlvodm -v $SVGNAME`     #Source volume group id

SLV_TYPE=`getlvodm -y $SLVID`    #Get the type of the source logical volume.

SLV_LABEL=`getlvodm -B $SLVNAME` #Get the label of the source logical volume.

				 #This will be used at the end of cplv to
                                 #determine whether or not to print a warning
                                 #message. If we fail here, it isn't critical;
                                 #therefore, will not treat as a fatal error.


# Depending on the options specified, will need the destination volume group
# name and/or id.

if [ -n "$VFLAG" ]               #If -v destination vgname specified ...
then
   DVGNAME=$VVAL                 #Destination vgname is required for later
                                 #call to mklv and following call to getlvodm.
   DVGID=`getlvodm -v $DVGNAME`  #Will need vgid for later call to lqueryvg.
   test_return $?                #Check return code and treat as a fatal error
                                 #if bad. (Will need to determine the physical
                                 #partition size of the vg from lqueryvg to
                                 #continue.)

elif [ -n "$EFLAG" ]      #If existing dest. lv specified (-e option) ...
then
   DLVID=`getlvodm -l $DLVNAME` #Get id of the lvname specified on cmd line
   test_return $?               #Check return code and treat as a fatal error
                                #if bad.
   DVGNAME=`getlvodm -b $DLVID` #Get the name and id of the lv's volume group
   test_return $?               #for later call to lqueryvg.  Check the return
                                #code and treat as a fatal error if bad.
   DVGID=`getlvodm -v $DVGNAME`
   test_return $?

else                            #Volume group or existing lv not specified ...
   DVGNAME=`getlvodm -b $SLVID` #Just get destination volume group name for
   test_return $?               #input to mklv. (Destination lv will lie
                                #in the same volume group as the source lv.)
   DVGID=$SVGID
fi

# create new lv?
if [ -z "$EFLAG" ]             # If an existing destination lv ( -e
then                           # option ) was not specified, create a new one.

   # lock our volume group
   getvglocks

   # get the source and destination lv sizes (SLVSIZE,DLVSIZE,SPPSIZE,DPPSIZE)
   calc_lvsizes

   # If the lvname has not already been specified
   if [ -z "$yFLAG" ]
   then
      yVAL=$DLVNAME
   else
      DLVNAME=$yVAL
   fi

   rc=`getlvodm -c $SLVID`
   test_return $?

   # Call mklv to create the new logical volume
   mklv_NOLOCK=1
   export mklv_NOLOCK

   set $rc
   UVAL=$8
   SVAL=$9
   ARGS="-r $7 -a $2 -t $1"
   if [ -n "$SVAL" -a \( "$SVAL" != 0 \) ]
   then
	# the source lv is a striped logical volume, create a lv with
	# the same characteric as the source lv
	TMP=`mklv $yFLAG $DLVNAME $YFLAG $YVAL -S $SVAL -u $UVAL $ARGS $DVGNAME $DLVSIZE`
	test_return $?   # Check return from mklv.  Fatal error if bad.

   else
	# regular copy
	TMP=`mklv $yFLAG $DLVNAME $YFLAG $YVAL $ARGS $DVGNAME $DLVSIZE`
	test_return $?   # Check return from mklv.  Fatal error if bad.
   fi
   unset mklv_NOLOCK

   DLVNAME=$TMP
   DLVID=`getlvodm -l $DLVNAME` #Get id of the lvname specified on cmd line
   test_return $?               #Check return code and treat as a fatal error
                                #if bad.

   # remember that we have created a new lv 
   # (mainly so we can delete it if we fail elsewhere)
   MADEDLV=1

else
   # An existing logical volume was specified as the destination

   # User will be prompted for a confirmation unless -f  option was specified.
   if [ -z "$FFLAG" ]
   then
      dspmsg -s 1 cmdlvm.cat 744 "`lvmmsg 744`\n" cplv >& 2

      dspmsg -s 1 cmdlvm.cat 632 "`lvmmsg 632`\n" cplv
      read RESPONSE
      tstresp $RESPONSE

      if test $? -ne 1	# If user does not wish to continue,
      then		# set exit code to 0 for a successful exit.
         EXIT_CODE=0
         cleanup
      fi
   fi


   # lock our volume group(s)
   getvglocks

   # get the source and destination lv sizes (SLVSIZE,DLVSIZE,SPPSIZE,DPPSIZE)
   calc_lvsizes

   # Call getlvodm to get the logical volume type.  (It must be of type copy
   # in order to continue.  If it isn't, issue error message and terminate.)

   DLV_TYPE=`getlvodm -y $DLVID`
   test_return $?

   if [ "$DLV_TYPE" != copy ]
   then
      dspmsg -s 1 cmdlvm.cat 746 "`lvmmsg 746`\n" cplv >& 2
      dspmsg -s 1 cmdlvm.cat 747 "`lvmmsg 747`\n" cplv >& 2
      cleanup
   fi

fi  # end if -z $EFLAG


# calculate the size of the lv to be copied in 131072-byte blocks:
SSIZE=`echo $SLVSIZE $SPPSIZE 131072 | awk '{print $1 * $2 / $3}'`
DSIZE=`echo $DLVSIZE $DPPSIZE 131072 | awk '{print $1 * $2 / $3}'`

# Are we copying to a smaller lv?
if test $SSIZE -gt $DSIZE
then
	SIZE=$DSIZE
	dspmsg -s 1 cmdlvm.cat 748 "`lvmmsg 748`\n" cplv >& 2
else
	SIZE=$SSIZE
fi

# Copy one lv to another.
copyrawlv /dev/r$SLVNAME /dev/r$DLVNAME $SIZE
if [ $? != 0 ]
then
      dspmsg -s 1 cmdlvm.cat 743 "`lvmmsg 743`\n" cplv >& 2
      exit
fi

MADEDLV=0 	# we no longer want to remove the dlv should we fail

# if we copied over an existing lv then we must update the type of the lv
# update the dest lv type to be the same as the source lv type.
if [ "$SLV_TYPE" != "$DLV_TYPE" ]  
then                               
	putlvodm -t $SLV_TYPE $DLVID
	if [ $? != 0 ]
	then
		dspmsg -s 1 cmdlvm.cat 752 "`lvmmsg 752`\n" cplv >& 2
	fi
        putlvcb -t $SLV_TYPE $DLVNAME
        if [ $? != 0 ]      #check for error return from putlvcb
        then
	       dspmsg -s 1 cmdlvm.cat 622 "`lvmmsg 622`\n" cplv >& 2
        fi
fi

#update the destination lv label to be the same as the source lv label

	putlvodm -B $SLV_LABEL $DLVID 
	if [ $? != 0 ]
  	then
		dspmsg -s 1 cmdlvm.cat 753 "`lvmmsg 753`\n" cplv >& 2  
	fi
	putlvcb -L $SLV_LABEL $DLVNAME
	if [ $? != 0 ]	#check for error return from putlvcb
	then
		dspmsg -s 1 cmdlvm.cat 622 "`lvmmsg 622`\n" cplv >& 2
	fi

freevglocks
dspmsg -s 1 cmdlvm.cat 754 "`lvmmsg 754`\n" cplv $SLVNAME $DLVNAME >& 2

EXIT_CODE=0                   #If execution reaches this point, command
                              #was successful. Reset exit status to 0
exit                          #to indicate success -- and exit.
