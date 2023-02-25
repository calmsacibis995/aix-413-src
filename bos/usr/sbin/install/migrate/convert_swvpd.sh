#!/bin/ksh
#@(#)11       1.5  src/bos/usr/sbin/install/migrate/convert_swvpd.sh, cmdinstl, bos411, 9428A410j 7/1/94 17:26:57 
#
#   COMPONENT_NAME: CMDINSTL
#
#   FUNCTIONS: none
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
# USAGE:
#   convert_swvpd  <32_vpd_path> <41_vpd_path>
#
# ARGUMENTS:
#   32_vpd_path 
#   41_vpd_path 
#
# RETURN VALUE DESCRIPTION:
#   0 if everything succeeded
#   !0 if something failed; 
#
# Restrictions:
#
# Known limitations:
#

#------------------------------------------------------------------------
# Function:     usage        
# Remarks :     Print Usage message and exit
# Arguments:    none                           
# Return code:  None
#
#------------------------------------------------------------------------
#

function usage
{
   dspmsg -s $MS_CONVERT_VPD $CATALOG $CONVERT_USAGE_E "$CONVERT_USAGE_D" "$cmdname"

   exit ${FAIL}
}

#------------------------------------------------------------------------
# Function:     validate_args
# Remarks:      Validate arguments to the command
# Arguments:    all arguments to convert_swvpd
# Return code:  None
#
#------------------------------------------------------------------------
#
function validate_args 
{
   [ $# -ne 2 ] && usage
   
   src_vpd=${1}
   target_vpd=${2}
}

#------------------------------------------------------------------------
# Function:     add_32_to_41 
# Remarks:      Adds all .add stanzas to 4.1 vpd 
# Arguments:    None 
# Return code:  Exits on failure
#
#------------------------------------------------------------------------
#
function add_32_to_41
{
   for vpd in ${vpd_list}
     do
       # .add need not exist for share parts
       if [ ! -s ${target_vpd}/${vpd}.32.add ]; then
          [ ${target_vpd} = "/usr/share/lib/objrepos" ] && continue
          print "\n${cmdname}: ${target_vpd}/${vpd}.32.add does not exist"
          exit ${FAIL}
       fi

       ODMAPPEND=1 ODMDIR=${target_vpd} $ODMADD ${target_vpd}/${vpd}.32.add 
       rc=$?
       if [ rc -ne 0 ]; then
          dspmsg -s $MS_CONVERT_VPD $CATALOG $CONVERT_ADDVPD_E \
                    "$CONVERT_ADDVPD_D" "$cmdname"

          exit ${FAIL}
       fi
       # get rid of the .add files
       # 
       rm -f ${target_vpd}/${vpd}.32.add 
     done

  return ${PASS}
}

#------------------------------------------------------------------------
# Function:     vpd_is_32 
# Remarks:      Figure out if the given vpd is of 3.2 format 
# Arguments:    vpd_path
# Return code:  0 on TRUE
#               1 on FALSE   
#
#------------------------------------------------------------------------
#
function vpd_is_32
{
   # cp_flag was changed from short to long in 4.1
   ODMDIR=$1 odmshow lpp | grep -q "[ ]*short[ ]*cp_flag;"
   return $?
}


#------------------------------------------------------------------------
# Function:     blast_option
# Remarks:      Get rid of all entries for option
#               Used for bos.rte
# Arguments:    vpd_path
# Return code:  PASS 
#               FAIL 
#
#------------------------------------------------------------------------
#
function blast_option 
{

   [[ $# -ne 2 ]] && return $FALSE

   ODM=$1
   OPT=$2

   ODMDIR=$1
   ODMDIR=${ODM} LPP_ID=`odmget -q name=$OPT lpp | awk '/lpp_id/{print $3}'`
   [ -z "$LPP_ID" ]  && return $PASS

   # blast product record
   ODMDIR=${ODM} odmdelete -o product -qlpp_name=$OPT >/dev/null 2>&1  \
                                                        || return $FALSE

   # blast lpp record
   ODMDIR=${ODM} odmdelete -o lpp -qname=$OPT >/dev/null 2>&1  \
                                                || return $FALSE

   # blast history vpd 
   ODMDIR=${ODM} odmdelete -o history -qlpp_id=$LPP_ID >/dev/null 2>&1  \
                                                         || return $FALSE

   # blast inventory vpd 
   ODMDIR=${ODM} odmdelete -o inventory -qlpp_id=$LPP_ID >/dev/null 2>&1  \
                                                         || return $FALSE

   return $TRUE
}


# --------------------------- main () --------------------------------


cmdname=`basename ${0}`           # argv[0]
vpd_list="product lpp history inventory"
ODMADD="/usr/bin/odmadd"
PATH=/usr/sbin:/usr/bin:/usr/lpp/bos/migrate:/usr/lib/instl:/bin:/etc:$PATH
src_vpd=
target_vpd=
typeset -i FAIL=2 \
           PASS=0

#------------------------------------------------------------------------
# Messages.
# initialize message_stuff
#------------------------------------------------------------------------
CATALOG=cmdinstl_e.cat
MS_CONVERT_VPD=17

CONVERT_USAGE_E=1 
CONVERT_ADDVPD_D=2

#------------------------------------------------------------------------
# Default Messages.
#------------------------------------------------------------------------
#

CONVERT_USAGE_D="\
\tUsage: %s <32_vpd_path> <41_vpd_path>\n"
# %s is the name of the command.

CONVERT_ADDVPD_D="\
0503-970%s: Could not merge %s database\n"
# %s is the name of the command.
# %s is the name of the vpd.

#------------------------------------------------------------------------

   validate_args ${*}
   vpd_is_32 ${target_vpd}
   if [ ${?} -eq 0 ]; then
      print "Target vpd " ${target_vpd} "can not be of 3.2 format"
      exit $FAIL
   fi

   # Get the max lpp_id in target vpd to use to eliminate lpp_id conflicts
   ODMDIR=${target_vpd} t_lpp_max=`odmget -q "name=__SWVPD_CTL__" lpp |
    			                 grep "lpp_id " | awk '{print $3}'`
   if [ -z "$t_lpp_max" ]
   then
      t_lpp_max=0
   fi

   # Get the max lpp_id in source vpd to use to set next lpp_id after merge
   ODMDIR=${src_vpd} s_lpp_max=`odmget -q "name=__SWVPD_CTL__" lpp |
    			                 grep "lpp_id " | awk '{print $3}'`
   if [ -z "$s_lpp_max" ]
   then
      s_lpp_max=0
   fi

   vpd_is_32 ${src_vpd}
   if [ ${?} -eq 0 ]; then
      migrate_32_to_41 ${src_vpd} ${target_vpd} ${t_lpp_max} || exit $FAIL   
      add_32_to_41 || exit $FAIL   
   else
      # We don't have to merge the entries for options in the new vpd
      for newlpp in `ODMDIR=${target_vpd} odmget lpp |
    			                 grep "name =" | awk -F'"' '{print $2}'`
      do
          blast_option ${src_vpd} $newlpp || exit $FAIL
      done
      merge_41vpds ${src_vpd} ${target_vpd} ${t_lpp_max} || exit $FAIL   
   fi

   # Bump the lpp_id control stanza in the target
   ODMDIR=${target_vpd} odmget -q "name=__SWVPD_CTL__" lpp > ${src_vpd}/cntl.add
   cat ${src_vpd}/cntl.add | sed "s/lpp_id = $t_lpp_max/lpp_id = `expr $t_lpp_max + $s_lpp_max`/" > ${src_vpd}/cntl.new.add
   ODMDIR=${target_vpd} odmchange -o lpp -q "name=__SWVPD_CTL__" ${src_vpd}/cntl.new.add >/dev/null || exit $FAIL
   rm -f ${src_vpd}/cntl.new.add ${src_vpd}/cntl.add 2>/dev/null


   exit ${PASS}
