# @(#)47        1.2  src/packages/bos/sysmgt/trace/root/bos.sysmgt.trace.unpost_i.sh, pkgtrc, pkg411, GOLD410 11/4/93 13:02:29
# COMPONENT_NAME: pkgtrc
#
# FUNCTIONS:  unconfig
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
    inuumsg $eprog "compress /etc/trcfmt"
    exit 1
}

#==========================================================#
#
#    the c program handfmt compress the file /etc/trcfmt
#=========================================================#


    /usr/lpp/bos.sysmgt/handfmt 0 && exit 0 || error handfmt


