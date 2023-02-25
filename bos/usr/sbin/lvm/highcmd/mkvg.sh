#!/bin/ksh
# @(#)89	1.21.2.9  src/bos/usr/sbin/lvm/highcmd/mkvg.sh, cmdlvm, bos41J, 9520A_all 5/9/95 16:23:56
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: mkvg.sh
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

# FILE NAME: mkvg
#
# FILE DESCRIPTION: High-level logical volume shell command for making
#                   a new volume group using specified physical
#                   volume(s).
#
# RETURN VALUE DESCRIPTION:
#	0	Successful
#	1	Unsuccessful
#
#
# EXTERNAL PROCEDURES CALLED: getlvodm, getvgname, lcreatevg,
#	linstallpv, putlvodm, lvgenminor, lvgenmajor
#
#
# GLOBAL VARIABLES: none
#

######################### getppsize #####################################
#
# NAME: getppsize
#
# DESCRIPTION: Maps the input meg size to a number that raises 2 to
# the number of bytes for that number of megabytes.
# The valid input meg sizes are 1, 2, 4, 8, 16, 32, 64, 128, 256
#
# NOTE: This function will not return if input value is invalid.
#
getppsize()
{

	case $1 in
		1) PPSIZE=20;;
		2) PPSIZE=21;;
		4) PPSIZE=22;;
		8) PPSIZE=23;;
		16) PPSIZE=24;;
		32) PPSIZE=25;;
		64) PPSIZE=26;;
		128) PPSIZE=27;;
		256) PPSIZE=28;;
		*) dspmsg -s 1 cmdlvm.cat 682 "`lvmmsg 682`\n" mkvg >& 2; exit;;
	esac

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
# NOTE: This function will not return if input value is invalid. (exit 1)
#
test_return()
{
	if [ "$1" != 0 ]
	then
		dspmsg -s 1 cmdlvm.cat 862 "`lvmmsg 862`\n" mkvg >& 2 ;
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
#	EXIT_CODE
#
# NOTE: This function will not return.
#
#
cleanup()
{
        trap '' 0 1 2 15 

	# unlock the volume group?
	if test $LOCKEDVG -eq 1
	then
		putlvodm -K $VGID > /dev/null 2>&1
		LOCKEDVG=0
	fi

	if test $EXIT_CODE -eq 1
	then
		if test $GOTMAJOR -eq 1
		then
			# release the major number if necessary
			lvrelmajor $VGNAME > /dev/null 2>&1

			# release the minor number if necessary
			if test $GOTMINOR -eq 1
			then
				lvrelminor $VGNAME >/dev/null 2>&1
			fi
		fi

		# remove the vg device?
		if test $MADENODE -eq 1
		then
			rm -f /dev/$VGNAME
		fi

	fi

	getlvodm -R 
	if [ $? -eq 0 ]
	then
		savebase	#save the database to the ipl_device
	fi

        umask $OLD_UMASK	# restore umask 
	exit $EXIT_CODE
}

ckvgda()
{
	# value must be between 1 and 32
	test $1 -lt 33 -a $1 -gt 0 > /dev/null 2>&1 && return 0

	dspmsg -s 1 cmdlvm.cat 868 "`lvmmsg 868`\n" mkvg >& 2 ;
	exit
}

getfirst()
{
	echo $1
}

shiftit()
{
	shift
	echo $*
}

Usage()
{
	dspmsg -s 1 cmdlvm.cat 860 "`lvmmsg 860`\n" mkvg >& 2 ;
}

############################## main ############################################
#  Make volume group
#  Input:
#       Command line options and arguments:
#       mkvg [-n] [-f] [-i] [-s ppsize] [-y vgname] [-d MaxPVs] pvname...
#  Output:
#       Error Messages (Standard error)
#
hash test_return putlvodm
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if mklv completes successfully.
LOCKEDVG=0
GOTMINOR=0
GOTMAJOR=0
MADENODE=0
MADEVG=0
PVOK=0

OLD_UMASK=`umask`	# save old umask value
umask 117		# set umask for rw-rw---- 

# Trap on exit/interrupt/break to clean up
trap 'cleanup' 0 1 2 15 

# Parse command line options
set -- `getopt d:s:y:m:nfiV: $*`

if [ $? != 0 ]                      # Determine if there is a syntax error.
then
	Usage
	exit
fi

NFLAG= ; IFLAG= ; SFLAG= ; SVAL= ; YFLAG= ; YVAL= ; DFLAG= ; DVAL= ; FFLAG=
MFLAG= ; MVAL= ; VFLAG= ; VVAL=

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
		   -m) MFLAG='-m';	MVAL=$2; shift; shift;;	#maximum size of PV 
           -n) NFLAG='-n';  shift;;             #vg not automatically varied
                                                #on at IPL
           -i) IFLAG='-i';  shift;;             #pvnames read from standard in
           -f) FFLAG='-f';  shift;;             #force flag
           -s) SFLAG='-s';  SVAL=$2;  shift;  shift;;   #ppsize
# Get vgname entered by user into VGNAME for preliminary validation.
           -y) YFLAG='-y';  YVAL="-n $2"; VGNAME=$2;  shift;  shift;; #vgname
	   -d) DFLAG='-d';  DVAL=$2; shift; shift;; # descriptor size
           -V) VFLAG='-V';  VVAL=$2; shift; shift;; # major number
  esac
done   #end - while there is a command line option

shift     # skip past "--" from getopt

# were the pvnames specified on the command line?
if [ -z "$IFLAG" ]
then           

	# Was a pvname specified on the command line?
	if test $# -ne 0
	then
		PVNAME=$1
	else
		dspmsg -s 1 cmdlvm.cat 604 "`lvmmsg 604`\n" mkvg >& 2 ;
		Usage
		exit
	fi

	# skip past pvname argument
	shift;
	PVLIST="$*"

else                  #-i option specified -- read pvnames from standard in

	# read the PVLIST
	while read LINE
	do
		PVLIST="$PVLIST $LINE"
	done

	if test -z "$PVLIST"
	then
		dspmsg -s 1 cmdlvm.cat 604 "`lvmmsg 604`\n" mkvg >& 2 ;
		Usage
		exit
	fi

	#extract the 1st pvname from PVLIST and save in var PVNAME
	PVNAME=`(getfirst $PVLIST)`
	PVLIST=`(shiftit $PVLIST)`
fi

# we must have at least one pvname to create a volume group
if [ -z "$PVNAME" ]
then
	dspmsg -s 1 cmdlvm.cat 604 "`lvmmsg 604`\n" mkvg >& 2 ;
	Usage
	exit
fi
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
   dspmsg -s 1 cmdlvm.cat 794 "`lvmmsg 794`\n" mkvg $PVNAME >& 2
   exit 1
fi
PVOK=0

#
# Check for disk without PVID and
# for PV that is already in a VG
#
PVID=`getlvodm -p $PVNAME`
test_return $?
if [ "$PVID" = 0000000000000000 ]
then
   dspmsg -s 1 cmdlvm.cat 796 "`lvmmsg 796`\n" mkvg $PVNAME >& 2
   chdev -l $PVNAME -a pv=yes >/dev/null 
   test_return $?
else
   getlvodm -j $PVNAME >/dev/null 2>&1
   if [ $? = 0 ]
   then
      dspmsg -s 1 cmdlvm.cat 696 "`lvmmsg 696`\n" mkvg $PVNAME >& 2
      test_return 1
   fi
fi

# For read/write optical, call "chdev" to write the boot
# record and media id on the optical media
if [ -x /usr/bin/odmget ]
then
	if [ ! -z "`odmget -q \"name=${PVNAME} and \
		PdDvLn LIKE rwoptical*\" CuDv 2>/dev/null`" ] # omd device found
	then
      		chdev -l $PVNAME -a pv=yes > /dev/null
      		test_return $?
	fi
fi

# Check the PV's in PVLIST to see if they belong in a VG
if [ -n "$PVLIST" ]
then
    for PVTEST in $PVLIST
    do
        # 
        # Check for disk that is not configured
        #
        CONFIGPVS=`getlvodm -P | cut -d" " -f1` 
        test_return $?
        for TESTPV in $CONFIGPVS
        do
           if [ "$TESTPV" = "$PVTEST" ]
           then
              PVOK=1
           fi
        done

        if [ "$PVOK" = 0 ]
        then 
           dspmsg -s 1 cmdlvm.cat 794 "`lvmmsg 794`\n" mkvg $PVTEST >& 2
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
	   chdev -l $PVNAME -a pv=yes
	   test_return $?
	else
	   #
	   # Check for PV that is already in a VG
	   #
	   getlvodm -j $PVNAME >/dev/null 2>&1
	   if [ $? = 0 ]
	   then
	      dspmsg -s 1 cmdlvm.cat 696 "`lvmmsg 696`\n" mkvg $PVNAME >& 2
	      test_return 1
	   fi
	fi
    done
fi
        
# Determine the physical partition size to be passed to lcreatevg.
# The posible values are 20 through 28. 2 will be raised to this
# power to determine the size for a pp. (20=1 meg, 28=256 meg)

# -s ppsize option not specified?
if [ -z "$SFLAG" ]  
then                 
	# partion size of 4 megabyes is default
	PPSIZE=22
	SVAL=4
else                 
	# Calculate the partion size
	getppsize $SVAL
fi

if [ -z "$DFLAG" ]  
then
	DVAL=32
else
	ckvgda $DVAL
fi

# Before performing any checks, check VGNAME for any
# non-alphanumeric characters. Do not allow the vg to get
# imported with a bad vgname.

# the case statement checks all the invalid characters:
# 	<space> through ,   
# 	/
# 	: through @
# 	[ through ^
# 	`
# 	{ through ~
#
# this leaves only the valid cases [A-Z][a-z][0-9][.-_]

# check to see if the user specified a VGNAME first...
if [ -n "$VGNAME" ]
then
    case $VGNAME in 
	*[\ -,/:-@[-\^\`\{-~]*)
        	#Invalid character in vgname
        	dspmsg -s 1 cmdlvm.cat 874 "`lvmmsg 874`\n" mkvg $VGNAME >& 2
        	test_return 1;;
    esac
fi

# generate and/or validate existence of the VGNAME
VGNAME=`getvgname $YVAL`
test_return $?      

# get the pvid for the first pv
PVID=`getlvodm -p $PVNAME`
test_return $?      

if [ -z "$VFLAG" ]
then

    # Get major number.
    MAJOR=`lvgenmajor $VGNAME`
    test_return $?       
    GOTMAJOR=1
else
    # Check major number.
    MAJOR=`lvchkmajor $VVAL $VGNAME`
    test_return $?
    GOTMAJOR=1
fi

# Get minor number ( should be zero )
MINOR=`lvgenminor $MAJOR $VGNAME`
test_return $?       
GOTMINOR=1

#
# Create special device file for volume group with mknod 
#
mknod /dev/$VGNAME c $MAJOR $MINOR
if test $? -ne 0
then
    dspmsg -s 1 cmdlvm.cat 870 "`lvmmsg 870`\n" mkvg /dev/$VGNAME >& 2
    exit
fi

MADENODE=1

#
# Calculate the volume group descriptor area size (VGDASIZE) and maximum
# number of logical volumes (MAXLVS) for input to lcreatevg.
#
if [ -z "$MFLAG" ]
then
    PPSPPV=1016			#each PV can have up to 1016 pps
else
	if [ -z "$MVAL" ]
	then
		Usage; exit
	else
		PPSPPV=`expr $MVAL / $SVAL`
		if [ "$PPSPPV" -gt 1016 ]
		then
		 dspmsg -s 1 cmdlvm.cat 872 "`lvmmsg 872`\n" mkvg "$PPSPPV"  
		 PPSPPV=1016	# 1016 is limitation
		fi
	fi
fi

if [ -x /usr/sbin/bootinfo ]
then
        DISK_SIZE=`/usr/sbin/bootinfo -s $PVNAME`
        PP_COUNT=`expr $DISK_SIZE / $SVAL`
        if [ "$PP_COUNT" -gt "$PPSPPV" ]
        then
#dspmsg -s 1 cmdlvm.cat 876 "`lvmmsg 876`\n" mkvg "$SVAL" "$PP_COUNT" "$PVNAME"

	echo "5016-876 mkvg: Warning, the Physical Partition Size of $SVAL"
	echo "requires the creation of $PP_COUNT partitions for $PVNAME."
	echo "The system limitation is 1016 physical partitions per disk."
	echo "Specify a larger Physical Partition size in order to create a volume group on this disk."
                dspmsg -s 1 cmdlvm.cat 862 "`lvmmsg 862`\n" mkvg >& 2 ;
                exit
        fi

        if [ -n "$PVLIST" ]
        then
                for PVTEST in $PVLIST
                do
                        DISK_SIZE=`/usr/sbin/bootinfo -s $PVTEST`
                        PP_COUNT=`expr $DISK_SIZE / $SVAL`
                        if [ "$PP_COUNT" -gt "$PPSPPV" ]
                        then
#dspmsg -s 1 cmdlvm.cat 876 "`lvmmsg 876`\n" mkvg "$SVAL" "$PP_COUNT" "$PVTEST"
			echo "5016-876 mkvg: Warning, the Physical Partition Size of $SVAL"
			echo "requires the creation of $PP_COUNT partitions for $PVTEST."
			echo "The system limitation is 1016 physical partitions per disk."
			echo "Specify a larger Physical Partition size in order to create a volume group on this disk."
                                dspmsg -s 1 cmdlvm.cat 862 "`lvmmsg 862`\n" mkvg >& 2 ;
                                exit
                        fi
                done
        fi

fi
MAXLVS=256
LVSBLK=16; PPSBLK=16; NAMESBLK=8
LVLIST=`expr $MAXLVS + $LVSBLK - 1`		# LV entries
PPLIST=`expr $PPSPPV + 1 + $PPSBLK - 1`	# PP entries
NAMELIST=`expr $MAXLVS + $NAMESBLK - 1`	# Name entries
VGDASIZE=`expr $LVLIST / $LVSBLK + $PPLIST / $PPSBLK \* $DVAL + $NAMELIST / $NAMESBLK + 2`

# Create the volume group installing the first physical volume.
VGID=`lcreatevg -a $VGNAME -V $MAJOR -N $PVNAME -n $MAXLVS -D $VGDASIZE -s $PPSIZE $FFLAG`
rc=$?
case $rc in
	# Everything is okay, just added mkboot to zero out any
	# potential old boot image that might reside on this disk
	0) if [ -x /usr/sbin/mkboot ]
	   then
		mkboot -c -d /dev/$PVNAME
	   fi;;

	#
	# test for PV already in a volume group
	# If it is in a volume group then retry with the force flag and
	# give the user the choose as to what to do.
	#
        3) dspmsg -s 1 cmdlvm.cat 631 "`lvmmsg 631`\n" mkvg $PVNAME
           dspmsg -s 1 cmdlvm.cat 632 "`lvmmsg 632`\n" mkvg
           read RESPONSE
           tstresp $RESPONSE
           if [ $? = 1 ]
           then             # positive response - retry 
               VGID=`lcreatevg -a $VGNAME -V $MAJOR -N $PVNAME -n $MAXLVS \
               -D $VGDASIZE -s $PPSIZE -f `
	       test_return $?
		# This line added to zero out any potential old boot images
		# that might reside on the disk.
		if [ -x /usr/sbin/mkboot ]
	   	then
			mkboot -c -d /dev/$PVNAME
	   	fi
	   else
	       test_return 1
	   fi;;

	*) test_return 1;;
	esac

# We no longer want to remove the vg should we be terminated
MADEVG=1
EXIT_CODE=0
# dspmsg -s 1 cmdlvm.cat 864 "`lvmmsg 864`\n" mkvg $VGNAME

echo $VGNAME    # name of the Volume group created


# Are we not varyoning the vg at ipl
if [ -n "$NFLAG" ]
then            
	AUTO_ON=n
else
	AUTO_ON=y
fi

# Add new volume group to odm.
putlvodm -v $VGNAME -o $AUTO_ON -q 0 $VGID
if [ $? != 0 ]       #If putlvodm failed, output warning and continue.
then
	dspmsg -s 1 cmdlvm.cat 1024 "`lvmmsg 1024`\n" mkvg >& 2
fi

putlvodm -k $VGID
if test $? -eq 0
then
	LOCKEDVG=1
fi
# add the pvid to the vg
putlvodm -p $VGID $PVID
if [ $? != 0 ]       #If putlvodm failed, output warning and continue.
then
	dspmsg -s 1 cmdlvm.cat 1025 "`lvmmsg 1025`\n" mkvg $PVNAME $VGNAME >& 2
fi

# Add additionial physical volumes to the volume group?
if test -n "$PVLIST"
then
	# varyon the vg in managment mode
	echo "$PVID $PVNAME" | lvaryonvg -a $VGNAME -V $MAJOR -g $VGID -pn - >/dev/null 2>&1 &&
	(
		# add the additionial physical volumes
		extendvg_NOLOCK=1
		export extendvg_NOLOCK
		extendvg $FFLAG $VGNAME $PVLIST 
		unset extendvg_NOLOCK
		lvaryoffvg -g $VGID > /dev/null 2>&1 || true

	)  ||   dspmsg -s 1 cmdlvm.cat 792 "`lvmmsg 792`\n" mkvg >&2
fi

putlvodm -K $VGID > /dev/null 2>&1
#
# call varyonvg without the syncvg option, since this is a new vg.
#
varyonvg -n $VGNAME
exit              #trap will handle cleanup.
