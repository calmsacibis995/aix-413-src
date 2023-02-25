#!/bin/ksh
# @(#)21       1.4  src/bos/usr/sbin/install/migrate/delvpd_product.sh, cmdinstl, bos411, 9428A410j 4/1/94 17:21:41
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
#   delvpd_product  <product> <product.option> <v.r.m.f>
#
# ARGUMENTS:
#   product name          
#   product.option name  
#   v.r.m.f              
#
#  Conditions for adding the entry:
#      The product option record is either not in the VPD
#      or is in the AVAILABLE state
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
# Function:     USR_or_SHARE
# Arguments:    None
# global:       $ODMDIR 
# Return code:  None
#
#------------------------------------------------------------------------
#

function USR_or_SHARE
{
   if [ "x${ODMDIR}" = "x" ]; then 
      dspmsg -s $MS_ADD_DEL_VPD $CATALOG $ADEL_ODMDIR_E "$ADEL_ODMDIR_D" "$cmdname"  
      exit ${FAIL} 
   fi
   
   if [ ${ODMDIR} != ${USR_VPD_PATH} -a ${ODMDIR} != ${SHARE_VPD_PATH} ]; then 
      dspmsg -s ${MS_ADD_DEL_VPD} ${CATALOG} ${ADEL_ODMDIR_USR_SHARE_E} \
                   "${ADEL_ODMDIR_USR_SHARE_D}" ${cmdname} "${ODMDIR}" 
      exit ${FAIL} 
   fi
}

#------------------------------------------------------------------------
# Function:   parse_command_line 
# Arguments:  All the arguments passed to the command 
# Global:     None
# Return code: rc of error on error 
#
#------------------------------------------------------------------------
#
function parse_command_line
{
   if [ ${#} -ne 3 ]; then
      dspmsg -s ${MS_ADD_DEL_VPD} ${CATALOG} ${ADEL_USAGE_E} "${ADEL_USAGE_D}" \
                                         ${cmdname}  
      exit ${FAIL}
   fi

   PRODUCT_NAME=${1}
   PRODUCT_OPTION=${2}
   print ${3} | awk -F. '{print $1 " " $2" "  $3" " $4" " }' | read ver rel mod fix

   if [ "x${ver}" = "x" -o "x${rel}" = "x" -o "x${mod}" = "x" \
                                        -o "x${fix}" = "x" ]; then 
      dspmsg -s ${MS_INUCOMMON} ${CATALOG} ${CMN_LEVEL_NUM_E} \
                              "${CMN_LEVEL_NUM_D}" ${cmdname} "${3}" 
      exit ${FAIL}
   fi
}

#------------------------------------------------------------------------
# Function:    is_it_committed
#              Check if product is committed in lpp & prod db.
# Arguments:   None 
# Global:      product_option ver rel mod fix cp_flag
# Return code: 
#              TRUE if prod_option is committed 
#              FALSE if not 
#
#------------------------------------------------------------------------
#
function is_it_committed
{
   #Check product db
   odmget -q"lpp_name=${PRODUCT_OPTION} and state=${ST_COMMITTED} \
             and ver=${ver} and rel=${rel} and mod=${mod} and fix=${fix} \
             and cp_flag=${CP_FLAG}" product 2>/dev/null | \
             fgrep ${PRODUCT_OPTION}  >/dev/null 2>&1  || return ${FALSE}

   #Check lpp db
   odmget -q"name=${PRODUCT_OPTION} and state=${ST_COMMITTED} \
             and ver=${ver} and rel=${rel} and mod=${mod} and fix=${fix} \
             and cp_flag=${CP_FLAG}" lpp 2>/dev/null | \
             fgrep ${PRODUCT_OPTION}  >/dev/null 2>&1  || return ${FALSE}

   return ${TRUE}
}

#------------------------------------------------------------------------
# Function:    del_odm_entries
# Arguments:   none 
# Global:      product_option ver rel mod fix cp_flag
# Return code: 
#              TRUE on success
#              FALSE on failure 
#
#------------------------------------------------------------------------
#
function del_odm_entries
{

   odmdelete -o product -q"lpp_name=${PRODUCT_OPTION} and state=${ST_COMMITTED} and \
              ver=${ver} and rel=${rel} and mod=${mod} and fix=${fix} \
              and cp_flag=${CP_FLAG}" >/dev/null 2>&1 || return ${FALSE}
   

   odmdelete -o lpp -q"name=${PRODUCT_OPTION} and state=${ST_COMMITTED} and \
              ver=${ver} and rel=${rel} and mod=${mod} and fix=${fix} \
              and cp_flag=${CP_FLAG}" >/dev/null 2>&1 || return ${FALSE}
  
   # see if odmdelete succeeded
   #
   is_it_committed
   rc=${?}
   [ ${rc} -eq ${TRUE} ] && return ${FALSE} # delete failed

   return ${TRUE}
}

#------------------------------------------------------------------------
# Function:    error
# Arguments:   Error message 
# Global:      None
# Return code: 1 
#
#------------------------------------------------------------------------
#
function error 
{
  dspmsg -s ${*}
  exit 1
}

#------------------------------------------------------------------------
# Function:    calculate_cp_flag 
# Arguments:   None
# Global:      CP_FLAG 
# Return code:
#              TRUE 
#              FALSE 
#
#------------------------------------------------------------------------
#
function calculate_cp_flag 
{


   if [ ${ODMDIR} = ${USR_VPD_PATH} ]; then
      let CP_FLAG=${CP_FLAG}+${LPP_USER}
   else                                     # gotta be share
      let CP_FLAG=${CP_FLAG}+${LPP_SHARE}
   fi

   return ${TRUE}  # FALSE is bad
}

# ----------------------- main () ----------------------------------------
#

cmdname=`basename ${0}`
PATH=/usr/sbin:/bin:usr/bin:/etc:$PATH

PRODUCT_NAME=
PRODUCT_OPTION=

# ST_AVAILABLE, and ST_COMMITTED are defined in swvpd.h
# Ther values should be changed if they change in the header file
typeset -i TRUE=1 \
           FALSE=0 \
           ST_AVAILABLE=1 \
           ST_COMMITTED=5 \
           LPP_COMPAT=8192 \
           BASE_LEVEL=16 \
           LPP_41_FMT=256 \
           CP_FLAG=0 \
           LPP_USER=1 \
           LPP_SHARE=4 \
           FAIL=1

USR_VPD_PATH="/usr/lib/objrepos"
SHARE_VPD_PATH="/usr/share/lib/objrepos"
ver=
rel=
mod=
fix=

#------------------------------------------------------------------------
# Messages.
# initialize message_stuff
#------------------------------------------------------------------------
CATALOG=cmdinstl_e.cat
MS_ADD_DEL_VPD=14
MS_INUCOMMON=3

ADEL_ODMDIR_E=2
ADEL_ODMDIR_USR_SHARE_E=3
ADEL_USAGE_E=4
ADEL_NOT_COMPAT_ENTRY_E=6

CMN_DELVPD_E=60
CMN_LEVEL_NUM_E=32

#------------------------------------------------------------------------
# Default Messages.
# initialize message_stuff
#------------------------------------------------------------------------
#
ADEL_EXISTS_D="\
0503-960 %s: %s already exists\n"
# First %s is the name of the command, second %s is the fileset name

ADEL_ODMDIR_D="\
0503-961 %s: ODMDIR not set\n"
# %s is the name of the command.

ADEL_ODMDIR_USR_SHARE_D="\
0503-962 %s: ODMDIR should be set to USR or SHARE instead of %s\n"
# First %s is the name of the command, second %s is ODMDIR value.

ADEL_USAGE_D="\
\tUsage: %s package fileset v.r.m.f\n"
# %s is the name of the command.

ADEL_NOT_COMPAT_ENTRY_D="\
0503-964 %s: %s is not a compatability entry or is missing.\n"
# First %s is the name of the command, second %s is the product option.

CMN_DELVPD_D="\
0503-046 %s:  Unable to delete an entry from the Software Vital \n\
\tProduct Data database.\n"
# %s is the name of the command.

CMN_LEVEL_NUM_D="\
0503-028 %s:  %s is an invalid level. \n\
\tALL levels must be in one of the following formats: \n\
\t\tvv.rr.mmmm.ffff  OR  vv.rr.mmmm.fffff.ppppppppp\n"
# The first %s is the name of the command.
# The second %s is the level number.

# End messages section
#------------------------------------------------------------------------

   # set cp_flag to COMPAT, and 41 format, and base level
   let CP_FLAG=$LPP_COMPAT+$LPP_41_FMT+$BASE_LEVEL

   # check ODMDIR
   USR_or_SHARE
  
   # check arguments
   parse_command_line ${*}

   #calculate the cp_flag value based on ODMDIR
   calculate_cp_flag

   # Does the prod record already exist?
   is_it_committed 
   rc=${?}
   if [ ${rc} -eq ${FALSE} ]; then 
      dspmsg -s ${MS_ADD_DEL_VPD} ${CATALOG} ${ADEL_NOT_COMPAT_ENTRY_E} \
                 "${ADEL_NOT_COMPAT_ENTRY_D}" ${cmdname} "${PRODUCT_OPTION}"  
      exit ${FAIL}
   fi

   # delete the vpd entries that were made for for compatibility 
   del_odm_entries   
   rc=${?}
   if [ ${rc} -eq ${FALSE} ]; then
      dspmsg -s ${MS_INUCOMMON} ${CATALOG} ${CMN_DELVPD_E} "${CMN_DELVPD_D}" \
                                       ${cmdname}  
      exit ${FAIL}
   fi

   exit 0
