; @(#)07	1.2  src/rspc/kernext/pci/ncr810/ncr8xx.ss, pciscsi, rspc41J, 9513A_all 3/28/95 14:44:11
;
; COMPONENT_NAME: (PCISCSI) IBM SCSI Adapter Main SCRIPT(tm) File
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
; This SCRIPT contains the one main script used for most of all the
; SCSI commands that will be sent to the target.  It is written in a manner
; to handle possible, erroneous bus phases and messages received during the
; execution of the script

; References to objects that are are resolved (``patched'') by the device
; driver.  Note that some of these objects reside in system memory or I/O
; memory (chip registers) while others are within the SCRIPTS themselves (but
; must be relocated) and still others are used to link the target SCRIPTS with
; iowait SCRIPTS.

; System memory references
EXTERNAL tim_addr, tim_cnt

; These chip registers are source/targets of memory moves
EXTERNAL DSA_Addr
EXTERNAL SCRATCHB_addr
EXTERNAL SCRATCHA0_addr

; These are relocated
EXTERNAL restore_patch_addr
EXTERNAL cmd_msg_in_addr
EXTERNAL cmd_addr
EXTERNAL status_addr
EXTERNAL identify_msg_addr
EXTERNAL wdtr_msg_out_addr
EXTERNAL neg_msg_addr
EXTERNAL sdtr_msg_out_addr
EXTERNAL extended_msg_addr
;EXTERNAL lun_msg_addr
EXTERNAL reject_msg_addr
EXTERNAL tag_msg_addr
EXTERNAL abort_bdr_msg_out_addr
EXTERNAL abort_bdr_msg_in_addr

; Target to iowait SCRIPTS linking:
;EXTERNAL scsi_0_lun, scsi_1_lun, scsi_2_lun, scsi_3_lun
;EXTERNAL scsi_4_lun, scsi_5_lun, scsi_6_lun, scsi_7_lun
EXTERNAL wait_reselect


; These are the entry points at which we may start SCRIPTS
; processing
ENTRY scripts_entry_point
;ENTRY iowait_entry_point
ENTRY after_data_move_check
ENTRY reject_target_sync
ENTRY unknown_msg_hdlr
ENTRY script_reconnect_point
ENTRY begin_negotiation
;ENTRY start_sync_msg_out
ENTRY start_negotiation_msg_out_loop
ENTRY renegotiate_wdtr
ENTRY renegotiate_sdtr
ENTRY msg_done
ENTRY save_pointers_wide
ENTRY tables_same
ENTRY goto_cleanup
ENTRY complete_ext_msg
ENTRY message_loop
ENTRY reject_loop
ENTRY renegotiate_sdtr_loop
ENTRY renegotiate_wdtr_loop
ENTRY cleanup_phase
ENTRY regular_phase_hdlr
ENTRY phase_error_entry
ENTRY bdr_sequence
ENTRY abort_sequence, abdr2_sequence
ENTRY save_scntl2
ENTRY failed_selection_hdlr, end_failed_sel_hdlr 
ENTRY failed_negotiation_selection_hdlr, end_failed_neg_sel_hdlr 
ENTRY failed_abort_bdr_selection_hdlr, end_failed_abdr_sel_hdlr 
ENTRY wait_for_bus_free, end_wait_for_bus_free 

; Buffers
ENTRY cmd_msg_in_buf, cmd_buf
ENTRY status_buf
ENTRY identify_msg_buf
ENTRY extended_msg_buf
ENTRY reject_msg_buf
ENTRY neg_msg_buf
;ENTRY lun_msg_buf
ENTRY sdtr_msg_out_buf
ENTRY wdtr_msg_out_buf
ENTRY abort_bdr_msg_out_buf, abort_bdr_msg_in_buf
ENTRY tag_msg_buf

ENTRY output_wide_data, output_data
ENTRY save_pointers
ENTRY in_not_zero

; Patch points
ENTRY cmd_complete_patch
ENTRY ext_msg_patch
ENTRY send_data
ENTRY save_ptrs_patch, save_ptrs_patch1
ENTRY negotiation_phase_patch
ENTRY start_sdtr
ENTRY start_sdtr_msg_in_phase
ENTRY start_wdtr_msg_in_phase
ENTRY scntl2_patch_out, scntl2_patch_in

ABSOLUTE        dptr_tbl_addr1_ref      = 0x01010101
ABSOLUTE        dptr_tbl_addr1          = 0x11111111
ABSOLUTE        dptr_tbl_addr2_ref      = 0x04040404
ABSOLUTE        dptr_tbl_addr2          = 0x44444444

ABSOLUTE        dptr_cnt_addr1_ref      = 0x02020202
ABSOLUTE        dptr_cnt_addr1          = 0x22222222
ABSOLUTE        dptr_cnt_addr2_ref      = 0x03030303
ABSOLUTE        dptr_cnt_addr2          = 0x33333333
ABSOLUTE        dptr_cnt_addr3_ref      = 0x05050505
ABSOLUTE        dptr_cnt_addr3          = 0x55555555
ABSOLUTE        dptr_cnt_addr4_ref      = 0x09090909
ABSOLUTE        dptr_cnt_addr4          = 0x99999999
ABSOLUTE        dptr_cnt_addr5_ref      = 0x06060606
ABSOLUTE        dptr_cnt_addr5          = 0x66666666
;ABSOLUTE        dptr_cnt_addr6_ref      = 0x06060606
;ABSOLUTE        dptr_cnt_addr6          = 0x66666666

;ABSOLUTE        patch6_ref      = 0x06060606
;ABSOLUTE        patch6          = 0x66666666

ABSOLUTE        cur_sav_tbl_addr1_ref      = 0x07070707
ABSOLUTE        cur_sav_tbl_addr1          = 0x77777777
ABSOLUTE        cur_sav_tbl_addr2_ref      = 0x0D0D0D0D
ABSOLUTE        cur_sav_tbl_addr2          = 0xDDDDDDDD

ABSOLUTE        cur_loc_tbl_addr1_ref      = 0x08080808
ABSOLUTE        cur_loc_tbl_addr1          = 0x88888888
ABSOLUTE        cur_loc_tbl_addr2_ref      = 0x0C0C0C0C
ABSOLUTE        cur_loc_tbl_addr2          = 0xCCCCCCCC

ABSOLUTE        dptr_diff_addr1_ref      = 0x0A0A0A0A
ABSOLUTE        dptr_diff_addr1          = 0xAAAAAAAA
ABSOLUTE        dptr_diff_addr2_ref      = 0x0B0B0B0B
ABSOLUTE        dptr_diff_addr2          = 0xBBBBBBBB
ABSOLUTE        dptr_diff_addr3_ref      = 0x0E0E0E0E
ABSOLUTE        dptr_diff_addr3          = 0xEEEEEEEE

ABSOLUTE        dptr_restore_addr_ref      = 0x0F0F0F0F
ABSOLUTE        dptr_restore_addr          = 0xFFFFFFFF

ABSOLUTE        NEXUS_data_base_adr3    = 0x88
ABSOLUTE        NEXUS_data_base_adr2    = 0x88
ABSOLUTE        NEXUS_data_base_adr1    = 0x88
ABSOLUTE        NEXUS_data_base_adr0    = 0x88

ABSOLUTE        save_tbl_base_adr3    = 0x99
ABSOLUTE        save_tbl_base_adr2    = 0x99
ABSOLUTE        save_tbl_base_adr1    = 0x99
ABSOLUTE        save_tbl_base_adr0    = 0x99

ABSOLUTE        restore_patch    = 0x00AAAA

; Interrupt values used to communicate the type of interrupt
; back to the device driver
; CAUTION: New interrupts added here likely will need a new entry in
; the itbl (interrupt table) contained in ncr8xxu.c.
ABSOLUTE phase_error            = 0x00001
ABSOLUTE save_ptrs_wsr		= 0x00004
ABSOLUTE save_ptrs_wss		= 0x00005
ABSOLUTE io_done                = 0x00006
ABSOLUTE io_done_wsr       	= 0x00007
ABSOLUTE io_done_wss       	= 0x00008
ABSOLUTE unknown_msg            = 0x00009
ABSOLUTE ext_msg                = 0x0000A
ABSOLUTE check_next_io          = 0x0000B
ABSOLUTE cmd_select_atn_failed  = 0x00012
ABSOLUTE err_not_ext_msg        = 0x00013
;ABSOLUTE sdtr_neg_done          = 0x00014
;ABSOLUTE unexpected_status      = 0x00015
ABSOLUTE sync_msg_reject        = 0x00016
;ABSOLUTE abort_msg_error        = 0x00017
ABSOLUTE neg_select_failed      = 0x00018
ABSOLUTE abort_select_failed    = 0x00019
ABSOLUTE abort_io_complete      = 0x0001A
;ABSOLUTE bdr_select_failed	= 0x0001B
;ABSOLUTE bdr_io_complete	= 0x0001C
;ABSOLUTE unknown_reselect_id    = 0x0001D
;ABSOLUTE uninitialized_reselect = 0X0001E
;ABSOLUTE bdr_msg_error		 = 0x0001F
;ABSOLUTE suspended              = 0X00020
ABSOLUTE wdtr_msg_ignored	= 0x00021
ABSOLUTE sdtr_msg_ignored	= 0x00022
ABSOLUTE wdtr_neg_done  	= 0x00023
ABSOLUTE sdtr_neg_done          = 0x00024
ABSOLUTE wdtr_msg_reject  	= 0x00025
ABSOLUTE sdtr_msg_reject  	= 0x00026
ABSOLUTE ignore_residue		= 0x00027
;ABSOLUTE trace_point            = 0X00040
;ABSOLUTE chkpoint               = 0X00041

; Chip register flags.  The SCRIPTS use these to set/reset the
; flags
ABSOLUTE SIOM_Flag              = 0x20
ABSOLUTE Not_SIOM_Flag          = 0xDF
ABSOLUTE DIOM_Flag              = 0x10
ABSOLUTE Not_DIOM_Flag          = 0xEF


; The device drivers patches all references to these with target
; specific data
RELATIVE target_id              = 0x01
RELATIVE cmd_bytes_out_count    = 0x00
RELATIVE ext_msg_size           = 0x00
RELATIVE scntl3_patch           = 0x13
RELATIVE sxfer_patch            = 0x08
RELATIVE tag_patch              = 0x99
RELATIVE abdr_tag_patch         = 0xAA

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
init_index:
        ; store scsi id and lun in SCRATCHA3
        ; SFBR = [ xxxx xLun_id ]
        MOVE SFBR & 0x07 TO SCRATCHA3
        ; SCRATCHA3 = [ 00000 Lun_id ]
        MOVE SCRATCHB0 TO SFBR
        ; SFBR = [ 0000 Scsi_id ]
        ; computes nexus=[0 ScsiId LunId] :
        JUMP REL(bit3), IF 0x00 AND MASK 0xF7
        MOVE SCRATCHA3 | 0x40 TO SCRATCHA3
bit3:   JUMP REL(bit2), IF 0x00 AND MASK 0xFB
        MOVE SCRATCHA3 | 0x20 TO SCRATCHA3
bit2:   JUMP REL(bit1), IF 0x00 AND MASK 0xFD
        MOVE SCRATCHA3 | 0x10 TO SCRATCHA3
bit1:   JUMP REL(bitend), IF 0x00 AND MASK 0xFE
        MOVE SCRATCHA3 | 0x08 TO SCRATCHA3
bitend: ; SCRATCHA3 = [ 0Scsi_id Lun_id ]
        RETURN

;handle being in message after receiving the identify message after a
;reselect
reconnect_msg_in:
        move 1, cmd_msg_in_addr, when MSG_IN
        jump queue_tag, if 0x20
        MOVE SFBR to CTEST0
        ;save sfbr before calling restore_ptrs
        call rel(restore_ptrs)
        MOVE CTEST0 to SFBR
        ;restore sfbr
        jump not_tag_msg
queue_tag:
        ; SFBR = Queue tag operator (0x20)
        ; Save the nexus given there
        CLEAR ACK
        MOVE 1, tag_msg_addr, when MSG_IN
        MOVE SFBR to SCRATCHA3
        call rel(restore_ptrs)
        jump msg_done

script_reconnect_point:
        call rel(init_index)
        move scntl3_patch to SCNTL3
        move sxfer_patch to SXFER
        clear ACK

;Handle the possible actions following a reconnection.  We have already read
;the initial MSG_IN in the IO_WAIT script (this is how we figured out which
;target was trying to reselect us).  If we aren't in MSG_IN any longer,
;set DSA.
        jump reconnect_msg_in, when MSG_IN
        call rel(restore_ptrs)
        jump receive_data, when DATA_IN
        jump send_data, if DATA_OUT
        jump status_complete, if STATUS
        int phase_error

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Arbitrate and select the target with the SCSI id written into the current
; zero position.  If during the selection, another target (including the target
; we were trying to select) reselects the chip, then we jump to the script at
; the address failed_selection_hdlr.  This script does a simple interrupt so
; that the process interrupt handler will see that this script never got
; started and needs to be restarted at a later time.  The interrupt handler
; will then start the SIOP at the script address of iowait_entry_point (also
; known as the IO_WAIT script) to start the handling of the reselection by the
; target.

scripts_entry_point:
        select ATN target_id, failed_selection_hdlr

; This handles the case in which we are trying to select a "dead" device.  Most
; "dead" devices should return a bad status to indicate their "dead" status.
; However, if the device is operating normally, the STATUS phase does not
; occur, and we will not take the jump to status_complete.  Instead, we will
; drop down to the MSG_OUT phase to start the SCSI phase sequence with the
; target.  If we get neither a STATUS or MSG_OUT phase, then something is wrong
; with the bus protocol and we interrupt to the interrupt handler.

        move tag_patch to SCRATCHA3
        move CTEST2 | 0x00 to CTEST2
        int phase_error, when not MSG_OUT

message_loop:
        move 1, identify_msg_addr, when MSG_OUT
cleanup_phase:
        call rel(restore_ptrs)
        move scntl3_patch to SCNTL3
        move sxfer_patch to SXFER

; Loop indefinitely in this message output loop until the device receives the
; messages and then changes to the next phase.  The next phase is dictated by
; the contents of the messages.  So, the following lines check for all possible
; phases that could occur for all possible types of messages that were just
; sent.
        jump send_command when CMD

; We check for three possible cases.  The default case we drop into is the CMD
; phase.  So we output the CMD and then check for all the possible phases that
; could occur as a result of that command.

        jump status_complete if STATUS
        jump msg_hdlr if MSG_IN
        jump message_loop_atn if MSG_OUT
        int phase_error

message_loop_atn:
	set ATN
	jump message_loop

send_command:
        move cmd_bytes_out_count, cmd_addr, when CMD
regular_phase_hdlr:
        jump msg_hdlr when MSG_IN
        jump status_complete if STATUS
        jump receive_data if DATA_IN
        jump send_data if DATA_OUT

; After the DATA_OUT check, there can be no more possible phases that we could
; expect from any command other than those we've just checked.  Thus, we must be
; in trouble.

phase_error_entry:
        int phase_error

; Handle a message-in phase.

msg_hdlr:
        move 1, cmd_msg_in_addr, when MSG_IN
        jump queue_tag, if 0x20
not_tag_msg:
        jump ext_msg_handler if 01              ; Extended messages
        jump disconnect_point if 04             ; Disconnect
        jump reject_cleanup if 07               ; Message reject
        jump restore_msg if 03                  ; Restore pointers
	jump ignore_wide_residue if 0x23	; Ignore Wide Residue
        jump unknown_msg_hdlr if not 0x02      	; Save data pointer
save_ptrs_patch:
	call save_pointers
msg_done:
        clear ACK
        jump regular_phase_hdlr

restore_msg:
	call restore_ptrs
	jump msg_done

unknown_msg_hdlr:
        int unknown_msg

ignore_wide_residue:
	clear ACK
        MOVE 1, cmd_msg_in_addr, when MSG_IN
	int ignore_residue

save_pointers_wide:
	move SCRATCHA2 & 0x09 to SFBR
	int save_ptrs_wss, if 0x08
	int save_ptrs_wsr, if 0x01

save_pointers:
;save the contents of DSA and SCRATCHB.  If we have taken a phase
;mismatch such that the loc and save tim tables differ, update the
;save tim table and clear the flags.  If there is a discrepancy
;between the # of bytes DMAed to/from memory and the # of bytes
;moved from/to the SCSI bus, let the host handle the processing
;instead.

        move DMODE | SIOM_Flag to DMODE         ; src:  I/O
        move DMODE & Not_DIOM_Flag to DMODE     ; dest: memory
        MOVE MEMORY 4, DSA_Addr, dptr_tbl_addr2_ref
        MOVE MEMORY 4, SCRATCHB_addr, dptr_cnt_addr3_ref

        move DMODE & Not_SIOM_Flag to DMODE	; src: memory
	move SCRATCHA0 to SFBR
	jump tables_same if 0

;should we assume the copy is always needed, and avoid the test?
;use scratcha to hold tables differ? would have to update scratcha
;from update_data_ptrs...

        move DMODE | SIOM_Flag to DMODE         ; src:  I/O
        ;move DMODE & Not_DIOM_Flag to DMODE     ; dest: memory
	MOVE 0 to SCRATCHA0
        MOVE MEMORY 1, SCRATCHA0_addr, dptr_diff_addr1_ref

	move DSA0 to SFBR
	move SFBR to SCRATCHB0
        move DSA1 & 0x0F to SFBR
	move SFBR to SCRATCHB1
	move 0 to SCRATCHB2
	move 0 to SCRATCHB3
	;scratchb holds the offset

	move SCRATCHB0 + save_tbl_base_adr0 to SCRATCHB0
	move SCRATCHB1 + save_tbl_base_adr1 to SCRATCHB1 WITH CARRY
	move SCRATCHB2 + save_tbl_base_adr2 to SCRATCHB2 WITH CARRY
	move SCRATCHB3 + save_tbl_base_adr3 to SCRATCHB3 WITH CARRY
	;scratchb now holds the destination address in the save tbl
	MOVE MEMORY 4, SCRATCHB_addr, cur_sav_tbl_addr1
	MOVE MEMORY 4, DSA_Addr, cur_loc_tbl_addr1

        move DMODE & Not_SIOM_Flag to DMODE     ; src:  Memory
	;move 8 bytes from the loc table to the save table
	MOVE MEMORY 8, cur_loc_tbl_addr1_ref, cur_sav_tbl_addr1_ref

        move DMODE | DIOM_Flag to DMODE         ; dest: I/O
	MOVE MEMORY 4, dptr_cnt_addr4_ref, SCRATCHB_addr
	;restore SCRATCHB (tim_cnt)

        move DMODE & Not_DIOM_Flag to DMODE     ; dest: Memory
	;int chkpoint
	return

tables_same:
        move DMODE & Not_DIOM_Flag to DMODE     ; dest: memory
	;int chkpoint
	return

restore_ptrs:
        ; SCRATCHB is initialized to NEXUS_data tim_cnt
        ; DSA initialized to NEXUS_data tim_address
	; SCRATCHA0 initialized to tim_flag
        ;Nexus_data_size = 16 bytes, then nexus_addr = base_addr + (8*nexus)
        ;equivalent to nexus left 4 times + base_addr
        ;assume nexus (tag) is in SCRATCHA3
        ;we patch base_addr during script_init, to be the
        ;dma address of our table-indirect table
        ;Since we initialize SCRATCHB to zero first, we don't need to worry
        ;about carry being set on the shifts from SCRATCHB1
        ;put NEXUS_data_addr into SCRATCHB.  Move the tim_address at
        ;NEXUS_data_addr into DSA, and the tim_cnt at NEXUS_data_addr
        ;into ld_tim_cnt and then SCRATCHB.
        MOVE 0 TO SCRATCHB0
        MOVE 0 TO SCRATCHB1
        MOVE 0 TO SCRATCHB2
        MOVE 0 TO SCRATCHB3
        MOVE SCRATCHA3 TO SFBR
        CLEAR CARRY
        MOVE SFBR SHL 0 TO SCRATCHB0
        MOVE SCRATCHB1 SHL 0 TO SCRATCHB1    ;carry?
        MOVE SCRATCHB0 SHL 0 TO SCRATCHB0
        MOVE SCRATCHB1 SHL 0 TO SCRATCHB1    ;carry?
        MOVE SCRATCHB0 SHL 0 TO SCRATCHB0
        MOVE SCRATCHB1 SHL 0 TO SCRATCHB1    ;carry?
        MOVE SCRATCHB0 SHL 0 TO SCRATCHB0
        MOVE SCRATCHB1 SHL 0 TO SCRATCHB1    ;carry?
        MOVE SCRATCHB0 + NEXUS_data_base_adr0 TO SCRATCHB0
        MOVE SCRATCHB1 + NEXUS_data_base_adr1 TO SCRATCHB1 WITH CARRY
        MOVE SCRATCHB2 + NEXUS_data_base_adr2 TO SCRATCHB2 WITH CARRY
        MOVE SCRATCHB3 + NEXUS_data_base_adr3 TO SCRATCHB3 WITH CARRY
        ; SCRATCHB = base address NEXUS_data

        move DMODE | SIOM_Flag to DMODE       ;src:  I/O
        move DMODE & Not_DIOM_Flag to DMODE   ;dest: mem
        MOVE MEMORY 4, SCRATCHB_addr, dptr_tbl_addr1
        MOVE MEMORY 4, SCRATCHB_addr, dptr_tbl_addr2

        MOVE SCRATCHB0 + 4 TO SCRATCHB0
        ;MOVE SCRATCHB1 + 0 TO SCRATCHB1 WITH CARRY
        ;MOVE SCRATCHB2 + 0 TO SCRATCHB2 WITH CARRY
        ;MOVE SCRATCHB3 + 0 TO SCRATCHB3 WITH CARRY
        ; SCRATCHB now has address of tim_cnt

        MOVE MEMORY 4, SCRATCHB_addr, dptr_cnt_addr1
        MOVE MEMORY 4, SCRATCHB_addr, dptr_cnt_addr2
        MOVE MEMORY 4, SCRATCHB_addr, dptr_cnt_addr3
        MOVE MEMORY 4, SCRATCHB_addr, dptr_cnt_addr4
        MOVE MEMORY 4, SCRATCHB_addr, dptr_cnt_addr5
        ;MOVE MEMORY 4, SCRATCHB_addr, dptr_cnt_addr6

        MOVE SCRATCHB0 + 4 TO SCRATCHB0
        ;MOVE SCRATCHB1 + 0 TO SCRATCHB1 WITH CARRY
        ;MOVE SCRATCHB2 + 0 TO SCRATCHB2 WITH CARRY
        ;MOVE SCRATCHB3 + 0 TO SCRATCHB3 WITH CARRY
        ; SCRATCHB now has address of tables_differ

        MOVE MEMORY 4, SCRATCHB_addr, dptr_diff_addr1
        MOVE MEMORY 4, SCRATCHB_addr, dptr_diff_addr2
        MOVE MEMORY 4, SCRATCHB_addr, dptr_diff_addr3

        MOVE SCRATCHB0 + 4 TO SCRATCHB0
        ;MOVE SCRATCHB1 + 0 TO SCRATCHB1 WITH CARRY
	; SCRATCHB now has the address of tim_restore
        MOVE MEMORY 4, SCRATCHB_addr, dptr_restore_addr

        move DMODE & Not_SIOM_Flag to DMODE  ; src:  mem
        move DMODE | DIOM_Flag to DMODE      ; dest: I/O

        MOVE MEMORY 4, dptr_tbl_addr1_ref, DSA_Addr
        ; DSA initialized

        MOVE MEMORY 4, dptr_cnt_addr1_ref, SCRATCHB_addr
        ; SCRATCHB now holds tim_cnt

	MOVE SCNTL2 | 0x09 to SCNTL2
	MOVE 0 to SCRATCHA2
	; clear bits 0 and 3 (WSR and WSS on NCR825)

        MOVE MEMORY 1, dptr_diff_addr2_ref, SCRATCHA0_addr
	; SCRATCHA0 now holds the tables_differ flag
	move SCRATCHA0 to SFBR
	jump tables_same if 0

        move DMODE | SIOM_Flag to DMODE         ; src: I/O
        move DMODE & Not_DIOM_Flag to DMODE     ; dest:  memory

	move 0 to SCRATCHA0
	MOVE MEMORY 1, SCRATCHA0_addr, dptr_diff_addr3_ref
	; clear tables_differ flag

	MOVE MEMORY 4, DSA_Addr, cur_loc_tbl_addr2
	; patch the destination address for the move below

	move DSA0 to SFBR
	move SFBR to SCRATCHB0
        move DSA1 & 0x0F to SFBR
	move SFBR to SCRATCHB1
	move 0 to SCRATCHB2
	move 0 to SCRATCHB3
	;scratchb holds the offset

	move SCRATCHB0 + save_tbl_base_adr0 to SCRATCHB0
	move SCRATCHB1 + save_tbl_base_adr1 to SCRATCHB1 WITH CARRY
	move SCRATCHB2 + save_tbl_base_adr2 to SCRATCHB2 WITH CARRY
	move SCRATCHB3 + save_tbl_base_adr3 to SCRATCHB3 WITH CARRY
	;scratchb now holds the source address in the save tbl
	MOVE MEMORY 4, SCRATCHB_addr, cur_sav_tbl_addr2

        move DMODE & Not_SIOM_Flag to DMODE     ; src:  memory
        move DMODE | DIOM_Flag to DMODE         ; dest: I/O

	MOVE MEMORY 4, dptr_restore_addr_ref, SCRATCHB_addr
	;scratchb now holds tim_restore
        ;multiply by 8 (size of an iovec)
	clear CARRY
	move SCRATCHB0 SHL 0 to SCRATCHB0
	move SCRATCHB1 SHL 0 to SCRATCHB1
	move SCRATCHB0 SHL 0 to SCRATCHB0
	move SCRATCHB1 SHL 0 to SCRATCHB1
	move SCRATCHB0 SHL 0 to SCRATCHB0
	move SCRATCHB1 SHL 0 to SCRATCHB1
	;scratchb now holds the number of bytes to move

        move DMODE | SIOM_Flag to DMODE         ; src: I/O
        move DMODE & Not_DIOM_Flag to DMODE     ; dest:  memory

	MOVE MEMORY 2, SCRATCHB_addr, restore_patch_addr
	;int chkpoint

        move DMODE & Not_SIOM_Flag to DMODE     ; src:  memory
	MOVE MEMORY restore_patch, cur_sav_tbl_addr2_ref, cur_loc_tbl_addr2_ref

        move DMODE | DIOM_Flag to DMODE         ; dest: I/O
	MOVE MEMORY 4, dptr_cnt_addr5_ref, SCRATCHB_addr
	;restore SCRATCHB to hold tim count

        move DMODE & Not_DIOM_Flag to DMODE     ; dest:  memory
	return

; The extended message is shown by the msg being a 0x01.  The first byte is the
; message length, and the second byte is the opcode that the processor will
; look at to see what the device was trying to say.
;
; Note: We currently do not support the MODIFY DATA POINTER message from
;       the target.  Consequently, we must reject that message.  This must
;       be done by raising ATN for the message reject *prior* to clearing ACK
;       for the message.  This needs to be implemented here!!!  FIXME

ext_msg_handler:
        clear ACK
        move 2, extended_msg_addr, when MSG_IN
        int ext_msg

complete_ext_msg:
        clear ACK
        move ext_msg_size, extended_msg_addr, when MSG_IN

; This nop may be patched to one of several possible interrupts by
; the device driver as follows:
;   target_wdtr_sent - if the target is negotiating data xfer width
;   target_sdtr_sent - if the target is negotiating data xfer speed
;   modify_data_ptr  - if the target has sent us a modify data pointer
;   unknown_ext_msg  - if the target has sent an extended message opcode
;			that we don't understand
; Otherwise it is a nop to complete the extended message and fall into
; the phase_vector jumps (SCSI-1 Extended Identify is handled this way).

ext_msg_patch:
        nop ext_msg_patch
        clear ACK

; We check for five possible cases.  The default case we drop into is the CMD
; phase.  So we output the CMD and then check for all the possible phases that
; could occur as a result of that command.

phase_vector:
        jump msg_hdlr when MSG_IN
        jump status_complete if STATUS
        jump receive_data if DATA_IN
        jump send_data if DATA_OUT
        jump send_command if CMD
	int phase_error


; Here we get the status byte in, we don't need to see what type of SCSI status
; we've got because that is up to the device driver to decide what to do with
; the status.  We are concerned with just correctly getting the status from the
; device.  We now go through the phases possible after receiving the status.
; This is the cleanup required before we disconnect from the target.

status_complete:
        move 1, status_addr, when STATUS
        int phase_error, when not MSG_IN
        move 1, cmd_msg_in_addr, when MSG_IN
        ;clear the SDU bit
        move SCNTL2 & 0x7F to SCNTL2
	clear ACK
	wait disconnect

	;update the nexus regarding the number of tims remaining
        move DMODE | SIOM_Flag to DMODE         ; src:  I/O
        MOVE MEMORY 4, SCRATCHB_addr, dptr_cnt_addr2_ref
        move DMODE & Not_SIOM_Flag to DMODE

	;for wide devices, this "int io_done" instruction 
	;is patched to the scratcha2 register move instruction, so
	;that we can check for and handle bytes that weren't 
	;transferred.
cmd_complete_patch:
	int io_done
	;move SCRATCHA2 & 0x09 to SFBR

	;if WSS or WSR are set, let the host process it.
        int io_done_wsr, if 0x01
	int io_done_wss, if 0x08
        int io_done

; Send (write) data to the target.  After WDTR negotiation, patch
; the call instruction so that it calls either the output_data or
; the output_wide_data subroutines
send_data:
        call output_data when DATA_OUT
        jump after_data_move_check

; Receive (read) data from the target
receive_data:
        call input_data when DATA_IN


; After a data in or data out phase, two cases can occur.  First, we have
; transferred all the bytes that the device expects and wants from us.  In this
; case, we might get back an immediate STATUS back from the device.  However,
; the device may want to send in the STATUS byte later on.  In that case, the
; target would be sending a MSG_IN to us to start the sequence leading up to
; disconnection.  This also covers the second case in which we have not
; transferred all the data necessary, but we give up the bus because the target
; device likes to disconnect between major transfers of data.  Thus, we would
; also need the MSG_IN phase to start the disconnect sequence mentioned above.

after_data_move_check:
        jump status_complete when STATUS
        jump msg_hdlr if MSG_IN
        int phase_error

disconnect_point:
        ;clear the SDU bit
        move SCNTL2 & 0x7F to SCNTL2
        clear ACK
        wait disconnect
        move SCRATCHB0 to SFBR
	jump not_last_data_xfer if NOT 0
	move SCRATCHB1 to SFBR
	jump not_last_data_xfer if NOT 0
	;save the pointers, this is the last data transfer
	;clear SCRATCHA0 since we don't care if the tables differ,
	;since we aren't going to have any more DATA phases
	move 0x00 to SCRATCHA0
save_ptrs_patch1:
	call save_pointers
not_last_data_xfer:
        nop check_next_io       ; will be patched to int if other devices
                                ; are waiting to be started
        jump wait_reselect      ; else, go wait for reselection


reject_target_sync:
        set ATN
        move scntl3_patch to SCNTL3
        move sxfer_patch to SXFER
        clear ACK
reject_loop:
        move 1, reject_msg_addr, when MSG_OUT
	jump reject_loop_atn when MSG_OUT
        jump phase_vector

reject_loop_atn:
	set ATN
	jump reject_loop

reject_cleanup:
        clear ATN
        int sync_msg_reject
goto_cleanup:
        clear ACK
        jump cleanup_phase

renegotiate_wdtr:
	set ATN
        move scntl3_patch to SCNTL3
        move sxfer_patch to SXFER
	clear ACK
renegotiate_wdtr_loop:
        move 9, wdtr_msg_out_addr, when MSG_OUT
	jump renegotiate_wdtr_loop_atn when MSG_OUT
	jump negotiation_phase_patch

renegotiate_wdtr_loop_atn:
	set ATN
	jump renegotiate_wdtr_loop

renegotiate_sdtr:
        set ATN
        move scntl3_patch to SCNTL3
        move sxfer_patch to SXFER
        clear ACK
        int phase_error, when not MSG_OUT
renegotiate_sdtr_loop:
        move 5, sdtr_msg_out_addr, when MSG_OUT
	jump renegotiate_sdtr_loop_atn when MSG_OUT
        jump phase_vector

renegotiate_sdtr_loop_atn:
	set ATN
	jump renegotiate_sdtr_loop

failed_selection_hdlr:
        ;manually clear Start bit to compensate for chip problem
        move SCNTL0 & 0xDF to SCNTL0
        move tag_patch to SCRATCHA3
        move CTEST2 | 0x00 to CTEST2
        int cmd_select_atn_failed
end_failed_sel_hdlr:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SYNC.SS
;       This SCRIPT contains the negotiation sequence for synchronous data
;       data transfer.  If it receives a message reject at the appropriate portion
;       of the code, then we send a message to the host that the target device
;       is a asynchronous device
;
; The following portions of code is for the SYNC negotiations
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Arbitrate and select the target with the SCSI id written into the current
; zero position.  If during the selection, another target (including the target
; we were trying to select) reselects the chip, then we jump to the script at
; the address failed_sync_selection_hdlr.  This script does a simple interrupt
; so that the process interrupt handler will see that this script never got
; started and needs to be restarted at a later time.  The interrupt handler
; will then start the SIOP at the script address of job_wait_reselection (also
; known as the IO_WAIT script) to start the handling of the reselection by the
; target.

begin_negotiation:
        select  ATN target_id, failed_negotiation_selection_hdlr
        move tag_patch to SCRATCHA3
        move CTEST2 | 0x00 to CTEST2
        jump start_negotiation_msg_out_loop when MSG_OUT
        clear ATN
        int phase_error

start_negotiation_msg_out_loop:
        move 5, neg_msg_addr, when MSG_OUT
; Move the identify message, any tag info, and first negotiation message

negotiation_phase_patch:
        jump start_sdtr ;patched to nop for wide negotiation
        jump start_wdtr_msg_in_phase, when MSG_IN
        jump start_negotiation_msg_out_atn, if MSG_OUT
	int wdtr_msg_ignored
	jump cleanup_phase

start_negotiation_msg_out_atn:
	set ATN
	jump start_negotiation_msg_out_loop

start_wdtr_msg_in_phase:
        move 1, extended_msg_addr, when MSG_IN
        jump wdtr_msg_in_rejected if 0x07

; 0x07 is a message reject.  Jump to an intermediate point to 
; ACK the byte, then continue.
; A 0x01 indicates that we will be getting an extended message.  We expect to
; get one back in response to our extended message asking for wide data
; transfer.  Interrupt the host to process, it will then restart the script
; and set ATN before ACKing the byte.  If it is an extended message, ACK the
; byte and move in the rest of the message.
        int err_not_ext_msg, if not 01
        clear ACK
        move 3, extended_msg_addr, when MSG_IN
        int wdtr_neg_done 
        set ATN
        clear ACK
        jump start_negotiation_msg_out_loop

start_sdtr:
        jump start_sdtr_msg_in_phase, when MSG_IN
        jump start_negotiation_msg_out_atn, if MSG_OUT
	int sdtr_msg_ignored
	jump cleanup_phase

start_sdtr_msg_in_phase:
        move 1, extended_msg_addr, when MSG_IN
        jump sdtr_msg_in_rejected if 0x07
        int err_not_ext_msg, if not 01
        clear ACK
        move 4, extended_msg_addr, when MSG_IN
        int sdtr_neg_done 
        clear ACK
        jump cleanup_phase

wdtr_msg_in_rejected:
	int wdtr_msg_reject
        set ATN
        clear ACK
        jump start_negotiation_msg_out_loop
        
sdtr_msg_in_rejected:
	int sdtr_msg_reject
        clear ACK
        jump cleanup_phase
        
failed_negotiation_selection_hdlr:
        ;manually clear Start bit to compensate for chip problem
        move SCNTL0 & 0xDF to SCNTL0
        ;patch the tag so the host knows what command lost negotiation
        move tag_patch to SCRATCHA3
        ; clear sigp if it was set, we are going to interrupt the host anyway
        move CTEST2 | 0X00 to CTEST2
        int neg_select_failed
end_failed_neg_sel_hdlr:


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ABORT SCRIPT
; This sends out the abort message to the target device.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;The get_to_msg_out routine allows the target to
;complete the current phase before switching to
;MSG_OUT.  Any bytes moved are garbage bytes.
get_to_msg_out:
	jump rel(abdr2_msg_in), if MSG_IN
	jump rel(abdr2_status), if STATUS
	jump rel(abdr2_data_in), if DATA_IN
	jump rel(abdr2_data_out), if DATA_OUT

abdr2_cmd:
	move 1, abort_bdr_msg_in_addr, when COMMAND
        jump rel(abdr2_cmd), when COMMAND
	return
abdr2_msg_in:
        move 1, abort_bdr_msg_in_addr, when MSG_IN
        clear ACK
	jump rel(abdr2_msg_in), when MSG_IN
	return
abdr2_status:
        move 1, abort_bdr_msg_in_addr, when STATUS
        clear ACK
	return
abdr2_data_in:
        move 1, abort_bdr_msg_in_addr, when DATA_IN
        clear ACK
	jump rel(abdr2_data_in), when DATA_IN
	return
abdr2_data_out:
        move 1, abort_bdr_msg_in_addr, when DATA_OUT
	jump rel(abdr2_data_out), when DATA_OUT
	return

; Arbitrate and select the target with the SCSI id written into the current
; zero position.  If during the selection, another target (including the target
; we were trying to select) reselects the chip, then we jump to the script at
; the address failed_abort_bdr_selection_hdlr.  This script does a simple
; interrupt so that the process interrupt handler will see that this script
; never got started and needs to be restarted at a later time.  The interrupt
; handler will then start the SIOP at the script address of iowait_entry_point
; (also known as the IO_WAIT script) to start the handling of the reselection
; by the target.

bdr_sequence:
        select ATN target_id, failed_abort_bdr_selection_hdlr
        ; put the qtag we will be using into SCRATCHA3
        move abdr_tag_patch to SCRATCHA3

        ; clear sigp if it was set, we are going to interrupt the host anyway
        move CTEST2 | 0X00 to CTEST2

        jump start_bdr_msg_out_phase when MSG_OUT
        clear ATN
        int phase_error

abort_sequence:
        select ATN target_id, failed_abort_bdr_selection_hdlr
        ; put the qtag we will be using into SCRATCHA3
        move abdr_tag_patch to SCRATCHA3

        ; clear sigp if it was set, we are going to interrupt the host anyway
        move CTEST2 | 0X00 to CTEST2
        jump start_abort_msg_out_phase when MSG_OUT
        clear ATN
        int phase_error

start_abort_msg_out_phase:
        move 2, abort_bdr_msg_out_addr, when MSG_OUT
	jump wait_for_bus_free

abdr2_sequence:
        set ATN
        clear ACK
        jump start_abdr2_msg_out_phase when MSG_OUT
	call rel(get_to_msg_out)
        jump start_abdr2_msg_out_phase when MSG_OUT
        clear ATN
        int phase_error

start_abdr2_msg_out_phase:
        move 1, abort_bdr_msg_out_addr, when MSG_OUT
wait_for_bus_free:
	clear ATN
	;clear the SDU bit
	move SCNTL2 & 0x7F to SCNTL2
	wait disconnect
	int abort_io_complete
end_wait_for_bus_free:

start_bdr_msg_out_phase:
        move 1, abort_bdr_msg_out_addr, when MSG_OUT
        jump wait_for_bus_free

failed_abort_bdr_selection_hdlr:
        ;manually clear Start bit to compensate for chip problem
        move SCNTL0 & 0xDF to SCNTL0
        move abdr_tag_patch to SCRATCHA3
        move CTEST2 | 0x00 to CTEST2
        int abort_select_failed
end_failed_abdr_sel_hdlr:

; The following two subroutines are used to move data to and from the SCSI
; bus.  These routines use the Table Indirect Move feature of the 53C8XX.  The
; address of this table is in tim_addr and the size of the table (in # of
; entries) is in tim_cnt.  These two The tim_addr is moved to the DSA register
; (where it is used by the chip and the tim_cnt is moved to the CTEST0 register
; and is used as a loop counter.  The current loop counter value is copied back
; into the tim_cnt field each time through the loop.
;
; After the registers are set up the data transfer is started.  Note that the
; routines will loop till either all of the entries in the table have been
; processed or the target signals a change of phase.

output_data:
        move DMODE & Not_DIOM_Flag to DMODE

output_data0:
        ;SCRATCHB0 and SCRATCHB1 hold the counter
        move SCRATCHB0 to SFBR
        jump rel(not_zero), if NOT 0
        move SCRATCHB1 to SFBR
        return, if 0
not_zero:
        move from 0, when DATA_OUT     ;move the data
        move DSA0 +8 to DSA0                    ; Pt to next TI entry
        move DSA1 +0 to DSA1 WITH CARRY

        move SCRATCHB0 -1 to SCRATCHB0          ; Update the counter
        move SCRATCHB1 -1 to SCRATCHB1 WITH CARRY

        return when not DATA_OUT                  ; change of phase
        jump output_data0              ; if more TIM entries

;the very last transfer should not be a chained move, we
;want all bytes moved to the scsi bus
output_wide_data:
        ; Initialize chip registers from system memory
        move DMODE & Not_DIOM_Flag to DMODE

output_wide_data0:
        ;SCRATCHB0 and SCRATCHB1 hold the counter
        move SCRATCHB1 to SFBR
	jump rel(chain_send), if NOT 0
	move SCRATCHB0 to SFBR
	return, if 0
        jump block_send, if 1
chain_send:
        chmov from 0, when DATA_OUT     ;chained move of the data
next_wide_TIM:
        move DSA0 +8 to DSA0            ; Pt to next TI entry
        move DSA1 +0 to DSA1 WITH CARRY

        move SCRATCHB0 -1 to SCRATCHB0  ; Update the counter
        move SCRATCHB1 -1 to SCRATCHB1 WITH CARRY

scntl2_patch_out:
	jump save_scntl2, when NOT DATA_OUT
        ;return when not DATA_OUT        ; change of phase
        jump output_wide_data0          ; if more TIM entries

block_send:
        move from 0, when DATA_OUT      ;block move of the data
	jump next_wide_TIM

input_data:
        ; Initialize chip registers from system memory
        move DMODE & Not_DIOM_Flag to DMODE
input_data0:
        ;SCRATCHB0 and SCRATCHB1 hold the counter
        move SCRATCHB0 to SFBR
        jump rel(in_not_zero), if NOT 0
        move SCRATCHB1 to SFBR
        return, if 0
in_not_zero:
	;after WDTR negotiation, patch this to be a chained move
	;instruction if we are using 16 bit data transfers
        move from 0, when DATA_IN
        move DSA0 +8 to DSA0                    ; Pt to next TI entry
        move DSA1 +0 to DSA1 WITH CARRY

        move SCRATCHB0 -1 to SCRATCHB0          ; Update the counter
        move SCRATCHB1 -1 to SCRATCHB1 WITH CARRY

scntl2_patch_in:
	jump save_scntl2, when NOT DATA_IN
        ;return when not DATA_IN                   
        jump input_data0                        ; if more TIM entries

save_scntl2:
	move SCNTL2 & 0x09 to SFBR
	move SFBR to SCRATCHA2
	move SWIDE to SFBR		;save SWIDE
	move SFBR to SCRATCHA1
	return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cmd_msg_in_buf:
        nop cmd_msg_in_buf
cmd_buf:
        nop cmd_buf
        nop cmd_buf
status_buf:
        nop status_buf
identify_msg_buf:
        nop identify_msg_buf
neg_msg_buf:
        nop neg_msg_buf
wdtr_msg_out_buf:
        nop wdtr_msg_out_buf
sdtr_msg_out_buf:
        nop sdtr_msg_out_buf
extended_msg_buf:
        nop extended_msg_buf
abort_bdr_msg_out_buf:
        nop abort_bdr_msg_out_buf
abort_bdr_msg_in_buf:
        nop abort_bdr_msg_in_buf
;lun_msg_buf:
;        nop lun_msg_buf
reject_msg_buf:
        nop reject_msg_buf
tag_msg_buf:
	nop tag_msg_buf
