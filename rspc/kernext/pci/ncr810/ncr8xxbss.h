/* @(#)40	1.2  src/rspc/kernext/pci/ncr810/ncr8xxbss.h, pciscsi, rspc41J, 9513A_all 3/28/95 14:57:37 */

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


ULONG R_cmd_bytes_out_count_Used[] = {
	0x0000005EL
};

ULONG R_ext_msg_size_Used[] = {
	0x0000018EL
};

ULONG R_sxfer_patch_Used[] = {
	0x00000030L,
	0x0000004EL,
	0x000001E1L,
	0x000001FBL,
	0x0000020DL
};

ULONG R_scntl3_patch_Used[] = {
	0x0000002EL,
	0x0000004CL,
	0x000001DFL,
	0x000001F9L,
	0x0000020BL
};

ULONG R_tag_patch_Used[] = {
	0x00000042L,
	0x0000021FL,
	0x00000227L,
	0x0000027BL
};

ULONG R_abdr_tag_patch_Used[] = {
	0x000002ADL,
	0x000002B9L,
	0x000002E5L
};

ULONG	INSTRUCTIONS	= 0x000001A0L;
ULONG E_scsi_0_lun_Used[] = {
	0x0000002BL,
	0x0000002DL,
	0x0000002FL,
	0x00000031L,
	0x00000033L,
	0x00000035L,
	0x00000037L,
	0x00000039L
};

ULONG E_scsi_1_lun_Used[] = {
	0x0000003FL,
	0x00000041L,
	0x00000043L,
	0x00000045L,
	0x00000047L,
	0x00000049L,
	0x0000004BL,
	0x0000004DL
};

ULONG E_scsi_2_lun_Used[] = {
	0x00000053L,
	0x00000055L,
	0x00000057L,
	0x00000059L,
	0x0000005BL,
	0x0000005DL,
	0x0000005FL,
	0x00000061L
};

ULONG E_scsi_3_lun_Used[] = {
	0x00000067L,
	0x00000069L,
	0x0000006BL,
	0x0000006DL,
	0x0000006FL,
	0x00000071L,
	0x00000073L,
	0x00000075L
};

ULONG E_scsi_4_lun_Used[] = {
	0x0000007BL,
	0x0000007DL,
	0x0000007FL,
	0x00000081L,
	0x00000083L,
	0x00000085L,
	0x00000087L,
	0x00000089L
};

ULONG E_scsi_5_lun_Used[] = {
	0x0000008FL,
	0x00000091L,
	0x00000093L,
	0x00000095L,
	0x00000097L,
	0x00000099L,
	0x0000009BL,
	0x0000009DL
};

ULONG E_scsi_6_lun_Used[] = {
	0x000000A3L,
	0x000000A5L,
	0x000000A7L,
	0x000000A9L,
	0x000000ABL,
	0x000000ADL,
	0x000000AFL,
	0x000000B1L
};

ULONG E_scsi_7_lun_Used[] = {
	0x000000B7L,
	0x000000B9L,
	0x000000BBL,
	0x000000BDL,
	0x000000BFL,
	0x000000C1L,
	0x000000C3L,
	0x000000C5L
};

ULONG E_scsi_F_lun_Used[] = {
	0x00000157L,
	0x00000159L,
	0x0000015BL,
	0x0000015DL,
	0x0000015FL,
	0x00000161L,
	0x00000163L,
	0x00000165L
};

ULONG E_scsi_8_lun_Used[] = {
	0x000000CBL,
	0x000000CDL,
	0x000000CFL,
	0x000000D1L,
	0x000000D3L,
	0x000000D5L,
	0x000000D7L,
	0x000000D9L
};

ULONG E_scsi_9_lun_Used[] = {
	0x000000DFL,
	0x000000E1L,
	0x000000E3L,
	0x000000E5L,
	0x000000E7L,
	0x000000E9L,
	0x000000EBL,
	0x000000EDL
};

ULONG E_scsi_A_lun_Used[] = {
	0x000000F3L,
	0x000000F5L,
	0x000000F7L,
	0x000000F9L,
	0x000000FBL,
	0x000000FDL,
	0x000000FFL,
	0x00000101L
};

ULONG E_scsi_B_lun_Used[] = {
	0x00000107L,
	0x00000109L,
	0x0000010BL,
	0x0000010DL,
	0x0000010FL,
	0x00000111L,
	0x00000113L,
	0x00000115L
};

ULONG E_scsi_C_lun_Used[] = {
	0x0000011BL,
	0x0000011DL,
	0x0000011FL,
	0x00000121L,
	0x00000123L,
	0x00000125L,
	0x00000127L,
	0x00000129L
};

ULONG E_scsi_D_lun_Used[] = {
	0x0000012FL,
	0x00000131L,
	0x00000133L,
	0x00000135L,
	0x00000137L,
	0x00000139L,
	0x0000013BL,
	0x0000013DL
};

ULONG E_scsi_E_lun_Used[] = {
	0x00000143L,
	0x00000145L,
	0x00000147L,
	0x00000149L,
	0x0000014BL,
	0x0000014DL,
	0x0000014FL,
	0x00000151L
};

extern ULONG A_phase_error_Used[];
extern ULONG A_save_ptrs_wsr_Used[];
extern ULONG A_save_ptrs_wss_Used[];
extern ULONG A_io_done_Used[];
extern ULONG A_io_done_wsr_Used[];
extern ULONG A_io_done_wss_Used[];
extern ULONG A_unknown_msg_Used[];
extern ULONG A_ext_msg_Used[];
extern ULONG A_check_next_io_Used[];
extern ULONG A_DIOM_Flag_Used[];
extern ULONG A_cmd_select_atn_failed_Used[];
extern ULONG A_err_not_ext_msg_Used[];
extern ULONG A_sync_msg_reject_Used[];
extern ULONG A_neg_select_failed_Used[];
extern ULONG A_abort_select_failed_Used[];
extern ULONG A_abort_io_complete_Used[];
extern ULONG A_SIOM_Flag_Used[];
extern ULONG A_wdtr_msg_ignored_Used[];
extern ULONG A_sdtr_msg_ignored_Used[];
extern ULONG A_wdtr_neg_done_Used[];
extern ULONG A_sdtr_neg_done_Used[];
extern ULONG A_wdtr_msg_reject_Used[];
extern ULONG A_sdtr_msg_reject_Used[];
extern ULONG A_ignore_residue_Used[];
extern ULONG A_NEXUS_data_base_adr3_Used[];
extern ULONG A_NEXUS_data_base_adr1_Used[];
extern ULONG A_NEXUS_data_base_adr2_Used[];
extern ULONG A_NEXUS_data_base_adr0_Used[];
extern ULONG A_save_tbl_base_adr1_Used[];
extern ULONG A_save_tbl_base_adr3_Used[];
extern ULONG A_save_tbl_base_adr2_Used[];
extern ULONG A_save_tbl_base_adr0_Used[];
extern ULONG A_Not_SIOM_Flag_Used[];
extern ULONG A_Not_DIOM_Flag_Used[];
extern ULONG A_restore_patch_Used[];
extern ULONG A_dptr_tbl_addr1_ref_Used[];
extern ULONG A_dptr_cnt_addr1_ref_Used[];
extern ULONG A_dptr_cnt_addr2_ref_Used[];
extern ULONG A_dptr_tbl_addr2_ref_Used[];
extern ULONG A_dptr_cnt_addr3_ref_Used[];
extern ULONG A_dptr_cnt_addr5_ref_Used[];
extern ULONG A_cur_sav_tbl_addr1_ref_Used[];
extern ULONG A_cur_loc_tbl_addr1_ref_Used[];
extern ULONG A_dptr_cnt_addr4_ref_Used[];
extern ULONG A_dptr_diff_addr1_ref_Used[];
extern ULONG A_dptr_diff_addr2_ref_Used[];
extern ULONG A_cur_loc_tbl_addr2_ref_Used[];
extern ULONG A_cur_sav_tbl_addr2_ref_Used[];
extern ULONG A_dptr_diff_addr3_ref_Used[];
extern ULONG A_dptr_restore_addr_ref_Used[];
extern ULONG A_dptr_tbl_addr1_Used[];
extern ULONG A_dptr_cnt_addr1_Used[];
extern ULONG A_dptr_cnt_addr2_Used[];
extern ULONG A_dptr_tbl_addr2_Used[];
extern ULONG A_dptr_cnt_addr3_Used[];
extern ULONG A_dptr_cnt_addr5_Used[];
extern ULONG A_cur_sav_tbl_addr1_Used[];
extern ULONG A_cur_loc_tbl_addr1_Used[];
extern ULONG A_dptr_cnt_addr4_Used[];
extern ULONG A_dptr_diff_addr1_Used[];
extern ULONG A_dptr_diff_addr2_Used[];
extern ULONG A_cur_loc_tbl_addr2_Used[];
extern ULONG A_cur_sav_tbl_addr2_Used[];
extern ULONG A_dptr_diff_addr3_Used[];
extern ULONG A_dptr_restore_addr_Used[];
extern ULONG A_unknown_reselect_id_Used[];
extern ULONG A_uninitialized_reselect_Used[];
extern ULONG A_suspended_Used[];
