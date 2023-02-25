$ $*@(#)40      1.2  src/tenplus/helpers/print/Hprthelp.msg, tenplus, tenplus411, GOLD410 7/26/93 15:52:21

$ $*COMPONENT_NAME: (INED) INed Editor
$ $*
$ $*ORIGINS:  27
$ $*
$ $*This module contains IBM CONFIDENTIAL code. -- (IBM
$ $*Confidential Restricted when combined with the aggregated
$ $*modules for this product)                  SOURCE MATERIALS
$ $*(C) COPYRIGHT International Business Machines Corp. 1991
$ $*All Rights Reserved
$ $*
$ $*US Government Users Restricted Rights - Use, duplication or
$ $*disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$quote "
$len
$set MS_HPRTHELP

DFPRINT		"Print on the default printer."
DFPRINTING	"Printing on the default printer."
QPRT_OPT	"| qprt -T $FILE \\_qprt options:\\+ -"
$ Translate only the word "options", leave everything else
$ unchanged.
PRINT_OPT	"Print (ask for options)."
PRINTING	"Printing."
PRINT_OVER	"Print to the file. Write over existing data."
PRTFILE1	"Printing to file $PRTFILE."
$ Do not translate $PRTFILE
PRINT_APP	"Print to the file. Append to existing data."
PRTFILE2	"Printing to file $PRTFILE."
$ Do not translate $PRTFILE
RUN_PIPE	"Run a pipe command."
PRTCMD1		"Running $PRTCMD."
$ Do not translate $PRTCMD
MAIL_TO		"| mail \\_To:\\+"
$ Do not translate. This message MUST stay exactly as it is.
MAIL_MSG	"Send the file in a mail message."
PRTCMD2		"Running $PRTCMD."
$ Do not translate $PRTCMD
RUN_COMM	"Run a command."
PRTCMD3		"Running $PRTCMD."
$ Do not translate $PRTCMD
PRINT_PRVTE	"Print on your private printer."
PRNTNG_PRVTE	"Printing on your private printer."
RUN_COMMS	"Run commands from a shell."
PRTCMD4		"Running $PRTCMD."
$ Do not translate $PRTCMD
