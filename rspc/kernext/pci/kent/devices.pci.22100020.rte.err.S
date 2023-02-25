# @(#)46  1.3 src/rspc/kernext/pci/kent/devices.pci.22100020.rte.err.S, pcient, rspc41J, 9519B_all  5/10/95  15:50:13
#
#   COMPONENT_NAME: PCIENT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
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
 
*
+ KENT_ADAP_ERR:
    Comment = "Adapter is failing to initialize"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM 
    Err_Desc = 1010
    Prob_Causes = 3330
    Fail_Causes = 3330 
    Fail_Actions = EB8D
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 140, EC38, HEX
    Detail_Data = 24, EC32, HEX
    Detail_Data = 6, 8191, HEX

+ KENT_ERROR_RCVRY:
    Comment = "PCI device driver has entered and exited network recovery mode"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = EC68
    Prob_Causes = 3330
    Fail_Causes = 3330
    Fail_Actions = E836, E825
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 140, EC38, HEX
    Detail_Data = 24, EC32, HEX
    Detail_Data = 6, 8191, HEX

+ KENT_TX_ERR:
    Comment = "A transmit error was detected"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 3309
    Prob_Causes = 0005, 3330
    Fail_Causes = 0005, 3330
    Fail_Actions = 0700, 3302, E836, E825
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 140, EC38, HEX
    Detail_Data = 24, EC32, HEX
    Detail_Data = 6, 8191, HEX

+ KENT_PIO:
    Comment = "The driver has detected a PIO error"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = EC66
    Prob_Causes = 0005, 3330
    Fail_Causes = 0005, 3330
    Fail_Actions = E836, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 140, EC38, HEX
    Detail_Data = 24, EC32, HEX
    Detail_Data = 6, 8191, HEX

+ KENT_DOWN:
    Comment = "device down due to unrecoverable error"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = EC67
    Prob_Causes = 3330, 0005
    Fail_Causes = 3330, 0005
    Fail_Actions = E836, E825
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 140, EC38, HEX
    Detail_Data = 24, EC32, HEX
    Detail_Data = 6, 8191, HEX
