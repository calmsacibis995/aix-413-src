# @(#)72	1.13  src/packages/bos/net/nfs/client/packdep.mk, pkgnfs, pkg411, 9438C411a 9/23/94 11:02:41
#
# COMPONENT_NAME: pkg
#
# FUNCTIONS: packaging definitions
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

ROOT_LIBLPP_LIST        += bos.net.nfs.client.pre_i \
                           bos.net.nfs.client.config \
                           bos.net.nfs.client.unconfig \
                           bos.net.nfs.client.cfgfiles \
                           bos.net.nfs.client.namelist \
                           bos.net.nfs.client.trc \
                           bos.net.nfs.client.rm_inv \
                           bos.net.nfs.client.post_i

USR_LIBLPP_LIST		+= bos.net.nfs.client.namelist \
                           bos.net.nfs.client.rm_inv \
                           bos.net.nfs.client.config \
                           bos.net.nfs.client.unconfig

USR_ODMADD_LIST		+= swapnfs.add%bos.net.nfs.client \
				sm_nfs.add%bos.net.nfs.client


INFO_FILES		+= bos.net.nfs.client.supersede \
				bos.net.nfs.client.prereq
