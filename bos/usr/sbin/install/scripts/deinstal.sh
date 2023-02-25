#!/bin/ksh
# @(#)64        1.24  src/bos/usr/sbin/install/scripts/deinstal.sh, cmdinstl, bos411, 9434B411a 8/9/94 11:38:08
# COMPONENT_NAME: (CMDINSTL) command install
#
# FUNCTIONS: deinstal (default deinstall script for installp)
#                                                                    
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: deinstal
#                                                                    
# FUNCTION: 
#       This script will deinstall all product options in the $OPTIONLIST
#       file.  The $OPTIONLIST file is the only parameter to this script.
#
# ARGUMENTS:
#       $1 is the full path to the file that contains the list of product
#          options to deinstall.
#
# RETURN VALUE DESCRIPTION:
#    0 if nothing fails
#   !0 if one or more options fail
#
#    This script looks for the following optional files that 
#    could be supplied with the product package:
#
#    $OPTION.unpre_i      The product-supplied script for cleaning up the
#                         pre-installation processing of a base level.
#    $OPTION.unpre_u      The product-supplied script for cleaning up the 
#                         pre-installation processing of an update.
#    $OPTION.unpost_i     The product-supplied script for cleaning up the 
#                         post-installation processing of a base level.
#    $OPTION.unpost_u     The product-supplied script for cleaning up the
#                         post-installation processing of an update.
#    $OPTION.undo.err     Cleans up the updates to the error 
#                         record template repository.
#    $OPTION.undo.trc     Cleans up the updates to the trace 
#                         report format templates.
#    $OPTION.unodmadd     A script that deletes the odm entries
#                         that were added during the install or update.
#    $OPTION.rodmadd      An odm add file that puts the database back
#                         the way it was before the install or update.
#    $OPTION.unconfig     The product-supplied script for unconfiguring
#                         the installation of the product option.
#    $OPTION.unconfig_u   The product-supplied script for unconfiguring
#                         the installation of the product option update.
#    $OPTION.unconfig_d   The product-supplied script for unconfiguring
#                         the installation of the product option.  
#
#    The following environment variables are used by this script:
#
#    INUTREE   = M|U|S        
#                M - root file system (formerly called miniroot)
#                U - /usr file system
#                S - /usr/share file system
#		 Tells this script which part of the product is being
#                deinstalled.  No default; fatal error if not set.
#    INUTEMPDIR  This is the location of the tmp directory
#                that this script is allowed to use.
#		 The default is /tmp.
#
#    The following is the only argument to this script
#
#    OPTIONLIST  The full path filename containing the list of product
#                options.
#
#
# NOTE 1:  Removal of save files is performed in the binary executable which
#          called this script.  
#
# NOTE 2:  This script assumes we are already in the deinstl directory for 
#          the part (USR, ROOT, or SHARE) of the product being deinstalled.
#
#---------------------------------------------------------------------------

#===========================================================================
#                          F U N C T I O N S
#===========================================================================

#-------------------------------------------------------------------------
# Given a list of files this function checks to see if the files exist and
# are executable, then executes those that are and removes them if they 
# execute successfully.
#-------------------------------------------------------------------------
attempt_exec ()
{
   ret_code=1

   while [ $# -gt 0 ]; do       # for each argument passed in to this function
      if [ -x "$1" ]; then      # if first argument is an executable file
         $1                     # execute the file
         if [ $? != 0 ]; then
            inuumsg $INU_DEINST_EXEC_ERROR $script $1
         else
            ret_code=0          # set the return code to zero if any exec
         fi                     # successfully
      fi
      rm -f $1                  # remove the file

      shift                     # shift all args one position to the left
   done

   return $ret_code
}


#---------------------------------------------------------------------
# Given a list of rodmadd files this script attempts to run odmadd for
# each, then removes them upon successful execution.
#---------------------------------------------------------------------
attempt_odmadd ()
{
   ret_code=1

   while [ $# -gt 0 ]; do       # for each argument passed in to this function
      if [ -r "$1" ]; then      # if first argument is a readable file
         odmadd $1
         if [ $? != 0 ]; then
            inuumsg $INU_DEINST_EXEC_ERROR $script $1
         else
            ret_code=0          # set the return code to zero if any odmadd
         fi                     # is successful
      fi
      rm -f $1                  # remove the file

      shift                     # shift all args one position to the left
   done

   return $ret_code
}


#----------------------------------------------------------------
# Records the result of the deinstallation attempt of the current 
# $OPTION in the STATUSFILE.
#----------------------------------------------------------------
record_status ()
{
   status=$1
   if [ $status = 0 ]; then
      echo "s $OPTION" >>$STATUSFILE   # Record success for $OPTION
   else
      echo "f $OPTION" >>$STATUSFILE   # Record failure for $OPTION
   fi
}


#---------------------------------------------------------------------
# From each directory listed, removes all subsequent empty directories
#---------------------------------------------------------------------
rm_empty_dirs ()
{
   LANG=C   # For sort
   for dir in $@; do
      if [ -d "$dir" ]; then
         find $dir -type d -print | sort -r | \
            xargs -i rmdir {} >/dev/null 2>&1
      fi
   done
}


#-------------------------------
# Run the undo stuff for $OPTION
#-------------------------------
do_undo_stuff ()
{
   if [ -f "$OPTION.undo.trc" ]; then
      #---------------------------------------------------------------
      # Note: trcupdate appends '.trc' to the undo script it is given.
      #       It MUST be invoked without the '.trc' appended.
      #---------------------------------------------------------------
      trcupdate $OPTION.undo  &&  rm -f $OPTION.undo.trc
   fi

   if [ -f "$OPTION.undo.err" ]; then
      errupdate -f $OPTION.undo.err  &&  rm -f $OPTION.undo.err
   fi

   if [ -f "$OPTION.codepoint.undo" ]; then
      errinstall -q -f $OPTION.codepoint.undo &&  rm -f $OPTION.codepoint.undo
   fi

   if [ "$REMOVE_TCB" -a -f $OPTION.tcb ]; then
      tcbck -d -f $OPTION.tcb
   fi
}


#-----------------------------------------
# Do pre-rminstal processing for an update
#-----------------------------------------
do_updt_pre_rm_stuff ()
{
   if [ ! "$UNCONFIG_D_EXISTS" ]; then
      attempt_exec $OPTION.unconfig_u
   fi

   attempt_exec $OPTION.unodmadd $OPTION.*.unodmadd
   attempt_odmadd $OPTION.rodmadd $OPTION.*.rodmadd

   do_undo_stuff

   if [ ! "$UNCONFIG_D_EXISTS" ]; then
      attempt_exec $OPTION.unpost_u
   fi
}


#----------------------------------------------
# Do pre-rminstal processing for the base level
#----------------------------------------------
do_base_pre_rm_stuff ()
{
   attempt_exec $OPTION.unodmadd $OPTION.*.unodmadd
   attempt_odmadd $OPTION.rodmadd $OPTION.*.rodmadd

   do_undo_stuff

   if [ "$UNCONFIG_D_EXISTS" ]; then
      attempt_exec $OPTION.unconfig_d
   else
      attempt_exec $OPTION.unconfig
      attempt_exec $OPTION.unpost_i
   fi
}


#------------------------------------------
# Do post-rminstal processing for an update
#------------------------------------------
do_updt_post_rm_stuff ()
{
   if [ ! "$UNCONFIG_D_EXISTS" -a ! "$MIGRATING" ]; then
      attempt_exec $OPTION.unpre_u
   fi

   # Get rid of $OPTION.* entries in $DEINSTLDIR/$UPDATE_DIR
   rm -f $OPTION.*
}


#-----------------------------------------------
# Do post-rminstal processing for the base level
#-----------------------------------------------
do_base_post_rm_stuff ()
{
   if [ ! "$UNCONFIG_D_EXISTS" -a ! "$MIGRATING" ]; then
      attempt_exec $OPTION.unpre_i
   fi

   #---------------------------------------------------------------------
   # Cleanup OPTION related stuff in LIBDIR for USR and SHARE parts.
   # We don't want to cleanup this up for ROOT parts because this infor-
   # mation would be needed for reapply of the OPTION.
   #---------------------------------------------------------------------
   if [ $INUTREE != M ]; then
      rm -rf $LIBDIR/$OPTION $LIBDIR/$OPTION.*

      #------------------------------------------------------------------
      # When we deinstall the USR part when can remove corresponding ROOT
      # LIBDIR stuff for the same $OPTION (if it exists)
      #------------------------------------------------------------------
      if [ $INUTREE = U  -a  -d $LIBDIR/inst_root ]; then
         rm -rf $LIBDIR/inst_root/$OPTION $LIBDIR/inst_root/$OPTION.*
      fi 
   fi

   rm -rf $DEINSTLDIR/$OPTION $DEINSTLDIR/$OPTION.*
   rm_empty_dirs $DEINSTLDIR
}


#--------------------------------------------------------------------
# Run all undo and unconfig scripts for updates and the base level 
# that exist and should be run BEFORE files are removed by rminstal.
#--------------------------------------------------------------------
do_pre_rm_stuff ()
{
    #------------------------------------------
    # Check to see if $OPTION.unconfig_d exists
    #------------------------------------------
    if [ -f $OPTION.unconfig_d ]; then
       UNCONFIG_D_EXISTS=TRUE
    else
       UNCONFIG_D_EXISTS=   
    fi

    #------------------------------------------------
    # Set REMOVE_TCB based on the TCB_STATE attribute
    #------------------------------------------------
    TCB_STATE=`ODMDIR=/usr/lib/objrepos \
               odmget -q attribute=TCB_STATE PdAt 2>/dev/null | \
               fgrep deflt | cut -d\" -f2`

    # if tcb is not disabled (ie either enabled, or corrupted)
    if [ "$TCB_STATE" != "tcb_disabled" ]; then
       REMOVE_TCB=TRUE
    else
       REMOVE_TCB=
    fi
    
    #-------------------------------------------
    # Do pre-rminstal processing for each update
    #-------------------------------------------
    while read UPDATE_DIR rest_of_line; do
       cd $DEINSTLDIR/$UPDATE_DIR 2>/dev/null  &&  do_updt_pre_rm_stuff
    done <$UPDATE_DIR_FILE

    #-------------------------------------------------------------
    # Do pre-rminstal processing for the base level product option
    #-------------------------------------------------------------
    cd $DEINSTLDIR 2>/dev/null  &&  do_base_pre_rm_stuff
}


#--------------------------------------------------------------------
# Run the un_pre scripts for updates and the base level if they exist.
# This function should be called AFTER files are removed by rminstal.
#--------------------------------------------------------------------
do_post_rm_stuff ()
{
    #--------------------------------------------
    # Do post-rminstal processing for each update
    #--------------------------------------------
    while read UPDATE_DIR rest_of_line; do
       cd $DEINSTLDIR/$UPDATE_DIR 2>/dev/null  &&  do_updt_post_rm_stuff
    done <$UPDATE_DIR_FILE

    #------------------------------------
    # We're done with the UPDATE_DIR_FILE
    #------------------------------------
    rm -f $UPDATE_DIR_FILE

    #--------------------------------------------------------------
    # Do post-rminstal processing for the base level product option
    #--------------------------------------------------------------
    cd $DEINSTLDIR 2>/dev/null
    do_base_post_rm_stuff
}


#-------------------------------------------------------------------
# Create a list of the updates corresponding to the current $OPTION
# using odmget; put them in the file, $UPDATE_DIR_FILE
#-------------------------------------------------------------------
create_update_list ()
{
    odmget -q "lpp_name='$OPTION' and update='1'" product 2>$ODMERR |
    awk '/\tver = |\trel = |\tmod = |\tfix = |\tptf = / \
       {print $3 }'|
    while read ver; do
       read rel
       read mod
       read fix
       read ptf
       if [[ $ver = 3 ]]; then
          echo "$ptf"
       else
          echo "$OPTION/$ver.$rel.$mod.$fix"
       fi
    done | sed -e 's/"//g' >$UPDATE_DIR_FILE 2>/dev/null

    if [ -s $ODMERR ]; then
       cat $ODMERR
       rm -f $ODMERR
       inuumsg $FATAL
       exit $FATAL
    fi

}


#===========================================================================
#                       M A I N   P R O G R A M
#===========================================================================

SUCCESS=0                  # successful return code
FATAL=103                  # Fatal error number of inuumsg
ST_AVAILABLE=1             # VPD state of available
ST_DEINSTALLING=8          # VPD state of deinstalling
INU_DEINST_EXEC_ERROR=118  # deinstal message numbers for inuumsg

#-----------------------------------
# Make sure the PATH is set properly
#-----------------------------------
export PATH=/usr/bin:/etc:/usr/sbin:/usr/lib/instl:$PATH

DEINSTLDIR=`pwd`
INUTEMPDIR=${INUTEMPDIR:-"/tmp"}
STATUSFILE=$INUTEMPDIR/status
UPDATE_DIR_FILE=$INUTEMPDIR/UPDATE_DIR_FILE.$$
ODMERR=$INUTEMPDIR/ODMERR.$$

script=`basename $0`
OPTIONLIST=$1

#-------------------------------------------
# Make sure the $OPTIONLIST file is readable
#-------------------------------------------
if [ ! -r "$OPTIONLIST" ]; then
   inuumsg $FATAL
   exit $FATAL
fi

#----------------------------------------------------------------------
# Set appropriate directories and variables based on whether USR, ROOT,
# or SHARE part
#----------------------------------------------------------------------
case $INUTREE in
    M)  # Deinstalling ROOT part
        LPPNAME=`echo $DEINSTLDIR | awk -F'/' '{ print $3 }'`
        LIBDIR=/usr/lpp/$LPPNAME/inst_root
        ODMDIR=/etc/objrepos
        ;;
    U)  # Deinstalling USR part
        LPPNAME=`echo $DEINSTLDIR | awk -F'/' '{ print $4 }'`
        LIBDIR=/usr/lpp/$LPPNAME
        ODMDIR=/usr/lib/objrepos
        ;;    
    S)  # Deinstalling SHARE part
        LPPNAME=`echo $DEINSTLDIR | awk -F'/' '{ print $5 }'`
        LIBDIR=/usr/share/lpp/$LPPNAME
        ODMDIR=/usr/share/lib/objrepos
        ;;    
    *)  # ERROR - INUTREE MUST BE SET!
        inuumsg $FATAL
        exit $FATAL
esac


#----------------------------------------------
# Process each OPTION from OPTIONLIST file
#----------------------------------------------
while read OPTION MIGRATING rest_of_line; do

    #---------------------------------------------
    # Determine if MIGRATING is set to "MIGRATING"
    #---------------------------------------------
    if [ "$MIGRATING" != "MIGRATING" ]; then
       MIGRATING=
    fi

    #------------------------------------------------------------------
    # Create a list of the updates corresponding to the current $OPTION
    # using odmget; put them in the file, $UPDATE_DIR_FILE
    #------------------------------------------------------------------
    create_update_list

    #--------------------------------------------------------------------
    # Run all undo and unconfig scripts for updates and the base that 
    # exist and should be run before files are removed by rminstal.
    #--------------------------------------------------------------------
    if [ ! "$MIGRATING" ]; then
       do_pre_rm_stuff
    fi

    #-------------------------------------------------------------------
    # Remove files belonging to the product option from the file system.
    # This routine subsequently removes corresponding information from
    # the SWVPD inventory table.
    #-------------------------------------------------------------------
    rminstal $LPPNAME LPPOPTION $OPTION
    record_status $?

    #--------------------------------------------------------------------
    # Run the un_pre scripts for updates and the base level if they exist.
    # This function should be called AFTER files are removed by rminstal.
    #--------------------------------------------------------------------
    do_post_rm_stuff

done <$OPTIONLIST   # end while read OPTION


#-----------------------------------------------------------------------
# Remove the $LPPNAME directory if all the product options have been
# deinstalled.  To do this, check the ODM product table to see if there
# are any entries for the product, LPPNAME, that are not AVAILABLE
# (state=1) or DEINSTALLING (state=8).
#-----------------------------------------------------------------------
ODMERR=$INUTEMPDIR/ODMERR.$$
MORE_OPTIONS=`odmget -q "name='$LPPNAME' and \
                         update='0' and \
                         state!='$ST_AVAILABLE' and \
                         state!='$ST_DEINSTALLING'" product 2>$ODMERR`
if [ -s $ODMERR ]; then
   cat $ODMERR
   rm -f $ODMERR

elif [ -z "$MORE_OPTIONS" ]; then
   cd $DEINSTLDIR/../.. 2>/dev/null  &&  rm -rf $LPPNAME

   #---------------------------------------------
   # Remove the LIBDIR if its a USR or SHARE part
   #---------------------------------------------
   [ $INUTREE != M ]  &&  rm -rf $LIBDIR
fi

exit $SUCCESS
