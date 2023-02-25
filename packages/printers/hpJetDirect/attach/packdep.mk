# @(#)58        1.9  src/packages/printers/hpJetDirect/attach/packdep.mk, pkgdevprint, pkg411, GOLD410 2/7/94 18:28:53
#
# COMPONENT_NAME: pkgdevprint
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

INFO_FILES		+= printers.hpJetDirect.attach.insize \
			   printers.hpJetDirect.attach.prereq

USR_LIBLPP_LIST		+= printers.hpJetDirect.attach.config \
			   printers.hpJetDirect.attach.unconfig \
			   printers.hpJetDirect.attach.namelist \
			   printers.hpJetDirect.attach.rm_inv

USR_ODMADD_LIST		+= hpJetDirect.add%printers.hpJetDirect.attach
