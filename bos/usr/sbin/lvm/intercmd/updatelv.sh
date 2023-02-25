#! /bin/bsh
# @(#)44	1.14.1.7  src/bos/usr/sbin/lvm/intercmd/updatelv.sh, cmdlvm, bos411, 9428A410j 6/22/94 16:14:55
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: updatelv.sh
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
# FILE NAME: updatelv
#
# FILE DESCRIPTION: High-level logical volume shell command for synchronizing
#                   or rebuilding the logical volume control block and the
#                   data base. The Volume Group must be varied on when this
#                   command is executed. 
#
#
# RETURN VALUE DESCRIPTION:
#                             0     Successful
#                             1     Unsuccessful
#
#
# EXTERNAL PROCEDURES CALLED: getlvname, getlvodm, lvgenminor, lvrelminor,
#                             lquerylv, lchangelv, putlvodm, putlvcb,
#                             getlvcb, mknod
#
#
# GLOBAL VARIABLES: none
#

############################ cleanup ########################################
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
#       EXIT_CODE
#
# NOTE: This function will not return.
#
#
cleanup()
{
	trap '' 0 1 2 15
	rm -f /tmp/lvlist$$ /tmp/lvcb$$ /tmp/odmdata$$
           
        for NAME in $RESERVED
        do
            putlvodm -L ${VGID}.999  #remove the bogus LV
        done

	if test $DMADENODE -eq 1
	then
		rm -f /dev/rdummylv$$ /dev/dummylv$$
	fi

        umask $OLD_UMASK	# restore umask 

	exit $EXIT_CODE
}



########################### fatal_error #####################################
#
# NAME: fatal_error()
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
fatal_error()
{
    if [ "$1" != 0 ]
    then
	dspmsg -s 1 cmdlvm.cat 522 "`lvmmsg 522`\n" $PROG $LVNAME >& 2
        EXIT_CODE=1
	exit $EXIT_CODE       #  reset EXIT_CODE
    fi
}

########################### nonfatal_error ##################################
#
# NAME: nonfatal_error()
#
# DESCRIPTION: Tests function return code. Will output error message
#              if bad and set exit code to 2
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
#
nonfatal_error()
{
    if [ "$1" != 0 ]
    then
	dspmsg -s 1 cmdlvm.cat 520 "`lvmmsg 520`\n" $PROG $LVNAME >& 2
	EXIT_CODE=2
    fi
}


######################### is_lvid_in_odm ##############################
#
# NAME: is_lvid_in_odm()
#
# DESCRIPTION: Checks whether LVID is defined in odm under a different
#              lvname, yes  then change the lvname in odm
#                      no   then define a new lvname in odm
#
# INPUT:
#        LVID LVNAME TYPE
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
#
#
is_lvid_in_odm()
{
    TLVID=$1	# temporary values
    TLVNAME=$2
    TTYPE=$3
    getlvodm -e $TLVID >/dev/null 2>&1 # is LVID defined under a different name?
    rc=$?
    if [ $rc -eq 3 ]
    then                                # no, it is not in odm
	if test "$TTYPE"            # is TYPE not null
	then
	    putlvodm -l $TLVNAME -t $TTYPE $TLVID    # define the new lv in odm
	    nonfatal_error $?
	else
	    putlvodm -l $TLVNAME $TLVID              # define the new lv in odm
	    nonfatal_error $?
	fi
    elif [ $rc -eq 0 ]      # lv is defined in odm under a different name
    then
		putlvodm -n $TLVNAME $TLVID   # update odm with the new lvname
		nonfatal_error $?
    else                    # ODM access error
		dspmsg -s 1 cmdlvm.cat 522 "`lvmmsg 522`\n" $PROG $LVNAME >& 2
		exit
    fi
}

######################### make_lv_dev_entries #########################
#
# NAME: make_lv_dev_entries()
#
# DESCRIPTION: Creates the special files for the lvname
#
# INPUT:
#        LVNAME MAJOR MINOR
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
#
#
make_lv_dev_entries()
{
    TLVNAME=$1		# set temporary values
    TMAJOR=$2
    TMINOR=$3

    mknod /dev/r"$TLVNAME" c $TMAJOR $TMINOR  #raw i/o device: prefixed with r
    fatal_error $?
    mknod /dev/"$TLVNAME" b $TMAJOR $TMINOR
    fatal_error $?
}


######################### validate_data ####################################
#
# NAME: validate_data()
#
# DESCRIPTION: Checks validity of input values.
#              Set to default value if input value is not valid.
#
# INPUT:
#        variables
#
# OUTPUT:
#        None
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
#
#
validate_data()
{

if [ "$INTRA" != e  -a  "$INTRA" != m  -a  "$INTRA" != c ]
  then             #Invalid input value for intra-policy option
     INTRA=m
  fi

if [ "$INTER" != m  -a  "$INTER" != x ]
  then             #Invalid input value for inter-policy option
     INTER=x
  fi

if [ "$COPIES" -lt 1  -o  "$COPIES" -gt 3 ]
  then             #Invalid input value for copy option
     COPIES=1
  fi

if [ "$RELOC" != y  -a  "$RELOC" != n ]
  then            #Invalid input value for relocatable value
     RELOC=n      #the default value is no here because we do not want
  fi              #the boot record to get relocated during reorg

if [ "$STRICT" != y  -a  "$STRICT" != n ]
  then             #Invalid input value for strict option
     STRICT=y
  fi

if [ "$UPPER" -lt 1  -o  "$UPPER" -gt 32 ]
  then             #Invalid input value for upperbound option
     UPPER=32
  fi

}

############################## main ########################################
#  Update Database for Logical Volume in the Volume Group
#
#  Input:
#       Command line options and arguments: updatelv lvname vgname
#
#  Output:
#       Error Messages (Standard error)
#


# Requirement: VG must be already varied on.


hash fatal_error nonfatal_error getlvodm getlvcb mknod lquerylv putlvcb putlvodm
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
		      #exiting only if updatelv completes successfully.
DMADENODE=0

OLD_UMASK=`umask`	# save old umask value
umask 117		# set umask for rw-rw---- 

#
# Trap on exit/interrupt/break to clean up
#

trap 'cleanup' 0 1 2 15

PROG=`basename $0`      # just get the basename of the command

ODMDELETE=/usr/bin/odmdelete

#
# Parse command line arguments
#

if test $# -gt 0  #if lvname argument on command line
then
   LVNAME=$1
   shift
else
   dspmsg -s 1 cmdlvm.cat 524 "`lvmmsg 524`\n" $PROG >& 2
   exit
fi

if test $# -ne 0         #check for vgname on command line
then                     #vgname is specified on the command line.
   VGNAME=$1
else
   dspmsg -s 1 cmdlvm.cat 526 "`lvmmsg 526`\n" $PROG >& 2
   dspmsg -s 1 cmdlvm.cat 524 "`lvmmsg 524`\n" $PROG >& 2
   exit
fi

#get the VGID from odm, updatevg should have updated the information already

VGID=`getlvodm -v $VGNAME`
fatal_error $?          # odm access error, exit

lqueryvg -g $VGID -L > /tmp/lvlist$$    #get the list of logical volumes
fatal_error $?          # lvm error, exit
			# else get the lvid of the corresponding lvname
if [ ! -f /bin/fgrep ] # create fgrep link, need it for accuracy below
then
	ln /bin/grep /bin/fgrep 2>&1
fi

LVID=`fgrep " $LVNAME " /tmp/lvlist$$ | cut -d" " -f1`
if test -z "$LVID"
then
    dspmsg -s 1 cmdlvm.cat 532 "`lvmmsg 532`\n" $PROG $LVNAME $VGNAME >& 2
    exit
fi

MAJOR=`getlvodm -d $VGID`       # get the major number
fatal_error $?                  # odm access error, exit


# Determine whether the logical volume is defined in odm

DMLVID=`getlvodm -l $LVNAME 2>/dev/null` # is LVNAME defined in ODM?
rc1=$?

if [ $rc1 -eq 2 ]                 # odm access error
then
   dspmsg -s 1 cmdlvm.cat 528 "`lvmmsg 528`\n" $PROG >& 2
   exit
elif [ $rc1 -eq 3 ]         # object not found
then     			
    if [ -s $ODMDELETE ]
    then
       odmdelete -q name="$LVNAME" -o CuAt >/dev/null
       odmdelete -q name="$LVNAME" -o CuDv >/dev/null
       odmdelete -q dependency="$LVNAME" -o CuDep >/dev/null
    fi
    is_lvid_in_odm $LVID $LVNAME

# lvname exists in odm but lvid is blank, odm is inconsistent, delete
# the entry from odm and create a new one for it.

elif [ $rc1 -eq 0 -a  -z "$DMLVID" ]
then
    if [ -s $ODMDELETE ]
    then
       odmdelete -q name="$LVNAME" -o CuAt >/dev/null
       odmdelete -q name="$LVNAME" -o CuDv >/dev/null
       odmdelete -q dependency="$LVNAME" -o CuDep >/dev/null
    fi
    is_lvid_in_odm $LVID $LVNAME

# The lvname is found in odm but the lvids do not match, i.e the volume group
# already has a lv with the same name then get a new lvname, change the lvname
# in LVM and in data base, then print out message
# Else if same lvname, same lvid then do nothing

elif [ $rc1 -eq 0 -a "$DMLVID" != "$LVID" ] #lvname exists but lvids not matched
then

    #pass a dummy lvname to create the /dev entry to access the lvcb
    MINOR=`echo $LVID | cut -d. -f2`     # get minor number from lvid
    make_lv_dev_entries dummylv$$ $MAJOR $MINOR
    DMADENODE=1

    TYPE=`getlvcb -t dummylv$$`       #get type characteristic out of lvcb
    if [ $? -ne 0 ]
    then
	TYPE="?"                 # default value of type
    fi

    rm -f /dev/rdummylv$$ /dev/dummylv$$  # remove the dummy /dev entries

    # get a new lvname ,if it exists in the volume group then get a new one.
    while true
    do
       NLVNAME=`getlvname $TYPE`           # get new lvname
       fatal_error $?
       if  fgrep " $NLVNAME " /tmp/lvlist$$ > /dev/null
       then
           RESERVED="$RESERVED $NLVNAME"
           putlvodm -l $NLVNAME ${VGID}.999  #add a bogus LV to reserve the name
           continue                          #so a new name can be generated
       else
	   break
       fi
    done

    #create the /dev entries for the new lvname to be able to change name in LVM
    make_lv_dev_entries $NLVNAME $MAJOR $MINOR

    lchangelv -l $LVID -n $NLVNAME  # change lvname in LVM
    fatal_error $?

    # check whether corresponding LVID is defined in odm, update or create new
    # entry in odm as necessary
    is_lvid_in_odm $LVID $NLVNAME $TYPE
    dspmsg -s 1 cmdlvm.cat 530 "`lvmmsg 530`\n" $PROG $LVNAME $NLVNAME >& 2
    LVNAME=$NLVNAME
fi

MINOR=`echo $LVID | cut -d. -f2`        # get minor number from lvid
lvrelminor $LVNAME 2>/dev/null          # release the minor number, which
                                        # also remove the existing /dev entries
NMINOR=`lvgenminor -p $MINOR $MAJOR $LVNAME`     # get the preferred minor back
fatal_error $?

# make sure that the /dev entries do not exist (lvrelminor might not be able
# to delete the /dev entries because the object classes are not there) otherwise
# the followed mknod call will fail.
rm -f /dev/"$LVNAME" /dev/r"$LVNAME"

make_lv_dev_entries $LVNAME $MAJOR $NMINOR

SIZE=`lquerylv -c -L $LVID`     # get the lv size
if [ $? -ne 0 ]
then
     SIZE=1			# if it failed set the default size to 1
fi

SCHED=`lquerylv -M -L $LVID`	# get the schedule policy (5 = striping)

# get all characteristics from the control block, if error (either read error
# or control block was not initialized because mklv failed while trying
# to do it) then get data from odm.

getlvcb -aceLrsSPtu $LVNAME > /tmp/lvcb$$   # get characteristics from lvcb
if [ $? -eq 0 -a `wc -w < /tmp/lvcb$$` -eq 10 ]
then
    read INTRA COPIES INTER LABEL RELOC STRICT STRIPING_WIDTH STRIPE_EXPN TYPE UPPER < /tmp/lvcb$$
    FS=`getlvcb -f $LVNAME`
    if [ $? -ne 0 ]
    then
       FS=
    fi

    # invalid data found exponent or striping width is not within
    # valid range 12-17 and 2-32 respectively, if not schedule policy 5 (stripe)
    # then also set the striping parameters to zero also
    # schedule policy must be set to 5.
    if [ $SCHED -ne 5 ]
    then
	STRIPE_BLK_SIZE=0
	STRIPE_EXPN=0
	STRIPING_WIDTH=0
    else

    	# map exponent to block size
    	case $STRIPE_EXPN in
		0) STRIPE_BLK_SIZE=0
           	   STRIPING_WIDTH=0;;	# no striping
		12) STRIPE_BLK_SIZE=4K;;
		13) STRIPE_BLK_SIZE=8K;;
		14) STRIPE_BLK_SIZE=16K;;
		15) STRIPE_BLK_SIZE=32K;;
		16) STRIPE_BLK_SIZE=64K;;
		17) STRIPE_BLK_SIZE=128K;;
    	esac
    fi

else
    LABEL=`getlvodm -B $LVID`
    getlvodm -c $LVID > /tmp/odmdata$$   #get lv characteristics from odm
    if [ $? -eq 0 ]
    then
	read TYPE INTRA INTER UPPER STRICT COPIES RELOC STRIPING_WIDTH STRIPE_BLK_SIZE < /tmp/odmdata$$

    	# map block size to exponent
    	if [ $SCHED -eq 5 -a -n "$STRIPING_WIDTH" ]
    	then
    	    case $STRIPE_BLK_SIZE in
		4K | 4k) STRIPE_EXPN=12;;
		8K | 8k) STRIPE_EXPN=13;;
		16K | 16k) STRIPE_EXPN=14;;
		32K | 32k) STRIPE_EXPN=15;;
		64K | 64k) STRIPE_EXPN=16;;
		128K | 128k) STRIPE_EXPN=17;;
    	    esac

    	    # invalid data found exponent or striping width is not within
    	    # valid range 12-17 and 2-32 respectively
	    if [ $STRIPE_EXPN -lt 12 -o $STRIPE_EXPN -gt 17 -o \
	 	 $STRIPING_WIDTH -lt 2 -o $STRIPING_WIDTH -gt 32 ]
	    then
		STRIPE_BLK_SIZE=0
		STRIPE_EXPN=0
		STRIPING_WIDTH=0
	    fi
	else
	    STRIPE_EXPN=0
	    STRIPING_WIDTH=0
	    STRIPE_BLK_SIZE=0
	fi
    fi
fi

# Validate the data just got, if no good then use the defaults.
# Write data to both control block and odm.

validate_data

PERMISSION=`lquerylv -L $LVID -P`
if [ "$PERMISSION" -eq 1 ]         # read-write permission ?
then

    # the striping parameters can always be written to the LVCB, since
    # the new values were in the padding, and this will insure the data
    # to be zero, and not some garbage value when getlvcb returns it.

    putlvcb -a $INTRA -c $COPIES -e $INTER -i $LVID -n $SIZE -r $RELOC \
	-L $LABEL -s $STRICT -t $TYPE -u $UPPER \
	-S $STRIPE_EXPN -O $STRIPING_WIDTH \
	$LVNAME

    if [ $? != 0 ]      #check for error return from putlvcb
    then
        dspmsg -s 1 cmdlvm.cat 622 "`lvmmsg 622`\n" updatelv >& 2
    fi

    if [ -n "$FS" ]
    then
        putlvcb -f "$FS" $LVNAME
        if [ $? != 0 ]      #check for error return from putlvcb
        then
            dspmsg -s 1 cmdlvm.cat 622 "`lvmmsg 622`\n" updatelv >& 2
        fi
    fi
fi

# if striping, then put the STRIPE_BLK_SIZE and STRIPING_WIDTH into
# ODM, otherwise ignore those values.

if [ $SCHED -eq 5 ]
then
	putlvodm -a $INTRA -c $COPIES -e $INTER -r $RELOC -s $STRICT \
		-B $LABEL -t $TYPE  -u $UPPER -z $SIZE \
		-S $STRIPE_BLK_SIZE -O $STRIPING_WIDTH \
		$LVID 2>/dev/null
else
	putlvodm -a $INTRA -c $COPIES -e $INTER -r $RELOC -s $STRICT \
		-B $LABEL -t $TYPE  -u $UPPER -z $SIZE \
		$LVID 2>/dev/null
fi
nonfatal_error $?

if [ "$EXIT_CODE" -ne 2 ]
then
   EXIT_CODE=0          # reset exit code to indicate successful completion
fi

exit                    #trap will handle cleanup.
