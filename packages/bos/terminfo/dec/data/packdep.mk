# @(#)03	1.5.1.1  src/packages/bos/terminfo/dec/data/packdep.mk, pkgterm, pkg41J, 9508A 2/2/95 16:28:11
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
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		post_i scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

SHARE_LIBLPP_LIST		+= bos.terminfo.dec.data.post_i \
                 		   bos.terminfo.dec.data.namelist \
                 		   bos.terminfo.dec.data.rm_inv

SHARE_INFO_FILES		+= bos.terminfo.dec.data.prereq \
                		   bos.terminfo.dec.data.insize
SHARE_LIBLPP_UPDT_LIST		+= bos.terminfo.dec.data.post_u
