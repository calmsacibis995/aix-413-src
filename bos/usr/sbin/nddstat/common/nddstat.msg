$ @(#)54  1.2  src/bos/usr/sbin/nddstat/common/nddstat.msg, nddstat, bos411, 9428A410j 2/23/94 18:14:22
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

$set MS_NDDSTAT        Define set#

$quote "         define quote character.

USAGE		"\nusage: %s [-dr] <device name>\n"
NO_PERMISSION	"\n%s: 0909-001 Permission denied.\n"
SOCKET_FAILED	"\n%s: 0909-002 Unable to open device %s, errno = %d\n"
CONNECT_FAILED	"\n%s: 0909-003 Unable to connect to device %s, errno = %d\n"
GET_FAILED	"\n%s: 0909-004 Unable to get statistics on device %s, errno = %d\n"
CLEAR_FAILED	"\n%s: 0909-005 Unable to clear statistics on device %s, errno = %d\n"
ELAPSED_TIME	"Elapsed Time: "
TIME		"%d days %d hours %d minutes %d seconds\n"
TX_RV_STATS	"\nTransmit Statistics:                          Receive Statistics:\n"
PKTS		"Packets: %-36.0f Packets: %.0f\n"
BYTES		"Bytes: %-38.0f Bytes: %.0f\n"
INTRS		"Interrupts: %-33.0f Interrupts: %.0f\n"
ERRORS		"Transmit Errors: %-28d Receive Errors: %d\n"
DROPS		"Packets Dropped: %-28d Packets Dropped: %d\n"
$ It is not a problem if the second column on the following message
$ is a few characters to the right of the other entries in the column.
MAX_TX_Q	"Max Packets on S/W Transmit Queue: %-10d Bad Packets: %d\n"
OFLOW		"S/W Transmit Queue Overflow: %d\n"
TXQ_LEN		"Current S/W+H/W Transmit Queue Length: %d\n"
