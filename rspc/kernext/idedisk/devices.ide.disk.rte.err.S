# @(#)77        1.2  src/rspc/kernext/idedisk/devices.ide.disk.rte.err.S, idedisk, rspc41J, 9510A 2/16/95 15:01:12
#
#	COMPONENT_NAME: IDEDISK
#
#	FUNCTIONS: codepoint description stanza file
#
#	ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
+ IDE_DISK_ERR1:
	Comment = "Physical volume media error"
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = 1312
	Prob_Causes = E855, 6330
	Fail_Causes = EA77, EA78, EA79, EA7A
	Fail_Actions = EC1B, 0301, 0000
	Detail_Data = 72, EC35, HEX

+ IDE_DISK_ERR2:
	Comment = "Physical volume software access error"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = 210F
	Prob_Causes = EA00, 6330
	Fail_Causes = 109B, EA7B
	Fail_Actions = EC1B, 0301, 0000
	Detail_Data = 72, EC35, HEX

+ IDE_DISK_ERR3:
	Comment = "Physical volume hardware error"
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = 1311
	Prob_Causes = 6330
	Fail_Causes = EA7C
	Fail_Actions = EC1B, 0301, 0000, 1804
	Detail_Data = 72, EC35, HEX

+ IDE_DISK_ERR4:
	Comment = "Physical volume recovered error"
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = EC64
	Prob_Causes = E855, 6330, EA00
	Fail_Causes = 5000, EA7B, EA7C
	Fail_Actions = 0700
	Detail_Data = 72, EC35, HEX

