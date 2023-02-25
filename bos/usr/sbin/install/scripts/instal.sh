#!/bin/ksh
# @(#)31        1.77  src/bos/usr/sbin/install/scripts/instal.sh, cmdinstl, bos41J, 9521B_all 5/24/95 11:02:46
#
# COMPONENT_NAME: (CMDINSTL) command install
#
# FUNCTIONS: instal and update (these are the default scripts)
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
# NAME: update and instal
#                                                                    
# FUNCTION: script called from installp to install or update a product.
#
# ARGUMENTS:
#       $1 is the device that contains the image for the product.
#       $2 is the full path to the file that contains the list of options
#          to install/update.
#
# RETURN VALUE DESCRIPTION:
#    the return code from inusave if it fails
#    the return code from inurest if it fails
#    0 if nothing fails
#    100 if one or more options fail
#
#	The instal/update script is invoked from installp with the first 
#	parameter being the device that is being used for installation or 
#	update, the second parameter is the full path to the file containing 
#	the list of options that need to be installed or updated.  The 
#	installp command will invoke the instal/update script as instal if 
#	an installation is taking place or as update if an update is taking 
#	place.
#
#	The instal/update script uses the INUTREE, the INUTEMPDIR environment
#	variables.
#
#       The instal/update script looks for the following files, which are
#       mandatory for the usr part but are not mandatory for the root part.
#       ($OPTION is the full product option name):
#
#		$OPTION.al 			the apply list
#		$OPTION.inventory		the inventory list
#
#	The instal/update script looks for the following optional files that 
#       could be supplied with the product package:
#
#               $OPTION.prereq       The files that list the requisites
#                                    of this product option.
#               $OPTION.pre_i        The product-supplied script for pre-
#                                    installation processing.
#                                    Must have execute permissions.
#               $OPTION.pre_u        The product-supplied script for pre-
#                                    update processing.
#                                    Must have execute permissions.
#               $OPTION.post_i       The product-supplied script for post-
#                                    installation processing.
#                                    Must have execute permissions.
#               $OPTION.post_u       The product-supplied script for post-
#                                    update processing.
#                                    Must have execute permissions.
#               $OPTION.err          Updates to the error record template
#                                    repository.
#               $OPTION.trc          Updates to the trace report format
#                                    templates.
#               $OPTION.codepoint    Run with errinstal command.  Creates
#                                    $OPTION.codepoint.undo in $RUNDIR.
#               $OPTION.odmdel       A script that removes odm stanzas and 
#                                    creates an $OPTION.rodmadd stanza file
#                                    to be used in the reject/cleanup script.
#                                    Must have execute permissions.
#               $OPTION.odmadd	     ODM add stanza file.
#               $OPTION.config       The product-supplied script for
#                                    configuration for installation of
#                                    the product option.
#                                    Must have execute permissions.
#               $OPTION.config_u     The product-supplied script for
#                                    configuration for update of the
#                                    product option.
#                                    Must have execute permissions.
#
#		
#       Files that are created by the instal/update script:
#
#            $OPTION.undo.err        Cleans up the updates to the error 
#                                    record template repository.
#            $OPTION.undo.trc        Cleans up the updates to the trace 
#                                    report format templates.
#            $OPTION.codepoint.undo  Removes codepoint entries.
#            $OPTION.rodmadd         An odmadd (stanza) file that puts the 
#                                    database back the way it was before the 
#                                    install or update.
#            $LPPNAME.s              Status file for use by the cleanup
#                                    script.  Referred to as $STATUSFILE.
#
#       The status file $LPPNAME.s has the following format for each line
#           $OPTION status
#       where
#           $OPTION   is the name of the option being installed/updated
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
#           odel      $OPTION.odmdel script failed
#           oadd      odmadd failed
#           cfg       $OPTION.config script failed
#
#       Files saved by the instal/update for future use in the deinstall 
#       directory, $DEINSTLDIR:
#
#               $OPTION.undo.err     Cleans up the updates to the error 
#                                    record template repository.
#               $OPTION.undo.trc     Cleans up the updates to the trace 
#                                    report format templates.
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
#               $OPTION.rodmadd      An odmadd (stanza) file that puts the 
#                                    database back the way it was before the
#                                    install or update.
#               $OPTION.unodmadd     A script that deletes the odm entries
#                                    that were added during the install or
#                                    update.
#                                    Must have execute permissions.
#               $OPTION.unconfig     The product-supplied script for
#                                    unconfiguring the product option for
#                                    cleanup or reject of an installation.
#                                    Must have execute permissions.
#               $OPTION.unconfig_u   The product-supplied script for
#                                    unconfiguring the product option for
#                                    cleanup or reject of an update.
#                                    Must have execute permissions.
#
#   The following environment variables are used by this script:
#    INUTREE    = M|U|S        Tells this script which one of the file
#                systems are being installed or updated.
#    INUTEMPDIR        This is the location of the tmp directory
#                that this script is allowed to use.
#
#------------------------------------------------------------------------------

#-------------------------------
# local, fast version of dirname
#-------------------------------
dname() 
{
	S=$1
	S=${S%%/*([!/])*(/)}	# handles arbitrary no of / at eol
	echo $S
}

#--------------------------------
# local, fast version of basename
#--------------------------------
bname()
{
	S=$1
	S=${S%%+(/)}	# remove rightmost /s if any
	S=${S##*/}	# remove path
	echo $S
}

#---------------------------------------------------------
# This function writes out the status that installp needs.
# This function takes two parameters
# $1 is the inustatus field
# $2 is the OPTION
#---------------------------------------------------------
inustatusfile()
{
    echo "$1 $2" >>$INUTEMPDIR/status
}

#----------------------------------------------------------------------
# This function writes out the status that cleanup needs and the status
# that installp needs.  This function only writes out failed status.
# This function takes three parameters
# $1 is the inustatus field
# $2 is the OPTION
# $3 is the cleanup status
#----------------------------------------------------------------------
statusfile()
{
    FAILED="true"
    echo "$2 $3" >> $STATUSFILE
    inustatusfile $1 $2
}

#------------------------------------------------------------------
# Links files saved for future use
# Takes 2 parameters the FileName ($1) and the TargetFileName ($2)
#------------------------------------------------------------------
linkfile()
{
    if [ -s "$1" ] ; then
        ln -f $1 $2 >/dev/null 2>&1 || cp $1 $2  >/dev/null 2>&1
    fi
}

#------------------------------------------------------------------
# Unconditionally moves files
# Takes 2 parameters the FileName ($1) and the TargetFileName ($2)
#------------------------------------------------------------------
mvfile()
{
    if [ -s "$1" ] ; then
        mv -f $1 $2  >/dev/null 2>&1
    fi
}

#--------------------
# Runs the scripts $2
#--------------------
inu_run_script()
{
   if [ -x "$2" ] ; then
      $2
      if [ $? -ne 0 ] ; then
         inuumsg $INU_SCRIPT_FAILED $ARGV0 $2
         statusfile f $1 $3
         return 1
      fi
   fi
   return 0
}

#---------------
# Runs odmadd $2
#---------------
inu_run_odm_add()
{
   if [ -s "$2" ] ; then
      odmadd $2
      if [ $? -ne 0 ] ; then
         statusfile f $1 oadd
         return 1
      fi
   fi
   return 0
}

#------------------------------
# Get current inventory entries
#------------------------------
inu_get_current_inventory()
{
   [ $ARGV0 != "update" ]  &&  return
   [ ! -s $OPTION.inventory ]  &&  return

   INV_REST=$DEINSTLDIR/$OPTION.inventory.restore
   rm -f $INV_REST

   # call sysck with new -s flag to save the current inventory
   # before the update occurs so we can go back to it on reject

   sysck -u -s $INV_REST -f $OPTION.inventory $OPTION

   if [ -s $INV_REST ] 
   then

       # Don't save the checksum and size of archive members
       if [ -f lpp.acf ]
       then
          cat $INV_REST | awk -v nonarchive=0 '$1 ~ /:$/ { sub( ":", "", $1); 
              nonarchive = system("/usr/bin/grep -q " $1 " lpp.acf");
              $1 = $1 ":"  # Replace the colon deleted for the grep
          }
              { if (( nonarchive ) || (($1 != "checksum") && ($1 != "size")))
                   { print $0 }
          }' > $INV_REST.2
          mv $INV_REST.2 $INV_REST
    
       fi
   else
       rm -f $INV_REST
   fi
}

#----------------------------------------------------------------------------
#  Function    -   install_cmd_being_updated
#
#  Purpose     -   Determines if either of 2 conditions is true:
#                       1.) installp is being updated
#                       2.) instlclient or maintclient is being updated AND
#                           instlclient invoked us.
#  Returns   
#              -   returns 0   -   if both conditions are false 
#              -   returns 1   -   if contition 1 is true 
#              -   returns 2   -   if contition 2 is true 
#
#  Assumptions -   If we were invoked from instlclient, the environment var
#                  INUCLIENTS will be set to some value.
#
#----------------------------------------------------------------------------
install_cmd_being_updated ()
{

  [ "$OPTION" != "bos.rte.install" ]  &&  return 0

  [ ! -s $OPTION.al ]  &&  return 0

  fgrep "installp" $OPTION.al  >/dev/null 2>&1 && return 1

  return 0
}

#---------------------------------------------------------------------------
#
# Function: set_installed_list
# Purpose:  set/unset MIGSAVE and INSTALLED_LIST to be used by *_i scripts
# Arguments: None
# Return code: None
#
#---------------------------------------------------------------------------
set_installed_list_N_migsave ()
{
   unset INSTALLED_LIST MIGSAVE
   [ $ARGV0 = "update" ] && return

   if [ -f $BASEDIR/$OPTION.installed_list ]; then
      # Can't get rid of the installed_list file yet
      export INSTALLED_LIST=$BASEDIR/$OPTION.installed_list 
      export MIGSAVE=$MIGDIR
   fi
   
   return
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
   S=$LIBDIR 

   if [ $INUTREE != S ]; then                 # For USR and ROOT parts
      UPDATE_DIR=${S##@(/usr/lpp/$LPPNAME/)}  # strip off "/usr/lpp/$LPPNAME/"
      if [ $INUTREE = M ]; then                   # For ROOT parts
         UPDATE_DIR=${UPDATE_DIR%%*(/inst_root)}  # also strip off "/inst_root"
      fi
   else                 # For SHARE parts strip off "(/usr/share/lpp/$LPPNAME/"
      UPDATE_DIR=${S##@(/usr/share/lpp/$LPPNAME/)}
   fi

   UPDATE=${UPDATE_DIR##@(*/)}  # strip everything up to and including the
                                # last "/" (for non 3.2 updates; <v.r.m.f>)

   if [ "$UPDATE_DIR" = "$UPDATE" ]; then
      IS_3_2_UPDT=TRUE
      UPDATE=${UPDATE##@(*inst_)}  # strip off anything up to and including
                                   # "inst_" (for 3.2 updates, left w/ <ptf>)
   else
      IS_3_2_UPDT=
   fi
}

#----------------------------------------------------------------------------
# Main program
#----------------------------------------------------------------------------

#-------------------------------------------------
# set DEBUG="yes" to see a lot of debugging output
#-------------------------------------------------
DEBUG=""
if [[ "${DEBUG}" = yes ]]
then
	set -x
	for i in $(typeset +f)
	do
		typeset -ft ${i}
	done
fi

PATH=/usr/sbin:/usr/bin:/etc:/bin:$PATH

typeset -i FALSE=0

#----------------------------
# was verbose flag specified?
#----------------------------
VERBOSE=""
if [[ "${1}" = -V* ]]
then
   VERBOSE=${1}
   shift
fi

#-------------------------------
# Set the device and option list
#-------------------------------
DEVICE=$1
OPTIONLIST=$2     # a full path to the optionlist that installp built.

#------------------
# Setup global vars
#------------------
FATAL=101         # Fatal error number of inuumsg
RC_NOLPP=2        # no product error number from inuerr.h
INUCANCL=5        # update or install unable to continue.
IFREQFAIL=127     # if only ifreq's fail then ckprereq will exit with 127
INU_SCRIPT_FAILED=146   # Failure msg when script fails
FAILURES=100
FAILED="false"
ARGV0=`bname $0`
export INU_INSTSCRIPT=$ARGV0
rm -f $INUTEMPDIR/status

if [ $ARGV0 = instal -o "$INUSAVE" = "0" -o -z "$INUSAVE" ]; then
   acN=1
else
   acN=$FALSE
fi

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

if [ $ARGV0 = update ]; then
   set_update_vars         # Sets UPDATE_DIR and UPDATE
else
   UPDATE=
   UPDATE_DIR=
   IS_3_2_UPDT=
fi

case $INUTREE in
   M)
       BASEDIR=/lpp/$LPPNAME

       if [ $ARGV0 = update ] ; then
          if [ "$IS_3_2_UPDT" ]; then
             ODMUPDTDIR=$LIBDIR/inst_ptf
          fi
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

       if [ "$IS_3_2_UPDT" ] ; then
           ODMUPDTDIR=$BASEDIR/inst_ptf
       fi
       RUNDIR=$LIBDIR
       ;;

   *)
       /usr/sbin/inuumsg $FATAL
       exit $INUCANCL
esac


MIGDIR=`dname $BASEDIR`/save.config

if [ $ARGV0 = update ] ; then
   if [ "$IS_3_2_UPDT" ]; then
      DEINSTLDIR=$BASEDIR/deinstl/$UPDATE
   else
      DEINSTLDIR=$BASEDIR/deinstl/$UPDATE_DIR
   fi
else
   DEINSTLDIR=$BASEDIR/deinstl
fi

mkdir -p $DEINSTLDIR >/dev/null 2>&1

STATUSFILE=$RUNDIR/$LPPNAME.s
rm -f $STATUSFILE

SAVEDIR=$RUNDIR            # export SAVEDIR since it can be used in odm scripts 
export SAVEDIR BASEDIR

if [ $ARGV0 = update ]; then
   export UPDATE
fi

#---------------------------------------------------------------------------
# Loop thru the option names, for each option, do any processing that
# needs to take place before the files are restored, if there are no errors
# add that option's apply list to the master apply list 
# CURFIX is the current fix level of the product option installed on 
#        the system.
# NEWMOD is the maintenance level of the product option that is being
#        installed.
# NEWFIX is the fix level of the product option that is being installed.
#
# In AIX v3.2 these levels will be checked by installp.
#---------------------------------------------------------------------------

rm -f $INUTEMPDIR/master.al
MASTERLIST=

> $OPTIONLIST.new

while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do

    #--------------------------------------------------------
    # Execute the options pre_update procedure (if it exists)
    #--------------------------------------------------------
    if [ $ARGV0 = update ] ; then
	inu_run_script $OPTION $OPTION.pre_u pre_u || continue
    else
	inu_run_script $OPTION $OPTION.pre_i pre_i || continue
    fi

    #------------------------------------------------------------------
    #  If this option has an apply list (.al file) and we're updating, 
    #  then save the names of any new files (files that didn't already 
    #  exist in the base level) that are being added with the update. 
    #------------------------------------------------------------------
    if [ -s $OPTION.al ] ; then
	MASTERLIST="$MASTERLIST $OPTION.al"
        # if we are updating, save the files that do not already exist 
        # on the file system
        # The algorithm is as follows:
        # while read next apply list entry
        #   if that apply list entry is not on filesystem,
        #      check if the entry is an archive file (it will be in lpp.acf)
        #      if the apply list entry is an archive member, 
        #         check if it currently exists in the lib it belongs to
        #         if it does not
        #            write an entry of the form "<.o file> <.a file>" to the
        #            $DEINSTLDIR/<option>.archive.rl file.
        #      echo the apply list entry to the $DEINSTLDIR/<option>.rl file
        # 
        if [ $acN -eq $FALSE ] ; then
           (cd /
            while read FILENAME; do
               if [ ! -r $FILENAME ]; then 
                  fgrep $FILENAME $LIBDIR/lpp.acf 2>/dev/null | 
                       read ar_MEM ar_LIB rest_of_line
                  if [ -n "$ar_MEM" ]; then   # then we have a match
                     #  if the apply list entry (FILENAME) is the lib
                     #  instead of the member, save the lib name in 
                     #  the regular file and continue.
                     
                     if [ $FILENAME = $ar_LIB ]; then
                        echo $FILENAME
                        continue
                     fi
                     ar -t /$ar_LIB `bname $ar_MEM` >/dev/null 2>&1
                     if [ $? -ne 0 ]; then
                        echo "$ar_MEM $ar_LIB" >>$DEINSTLDIR/$OPTION.archive.rl
                     fi
                  fi
                  echo $FILENAME 
               fi
	    done) < $OPTION.al > $DEINSTLDIR/$OPTION.rl
            
            #  Save the current inventory table entry of the SWVPD to
            #  the file $DEINSTLDIR/<option>.inventory.restore.
            #  This is used when this update is rejected (we can put
            #  this entry back into the SWVPD).
	fi
    fi
    [ $acN -eq $FALSE ]  &&  inu_get_current_inventory

    #------------------------------------------------------------
    # if no errors so far, add this option to the new option list
    #------------------------------------------------------------
    echo $OPTION $CURMOD $CURFIX $NEWMOD $NEWFIX >> $OPTIONLIST.new

done < $OPTIONLIST

#----------------------------------------------------------------------------
# We no longer consider the "no apply list" case as an error, because some 
# base levels may have an installable option that only performs configuration 
# and delivers no files.
#----------------------------------------------------------------------------
if [ -n "$MASTERLIST" ]; then
    cat $MASTERLIST > $INUTEMPDIR/master.al
    if [ $? -ne 0 ] ; then
        while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do
            statusfile f $OPTION noapp
        done < $OPTIONLIST.new
        exit 1
    fi
    APPLY=TRUE
    
    # save backup versions of files
   
    if [ $ARGV0 = "update" ]; then
        inusave $INUTEMPDIR/master.al $LPPNAME
        RC=$?
  
        # if inusave failed exit with the return code from inusave
 
        if [ $RC -ne 0 ]; then
            while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do
                statusfile f $OPTION save
            done < $OPTIONLIST.new
            exit $RC
        fi
    fi
else
    APPLY=FALSE
fi

#----------------------------------------------------------------------
# Following is not necessary for -acN.  If for some reason file f1 had 
# symlinks/hardlinks previously, updating does not remove them since we 
# are not calling sysck -u on -acN
#----------------------------------------------------------------------
if [ $acN -eq $FALSE ]; then
   while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do
      # Check if this is BOS or not and skip if it is.
      if [[ $OPTION != bos.rte* ]]; then
         if [ -s $DEINSTLDIR/$OPTION.inventory.restore ]; then
            sysck -u -f $DEINSTLDIR/$OPTION.inventory.restore $OPTION
         fi
      fi
   done < $OPTIONLIST.new
fi

#-------------------------------------------
# Restore the files for all selected options
#-------------------------------------------
if [ $APPLY = TRUE ] ; then
  
   # See if the "HYPERTEXT" keyword exists in any of the cfginfo files
   HYPERTEXT=`grep "^HYPERTEXT$" *.cfginfo 2>/dev/null`

   if [ "$HYPERTEXT" -a $ARGV0 = instal ]; then
      CP_DIR=${DEVICE%@(/usr/sys/inst.images*)}
      CP_DIR=${CP_DIR:-/}
      inucp ${VERBOSE} -s $CP_DIR $INUTEMPDIR/master.al $LPPNAME
      RC=$?

   elif [ $INUTREE = M ] ; then
      inucp ${VERBOSE} -s $LIBDIR $INUTEMPDIR/master.al $LPPNAME
      RC=$?

   elif [ -f $INUTEMPDIR/.location ]; then
      inurdsd `cat $INUTEMPDIR/.location | 
         awk '{print "-v" $1 " -b" $2 " -n" $3}'` | 
      inurest -d - -q ${VERBOSE} $INUTEMPDIR/master.al $LPPNAME $OPTIONLIST.new
      RC=$?

   else
      inurest -d $DEVICE -q ${VERBOSE} $INUTEMPDIR/master.al $LPPNAME $OPTIONLIST.new
      RC=$?
   fi
   
   # if inurest or inucp failed, set all options to failed and exit with the
   # return code
  
   if [ $RC -ne 0 ]; then
      while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do
         if [ $INUTREE = M ] ; then
            statusfile f $OPTION cp
         else
            statusfile f $OPTION rest
         fi
      done < $OPTIONLIST.new
      exit $RC
   fi
fi

#------------------------------------------------------------------------
# Read in the modified option list and do any special post install/update 
# processing for each option
#------------------------------------------------------------------------
while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do

    # For migration
    set_installed_list_N_migsave

    #-------------------------------------------------------------------------
    # Execute the option's post_update or post_install procedure (if it exists)
    #-------------------------------------------------------------------------
    if [ $ARGV0 = update ] ; then
       inu_run_script $OPTION $OPTION.post_u post_u || continue
    else
       inu_run_script $OPTION ./$OPTION.post_i post_i || continue
       if [ -n "$MIGSAVE" ] && [ -f ./$OPTION.cfgfiles ]; then
          /usr/lib/instl/migrate_cfg ./$OPTION.cfgfiles $MIGSAVE
       fi
    fi

    #--------------------------------------
    # Call sysck -i to add file definitions
    #--------------------------------------
    if [ -s $OPTION.inventory ] ; then
       sysck -i -f $OPTION.inventory $OPTION
       if [ $? -ne 0 ]; then
          statusfile v $OPTION sys
          continue
       fi
    fi

    #--------------------------------------------
    # Need to call tcbck if $OPTION.tcb exists
    #--------------------------------------------
    if [ -s $OPTION.tcb ] ; then

       #--------------------------------------------------------
       # Check the tcb_enabled attribute in PdAt to see if it is 
       # enabled and only run the tcbck stuff if it is.
       #--------------------------------------------------------
       TCB_STATE=`ODMDIR=/usr/lib/objrepos \
                 odmget -q attribute=TCB_STATE PdAt 2>/dev/null | \
                 fgrep deflt | cut -d\" -f2`

       # if tcb is not enabled (explicitly enabled, or corrupted)
       if [ "$TCB_STATE" != "tcb_disabled" ]; then

          # Save the current state of the sysck database to a file called 
          # $OPTION.tcb.preapply, update the sysck database, and finally
          # verify the state of the TCB w.r.t. $OPTION.  Note that 
          # $OPTION.tcb.preapply is written to $DEINSTLDIR (since its
          # guaranteed to be writable.
       
          tcbck -f $OPTION.tcb -s $DEINSTLDIR/$OPTION.tcb.preapply
          tcbck -a -f $OPTION.tcb
          tcbck -y $OPTION

          if [ $? -ne 0 ]; then
	     # Rerun sysck to restore any ACLs removed by sysck
	     # Fail the install only if sysck now fails
       	     sysck -i -f $OPTION.inventory $OPTION
             if [ $? -ne 0 ]; then
                 statusfile f $OPTION tcb
                 continue
             fi
          fi
	fi
    fi
     
    #--------------------------------------------------------------------------
    # Execute the errupdate cmd, if the $OPTION.err file exists.  The errupdate
    # command takes $OPTION.err as an input file and creates $OPTION.err.undo 
    # in $RUNDIR.  $OPTION.err.undo must be renamed to $OPTION.undo.err for
    # use in the reject/cleanup script.
    #-------------------------------------------------------------------------
    if [ -s $OPTION.err ]; then
       (cd $RUNDIR    
        errupdate -f $LIBDIR/$OPTION.err
        if [ $? -ne 0 ]; then
           statusfile f $OPTION err
           exit 1
        fi) || { FAILED=true; continue; }
        mv $RUNDIR/$OPTION.err.undo $RUNDIR/$OPTION.undo.err
    fi

    #--------------------------------------------------------------------------
    # Execute the trcupdate cmd, if the $OPTION.trc file exists.  The trcupdate
    # command takes $OPTION.trc as an input file and creates $OPTION.undo.trc
    # in the same directory the input file is located.  For this reason 
    # $OPTION.trc is linked from the LIBDIR to the RUNDIR.
    #--------------------------------------------------------------------------
    if [ -s $OPTION.trc ]; then
        if [ $INUTREE = M ]; then
           ln -s $LIBDIR/$OPTION.trc $RUNDIR/$OPTION.trc >/dev/null 2>&1
        fi
        trcupdate $RUNDIR/$OPTION
        if [ $? -ne 0 ] ; then
           statusfile f $OPTION trc
           continue
        fi
        if [ $INUTREE = M ] ; then
           rm -f $RUNDIR/$OPTION.trc
        fi
    fi

    #--------------------------------------------------------------------------
    # Execute the errinstall cmd, if the $OPTION.codepoint file exists.
    # The errinstall command creates a $OPTION.codepoint.undo file in $RUNDIR.
    #--------------------------------------------------------------------------
    if [ -s $OPTION.codepoint ]; then
       (cd $RUNDIR
        errinstall -f $LIBDIR/$OPTION.codepoint 
        if [ $? -ne 0 ] ; then
           statusfile f $OPTION errinstl
           exit 1
        fi) || { FAILED=true; continue; }
    fi

    #--------------------------------------------------------------------------
    # This is for 3.2 updates only.  ODM files were archived in odmupdt.a files
    # and must be unarchived so that they can be run below.
    #--------------------------------------------------------------------------
    if [ "$IS_3_2_UPDT" ]; then
      (cd $RUNDIR
       for ODMUPDTFILE in $ODMUPDTDIR/$OPTION.*.odmupdt.a; do
          if [ -s $ODMUPDTFILE ]; then
             ar -x $ODMUPDTFILE >/dev/null 2>&1
             if [ $? -ne 0 ]; then
                statusfile f $OPTION oupdt
                exit 1
             fi 
             if [ $INUTREE != M ]; then
                rm -f $ODMUPDTFILE
             fi
          fi
       done) || { FAILED=true; continue; }
    fi

    #-----------------------------------------------------------------------
    # All .odmadd and .odmdel files will be processed in RUNDIR.  This is 
    # because 1.) If this is a 3.2 update there may have been .odmadd and/or
    # .odmdel files unarchived from a .odmupdt.a library into the RUNDIR; 
    # and 2.) .odmdel scripts create .rodmadd stanza files in the current
    # directory.  Because RUNDIR is guaranteed to be writable and LIBDIR is
    # not (in the ROOT case) this is why these files are processed in RUNDIR.
    # So, for ROOT parts only we will link any $OPTION.*odmadd and 
    # $OPTION.*odmdel files from LIBDIR to RUNDIR.
    #-----------------------------------------------------------------------
    if [ $INUTREE = M ]; then
       for ODMFILE in $OPTION.*odmadd $OPTION.*odmdel; do
          linkfile $ODMFILE $RUNDIR
       done
    fi

    #-----------------------------------------------------------------------
    # Execute the $OPTION.*.odmdel script(s) in RUNDIR if they exist.  Each 
    # .odmdel script should create a .rodmadd stanza file containing the 
    # entries that will be overwritten by the .odmadd file that is provided.
    # Since odmadd does not update stanzas (it only adds stanzas), it is 
    # necessary for this script to do an odmget on the stanzas it wants to 
    # delete and append them to the $OPTION.rodmadd file,then do an odmdelete
    # on those stanzas.  The $OPTION.rodmadd file will be used by the cleanup
    # and reject scripts to restore the odm database to its previous state.
    #----------------------------------------------------------------------
   (cd $RUNDIR
    if [ $ARGV0 = instal ] ; then
       # if there is no installed_list, then it is a fresh install
       # No need to do odmdel
       [ ! -f "$INSTALLED_LIST" ] && exit 0
    fi
    for ODMDELFILE in $OPTION.*.odmdel $OPTION.odmdel; do
       inu_run_script $OPTION $ODMDELFILE odel || exit 1
    done) || { FAILED=true; continue; }
        
    #--------------------------------------------------------------
    # Execute all "OPTION.*.odmadd" and "OPTION.odmadd" files 
    #--------------------------------------------------------------
   (cd $RUNDIR
    for ODMADDFILE in $OPTION.*.odmadd $OPTION.odmadd; do
       inu_run_odm_add $OPTION $ODMADDFILE || exit 1 
    done) || { FAILED=true; continue; }
        
    #-----------------------------------------------------
    # Execute the option's config procedure (if it exists)
    #-----------------------------------------------------
    if [ $ARGV0 = instal ]; then
        inu_run_script $OPTION $OPTION.config cfg || continue
    else
        inu_run_script $OPTION $OPTION.config_u cfg || continue
    fi

    #----------------------------------------------------------------
    # If the installp command is being updated, then we want to
    # only allow that update to be processed. All other updates will
    # be cancelled (via putting an 'S' in the status file).
    #
    # Also if instlclient or maintclient is being updated AND
    # we were invoked by instlclient, then we only want to
    # allow that update to be processed.  All other updates
    # will be cancelled (via putting an 'S'in the status file).
    #----------------------------------------------------------------
    install_cmd_being_updated
    if [ $? -eq 1 ]; then 
        inustatusfile S $OPTION   # installp being updated
    else
        inustatusfile s $OPTION
    fi

    #-----------------------------------
    # Get rid of migration related files
    #-----------------------------------
    if [ $INUTREE != M ]; then
       rm -f ./$OPTION.cfgfiles $BASEDIR/$OPTION.rm_inv
    fi
    rm -f $BASEDIR/$OPTION.installed_list

    echo $OPTION $CURMOD $CURFIX $NEWMOD $NEWFIX >> $OPTIONLIST.new1
done < $OPTIONLIST.new

if [ -s $OPTIONLIST.new1 ] ; then
    mv $OPTIONLIST.new1 $OPTIONLIST.new >/dev/null 2>&1
else
    > $OPTIONLIST.new
fi

#----------------------------------
# Move or link files for future use
#----------------------------------
mvfile lpp.deinstal $DEINSTLDIR

while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do

   #--------------------------------------------------------------
   # First remove any existing $OPTION.* files which may exist in 
   # the DEINSTLDIR
   #--------------------------------------------------------------
   rm $DEINSTLDIR/$OPTION.*

   mvfile   $OPTION.pre_d                  $DEINSTLDIR
   mvfile   $OPTION.unconfig_d             $DEINSTLDIR
   linkfile $OPTION.al                     $DEINSTLDIR
   linkfile $OPTION.unpre_i                $DEINSTLDIR
   linkfile $OPTION.unpre_u                $DEINSTLDIR
   linkfile $OPTION.unpost_i               $DEINSTLDIR
   linkfile $OPTION.unpost_u               $DEINSTLDIR
   linkfile $OPTION.unodmadd               $DEINSTLDIR
   linkfile $OPTION.unconfig               $DEINSTLDIR
   linkfile $OPTION.unconfig_u             $DEINSTLDIR
   linkfile $OPTION.inventory              $DEINSTLDIR
   linkfile $OPTION.tcb                    $DEINSTLDIR
   linkfile $RUNDIR/$OPTION.rodmadd        $DEINSTLDIR
   linkfile $RUNDIR/$OPTION.undo.err       $DEINSTLDIR
   linkfile $RUNDIR/$OPTION.undo.trc       $DEINSTLDIR
   linkfile $RUNDIR/$OPTION.codepoint.undo $DEINSTLDIR

   for RODMADDFILE in $RUNDIR/$OPTION.*.rodmadd; do
   	linkfile $RODMADDFILE $DEINSTLDIR
   done

   for UNODMADDFILE in $RUNDIR/$OPTION.*.unodmadd; do
   	linkfile $UNODMADDFILE $DEINSTLDIR
   done
done <$OPTIONLIST.new >/dev/null 2>&1

#-----------------------------------------------------------------------------
# Remove the $OPTIONLIST.new file.  Even though it is still being used as 
# input file for the sub-shell process above, we must remove it here.  If
# we don't, the next call to the instal script can interfere with the 
# processing in this loop which can result in one or more of the files above
# not being copied to the deinstl directory (see defect 152664).
#-----------------------------------------------------------------------------
rm -f $OPTIONLIST.new

#-----------------------------------------------------
# Make sure we remove the root RUNDIR if it's empty.
#-----------------------------------------------------
if [ $INUTREE = M ]; then
   rmdir $RUNDIR 2>/dev/null 
fi

#--------------------------------------------------
# If any of the options failed exit with a non-zero
#--------------------------------------------------
if [ "$FAILED" = "true" ] ; then
    exit $FAILURES
else
    exit 0
fi
