# @(#)71  1.1  src/rspc/kernext/pcmcia/ssisa/pcm.err.S, pcmciaker, rspc411, 9433A411a 8/6/94 15:08:01
#
#   COMPONENT_NAME: PCMCIAKER
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
*    Err_Desc = "INVALID PCMCIA SOCKET SERVICE"
*    Prob_Causes = "PCMCIA CARD SERVICE"
*    Fail_Causes = "PCMCIA CARD SERVICE"
*    Fail_Actions = "CHECK PCMCIA SYSTEM CONFIGURATION"

+ PCMCIA_SS_BAD_FUNC:
    Comment = "Socket Service Bad Function"
    Class = S
    Err_Type = UNKN
	Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = 1000
    Prob_Causes = 1000
    Fail_Causes = 3434
    Fail_Actions = 1009
    Detail_Data = 128, A5, HEX

