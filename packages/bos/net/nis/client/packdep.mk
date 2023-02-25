# @(#)75        1.8  src/packages/bos/net/nis/client/packdep.mk, pkgnis, pkg411, 9438C411a 9/23/94 11:02:16
#
# COMPONENT_NAME: pkgnis
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

USR_LIBLPP_LIST		+= bos.net.nis.client.namelist \
                           bos.net.nis.client.rm_inv

ROOT_LIBLPP_LIST	+= bos.net.nis.client.config \
			   bos.net.nis.client.unconfig \
			   bos.net.nis.client.pre_i \
			   bos.net.nis.client.post_i \
                           bos.net.nis.client.namelist \
                           bos.net.nis.client.rm_inv \
                           bos.net.nis.client.cfgfiles

INFO_FILES		+= bos.net.nis.client.insize \
			   bos.net.nis.client.prereq

USR_ODMADD_LIST		+= sm_nis_client.add%bos.net.nis.client
