/* @(#)91	1.3  src/bos/usr/sbin/fsck/fsck.h, cmdfs, bos411, 9428A410j 6/15/90 21:19:02 */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: fsck header
 *
 * ORIGINS: 3, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. date 1, date 2
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * defined fsck exit codes
 *      an actual exit code may be the OR of various values
 */

#define EXIT_OK         0       /* file systems OK or damage repaired   */
#define EXIT_INTR       2       /* interrupted during checks            */
#define EXIT_BOOT       4       /* root changed - reboot                */
#define EXIT_FAIL       8       /* unrepaired damage                    */
