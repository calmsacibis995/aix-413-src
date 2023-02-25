# @(#)80	1.6  src/packages/bos/net/tcp/smit/packdep.mk, pkgtcpip, pkg411, 9440G411b 10/13/94 18:10:34
#
# COMPONENT_NAME: pkgtcpip
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

USR_LIBLPP_LIST		+= bos.net.tcp.smit.namelist \
			   bos.net.tcp.smit.rm_inv \
                           bos.net.tcp.smit.sm_tcpip1fix.odmdel

USR_ODMADD_LIST		+= sm_tcpip1.add%bos.net.tcp.smit \
                           sm_tcpip2.add%bos.net.tcp.smit

INFO_FILES		+= bos.net.tcp.smit.prereq
