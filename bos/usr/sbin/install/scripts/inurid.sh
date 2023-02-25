#!/bin/ksh
# @(#)27	1.10  src/bos/usr/sbin/install/scripts/inurid.sh, cmdinstl, bos411, 9434A411a 8/17/94 16:16:00
#
#   COMPONENT_NAME: CMDINSTL
#
#   FUNCTIONS: Usage
#		is_BIRON_bit_set
#		on_exit
#		set_BIRON_to_yes
#		set_lock
#		unset_lock
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#---------------------------------------------------------------
#
# COMPONENT_NAME: cmdinstl
#
# NAME:           inurid
#
# FUNCTION:       
#                 Two main purposes:
#                 1. (-q flag) Indicates whether the BIRON variable 
#                    is set to "yes" or "no".
#                 2. (-r flag) Sets the BIRON variable to "yes", 
#                    and removes appropriate files.
#
# USAGE:
#                 inurid {-q | -r}
# ARGUMENTS:
#                 -q   (q)uery if BIRON is set or not
#                 -r   (r)emove inst_roots & others AND set BIRON 
#                      to "yes"
#
# RETURN VALUE DESCRIPTION:
#                 0        if everything succeeded
#                 nonzero  if something failed
#
#---------------------------------------------------------------

#---------------------------------------------------------------
#  Function Name:    give_msg
#
#  Purpose:          Displays a translated message to stderr.o
#                    via inuumsg.  If inuumsg fails, then this
#                    routine prints the english default version
#                    of the msg.
#
#  Arguments:        $1      -  catalog message number for inuumsg
#                    others  -  parameters for message
#
#---------------------------------------------------------------
give_msg()
{
   #---------------------------------------------------------------
   #  Attempt to give the translated version of the msg.  If that
   #  fails then give the default English message. 
   #---------------------------------------------------------------

   inuumsg $@

   if [ $? != 0 ]; then
      case $1 in
           $INU_BIRON_LOCK)
           echo "inurid:  Cannot remove the $BIRON_LOCK lock file."
           ;; 
           $INU_BIRON_ALREADY_RUN)
           echo "inurid:  The inurid command is already running.  If it"
           echo "\treally is not, remove the $BIRON_LOCK file."
           ;; 
           $INU_BIRON_CANT_CR)
           echo "inurid:  Cannot create the $BIRON_LOCK lock file."
           ;; 
           $INU_BIRON_VPD_ACCESS)
           echo "inurid:  Failed while attempting to access the SWVPD database."
           ;; 
           $INU_BIRON_SPOT_EXISTS)
           echo "inurid:  Cannot execute because a /usr diskless SPOT exists."
           ;; 
           $INU_BIRON_MUT_EX_FLAGS)
           echo "inurid:  The -q and -r flags are mutually exclusive."
           ;; 
           $INU_BIRON_Q_ONLY_ONCE)
           echo "inurid:  The -q flag may only be given once."
           ;; 
           $INU_BIRON_R_ONLY_ONCE)
           echo "inurid:  The -r flag may only be given once."
           ;; 
           $INU_BIRON_CANT_CD)
           shift
           echo "inurid:  Cannot change directory to $1"
           ;; 
           $INU_BIRON_IR_FILES)
           echo "inurid:  Failed cleaning up the inst_root files."
           ;; 
           $INU_BIRON_INSTALLP_RUNNING)
           echo "inurid:  The installp command is currently running.  The"
           echo "\tinurid command cannot execute while the installp command"
           echo "\tis running.\n"
           ;; 
           $INU_BIRON_USAGE)
           ${CAT} <<-eof
DEFAULT
           Usage:    inurid {-q | -r}
                       -q Query if the BIRON variable is set or not
                          returns 1 if BIRON is set to "yes" and
                                  0 if BIRON is set to "no"
                       -r Set the BIRON variable to "yes" and remove
                          appropriate files

           WARNING:  Use of this script will prevent your system from 
                     being used as a /usr SPOT or /usr server. 

                     This script will not change the BIRON variable
                     to "yes" if a /usr SPOT already exists. 
eof
           ;; 
           *)
           ;; 
      esac
   fi
}



#---------------------------------------------------------------
#  Function Name:  unset_lock
#  Purpose:        
#  Arguments:      None
#
#---------------------------------------------------------------
unset_lock()
{
   #---------------------------------------------------------------
   #  If the lock exists, remove it (complain if we cannot).
   #---------------------------------------------------------------
   if [ -f $BIRON_LOCK ]; then
      $RM -rf $BIRON_LOCK    2> /dev/null
      if [ $? -ne 0 ]; then
         give_msg $INU_BIRON_LOCK $BIRON_LOCK
         exit $FAIL_GENERAL;
      fi
   fi
}

#---------------------------------------------------------------
#  Function Name:  set_lock
#  Purpose:        
#  Arguments:      None
#
#---------------------------------------------------------------
set_lock()
{
   if [ -f $BIRON_LOCK ]; then
      give_msg $INU_BIRON_ALREADY_RUN $BIRON_LOCK
      exit $FAIL_GENERAL;
   fi

   INSTALLP_RUNNING=`$PS -ef 2> /dev/null | $GREP installp 2> /dev/null | \
                     $GREP -v grep 2> /dev/null | $WC -l`

   if [ $INSTALLP_RUNNING = 1 ]; then
      give_msg $INU_BIRON_INSTALLP_RUNNING
      exit $FAIL_GENERAL;
   fi

   $TOUCH $BIRON_LOCK    2> /dev/null
   if [ $? -ne 0 ]; then
      give_msg $INU_BIRON_CANT_CR $BIRON_LOCK
      exit $FAIL_GENERAL;
   fi
}

#---------------------------------------------------------------
#  Function Name:  on_exit 
#  Purpose:        Removes locks and exits w/ return code 
#                  passed in 
#  Arguments:      None
#
#---------------------------------------------------------------

on_exit()
{
   unset_lock   
   $CD /tmp
   $RM -rf $TEMP_DIR
   exit $1
}

#---------------------------------------------------------------
#  Function Name:  Usage
#  Purpose:        Prints the usage message to stdout 
#  Arguments:      None
#
#---------------------------------------------------------------

Usage()
{
   give_msg $INU_BIRON_USAGE
   on_exit $FAIL_GENERAL
}

#---------------------------------------------------------------
#  Function Name:  is_BIRON_bit_set
#
#  Purpose:        Determines if the BIRON bit is already set
#
#  Arguments:      None
#  
#  Returns:        0   if the BIRON variable is NOT set 
#                  1   if the BIRON variable IS set
#
#---------------------------------------------------------------

is_BIRON_bit_set()
{
  typeset -i CP_FLAG=0
  typeset -i BIRON=0
  typeset -i vpdrc=0
 
  CP_FLAG=`(ODMDIR=/usr/lib/objrepos $ODMGET -qname=$LPP_CTL_REC_NAME lpp \
            2> /dev/null;  echo $? > $TEMP_DIR/cpflag) | $FGREP cp_flag | \
            awk {'print $3'}`

  vpdrc=`$CAT $TEMP_DIR/cpflag`

  if [ $vpdrc -ne 0 ]; then
     give_msg $INU_BIRON_VPD_ACCESS 
     return 1 
  fi

  let BIRON="CP_FLAG&BIRON_BIT_VALUE"

  if [ $BIRON -eq 0 ] ; then
     return 0
  fi

  return 1
}

#---------------------------------------------------------------
#  Function Name:   set_BIRON_to_yes
#
#  Purpose:         Turns the BIRON variable "on" to "yes".
#
#  Arguments:       None
#
#---------------------------------------------------------------
set_BIRON_to_yes()
{
  typeset -i ORIG_CP_FLAG=0
  typeset -i NEW_CP_FLAG=0
  typeset -i BIRON=0
  typeset -i MASK=0
  typeset -i vpdrc=0

  #------------------------------------------------
  #  Get the cp_flag value of the BIRON record
  #------------------------------------------------

  ORIG_CP_FLAG=`(ODMDIR=/usr/lib/objrepos $ODMGET -qname=$LPP_CTL_REC_NAME lpp \
            2> /dev/null;  echo $? > $TEMP_DIR/cpflag) | $FGREP cp_flag | \
            awk {'print $3'}`

  vpdrc=`$CAT $TEMP_DIR/cpflag`

  if [ $vpdrc -ne 0 ]; then
     give_msg $INU_BIRON_VPD_ACCESS
     return 1 
  fi

  #--------------------------------------------------
  #  Make sure the BIRON bit is turned "on" 
  #--------------------------------------------------
  let NEW_CP_FLAG="ORIG_CP_FLAG|BIRON_BIT_VALUE"

  #------------------------------------------------
  #  Change the BIRON bit in SWVPD record ...
  #------------------------------------------------

  (ODMDIR=/usr/lib/objrepos odmget -qname=$LPP_CTL_REC_NAME lpp 2> /dev/null; \
                            echo $? > $TEMP_DIR/cpflag)    | 
   awk '{
   if (NF > 1)
     if ( $1 == "cp_flag")
        print "        " "cp_flag = " cpval
     else
        print "        " $1 " " $2 " " $3
   else
     print $1}' cpval=$NEW_CP_FLAG > $TEMP_DIR/odm.chg

  vpdrc=`$CAT $TEMP_DIR/cpflag`

  if [ $vpdrc -ne 0 ]; then
     give_msg $INU_BIRON_VPD_ACCESS
     return 1 
  fi

  (ODMDIR=/usr/lib/objrepos odmchange -o lpp -qname=$LPP_CTL_REC_NAME \
         $TEMP_DIR/odm.chg 2> /dev/null; echo $? > $TEMP_DIR/cpflag)

  vpdrc=`$CAT $TEMP_DIR/cpflag`

  if [ $vpdrc -ne 0 ]; then
     give_msg $INU_BIRON_VPD_ACCESS
     return 1 
  fi

  return 0 
}
#---------------------------------------------------------------
#  Function Name:  validate_environment
#
#  Purpose:        Ensures that a /usr SPOT doesn't exist.  If
#                  one does, we error off.
#
#  Arguments:      None
#
#---------------------------------------------------------------
validate_environment()
{
   #-------------------------------------------
   #  If a /usr SPOT already exists, error off
   #-------------------------------------------
   if [ -s $NIM_FILE ]; then
      $GREP "NIM_USR_SPOT" $NIM_FILE > /dev/null 2>&1      
      if [ $? -eq 0 ]; then
         give_msg $INU_BIRON_SPOT_EXISTS
         on_exit $FAIL_NIM_EXISTS
      fi
   fi
}

#---------------------------------------------------------------
#  Function Name:    set_format_and_type
#
#  Purpose:          Sets some environment variables:
#                      PKG_FORMAT
#                          3 -  3.2 pkgs
#                          4 -  4.1 pkgs
#                      PKG_TYPE
#                          I -  Installs
#                          U -  Updates
#                
#  Environment vars: CPFLAG - cp_flag value
#
#---------------------------------------------------------------

set_format_and_type ()
{
   #---------------------------------------------------------------
   # Bit definitions from swvpd.h
   #---------------------------------------------------------------

   INSTALL_BIT=16
   UPDATE_BIT=32
   F_32_BIT=128
   F_41_BIT=256

   PKT_TYPE=INVALID
   PKG_FORMAT=INVALID

   let I="$CPFLAG&$UPDATE_BIT"
   if [ $I != 0 ]; then
      PKG_TYPE=U
   fi

   let I="$CPFLAG&$INSTALL_BIT"
   if [ $I != 0 ]; then
      PKG_TYPE=I
   fi

   let T="$CPFLAG&F_32_BIT"
   if [ $T != 0 ]; then
      PKG_FORMAT=3
   fi
  
   let T="$CPFLAG&F_41_BIT"
   if [ $T != 0 ]; then
      PKG_FORMAT=4
   fi
}

#---------------------------------------------------------------
#  Function Name:   set_usr_libdir
#
#  Purpose:         Sets the USR_LIBDIR env var
#                  
#                   4.1 Install  (Same as 3.2 Installs)
#                      Usr    /usr/lpp/<prodname>
#
#                   4.1 Update
#                      Usr    /usr/lpp/<prodname>/<option>/<v.r.m.f>
#
#                   3.2 Update
#                      Usr    /usr/lpp/<prodname>/inst_<ptf>
#
#  Env vars used:   PRODUCT    -  Product name
#                   OPTION     -  Option name
#                   PKG_FORMAT -  Package format (3 ==> 3.2, 4 ==> 4.1)
#                   PKG_TYPE   -  Package Type (I ==> install, U ==> update)
#                   VER        -  Level (V.r.m.f)
#                   REL        -  Level (v.R.m.f)
#                   MOD        -  Level (v.r.M.f)
#                   FIX        -  Level (v.r.m.F)
#                   PTF        -  PTF ID
#
#---------------------------------------------------------------

set_usr_libdir ()
{
   BASE=/usr/lpp/$PRODUCT
   REST=

   if [ $PKG_TYPE = U ]; then
      if [ $PKG_FORMAT = 3 ]; then
         REST=inst_${PTF}
      else 
         if [ $PKG_FORMAT = 4 ]; then
            REST=$OPTION/$VER.$REL.$MOD.$FIX
         fi
      fi
   fi

   #---------------------------------------------------------------
   #  If an update, then add $REST to the $BASE
   #---------------------------------------------------------------

   if [ ! -z "$REST" ]; then
      USR_LIBDIR=${BASE}/${REST}
   else
      USR_LIBDIR=${BASE}
   fi
   
}

#---------------------------------------------------------------
#  Function Name:  can_we_remove_usr_libdir
#
#  Purpose:        Determines if the entire usr libdir can
#                  be removed.  
#
#  Returns:        TRUE  - if the usr libdir can be removed
#                  FALSE - if it cannot be removed
#
#  Env vars used:   PRODUCT    -  Product name
#                   PKG_TYPE   -  Package type (I = Install, U = Update)
#                   VER        -  Level (V.r.m.f)
#                   REL        -  Level (v.R.m.f)
#                   MOD        -  Level (v.r.M.f)
#                   FIX        -  Level (v.r.m.F)
#                   PTF        -  PTF ID
#
#---------------------------------------------------------------
can_we_remove_usr_libdir ()
{
   typeset -i vpdrc=0      # return code from vpd (odmget) query
   typeset -i rc=0         # return code from fgrep of query output

   # -------------------------------------------------------------------
   #  Don't remove usr libdir for an install (it'd also blast deinstl)
   # -------------------------------------------------------------------

   if [ $PKG_TYPE = I ]; then
      return
   fi

   # -------------------------------------------------------------
   #  If a 4.1 update, search based on
   #     - Product name
   #     - state = ST_COMMITTED
   #     - Entire level:
   #         ver = version
   #         rel = release
   #         mod = mod
   #         fix = fix
   # -------------------------------------------------------------

   if [ -z "$PTF" ]; then
      (ODMDIR=/usr/lib/objrepos  odmget -q"name=$PRODUCT and \
      ver=$VER and rel=$REL and mod=$MOD and fix=$FIX and \
      state != $ST_COMMITTED and state != $ST_AVAILABLE" product  \
      2> /dev/null; echo $? > $TEMP_DIR/cpflag)  |  $FGREP lpp_name \
      > /dev/null 2>&1

   # -------------------------------------------------------------
   #  Else if a 3.2 update, search based on
   #     - Product name
   #     - state = ST_COMMITTED
   #     - ptf = ptf id passed in
   # -------------------------------------------------------------

   else
      (ODMDIR=/usr/lib/objrepos odmget -q"name=$PRODUCT and ptf=$PTF and\
      state != $ST_COMMITTED and state != $ST_AVAILABLE" product \
      2> /dev/null; echo $? > $TEMP_DIR/cpflag)  |  $FGREP lpp_name \
      > /dev/null 2>&1
   fi

   rc=$?     # from fgrep 

   vpdrc=`$CAT $TEMP_DIR/cpflag`

   #-------------------------------------------------------
   #  If the odmget did not succeed  or
   #  if we found something for this product that is
   #     not in the committed or available state. 
   #-------------------------------------------------------

   if [ $vpdrc -ne 0  -o  $rc -eq 0 ]; then
      return $FALSE
   fi
   return $TRUE
}

#---------------------------------------------------------------
#  Function Name:  rm_inst_root_dir_if_can
#
#  Purpose:        Checks if the inst_root dir can be blasted.  
#                  If it can be blasted, then we make an 
#                  additional check to see if the parent dir
#                  can be blasted.  We'll opt for blasting the
#                  parent if we can.  If we cannot, though, 
#                  we'll settle for just blasting the inst_root.
#
#  Env vars used:  USR_LIBDIR -  Usr libdir
#       
#---------------------------------------------------------------
rm_inst_root_dir_if_can ()
{
   typeset rc=0
   typeset NUM_ALS=0


   ROOT_LIBDIR=${USR_LIBDIR}/inst_root

   # -------------------------------------------------------------
   #  If no apply lists exist in the inst_root dir, then 
   #  see what we can blast.   
   # -------------------------------------------------------------

   NUM_ALS=`$LS ${ROOT_LIBDIR}/*.al 2> /dev/null | $WC -l`

   if [ $NUM_ALS -eq 0 ]; then
      can_we_remove_usr_libdir 
      rc=$?
      $CD /tmp  ||  return
      if [ $rc -eq $TRUE ]; then
         $RM -rf $USR_LIBDIR
      else
         $RM -rf $ROOT_LIBDIR
      fi
   fi
}

#---------------------------------------------------------------
#  Function Name:  rm_inst_root_files
#
#  Purpose:        Blasts the delivered inst_root files as 
#                  specified in the root apply list.
#                  Also removes any empty directories that are
#                  children of the inst_root directory.
#
#---------------------------------------------------------------
rm_inst_root_files ()
{
   ROOT_LIBDIR=$USR_LIBDIR/inst_root 

   # ------------------------------------------------------
   #  remove the whole inst_root directory
   # ------------------------------------------------------

   $RM -fr $ROOT_LIBDIR 2> /dev/null
}

#---------------------------------------------------------------
#  Function Name:  remove_files_based_on_state
#
#  Purpose:        Removes: inst_root files for applied options,
#                  and additionaly the whole inst_root directory 
#                  for option that have everything in the 
#                  committed state.
#
#  Arguments:      $1  - state to remove files for 
#
#---------------------------------------------------------------
remove_files_based_on_state()
{
   typeset -i vpdrc=0

   STATE=$1
   NO_PTF_MARKER=NOT_A_PTF

   #------------------------------------------------------------
   #  Get list of all committed root parts
   #
   #  We're gonna have a pipe of the following format:
   #    odmget | awk | sort | tr | while 
   #  which basically gets the "prod-name option-name ptf-id"
   #  info from the SWVPD, sorts it, removes double-quotes and
   #  processes the whole thing.  
   #------------------------------------------------------------

   (ODMDIR=/etc/objrepos odmget -qstate=$STATE product 2> /dev/null; echo $? \
                         > $TEMP_DIR/cpflag) |  
      awk '{
         if ($1 == "cp_flag") 
            cpflag = $3;
         else if ($1 == "lpp_name") 
            on = $3;
         else if ($1=="name")
            pn = $3;
         else if ($1=="ver")
            ver = $3;
         else if ($1=="rel")
            rel = $3;
         else if ($1=="mod")
            mod = $3;
         else if ($1=="fix")
            fix = $3;
         else if ($1=="ptf")
         {
            printf ("%s %s %s %s %s %s %s %s\n", \
                     cpflag, pn, on, ver, rel, mod, fix, $3);
         }}'                     |    

      $SORT -u 2> /dev/null      |

      $TR \" ' '  2> /dev/null   |

      #------------------------------------------------------------------
      #  Blast the shipped inst_root files and/or the entire 
      #  inst_root directory for each root in the list.
      # 
      #  Format of input to while loop is (1 per line):
      #   cp_flag product-name option-name version release mod fix ptf
      #------------------------------------------------------------------

      while read CPFLAG PRODUCT OPTION VER REL MOD FIX PTF; do
         set_format_and_type

         set_usr_libdir 

         rm_inst_root_files 

         rm_inst_root_dir_if_can 
      done

  vpdrc=`$CAT $TEMP_DIR/cpflag`

  if [ $vpdrc -ne 0 ]; then
     give_msg $INU_BIRON_VPD_ACCESS
     return 1 
  fi
}

#---------------------------------------------------------------
#  Function Name:   remove_BIRON_files
#
#  Purpose:         The driver routine for routines that blast
#                   the BIRON-specific files.
#
#  Arguments:       None
#
#---------------------------------------------------------------
remove_BIRON_files()
{
   $RM -rf /tmp/inutmp*

   #---------------------------------------------------
   # remove all files for items in the COMMITTED state
   #---------------------------------------------------

   remove_files_based_on_state $ST_COMMITTED
   [ $? -ne  $SUCCESS ]  &&  return $FAIL_RM_FILES

   return $SUCCESS
}

#---------------------------------------------------------------
#  Function Name:  parse_cmdline
#
#  Purpose:        Parses the cmd line passed in
#
#  Arguments:      The entire cmd line given by user
#
#---------------------------------------------------------------
parse_cmdline()
{
   set -- $(${GETOPT} qr ${*})
   [ ${?} != 0 ] && Usage

   while [ $1 != -- ]; do
      case $1 in 
           #---------------------------------------------------
           # -q ==> query if BIRON is set or not
           #---------------------------------------------------

          -q) if [ $r_FLAG -eq $TRUE ]; then
                 give_msg $INU_BIRON_MUT_EX_FLAGS
                 Usage
              elif [ $q_FLAG -eq $TRUE ]; then
                 give_msg $INU_BIRON_Q_ONLY_ONCE
                 Usage
              fi
              q_FLAG=$TRUE
              shift
              ;;

           #---------------------------------------------------
           # -r ==> set BIRON to "yes" and remove files
           #---------------------------------------------------
          -r) if [ $q_FLAG -eq $TRUE ]; then
                 give_msg $INU_BIRON_MUT_EX_FLAGS
                 Usage
              elif [ $r_FLAG -eq $TRUE ]; then
                 give_msg $INU_BIRON_R_ONLY_ONCE
                 Usage
              fi
              r_FLAG=$TRUE
              shift
              ;;

           #---------------------------------------------------
           # All other flags/arguments are errors 
           #---------------------------------------------------
           *) Usage
              ;;
      esac 
   done

   if [ ${r_FLAG} -eq ${FALSE}  -a  ${q_FLAG} -eq ${FALSE} ]; then
      Usage
   fi

}




#---------------------------------------------------------------
#                 M A I N       P R O G R A M
#---------------------------------------------------------------

#----------------------------------
#  Initialization ...
#----------------------------------
TEMP_DIR=/tmp/BIRON_$$
LPP_CTL_REC_NAME="__SWVPD_CTL__"
NIM_FILE="/etc/niminfo"
PATH=/usr/bin:/usr/sbin/bin:$PATH
BIRON_LOCK="/usr/lpp/inurid_LOCK"
ST_AVAILABLE=1          # SWVPD state of available
ST_APPLIED=3            # SWVPD state of applied  
ST_COMMITTED=5          # SWVPD state of committed


#----------------------------------
#  inuumsg message numbers ... 
#----------------------------------
INU_BIRON_LOCK=119
INU_BIRON_ALREADY_RUN=120
INU_BIRON_CANT_CR=121
INU_BIRON_VPD_ACCESS=122
INU_BIRON_SPOT_EXISTS=123
INU_BIRON_MUT_EX_FLAGS=124
INU_BIRON_Q_ONLY_ONCE=125
INU_BIRON_R_ONLY_ONCE=126
INU_BIRON_CANT_CD=127
INU_BIRON_IR_FILES=128
INU_BIRON_USAGE=129
INU_BIRON_INSTALLP_RUNNING=130



#----------------------------------
#  Exit codes ...
#----------------------------------

typeset -i FALSE=0
typeset -i TRUE=1
typeset -i SUCCESS=0
typeset -i BIRON_IS_NOT_SET=0
typeset -i BIRON_IS_SET=1

typeset -i FAIL_SETTING_BIRON=5
typeset -i FAIL_NIM_EXISTS=6
typeset -i FAIL_BIRON_QUERY=7
typeset -i FAIL_RM_FILES=8
typeset -i FAIL_GENERAL=9
typeset -i q_FLAG=${FALSE}
typeset -i r_FLAG=${FALSE}
typeset -i BIRON_BIT_VALUE=1


CAT="/usr/bin/cat"
CD="cd"
FGREP="/usr/bin/fgrep"
FIND="/usr/bin/find"
GETOPT="/usr/bin/getopt"
GREP="/usr/bin/grep"
LS="/usr/bin/ls"
ODMGET="/usr/bin/odmget"
MKDIR="/usr/bin/mkdir"
PS="/usr/bin/ps"
RM="/usr/bin/rm"
RMDIR="/usr/bin/rmdir"
SORT="/usr/bin/sort"
TOUCH="/usr/bin/touch"
TR="/usr/bin/tr"
WC="/usr/bin/wc"
XARGS="/usr/bin/xargs"


#----------------------------------
#  Beginning of the action ... 
#----------------------------------

$RM -rf $TEMP_DIR
$MKDIR -p $TEMP_DIR 2> /dev/null
$CD $TEMP_DIR || give_msg $INU_BIRON_CANT_CD $TEMP_DIR

#----------------------------------
#  Process the cmd line 
#----------------------------------

parse_cmdline ${*}

#------------------------------------------------
#  Make sure a /usr SPOT does not already exist. 
#------------------------------------------------

validate_environment

#------------------------------------------------
#  Set locks so that installp can't run ...
#------------------------------------------------

if [ $r_FLAG -eq $TRUE ]; then
   set_lock
fi

#-----------------------------------------------
#  If -q flag was given, 
#    a. check if BIRON is set,
#    b. inform the user which it is, 
#    c. get out. 
#-----------------------------------------------

if [ $q_FLAG -eq $TRUE ]; then
   is_BIRON_bit_set
   rc=$?
   on_exit $rc 
fi

#--------------------------------------------------
#  If the -r flag was given, 
#    a. set BIRON to "yes",
#    b. remove appropriate files,
#    c. get out.
#--------------------------------------------------

if [ $r_FLAG -eq $TRUE ]; then
   set_BIRON_to_yes 
   rc=$?
   if [ $rc -ne 0 ]; then
      on_exit $FAIL_SETTING_BIRON
   fi

   remove_BIRON_files
   rc=$?
   if [ $rc -ne $SUCCESS ]; then
      give_msg $INU_BIRON_IR_FILES
      on_exit $FAIL_RM_FILES
   fi
   on_exit $SUCCESS
fi

on_exit $FAIL_GENERAL
