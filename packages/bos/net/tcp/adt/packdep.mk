# @(#)31        1.4  src/packages/bos/net/tcp/adt/packdep.mk, pkgtcpip, pkg411, GOLD410 5/31/94 15:31:01
#
# COMPONENT_NAME: pkgtcpip
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
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= bos.net.tcp.adt.namelist \
			   bos.net.tcp.adt.rm_inv

INFO_FILES		+= bos.net.tcp.adt.prereq
			   
