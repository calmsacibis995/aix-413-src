$ @(#)19	1.13.2.4  src/bos/usr/sbin/tsm/tsm.msg, cmdsauth, bos411, 9428A410j 3/15/94 18:05:45
$
$ COMPONENT_NAME: (CMDSAUTH) security: authentication functions
$
$ FUNCTIONS: NONE
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ Messages for TSM (login and getty) command
$
$set MS_TSM

$

$quote "

M_LOGIN  	"login: "
M_PASSWD	"%s's Password: "
$ The %s in this message is replaced in the program by the user's name.
$ Notice that this is made possessive in English with the 's.  Make this
$ possessive in the appropriate language and place the %s in the
$ appropriate position.
M_NOSESSIONS	"3004-003 All available login sessions are in use"
M_NOTLOWEST	"3004-004 You must \"exec\" login from the lowest login shell."
M_OPENWIN	"3004-005 You must close all windows before logging out."
M_NOSAK		"3004-006 You must use the SAK key to get on the trusted path."
M_BADLOGIN	"3004-007 You entered an invalid login name or password."
M_FAILCRED 	"3004-008 Failed setting credentials."
M_FAILEXEC 	"3004-009 Failed running login shell."
M_FAILTERM 	"3004-010 Failed setting terminal ownership and mode."
M_OFFTPATH	"3004-011 Trusted path required for this terminal."
M_NOTPATH	"3004-012 Trusted path not allowed for this terminal."
M_NOTSH		"3004-013 Trusted shell not allowed for this account."
M_NOARGS	"3004-014 TSM was invoked without a port name"
M_CANTOPEN	"3004-015 TSM was unable to open port \"%s\""
M_CANTCLEAN	"3004-016 TSM was unable to establish a trusted path."
M_BADBAUD	"3004-017 TSM was invoked with an illegal baud rate."
M_NOLOGGER	"3004-018 TSM was unable to execute \"%s\"."
M_PLEASEFIX	"3004-019 TSM encountered an error on terminal \"%s\"."
M_BADENV	"3004-020 The variable \"%s\" may not be set.\n"
M_NOPRIV	"3004-021 TSM lacks a required privilege.\n"
M_FAILREMLOG	"3004-022 Remote login failed.\n"
M_UNKLOGOPT	"3004-023 -\"%c\" is not a valid option to login.\n"
M_IOCTLFAIL     "3004-024 - ioctl failed errno \"%d\".\n"
M_TCGATTRFAIL   "3004-025 - tcgetattr  failed errno \"%d\".\n"
M_TCSATTRFAIL   "3004-026 - tcsetattr  failed errno \"%d\".\n"
M_ALLOCFAIL     "3004-027 - malloc failed.\n"
M_DUPFAIL       "3004-028 - dup failed.\n"
M_SEMFAIL       "3004-029 - sem op failed.\n"
M_LINEDISPF     "3004-030 - Failure setting line disp.\n"
M_SLOGIN 	"Last login: %s on %s\n"
M_SLOGINHT 	"Last login: %s on %s from %s\n"
M_FLOGIN 	"Last unsuccessful login: %s on %s\n"
M_FLOGINHT 	"Last unsuccessful login: %s on %s from %s\n"
M_FCOUNT 	"1 unsuccessful login attempt since last login\n"
M_FCOUNTS 	"%d unsuccessful login attempts since last login\n"
M_NOISE         "3004-031 Write timed out -- possible noise on port"
M_NOPAS         "3004-031 Password read timed out -- possible noise on port"
M_ALLCAPS	"3004-033 YOU LOGGED IN USING ALL UPPERCASE CHARACTERS.\r\n         IF YOUR WORKSTATION ALSO SUPPORTS LOWERCASE\r\n         CHARACTERS, LOG OFF, THEN LOG IN AGAIN USING\r\n         LOWERCASE CHARACTERS.\r\n";
$ This message should be translated into all uppercase letters.
M_HERALD	"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\rAIX Version 4\n\r(C) Copyrights by IBM and by others 1982, 1994.\n\rlogin: "
M_CONHERALD	"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\rAIX Version 4\n\r(C) Copyrights by IBM and by others 1982, 1994.\n\rConsole login: "
M_SETTINGS	"3004-034 TSM: All possible term settings tried unsuccessfully."
M_WRITEERR	"3004-035 TSM: write to %s failed."
$ The %s is replaced with the name of the port tsm is running on.
M_READERR	"3004-036 TSM: read from %s failed."
$ The %s is replaced with the name of the port tsm is running on.

