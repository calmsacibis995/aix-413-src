# @(#)50	1.1  src/packages/internet_server/httpd/packdep.mk, pkgweb, pkg41J, 9516B_all 4/19/95 18:17:56
#
# COMPONENT_NAME: pkgweb
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
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
ROOT_LIBLPP_LIST	+= internet_server.httpd.post_i \
			   internet_server.httpd.config \
			   internet_server.httpd.unconfig

ROOT_LIBLPP_UPDT_LIST	+= internet_server.httpd.config_u
