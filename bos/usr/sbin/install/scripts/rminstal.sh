#!/bin/ksh
# @(#)72	1.45  src/bos/usr/sbin/install/scripts/rminstal.sh, cmdinstl, bos411, 9440A411i 10/6/94 15:09:39
# COMPONENT_NAME: (CMDINSTL) command install
#
# FUNCTIONS: rminstal
#                                                                    
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1990, 1991
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                    
#
# NAME: rminstal
#                                                                    
# FUNCTION: 
#
# ARGUMENTS:
#       $1 is the name of the product.
#       $2 is used to indicate whether the third argument is the name of
#          a file containing a list of product options, or an individual
#          product option.  If $2 is "OPTIONLIST" then $3 is assumed to be
#          a file containing a list of product options, otherwise $3 is 
#          assumed to be an individual product option.
#	$3 is either the full path to the optionlist that installp builds,
#          or an individual product option.
#
# RETURN VALUE DESCRIPTION:
#    0 if nothing fails
#    Non-zero means something failed
#
#       The rminstal script is invoked from installp and the deinstal script.
#       The first parameter is the name of the product and its value is 
#       assigned to LPPNAME.  The second argument is used to indicate whether
#       the third argument is a file containing a list of product options, or
#       a single product option.  When called from installp (actually from
#       inu_remove_opts.c) the second argument will be "OPTIONLIST" indicating
#       that the third argument is a file containing a list of product options.
#       When called from the deinstal script the second argument will be 
#       "OPTION" indicating that the third argument is a single product
#       option.
#
#	This script uses the INUTREE, the INUTEMPDIR environment variables.
#
#	This script looks for the following optional files 
#       ($OPTION is the full option name) ($LPPNAME is the product name):
#
#		$OPTION.al		the apply list
#
#   The following environment variables are used by this script:
#    INUTREE   = M|U|S        
#                M - root file system
#		 U - /usr file system
#		 S - /usr/share file system
#		 Tells this script which one of the file
#                systems are being installed.
#		 No default fatal error if not set.
#    INUTEMPDIR  This is the location of the tmp directory
#                that this script is allowed to use.
#		 The default is /tmp.
#
#-----------------------------------------------------------------------
# Functions
#-----------------------------------------------------------------------


#----------------------------------------------------------------------
# Function    : local, fast version of dirname 
# Arguments   : path 
# Globals     : None 
# Return code :  echos dirname
#----------------------------------------------------------------------
dname()
{
        S=$1
        S=${S%%/*([!/])*(/)}    # handles arbitrary no of / at eol
        echo $S
}

#----------------------------------------------------------------------
# Function    : local, fast version of basename for / delimiters
# Arguments   : path
# Globals     : None
# Return code :  echos basename
#----------------------------------------------------------------------
bname()
{
        S=$1
        S=${S%%+(/)}    # remove rightmost /s if any
        S=${S##*/}      # remove path
        echo $S
}

#----------------------------------------------------------------------
# Function    : Remove files from the system based on the apply list 
# Arguments   : None
# Globals     : OPTION
# Return code : None
#----------------------------------------------------------------------
inu_rm_by_al_files()
{
   # This won't happen for 1to1 migration
   #
   if [[ -s $DEINSTLDIR/$OPTION.al ]]; then
      (cd /; xargs rm -f) < $DEINSTLDIR/$OPTION.al > /dev/null 2>&1
      rm -f $DEINSTLDIR/$OPTION.al
   fi

   if [[ -s ./$OPTION.al ]]; then
      (cd /; xargs rm -f) < ./$OPTION.al > /dev/null 2>&1
   fi

   return 0
}

#----------------------------------------------------------------------
# Function    : Remove files from the system based on the inventory file 
# Arguments   : None
# Globals     : None 
# Return code : None
#----------------------------------------------------------------------
inu_rm_by_swvpd_files()
{
   awk -F':' '{ if ($0=/:$/) print $1 }' $1 | 
       (cd /; xargs rm -f) >/dev/null 2>&1
   return 0
}

#----------------------------------------------------------------------
# Function    : Driver function for normal (non-migration) file removal
#               based on al, and inv. 
#               1to1 migration also uses this. 
# Arguments   : None
# Globals     : OPTION ODMDIR INUTEMPDIR 
# Return code : None
#----------------------------------------------------------------------
inu_rm_install()
{
   LPP_ID=`odmget -q name=$OPTION lpp | awk '/lpp_id/{print $3}'`
   if [[ -z "$LPP_ID" ]]; then
      inu_rm_by_al_files 
      return 
   fi
		
   #-------------------------------------------------------------------
   # Create a file, $INUTEMPDIR/tmp.inv, that contains the inventory
   # entries corresponding to $LPP_ID (Note:  ODMDIR is already set
   # prior to calling this function).  The stanza records are filtered
   # through awk to print information about the filename (loc0),
   # links (loc1), and symlinks (loc2).  The filename followed by a ":"
   # will always be printed.  Information about links and symlinks
   # will not be printed unless these fields have something in them.
   # Double quotes are stripped out in the call to sed.
   #------------------------------------------------------------------

   odmget -q lpp_id=$LPP_ID inventory |
   awk '/loc/\
   {
      if ($1 == "loc0")
         print $3":";
   }' |
   sed -e 's/"//g' > $INUTEMPDIR/tmp.invlist
   
   # call sysck with -s flag to build inventory file for remove
   sysck -u -s $INUTEMPDIR/tmp.inv -f $INUTEMPDIR/tmp.invlist $OPTION
   rm $INUTEMPDIR/tmp.invlist

   if [ "$REMOVE_TCB" -a -f $DEINSTLDIR/$OPTION.tcb ]; then
      tcbck -d -f $DEINSTLDIR/$OPTION.tcb
   fi

   cat $INUTEMPDIR/tmp.inv | egrep -v "size = |checksum = " > $INUTEMPDIR/tmp.inv2

   # Remove the inventory entries for each stanza in $INUTEMPDIR/tmp.inv2
   sysck -u -f $INUTEMPDIR/tmp.inv2 $OPTION

   inu_rm_by_swvpd_files $INUTEMPDIR/tmp.inv2
   inu_rm_by_al_files 

   # get rid of the empty directories now
   rmdir_by_inv $INUTEMPDIR/tmp.inv2
   rmdir_by_al  $DEINSTLDIR/$OPTION.al 

   # Remove update and deinstall related files and directories
   rm_updt_deinst_files
   rm $INUTEMPDIR/tmp.inv
   rm $INUTEMPDIR/tmp.inv2
 
   return $TRUE
}


#----------------------------------------------------------------------
#
# Function: Remove update and deinstall related files and directories
#           having to do with OPTION.
#
#----------------------------------------------------------------------
rm_updt_deinst_files ()
{
   #------------------------------------------------------------
   # If $DEINSTLDIR/$OPTION we have a post-3.2 cleanup situation
   #------------------------------------------------------------
   if [ -d "$DEINSTLDIR/$OPTION" ]; then
      rm -rf $BASEDIR/$OPTION $DEINSTLDIR/$OPTION

      #------------------------------------------------------------------
      # If processing a USR part then we can get rid of all the ROOT part 
      # update LIBDIRs
      #------------------------------------------------------------------
      if [ $INUTREE = U ]; then
         rm -rf /usr/lpp/$LPPNAME/$OPTION/*/inst_root
      fi

   #------------------------------------------
   # Otherwise we have a 3.2 cleanup situation
   #------------------------------------------
   else
      #--------------------------------------------------------------
      # Remove all OPTION related files from PTF directories decended
      # from DEINSTLDIR
      #--------------------------------------------------------------
      rm -f $DEINSTLDIR/*/$OPTION.*

      #-----------------------------------------------------------
      # If processing a USR or a SHARE part then we can remove all 
      # OPTION related files from PTF LIBDIRs
      #-----------------------------------------------------------
      if [ $INUTREE = U ]; then
         rm -f $LIBDIR/inst_*/inst_root/$OPTION.*
         ls $LIBDIR/inst_*/$OPTION.* | grep -v inst_root | xargs rm -f
      elif [ $INUTREE = S ]; then
         rm -f $LIBDIR/inst_*/$OPTION.*
      fi

      #--------------------------------------------------------------------
      # Now see if we can remove the ptf directories.  To determine this
      # we must check the product database and make sure there are no other
      # OPTIONs still installed that depend on this ptf
      #--------------------------------------------------------------------

      #--------------------------------------------------------------------
      # First make a list of all the non-AVAILABLE ptf records corresponding
      # to the current $OPTION
      #--------------------------------------------------------------------
      ptf_list=`odmget -q "lpp_name=$OPTION and update=1 and state!=1" \
                product | grep "ptf = " | awk '{ print $3 }' | sort -u | \
                sed -e "s/\"//g"`

      #--------------------------------------------------------------------
      # For each $OPTION related ptf see if there is also a non-AVAILABLE
      # product record for the same ptf for some other OPTION in the same
      # product.  If there are none then we can blast that ptf's directories
      #--------------------------------------------------------------------
      for ptf in $ptf_list; do
         still_needed=`odmget -q "name=$LPPNAME and lpp_name!=$OPTION and \
                       ptf=$ptf and state!=1" product | grep "lpp_name = "`
         if [ ! "$still_needed" ]; then
            rm -rf $DEINSTLDIR/$ptf

            #---------------------------------------------------------------
            # If processing a USR or SHARE part then we can blast the LIBDIR
            # for this ptf
            #---------------------------------------------------------------
            if [ $INUTREE != M ]; then
               rm -rf $LIBDIR/inst_$ptf

               #-------------------------------------------------------------
               # If processing a USR part then we can blast the corresponding
               # ROOT ptf LIBDIR as well
               #-------------------------------------------------------------
               if [ $INUTREE = U ]; then
                  rm -rf $LIBDIR/inst_$ptf/inst_root
               fi
            fi
         fi
      done
   fi
}


#----------------------------------------------------------------------
#
# Function    : Remove the files in OPTION.rm_inv, and inventory
#               Set cp_flag to MIGRATING/ COMPAT 
#
# Arguments   : None
#
# Globals     : OPTION
#
# Return code :  0 for success 
#
#----------------------------------------------------------------------
mig_rm_install ()
{

   # Set each option in the installed_list to migrating
   #
   if [[ -f $BASEDIR/$OPTION.installed_list ]]; then

      while read option level; do        
          set_cp_flag $option $LPP_MIGRATING_VALUE
      done < $BASEDIR/$OPTION.installed_list

   fi

   # Do the removal based on rm_inv and inventory 
   rm_by_inv 

   # Blast each option now if it has completely migrated
   if [[ -f $BASEDIR/$OPTION.installed_list ]]; then
      while read option; do
          check_completely_migrated $option  && { 
                blast_migrated_option $option
                # blast usr part if necessary
                if [ "$INUTREE" = "M" -a "$ROOT_ONLY_OP" != "y" ]; then  
                    ODMDIR=/usr/lib/objrepos
                    blast_migrated_option $option
                    ODMDIR=/etc/objrepos
                fi
          }
      done < $BASEDIR/$OPTION.installed_list
   fi
  
   return $TRUE
}

#----------------------------------------------------------------------
# Function    : set MASK bit of the cp_flag
# Arguments   : option  MASK 
# Globals     : ODMDIR 
# Return code : TRUE on success
#               FALSE on failure
#----------------------------------------------------------------------
set_cp_flag ()
{
   OPT=$1
   MASK=$2
   NAME=
   typeset -i NEW_CP_FLAG=0 \
              ORIG_CP_FLAG=0 \
              INSTALLED_VERSION=0

   # Don't mark bos.data > 3 as MIGRATING 
   if [[ "$INUTREE" = "S" && $MASK -eq $LPP_MIGRATING_VALUE ]]; then
      if [[ "$OPT" = "bos.data" ]]; then
         INSTALLED_VERSION=`odmget -q"name=bos.data" lpp |\
			    awk '$1 == "ver" {print $0}'`
         [[ $INSTALLED_VERSION -gt 3 ]] && return $TRUE
      fi
   fi

   ORIG_CP_FLAG=`odmget -q"lpp_name=$OPT and update=0 and state!=1" product | \
            awk '/cp_flag/{print $3}'`
   [[ $ORIG_CP_FLAG -eq 0 ]] && return $FALSE

   # No need to go to the ODM if the flag is already set
   (( already_set=$ORIG_CP_FLAG & $LPP_MIGRATING_VALUE ))
   [[ $already_set -ne 0 ]] && return $TRUE


   let NEW_CP_FLAG="$ORIG_CP_FLAG|$MASK"
   print "product: cp_flag=$NEW_CP_FLAG" | \
          odmchange -o product -q"lpp_name=$OPT and update=0"

   return $TRUE
}

#----------------------------------------------------------------------
# Function    : unset MASK bit of the cp_flag
# Arguments   : option  MASK
# Globals     : ODMDIR
# Return code : TRUE on success
#               FALSE on failure
#----------------------------------------------------------------------
unset_cp_flag ()
{
   OPT=$1
   MASK=$2
   NAME=
   typeset -i NEW_CP_FLAG=0 \
              ORIG_CP_FLAG=0

   ORIG_CP_FLAG=`odmget -q"lpp_name=$OPT and update=0" product | \
            awk '/cp_flag/{print $3}'`
   [[ $ORIG_CP_FLAG -eq 0 ]] && return $FALSE

   [[ $ORIG_CP_FLAG -lt $MASK ]] && return $TRUE

   # subtract MASK from orig val
   let NEW_CP_FLAG=$ORIG_CP_FLAG-$MASK

   print "product: cp_flag=$NEW_CP_FLAG" | \
          odmchange -o product -q"lpp_name=$OPT and update=0"

   return $TRUE
}

#----------------------------------------------------------------------
# Function    : blast_migrated_option
# Arguments   : mig_option 
# Globals     : ODMDIR 
# Return code : $TRUE for success, $FALSE for failure
#----------------------------------------------------------------------
blast_migrated_option ()
{
   mig_option=$1                      # The migrated option
   mig_lpp=`get_lppname $mig_option`  # Get the lppname of the migrated option

   [ ! "$mig_lpp" ] && return $FALSE

   # Get lpp_id of the migrated option
   LPP_ID=`odmget -q name=$mig_option lpp | awk '/lpp_id/{print $3}'`

   # Blast product record
   odmdelete -o product -qlpp_name=$mig_option >/dev/null 2>&1 || return $FALSE

   # Blast lpp record
   odmdelete -o lpp -qname=$mig_option >/dev/null 2>&1 || return $FALSE
   
   # No need to remove inventory stuff since it does not exist

   # History vpd is also blasted since with entries in lpp db, we can't
   # read this
   odmdelete -o history -qlpp_id=$LPP_ID >/dev/null 2>&1 || return $FALSE

   rm -f $BASEDIR/.$OPTION.use_invfile 

   blast_mig_libdir $mig_lpp

   return $TRUE 
}

#----------------------------------------------------------------------
# Function    : Remove LIBDIR for migrated lpps only if we're not in
#               the LIBDIR we're trying to remove.
# Arguments   : mig_lpp
# Return code : TRUE
#----------------------------------------------------------------------
blast_mig_libdir ()
{
  mig_lpp=$1

  [[ $mig_lpp = "bos" ]]  && return $TRUE

  BDIR=lpp/$mig_lpp   # Base directory of the migrated option

  CWD=`pwd`

  case $INUTREE in

       M) [[ $CWD = /usr/$BDIR/inst_root ]]  &&  return $TRUE
          rm -rf /$BDIR $BDIR/deinstl /usr/$BDIR /usr/$BDIR/$DEINSTLDIR
          ;;

       U) [[ $CWD = /usr/$BDIR ]]  &&  return $TRUE
          if [[ ! -d /usr/$BDIR/inst_root ]]; then
             rm -rf /usr/$BDIR /usr/$BDIR/$DEINSTLDIR
          fi 
          ;;
          
       S) [[ $CWD = /usr/share/$BDIR ]]  &&  return $TRUE
          rm -rf /usr/share/$BDIR /usr/share/$BDIR/deinstl
          ;;

       *)   

   esac

  return $TRUE
}

#----------------------------------------------------------------------
#
# Function    : Remove SWVPD entries, and the files and directories
#               based on the RM_INV and the .inventory files
#
# Arguments   : None 
#
# Globals     : OPTION, RM_INV 
# 
# Note        : Directories need to be empty by the time we call
#               rm.  The burden of ensuring this is on the lpp_owner.   
#
# Return code :  0 for success 
#                exits with INUCANCL for errors.
#
#----------------------------------------------------------------------
rm_by_inv ()
{
   if [ "$REMOVE_TCB" -a -f $DEINSTLDIR/$OPTION.tcb ]; then
      tcbck -d -f $DEINSTLDIR/$OPTION.tcb
   fi

   # Process rm_inv 
   files=
   if [[ -s $OPTION.rm_inv ]]; then

      # remove VPD entries using rm_inv
      sysck -u -f $OPTION.rm_inv  ||  abort_rminstal

      # remove files based on rm_inv 
      inu_rm_by_swvpd_files $OPTION.rm_inv  $OPTION
      files=`pwd`/$OPTION.rm_inv
   fi

   # remove VPD entries using inventory 
   if [[ -s $OPTION.inventory ]]; then
      cat $OPTION.inventory | grep ":" > $INUTEMPDIR/tmp.inv
      sysck -u -f $INUTEMPDIR/tmp.inv  ||  abort_rminstal

      # remove files based on inventory 
      inu_rm_by_swvpd_files $OPTION.inventory

      files="$files `pwd`/$OPTION.inventory"
   fi

   # get rid of the empty directories now
   rmdir_by_inv  $files

   # Should we do blasting based on .al also??
    
   return $TRUE 
}

#----------------------------------------------------------------------
#
# Function:    Prints any inuumsgs and exits the rminstal script.
#
# Arguments:   0 to N Message numbers to be printed with inuumsg.
#
# Globals:     INUCANCL, INU_RMI_FAIL
#
# Return code: exits with INUCANCL 
#
#----------------------------------------------------------------------
abort_rminstal ()
{
   while [ $# != 0 ]; do
       inuumsg $1
       shift
   done

   inuumsg $INU_RMI_FAIL
   exit $INUCANCL
}

#----------------------------------------------------------------------
#
# Function    : Check if the fileset is completely migrated 
#               Blast root part during U processing if it is empty.
# Arguments   : Option name 
# Globals     : ODMDIR
#
# Return code : TRUE    if no files are associated with the option
#               FALSE   Files are still associated with the option OR
#			option no longer exists
# 
# Note: A fileset is completely migrated if:
#       1. It has UR, and both parts are  empty (no files)
#       2. It has UR, root only op, root has no files
#       3. It has U,  U is empty
#       4. It has S,  S is empty
#----------------------------------------------------------------------
#
check_completely_migrated ()
{
   option=$1

   # Check to see if this option has already been blasted
   LPP_ID=`odmget -q name=$option lpp 2>/dev/null | awk '/lpp_id/{print $3}'`
   if [ -z "$LPP_ID" ]
   then
      return $FALSE	 # Inhibit any further processing of this option
   fi

   if [ "$INUTREE" = "S" ]; then     # doing share part
       no_files_in_inventory  $option
       return $? 
   fi

   if [ "$INUTREE" = "U" ]; then     # doing usr part
       # Check if USR part contains root part
       usr_contains_root 
       if [ $? -eq $FALSE  ]; then 
           no_files_in_inventory $option
           return $? 
       else
           # If usr part is non-empty, don't blast it
           no_files_in_inventory $option || return $FALSE
           # If root is empty, try to blast it
           root_can_be_blasted $option && {
               # Get rid of the root part
               ODMDIR=/etc/objrepos
               blast_migrated_option $option
               ODMDIR=/usr/lib/objrepos
               # Now return TRUE so that U can be blasted
               return $TRUE 
           }
           return $FALSE  # doing U of UR fileset, don't blast
       fi
   fi

   # ROOT_ONLY_OP is set by installp 
   if [ "$ROOT_ONLY_OP" = "y" ]; then 
      no_files_in_inventory $option
      return $? 
   fi

   # This is the root of a typical usr-root case 

   # check the U part first 
   ODMDIR=/usr/lib/objrepos
   no_files_in_inventory $option
   if [ $? -ne $TRUE ]; then 
       ODMDIR=/etc/objrepos
       return $FALSE
   fi 

   ODMDIR=/etc/objrepos
   no_files_in_inventory $option
   return $? 
}

#----------------------------------------------------------------------
#
# Function    : Check if root part of the migrating option
#               can be removed during usr part processing 
# Arguments   : Option name
# Globals     : ODMDIR
#
# Return code : TRUE    If root can be removed 
#               FALSE   If root can't be removed 
#
#----------------------------------------------------------------------
#
root_can_be_blasted ()
{
   option=$1
   ODMDIR=/etc/objrepos
   no_files_in_inventory  $option || {
      ODMDIR=/usr/lib/objrepos
      return $FALSE
   }

   ODMDIR=/usr/lib/objrepos

   # If there is a namelist in the root part, then don't blast
   namelist_present_in_root && return $FALSE
   
   # No namelist, No files in root part
   return $TRUE
}

#----------------------------------------------------------------------
#
# Function    : Check if root part liblpp.a contains a namelist 
# Arguments   : None 
# Globals     : ODMDIR, OPTION
#
# Return code : TRUE    If namelist preset in liblpp.a 
#               FALSE   If namelist not preset in liblpp.a 
#
#----------------------------------------------------------------------
#
namelist_present_in_root ()
{ 
   
   # check if root liblpp.a is present
   [ ! -f inst_root/liblpp.a ] && return $FALSE

   ar tv inst_root/liblpp.a $OPTION.namelist 1>/dev/null 2>&1 
   return $?

}

#----------------------------------------------------------------------
#
# Function    : Check if the given option has any files associated
#               with it.
# Arguments   : Option name
# Globals     : ODMDIR
#
# Return code : TRUE    If no files are associated with the option
#               FALSE   Files are still associated with the option
#
# Note:  In 4.1 there could be options which don't ship any files.
#        In future if they do a 1-to-1 migration, it is OK.
#        If they do a 1-to-N migration, the assumption of having
#        completely migrated when the original option does not have
#        any files left with it fails.
#
#----------------------------------------------------------------------
#
no_files_in_inventory ()
{
   option=$1

   # get lpp_id
   LPP_ID=`odmget -q name=$option lpp | awk '/lpp_id/{print $3}'`

   # see if lpp_id has any files associated with it
   odmget -q lpp_id=$LPP_ID inventory 2>/dev/null | fgrep -q "inventory:"
   [[ $? -eq 0 ]]  && return $FALSE  # files exist
   return $TRUE 
}

#----------------------------------------------------------------------
#
# Function    : Check if the given fileset has a root part also 
# Arguments   : Option name
# Globals     : ODMDIR
#
# Return code : TRUE    If fileset has a root part
#               FALSE   If fileset has no root part 
#
#----------------------------------------------------------------------
#
usr_contains_root ()
{
   ODMDIR=/etc/objrepos odmget -qname=$option lpp 2>/dev/null |\
                                                       fgrep -q $option
   return $?
}

#----------------------------------------------------------------------
#
# Function    :  Move the files listed in OPTION.cfgfiles
#                to $MIGSAVE.
#
# Arguments   : Option name
#
# Globals     : MIGSAVE 
#
# Return code : TRUE    
#               FALSE  
#
#----------------------------------------------------------------------
inu_save_config ()
{
   # Is there anything to save?
   [[ ! -f $OPTION.cfgfiles ]] && return $TRUE 
   
   # Does $MIGSAVE dir exist?
   if [ ! -d $MIGSAVE ]; then
      mkdir -p $MIGSAVE || return $FALSE 
   fi

   # Is $MIGSAVE dir writable?
   [[ ! -w $MIGSAVE ]] &&  return $FALSE 

   # xargs does not handle multiple commands
   while read SAVEFILE junk; do
        SAVEFILE=/$SAVEFILE               # Ensure we have an absolute path

        [ ! -f $SAVEFILE ] && continue    # No file to save

        target_dir=`dname $SAVEFILE`

        if [ ! -d $MIGSAVE/$target_dir ]; then
           mkdir -p $MIGSAVE/$target_dir || return $FALSE 
        fi

        mv $SAVEFILE $MIGSAVE/$target_dir  || return $FALSE

   done < $OPTION.cfgfiles 

   return $TRUE
}

#----------------------------------------------------------------------
#
# Function    : Check if we are doing a 1-to-1 migration and set is_1to1_mig
#               to $TRUE if it is 1-to-1; unset is_1to1_mig otherwise.
#
# Arguments   : Option name
#
# Globals     : ODMDIR
#
# Return code : TRUE    
#               FALSE  
#
#----------------------------------------------------------------------
is_it_1to1_mig ()
{
   typeset -i nl=0
   is_1to1_mig=

   # if namelist, and installed_list don't exist, its not mig.
   [[ ! -f $OPTION.namelist || ! -f $BASEDIR/$OPTION.installed_list ]] \
        &&  return $FALSE

   # How many lines in namelist?
   nl=`wc -l $OPTION.namelist | awk '{print $1}'`

   #If > 1 not a 1to1 
   [[ $nl -ne 1 ]] && return $FALSE 

   # How many lines in installed_list?
   nl=`wc -l $BASEDIR/$OPTION.installed_list | awk '{print $1}'`

   # If > 1 not a 1to1 
   [[ $nl -ne 1 ]] && return $FALSE 

   # If the names are not the same in both lists, error
   read  OPTION_NAMELIST < $OPTION.namelist
   # if namelist had *only* bos.obj, don't do 1to1 !
   [[ "$OPTION_NAMELIST" = "bos.obj" ]] && return $FALSE 

   read  OPTION_INSTALLED_LIST junk < $BASEDIR/$OPTION.installed_list
   if [[ "$OPTION_NAMELIST" != "$OPTION_INSTALLED_LIST" ]]; then
      [[ "$OPTION" = "$OPTION_INSTALLED_LIST" ]] && return $FALSE
      inuumsg $INU_RMI_FAIL
      exit $INUCANCL
   fi
        
   is_1to1_mig=$TRUE
   return $TRUE
}

#----------------------------------------------------------------------
# Function    : Given an option name, gets the product name (or lppname)
# Arguments   : lpp_name of the prod record 
# Note        : In the product table "lpp_name" refers to the option name
#               and "name" refers to the product name (which is also referred
#               to as the lppname)
# Globals     : ODMDIR 
# Return code : None, but prints the name to stdout
#----------------------------------------------------------------------
get_lppname ()
{
   OPT=$1
   NAME=`odmget -q"lpp_name=$OPT and update=0" product | \
                 awk '($1 == "name") {print $3}' | sed 's/\"//g'`
   print $NAME
}

#-----------------------------------------------------------------------
# Function:    Remove empty directories based on the apply list
# Arguments:   AL apply_list name
# Global:      None
# Return Code: None
#-----------------------------------------------------------------------
rmdir_by_al ()
{
   LANG=C         # For sort     
   AL=$1
   [[ ! -s $AL ]] && return $TRUE
   (cd /
      sort -u -r $AL | xargs rmdir 2>/dev/null 
   )
   return 0
}


#-----------------------------------------------------------------------
# Function:    Remove empty directories based on the inventory file 
#
# Arguments:   INV -- inventory file names 
#
# Global:      None
#
# Return Code: None
#
#-----------------------------------------------------------------------
#
rmdir_by_inv ()
{
   LANG=C         # For sort     
   INV=$*
   for invfile in $INV; do
   (cd /
      awk -F':' '{ if ($0=/:$/) print $1 }' $invfile | sort -r | \
          xargs rmdir 2>/dev/null
   )
   done

   return 0
}

#-----------------------------------------------------------------------
# Main
#-----------------------------------------------------------------------

PATH=/usr/sbin:/usr/bin:/etc:/usr/lib/instl:bin:$PATH

typeset -i TRUE=0 \
           FALSE=1

INUCANCL=1 #Bad rc
INU_RMI_FAIL=148     
INU_PRE_RM_FAIL=143
INU_SAVECONFIG_FAIL=147

[[ $# != 3 ]]  && exit $INUCANCL

LPPNAME=$1
TYPE=$2
OPTION=
OPTIONLIST=

# Determine if we are given an option list, or a single option
# name to deal with
case $TYPE in 
    LPPOPTION) OPTION=$3;;        # Its a single option
   OPTIONLIST) OPTIONLIST=$3;;    # Its a list of options
            *) inuumsg $INU_RMI_FAIL
               exit $INUCANCL
esac

#-----------------------------------------------------------------
# NOTE:  The current directory is the LIBDIR for the part being
#        operated on.  The LIBDIR is
#
#        USR:   /usr/lpp/$LPPNAME
#        ROOT:  /usr/lpp/$LPPNAME/inst_root
#        SHARE: /usr/share/lpp/$LPPNAME
#
#-----------------------------------------------------------------
LIBDIR=`pwd`

case $INUTREE in
    M)  # ROOT 
        LPPDIR=/lpp 
        ;;
    U)  # USR
        LPPDIR=/usr/lpp 
        ;;
    S)  # SHARE
        LPPDIR=/usr/share/lpp 
        ;;
    *)  # ERROR - INUTREE MUST BE SET!
        inuumsg $INU_RMI_FAIL
        exit $INUCANCL
esac

# defined in swvpd.h
typeset -i LPP_MIGRATING_VALUE=16384

BASEDIR=$LPPDIR/$LPPNAME
MIGSAVE=$LPPDIR/save.config
DEINSTLDIR=$BASEDIR/deinstl

#------------------------------------------------
# Set REMOVE_TCB based on the TCB_STATE attribute
#------------------------------------------------
TCB_STATE=`ODMDIR=/usr/lib/objrepos \
           odmget -q attribute=TCB_STATE PdAt 2>/dev/null | \
           fgrep deflt | cut -d\" -f2`

# if tcb is not disabled (ie. either enabled, or corrupted)
if [ "$TCB_STATE" != "tcb_disabled" ]; then
   REMOVE_TCB=TRUE
else
   REMOVE_TCB=
fi


# If there's more than one OPTION to process...
if [[ $TYPE = OPTIONLIST ]]; then

   while read OPTION CURMOD CURFIX NEWMOD NEWFIX; do

      if [ -f $OPTION.pre_rm ]; then
         $OPTION.pre_rm  ||  abort_rminstal $INU_PRE_RM_FAIL 
      fi

      #-----------------------------------------------------------------
      # If the $OPTION.installed_list file exists then we are doing a
      # migration.
      #-----------------------------------------------------------------
      if [[ -f $BASEDIR/$OPTION.installed_list ]]; then
         inu_save_config  ||  abort_rminstal $INU_SAVECONFIG_FAIL

         #-----------------------------------------------------------------
         # The $OPTION.use_invfile file is created by inu_remove_opts.  If
         # it exists then an $OPTION.rm_inv also exists.
         #-----------------------------------------------------------------
         if [[ -f $BASEDIR/.$OPTION.use_invfile ]]; then
            mig_rm_install  ||  abort_rminstal 
            rm -f $BASEDIR/.$OPTION.use_invfile

         else

            is_it_1to1_mig  

            if [ "$is_1to1_mig" ]; then
               # Just change the OPTION to what it is currently
               # installed as if it is one-to-one
               read OPTION < $OPTION.namelist
               set_cp_flag $OPTION $LPP_MIGRATING_VALUE
            fi

            inu_rm_install >/dev/null 2>&1  ||  abort_rminstal 

            if [ "$is_1to1_mig" ]; then

               # For pure 1to1 migration blast the inventory, files 
               # and VPD records for the entry in installed_list
               # if it is present. 
            
               check_completely_migrated $OPTION && {
                  blast_migrated_option $OPTION

                  # blast usr part if necessary
                  if [ "$INUTREE" = "M" -a "$ROOT_ONLY_OP" != "y" ]; then  
                     ODMDIR=/usr/lib/objrepos
                     blast_migrated_option $OPTION
                     ODMDIR=/etc/objrepos
                  fi
               }
           fi
        fi

      else  # Normal installation, no migration
         inu_rm_install >/dev/null 2>&1  ||  abort_rminstal 
      fi

   done < $OPTIONLIST

else
   # $OPTION set above
   # Since this form of invocation is only applicable to deinstl/reject
   # migration work need not be done

   inu_rm_install >/dev/null 2>&1  ||  abort_rminstal 

fi

exit 0
