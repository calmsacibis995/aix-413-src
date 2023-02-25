# @(#)99	1.5  src/tenplus/interface/helper.in.S, tenplus, tenplus411, GOLD410 7/19/91 14:46:24
# These comment lines must be stripped out before this file is used.
#
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  9, 10, 27
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1988
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
int     Hinit           Hinit         char
void    Hafter          Hafter        int
int     Hbefore         Hbefore       int     int     char    int
void    Hdel            Hdel          char    int     int
void    Hins            Hins          char    int     int
int     Hmod            Hmod          char    int     char    char
int     Hrestart        Hrestart      obj
obj     Hsavestate      Hsavestate
void    Halarm          Halarm
