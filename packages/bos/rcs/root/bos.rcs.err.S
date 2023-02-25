* @(#)18  1.2  src/packages/bos/rcs/root/bos.rcs.err.S, pkgrcs, pkg411, GOLD410 8/20/93 14:54:03
*
*   COMPONENT_NAME: RCS
*
*   FUNCTIONS: none
*
*   ORIGINS: 27
*
*   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*   combined with the aggregated modules for this product)
*                    SOURCE MATERIALS
*
*   (C) COPYRIGHT International Business Machines Corp. 1993
*   All Rights Reserved
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*

+ RCS_ERR_MSG_1:
	Comment = "ERROR CREATING SLIP CONTROLLER LOCKFILE"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC0A
	Prob_Causes = E90C, E90D, E90E
	User_Causes =  EB21
	User_Actions = 1205, E812, EB67

+ RCS_ERR_MSG_2:
	Comment = "UNABLE TO CREATE SLIP CONTROLLER PIPE"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC0B
	Prob_Causes = E90C, E90D
	User_Causes =  EB21
	User_Actions = 1205, E812, EB67

+ RCS_ERR_MSG_3:
	Comment = "UNABLE TO WRITE TO CLIENT PIPE"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC0C
	Prob_Causes = E90C
	User_Causes =  EB22
	User_Actions = 1205, E812

+ RCS_ERR_MSG_4:
	Comment = "UNKNOWN SLIP CONTROLLER ERROR"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC0D
	Prob_Causes = E90C
	Fail_Causes =  EB44
	Fail_Actions = EB61
	User_Causes =  EB22
	User_Actions = 1205, E812

+ RCS_ERR_MSG_5:
	Comment = "ERROR RETRIEVING LINE DISCIPLINE (TTY)"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC0E
	Prob_Causes = E90C
	User_Causes =  EB22
	User_Actions = 1205, E812 

+ RCS_ERR_MSG_6:
	Comment = "ERROR REMOVING A SLIP INTERFACE"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC0F
	Prob_Causes = E90C
	User_Causes = EB22
	User_Actions = 1205, E812 

+ RCS_ERR_MSG_7:
	Comment = "ERROR ON IFCONFIG OF A SLIP INTERFACE"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC10
	Prob_Causes = E90C, E90D, E90E
	User_Causes = EB21, EB22
	User_Actions = 1205, E812, EB67

+ RCS_ERR_MSG_8:
	Comment = "UNABLE TO CREATE A CHILD PROCESS"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC25
	Prob_Causes = E922
	Fail_Causes = EB22
	Fail_Actions = EB67 

+ RCS_ERR_MSG_9:
	Comment = "UNABLE TO DELETE RTS ON TTY PORT"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC23
	Prob_Causes = 0503, 1001, 8003
	Fail_Causes = F0A1, F0A2
	Fail_Actions = 1205, E812

+ RCS_FILE_IO_ERR:
	Comment = "FILE INPUT/OUTPUT ERROR"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = False
	Err_Type = UNKN
	Err_Desc = EC24
	Prob_Causes = E90C, E90D, E90E
	Fail_Causes = F0A1, F0A2
	Fail_Actions = EB61

