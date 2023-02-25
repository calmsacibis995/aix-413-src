/* @(#)48	1.2  src/rspc/kernext/pci/ncr810/ncr8xxss.h, pciscsi, rspc41J, 9513A_all 3/28/95 14:57:00 */

/*
 * COMPONENT_NAME: (PCISCSI) IBM SCSI Adapter Driver (NCR 53C8XX)
 *		SCRIPTS header file
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Note: This file was machine generated from the output file created
 *	 by the NCR NASM SCSI SCRIPTS compiler -- DO NOT EDIT -- 
 *	 To change, you should edit the ncr8xx.ss SCRIPTS source file,
 *	 recompile it, and rerun ncr8xxcnvt against the compiler's
 *	 output to regenerate this file.
 *
 * Created with ncr8xxcnvt on Tue Mar 28 14:52:24 CST 1995.
 */


typedef unsigned long ULONG;

#define E_abort_bdr_msg_out_addr	0x00000000L
#define E_cmd_addr	0x00000000L
#define E_cmd_msg_in_addr	0x00000000L
#define E_DSA_Addr	0x00000000L
#define E_extended_msg_addr	0x00000000L
#define E_identify_msg_addr	0x00000000L
#define E_neg_msg_addr	0x00000000L
#define E_abort_bdr_msg_in_addr	0x00000000L
#define E_reject_msg_addr	0x00000000L
#define E_wdtr_msg_out_addr	0x00000000L
#define E_restore_patch_addr	0x00000000L
#define E_SCRATCHA0_addr	0x00000000L
#define E_SCRATCHB_addr	0x00000000L
#define E_sdtr_msg_out_addr	0x00000000L
#define E_status_addr	0x00000000L
#define E_tag_msg_addr	0x00000000L
#define E_wait_reselect	0x00000000L
#define R_cmd_bytes_out_count	0x00000000L
#define R_ext_msg_size	0x00000000L
#define R_target_id	0x00000001L
#define R_sxfer_patch	0x00000008L
#define R_scntl3_patch	0x00000013L
#define R_tag_patch	0x00000099L
#define R_abdr_tag_patch	0x000000AAL
#define A_phase_error	0x00000001L
#define A_save_ptrs_wsr	0x00000004L
#define A_save_ptrs_wss	0x00000005L
#define A_io_done	0x00000006L
#define A_io_done_wsr	0x00000007L
#define A_io_done_wss	0x00000008L
#define A_unknown_msg	0x00000009L
#define A_ext_msg	0x0000000AL
#define A_check_next_io	0x0000000BL
#define A_DIOM_Flag	0x00000010L
#define A_cmd_select_atn_failed	0x00000012L
#define A_err_not_ext_msg	0x00000013L
#define A_sync_msg_reject	0x00000016L
#define A_neg_select_failed	0x00000018L
#define A_abort_select_failed	0x00000019L
#define A_abort_io_complete	0x0000001AL
#define A_SIOM_Flag	0x00000020L
#define A_wdtr_msg_ignored	0x00000021L
#define A_sdtr_msg_ignored	0x00000022L
#define A_wdtr_neg_done	0x00000023L
#define A_sdtr_neg_done	0x00000024L
#define A_wdtr_msg_reject	0x00000025L
#define A_sdtr_msg_reject	0x00000026L
#define A_ignore_residue	0x00000027L
#define A_NEXUS_data_base_adr3	0x00000088L
#define A_NEXUS_data_base_adr1	0x00000088L
#define A_NEXUS_data_base_adr2	0x00000088L
#define A_NEXUS_data_base_adr0	0x00000088L
#define A_save_tbl_base_adr1	0x00000099L
#define A_save_tbl_base_adr3	0x00000099L
#define A_save_tbl_base_adr2	0x00000099L
#define A_save_tbl_base_adr0	0x00000099L
#define A_Not_SIOM_Flag	0x000000DFL
#define A_Not_DIOM_Flag	0x000000EFL
#define A_restore_patch	0x0000AAAAL
#define A_dptr_tbl_addr1_ref	0x01010101L
#define A_dptr_cnt_addr1_ref	0x02020202L
#define A_dptr_cnt_addr2_ref	0x03030303L
#define A_dptr_tbl_addr2_ref	0x04040404L
#define A_dptr_cnt_addr3_ref	0x05050505L
#define A_dptr_cnt_addr5_ref	0x06060606L
#define A_cur_sav_tbl_addr1_ref	0x07070707L
#define A_cur_loc_tbl_addr1_ref	0x08080808L
#define A_dptr_cnt_addr4_ref	0x09090909L
#define A_dptr_diff_addr1_ref	0x0A0A0A0AL
#define A_dptr_diff_addr2_ref	0x0B0B0B0BL
#define A_cur_loc_tbl_addr2_ref	0x0C0C0C0CL
#define A_cur_sav_tbl_addr2_ref	0x0D0D0D0DL
#define A_dptr_diff_addr3_ref	0x0E0E0E0EL
#define A_dptr_restore_addr_ref	0x0F0F0F0FL
#define A_dptr_tbl_addr1	0x11111111L
#define A_dptr_cnt_addr1	0x22222222L
#define A_dptr_cnt_addr2	0x33333333L
#define A_dptr_tbl_addr2	0x44444444L
#define A_dptr_cnt_addr3	0x55555555L
#define A_dptr_cnt_addr5	0x66666666L
#define A_cur_sav_tbl_addr1	0x77777777L
#define A_cur_loc_tbl_addr1	0x88888888L
#define A_dptr_cnt_addr4	0x99999999L
#define A_dptr_diff_addr1	0xAAAAAAAAL
#define A_dptr_diff_addr2	0xBBBBBBBBL
#define A_cur_loc_tbl_addr2	0xCCCCCCCCL
#define A_cur_sav_tbl_addr2	0xDDDDDDDDL
#define A_dptr_diff_addr3	0xEEEEEEEEL
#define A_dptr_restore_addr	0xFFFFFFFFL
#define Ent_abdr2_sequence   	0x00000B1CL
#define Ent_abort_bdr_msg_in_buf	0x00000D5CL
#define Ent_abort_bdr_msg_out_buf	0x00000D54L
#define Ent_abort_sequence   	0x00000ADCL
#define Ent_after_data_move_check	0x00000704L
#define Ent_bdr_sequence     	0x00000AACL
#define Ent_begin_negotiation	0x00000894L
#define Ent_cleanup_phase    	0x00000128L
#define Ent_cmd_buf          	0x00000D14L
#define Ent_cmd_complete_patch	0x000006CCL
#define Ent_cmd_msg_in_buf   	0x00000D0CL
#define Ent_complete_ext_msg 	0x00000630L
#define Ent_end_failed_abdr_sel_hdlr	0x00000BACL
#define Ent_end_failed_neg_sel_hdlr	0x00000A04L
#define Ent_end_failed_sel_hdlr	0x00000894L
#define Ent_end_wait_for_bus_free	0x00000B7CL
#define Ent_ext_msg_patch    	0x00000640L
#define Ent_extended_msg_buf 	0x00000D4CL
#define Ent_failed_abort_bdr_selection_hdlr	0x00000B8CL
#define Ent_failed_negotiation_selection_hdlr	0x000009E4L
#define Ent_failed_selection_hdlr	0x00000874L
#define Ent_goto_cleanup     	0x000007CCL
#define Ent_identify_msg_buf 	0x00000D2CL
#define Ent_in_not_zero      	0x00000CACL
#define Ent_message_loop     	0x00000120L
#define Ent_msg_done         	0x000001F0L
#define Ent_neg_msg_buf      	0x00000D34L
#define Ent_negotiation_phase_patch	0x000008CCL
#define Ent_output_data      	0x00000BACL
#define Ent_output_wide_data 	0x00000C0CL
#define Ent_phase_error_entry	0x000001A0L
#define Ent_regular_phase_hdlr	0x00000180L
#define Ent_reject_loop      	0x00000794L
#define Ent_reject_msg_buf   	0x00000D64L
#define Ent_reject_target_sync	0x00000774L
#define Ent_renegotiate_sdtr 	0x00000824L
#define Ent_renegotiate_sdtr_loop	0x0000084CL
#define Ent_renegotiate_wdtr 	0x000007DCL
#define Ent_renegotiate_wdtr_loop	0x000007FCL
#define Ent_save_pointers    	0x00000248L
#define Ent_save_pointers_wide	0x00000230L
#define Ent_save_ptrs_patch  	0x000001E8L
#define Ent_save_ptrs_patch1 	0x0000075CL
#define Ent_save_scntl2      	0x00000CE4L
#define Ent_scntl2_patch_in  	0x00000CD4L
#define Ent_scntl2_patch_out 	0x00000C64L
#define Ent_script_reconnect_point	0x000000B0L
#define Ent_scripts_entry_point	0x00000100L
#define Ent_sdtr_msg_out_buf 	0x00000D44L
#define Ent_send_data        	0x000006ECL
#define Ent_start_negotiation_msg_out_loop	0x000008C4L
#define Ent_start_sdtr       	0x0000094CL
#define Ent_start_sdtr_msg_in_phase	0x0000096CL
#define Ent_start_wdtr_msg_in_phase	0x00000904L
#define Ent_status_buf       	0x00000D24L
#define Ent_tables_same      	0x00000344L
#define Ent_tag_msg_buf      	0x00000D6CL
#define Ent_unknown_msg_hdlr 	0x00000210L
#define Ent_wait_for_bus_free	0x00000B5CL
#define Ent_wdtr_msg_out_buf 	0x00000D3CL
#define E_scsi_0_lun	0x00000000L
#define E_scsi_1_lun	0x00000000L
#define E_scsi_2_lun	0x00000000L
#define E_scsi_3_lun	0x00000000L
#define E_scsi_4_lun	0x00000000L
#define E_scsi_5_lun	0x00000000L
#define E_scsi_6_lun	0x00000000L
#define E_lun_msg_addr	0x00000000L
#define E_scsi_7_lun	0x00000000L
#define E_scsi_F_lun	0x00000000L
#define E_scsi_8_lun	0x00000000L
#define E_scsi_9_lun	0x00000000L
#define E_scsi_A_lun	0x00000000L
#define E_scsi_B_lun	0x00000000L
#define E_scsi_C_lun	0x00000000L
#define E_scsi_D_lun	0x00000000L
#define E_scsi_E_lun	0x00000000L
#define A_unknown_reselect_id	0x0000001DL
#define A_uninitialized_reselect	0x0000001EL
#define A_suspended	0x00000020L
#define Ent_iowait_entry_point	0x00000000L
#define Ent_lun_msg_buf      	0x000005B0L
#define Ent_scsi_id_patch    	0x00000018L
