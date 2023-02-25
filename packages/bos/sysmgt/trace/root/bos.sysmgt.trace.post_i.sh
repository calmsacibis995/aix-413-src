# @(#)26        1.2  src/packages/bos/sysmgt/trace/root/bos.sysmgt.trace.post_i.sh, pkgtrc, pkg411, GOLD410 11/4/93 13:02:19
# 
# COMPONENT_NAME: pkgtrc
#
# FUNCTIONS: config  
#
# ORIGINS: 27 83
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
# 

# the error message numbers for inumsg
#
    eprog=36  # "program %s failed"

#============================================================#
#
# Do an error message and exit
#
# $1:  program name that caused failure (to supply to the error message)

error()
{
    inuumsg $eprog "uncompress /etc/trcfmt"
    exit 1
}

#==========================================================#
#
#    the c program handfmt uncompress the file /etc/trcfmt.Z
#=========================================================#


    /usr/lpp/bos.sysmgt/handfmt 1 && exit 0 || error handfmt

