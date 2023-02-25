# @(#)14 1.1  src/packages/devices/mca/8f97/com/root/devices.mca.8f97.com.codepoint.S, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:31:18
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    325 : ssa.rte.obj.codepoint.S			#
#		    410 : devices.mca.8f97.com.codepoint.S		#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Stanza file for the errinstall command. It installs	#
#		    the mayflower adapter device driver's new error log	#
#		    messages.						#
#									#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when combined	#
#			 with the aggregate modules for	this project)	#
#									#
#   (C)	Copyright International	Business Machines Corp.	1995		#
#   All	rights reserved.						#
#									#
#########################################################################

#########################################################################
# Additions to the Error Description message set			#
#########################################################################

SET E
EC60 "LOSS OF REDUNDANT POWER/COOLING"
EC61 "ADAPTER PERFORMANCE DEGRADED"
EC62 "UNABLE TO LOG AN ERROR AGAINST A PDISK"
EC63 "ADAPTER DETECTED OPEN SERIAL LINK"

#########################################################################
# Additions to the Probable Cause message set				#
#########################################################################

SET P
EC2A "PDISK DELETED FROM SYSTEM CONFIGURATION"
EC2B "SSA DEVICE POWERED OFF OR FAILED"

#########################################################################
# Additions to the User Cause message set				#
#########################################################################

SET U
EC04 "ANOTHER SYSTEM ON THE LINK HAS BEEN POWERED OFF OR AN SSA DEVICE HAS BEEN REMOVED FROM THE LINK"

#########################################################################
# Additions to the Install Cause message set				#
#########################################################################

SET I
EB36 "CABLING OR POWER FAULT"

#########################################################################
# Additions to the Recommended Action message set			#
#########################################################################

SET R
EC18 "RUN CFGMGR TO RESTORE VOLUME TO THE SYSTEM CONFIGURATION"
EC19 "POWER ON THE OTHER SYSTEM OR REPLACE THE REMOVED DISK DRIVE"
EC1A "CHECK LINK CONFIGURATION"

