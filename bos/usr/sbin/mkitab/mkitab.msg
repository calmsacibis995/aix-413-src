$ @(#)54	1.4  src/bos/usr/sbin/mkitab/mkitab.msg, cmdoper, bos411, 9428A410j 2/2/91 13:02:15
$
$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs  
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989  
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$quote "         define quote character.

$len 
$set MS_MKITAB        Define initial set#
$ Message numbers have been assigned a range of 201-218
$ Do not move the first %s of each command line 

E_CMD                "%s: 0481-201 Command field is too long.\n"

E_CREATE             "%s: 0481-202 Cannot create temporary file.\n"

E_FIELD              "%s: 0481-203 Cannot specify more than one parameter.\n"

E_IDENT              "%s: 0481-204 The ident field is not correct.\n"
$ Do not translate ident  

E_IDEXIST            "%1$s: 0481-205 The ident entry already exists in %2$s.\n"

E_KEYW               "%s: 0481-206 The keyword field is too long.\n"

E_NAME               "The %s command is not recognized.\n"

E_NOIDENT            "%s: 0481-207 The ident field specified does not exist.\n"

E_OPEN               "%1$s: 0481-208 Cannot open %2$s.\n"

E_RENAME             "%s: 0481-209 Cannot rename temporary file.\n"

E_RUNL               "%s: 0481-210 The runlevel field is too long or its format is not correct.\n"

E_UPDATE             "%1$s: 0481-211 Cannot update %2$s; errno is %3$d.\n"

M_CH_USE          "Usage:\t%s \"ident:runlevel:keyword:command\"\n"

M_RM_USE             "Usage:\t%s \"ident\"\n"

M_LS_USE             "Usage:\t%s [\"ident\"|-a]\n"

M_MK_USE             "Usage:\t%s \"[-i ident] ident:runlevel:keyword:command\"\n"
