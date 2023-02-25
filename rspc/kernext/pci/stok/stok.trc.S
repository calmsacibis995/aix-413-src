#
#   COMPONENT_NAME: pcitok -- Skyline Token Ring Devive Driver
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
* /etc/trcfmt 

+32D 1.0 "@stokdd" \
        "STOK DD XMT: " O2 A4.8 "d1=" X4 " d2=" X4 " d3=" X4 

+32E 1.0 "@stokdd" \
        "STOK DD RCV: " O2 A4.8 "d1=" X4 " d2=" X4 " d3=" X4

+32F 1.0 "@stokdd" \
        "STOK DD ERR: " O2 A4.8 "d1=" X4 " d2=" X4 " d3=" X4

+338 1.0 "@stokdd" \
        "STOK DD OTHER: " O2 A4.8 "d1=" X4 " d2=" X4 " d3=" X4
