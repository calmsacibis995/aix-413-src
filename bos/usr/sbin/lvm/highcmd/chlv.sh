#! /bin/ksh
# @(#)74	1.18.2.15  src/bos/usr/sbin/lvm/highcmd/chlv.sh, cmdlvm, bos41J, 9518A_all 4/25/95 16:15:35
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: chlv.sh
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
#                   the characteristics of a logical volume.
#
#
# RETURN VALUE DESCRIPTION: 
#                             0     Successful
#                             1     Unsuccessful
#                             2     Partially successful
#                                   (at least one LV)
#
# EXTERNAL PROCEDURES CALLED: getlvodm, lchangelv, lquerylv, mv, putlvodm
#
#
# GLOBAL VARIABLES: none
#

######################### check_input #########################################
#
# NAME: check_input()
#
# DESCRIPTION: Checks validity of input values.  Will output error message
#              and exit if input is not valid.
#
# INPUT:
#        $1 : Option flag  (ie. -a for intra-policy option)
#        $2 : Option value (ie. e for edge intra-policy value)
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

   if [ "$1" -ne '-n' ]
   then       # -n option must be used alone
      if [ -n "$RENAME" ]
      then
      		dspmsg -s 1 cmdlvm.cat 706 "`lvmmsg 706`\n" chlv >& 2
      		exit
      fi
   else
      OTHER='y'
   fi

   case "$1" in
      
      -a) if [ "$2" != e  -a  "$2" != m  -a  "$2" != c  -a "$2" != "ie" -a "$2" != "im" ]
	  then    #Invalid input value specified for intra-policy option
	     dspmsg -s 1 cmdlvm.cat 652 "`lvmmsg 652`\n" chlv >& 2
             exit 
          fi
	  ODM='y'
	  shift;  shift  ;;    #shift past this option flag/value pair

      -e) if [ "$2" != m  -a  "$2" != x ]
	  then    #Invalid input value specfied for inter-policy option
	     dspmsg -s 1 cmdlvm.cat 654 "`lvmmsg 654`\n" chlv >& 2
	     exit
          fi
	  ODM='y'
          shift;  shift ;;     #shift past this option flag/value pair

      -u) if [ "$2" -lt 1  -o  "$2" -gt 32 ]
	  then    #Invalid input value specified for upperbound option
	     dspmsg -s 1 cmdlvm.cat 656 "`lvmmsg 656`\n" chlv >& 2
             exit 
          fi
	  ODM='y'
          shift;  shift  ;;    #shift past this option flag/value pair

      -s) if [ "$2" != y  -a  "$2" != n ]
	  then    #Invalid input value specified for strict option
	     dspmsg -s 1 cmdlvm.cat 658 "`lvmmsg 658`\n" chlv >& 2
             exit 
          fi
	  ODM='y'
          shift;  shift ;;     #shift past this option flag/value pair

      -b) if [ "$2" != y  -a  "$2" != n ]
	  then    #bad block relocation not specified as y or n
	     dspmsg -s 1 cmdlvm.cat 660 "`lvmmsg 660`\n" chlv >& 2
             exit 
          fi
	  LVM='y'
          shift;  shift ;;    #shift past this option flag/value pair

      -M) if [ "$2" != s  -a  "$2"  != p ]
	  then    #Invalid input specified for scheduling policy
	     dspmsg -s 1 cmdlvm.cat 664 "`lvmmsg 664`\n" chlv >& 2
             exit 
          fi
	  LVM='y'
          shift;  shift ;;   #shift past this option flag/value pair

      -p) if [ "$2" != r  -a  "$2" != w ]
	  then    #Invalid input specified for access permission value
	     dspmsg -s 1 cmdlvm.cat 666 "`lvmmsg 666`\n" chlv >& 2
             exit 
          fi
	  LVM='y'
          shift;  shift ;;   #shift past this option flag/value pair        

      -x) if [ "$2" -lt 1  -o  "$2" -gt 32512 ]
	  #val of 32512 derived from 1016 lps/disk * 32 disks/vg
	  then    #Invalid input specified for maxlps
	     dspmsg -s 1 cmdlvm.cat 668 "`lvmmsg 668`\n" chlv >& 2
             exit 
          fi
	  LVM='y'
          shift;  shift ;;   #shift past this option flag/value pair 

      -v) if [ "$2" != y  -a  "$2" != n ]
	  then    #Invalid input specified for write-verify value
	     dspmsg -s 1 cmdlvm.cat 676 "`lvmmsg 676`\n" chlv >& 2
             exit 
          fi
	  LVM='y'
          shift;  shift ;;   #shift past this option flag/value pair

      -r) if [ "$2" != y  -a  "$2" != n ]
	  then    #Invalid input specified for relocatable value
	     dspmsg -s 1 cmdlvm.cat 672 "`lvmmsg 672`\n" chlv >& 2
	     exit
	  fi
	  ODM='y'
	  shift;  shift ;;   #shift past this option flag/value pair

      -n) 
      	  if [ -n "$OTHER" ]
          then
      		dspmsg -s 1 cmdlvm.cat 706 "`lvmmsg 706`\n" chlv >& 2
      		exit
          fi
          RENAME='y'
          ODM='y'
	  shift;  shift ;;   #shift past this option flag/value pair

      -t) 
           if [ "$2" = 'paging' ]
           then
             if [ "$WVAL" = 'y' ] 
             then   #Invalid input specified for mirror-write consistency value
             dspmsg -s 1 cmdlvm.cat 1005 "`lvmmsg 1005`\n" chlv >& 2
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
   		dspmsg -s 1 cmdlvm.cat 693 "`lvmmsg 693`\n" chlv "t">& 2
                exit
         fi
          shift;  shift ;;   

      -L) # We don't allow labels 128 characters or greater.
          # wc -m also counts the null character in the string, so we are
          # looking for a return count of 129 characters or larger
          CHAR_COUNT=`echo $LVAL | wc -m`
          if [ "$CHAR_COUNT" -gt 128 ]
          then
   		dspmsg -s 1 cmdlvm.cat 693 "`lvmmsg 693`\n" chlv "L">& 2
                exit
          fi

          shift;  shift ;;   #shift past this option flag/value pair


      -w) if [ "$2" != y  -a  "$2" != n ]
          then    #Invalid input specified for mirror-write consistency value
	         dspmsg -s 1 cmdlvm.cat 1004 "`lvmmsg 1004`\n" chlv >& 2
             exit
          fi
	      LVM='y'
          shift;  shift ;;   #shift past this option flag/value pair

   esac

done   #end - while there are still arguments to process
}

########################### errhandler #######################################
#
# NAME: errhandler()
#
# DESCRIPTION: Displays fatal error message and sets up exit status for
#              error return from chlv.
#
# INPUT: 
#        Logical volume name
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
   dspmsg -s 1 cmdlvm.cat 704 "`lvmmsg 704`\n" chlv $name >& 2
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
cleanup()
{
   trap '' 0 1 2 15
   #Unlock volume group if it was locked in this command
   if test $LOCKVG -eq 1
	then                               
		putlvodm -K $VGID
   fi

   savebase > /dev/null 	#save the database to the ipl_device

   umask $OLD_UMASK	# restore umask 

   exit $EXIT_CODE 
}

############################## main ############################################
#  Change logical volume
#  Input:
#       Command line options and arguments:  	
#       chlv [-a] [-e] [-u] [-s] [-b] [-d] [-L]
#            [-p] [-r] [-t] [-v] [-w] [-x] lvname ...
#   OR  chlv -n NewLVName LVName
#  Output:
#       Error Messages (Standard error)	
#
hash errhandler putlvodm getlvodm mv dspmsg
EXIT_CODE=1
LOCKVG=0

OLD_UMASK=`umask`	# save old umask value
umask 117		# set umask for rw-rw---- 

#
# Trap on exit/interrupt/break to clean up 
#

trap 'cleanup' 0 1 2 15

#
# Parse command line options

set -- `getopt n:a:e:u:s:b:d:p:r:t:v:w:x:L: $*`
		
if [ $? != 0 ]                      # Determine if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 1003 "`lvmmsg 1003`\n" chlv chlv >& 2
   exit  
fi

AFLAG= ; EFLAG= ; UFLAG= ; SFLAG= ; BFLAG= ; DFLAG= ;
PFLAG= ; RFLAG= ; TFLAG= ; LFLAG= ; VFLAG= ; XFLAG= ;
NFLAG= ; WFLAG= ; 

AVAL= ; EVAL= ; UVAL= ; SVAL= ; BVAL= ; DVAL= ;
PVAL= ; RVAL= ; TVAL= ; LVAL= ; VVAL= ; XVAL= ;
NVAL= ; WVAL= ; 

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
           -a) AFLAG='-a';  AVAL=$2;  shift;  shift;;   #intrapolicy
           -e) EFLAG='-e';  EVAL=$2;  shift;  shift;;   #interpolicy
           -u) UFLAG='-u';  UVAL=$2;  shift;  shift;;   #upperbound
           -s) SFLAG='-s';  SVAL=$2;  shift;  shift;;   #strict policy
    	   -b) BFLAG='-b';  BVAL=$2;  shift;  shift;;   #bad block relocation
	   -d) DFLAG='-M';  DVAL=$2;  shift;  shift;;   #scheduling policy
           -p) PFLAG='-p';  PVAL=$2;  shift;  shift;;   #access permission
           -r) RFLAG='-r';  RVAL=$2;  shift;  shift;;   #relocatable 
           -t) TFLAG='-t';  TVAL=$2;  ODM='y'; shift; shift;; # lvtype   
           -L) LFLAG='-L';  LFLAG2='-B'; LVAL=$2;  ODM='y'; shift;  shift;; #lvlabel
           -v) VFLAG='-v';  VVAL=$2;  shift;  shift;;   #write/verify 
           -w) WFLAG='-w';  WVAL=$2;  shift;  shift;;   #mirrorwrite consistency
	       -x) XFLAG='-x';  XVAL=$2;  shift;  shift;;   #max numlps
	       -n) NFLAG='-n';  NVAL=$2;  shift;  shift;;   #NewLVName
  esac
done   #end - while there is a command line option

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

if [ -n "$1" ]  #if lvdescript argument on command line
then
      lvdescript=$*         #save lvdescripts in string var lvdescript
else  
   dspmsg -s 1 cmdlvm.cat 602 "`lvmmsg 602`\n" chlv >& 2   # lvname required
   dspmsg -s 1 cmdlvm.cat 1003 "`lvmmsg 1003`\n" chlv chlv >& 2
   exit 
fi

#
# Check for invalid input
#

check_input $AFLAG $AVAL $EFLAG $EVAL $UFLAG $UVAL $SFLAG $SVAL $BFLAG \
$BVAL $DFLAG $DVAL $PFLAG $PVAL $XFLAG $XVAL $VFLAG $VVAL \
$RFLAG $RVAL $NFLAG $NVAL $WFLAG $WVAL $TFLAG $TVAL $LFLAG $LVAL

# check for illegal combination
if [ "$WVAL" =  'y' -a  \( "$TVAL" = 'paging' \) ]
then
   dspmsg -s 1 cmdlvm.cat 1005 "`lvmmsg 1005`\n" chlv >& 2
   exit 
fi

#
# Convert option values for following call to lchangelv
#

if [ -n "$BFLAG" ]      #Bad block relocation option specified?
then
   BFLAG=-r
   if [ "$BVAL" = y ]   #Bad block relocation specified as yes?
   then                 #Set BVAL to 1 to indicate bad block
      BVAL=1            #relocation for lchangelv.
   else
      BVAL=2            #Bad block relocation specified as no. Convert
   fi                   #BVAL from n(o) to 2 for lchangelv.
fi

if [ -n "$DFLAG" ]      #Scheduling policy option specified?
then
   if [ "$DVAL" = s ]   #Sequential scheduling policy specified?
   then                 #Set DVAL to 1 to indicate sequential
      DVAL=1            #scheduling policy for lchangelv.
   else                 #Parallel scheduling policy specified.
      DVAL=2            #Set DVAL to 2 for lchangelv.
   fi
fi
           
if [ -n "$PFLAG" ]      #Access permission option specified?
then
   if [ "$PVAL" = r ]   #Read permission specified?
   then                 #Set PVAL to 2 to indicate read permission
      PVAL=2            #for lchangelv.
   else                 #Read/write permission specified.
      PVAL=1            #Set PVAL to 1 to indicate read/write permission
   fi                   #for lchangelv.
fi

if [ -n "$VFLAG" ]      #Write/verify state option specified?
then                   
   if [ "$VVAL" = y ]   #Write/verify specified as yes - set to 1
   then                 #for lchangelv.
      VVAL=1    
   else                 #Write/verify specified as no - set to 2 for
      VVAL=2            #lchangelv.
   fi
fi

if [ -n "$WFLAG" ]      #MirrorWrite consistency option specified?
then
   if [ "$WVAL" = y ]   #MirrorWrite consistency specified as yes - set to 1
   then                 #for lchangelv.
      WVAL=1
   else                 #MirrorWrite consistency specified as no - set to 2 for
      WVAL=2            #lchangelv.
   fi
fi

if [ -n "$XFLAG" ]
then
   XFLAG=-s
fi


#
# Loop through lvdescripts updating LV attributes.
# lvdescript(s) specified on command line -- call getlvodm
# to get the lvid for each lvdescript.
#

for name in $lvdescript
do
   LVID=`getlvodm -l $name`   #get the lvid
   if [ $? != 0 ]             #check for error return from getlvodm
   then
      errhandler $name
      continue
   fi

   # Check if it is a striped logical volume, if so, do not accept
   # allow the change to relocatable logical volume (reorgvg)

   STRIPE_SIZE=`lquerylv -L $LVID -b`
   if [ $? != 0 ]             #check for error return from lquerylv
   then
      errhandler $name
      continue
   fi


	DISK_COUNT=`lquerylv -L $LVID -d | cut -f1 -d: |sort | uniq | wc -l`
   	if [ -n "$UFLAG" -a "$DISK_COUNT" -gt "$UVAL" ]
   	then
		echo "Cannot change -u value.  New value violates current lv paramters"
		dspmsg -s 1 cmdlvm.cat 704 "`lvmmsg 704`\n" chlv $name >& 2
		exit
   	fi

   # if striping then we check things like, are you trying to change
   # it to relocatable, or change the schedule, or change the type to
   # boot, all which are invalid.
   #
   if [ $STRIPE_SIZE != 0 ]
   then
	if [ "$RVAL" = "y" ]
   	then
      		dspmsg -s 1 cmdlvm.cat 1042 "`lvmmsg 1042`\n" chlv >& 2
		dspmsg -s 1 cmdlvm.cat 704 "`lvmmsg 704`\n" chlv $name >& 2
		exit
   	fi

   	if [ -n "$DFLAG" -o -n "$EFLAG" -o -n "$UFLAG" -o -n "$SFLAG" -o \
	     -n "$WFLAG" ]
   	then
      		dspmsg -s 1 cmdlvm.cat 1046 "`lvmmsg 1046`\n" chlv >& 2
		dspmsg -s 1 cmdlvm.cat 704 "`lvmmsg 704`\n" chlv $name >& 2
		exit
   	fi

   	if [ "$TVAL" = 'boot' ]
   	then
      		# can't change a striped lv to a boot logical volume
      		dspmsg -s 1 cmdlvm.cat 1049 "`lvmmsg 1049`\n" chlv >& 2
		dspmsg -s 1 cmdlvm.cat 704 "`lvmmsg 704`\n" chlv $name >& 2
		exit
   	fi
   fi

   # no mirror write consistency specifiied for paging or log file systems
   # then set mirrorwrite to no
   if [ -z "$WFLAG"  -a  \( "$TVAL" = 'paging' \) ]
   then                                              
      # check mirror write consistency status,
      # if mirror write consistency is on, set it off.
      # on = 1, off = 2
      MIRROR_ON_OFF=`lquerylv -L "$LVID" -w`
      if [ "$MIRROR_ON_OFF" = 1 ]
      then
         WFLAG=-w
         WVAL=2
	 LVM='y'
      fi
   fi

   if [ -n "$ODM" -a `lquerylv -L "$LVID" -P` -eq 2 ] # do not change the odm
   then                                               # or the lv control block
      errhandler $name                                # if the permission is 
      continue                                        # readonly
   fi
   
   if [ -n "$XFLAG" ]
   then
        # get the current number of lps
   	    N_OF_LPS=`lquerylv -L "$LVID" -c`
        if [ "$N_OF_LPS" -gt "$XVAL" ]
        then
	         dspmsg -s 1 cmdlvm.cat 668 "`lvmmsg 668`\n" chlv >& 2
      	     continue
        fi
   fi
   
   VGID=`echo "$LVID"|cut -f1 -d"."`  #Extract VGID from LVID

   putlvodm -k $VGID             #lock the volume group
   if [ $? != 0 ]                #Check for error return from putlvodm
      	then                     #Error return - nonfatal error
 		errhandler $name #Display error msg and set $EXIT_CODE to 2
        	continue         #Continue with next lvdescript in list
   fi
   LOCKVG=1

   MAJOR=`getlvodm -d $VGID`          #Retrieve major number
   if [ $? != 0 ]             #check for error return from getlvodm
   then
      errhandler $name
      continue
   fi

   MINOR=`echo "$LVID"|cut -f2 -d"."` #Extract minor number from LVID

   OPEN_CLOSE=`lquerylv -L $LVID -o`  #Determine logical volume open state
   if [ $? != 0 ]  #test for error return from lquerylv
   then
      errhandler $name
      continue
   fi

   if [ -n "$TFLAG" -a "$OPEN_CLOSE" != 2 ] #if logical volume in open state
   then                                     #and action is to change type
      errhandler $name                      #return error
      continue
   fi

   LVNAME=`getlvodm -e $LVID`   #get the lvname
   if [ $? != 0 ]             #check return from getlvodm
   then
      errhandler $name
      exit
   fi

   # no type change input from user but mirrorwrite consistency is yes then
   # need to check for the current file system type of the logical volume.
   if [ "$WVAL" -eq 1  -a  -z  "$TFLAG" ]
   then
      TYPE=`getlvodm -y $LVID`
      if [ $? != 0 ]             #check return from getlvodm
      then
		 errhandler $name
         exit
	  elif [ "$TYPE" = 'paging' ]
	  then
         dspmsg -s 1 cmdlvm.cat 1005 "`lvmmsg 1005`\n" chlv >& 2
         exit 
      fi
   fi   


#
#  If not rename case, change the logical volume attributes in LVM.
#
   if [ "$NFLAG" != -n ]
   then
      if [ -n "$LVM" ]           #If LVM flag is not null, update
      then
  	 lchangelv -l $LVID $WFLAG $WVAL $DFLAG $DVAL $XFLAG $XVAL \
	 $PFLAG $PVAL $BFLAG $BVAL $VFLAG $VVAL
	 if [ $? != 0 ]  #test for error return from lchangelv
	 then
	    errhandler $name
	    continue
	 fi
      fi
#
#  Update ODM with new logical volume data.  Any options left null
#  will not be changed by putlvodm or putlvcb.
#
      if [ -n "$ODM" ]            #If ODM flag is not null, update
      then
	     putlvodm $AFLAG $AVAL $EFLAG $EVAL $NFLAG $NVAL \
	     $RFLAG $RVAL $SFLAG $SVAL $TFLAG $TVAL $UFLAG $UVAL \
         $LFLAG2 $LVAL $LVID
	 if [ $? != 0 ]          #check for error return from putlvodm
	 then
	    dspmsg -s 1 cmdlvm.cat 1023 "`lvmmsg 1023`\n" chlv >& 2
	 else
	     putlvcb $AFLAG $AVAL $EFLAG $EVAL -i $LVID \
	     $RFLAG $RVAL $SFLAG $SVAL $TFLAG $TVAL $UFLAG $UVAL \
         $LFLAG $LVAL $LVNAME
	 if [ $? != 0 ]          #check for error return from putlvcb
	 then
	    dspmsg -s 1 cmdlvm.cat 711 "`lvmmsg 711`\n" chlv >& 2
	    continue
	 fi
	 fi
      fi
#
#  Rename case, change the logical volume name in the device configuration
#  database, make the devices with the new names, and update the logical
#  volume control block.
#
   else
      if [ "$OPEN_CLOSE" = 2 ]   #If the logical volume is closed, change
      then
         if [ -f /dev/r$NVAL -o -f /dev/$NVAL ]
         then #picked an already existing name, error, leave chlv!
            dspmsg -s 1 cmdlvm.cat 704 "`lvmmsg 704`\n" chlv $name >& 2
            NVAL=`getlvname -n hd5` #doing this just to gen right error msg
            exit
         fi
	 NVAL=`getlvname -n $NVAL`
	 if [ $? != 0 ]          #check for error return from getlvname
	 then
	    dspmsg -s 1 cmdlvm.cat 704 "`lvmmsg 704`\n" chlv $name >& 2
	    exit 
	 fi
	 putlvodm $NFLAG $NVAL $LVID
	 if [ $? != 0 ]          #check for error return from putlvodm
	 then
	    dspmsg -s 1 cmdlvm.cat 1023 "`lvmmsg 1023`\n" chlv >& 2
	 fi
	 mknod /dev/$NVAL b $MAJOR $MINOR
	 if [ $? != 0 ]          #check for error return from mknod
	 then
	    dspmsg -s 1 cmdlvm.cat 710 "`lvmmsg 710`\n" chlv /dev/$NVAL >& 2
	    exit 
	 fi
	 mknod /dev/r$NVAL c $MAJOR $MINOR
	 if [ $? != 0 ]          #check for error return from mknod
	 then
	    rm -f /dev/$NVAL
	    dspmsg -s 1 cmdlvm.cat 710 "`lvmmsg 710`\n" chlv /dev/r$NVAL >& 2
	    exit 
	 fi
	 lchangelv -l $LVID $NFLAG $NVAL
	 if [ $? != 0 ]  #test for error return from lchangelv
	 then
	    rm -f /dev/$NVAL /dev/r$NVAL
	    errhandler $name     #If the lchangelv fails, error
	    exit
	 fi
	  putlvcb -i $LVID $NVAL
	  if [ $? != 0 ]          #check for error return from putlvcb
	  then
	       dspmsg -s 1 cmdlvm.cat 711 "`lvmmsg 711`\n" chlv >& 2
	       continue
	  fi
	 #
	 # unlock volume group BEFORE we call chfs (because chfs
	 # may call someone who wants to lock the vg)...
	 #
	 if test $LOCKVG -eq 1
	 then
	      putlvodm -K $VGID
	      LOCKVG=0
	 fi
	# user must manually go through and update /etc/filesystems
	# with the change to the log name.  No simple way to do this.
         TYPE=`getlvodm -y $LVID`
	 if [ "$TYPE" = 'jfslog' ]
	 then
	    dspmsg -s 1 cmdlvm.cat 712 "`lvmmsg 712`\n" chlv $LVNAME >& 2
	 fi
	 #
	 # ask chfs to change /etc/filesystems.  however, since
	 # this may or may not be a filesystem (and it's tough
	 # to tell here), ignore any errors
	 #
	 chfs -a dev=/dev/$NVAL /dev/$LVNAME > /dev/null 2>&1
      else                       #If the logical volume is open, error
	 dspmsg -s 1 cmdlvm.cat 708 "`lvmmsg 708`\n" chlv $LVNAME >& 2
	 exit
      fi
   fi
   if test $LOCKVG -eq 1
   	then                               # unlock volume group if it was locked
	      putlvodm -K $VGID
          LOCKVG=0
   fi
done     #end - for each lvdescript

if [ "$EXIT_CODE" != 2 ]
then
   EXIT_CODE=0     #Reset exit code to indicate successful completion of chlv
fi
exit                 #trap will handle cleanup.
