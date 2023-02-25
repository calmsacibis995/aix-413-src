#! /bin/ksh
# @(#)81	1.25.2.9  src/bos/usr/sbin/lvm/highcmd/extendlv.sh, cmdlvm, bos41J, 9518A_all 4/18/95 14:14:50
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: extendlv.sh and mklvcopy.sh
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
# FILE NAME: extlv_mklvcopy
#
# FILE DESCRIPTION: Shell script to which the high-level logical volume
#                   commands extendlv and mklvcopy are linked.  
#       extendlv:   High-level logical volume shell command for increasing
#                   the number of logical partitions allocated to a 
#                   specified logical volume by a specified amount.
#       mklvcopy:   High-level logical volume shell command for adding a 
#                   specified number of physical data copies to each
#                   logical partition in a specified logical volume.
#
#
# RETURN VALUE DESCRIPTION: 
#                             0     Successful
#                             1     Unsuccessful
# 
#
# EXTERNAL PROCEDURES CALLED: allocp, getlvodm, lextendlv, lquerylv,
#                             lquerypv, lqueryvg, putlvodm, lresynclv
#
#
# EXTERNAL VARIABLES: none
#

######################### check_input #########################################
#
# NAME: check_input()
#
# DESCRIPTION: Checks validity of input values.  Will output error message and 
#              exit if input is not valid.
#
# INPUT:
#        $1 : Option flag  (ie. -a for intra-policy option)
#        $2 : Option value (ie. e for edge intra-policy value)
#        $3 : $4 : ... : Same as $1 $2 pair
#
# RETURN VALUES:
#        0    Successful
#
# OUTPUT:
#        Error Messages (Standard Error)
#
# NOTE: This function will not return if input value is invalid.
#	
#
check_input()
{

while [ "$#" -ne 0 ]    # While there are still arguments to process  
do

   case "$1" in
      
      -a) if [ "$2" != e  -a  "$2" != m  -a  "$2" != c  -a "$2" != "ie" -a "$2" != "im" ]
	  then  #Invalid input value specified for intra-policy option
	     dspmsg -s 1 cmdlvm.cat 652 "`lvmmsg 652`\n" $PROG >& 2
             exit 
          fi
          shift;  shift  ;;    #shift past this option flag/value pair

      -e) if [ "$2" != m  -a  "$2" != x ]
          then             #Invalid input value specfied for inter-policy option
	    dspmsg -s 1 cmdlvm.cat 654 "`lvmmsg 654`\n" $PROG >& 2
            exit 
          fi
          shift;  shift ;;     #shift past this option flag/value pair

      -u) if [ "$2" -lt 1  -o  "$2" -gt 32 ]
          then             #Invalid input value specified for upperbound option
	     dspmsg -s 1 cmdlvm.cat 656 "`lvmmsg 656`\n" $PROG >& 2
             exit 
          fi
          shift;  shift  ;;    #shift past this option flag/value pair

      -s) if [ "$2" != y  -a  "$2" != n ]
          then             #Invalid input value specified for strict option
	     dspmsg -s 1 cmdlvm.cat 658 "`lvmmsg 658`\n" $PROG >& 2
             exit 
          fi
          shift;  shift ;;     #shift past this option flag/value pair

      -m) if [ ! -r "$2" ] 
          then            #-m mapfile doesn't exist or is not readable 
	    dspmsg -s 1 cmdlvm.cat 680 "`lvmmsg 680`\n" $PROG >& 2
            exit 
          fi
          shift;  shift ;;    #shift past this option flag/value pair

   esac

done   #end - while there are still arguments to process

}

Usage()
{
   if [ "$COMMAND" = mklvcopy ]
   then
      dspmsg -s 1 cmdlvm.cat 840 "`lvmmsg 840`\n" $PROG >& 2
   else
      dspmsg -s 1 cmdlvm.cat 786 "`lvmmsg 786`\n" $PROG >& 2
   fi
}

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
# RETURN VALUES:
#         0 : Successful
#
# OUTPUT: 
#         Error Messages (Standard Error)
#
# NOTE: This function will not return if the function return code
#       being tested is not equal to zero.
#
test_return()
{
   if [ "$1" != 0 ]             
   then                # Unsuccessful - give general error message and exit ...
      if [ "$COMMAND" = extendlv ]
      then
	 dspmsg -s 1 cmdlvm.cat 788 "`lvmmsg 788`\n" $PROG $LVDESCRIPT >& 2
      else
	 dspmsg -s 1 cmdlvm.cat 842 "`lvmmsg 842`\n" $PROG $LVDESCRIPT >& 2
      fi
      exit
   fi
}

############################ Cleanup ###########################################
#
# NAME: cleanup()
#
# DESCRIPTION: Called from trap command to clean up environment and exit.
# 
# INPUT: none
#
# OUTPUT: none
#
#
# NOTE: This function does not return
#
cleanup()
{
   trap '' 0 1 2 15  

   rm -f /tmp/aout$$ 2>/dev/null
   rm -f /tmp/allocpin$$ 2>/dev/null
   rm -f /tmp/disks_used$$ 2>/dev/null

   if [ -n "$LOCKED" ]
   then
     putlvodm -K $VGID
   fi

   getlvodm -R 
   if [ $? -eq 0 ]
   then
     savebase > /dev/null #save the database to the ipl_device
   fi

   exit $EXIT_CODE

} 

########################### test_state #######################################
#
# NAME: test_state()
#
# DESCRIPTION: Tests physical volume state bit number. Returns the value
#	       of the bit.
#
# INPUT: 
#        $1 : Physical volume state 
#	 $2 : Bit number to test. (0-4)
#
# RETURN VALUES:
#         0 : Bit is not set.
#	  1 : Bit is set.
#
# OUTPUT: 
#         None.
#
#
test_state()
{
	STATE=$1		#set the state 
	BITNUM=$2		#set the bitnum

	#shift left until the bit is reached
	while [ "$BITNUM" -gt 0 ]
	do
		STATE=`expr $STATE / 2`
		BITNUM=`expr $BITNUM - 1`
	done
	     
	rc=`expr $STATE % 2`	#set the return code based on the bit
	echo $rc  		#return bit value
}

##############################  allocpin #######################################
#  input
#    $1 :  -m          Map option specified
#    $2 :  Map file name if $1 is -m otherwise = None
#    $3 :  Volume group id
#    $4 :  Logical volume id
#    $5 :  List of physical volume names if specified empty otherwise
#  output
#     prints pvmaps followed by lvmap

allocpin()
{
MFLAG=$1
MVAL=$2
VGID=$3
LVID=$4
pvname=$5

if [ "$MFLAG" = -m ]
then                         # pv mapfile is specified on command line
   cat $MVAL                 # cat pvmap pp s  
   lquerylv -L $LVID -ld -t  # print lvmap pp s  
   test_return $?
   return
fi

#
# Get list of PVIDs to be used in allocating logical partitions. 
# If pvnames are specified on the command line, only corresponding 
# physical volumes will be used for allocating.  If no pvnames 
# are specified, all physical volumes within the volume group will 
# be used for allocation.
#

if [ -n "$pvname" ]   
   then            #pvname(s) specified on command line -- call getlvodm
                   #to get the pvid for each pvname and store in $pvlist.
      for name in $pvname
      do  
          temp=`getlvodm -p $name` 
          test_return $?                  #check for error return from getlvodm 
	  vgtemp=`getlvodm -j $name`
	  test_return $?
	  if [ "$vgtemp" != "$VGID" ]
	  then 
	    dspmsg -s 1 cmdlvm.cat 789 "`lvmmsg 789`\n" $PROG $name $VGNAME >& 2
            test_return 1
	  fi
          pvlist="$pvlist$temp "
      done                                #end - for each pvname 
      USERPV=true

else                #pvnames not specified on command line - use all pv's
                    #within volume group. 

      #
      # Call lqueryvg to get the list of all PVIDs in the volume group,
      # Use the cut command to retrieve only the pvids from lqueryvg's
      # output tuples, assigning the output of cut to string variable pvlist.
      #

      for PVID in `lqueryvg -g $VGID -P | cut -d" " -f1`   #extract pvids
	  do
		 PVSTATE=`lquerypv -g $VGID -p $PVID -c`
		 if [ `test_state $PVSTATE 1` -eq 1 -o `test_state $PVSTATE 2` -eq 1 ]
		   then
	         dspmsg -s 1 cmdlvm.cat 834 "`lvmmsg 834`\n" $PROG `getlvodm -g $PVID` >& 2
		   else
	         pvlist="$pvlist $PVID"
		 fi
	  done

fi  #end if pvname not null

#
# For each pvid in $pvlist, determine if its state is set to allocation. 
#

for PVID in $pvlist
   do                                          #for each pvid in the list
      PVSTATE=`lquerypv -g $VGID -p $PVID -c`  #get the state of this pv
      
      if [ $? != 0 -o `test_state $PVSTATE 1` -eq 1 -o `test_state $PVSTATE 2` -eq 1 ] 
      then
         PVNAME=`getlvodm -g $PVID`     #get pvname for error message
	 dspmsg -s 1 cmdlvm.cat 848 "`lvmmsg 848`\n" $PROG `getlvodm -g $PVID` >& 2
         exit
      fi
	
      NOALLOC=`test_state $PVSTATE 3`
    
      if [ "$NOALLOC" = 0 ]     #physical volume is available for allocation,
      then                      #ie. the 3rd bit in PVSTATE is not set to NOALLOC ... 
         GOTAPV=1
	 lquerypv -g $VGID -p $PVID -d -t  # print physical volume map
         
         if [ $? != 0 ]       #error return from lquerypv
         then
            PVNAME=`getlvodm -g $PVID`   #get pvname for error message
	    dspmsg -s 1 cmdlvm.cat 848 "`lvmmsg 848`\n" $PROG ${PVNAME:=$PVID} >& 2
	    exit
         fi
	
      else
         if [ -n "$USERPV" ]
	    then
                PVNAME=`getlvodm -g $PVID`   #get pvname for error message
	        dspmsg -s 1 cmdlvm.cat 850 "`lvmmsg 850`\n" $PROG ${PVNAME:=$PVID} >& 2
	        test_return 1
	    fi
         fi  #end - pv is available for allocation

   done   #end - for each pvid

   if [ -z "$GOTAPV" ] 
      then
	      dspmsg -s 1 cmdlvm.cat 846 "`lvmmsg 846`\n" $PROG >& 2
              exit
   fi

#
# To pass the logical volume's physical partitions to allocp ,
# get and append the logical volume map to /tmp/pvmap$$
# 

   lquerylv -L $LVID -ld -t 
   test_return $?
} 

############################## Main ############################################
#  Extend logical volume / Make logical volume copies
#  Input:
#       Command line options and arguments:  	
#       extendlv [-a] [-e] [-u] [-s] [-m mapfile] lvdescript numlps [pvname...]
#       mklvcopy [-a] [-e] [-u] [-s] [-k] [-m mapfile] lvdescript copynum [pvname...]	
#  Output:
#       Error Messages (Standard Error)	
#

PROG=`basename $0`

#
# Parse command line options
#
hash  test_return getlvodm lquerypv cat

EXIT_CODE=1                        # Initialize exit code.  This will be reset
                                   # to 0 before exiting only if mklvcopy
                                   # completes successfully.
pvidused=
STRIPE_SIZE=
MAXPVS=32
#
# Trap on exit/interrupt/break to clean up 
#

trap 'cleanup' 0 1 2 15


if [ "$PROG" = mklvcopy ]          #Determine if this is the mklvcopy or extendlv
then                               #command
   COMMAND='mklvcopy'
   set -- `getopt a:e:u:s:m:k $*`    # Break cmd line options into separate tokens.
else
   if [ "$PROG" = extendlv ]
   then
      COMMAND='extendlv'
      set -- `getopt a:e:u:s:m: $*`
   fi
fi

if [ $? != 0 ]          #Determine if there is a syntax error
then
   Usage
   exit
fi

AFLAG= ; AVAL= ; EFLAG= ; EVAL= ; UFLAG= ; UVAL= ; SFLAG= ; SVAL= ;
MFLAG= ; MVAL= ; KFLAG= ; CHK_DISK_LIMIT= ; NEW_LIMIT= ;

while  [ "$1"  !=  -- ]               # While there is a command line option
do
   case $1 in
           -a) AFLAG='-a';  AVAL=$2;  shift;  shift;;
           -e) EFLAG='-e';  EVAL=$2;  shift;  shift;;
           -u) UFLAG='-u';  UVAL=$2;  shift;  shift;;
           -s) SFLAG='-s';  SVAL=$2;  shift;  shift;;
           -m) MFLAG='-m';  MVAL=$2;  shift;  shift;;
           -k) KFLAG='-k';  shift;;
  esac
done

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

if [ -n "$1" ]  #if $1 not null
then
   LVDESCRIPT=$1
else                            #give error message if missing argument
   dspmsg -s 1 cmdlvm.cat 602 "`lvmmsg 602`\n" $PROG >& 2
   if [ "$COMMAND" = mklvcopy ]
   then
      dspmsg -s 1 cmdlvm.cat 840 "`lvmmsg 840`\n" $PROG >& 2
   else
      dspmsg -s 1 cmdlvm.cat 786 "`lvmmsg 786`\n" $PROG >& 2
   fi
   exit 
fi

if [ -n "$2" ]  #if $2 not null
then
   if [ "$COMMAND" = extendlv ]
   then                       # 2nd argument is number of logical partitions
      NUMLPS=$2               # if command is extendlv 
      SIZE_FLAG=-s            # set for input to allocp
   else
      COPYNUM=$2              # 2nd argument is number of copies if command 
                              # is mklvcopy

      if [ "$COPYNUM" -lt 1  -o  "$COPYNUM" -gt 3 ]
      then                    # number of copies out of range
	 dspmsg -s 1 cmdlvm.cat 684 "`lvmmsg 684`\n" $PROG >& 2
         exit
      fi
   fi
else
   if [ "$COMMAND" = extendlv ]
   then                       #2nd argument is number of logical partitions
      dspmsg -s 1 cmdlvm.cat 610 "`lvmmsg 610`\n" $PROG >& 2
      dspmsg -s 1 cmdlvm.cat 786 "`lvmmsg 786`\n" $PROG >& 2
      exit
   else
      dspmsg -s 1 cmdlvm.cat 616 "`lvmmsg 616`\n" $PROG >& 2
      dspmsg -s 1 cmdlvm.cat 840 "`lvmmsg 840`\n" $PROG >& 2
      exit
   fi
fi

shift; shift  # skip past lvdescript and numlps/copynum arguments

pvname=$*     #save pvnames in string variable pvname

#
# Check for invalid input
#

check_input $AFLAG $AVAL $EFLAG $EVAL $UFLAG $UVAL $SFLAG $SVAL $MFLAG $MVAL

#
# get the logical volume id, name, and group id
#

LVID=`getlvodm -l $LVDESCRIPT`
test_return $?    #check for getlvodm error

if [ "`lquerylv -L $LVID -P`" -eq 2 ]
then            # lv has read only permission
    test_return 1    
fi

VGNAME=`getlvodm -b $LVID`
test_return $?    #check for getlvodm error

VGID=`getlvodm -v $VGNAME`
test_return $?    #check for getlvodm error

#
# Lock the volume group.
#

putlvodm -k $VGID
test_return $?    #check for putlvodm error
LOCKED=y	  #set locked variable for cleanup

MAXLPS=`lquerylv -L $LVID -n`
test_return $?

CURLPS=`lquerylv -L $LVID -c`
test_return $?

if [ "$COMMAND" = extendlv ]
then
	TESTLPS=`expr $CURLPS + $NUMLPS`
	if [ $TESTLPS -gt $MAXLPS ]
	then                          # no of lps to extend with is greater than maxlps
	dspmsg -s 1 cmdlvm.cat 787 "`lvmmsg 787`\n" $PROG $LVDESCRIPT $MAXLPS >& 2
      	exit
	fi
fi

#
# Call getlvodm to get the logical volume characteristics.  These will
# be used for call to allocp if they were not already specified on
# the command line.
#

character=`getlvodm -c $LVID`
test_return $?
set $character

COPYNUM2=$6
AVAL2=$2
EVAL2=$3
UVAL2=$4
STRIPE_WIDTH=$8

STRIPE_SIZE=`lquerylv -L $LVID -b`
test_return $?    #check for lquerylv error

if [ "$COMMAND" = mklvcopy -a $STRIPE_SIZE != 0 ]
then
	dspmsg -s 1 cmdlvm.cat 1044 "`lvmmsg 1044`\n" mklvcopy >& 2
	exit
fi

if [ -n "$UVAL" -o "$UVAL2" -lt 32 ]
then
	CHK_DISK_LIMIT=1
	lquerylv -L $LVID -d | cut -f1 -d: |sort | uniq > /tmp/disks_used$$
	CURRENT_DISKS=`cat /tmp/disks_used$$ | wc -l`
	if [ -n "$UVAL" -a "$UVAL2" -gt "$UVAL" ]
	then
	# user trying to reduce the -u val for lv during this extension
		if [ "$CURRENT_DISKS" -gt "$UVAL" ]
		then
			echo "Current lv mapping already exceeds -u limit"
			echo "Reorganize current lv or change -u value"
			if [ "$COMMAND" = extendlv ]
			then
	 			dspmsg -s 1 cmdlvm.cat 788 "`lvmmsg 788`\n" $PROG $LVDESCRIPT >& 2
			else
	 			dspmsg -s 1 cmdlvm.cat 842 "`lvmmsg 842`\n" $PROG $LVDESCRIPT >& 2
			fi
			exit
		else
			NEW_LIMIT=$UVAL
			#although not really larger, use it to show a change
		fi
	else
	# enlarging the -u val for the lv during this extension or original
	# -u value is not the 32 default
		if [ -n "$UVAL" ]
		then
			NEW_LIMIT=$UVAL
		else
		# no new -u given, but the original -u still not 32
			NEW_LIMIT=$UVAL2
		fi
	fi
fi

if [ "$COMMAND" = extendlv ]
then
	# Check if it is a striped logical volume, if so, do not accept 
	# -e, -u, -m and pvnames as input parameters.

	if [ $STRIPE_SIZE != 0 ]
	then
		if [ -n "$EFLAG" -o -n "$UFLAG" -o -n "$MFLAG" -o -n "$SFLAG" ]
		then
			dspmsg -s 1 cmdlvm.cat 1035 "`lvmmsg 1035`\n" extendlv >& 2
			Usage
			exit
		fi

		if [ -n "$pvname" ]
		then
			dspmsg -s 1 cmdlvm.cat 1040 "`lvmmsg 1040`\n" extendlv >& 2
			Usage
			exit
		else
			# make sure the number of pvs is a multiple of
			# the stripe width

			LFOVER=`expr $NUMLPS % $STRIPE_WIDTH`
			if [ "$LFOVER" != 0 ]
			then
				dspmsg -s 1 cmdlvm.cat 1036 "`lvmmsg 1036`\n" extendlv >& 2
				EXIT_CODE=3
				test_return 1
			fi

			# find the disks the current lv is striped across
			# and use those
			lquerylv -L $LVID -d | head -$UVAL2 | cut -d":" -f1 \
				> /tmp/pvidlist$$

			pvname=
			for pvid in `cat /tmp/pvidlist$$`
			do
				pvidused="$pvidused $pvid"
				PVNAME=`getlvodm -g $pvid`
				test_return $?
				pvname="$pvname $PVNAME"
			done
			rm -f /tmp/pvidlist$$ 2>/dev/null
		fi

		# round up the requested logical partitions and make sure
		# there is enought free partitions on each physcial volume
		# that will be used for allocation
		AVEPPS=`expr $NUMLPS / $STRIPE_WIDTH`	# logical partition in
							# each disk
   		for PVID in $pvidused
   		do     
			TOTALPPS=`lquerypv -p $PVID -g $VGID -P`
			test_return $?

			USEDPPS=`lquerypv -p $PVID -g $VGID -n`
			test_return $?

			FREEPPS=`expr $TOTALPPS - $USEDPPS`
			if [ $FREEPPS -lt $AVEPPS ]
			then
				PVNAME=`getlvodm -g $PVID 2>/dev/null`
				dspmsg -s 1 cmdlvm.cat 1034 "`lvmmsg 1034`\n" extendlv $PVNAME >& 2
				EXIT_CODE=3
				test_return 1
			fi
		done

		# It is possible the "chlv" command change the interpolicy
		# to minimum when copy a striped logical volume to a non-
		# striped logical volume.  So make sure set the upperbound
		# to maximum before calling allocp.
		EVAL2='x'
	fi

	TESTLPS=`expr $CURLPS + $NUMLPS`
	if [ $TESTLPS -gt $MAXLPS ]
	then                          # no of lps to extend with is greater than maxlps
		dspmsg -s 1 cmdlvm.cat 787 "`lvmmsg 787`\n" $PROG $LVDESCRIPT $MAXLPS >& 2
      		exit
	fi
fi

#
# Set up $SFLAG for call to allocp
#

if [  -n "$SFLAG" ]        #If $SFLAG not null (ie. if already specified
then                       #on extendlv cmd line)  
   if [ "$SVAL" = n ]      #if $SVAL indicates non-strict policy, reset
   then                    #$SFLAG to -k for input to allocp
      SFLAG=-k
   else                    #otherwise, $SVAL indicates strict policy  --  
      SFLAG=               #reset $SFLAG to null for input to allocp
   fi
else                       #$SFLAG null -- not set on extendlv cmd line 
   if [ "$5" = n ]         #Use lv characteristic returned from getlvodm
   then                    #to determine whether or not to set $SFLAG to -k.
      SFLAG=-k
   fi
fi

if [ "$COMMAND" = extendlv ]
then
   TVAL=e              # command is extendlv type is extend  
else
   TVAL=c              # command is mklvcopy type is copy
fi

if [ "$MFLAG" = -m ]
then                      	# pv mapfile is specified on command line
      EVAL='p'
else
      MFLAG='None'
      MVAL='None'
fi

# Call allocp to decide on which pp's to use for the new logical partitions
#  type of allocation is:  copy for mklvcopy,  extend for extendlv
#  copy option - use lv characteristic if $COPYNUM is null
#  use $numlps as specified on extendlv command line for size option
#  use value determined above for strict/nonstrict option
#  upperbound option - use lv characteristic if $UVAL is null
#  interpolicy option - use lv characteristic if $EVAL is null
#  intrapolicy option - use lv characteristic if $AVAL is null
#

ALLOCPOUT=`allocpin $MFLAG $MVAL $VGID $LVID "$pvname"|allocp -i $LVID    \
               -t $TVAL           \
	       -c ${COPYNUM:-$COPYNUM2}  \
       		$SIZE_FLAG $NUMLPS \
       		$SFLAG             \
       		-u ${UVAL:-$UVAL2}     \
       		-e ${EVAL:-$EVAL2}     \
       		-a ${AVAL:-$AVAL2}`        

if [ $? -ne 0 ]		#check for error return from allocp
then
	exit
fi

if [ -n "$CHK_DISK_LIMIT" ]
then
	set $ALLOCPOUT  
	while [ "$1" ] 
     	do 
		echo  $1 >> /tmp/disks_used$$
		shift 3 
	done
	UNIQUE_DISKS=`cat /tmp/disks_used$$ | sort | uniq | wc -l`
	if [ "$UNIQUE_DISKS" -gt "$NEW_LIMIT" ]
	then
	#this implies that the result of the allocp mapping would violate -u
		echo "Current lv mapping already exceeds -u limit"
		echo "Reorganize current lv or change -u value"
		if [ "$COMMAND" = extendlv ]
		then
	 		dspmsg -s 1 cmdlvm.cat 788 "`lvmmsg 788`\n" $PROG $LVDESCRIPT >& 2
		else
	 		dspmsg -s 1 cmdlvm.cat 842 "`lvmmsg 842`\n" $PROG $LVDESCRIPT >& 2
		fi
		exit
	fi
fi

#
# Get the size by which to extend the logical volume (ie. number of
# partitions) by counting the number of partitions returned by
# allocp.
#

set `echo $ALLOCPOUT | wc`
N_OF_WORDS=$2
EXT_SIZE=`expr $N_OF_WORDS \/ 3`

if [ $EXT_SIZE  -eq  0 ] 
then
   dspmsg -s 1 cmdlvm.cat 852 "`lvmmsg 852`\n" $PROG $LVDESCRIPT >& 2
   exit
fi

#
# perform the extension of the logical volume, given the map output from allocp 
#

{
set $ALLOCPOUT  
  while [ "$1" ] 
     do 
         echo  $1 $2 $3 
         shift 3 
     done 
} | lextendlv -l $LVID -s $EXT_SIZE -
test_return $?                             #check for error return from extendlv 

if [ "$COMMAND" = mklvcopy  -a  -n "$KFLAG" ]
then                                            # -k option is specified
   lresynclv -l $LVID                           # syncronize the new copies 
   if [ $? != 0 ]
   then
      dspmsg -s 1 cmdlvm.cat 844 "`lvmmsg 844`\n" $PROG $LVDESCRIPT >& 2
      EXIT_CODE=2
   fi
fi

if [ "$COMMAND" = extendlv ]
then                                           # command is extendlv
   LVSIZE=`lquerylv -L $LVID -c`               # get the lv size from the lvm record
   if [ $? = 0 ]
   then
      putlvodm -z $LVSIZE $LVID                # update lv size in odm
      if [ $? != 0 ]   #error return from putlvodm 
      then
	 dspmsg -s 1 cmdlvm.cat 1026 "`lvmmsg 1026`\n" $PROG $LVDESCRIPT >& 2
      fi
   else                #error return from lquerylv
      dspmsg -s 1 cmdlvm.cat 1026 "`lvmmsg 1026`\n" $PROG $LVDESCRIPT >& 2
   fi
   LVNAME=`getlvodm -a $LVDESCRIPT`
   putlvcb -n $LVSIZE $LVNAME                  # update the lv control block
   if [ $? != 0 ]   #error return from putlvodm 
   then
	dspmsg -s 1 cmdlvm.cat 622 "`lvmmsg 622`\n" $PROG >& 2
   fi 
fi 

if [ "$COMMAND" = mklvcopy ]
then
                                                  # cmd is mklvcopy
   putlvodm -c $COPYNUM $LVID                     # update odm with new number of copies
   if [ $? != 0 ]
   then
      dspmsg -s 1 cmdlvm.cat 1026 "`lvmmsg 1026`\n" $PROG $LVDESCRIPT >& 2
   fi
   LVNAME=`getlvodm -a $LVDESCRIPT`
   putlvcb -c $COPYNUM $LVNAME                    # update the lv control block
   if [ $? != 0 ]   #error return from putlvodm 
   then
	dspmsg -s 1 cmdlvm.cat 622 "`lvmmsg 622`\n" $PROG >& 2
   fi 
fi

if [ "$EXIT_CODE" = 1 ]
then
   EXIT_CODE=0              #reset EXIT_CODE to 0 for use in cleanup()
fi
exit      #trap will handle cleanup.
