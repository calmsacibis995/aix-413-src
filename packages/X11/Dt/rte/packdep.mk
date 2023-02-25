# @(#)32        1.11  src/packages/X11/Dt/rte/packdep.mk, pkgxcde, pkg41J, 9520B_all  5/19/95  15:24:14
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: packaging definition
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# X11.Dt.rte packdep.mk
#

USR_LIBLPP_LIST		+= 	

ROOT_LIBLPP_LIST	+= X11.Dt.rte.config	\
			   X11.Dt.rte.cfgfiles  \
			   X11.Dt.rte.unconfig  \
			   X11.Dt.rte.post_i \
			   X11.Dt.rte.unpost_i \
                           X11.Dt.rte.pre_rm

USR_ODMADD_LIST		+= dtsmit.add%X11.Dt.rte
ROOT_ODMADD_LIST	+= 

INFO_FILES		+= X11.Dt.rte.prereq
