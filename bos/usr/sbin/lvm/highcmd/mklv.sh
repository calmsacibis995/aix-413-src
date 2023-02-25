#! /bin/ksh
# @(#)88	1.25.2.21  src/bos/usr/sbin/lvm/highcmd/mklv.sh, cmdlvm, bos41J, 9522A_all 5/29/95 23:42:45
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: mklv.sh
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

# FILE NAME: mklv
#
# FILE DESCRIPTION: High-level logical volume shell command for making
#                   a new logical volume and allocating a specified number
#                   of logical partitions to the new volume.
#
# RETURN VALUE DESCRIPTION:
#	0	Successful
#	1	Unsuccessful
#
#
# EXTERNAL PROCEDURES CALLED: allocp, getlvname, getlvodm, lcreatelv,
#  lextendlv, lquerypv, lqueryvg, putlvcb, putlvodm, rmlv


######################### getstripesize #####################################
#
# NAME: getstripesize
#
# DESCRIPTION: Set SSIZE base on the input -  no.of bytes per stripe
# Stripe size is 2 to the power of SSIZE and the input must be equal 
# to a power of 2.
# Valid value of bytes per stripe are: 4K, 8K, 16K, 32K, 64K, 128K
#
# NOTE: This function will not return if input value is invalid.
#
getstripesize()
{
	case $1 in

		4K | 4k) SSIZE=12;;
		8K | 8k) SSIZE=13;;
		16K | 16k) SSIZE=14;;
		32K | 32k) SSIZE=15;;
		64K | 64k) SSIZE=16;;
		128K | 128k) SSIZE=17;;
		*) dspmsg -s 1 cmdlvm.cat 1039 "`lvmmsg 1039`\n" mklv >& 2; exit;;
	esac

}


######################### check_input #########################################
#
# NAME: check_input()
#
# DESCRIPTION: Checks validity of input aguments.
#
# RETURNS:
#	returns on success
#	exits on error
#
check_input()
{
while [ "$#" -ne 0 ]    # While there are still arguments to process
do

   case "$1" in

      -a) if [ "$2" != e  -a  "$2" != m  -a  "$2" != c  -a "$2" != "ie" -a "$2" != "im" ]
          then            #Invalid input value specified for intra-policy option
	         dspmsg -s 1 cmdlvm.cat 652 "`lvmmsg 652`\n" mklv >& 2
             exit
          fi
          shift;  shift  ;;    #shift past this option flag/value pair

      -e) if [ "$2" != m  -a  "$2" != x ]
          then             #Invalid input value specfied for inter-policy option
	        dspmsg -s 1 cmdlvm.cat 654 "`lvmmsg 654`\n" mklv >& 2
            exit
          fi
          shift;  shift ;;     #shift past this option flag/value pair

      -u) if [ "$2" -lt 1  -o  "$2" -gt 32 ]
          then             #Invalid input value specified for upperbound option
	         dspmsg -s 1 cmdlvm.cat 656 "`lvmmsg 656`\n" mklv >& 2
             exit
          fi
          shift;  shift  ;;    #shift past this option flag/value pair

      -s) if [ "$2" != y  -a  "$2" != n ]
          then             #Invalid input value specified for strict option
	         dspmsg -s 1 cmdlvm.cat 658 "`lvmmsg 658`\n" mklv >& 2
             exit
          fi
          shift;  shift ;;     #shift past this option flag/value pair

      -m) if [ ! -r "$2" ]
          then            #-m mapfile doesn't exist or is not readable
	        dspmsg -s 1 cmdlvm.cat 680 "`lvmmsg 680`\n" mklv >& 2
            exit
          fi
          shift;  shift ;;    #shift past this option flag/value pair

      -b) if [ "$2" != y  -a  "$2" != n ]
          then                #bad block relocation not specified as y or n
	         dspmsg -s 1 cmdlvm.cat 660 "`lvmmsg 660`\n" mklv >& 2
             exit
          fi
          shift;  shift ;;    #shift past this option flag/value pair

      -c) 
          if [ "$2" -lt 1  -o  "$2" -gt 3 ]
          then              #Invalid input value specified for copy option
	         dspmsg -s 1 cmdlvm.cat 662 "`lvmmsg 662`\n" mklv >& 2
	         exit
          fi
          shift;  shift ;;    #shift past this option flag/value pair

      -M) if [ "$2" != s  -a  "$2"  != p ]
          then              #Invalid input specified for scheduling policy
	         dspmsg -s 1 cmdlvm.cat 664 "`lvmmsg 664`\n" mklv >& 2
             exit
          fi
          shift;  shift ;;   #shift past this option flag/value pair

      -x) if [ "$2" -lt 1  -o  "$2" -gt 65535 ]
          then             #Invalid input specified for maxlps
	         dspmsg -s 1 cmdlvm.cat 668 "`lvmmsg 668`\n" mklv >& 2
             exit
          fi

	  if [ "$2" -lt "$NUMLPS" ]
          then             #Invalid input specified for maxlps
	         dspmsg -s 1 cmdlvm.cat 693 "`lvmmsg 693`\n" mklv "x" >& 2
	 	dspmsg -s 1 cmdlvm.cat 822 "`lvmmsg 822`\n" mklv >& 2
             exit
          fi
          shift;  shift ;;   #shift past this option flag/value pair

      -v) if [ "$2" != y  -a  "$2" != n ]
          then            #Invalid input specified for write-verify value
	         dspmsg -s 1 cmdlvm.cat 676 "`lvmmsg 676`\n" mklv >& 2
             exit
          fi
          shift;  shift ;;   #shift past this option flag/value pair

      -r) if [ "$2" != y  -a  "$2" != n ]
          then            #Invalid input specified for relocatable value
	         dspmsg -s 1 cmdlvm.cat 672 "`lvmmsg 672`\n" mklv >& 2
             exit
          fi
          shift;  shift ;;   #shift past this option flag/value pair

      -w) if [ "$2" != y  -a  "$2" != n ]
          then    #Invalid input specified for mirror-write consistency value
	         dspmsg -s 1 cmdlvm.cat 1004 "`lvmmsg 1004`\n" mklv >& 2
             exit
          fi
          shift;  shift ;;   #shift past this option flag/value pair

      -t) if [ "$2" = 'paging' ]
          then
             if [ "$WVAL" = 'y' ] 
             then   #Invalid input specified for mirror-write consistency value
	         dspmsg -s 1 cmdlvm.cat 1005 "`lvmmsg 1005`\n" mklv >& 2
                 exit
             fi
          fi
	  # check to see that the type string is not longer than
	  # 31 characters.  This test is really just for user generated
	  # lv types. Search for 32 characters, since an embeded eoln
	  # gets counted by wc.
	  
	 CHAR_COUNT=`echo $TVAL | wc -m`
	 if [ "$CHAR_COUNT" -gt 32 ]
	 then
                dspmsg -s 1 cmdlvm.cat 693 "`lvmmsg 693`\n" mklv "t" >& 2
	 	dspmsg -s 1 cmdlvm.cat 822 "`lvmmsg 822`\n" mklv >& 2
		exit
	 fi
          shift;  shift ;;   

      -S) getstripesize $SSVAL 

          shift;  shift ;;   #shift past this option flag/value pair

      -L) # We don't allow labels 128 characters or greater.
	  # wc -m also counts the null character in the string, so we are
	  # looking for a return count of 129 characters or larger
          CHAR_COUNT=`echo $LVAL | wc -m`
          if [ "$CHAR_COUNT" -gt 128 ]
          then
                dspmsg -s 1 cmdlvm.cat 693 "`lvmmsg 693`\n" mklv "L" >& 2
                dspmsg -s 1 cmdlvm.cat 822 "`lvmmsg 822`\n" mklv >& 2
                exit
          fi

          shift;  shift ;;   #shift past this option flag/value pair

   esac

done   #end - while there are still arguments to process

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
		# unlock volume group
		if test $VGLOCKED -eq 1
		then
			putlvodm -K $VGID
		fi
		dspmsg -s 1 cmdlvm.cat 822 "`lvmmsg 822`\n" mklv >& 2
		exit $1
	fi
}

########################### fatal_errhdlr #################################
#
# NAME: fatal_errhdlr()
#
# DESCRIPTION: Displays fatal error message, sets up exit status for an
#              unsuccessful return, and unlocks volume group, if locked. 
#
# RETURN VALUE:
#	0	Success
#
fatal_errhdlr()
{

	dspmsg -s 1 cmdlvm.cat 634 "`lvmmsg 634`\n" mklv >& 2

	# Set unsuccessful exit status for later use in cleanup()
	EXIT_CODE=2

	exit	# if there is no space in /tmp we have to abort...

}

############################ cleanup ##########################################
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
	if [ $EXIT_CODE -eq 1 -o $EXIT_CODE -eq 2 ]
	then
		# release the minor number & del all /dev entries associated with minor no.
		if test $GOTMINOR -eq 1
		then
			lvrelminor $LVNAME
		fi

		# remove the lv that we made
		if test $MADELV -eq 1
		then
			rmlv_NOLOCK=1
			export rmlv_NOLOCK
			rmlv -f $LVID 2>&1 > /dev/null
			unset rmlv_NOLOCK
		fi

		# clean up special file for logical volume
		if test $MADENODE -eq 1
		then
			rm -f /dev/r$LVNAME /dev/$LVNAME
		fi

	fi

	rm -f  /tmp/pvmap$$ /tmp/aout$$ /tmp/temp$$ /tmp/pvlist$$

	# unlock volume group
	if test $VGLOCKED -eq 1
	then
		putlvodm -K $VGID
	fi

	getlvodm -R 
	if [ $? -eq 0 ]
	then
		savebase > /dev/null #save the database to the ipl_device
	fi

        umask $OLD_UMASK	# restore umask 

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
Usage()
{
	dspmsg -s 1 cmdlvm.cat 1006 "`lvmmsg 1006`\n" mklv >& 2
}
Pvfatal()
{
	PVNAME=`getlvodm -g $PVID 2>/dev/null`
	if test $? -ne 0
	then
		PVNAME=$PVID
	fi
	dspmsg -s 1 cmdlvm.cat 848 "`lvmmsg 848`\n" mklv $PVNAME >& 2
	exit
}
############################## main ############################################
#  Make logical volume
#  Input:
#       Command line options and arguments:
#       mklv [-a] [-e] [-u] [-s] [-m mapfile] [-b] [-c] [-d] [-i] [-w]
#            [-L] [-r] [-t] [-v] [-x] [-y | -Y] vgname numlps [pvname...]
#  Output:
#       Error Messages (Standard error)
#
hash test_return getlvodm mknod lquerypv cat
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if mklv completes successfully.

VGLOCKED=0
GOTMINOR=0
MADELV=0
MADENODE=0
PVNUM=0

OLD_UMASK=`umask`	# save umask value
umask 117		# set umask for rw-rw----

# Trap on exit/interrupt/break to clean up
trap 'cleanup' 0 1 2 15 

# Parse command line options
set -- `getopt a:e:u:s:S:m:b:c:d:r:t:v:w:x:y:L:Y:i "$@"`

if [ $? != 0 ]                      # Determine if there is a syntax error.
then
	Usage
	exit
fi

AFLAG= ; AVAL= ; EFLAG= ; EVAL= ; UFLAG= ; UVAL= ; SFLAG= ; SVAL=
MFLAG= ; MVAL= ; BFLAG= ; BVAL= ; CFLAG= ; CVAL= ; DFLAG= ; DVAL= 
RFLAG= ; RVAL= ; TFLAG= ; TVAL= ; LFLAG= ; LFLAG2= ; LVAL= 
VFLAG= ; VVAL= ; XFLAG= ; XVAL= ; yFLAG= ; yVAL= ; YFLAG= ; YVAL= 
WFLAG= ; WVAL= ; IFLAG= ; SSFLAG= ; SSVAL= ; SSIZE= ; CHK_DISK_LIMIT=

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
           -a) AFLAG='-a';  AVAL=$2;  shift;  shift;;   #intrapolicy
           -e) EFLAG='-e';  EVAL=$2;  shift;  shift;;   #interpolicy
           -u) UFLAG='-u';  UVAL=$2;  shift;  shift;;   #upperbound
           -s) SFLAG='-s';  SVAL=$2;  shift;  shift;;   #strict policy
           -S) SSFLAG='-S';  SSVAL=$2;  shift;  shift;; #strip lv
	   -m) MFLAG='-m';  MVAL=$2;  shift;  shift;;   #pv mapfile
           -b) BFLAG='-b';  BVAL=$2;  shift;  shift;;   #bad block relocation
           -c) CFLAG='-c';  CVAL=$2;  shift;  shift;;   #copies
           -d) DFLAG='-M';  DVAL=$2;  shift;  shift;;   #scheduling policy
           -r) RFLAG='-r';  RVAL=$2;  shift;  shift;;   #relocatable
           -t) TFLAG='-t';  TVAL=$2;  shift;  shift;;   #lvtype
           -L) LFLAG='-L';  LFLAG2='-B';		#lvlabel
		LVAL=$2;  shift
	#
	# soak up all words until we hit the next flag
	#
		while [ `expr "X$2" : "X-*"` = 1 ]
		do
			LVAL="$LVAL $2"; shift
		done
		shift;;
           -v) VFLAG='-v';  VVAL=$2;  shift;  shift;;   #write/verify
           -w) WFLAG='-w';  WVAL=$2;  shift;  shift;;   #mirrorwrite consistency
           -x) XFLAG='-x';  XVAL=$2;  shift;  shift;;   #max numlps
           -y) yFLAG='-y';  yVAL=$2;  shift;  shift;;   #lvname
           -Y) YFLAG='-Y';  YVAL=$2;  shift;  shift;;   #lvname prefix
           -i) IFLAG='-i';  shift;;                     #pvnames from standard in

  esac
done   #end - while there is a command line option

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

if test $# -gt 0  #if vgname argument on command line
then
	VGNAME=$1
	shift
else
	dspmsg -s 1 cmdlvm.cat 606 "`lvmmsg 606`\n" mklv >& 2
	Usage
	exit
fi


if test $# -gt 0  #if numlps argument on command line
then
	NUMLPS=$1
	shift
else
	dspmsg -s 1 cmdlvm.cat 830 "`lvmmsg 830`\n" mklv >& 2
	Usage
	exit
fi

# are we to read pvnames from stdin?
if [ -n "$IFLAG" ]
then
	while read NAME
	do
		pvname="$pvname$NAME "
	done
        if [ -z "$pvname" ]
	then
		dspmsg -s 1 cmdlvm.cat 825 "`lvmmsg 825`\n" mklv >& 2
		Usage
		exit
        fi

# pvnames entered on the command line?
elif test $# -ne 0
then
	#pvnames are specified on the command line.
	pvname=$*
fi

#
# Check for invalid input
#

check_input $AFLAG $AVAL $EFLAG $EVAL $UFLAG $UVAL $SFLAG $SVAL \
	$MFLAG $MVAL $BFLAG $BVAL $CFLAG $CVAL $DFLAG $DVAL \
	$XFLAG $XVAL $VFLAG $VVAL $RFLAG $RVAL $WFLAG $WVAL $TFLAG $TVAL \
	$SSFLAG $SSVAL $LFLAG $LVAL

# check for illegal comination of command linen options
if [ -n "$YVAL"  -a  -n "$yVAL" ]
then
	dspmsg -s 1 cmdlvm.cat 688 "`lvmmsg 688`\n" mklv >& 2
	Usage
	exit
fi

if [ -n "$YVAL" ]
then
	VALID_CHECK=`odmget -q "prefix= $YVAL" PdDv`
	if [ -n "$VALID_CHECK" ]
	then
                dspmsg -s 1 cmdlvm.cat 693 "`lvmmsg 693`\n" mklv "Y" >& 2
		dspmsg -s 1 cmdlvm.cat 822 "`lvmmsg 822`\n" mklv >& 2
	exit
	fi

fi

# check illegal combinations
if [ -n "$MVAL"  -a \( -n "$AVAL" -o -n "$EVAL" -o -n "$UVAL" -o -n "$SVAL" \) ]
then
	dspmsg -s 1 cmdlvm.cat 690 "`lvmmsg 690`\n" mklv >& 2
	Usage
	exit
fi

if [ "$WVAL" = 'y' -a  \( "$TVAL" = 'paging' \) ]
then
	dspmsg -s 1 cmdlvm.cat 1005 "`lvmmsg 1005`\n" mklv >& 2
	Usage
	exit
fi

#
# if -S (striping lv) flag specified 
#
if [ -n "$SSFLAG" ]
then
	#
	# not allowed with -c, -e, -w, -d, -m, or -s flags
	#
	if  [ \( -n "$CFLAG" -o -n "$EFLAG" -o -n "$WFLAG" -o \
		 -n "$DFLAG" -o -n "$MFLAG" -o -n "$SFLAG" \) ]
	then
		dspmsg -s 1 cmdlvm.cat 1032 "`lvmmsg 1032`\n" mklv >& 2
		Usage
		exit
	fi

	if [ "$RVAL" = 'y' ]
	then
		dspmsg -s 1 cmdlvm.cat 1041 "`lvmmsg 1041`\n" mklv >& 2
		Usage
		exit
	else
		#
		# all striped locical volumes must be set to non-relocatable
		#
		RFLAG='-r';  RVAL='n'
	fi

	if [ -z "$pvname" ]
	then
  		# if neither upperbound or pvnames specify
   		# exit with error message
		if [ -z "$UVAL" ]
		then
	      		dspmsg -s 1 cmdlvm.cat 1037 "`lvmmsg 1037`\n" mklv >& 2
	      		Usage
	      		exit
		else
			if [ "$UVAL" -eq 1 ]
			then
				echo "The -u value of 1 is not valid for striping"
				echo "Choose another value."
	 			dspmsg -s 1 cmdlvm.cat 822 "`lvmmsg 822`\n" mklv >& 2
				exit
			fi

			# make sure the number of LPs is a multiple of
			# the stripe width

			LFOVER=`expr $NUMLPS % $UVAL`
			if [ "$LFOVER" != 0 ]
			then
				dspmsg -s 1 cmdlvm.cat 1036 "`lvmmsg 1036`\n" mklv >& 2
				exit
			fi

	      		# Estimate the number of partitions that will be 
			# allocate from each disk

	      		ES_AVEPPS=`expr $NUMLPS / $UVAL`
		fi
	fi

	if [ "$TVAL" = 'boot' ]
	then
		# can't create a striped lv on as a boot logical volume
		dspmsg -s 1 cmdlvm.cat 1048 "`lvmmsg 1048`\n" mklv >& 2
		exit
	fi
fi

# default interpolicy is use minimum number of pvs 
if [ -z "$EFLAG" ] 
then
	EFLAG=-e
	EVAL=m
fi

VGID=`getlvodm -v $VGNAME`      #get logical volume group id from odm
test_return $?                  #check for getlvodm error

if [ "$mklv_NOLOCK" != 1 ]              #If not locked before  lock the volume group
then
	putlvodm -k $VGID               #lock the volume group
	test_return $?                  #check for putlvodm error
	VGLOCKED=1
fi

MAJOR=`getlvodm -d $VGID`       #get volume group major number from odm
test_return $?

#
# get the logical volume name ...
#

if [ -n "$yFLAG" ]                   #if lvname specified as cmd line option
then
	LVNAME=$yVAL
elif [ -z "$YFLAG" ]                   #If no prefix and no lvname
then
	TYPE=$TVAL                        #Save the lvtype
fi

# Call getlvname to get/verify the lvname.
# Lvnames are formed by concatenating the -Y lvname prefix
# value with a sequence number.  If the -Y option is not specified
# on the command line, getlvname will get the prefix value from the ODM
# Prefix Object Class, using the -t type option to search for a match.
# If a match is not found, getlvname will use the default prefix, lv,
# to build the lvname.
#

if [ -n "$LVNAME" ]                   #if lvname specified as cmd line option
then
	NFLAG=-n
fi

# check the lv name. 
LVNAME=`getlvname $NFLAG $LVNAME $YFLAG $YVAL $TYPE`
test_return $?

#
# Call lvgenminor to get the lv's minor number
#
MINOR=`lvgenminor $MAJOR $LVNAME`
test_return $?
GOTMINOR=1

# Construct the lvid

LVID=$VGID"."$MINOR

#
# Two devices must be created for each logical volume, one for block
# i/o and the other for raw i/o.  Call mknod to do this.
#
mknod /dev/$LVNAME b $MAJOR $MINOR    # block i/o device
test_return $?                        # check for error return from mknod

mknod /dev/r$LVNAME c $MAJOR $MINOR   # raw i/o device -- lvname prefixed with r
test_return $?                        # check for error return from mknod

MADENODE=1  # signals cleanup that the nodes must be deleted

#
# Convert option values for following call to lcreatelv
#

if [ -n "$BFLAG" ]      #Bad block relocation option specified?
then
   BFLAG=-r
   if [ "$BVAL" = y ]   #Bad block relocation specified as yes?
   then                 #Set BVAL to 1 to indicate bad block
      BVAL=1            #relocation for lcreatelv.
   else
      BVAL=2            #Bad block relocation specified as no. Convert
   fi                   #BVAL from n(o) to 2 for lcreatelv.
fi

if [ -n "$DFLAG" ]      #Scheduling policy option specified?
then
   if [ "$DVAL" = s ]   #Sequential scheduling policy specified?
   then                 #Set DVAL to 1 to indicate sequential
      DVAL=1            #scheduling policy for lcreatelv.
   else                 #Parallel scheduling policy specified.
      DVAL=2            #Set DVAL to 2 for lcreatelv.
   fi
fi

if [ -n "$VFLAG" ]      #Write/verify state option specified?
then
   if [ "$VVAL" = y ]   #Write/verify specified as yes - set to 1
   then                 #for lcreatelv.
      VVAL=1
   else                 #Write/verify specified as no - set to 2 for
      VVAL=2            #lcreatelv.
   fi
fi

if [ -n "$WFLAG" ]      #MirrorWrite consistency option specified?
then
   if [ "$WVAL" = y ]   #MirrorWrite consistency specified as yes - set to 1
   then                 #for lcreatelv.
      WVAL=1
   else                 #MirrorWrite consistency specified as no - set to 2 for
      WVAL=2            #lcreatelv.
   fi
elif [ "$TVAL" = 'paging' ] # no mirrorwrite consistency
then                                               # for paging or log file syst
    WFLAG=-w
    WVAL=2
else                 # default for other types is mirrorwrite consistency
	WFLAG=-w
	WVAL=1
fi

XFLAG=${XFLAG:+"-s"}

# If -m mapfile option specified, then copy mapfile to pvmap$$ for input to
# allocp.  Otherwise, build list of pvmaps for input to allocp.
if [ -n "$MFLAG" ]
then                      # pv mapfile is specified on command line
	#check on the filesize of MVAL before going on
	#fail if /tmp is too small
	TEMPSIZ=`wc -c < $MVAL`
	TMPFILE1=`lmktemp /tmp/pvmap$$ $TEMPSIZ`
	if [ $? != 0 ]            #Check for error return from lmktemp
	then                      #error return - fatal error
	  fatal_errhdlr       #Display error msg and set $EXIT_CODE to 2
	fi

	cp $MVAL $TMPFILE1    # copy it to pvmap$$ for input to allocp.
	test_return $?
	EFLAG='-e';  EVAL=p;     # if map file allocp will expect the -e p
else

   # Get list of PVIDs to be used in allocating logical partitions.
   # If pvnames are specified on the command line, only corresponding
   # physical volumes will be used for allocating.  If no pvnames
   # are specified, all physical volumes within the volume group will
   # be used for allocation.

   if [ -n "$pvname" ]    #if pvname is not null
   then            #pvname(s) specified on command line -- call getlvodm
                   #to get the pvid for each pvname and store in $pvlist.
      for name in $pvname
      do
          temp=`getlvodm -p $name`   #get the pvid
          test_return $?             #check for error return from getlvodm
          pvlist="$pvlist $temp"     #append the pvid to pvlist
      done     #end - for each pvname

   else                #pvnames not specified on command line - use all pv's
                       #within volume group.

      #
      # Call lqueryvg to get the list of all pvids in the volume group.
      # Use the cut command to retrieve only the pvids from lqueryvg's
      # output tuples, assigning the output of cut to string variable pvlist.
      #

      #check on the filesize of lquery output before going on
      #fail if /tmp is too small
      TEMPSIZ=`lqueryvg -g $VGID -P | wc -c`
      PVLSTFILE=`lmktemp /tmp/pvlist$$ $TEMPSIZ`
      if [ $? != 0 ]            #Check for error return from lmktemp
      then                      #error return - fatal error
	  fatal_errhdlr      #Display error msg and set $EXIT_CODE to 2
      fi

      lqueryvg -g $VGID -P > $PVLSTFILE #Store lqueryvg output in temporary file
      test_return $?                    #Check for error return from lqueryvg

      for PVID in `cat $PVLSTFILE |cut -d" " -f1`
      do                         #for each pvid in the list

         PVSTATE=`lquerypv -g $VGID -p $PVID -c`  #get the state of this pv

         if [ `test_state $PVSTATE 1` -eq 1 -o `test_state $PVSTATE 2` -eq 1 ]
          then
            dspmsg -s 1 cmdlvm.cat 834 "`lvmmsg 834`\n" mklv `getlvodm -g $PVID` >& 2
          else
	    # if creating a stripe lv and pvnames are not specify in command line,
	    # then search for the pvs which have enough free partition to allocate
	    if [ -n "$SSFLAG" ] 
	    then
      	 	#physical volume is available for allocation - not set to NOALLOC
      		NOALLOC=`test_state $PVSTATE 3`
      		if [ "$NOALLOC" -ne 1 ]
      		then
		   TOTALPPS=`lquerypv -p $PVID -g $VGID -P`
		   USEDPPS=`lquerypv -p $PVID -g $VGID -n`
		   FREEPPS=`expr $TOTALPPS - $USEDPPS`
		   if [ $FREEPPS -ge $ES_AVEPPS ]
		   then
		      if [ "$PVNUM" -eq "$UVAL" ]
		      then
			 # enough PPs to allocate
			 break
		      else
		         pvlist="$pvlist $PVID"
	 	         PVNUM=`expr $PVNUM + 1`
		      fi
		   fi
	       fi
	    else
		pvlist="$pvlist $PVID"
            fi  
 	fi
      done
      # if PVNUM != UVAL, there is not
      # enough physical volume for
      # stripe logical volume
      if [ -n "$SSFLAG" -a "$PVNUM" != "$UVAL" ]
      then
      	dspmsg -s 1 cmdlvm.cat 1043 "`lvmmsg 1043`\n" mklv $LVNAME >& 2
	test_return 1
      fi	
   fi  #end if pvname not null

   PVNUM=0
   #
   # For each pvid in $pvlist, determine if its state is set to allocation.
   # If so, get the pv map and append to file pvmap$$.
   #

   #check on the filesize of lquery output before going on
   #fail if /tmp is too small
   TEMPSIZ=`lqueryvg -g $VGID -P | wc -c`
   PVLSTFILE=`lmktemp /tmp/pvlist$$ $TEMPSIZ`
   if [ $? != 0 ]            #Check for error return from lmktemp
   then                      #error return - fatal error
	  fatal_errhdlr   #Display error msg and set $EXIT_CODE to 2
   fi

   lqueryvg -g $VGID -P > $PVLSTFILE #Store lqueryvg output in temporary file

   >/tmp/pvmap$$              # make sure that the file has zero lenghth
   for PVID in $pvlist
   do                         #for each pvid in the list

      #
      # check to see if PVID is in $PVLSTFILE (created from lquerypv command)
      # is pv in vg?
      #
      grep -i $PVID $PVLSTFILE > /dev/null 2>&1
      if [ $? -ne 0 ]
      then
      	 dspmsg -s 1 cmdlvm.cat 15 "`lvmmsg 15`\n" "mklv: [`getlvodm -g $PVID`]" >& 2
	 test_return 1
      fi
      
      PVSTATE=`lquerypv -g $VGID -p $PVID -c`  #get the state of this pv

      if [ $? != 0 -o `test_state $PVSTATE 1` -eq 1 -o `test_state $PVSTATE 2` -eq 1 ]
      then
	 Pvfatal
      fi  #end - error return from lquerypv

      #physical volume is available for allocation - not set to NOALLOC
      NOALLOC=`test_state $PVSTATE 3`
      if [ "$NOALLOC" -ne 1 ]
      then
	#check on the filesize of lquery output before going on
	#fail if /tmp is too small
	TEMPSIZ=`lquerypv -g $VGID -p $PVID -d -t | wc -c`
	TMPFILE1=`lmktemp /tmp/size.check$$ $TEMPSIZ`
	if [ $? != 0 ]            #Check for error return from lmktemp
	then                      #error return - fatal error
	  fatal_errhdlr       #Display error msg and set $EXIT_CODE to 2
	fi
	# remove the temp file now that we know the data will fit
	rm $TMPFILE1
         #
         # get physical volume map - store in pvmap$$
	 lquerypv -g $VGID -p $PVID -d -t >> /tmp/pvmap$$
         if [ $? != 0 ] #error return from lquerypv
         then
	    Pvfatal
	 fi
	 PVNUM=`expr $PVNUM + 1`
      else
	 # pvname entered on command line?
	 if [ -n "$pvname" ]
	 then
            dspmsg -s 1 cmdlvm.cat 823 "`lvmmsg 823`\n" mklv "$pvname" >& 2
	    Pvfatal
	 fi
      fi

   done   #end - for each pvid

fi       # end - if map file specified

rm -f $PVLSTFILE

#
# If -s y or n (strict policy) was specified on the command line, set up
# $SVAL for call to allocp.  (If it was not specified, the default value
# of strict policy will be assumed.)
#

SS=" "
if  [ -n "$SFLAG" ]           #If $SFLAG not null (ie. if already specified
then                          #on mklv cmd line)
	if [ "$SVAL" = n ]         #if $SVAL indicates non-strict policy, set
	then                       #$SS to -k for input to allocp.
		SS=-k
	fi
fi

# For stripe lv: 1) number of copy should be 1 
#		2) the max. number of physical volumes to 
#		   extend should be the number of physical volumes
# 		   specified in the command line
#		3) allocation policy should be across the max. 
#		   number of physical volumes.
#		4) The size of logical partition should be evenly
#		   distributed across disks being striped.
#
#		Before calling allocp, make sure there is enough
#		logical partition on each disk.

if [ -n "$SSFLAG" ]
then
	CFLAG='-c'    # number of copies
	CVAL=1 
	UFLAG='-u'    # upperbound - max. of physical volume
	UVAL=$PVNUM 
	EFLAG='-e'    # allocation policy
	EVAL='x'
	DFLAG='-M'    # set scheduling policy to SCH_STRIPED
	DVAL=5
	OFLAG='-O'    # stripe width flag
	# round off the number of logical partitions so
	# they can be evenly distribute across the disks
	EXLPS=0
        LFOVER=`expr $NUMLPS % $PVNUM`
	if [ "$LFOVER" != 0 ]
	then
		EXLPS=`expr $PVNUM - $LFOVER`
	fi
	NUMLPS=`expr $NUMLPS + $EXLPS`
	AVEPPS=`expr $NUMLPS / $PVNUM`	# logical partition in
					# each disk
   	for PVID in $pvlist
   	do     
		TOTALPPS=`lquerypv -p $PVID -g $VGID -P`
		USEDPPS=`lquerypv -p $PVID -g $VGID -n`
		FREEPPS=`expr $TOTALPPS - $USEDPPS`
		if [ $FREEPPS -lt $AVEPPS ]
		then
			PVNAME=`getlvodm -g $PVID 2>/dev/null`
			dspmsg -s 1 cmdlvm.cat 1034 "`lvmmsg 1034`\n" mklv $PVNAME >& 2
			test_return 1
		fi
	done
else
	if [ -n "$UVAL" -a "$UVAL" -lt 32 ]
	then
	# For a non-striped lv, make sure that the lv created won't violate -u
		CHK_DISK_LIMIT=1
	fi
fi

# Call allocp to generate an allocation map for the new logical volume
allocp -i $LVID    \
	-t e               \
	$CFLAG $CVAL       \
	-s $NUMLPS         \
	$SS                \
	$UFLAG $UVAL       \
	$EFLAG $EVAL       \
	$AFLAG $AVAL       \
		< /tmp/pvmap$$ \
		> /tmp/aout$$

test_return $?          #check for error return from allocp

if [ -n "$CHK_DISK_LIMIT" ]
then
	UNIQUE_DISKS=`cat /tmp/aout$$ | cut -f1 -d" "| sort | uniq | wc -l`
	if [ "$UNIQUE_DISKS" -gt "$UVAL" ]
	then
        #this implies that the result of the allocp mapping would violate -u
		echo "The lv creation would violate user provided -u limit."
		echo "Change -u value or designate specific disks for mklv."
	 	dspmsg -s 1 cmdlvm.cat 822 "`lvmmsg 822`\n" mklv >& 2
		exit
        fi

fi

if [ -z "$XFLAG" -a "$NUMLPS" -gt 128 ]
then
	XFLAG=-s
	XVAL=$NUMLPS
fi

if [ -z "$OFLAG" ]
then
   PVNUM=0
fi

lcreatelv -N $LVNAME -g $VGID -n $MINOR $WFLAG $WVAL $DFLAG $DVAL $XFLAG $XVAL \
$BFLAG $BVAL $VFLAG $VVAL $SSFLAG $SSIZE -O $PVNUM
test_return $?  #test for error return from lcreatelv
MADELV=1

# give the  logical volume its pps
lextendlv -l $LVID -s `wc -l < /tmp/aout$$` /tmp/aout$$
test_return $?             #check for error return from lextendlv

if [ -z "$LFLAG" ]                  
then
	LFLAG=-L
	LFLAG2=-B
	LVAL='None'
fi

if [ "$EVAL" = p ]
then
       EVAL='m'
fi

putlvcb $AFLAG $AVAL $CFLAG $CVAL $EFLAG $EVAL -i $LVID $RFLAG $RVAL \
$SFLAG $SVAL $TFLAG $TVAL $LFLAG "$LVAL" $UFLAG $UVAL $SSFLAG $SSIZE \
-O $PVNUM -n $NUMLPS -N $LVNAME
if [ $? != 0 ]      #check for error return from putlvcb
then
	dspmsg -s 1 cmdlvm.cat 622 "`lvmmsg 622`\n" mklv >& 2
fi

putlvodm $AFLAG $AVAL $CFLAG $CVAL $EFLAG $EVAL -l $LVNAME $RFLAG $RVAL \
$SFLAG $SVAL $TFLAG $TVAL $LFLAG2 "$LVAL" $UFLAG $UVAL $SSFLAG $SSVAL \
-O $PVNUM -z $NUMLPS $LVID
if [ $? != 0 ]          #check for error return from putlvodm, exit if error
then
	dspmsg -s 1 cmdlvm.cat 1023 "`lvmmsg 1023`\n" mklv >& 2
	exit
fi

echo $LVNAME

EXIT_CODE=0       #Reset exit code to indicate successful completion of mklv
exit              #trap will handle cleanup.
