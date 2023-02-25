#!/bin/ksh
#
# @(#)72        1.1  bos.sysmgt.smit.post_i.sh, pkgsmit, pkg410 5/1/94 13:06:44
# COMPONENT_NAME: pkgdsmit                    
#
# FUNCTIONS: Deletes links to .add files for the install assistant
#	     if bos.sysmgt.smit is already installed.  That way
#	     the .add files won't get re-added when the user
#	     runs the installation assistant.
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------#

#delete install assistant stanzas from odm
/usr/lib/assist/assist_migrate

#rm links that were created in the post_i file
rm /usr/lib/assist/sm_instdev.add > /dev/null 2>&1
rm /usr/lib/assist/sm_migrate.add > /dev/null 2>&1
rm /usr/lib/assist/sm_overwrite.add > /dev/null 2>&1
rm /usr/lib/assist/sm_preinstall.add > /dev/null 2>&1
rm /usr/lib/assist/sm_preserve.add > /dev/null 2>&1
rm /usr/lib/assist/sm_isa.add > /dev/null 2>&1
