# @(#)60	1.1  src/rspc/kernext/isa/itok/isatok.err.S, isatok, rspc411, GOLD410 5/2/94 16:01:17
#
#   COMPONENT_NAME: ISATOK
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
*
* errids.desc
* error logging description file
*
* errupdate reads this file at system installation time
*  and fills in the ODM templates.
*
* The message corresponding to each codepoint is in codepoint.desc
*

+ ITOK_ADAP_OPEN:
    Comment = "hardware error opening adapter"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3211
    Prob_Causes = 3702
    Fail_Causes = 3434, 3711, 3320
    Fail_Actions = 1009, 3301, 2010, 3101
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX
 
+ ITOK_AUTO_RMV:
    Comment = "adapter auto-remove error"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3213
    Prob_Causes = 3702
    Fail_Causes = 3711, 3434, 3320
    Fail_Actions= 2010, 3101, 0105
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX
 
+ ITOK_BEACONING:
    Comment = "Ring beaconing"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3215
    Prob_Causes = 3703
    Fail_Causes = 3703
    Fail_Actions= 2010, 3101, 0105
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX
 
+ ITOK_DUP_ADDR:
    Comment = "duplicate station address"
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3211
    Prob_Causes = 3704
    Inst_Causes = 3704
    Inst_Actions= 2010, 3101
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX
 
+ ITOK_ESERR:
    Comment = "excessive Soft Errors for the ring"
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 4001
    Prob_Causes = 3703
    Fail_Causes = 3703, 1022
    Fail_Actions= 2010, 3101
    Detail_Data = 168, 11, HEX
 
+ ITOK_MEM_ERR:
    Comment = "memory allocation denied"
    Class = S
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = FE01
    Prob_Causes = 1022
    Fail_Causes = 1000
    Fail_Actions = 0000
    Detail_Data = 168, 11, HEX
 
+ ITOK_PERM_HW:
    Comment = "permanent adapter hardware error"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3211
    Prob_Causes = 3702, 3701, 3320
    Fail_Causes = 3712, 3701, 2600
    Fail_Actions= 2010, 3101, 1803
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX
 
+ ITOK_RCVRY_EXIT:
    Comment = "Successful exit of Network Recovery Mode"
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = A000
    Prob_Causes = FE00
    Fail_Causes = 3320
    Fail_Actions = 0100
    Detail_Data = 168, 11, HEX
 
+ ITOK_RCVRY_TERM:
    Comment = "Token-Ring has terminated Network Recovery Mode"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 1010
    Prob_Causes = 3323, 3320, 3330, 3331
    Fail_Causes = 3220, 3324
    Fail_Actions = 0302, 1206, 0000
    Detail_Data = 168, 11, HEX
 
+ ITOK_RING_SPEED:
    Comment = "beaconing during insertion process"
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3211
    Prob_Causes = 3703
    Fail_Causes = 3703
    Fail_Actions = 1009, 3301, 2010, 3101
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX
 
+ ITOK_RMV_ADAP:
    Comment = "remove adapter cmd received"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3211
    Prob_Causes = 3705
    User_Causes = 7101
    User_Actions= 2010, 3101
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX
 
+ ITOK_TX_ERR:
    Comment = "Token-Ring transmit error"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 1010
    Prob_Causes = 0005, 3220
    Fail_Causes = 0005, 3320
    Fail_Actions = 0000, 0400
    Detail_Data = 168, 11, HEX

+ ITOK_WIRE_FAULT:
    Comment = "wire-fault condition on the ring"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3212
    Prob_Causes = 3702
    Fail_Causes = 3711, 3434, 3320, 3400
    Fail_Actions= 0302, 2010, 3101, 0105
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX

+ ITOK_TX_TIMEOUT:
    Comment = "Token-Ring transmit timeout"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3707
    Prob_Causes = 3702
    Fail_Causes = 3711, 3434, 3320, 3400
    Fail_Actions= 0302, 2010, 3101, 0105
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX
