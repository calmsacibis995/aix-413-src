$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)88  1.3  src/msg/bos/base/entstat.msg, bosmsg, msg411, GOLD410 4/26/94 17:05:10
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

$set MS_ENTSTAT        Define set#

$quote "         define quote character.

DEVNAME		"\nETHERNET STATISTICS (%s) :\n"
DEVTYPE		"Device Type: "
HPLAN		"Ethernet High Performance LAN Adapter\n"
$ The next message has been replaced by message ITDENT.
STDENT		"Standard Ethernet Adapter\n"
ISAIEN		"IBM ISA Ethernet adapter\n"
ENLCE		"Low Cost High Performance Ethernet adapter\n"
HWADDR		"Hardware Address: "
ELAPSED_TIME    "Elapsed Time: "
TIME            "%d days %d hours %d minutes %d seconds\n"
BCAST		"Broadcast Packets: %-26d Broadcast Packets: %d\n"
MCAST		"Multicast Packets: %-26d Multicast Packets: %d\n"
CARRIER		"No Carrier Sense: %-27d CRC Errors: %d\n"
DMA		"DMA Underrun: %-31d DMA Overrun: %d\n"
CTSERR		"Lost CTS Errors: %-28d Alignment Errors: %d\n"
MAXCOLL		"Max Collision Errors: %-23d No Resource Errors: %d\n"
LATECOLL	"Late Collision Errors: %-22d Receive Collision Errors: %d\n"
DEFERED		"Deferred: %-35d Packet Too Short Errors: %d\n"
SQE		"SQE Test: %-35d Packet Too Long Errors: %d\n"
TMOUT		"Timeout Errors: %-29d Packets Discarded by Adapter: %d\n"
SCOLL		"Single Collision Count: %-21d Receiver Start Count: %d\n"
MCOLL		"Multiple Collision Count: %d\n"
HW_TXQ_LEN	"Current HW Transmit Queue Length: %d\n"
GENSTATS	"\nGeneral Statistics:\n"
NOMBUF		"No mbuf Errors: %d\n"
RESET		"Adapter Reset Count: %d\n"
FLAGS		"Driver Flags: "
HPLAN_STATS	"\nEthernet High Performance LAN Adapter Specific Statistics:\n"
RV_POOL		"Receive Buffer Pool Size: %d\n"
TX_POOL		"Transmit Buffer Pool Size: %d\n"
IP_PROMIS	"In Promiscuous Mode for IP Multicast: "
UPLOAD		"Packets Uploaded from Adapter: %d\n"
HOST_EOL	"Host End-of-List Encountered: %d\n"
EOL_586		"82586 End-of-List Encountered: %d\n"
DMATMOUT	"Receive DMA Timeouts: %d\n"
RESERVED	"Adapter Internal Data: 0x%x 0x%x 0x%x 0x%x 0x%x\n"
$ The next message has been replaced by message ITDENT_STATS.
STDENT_STATS	"\nStandard Ethernet Adapter Specific Statistics:\n"
COLL_TABLE	"Packets with Transmit collisions:\n"
COLL_FREQ	"%2d collisions: %-10d %2d collisions: %-10d %2d collisions: %d\n"
IENISA_STATS 	"\nIBM ISA Ethernet Adapter Specific Statistics:\n"
ENLCE_STATS	"\nLow Cost High Performance Ethernet Adapter Specific Statistics:\n"
ENLCE_TYPE	"Adapter Type: "
ENLCE_MEDIA	"Media Selection: "
ENLCE_MACE	"Ethernet Controller Version: 0x%x\n"
ENLCE_MCNET	"Micro Channel Controller Version: 0x%x\n"
$ The next message is to replace message STDENT.
ITDENT		"Integrated Ethernet Adapter\n"
$ The next message is to replace STDENT_STATS.
ITDENT_STATS	"\nIntegrated Ethernet Adapter Specific Statistics:\n"
