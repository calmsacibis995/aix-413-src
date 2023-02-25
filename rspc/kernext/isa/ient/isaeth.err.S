#  @(#)68       1.1.1.1  src/rspc/kernext/isa/ient/isaeth.err.S, isaent, rspc41B, 412_41B_sync 1/6/95 15:46:47 
#
#   COMPONENT_NAME: ISAENT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
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
 
+ IEN_ISA_TMOUT:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "ISA Ethernet Adapter Command Timed Out"
    Err_Desc = 1010
    Prob_Causes = 3111, 0443, 3330
    Fail_Causes = 0403
    Fail_Actions = 0012, 0302, EA01, 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX

+ IEN_ISA_FAIL:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "ISA Ethernet Adapter Hardware Failure"
    Err_Desc = 1010
    Prob_Causes = 3302, E890, E89D
    Fail_Causes = 10C8, 10C7, 12B7, 12BB
    Fail_Actions = EA01, 0404, 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
 
+ IEN_ISA_PARITY:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "ISA Ethernet Adapter Parity Error"
    Err_Desc = 1010
    Prob_Causes = 3330, 2017, E911
    Fail_Causes = 3322, 2000, 2600, 3400 
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
 
+ IEN_ISA_CARD_INFO:
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "ISA Ethernet Adapter VPD Problem"
    Err_Desc = FE01
    Prob_Causes = 8000, 3331, 3302, 8008
    Fail_Causes = 1000, 1070, 2000
    Fail_Actions = 0001
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX

+ IEN_ISA_NOBUFS:
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "System Resource Problem"
    Err_Desc = FE01
    Prob_Causes = 8000, 0010, 0509
    Fail_Causes = 1000
    Fail_Actions = 0001
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
