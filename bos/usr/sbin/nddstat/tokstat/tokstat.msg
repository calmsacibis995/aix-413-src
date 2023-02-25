$ @(#)22  1.1.1.3  src/bos/usr/sbin/nddstat/tokstat/tokstat.msg, nddstat, bos41J, 9520B_all 5/18/95 11:44:03
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

$set MS_TOKSTAT        Define set#

$quote "         define quote character.

DEVNAME		"\nTOKEN-RING STATISTICS (%s) :\n"
DEVTYPE		"Device Type: "
MONNAME		"Token-Ring High-Performance Adapter (8fc8)\n"
MPSNAME		"Token-Ring High-Performance Adapter (8fa2)\n"
ISANAME		"Token-Ring IBM ISA Adapter\n"
HWADDR		"Hardware Address: "
ELAPSED_TIME	"Elapsed Time: "
TIME		"%d days %d hours %d minutes %d seconds\n"
BCAST		"Broadcast Packets: %-26d Broadcast Packets: %d\n"
MCAST		"Multicast Packets: %-26d Multicast Packets: %d\n"
HW_TXQ_LEN	"Current HW Transmit Queue Length: %d\n"
SW_TXQ_LEN	"Current SW Transmit Queue Length: %d\n"
GENSTATS	"\nGeneral Statistics:\n"
NOMBUF		"No mbuf Errors: %d\n"
FLAGS		"Driver Flags: "
TIMEOUT_CONG	"Timeout Errors: %-29d Receive Congestion Errors: %d\n"
NOMBUF_LOBE	"No mbuf Errors: %-29d Lobe Wire Faults: %d\n"
ABORT_AC	"Abort Errors: %-31d AC Errors: %d\n"
BURST_COPY	"Burst Errors: %-31d Frame Copy Errors: %d\n"
FREQ_HARD	"Frequency Errors: %-27d Hard Errors: %d\n"
INT_LINE	"Internal Errors: %-28d Line Errors: %d\n"
LOST_ONLY	"Lost Frame Errors: %-26d Only Station: %d\n"
TOKEN_REMOVE	"Token Errors: %-31d Remove Received: %d\n"
RING_SIGNAL	"Ring Recovered: %-29d Signal Loss Errors: %d\n"
SOFT_BEACON	"Soft Errors: %-32d Transmit Beacon Errors: %d\n"
MONSTATS	"Token-Ring High-Performance Adapter (8fc8) Specific Statistics\n"
BUS_PARITY	"DMA Bus Errors: %-29d DMA Parity Errors: %d\n"
ARI		"ARI/FCI Errors: %d\n"
MPSSTATS	"Token-Ring High-Performance Adapter (8fa2) Specific Statistics\n"
OVER_UNDER	"Receive Overruns: %-27d Transmit Underruns: %d\n"
ISASTATS	"Token-Ring IBM ISA Adapter Specific Statistics\n"
READLOGS	"Number of read log commands issued: %d\n"
SKYNAME		"IBM PCI Token-Ring High-Performance Adapter (14101800)\n"
SKYSTATS	"IBM PCI Token-Ring High-Performance Adapter (14101800) Specific Statistics\n"
RX_F_ERROR	"Receive Frame Errors: %-23d\n"
R_ERR_N_TX_R	"Receive Frame Errors: %-23d No_Txq_Resource: %d\n"
