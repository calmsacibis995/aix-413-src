# @(#)13 1.2  src/packages/devices/mca/8f97/com/root/devices.mca.8f97.com.err.S, mayaixpackaging, pkg41J, 9513A_all 3/8/95 12:54:20
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    325 : ssa.rte.obj.err.S				#
#		    410 : devices.mca.8f97.com.err.S			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Stanza file for the errupdate command. It installs	#
#		    the mayflower adapter device driver's error IDs.	#
#									#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when combined	#
#			 with the aggregate modules for	this project)	#
#									#
#   (C)	Copyright International	Business Machines Corp.	1995		#
#   All	rights reserved.						#
#									#
#########################################################################

+SSA_LINK_ERROR:
    Comment      = "SSA serial link failures"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = FALSE
    Err_type     = TEMP
    Err_Desc     = 3300
    Prob_Causes  = 3452
    Fail_Causes  = 3452,6310,3300
    Fail_Actions = 0000
    Detail_Data  = 156,0007,hex

+SSA_LINK_OPEN:
    Comment      = "SSA serial link open"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = FALSE
    Err_type     = PERM
    Err_Desc     = EC63
    Prob_Causes  = EC2B 
    User_Causes  = EC04 
    User_Actions = EC19
    Inst_Causes  = EB36
    Inst_Actions = EC1A
    Fail_Causes  = 3452,6310,3300
    Fail_Actions = 0000
    Detail_Data  = 156,0007,hex
 
+SSA_DETECTED_ERROR:
    Comment      = "SSA detected failures"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = FALSE
    Err_type     = PERM
    Err_Desc     = EC60
    Prob_Causes  = 0201,6310
    Fail_Causes  = 6310,0201,0204
    Fail_Actions = 0000
    Detail_Data  = 156,0007,hex

+SSA_DEVICE_ERROR:
    Comment      = "SSA device failures"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = TRUE
    Err_type     = PERM
    Err_Desc     = 1312
    Prob_Causes  = 6310
    Fail_Causes  = 6310
    Fail_Actions = 0000
    Detail_Data  = 156,0007,hex

+SSA_DEGRADED_ERROR:
    Comment      = "SSA Degraded Condition"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = FALSE
    Err_type     = PERF
    Err_Desc     = EC61
    Prob_Causes  = 3300
    Fail_Causes  = 3300
    Fail_Actions = 0000
    Detail_Data  = 156,0007,hex

+SSA_HDW_ERROR:
    Comment      = "SSA Hardware Error Condition"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = TRUE
    Err_type     = PERM
    Err_Desc     = 1010
    Prob_Causes  = 3300
    Fail_Causes  = 3300
    Fail_Actions = 0000
    Detail_Data  = 156,0007,hex

+SSA_HDW_RECOVERED:
    Comment      = "Recovered SSA Hardware Error"
    Class        = H
    Report       = TRUE
    Log          = TRUE
    Alert        = FALSE
    Err_type     = TEMP
    Err_Desc     = 1010
    Prob_Causes  = 3300
    Fail_Causes  = 3300
    Fail_Actions = 0000
    Detail_Data  = 156,0007,hex

+SSA_SOFTWARE_ERROR:
    Comment      = "SSA Software or microcode errors"
    Class        = S
    Report       = TRUE
    Log          = TRUE
    Alert        = TRUE
    Err_type     = PERM
    Err_Desc     = 2100
    Prob_Causes  = 1012,3331
    Inst_Causes  = 1500,1600
    Inst_Actions = 0104
    Fail_Causes  = 3300,6310
    Fail_Actions = 0000
    Detail_Data  = 156,0007,hex

+SSA_LOGGING_ERROR:
    Comment      = "Unable to log against a pdisk"
    Class        = S
    Report       = TRUE
    Log          = TRUE
    Alert        = FALSE
    Err_type     = PERM
    Err_Desc     = EC62 
    Prob_Causes  = EC2A
    User_Causes  = E802
    User_Actions = EC18
    Detail_Data  = 156,0007,hex

