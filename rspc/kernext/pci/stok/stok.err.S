*
*   COMPONENT_NAME: pcitok -- Skyline Token Ring Devive Driver
*
*   FUNCTIONS: none
*
*   ORIGINS: 27
*
*
*   (C) COPYRIGHT International Business Machines Corp. 1995
*   All Rights Reserved
*   Licensed Materials - Property of IBM
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* /afs/austin/aix/project/aix41J/build/latest/src/bos/usr/bin/errlg/en_US
 
+ STOK_ADAP_CHECK:
    Comment = "Token-Ring adapter check"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3302
    Prob_Causes = 0443, 3330, 3331
    Fail_Causes = 3324
    Fail_Actions = 0000, 1803
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX

+ STOK_ADAP_OPEN:
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
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX

+ STOK_DMAFAIL:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "system/token ring adapter DMA error"
    Err_Desc = 1603
    Prob_Causes = 1020, 3322
    Fail_Causes = 0005, 0403, 0443
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX
 
+ STOK_BUS_ERR:
    Comment = "Token-Ring detected a bus error"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 1010
    Prob_Causes = 0005
    Fail_Causes = 0005
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX
 
+ STOK_DUP_ADDR:
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
 
+ STOK_MEM_ERR:
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
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX
 
+ STOK_PIO_ERR:
    Comment = "Token-Ring detected a PIO error"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 1010
    Prob_Causes = 0005, 3220
    Fail_Causes = 3320, 0005
    Fail_Actions = 0000, 0400
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX
 
+ STOK_RCVRY_EXIT:
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
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX
 
+ STOK_RING_SPEED:
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
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX
 
+ STOK_RMV_ADAP:
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
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX


+ STOK_WIRE_FAULT:
    Alert = TRUE
    Comment = "wire fault"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 3212
    Prob_Causes = 3702
    Fail_Causes = 3434, 3400, 3711, 3320
    Fail_Actions= 0302, 2010, 3101
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX

+ STOK_AUTO_RMV:
    Alert = TRUE
    Comment = "auto removal"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 3213
    Prob_Causes = 3320
    Fail_Causes = 3320, 3711, 3434
    Fail_Actions= 0000, 2010, 3101
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX
 
+ STOK_TX_TIMEOUT:
    Comment = "Token-Ring transmit timeout"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 3212
    Prob_Causes = 3702
    Fail_Causes = 3711, 3434, 3320, 3400
    Fail_Actions= 0302, 2010, 3101, 0105
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX

+ STOK_CTL_ERR:
    Comment = "Token-Ring ioctl timeout"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 1010
    Prob_Causes = 3702
    Fail_Causes = 3711, 3434, 3320, 3400
    Fail_Actions= 0302, 2010, 3101, 0105
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 14, 11, HEX

