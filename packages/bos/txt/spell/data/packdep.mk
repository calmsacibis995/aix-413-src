# @(#)37        1.5  src/packages/bos/txt/spell/data/packdep.mk, pkgtxt, pkg411, GOLD410 6/4/94 12:33:49
#
# COMPONENT_NAME: pkgtxt
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

SHARE_LIBLPP_LIST	+= bos.txt.spell.data.namelist

SHARE_ODMADD_LIST	+=

SHARE_INFO_FILES	+= bos.txt.spell.data.supersede
