# @(#)80        1.2  src/rspc/kernext/idedisk/devices.ide.disk.rte.codepoint.S, idedisk, rspc41J, 9510A 2/16/95 15:01:18
#
#	COMPONENT_NAME: IDEDISK
#
#	FUNCTIONS: error description stanza file for the IDE Disk DD
#
#	ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
SET E
EC64 "DISK FAILURE RECOVERED DURING RETRY"
SET R
EC1B "VERIFY DISK'S MASTER AND SLAVE JUMPERS ARE PROPERLY SET"
SET D
EC35 "STATUS VALIDITY AND ATA COMMAND"
SET F
EA77 "BAD BLOCK DETECTED"
EA78 "UNCORRECTABLE DATA ERROR"
EA79 "REQUESTED SECTOR'S ID OR ADDRESS MARK NOT FOUND"
EA7A "TRACK 0 NOT FOUND"
EA7B "ATA STATUS ERROR"
EA7C "INVALID MEDIA-CHANGE STATUS"
