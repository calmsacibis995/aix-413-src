* @(#)06        1.3  src/rspc/kernext/isa/ide/devices.isa_sio.PNP0600.rte.err.S, isaide, rspc41J, 9516A_all 4/17/95 14:03:47
*
*	COMPONENT_NAME: ATAIDE
*
*	FUNCTIONS: none
*
*	ORIGINS: ??
*
*		This file contains the error log templates for the ATA/IDE
*               adapter DD
*
+ ATAIDE_ERR1:
	Comment = "IDE ADAPTER RESET FAILURE"
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E98A
	Prob_Causes = E901, 3452, 6310, EA01
        User_Causes = 710F, 3401
        User_Actions = 0102, 0301, 011D, EA01
	Fail_Causes = 3400, 3303
	Fail_Actions = 0301, 0000
	Detail_Data = 56, EC35, HEX

+ ATAIDE_ERR2:
	Comment = "IDE DMA TRANSFER ERROR"
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = EB75
	Prob_Causes = E901, 3452, 6310, EA03
	Fail_Causes = 3400, 3303
	Fail_Actions = 0301, 0000
	Detail_Data = 56, EC35, HEX

+ ATAIDE_ERR3:
	Comment = "IDE DEVICE ERROR"
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 1002
	Prob_Causes = E901, 3452, 6310, EA01
        User_Causes = 3401
        User_Actions = 0102, 0301, 011D, EA01
	Fail_Causes = 3400, 3303
	Fail_Actions = 0301, 0000
	Detail_Data = 56, EC35, HEX

+ ATAIDE_ERR4:
	Comment = "IDE COMMAND TIMEOUT ERROR"
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E96B
	Prob_Causes = E901, 3452, 6310, EA01
        User_Causes = 3401
        User_Actions = 0102, 0301, 011D, EA01
	Fail_Causes = 3400, 3303
	Fail_Actions = 0301, 0000
	Detail_Data = 56, EC35, HEX

