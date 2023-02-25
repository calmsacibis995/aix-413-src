$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)27  1.1 src/msg/bos/base/fddistat.msg, bosmsg, msg411, GOLD410  4/14/94  16:34:42
$ COMPONENT_NAME: (NDDSTAT) 
$ FUNCTIONS:   
$ ORIGINS: 27   
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$ (C) COPYRIGHT International Business Machines Corp. 1994
$ All Rights Reserved  
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$
$ GLOBAL SPACING NOTE:
$ Some of these messages are used in table formats, therefore the
$ spacing is very important to the formatting of the data.
$ The spacing between the two columns is forced by specifying the minimum
$ width when printing the numbers.  The width specified is generally much
$ wider than necessary to print the number and the extra width is for column
$ alignment.  However, the double precision numbers "%-xx.0f" must have a
$ minimum of 20 characters.  The decimal numbers "%-xxd" must have a minimum
$ of 11 characters.  If the translated words in the first column have a
$ number of characters different than the original words, the width specified
$ for the number must be changed.
$

$set MS_FDDISTAT        Define set#

$quote "         define quote character.

DEVNAME		"\nFDDI STATISTICS (%s) :\n"
BCAST		"Broadcast Packets: %-26d Broadcast Packets: %d\n"
MCAST		"Multicast Packets: %-26d Multicast Packets: %d\n"
GENSTATS	"\nGeneral Statistics:\n"
NOMBUF		"No mbuf Errors: %d\n"
FLAGS		"Driver Flags: "
SMT_ERR_EVT 	"SMT Error Word: %08x%22 SMT Event Word: %08x\n"
CPV_PORT 	"Connection Policy Violation: %04x%13 Port Event: %04x\n"
SETCOUNT 	"Set Count Hi: %04x%28 Set Count Lo: %04x\n"
ACI_PFRAME 	"Adapter Check Code: %04x%22 Purged Frames: %d\n"
ECM 		"ECM State Machine:        "
PCMA 		"PCM State Machine Port A: "
PCMB 		"PCM State Machine Port B: "
CFMA 		"CFM State Machine Port A: "
CFMB 		"CFM State Machine Port B: "
CF 		"CF State Machine:         "
MCFM 		"MAC CFM State Machine:    "
RMT 		"RMT State Machine:        "
