# @(#)92        1.1  src/rspc/kernext/idedisk/devices.ide.disk.rte.trc.S, idedisk, rspc41J, 9510A 2/21/95 14:27:12
#
#   COMPONENT_NAME: IDEDISK
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
* /etc/trcfmt
+2A1 1.0 "IDEDISKDD" \
        $HDU, \
        0x0F { strategy: bp=$D2 starttimer(0x2A1,$D2) }, \
        0x1E { `coalesce: (bp,ataidebuf)=($D2,$D3)` }, \
        0x1F { iodone: devtoname($D1) bp=$D2 buftofilename($D2) \
                endtimer(0x2A1,$D2) }, \
        0x17 { $D2, 0 { $SKIP } \
                bstart_entry: devtoname($D1) bp=$D2 buftofilename($D2) \
                        blkno=$D3 bcount=$D4 bflags($D5) \
        }, \
        0x18 { $D2, 0 { $SKIP } \
                bstart_exit: devtoname($D1) \
        }, \
        0x1b { $D2, 0 { $SKIP } \
               iodone : devtoname($D1) b_resid=$D2 b_error=$D3 \
        }, \
        \*   { $DD1 }
