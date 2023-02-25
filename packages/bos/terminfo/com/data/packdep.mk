# @(#)40	1.1  src/packages/bos/terminfo/com/data/packdep.mk, pkgterm, pkg411, GOLD410 6/10/94 12:37:44
#
# COMPONENT_NAME: pkgterm
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST contains the list of files that need to be added to
#       the respective liblpp.a other than the copyright, .al,
#       .tcb, .size, and .inventory files.  For example, any
#       post_i scripts would be included here.
#       The variable may be left blank if there are no additional
#       files, but the option will exist.
#

SHARE_LIBLPP_LIST         += bos.terminfo.com.data.namelist \
                             bos.terminfo.com.data.rm_inv

SHARE_ODMADD_LIST         +=
