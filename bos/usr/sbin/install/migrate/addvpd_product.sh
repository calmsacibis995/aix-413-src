#!/bin/ksh
# @(#)20       1.5  src/bos/usr/sbin/install/migrate/addvpd_product.sh, cmdinstl, bos411, 9428A410j 4/1/94 17:21:24 
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
#   addvpd_product  <product> <product.option> <v.r.m.f>
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
# Function:    already_exists
#              Determine if an entry exists in lpp/prod databases
# Arguments:   product_option 
# Global:      None
# Return code: 
#              TRUE if prod_option of ANY level already exists
#              FALSE if not 
#
#------------------------------------------------------------------------
#
function already_exists 
{
   [ -z ${1} ] && return ${FALSE}    # argument is product option

   lppname=${1}

   odmget -q"lpp_name=${lppname} and state!=${ST_AVAILABLE}" product 2>/dev/null | \
                fgrep ${lppname}  >/dev/null 2>&1  && return ${TRUE}

   odmget -q"name=${lppname} and state!=${ST_AVAILABLE}" lpp 2>/dev/null | \
                fgrep ${lppname}  >/dev/null 2>&1  && return ${TRUE}

   return ${FALSE}  # FALSE is good
}

#------------------------------------------------------------------------
# Function:    blast_available_records 
# Arguments:   product_option 
# Global:      None
# Return code: None
#              rc of error on error
#
#------------------------------------------------------------------------
#
function blast_available_records
{
   [ -z ${1} ] && return ${FALSE}    # argument is product option
   lppname=${1}

   # blast product record
   odmdelete -o product -q"lpp_name=${lppname} and state=${ST_AVAILABLE} and \
              ver=${ver} and rel=${rel} and mod=${mod} and fix=${fix}" \
                >/dev/null 2>&1
   rc=${?}

   # blast lpp record
   odmdelete -o lpp -q"name=${lppname} and state=${ST_AVAILABLE} and \
              ver=${ver} and rel=${rel} and mod=${mod} and fix=${fix}" \
                >/dev/null 2>&1
   let rc=${rc}+${?}

   return ${rc}

}

#------------------------------------------------------------------------
# Function:    create_odm_entries
#              creates .add files for lpp, and prod data bases
# Arguments:   
# Global:      $PRODUCT_OPTION
#              $PRODUCT_NAME
#              $CP_FLAG
#              $TEMP_PROD_REC
#              $ST_AVAILABLE
#              $ST_COMMITTED
#              ver, rel, mod, fix
# Return code: 
#
#------------------------------------------------------------------------
#
function create_odm_entries
{
   # set the cp_flag first 
   if [ ${ODMDIR} = ${USR_VPD_PATH} ]; then
      let CP_FLAG=${CP_FLAG}+${LPP_USER}
   else   # gotta be share
      let CP_FLAG=${CP_FLAG}+${LPP_SHARE}
   fi

   # create .add file for the lpp database
   [ -f ${TEMP_LPP_REC} ] && rm  -f ${TEMP_LPP_REC}  >/dev/null 2>&1 

cat<<lpp_eof>${TEMP_LPP_REC}
lpp:
name=${PRODUCT_OPTION}
cp_flag=${CP_FLAG}
state=${ST_COMMITTED}
ver=${ver}
rel=${rel}
mod=${mod}
fix=${fix}
description="Compatibility entry"
lpp_eof

   rc=${?}
   if [ ${rc} -ne 0 ]; then
      dspmsg -s ${MS_INUCOMMON} ${CATALOG} ${ADEL_CREATE_REC_E} "${ADEL_CREATE_REC_E}" \
            ${cmdname}  "lpp"
      exit ${FAIL}
   fi

   [ -f ${TEMP_PROD_REC} ] && rm  -f ${TEMP_PROD_REC}  >/dev/null 2>&1 


   cat<<prod_eof>${TEMP_PROD_REC}
product:
lpp_name=${PRODUCT_OPTION}
update=${NOT_AN_UPDATE}
cp_flag=${CP_FLAG}
name=${PRODUCT_NAME} 
state=${ST_COMMITTED} 
ver=${ver}
rel=${rel}
mod=${mod}
fix=${fix}
prod_eof

   rc=${?} 
   if [ ${rc} -ne 0 ]; then
      dspmsg -s ${MS_INUCOMMON} ${CATALOG} ${ADEL_CREATE_REC_E} "${ADEL_CREATE_REC_E}" \
            ${cmdname}  "product"
      exit ${FAIL}
   fi

   return ${TRUE}
}

#------------------------------------------------------------------------
# Function:    add_odm_entry
# Arguments:   .add filename  
# Global:      ODMDIR
# Return code: 
#              TRUE on success
#              FALSE on failure 
#
#------------------------------------------------------------------------
#
function add_odm_entry
{
   [ -z ${1} ] && return ${FALSE}    # argument is .add file 
   add_file=${1}
   odmadd ${add_file} || return ${FALSE}
   rm -f ${add_file} >/dev/null 2>&1
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

# ----------------------- main () ----------------------------------------
#

cmdname=`basename ${0}`
PATH=/usr/sbin:/bin:usr/bin:/etc:$PATH

PRODUCT_NAME=
PRODUCT_OPTION=

# ST_AVAILABLE and ST_COMMITTED are defined in swvpd.h
# If the values ever change in swvpd.h, these need to be changed
# accordingly
#

typeset -i TRUE=1 \
           FALSE=0 \
           LPP_COMPAT=8192 \
           LPP_41_FMT=256 \
           CP_FLAG=0 \
           LPP_USER=1 \
           LPP_SHARE=4 \
           BASE_LEVEL=16 \
           ST_AVAILABLE=1 \
           ST_COMMITTED=5 \
           FAIL=1

TEMP_PROD_REC="/tmp/.prod.add"
TEMP_LPP_REC="/tmp/.lpp.add"
USR_VPD_PATH="/usr/lib/objrepos"
SHARE_VPD_PATH="/usr/share/lib/objrepos"
ver=
rel=
mod=
fix=

NOT_AN_UPDATE=0   # For the new update field in the product VPD table

#------------------------------------------------------------------------
# Messages.
# initialize message_stuff
#------------------------------------------------------------------------
CATALOG=cmdinstl_e.cat
MS_ADD_DEL_VPD=14
MS_INUCOMMON=3

ADEL_EXISTS_E=1
ADEL_ODMDIR_E=2
ADEL_ODMDIR_USR_SHARE_E=3
ADEL_USAGE_E=4
ADEL_CREATE_REC_E=5

CMN_ADDVPD_E=30
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

ADEL_CREATE_REC_D="\
0503-963 %s: Cannot create record for SWVPD file:  %s\n"
# %s is the name of the command.
# %s is the either lpp/product. 

CMN_ADDVPD_D="\
0503-026 %s:  Unable to add an entry to the Software Vital \n\
\tProduct Data database.\n"
# %s is the name of the command.

CMN_LEVEL_NUM_D="\
0503-028 %s:  %s is an invalid level. \n\
\tALL levels must be in one of the following formats: \n\
\t\tvv.rr.mmmm.ffff  OR  vv.rr.mmmm.fffff.ppppppp\n"
# The first %s is the name of the command.
# The second %s is the level number.

# End messages section
#------------------------------------------------------------------------

   # set cp_flag to COMPAT, and 41 format, andbase level
   let CP_FLAG=$LPP_COMPAT+$LPP_41_FMT+$BASE_LEVEL

   # check ODMDIR
   USR_or_SHARE
  
   # check arguments
   parse_command_line ${*}

   # Remove the AVAILABLE records if any
   blast_available_records ${PRODUCT_OPTION} 
   rc=${?} 
   # Could not delete available record -- but use a generic message
   if [ ${rc} -eq ${TRUE} ]; then
      dspmsg -s ${MS_ADD_DEL_VPD} ${CATALOG} ${ADEL_EXISTS_E} "${ADEL_EXISTS_D}"
 \
                        ${cmdname} "${PRODUCT_OPTION}"
      exit ${FAIL}
   fi


   # Does the prod record already exist?
   already_exists ${PRODUCT_OPTION}  
   rc=${?}
   if [ ${rc} -eq ${TRUE} ]; then 
      dspmsg -s ${MS_ADD_DEL_VPD} ${CATALOG} ${ADEL_EXISTS_E} "${ADEL_EXISTS_D}" \
                           ${cmdname} "${PRODUCT_OPTION}"  
      exit ${FAIL}
   fi

   # create the .add files
   create_odm_entries  
   rc=${?}
   if [ ${rc} -eq ${FALSE} ]; then
      dspmsg -s ${MS_INUCOMMON} ${CATALOG} ${ADEL_CREATE_REC_E} "${ADEL_CREATE_REC_E}" \
            ${cmdname}  
      exit ${FAIL}
   fi
   

   # add the vpd entry for compatibility  in the prod database
   add_odm_entry   ${TEMP_PROD_REC}
   rc=${?}
   if [ ${rc} -eq ${FALSE} ]; then
      dspmsg -s ${MS_INUCOMMON} ${CATALOG} ${CMN_ADDVPD_E} "${CMN_ADDVPD_D}" \
                                       ${cmdname}  
      exit ${FAIL}
   fi

   # add the vpd entry for compatibility in the lpp database
   add_odm_entry   ${TEMP_LPP_REC}
   rc=${?}
   if [ ${rc} -eq ${FALSE} ]; then
      dspmsg -s ${MS_INUCOMMON} ${CATALOG} ${CMN_ADDVPD_E} "${CMN_ADDVPD_D}" \
                                       ${cmdname}  
      exit ${FAIL}
   fi

   exit 0
