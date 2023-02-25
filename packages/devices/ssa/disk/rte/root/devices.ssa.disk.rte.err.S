#@(#)27 1.2  src/packages/devices/ssa/disk/rte/root/devices.ssa.disk.rte.err.S, mayaixpackaging, pkg41J, 9513A_all 3/8/95 12:55:14
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    325 : ssadisk.obj.err.S				#
#		    410 : devices.ssa.disk.rte.err.S			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Stanza file for the errupdate command. It installs	#
#		    the SSA DASD device driver's error IDs.		#
#									#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when combined	#
#			 with the aggregate modules for	this project)	#
#									#
#   (C)	Copyright International	Business Machines Corp.	1995		#
#   All	rights reserved.						#
#									#
#########################################################################

+SSA_DISK_ERR1:
    Comment      = "DASD Detected Software Error"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = TRUE
    Err_type     = PERM
    Err_Desc     = 6100
    Prob_Causes  = 3331,1012
    Fail_Causes  = 3300,6310
    Fail_Actions = 0000
    Detail_Data  = 156,0011,hex

+SSA_DISK_ERR2:
    Comment      = "DASD statistical data"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = FALSE
    Err_type     = TEMP
    Err_Desc     = 1312
    Prob_Causes  = 6310
    Fail_Causes  = 6310
    Fail_Actions = 0700
    Detail_Data  = 156,0011,hex
 
+SSA_DISK_ERR3:
    Comment      = "Recovered SSA Disk Media Error"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = FALSE
    Err_type     = TEMP
    Err_Desc     = 1312
    Prob_Causes  = 5001
    Fail_Causes  = 5001
    Fail_Actions = 0000
    Detail_Data  = 156,0011,hex

+SSA_DISK_ERR4:
    Comment      = "Physical Volume Hardware Error"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = TRUE
    Err_type     = PERM
    Err_Desc     = 1312
    Prob_Causes  = 6310
    Fail_Causes  = 6310,6330
    Fail_Actions = 0000
    Detail_Data  = 156,0011,hex

