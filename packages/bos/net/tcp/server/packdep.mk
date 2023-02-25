# @(#)79      1.5  src/packages/bos/net/tcp/server/packdep.mk, pkgtcpip, pkg411, GOLD410 5/24/94 22:18:45
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

USR_LIBLPP_LIST		+= bos.net.tcp.server.namelist \
			   bos.net.tcp.server.rm_inv \
			   bos.net.tcp.server.cfgfiles

ROOT_LIBLPP_LIST	+= bos.net.tcp.server.post_i \
			   bos.net.tcp.server.namelist \
			   bos.net.tcp.server.rm_inv \
			   bos.net.tcp.server.cfgfiles

USR_ODMADD_LIST		+=
ROOT_ODMADD_LIST	+=

INFO_FILES              += bos.net.tcp.server.prereq \
			   bos.net.tcp.server.supersede
