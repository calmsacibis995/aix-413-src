# @(#)03 1.1  src/packages/devices/mca/8f97/diag/usr/devices.mca.8f97.diag.insize.S, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:29:59
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    devices.mca.8f97.diag.insize.S			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Declare the additional space required to do the	#
#		    root installation.					#
#		    Ask for an extra 10 512 byte blocks in the root	#
#		    part of our installation image - enough room for	#
#		    a backup copy of crontab.				#
#									#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when combined	#
#			 with the aggregate modules for	this project)	#
#									#
#   (C)	Copyright International	Business Machines Corp.	1995		#
#   All	rights reserved.						#
#									#
#########################################################################

/usr/lpp/devices.mca.8f97/inst_root +10 0
