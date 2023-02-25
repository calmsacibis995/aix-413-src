# @(#)81	1.6  src/packages/bos/net/uucp/packdep.mk, pkguucp, pkg412, 9446B 11/15/94 16:39:43
#
# COMPONENT_NAME: pkguucp
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts or prereq files would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#
INFO_FILES		+= bos.net.uucp.insize \
			   bos.net.uucp.supersede

USR_LIBLPP_LIST		+= bos.net.uucp.rm_inv bos.net.uucp.namelist \
			   bos.net.uucp.cfgfiles

ROOT_LIBLPP_LIST	+= bos.net.uucp.cfgfiles bos.net.uucp.namelist \
			   bos.net.uucp.config bos.net.uucp.unconfig

ROOT_LIBLPP_UPDT_LIST	+= bos.net.uucp.config_u
