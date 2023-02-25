; @(#)37	1.1  src/rspc/kernext/pci/ncr810/ncr8xxi.ss, pciscsi, rspc41J, 9507A 1/25/95 14:55:18
;
; COMPONENT_NAME: (PCISCSI) IBM SCSI Adapter Iowait SCRIPT(tm) File
;
; FUNCTIONS: NONE
;
; ORIGINS: 27
;
; (C) COPYRIGHT International Business Machines Corp. 1989, 1995
; All Rights Reserved
; Licensed Materials - Property of IBM
;
; US Government Users Restricted Rights - Use, duplication or
; disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
;

; SCRIPT
; This SCRIPT contains the iowait script used to handle reselections from
; all targets.

; References to objects that are are resolved (``patched'') by the device
; driver.  Note that some of these objects reside in system memory or I/O
; memory (chip registers) while others are within the SCRIPTS themselves (but
; must be relocated) and still others are used to link the target SCRIPTS with
; iowait SCRIPTS.

; These are relocated
EXTERNAL lun_msg_addr

; Target to iowait SCRIPTS linking:
EXTERNAL scsi_0_lun, scsi_1_lun, scsi_2_lun, scsi_3_lun
EXTERNAL scsi_4_lun, scsi_5_lun, scsi_6_lun, scsi_7_lun
EXTERNAL scsi_8_lun, scsi_9_lun, scsi_A_lun, scsi_B_lun
EXTERNAL scsi_C_lun, scsi_D_lun, scsi_E_lun, scsi_F_lun

; These are the entry points at which we may start SCRIPTS
; processing
ENTRY iowait_entry_point

; Buffers
ENTRY lun_msg_buf

; Patch points
ENTRY scsi_id_patch

ABSOLUTE unknown_reselect_id    = 0x0001D
ABSOLUTE uninitialized_reselect = 0X0001E
ABSOLUTE suspended              = 0X00020


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
iowait_entry_point:
        wait reselect suspend_chip

reselect_router:
        ;examine bits 7 (VAL) and 3-0 (the reselecting device's SCSI id)
        move SSID & 0x8F to SFBR
        move SFBR & 0x0F to SCRATCHB0
scsi_id_patch:
        jump scsi_id_0 if 0x80
        jump scsi_id_1 if 0x81
        jump scsi_id_2 if 0x82
        jump scsi_id_3 if 0x83
        jump scsi_id_4 if 0x84
        jump scsi_id_5 if 0x85
        jump scsi_id_6 if 0x86
        jump scsi_id_7 if 0x87
        jump scsi_id_8 if 0x88
        jump scsi_id_9 if 0x89
        jump scsi_id_A if 0x8A
        jump scsi_id_B if 0x8B
        jump scsi_id_C if 0x8C
        jump scsi_id_D if 0x8D
        jump scsi_id_E if 0x8E
        jump scsi_id_F if 0x8F
        ;interrupt if not VAL or bits 3-0 indicate our SCSI id
        int unknown_reselect_id


; Reselection routing...jump depending upon what target has reselected
; us.
scsi_id_0:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_0_lun if 0x80
        jump scsi_0_lun if 0x81
        jump scsi_0_lun if 0x82
        jump scsi_0_lun if 0x83
        jump scsi_0_lun if 0x84
        jump scsi_0_lun if 0x85
        jump scsi_0_lun if 0x86
        jump scsi_0_lun if 0x87
        int uninitialized_reselect
scsi_id_1:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_1_lun if 0x80
        jump scsi_1_lun if 0x81
        jump scsi_1_lun if 0x82
        jump scsi_1_lun if 0x83
        jump scsi_1_lun if 0x84
        jump scsi_1_lun if 0x85
        jump scsi_1_lun if 0x86
        jump scsi_1_lun if 0x87
        int uninitialized_reselect
scsi_id_2:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_2_lun if 0x80
        jump scsi_2_lun if 0x81
        jump scsi_2_lun if 0x82
        jump scsi_2_lun if 0x83
        jump scsi_2_lun if 0x84
        jump scsi_2_lun if 0x85
        jump scsi_2_lun if 0x86
        jump scsi_2_lun if 0x87
        int uninitialized_reselect
scsi_id_3:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_3_lun if 0x80
        jump scsi_3_lun if 0x81
        jump scsi_3_lun if 0x82
        jump scsi_3_lun if 0x83
        jump scsi_3_lun if 0x84
        jump scsi_3_lun if 0x85
        jump scsi_3_lun if 0x86
        jump scsi_3_lun if 0x87
        int uninitialized_reselect
scsi_id_4:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_4_lun if 0x80
        jump scsi_4_lun if 0x81
        jump scsi_4_lun if 0x82
        jump scsi_4_lun if 0x83
        jump scsi_4_lun if 0x84
        jump scsi_4_lun if 0x85
        jump scsi_4_lun if 0x86
        jump scsi_4_lun if 0x87
        int uninitialized_reselect
scsi_id_5:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_5_lun if 0x80
        jump scsi_5_lun if 0x81
        jump scsi_5_lun if 0x82
        jump scsi_5_lun if 0x83
        jump scsi_5_lun if 0x84
        jump scsi_5_lun if 0x85
        jump scsi_5_lun if 0x86
        jump scsi_5_lun if 0x87
        int uninitialized_reselect
scsi_id_6:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_6_lun if 0x80
        jump scsi_6_lun if 0x81
        jump scsi_6_lun if 0x82
        jump scsi_6_lun if 0x83
        jump scsi_6_lun if 0x84
        jump scsi_6_lun if 0x85
        jump scsi_6_lun if 0x86
        jump scsi_6_lun if 0x87
        int uninitialized_reselect
scsi_id_7:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_7_lun if 0x80
        jump scsi_7_lun if 0x81
        jump scsi_7_lun if 0x82
        jump scsi_7_lun if 0x83
        jump scsi_7_lun if 0x84
        jump scsi_7_lun if 0x85
        jump scsi_7_lun if 0x86
        jump scsi_7_lun if 0x87
; If we have reached this point, that means a device that we know nothing about
; has reselected us, so we have jumped here.  it could mean we forgot to
; write in the patch address to a device we do recognize, and we jumped here
; as part of the "safety net".  Or, that the device has done an illegal
; reselection after an ABORT or BDR has been issued to it.
        int uninitialized_reselect
scsi_id_8:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_8_lun if 0x80
        jump scsi_8_lun if 0x81
        jump scsi_8_lun if 0x82
        jump scsi_8_lun if 0x83
        jump scsi_8_lun if 0x84
        jump scsi_8_lun if 0x85
        jump scsi_8_lun if 0x86
        jump scsi_8_lun if 0x87
        int uninitialized_reselect
scsi_id_9:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_9_lun if 0x80
        jump scsi_9_lun if 0x81
        jump scsi_9_lun if 0x82
        jump scsi_9_lun if 0x83
        jump scsi_9_lun if 0x84
        jump scsi_9_lun if 0x85
        jump scsi_9_lun if 0x86
        jump scsi_9_lun if 0x87
        int uninitialized_reselect
scsi_id_A:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_A_lun if 0x80
        jump scsi_A_lun if 0x81
        jump scsi_A_lun if 0x82
        jump scsi_A_lun if 0x83
        jump scsi_A_lun if 0x84
        jump scsi_A_lun if 0x85
        jump scsi_A_lun if 0x86
        jump scsi_A_lun if 0x87
        int uninitialized_reselect
scsi_id_B:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_B_lun if 0x80
        jump scsi_B_lun if 0x81
        jump scsi_B_lun if 0x82
        jump scsi_B_lun if 0x83
        jump scsi_B_lun if 0x84
        jump scsi_B_lun if 0x85
        jump scsi_B_lun if 0x86
        jump scsi_B_lun if 0x87
        int uninitialized_reselect
scsi_id_C:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_C_lun if 0x80
        jump scsi_C_lun if 0x81
        jump scsi_C_lun if 0x82
        jump scsi_C_lun if 0x83
        jump scsi_C_lun if 0x84
        jump scsi_C_lun if 0x85
        jump scsi_C_lun if 0x86
        jump scsi_C_lun if 0x87
        int uninitialized_reselect
scsi_id_D:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_D_lun if 0x80
        jump scsi_D_lun if 0x81
        jump scsi_D_lun if 0x82
        jump scsi_D_lun if 0x83
        jump scsi_D_lun if 0x84
        jump scsi_D_lun if 0x85
        jump scsi_D_lun if 0x86
        jump scsi_D_lun if 0x87
        int uninitialized_reselect
scsi_id_E:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_E_lun if 0x80
        jump scsi_E_lun if 0x81
        jump scsi_E_lun if 0x82
        jump scsi_E_lun if 0x83
        jump scsi_E_lun if 0x84
        jump scsi_E_lun if 0x85
        jump scsi_E_lun if 0x86
        jump scsi_E_lun if 0x87
        int uninitialized_reselect
scsi_id_F:
        move 1, lun_msg_addr, when MSG_IN
        jump scsi_F_lun if 0x80
        jump scsi_F_lun if 0x81
        jump scsi_F_lun if 0x82
        jump scsi_F_lun if 0x83
        jump scsi_F_lun if 0x84
        jump scsi_F_lun if 0x85
        jump scsi_F_lun if 0x86
        jump scsi_F_lun if 0x87
        int uninitialized_reselect

; We get here if the SIGP flag has been set by the processor whilst we
; were in the wait reselect instruction.  This is done to stop the chip
; so that a new I/O operation can be initiated.
suspend_chip:
        move CTEST2 to SFBR     ; clears sigp
        int suspended

;;;;;;;;;;;;;
lun_msg_buf:
        nop lun_msg_buf

