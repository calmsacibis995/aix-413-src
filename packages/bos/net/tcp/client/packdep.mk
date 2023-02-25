# @(#)78        1.12  src/packages/bos/net/tcp/client/packdep.mk, pkgtcpip, pkg412, 9446B 11/15/94 16:43:06
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
INFO_FILES		+= bos.net.tcp.client.insize \
			   bos.net.tcp.client.supersede

USR_LIBLPP_LIST		+= bos.net.tcp.client.cfgfiles \
			   bos.net.tcp.client.namelist \
			   bos.net.tcp.client.rm_inv

ROOT_LIBLPP_LIST	+= bos.net.tcp.client.post_i \
			   bos.net.tcp.client.config \
			   bos.net.tcp.client.unconfig \
			   bos.net.tcp.client.cfgfiles \
			   bos.net.tcp.client.namelist \
			   bos.net.tcp.client.rm_inv \
			   bos.net.tcp.client.cfgrule.inet.odmdel \
			   bos.net.tcp.client.cfgrule.inet.unodmadd

ROOT_LIBLPP_UPDT_LIST	+= bos.net.tcp.client.config_u

USR_ODMADD_LIST	      += if_ca.add%bos.net.tcp.client \
			   if_en.add%bos.net.tcp.client \
			   if_fi.add%bos.net.tcp.client \
			   if_ie3.add%bos.net.tcp.client \
			   if_loop.add%bos.net.tcp.client \
			   if_sl.add%bos.net.tcp.client \
			   if_so.add%bos.net.tcp.client \
			   if_tr.add%bos.net.tcp.client \
			   if_x25.add%bos.net.tcp.client \
			   netinet.add%bos.net.tcp.client


ROOT_ODMADD_LIST        += cfgrule.inet.add%bos.net.tcp.client

