#!/bin/ksh
# @(#)30        1.51  src/bos/usr/sbin/install/scripts/cleanup.sh, cmdinstl, bos412, 9445C412a 11/8/94 17:35:53
# COMPONENT_NAME: (CMDINSTL) command install
#
# FUNCTIONS: cleanup and reject (default scripts for installp)
#                                                                    
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1990, 1994
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                    
#
# NAME: cleanup and reject scripts
#                                                                    
# FUNCTION: 
#       if called as cleanup from installp then this script needs to 
#       cleanup an install or an update
#	if called as reject from installp then this script needs to 
#	reject an install or an update
#
# ARGUMENTS:
#       $1 is the Device that contains the image for the product.
#          This is not really valid since the distribution media may
#          no longer be in place.  This is here for backwards compatibility.
#       $2 is the full path to the file that contains the list of options
#          to cleanup/reject.
#
# RETURN VALUE DESCRIPTION:
#    0 if nothing fails
#    100 if one or more options fail
#
#	The cleanup script is invoked from installp with the first 
#	parameter being the device that is being used for installation or 
#	update, the second parameter is the full path to the file containing 
#	the list of options that need to be cleaned-up.
#
#	The reject script is invoked from installp with the first 
#	parameter being the device that is being used for installation or 
#	update, the second parameter is the full path to the file containing 
#	the list of options that need to be rejected.
#
#	This script uses the INUTREE, the INUTEMPDIR and INUROOT environment
#	variables.
#
#	This script looks for the following mandatory files 
#       ($OPTION is the full option name) ($LPPNAME is the product name):
#
#		$OPTION.al              The apply list
#		$OPTION.inventory       The inventory list
#               $LPPNAME.s              Status file used for cleanup;  only
#                                       mandatory if this script is called
#                                       as cleanup.  Referred to as 
#                                       $STATUSFILE.
#
#       The status file $LPPNAME.s has the following format for each line
#           $OPTION status
#       where
#           $OPTION      is the name of the option being installed/updated
#           status       is status to be used by the cleanup script
#       and status can be one of the following values.
#           Status    Meaning to cleanup script
#           ------    -------------------------
#           pre_u     $OPTION.pre_u script failed
#           pre_i     $OPTION.pre_i script failed
#           noapp     failed because the apply list was missing
#           save      inusave failed
#           cp        inucp failed
#           rest      inurest failed
#           noinv     no inventory file
#           sys       sysck failed
#           tcb       tcbck failed
#           post_u    $OPTION.post_u script failed
#           post_i    $OPTION.post_i script failed
#           err       errupdate failed
#           trc       trcupdate failed
#           errinstl  errinstall failued
#           odel      $OPTION.odmdel script failed
#           oadd      odmadd failed
#           cfg       $OPTION.config script failed
#
#	This script looks for the following optional files that 
#       could be supplied with the product package:
#
#               $OPTION.unpre_i      The product-supplied script for cleaning
#                                    up the pre-installation processing.
#                                    Must have execute permissions.
#               $OPTION.unpre_u      The product-supplied script for cleaning
#                                    up the pre-updating processing.
#                                    Must have execute permissions.
#               $OPTION.unpost_i     The product-supplied script for cleaning
#                                    up the post-installation processing.
#                                    Must have execute permissions.
#               $OPTION.unpost_u     The product-supplied script for cleaning
#                                    up the post-updating processing.
#                                    Must have execute permissions.
#               $OPTION.unodmadd     A script that deletes the odm entries
#                                    that were added during the install or
#                                    update.
#                                    Must have execute permissions.
#               $OPTION.unconfig     The product-supplied script for
#                                    unconfiguring the installation of
#                                    the product option.
#                                    Must have execute permissions.
#               $OPTION.unconfig_u   The product-supplied script for
#                                    unconfiguring the update of the
#                                    product option.
#                                    Must have execute permissions.
#
#       Files that could be created by the instal/update script:
#
#             $OPTION.undo.err       Cleans up the updates to the error 
#                                    record template repository.
#             $OPTION.undo.trc       Cleans up the updates to the trace 
#                                    report format templates.
#             $OPTION.rodmadd        An odmadd (stanza) file that puts the 
#                                    database back the way it was before the 
#                                    install or update.
#             $OPTION.codepoint.undo Removes codepoint entries.
#
#   The following environment variables are used by this script:
#    INUTREE    = M|U|S        
#		 M - root file system (formerly called miniroot)
#		 U - /usr file system
#		 S - /usr/share file system
#		 Tells this script which one of the file
#                systems are being cleaned up or rejected.
#		 No default fatal error if not set.
#    INUTEMPDIR  This is the location of the tmp directory
#                that this script is allowed to use.
#		 The default is /tmp.
#    INUROOT	 Tells this script the location of the root directory
#		 The default is /.
#
#-----------------------------------------------------------------------------

#-------------------------------
# local, fast version of dirname
#-------------------------------
dname()
{
        S=$1
        S=${S%%/*([!/])*(/)}    # handles arbitrary no of / at eol
        echo $S
}

#--------------------------------
# local, fast version of basename
#--------------------------------
bname()
{
        S=$1
        S=${S%%+(/)}    # remove rightmost /s if any
        S=${S##*/}      # remove path
        echo $S
}

#---------------------------------------------------------
# This function writes out the status that installp needs.
# This function takes two parameters
# $1 is the inustatus field
# $2 is the LPPOPTION
#---------------------------------------------------------
inustatusfile()
{
    [ $1 = f ]  &&  FAILED="true"
    echo "$1 $2" >> $INUTEMPDIR/status
}

#--------------------
# Runs the scripts $2
#--------------------
inu_run_script()
{
   if [ -x $2 ]; then
      $2
      if [ $? -ne 0 ]; then
         inuumsg $INU_SCRIPT_FAILED $ARGV0 $2
         inustatusfile f $1
         return 1
      fi
   fi
   return 0
}

#--------------------
# Runs the scripts $2
#--------------------
inu_run_odm_add()
{
   if [ -s $2 ]; then
      odmadd $2
      if [ $? -ne 0 ]; then
         inustatusfile f $1
         return 1
      fi
   fi
   return 0
}

#--------------------------------------------
# This function sets up all status flags to 0
#--------------------------------------------
zeroflags()
{
    MRGA=0; MRGD=0; MRGG=0; CFG=0; OADD=0; ODEL=0; TRC=0; ERR=0; ERRINSTL=0;
    TCB=0; SYS=0; RMFILES=0; UNPRE_I=0; UNPOST_I=0; UNPRE_U=0; UNPOST_U=0
    RECV=0; ODMUP=0
}

#---------------------------------------------------------------------------
# This function reads the status file that was created during the install or 
# update and figures out what needs to be fixed.  Only used if this script 
# was called as cleanup.  $1 is the LPPOPTION that is being cleaned up
#---------------------------------------------------------------------------
readstatus()
{
    zeroflags
    set `grep $1 $STATUSFILE`
    STATUS=$2    
    case $STATUS in
	pre_u)  UNPRE_U=1;;
	pre_i)  UNPRE_I=1;;
	noapp)
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1
		else
		    UNPRE_U=1
		fi;;
	save)  
                RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1
		else
		    UNPRE_U=1
		fi;;
	cp|rest|noinv) 
		RMFILES=1; RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1
		else
		    UNPRE_U=1
		fi;;
	post_u) UNPOST_U=1; RMFILES=1; UNPRE_U=1; RECV=1;;
	post_i) UNPOST_I=1; RMFILES=1; UNPRE_I=1; RECV=1;;
	sys)    SYS=1; RMFILES=1; RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1
		else
		    UNPRE_U=1
		fi;;
	tcb)    TCB=1; SYS=1; RMFILES=1; RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1
		else
		    UNPRE_U=1
		fi;;
	err)    ERR=1; TCB=1; SYS=1; RMFILES=1; RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1; UNPOST_I=1
		else
		    UNPRE_U=1; UNPOST_U=1
		fi;;
	trc)    TRC=1; ERR=1; TCB=1; SYS=1; RMFILES=1; RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1; UNPOST_I=1
		else
		    UNPRE_U=1; UNPOST_U=1
		fi;;
	errinstl)
                ERRINSTL=1; TRC=1; ERR=1; TCB=1; SYS=1; RMFILES=1; RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1; UNPOST_I=1
		else
		    UNPRE_U=1; UNPOST_U=1
		fi;;
	oupdt)  ODMUP=1; TRC=1; ERR=1; TCB=1; SYS=1; RMFILES=1; RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1; UNPOST_I=1
		else
		    UNPRE_U=1; UNPOST_U=1
		fi;;
	odel)   ODEL=1; TRC=1; ERR=1; TCB=1; SYS=1; RMFILES=1; RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1; UNPOST_I=1
		else
		    UNPRE_U=1; UNPOST_U=1
		fi;;
	oadd)   OADD=1; ODEL=1; TRC=1; ERR=1; TCB=1; SYS=1; RMFILES=1; RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1; UNPOST_I=1
		else
		    UNPRE_U=1; UNPOST_U=1
		fi;;
	cfg)    CFG=1; OADD=1; ODEL=1; TRC=1; ERR=1; TCB=1; SYS=1; RMFILES=1
		RECV=1 
                if [ "$BASE_LEVEL" ]; then
		    UNPRE_I=1; UNPOST_I=1
		else
		    UNPRE_U=1; UNPOST_U=1
		fi;;
	*)      ;;
    esac
}

#---------------------------------------------------------------------------
# Function:    set_lppname
# Purpose:     Set LPPNAME based on the current directory
# Arguments:   None.
# Return code: None.
#---------------------------------------------------------------------------
set_lppname ()
{
   S=$LIBDIR

   if [ $INUTREE != S ]; then      # For USR and ROOT parts
      S=${S##@(/usr/lpp/)}         # strip "/usr/lpp" from front of $S
   else                            # For SHARE parts
      S=${S##@(/usr/share/lpp/)}   # strip "/usr/share/lpp" from front of $S
   fi
   LPPNAME=${S%%*(/*)}    # Now strip off everything from the first occurence
                          # of "/" to the end of $S
}

#---------------------------------------------------------------------------
# Function:    set_update_vars
# Purpose:     Sets UPDATE_DIR based on the current directory and LPPNAME.
#              Sets UPDATE based on UPDATE_DIR.
# Arguments:   None.
# Return code: None.
#---------------------------------------------------------------------------
set_update_vars ()
{
   UPDATE_DIR=$LIBDIR

   if [ $INUTREE != S ]; then
      UPDATE_DIR=${UPDATE_DIR##*(/usr/lpp/$LPPNAME)}
   else
      UPDATE_DIR=${UPDATE_DIR##*(/usr/share/lpp/$LPPNAME)}
   fi
 
   if [ $INUTREE = M ]; then
      UPDATE_DIR=${UPDATE_DIR##*(/inst_root)}
   fi

   UPDATE_DIR=${UPDATE_DIR##*(/)}

   # If UPDATE_DIR is zero length string (then we have a base level)
   if [ -z "$UPDATE_DIR" ]; then
      UPDATE=
   else
      # For ROOT parts also strip off "/inst_root" from end
      if [ $INUTREE = M ]; then
         UPDATE_DIR=${UPDATE_DIR%%*(/inst_root)}
      fi

      # Strip everything up to and including the last "/" 
      # (for non 3.2 updates; will leave <v.r.m.f>)
      UPDATE=${UPDATE_DIR##@(*/)}

      if [ "$UPDATE_DIR" = "$UPDATE" ]; then
         IS_3_2_UPDT=TRUE
      else
         IS_3_2_UPDT=
      fi

      # Strip off anything up to and including "inst_" 
      # (for 3.2 updates, will leave <ptf>)
      UPDATE=${UPDATE##@(*inst_)}
   fi
}


#----------------------------------------------------------------------------
# Main program
#----------------------------------------------------------------------------

PATH=/usr/sbin:/usr/bin:/etc:/bin:$PATH

#-------------------------------
# Set the device and option list
#-------------------------------
DEVICE=$1
OPTIONLIST=$2           # Full path to the optionlist that installp built.

ARGV0=`bname $0`        # The name of the script (cleanup or reject)

INU_INSTSCRIPT=$ARVG0
export INU_INSTSCRIPT

#------------------
# Setup global vars
#------------------
FATAL=103               # Fatal error number of inuumsg
RC_NOLPP=2              # no product error number from inuerr.h
INUCANCL=5              # update or install unable to continue.
FAILURES=100
INU_SCRIPT_FAILED=146   # Failure msg when script fails
FAILED="false"
INUROOT=${INUROOT:-"/"}
INUTEMPDIR=${INUTEMPDIR:-"/tmp"}
AR_OPTION_DIR="/tmp"     # Could have used INUTEMPDIR, but this would 
                         # be easier if we wanted to move it
rm -f $AR_OPTION_DIR/ar_option.list

#----------------------------------------------------------------------------
#
# The current directory is the LIBDIR, or the directory where the liblpp.a
# is unarchived.  The LIBDIR is determined by the following:
#
# If installing:
#    USR:   /usr/lpp/<prodname>
#    ROOT:  /usr/lpp/<prodname>/inst_root
#    SHARE: /usr/share/lpp/<prodname>
#
# If updating:
#    If 3.2:
#       USR:   /usr/lpp/<prodname>/inst_<fixid>
#       ROOT:  /usr/lpp/<prodname>/inst_<fixid>/inst_root
#       SHARE: /usr/share/lpp/<prodname>/inst_<fixid>
#    else
#       USR:   /usr/lpp/<prodname>/option/<v.r.m.f>
#       ROOT:  /usr/lpp/<prodname>/option/<v.r.m.f>/inst_root
#       SHARE: /usr/share/lpp/<prodname>/option/<v.r.m.f>
#
# NOTE:  Read-only access in the current directory is assumed when processing
#        a ROOT part.  Because of this we have the RUNDIR.  This is a directory
#        where scripts can be run and write access is guaranteed.  RUNDIR is
#        the same as the LIBDIR for USR and SHARE parts.  For ROOT parts the
#        RUNDIR is determined by the following:
#
# If installing:
#    ROOT:  /lpp/<prodname>
#
# If updating:
#    If 3.2:
#       ROOT:  /lpp/<prodname>/inst_<ptf>
#    else
#       ROOT:  /lpp/<prodname>/<option>/<v.r.m.f>
#
#----------------------------------------------------------------------------

LIBDIR=`pwd`               # The current directory is the LIBDIR
set_lppname                # Sets LPPNAME variable

set_update_vars            # Sets UPDATE_DIR and UPDATE

if [ ! "$UPDATE" ]; then
   BASE_LEVEL=TRUE
else
   BASE_LEVEL=
fi

case $INUTREE in
   M)
       BASEDIR=/lpp/$LPPNAME

       if [ "$UPDATE" ]; then
          RUNDIR=$BASEDIR/$UPDATE_DIR
       else
          RUNDIR=$BASEDIR
       fi
       mkdir -p $RUNDIR >/dev/null 2>&1
       ;;

   U|S)
       if [ $INUTREE = U ]; then
          BASEDIR=/usr/lpp/$LPPNAME
       else
          BASEDIR=/usr/share/lpp/$LPPNAME
       fi
       RUNDIR=$LIBDIR
       ;;

   *)
       /usr/sbin/inuumsg $FATAL
       exit $INUCANCL
esac

if [ "$UPDATE" ]; then
   if [ "$IS_3_2_UPDT" ]; then
      DEINSTLDIR=$BASEDIR/deinstl/$UPDATE
   else
      DEINSTLDIR=$BASEDIR/deinstl/$UPDATE_DIR
   fi
else
   DEINSTLDIR=$BASEDIR/deinstl
fi

export SAVEDIR=$RUNDIR     # export SAVEDIR since it can be used in odm scripts

STATUSFILE=$RUNDIR/$LPPNAME.s

if [ ! -s $STATUSFILE -a $ARGV0 != "reject" ]; then
   ARGV0=reject
fi

#-----------------------------------------------------------------------------
# Loop thru the option list; for each option, do any cleanup or rejecting that
# needs to take place.  Add that option's apply list to the master apply list.
# CURMOD is the current maintenance level of the product option installed
#        on the system.
# CURFIX is the current fix level of the product option installed on the
#        system.
# NEWMOD is equal to the maintenance level of the product option that is being
#        installed.
# NEWFIX is equal to the fix level of the product option that is being 
#        installed.
# In AIX v3.2 these levels will be checked by installp.
#-----------------------------------------------------------------------------

rm -f $INUTEMPDIR/master.al $INUTEMPDIR/master.al.rl $INUTEMPDIR/status

#--------------------------------------------------------------
# if rejecting then all cleanup status flags should be set to 0
#--------------------------------------------------------------
if [ $ARGV0 = "reject" ]; then
    zeroflags
fi

> $OPTIONLIST.new

while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do
    TEST_OPTION=$OPTION

    if [ $ARGV0 != "reject" ]; then
        readstatus $OPTION
    elif [ $INUTREE != "M" ]; then
    	rm -f $OPTION.*
    	cp $DEINSTLDIR/$OPTION.* . >/dev/null 2>&1
    fi

    if [ $ARGV0 = "reject" -o $CFG -eq 1 ]; then
        if [ "$BASE_LEVEL" ]; then
           inu_run_script $OPTION $OPTION.unconfig || continue
        else
           inu_run_script $OPTION $OPTION.unconfig_u || continue
        fi
    fi

    #--------------------------------------------------------------
    # Execute all "$OPTION.*.unodmadd" and $OPTION.unodmadd" files.
    #--------------------------------------------------------------
    if [ $ARGV0 = "reject" -o $OADD -eq 1 ]; then
      (for UNODMADDFILE in $RUNDIR/$OPTION.*.unodmadd \
                           $RUNDIR/$OPTION.unodmadd; do
          inu_run_script $OPTION $UNODMADDFILE || exit 1
       done) || { FAILED=true; continue; }
    fi
    if [ $ODMUP -eq 1 ]; then
	rm -rf ./inst_ptf > /dev/null 2>&1
    fi

    #------------------------------------------------------------
    # Execute all "$OPTION.*.rodmadd" and $OPTION.rodmadd" files.
    #------------------------------------------------------------
    if [ $ARGV0 = "reject" -o $ODEL -eq 1 ]; then
      (for RODMADDFILE in $RUNDIR/$OPTION.*.rodmadd $RUNDIR/$OPTION.rodmadd; do
          inu_run_odm_add $OPTION $RODMADDFILE || exit 1
       done) || { FAILED=true; continue; }
    fi

    #-------------------------
    # Run trcupdate undo stuff
    #-------------------------
    if [ $ARGV0 = "reject" -o $TRC -eq 1 ]; then
       if [ -s $RUNDIR/$OPTION.undo.trc ]; then
         (cd $RUNDIR
          trcupdate $OPTION.undo
          if [ $? -ne 0 ]; then
             inustatusfile f $OPTION
             exit 1
          fi) || { FAILED=true; continue; }
       fi
    fi

    #------------------------------------------------------------------
    # Call errupdate with the -q flag to suppress creation of undo file
    #------------------------------------------------------------------
    if [ $ARGV0 = "reject" -o $ERR -eq 1 ]; then
       if [ -s $RUNDIR/$OPTION.undo.err ]; then
         (cd $RUNDIR
          errupdate -q -f $OPTION.undo.err
          if [ $? -ne 0 ]; then
             inustatusfile f $OPTION
             exit 1
          fi) || { FAILED=true; continue; }
       fi
    fi

    #----------------------------------------------------
    # Undo the codepoint entries by calling errinstall -q
    #----------------------------------------------------
    if [ $ARGV0 = "reject" -o $ERRINSTL -eq 1 ]; then
       if [ -s $RUNDIR/$OPTION.codepoint.undo ]; then
         (cd $RUNDIR
          errinstall -q -f $OPTION.codepoint.undo
          if [ $? -ne 0 ]; then
             inustatusfile f $OPTION
             exit 1
          fi) || { FAILED=true; continue; }
       fi
    fi

    #----------------------------
    # Run tcbck only if necessary
    #----------------------------
    if [ $ARGV0 = "reject" -o $TCB -eq 1 ]; then
       if [ -s $OPTION.tcb ]; then
      
          # Check the TCB_STATE attribute in PdAt to see if it is enabled
          # and if it isn't then return 0 and don't run tcbck at all
        
          TCB_STATE=`ODMDIR=/usr/lib/objrepos \
                     odmget -q attribute=TCB_STATE PdAt 2>/dev/null | \
                     fgrep deflt | cut -d\" -f2`

          # if tcb is not disabled (ie either enabled, or corrupted)
          if [ "$TCB_STATE" != "tcb_disabled" ]; then

             # Delete the sysck db entry that this update added
             tcbck -d -f $OPTION.tcb

             if [ $? -ne 0 ]; then
                inustatusfile f $OPTION
                continue
             fi
          fi
       fi
    fi

    #-----------------------------------------------------
    # Run 'sysck -u' if necessary.
    #  sysck -u removes:
    #   1.) The hard links to the new file
    #   2.) The soft links to the new file
    #   3.) The inventory VPD entry for the new file 
    #-----------------------------------------------------
    if [ $ARGV0 = "reject" -o $SYS -eq 1 ]; then
       if [ -s $OPTION.inventory ]; then

          #------------------
          #  Non-bos.rte case
          #------------------
          if [[ $OPTION != bos.rte* ]]; then
             sysck -u -f $OPTION.inventory $OPTION >/dev/null 2>&1

          #--------------
          #  bos.rte case
          #--------------
          else

             #---------------------------------------------------------
             # Determine if any new files were delivered in the update. 
             #---------------------------------------------------------
	     if [ -s $OPTION.rl ]; then
                RL_FILE=$OPTION.rl 
             elif [ -s $DEINSTLDIR/$OPTION.rl ]; then
                RL_FILE=$DEINSTLDIR/$OPTION.rl
             else
                RL_FILE=
             fi

             if [ -n "$RL_FILE" ]; then
                rm -f $INUTEMPDIR/$OPTION.inventory.newfs

                #-----------------------------------------------------------
                # Remove leading dot since the .rl file was built from the al
                #-----------------------------------------------------------
	        cut -c2- $RL_FILE |  
                while read FILE; do
	           grep -p "^$FILE:" $OPTION.inventory >> \
                                     $INUTEMPDIR/$OPTION.inventory.newfs
                done

                if [ -s $INUTEMPDIR/$OPTION.inventory.newfs ]; then
	           sysck -u -f $INUTEMPDIR/$OPTION.inventory.newfs $OPTION
	           if [ $? -ne 0 ]; then
                      inustatusfile f $OPTION
                      continue
	           fi
	        fi
	     fi  # -n
	  fi
       fi # -s
    fi

    #-------------------------------------------
    # Run appropriate unpost script if necessary
    #-------------------------------------------
    if [ "$BASE_LEVEL" ]; then
       if [ $ARGV0 = "reject" -o $UNPOST_I -eq 1 ]; then
          inu_run_script $OPTION $OPTION.unpost_i || continue
       fi
    else
       if [ $ARGV0 = "reject" -o $UNPOST_U -eq 1 ]; then
          inu_run_script $OPTION $OPTION.unpost_u || continue
       fi
    fi

    #----------------------------------------------------------------
    #  Build a master apply list 
    #----------------------------------------------------------------
    if [ $ARGV0 = "reject" -o $RMFILES -eq 1 -o $RECV -eq 1 ]; then
       if [ "$UPDATE" ]; then

          #--------------------------------------------------------------
          #  Theses *.rl files that may exist here are files that contain
          #  lists of file names that were added by this update.  Because
          #  they were added by the update, we need to remove them here
          #  We don't keep track of them anywhere else.
          #--------------------------------------------------------------
          if [ -s $OPTION.rl ]; then
             cat $OPTION.rl >> $INUTEMPDIR/master.al.rl
             if [ $? -ne 0 ]; then
                inustatusfile f $OPTION
                rm -f $OPTION.rl
                continue
             fi
             rm -f $OPTION.rl
          elif [ -s $DEINSTLDIR/$OPTION.rl ]; then
             cat $DEINSTLDIR/$OPTION.rl >> $INUTEMPDIR/master.al.rl
             if [ $? -ne 0 ]; then
                inustatusfile f $OPTION
                rm -f $DEINSTLDIR/$OPTION.rl
                continue
             fi
             rm -f $DEINSTLDIR/$OPTION.rl
          fi

          #-----------------------------------------------------------------
          # Save the option name so that we can check if this option has any
          # archive member files that need to be removed from it's library.
          # These are listed in the $DEINSTLDIR/$OPTION.archive.rl file.
          #----------------------------------------------------------------
          echo $OPTION >> $AR_OPTION_DIR/ar_option.list

       elif [ -s $OPTION.al ]; then
	  cat $OPTION.al >> $INUTEMPDIR/master.al
	  if [ $? -ne 0 ]; then
             inustatusfile f $OPTION
             continue
          fi
       fi
    fi

    #------------------------------------------------------------
    # if no errors so far, add this option to the new option list
    #------------------------------------------------------------
    echo $OPTION $CURMOD $CURFIX $NEWMOD $NEWFIX >> $OPTIONLIST.new

done < $OPTIONLIST

#----------------------------------------------------------------
# Remove any new files that were provided. 
#----------------------------------------------------------------
if [ -s $INUTEMPDIR/master.al.rl ]; then
   if [ $ARGV0 = "reject" -o $RMFILES -eq 1 ]; then
      # need to cd to root because the apply list has relative paths
      # Need to remove all of the files that were applied
      (cd $INUROOT; xargs rm -f) < $INUTEMPDIR/master.al.rl >/dev/null 2>&1
      if [ "$BASE_LEVEL" ]; then
         (cd $INUROOT; xargs rmdir -p) <$INUTEMPDIR/master.al.rl >/dev/null 2>&1
      fi
   fi
fi

#----------------------------------------------------------------
# Remove only the non-bos.rte already-existing files.
#----------------------------------------------------------------
if [[ $TEST_OPTION != bos.rte* ]]; then
   if [ -s $INUTEMPDIR/master.al ]; then
      if [ $ARGV0 = "reject" -o $RMFILES -eq 1 ]; then
         # need to cd to root because the apply list has relative paths
         # Need to remove all of the files that were applied
         (cd $INUROOT; xargs rm -f) < $INUTEMPDIR/master.al > /dev/null 2>&1
         if [ "$BASE_LEVEL" ]; then
            (cd $INUROOT; xargs rmdir -p) <$INUTEMPDIR/master.al >/dev/null 2>&1
         fi
      fi
   fi
fi

#----------------------------
# Done with master apply list
#----------------------------
rm -f $INUTEMPDIR/master.al.rl

if [ $ARGV0 = "reject" -o $RMFILES -eq 1 ]; then

   #---------------------------------------------------------------------
   # if the file $AR_OPTION_DIR/ar_option exists, then we are rejecting 
   # an update, because it was created in an above if statement that 
   # checked if we are rejecting an update.
   #---------------------------------------------------------------------
   if [ -s $AR_OPTION_DIR/ar_option.list ]; then

      # ----------------------------------------------------------------------
      #  The algorithm is:
      #  while read an option from the ar_option.list file (which just
      #             contains a list of options). 
      #      if the file $OPTION.archive.rl exists (same form as lpp.acf)
      #        remove all member files specified in the file from their libs
      #
      #      if the file $DEINSTLDIR/$OPTION.archive.rl exists (same form 
      #                                                         as lpp.acf)
      #        remove all member files specified in the file from their libs
      # ----------------------------------------------------------------------

      while read OPTION; do
         if [ -s $OPTION.archive.rl ]; then
            while read ar_MEM ar_LIB; do
               mem_name_only=`bname $ar_MEM`
               ar -do /$ar_LIB $mem_name_only 2> /dev/null
            done < $OPTION.archive.rl 2>/dev/null
         fi

         if [ -s $DEINSTLDIR/$OPTION.archive.rl ]; then
            while read ar_MEM ar_LIB; do
               mem_name_only=`bname $ar_MEM`
               ar -do /$ar_LIB $mem_name_only 2> /dev/null
            done < $DEINSTLDIR/$OPTION.archive.rl 2>/dev/null
         fi
      done < $AR_OPTION_DIR/ar_option.list 2>/dev/null 
   fi # to -s 
fi

rm -f $AR_OPTION_DIR/ar_option.list

while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do
   if [ "$BASE_LEVEL" ]; then
      if [ $ARGV0 = "reject" -o $UNPRE_I -eq 1 ]; then
         inu_run_script $OPTION $OPTION.unpre_i || continue
      fi
   else
      # Need to recover the save copies of the files
      if [ $ARGV0 = "reject" -o $RECV -eq 1 ]; then
         inurecv $LPPNAME $OPTIONLIST
         if [ $? -ne 0 ]; then
            while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do
               inustatusfile f $OPTION
            done < $OPTIONLIST.new
            rm -f $INUTEMPDIR/master.al $STATUSFILE
            exit $FAILURES
         fi
      fi

      if [ $ARGV0 = "reject" -o $UNPRE_U -eq 1 ]; then
         inu_run_script $OPTION $OPTION.unpre_u || continue
      fi

      # reset the stuf that was unset with the new inventory list
      SYSCK_FLAGS=F
      export SYSCK_FLAGS
      if [ -s $OPTION.inventory.restore ]; then
         sysck -i -f $OPTION.inventory.restore $OPTION
         if [ $? -ne 0 ]; then
	    inustatusfile f $OPTION
	    continue
         fi
      elif [ -s $DEINSTLDIR/$OPTION.inventory.restore ]; then
         sysck -i -f $DEINSTLDIR/$OPTION.inventory.restore $OPTION
         if [ $? -ne 0 ]; then
	    inustatusfile f $OPTION
	    continue
         fi
      fi
      unset SYSCK_FLAGS

      #----------------------------
      # Run tcbck only if necessary
      #----------------------------
      if [ $ARGV0 = "reject" -o $TCB -eq 1 ]; then
         if [ -s $OPTION.tcb ]; then

            # Check the TCB_STATE attribute in PdAt to see if it is enabled
            # and if it isn't then return 0 and don't run tcbck at all

            TCB_STATE=`ODMDIR=/usr/lib/objrepos \
                       odmget -q attribute=TCB_STATE PdAt 2>/dev/null | \
                       fgrep deflt | cut -d\" -f2`

            # if tcb is not disabled (ie either enabled, or corrupted)
            if [ "$TCB_STATE" != "tcb_disabled" ]; then

               # Add back in the file definitions to the sysck db that existed
               # before the update was applied, and verify the state of the
               # TCB w.r.t. $OPTION.

               tcbck -a -f $DEINSTLDIR/$OPTION.tcb.preapply
               tcbck -y $OPTION

               if [ $? -ne 0 ]; then
                  inustatusfile f $OPTION
                  continue
               fi
            fi
         fi
      fi
   fi

   inustatusfile s $OPTION

   #---------------------------------------------------
   # Cleanup update directories associated with $OPTION
   #---------------------------------------------------
   if [ "$UPDATE" ]; then

      if [ "$IS_3_2_UPDT" ]; then
         if [ $INUTREE = "M" ]; then
            rm -rf $RUNDIR/$OPTION.*
         else
            rm -rf $OPTION.*
            if [ $INUTREE = "U" ]; then
               rm -rf inst_root/$OPTION.* 
            fi
         fi
         rm -rf $DEINSTLDIR/$OPTION.*
      else 
         if [ $INUTREE = "M" ]; then
            rm -rf $RUNDIR
         else
            rm -rf *
         fi
         rm -rf $DEINSTLDIR
         rmdir $BASEDIR/$OPTION $BASEDIR/deinstl/$OPTION >/dev/null 2>&1
      fi
   fi

done < $OPTIONLIST.new

#----------------------------------------------------------
# Cleanup any other empty update directories that may exist
#----------------------------------------------------------
if [ "$UPDATE" ]; then
   if [ "$IS_3_2_UPDT" ]; then
      if [ $INUTREE != "M" ]; then
         cd $LIBDIR/..
         rmdir $LIBDIR >/dev/null 2>&1
      else
         rmdir $RUNDIR >/dev/null 2>&1
      fi
      rmdir $DEINSTLDIR >/dev/null 2>&1
   else
      if [ $INUTREE != "M" ]; then
         cd $LIBDIR/..
         rmdir * >/dev/null 2>&1
         cd ..
         # Note: OLDPWD is the previous directory.
         rmdir $OLDPWD > /dev/null 2>&1
      fi
   fi
fi

rm -f $STATUSFILE
if [ $FAILED = "true" ]; then
   exit $FAILURES
else
   exit 0
fi
