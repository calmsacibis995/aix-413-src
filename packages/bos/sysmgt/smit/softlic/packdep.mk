# @(#)59        1.1  src/packages/bos/sysmgt/smit/softlic/packdep.mk, pkgsoftlic, pkg411, 9439A411a 9/26/94 18:05:18
#
# COMPONENT_NAME: pkgsoftlic
#
# FUNCTIONS: packaging definition
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
INFO_FILES		+= bos.sysmgt.smit.softlic.prereq

ROOT_LIBLPP_LIST	+= bos.sysmgt.smit.softlic.post_i

USR_ODMADD_LIST		+= softlic.add%bos.sysmgt.smit.softlic
