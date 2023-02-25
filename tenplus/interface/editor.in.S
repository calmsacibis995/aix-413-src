# @(#)02	1.7  src/tenplus/interface/editor.in.S, tenplus, tenplus411, GOLD410 3/23/93 12:02:42
# These comment lines must be stripped out before this file is used.
#
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  9, 10, 27
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
void    Eoldhelper      Eoldhelper    char
char    Eask            Iask          int     char
void    Ebell           Ebell 
void    Echangepfx      Echangepfx    int     char
void    Eclrhook        Eclrhook      char    char
int     Ecolumn         Ecolumn 
int     Edelline        Edelline      char    int     int
void    Edisplay        Edisplay 
void    Edispname       Edispname     char
void    Eerror          Ierror        int     char
void    Efatal          Ifatal        int     char
char    Efieldname      Efieldname 
char    Efilename       Efilename 
void    Eflushform      Eflushform    char
char    Eformname       Eformname 
int     Egetflags       Egetflags     char
obj     Egetlist        Egetlist      char    int     int
char    Egetpath        Egetpath 
char    Egetprefix      Egetprefix    char
char    Egetsuffix      Egetsuffix    char
char    Egettext        Egettext      char    int
int     Einsline        Einsline      char    int     int
int     Ekeyvalue       Ekeyvalue 
void    Ekillhelper     Ekillhelper 
int     Eline           Eline 
int     Emenu           Emenu         int     char    int      char
void    Emessage        Imessage      int     char
int     Enumlines       Enumlines     char
int     Enumsiblings    Enumsiblings 
int     Eputcursor      Eputcursor    char    int     int
int     Eputformat      Eputformat    char    char
int     Eputlist        Eputlist      char    int     obj
int     Eputtext        Eputtext      char    int     char
int     Ereadonly       Ereadonly 
void    Ereopen         Ereopen 
void    Ermfile         Ermfile       char
void    Esaveascii      Esaveascii    char
int     Esavefile       Esavefile 
void    Esethook        Esethook      char    char
void    Esetpath        Esetpath      char    char
char    Esync           Esync 
void    Ettyfix         Ettyfix 
void    Ettyunfix       Ettyunfix 
int     Eusefile        Eusefile      char
int     Euseform        Euseform      char
int     Ewritefield     Ewritefield   char    char
int     Econfirm        Iconfirm      int     char
void    Ecallfun        Ecallfun      int     int     char    int
void    Eflushhelper    Eflushhelper  char
char    Egetmessage     Egetmessage   int     char    int
int     Ecountlines     Ecountlines   char    int
int     Ecopyprofile    Ecopyprofile  char    char
int     Esizelines      Esizelines    char    int     
int     Ecustommenu     Ecustommenu   int     char    obj    int
char    Efixvars        Efixvars      char
int     Esetalarm       Esetalarm     int
int     Eclearalarm     Eclearalarm
int     Escrnio         Escrnio       char
void    Ediskfull       Ediskfull     char
int     Eemptyrecord    Eemptyrecord 
int     Egetttline      Egetttline    int
int     Egettbline      Egettbline    int
