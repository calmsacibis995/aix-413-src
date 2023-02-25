# @(#)94        1.8  src/packages/bos/sysmgt/serv_aid/packdep.mk, pkgserv_aid, pkg41B, 9504A 12/13/94 14:01:40
#
# COMPONENT_NAME: pkgserv_aid
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

USR_LIBLPP_LIST		+= bos.sysmgt.serv_aid.namelist
ROOT_LIBLPP_LIST	+= bos.sysmgt.serv_aid.post_i \
				bos.sysmgt.serv_aid.unpost_i \
				bos.sysmgt.serv_aid.config \
				bos.sysmgt.serv_aid.unconfig \
				bos.sysmgt.serv_aid.err

USR_ODMADD_LIST		+= errlg.add%bos.sysmgt.serv_aid \
			   dump.add%bos.sysmgt.serv_aid
ROOT_ODMADD_LIST	+= logsymptom.add%bos.sysmgt.serv_aid

