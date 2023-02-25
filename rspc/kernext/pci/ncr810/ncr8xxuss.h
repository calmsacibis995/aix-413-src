/* @(#)53	1.2  src/rspc/kernext/pci/ncr810/ncr8xxuss.h, pciscsi, rspc41J, 9513A_all 3/28/95 14:55:32 */

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


ULONG A_phase_error_Used[] = {
	0x0000003FL,
	0x00000047L,
	0x00000059L,
	0x00000069L,
	0x0000019FL,
	0x000001A3L,
	0x000001C6L,
	0x00000212L,
	0x00000230L,
	0x000002B6L,
	0x000002C2L,
	0x000002D4L
};

ULONG A_save_ptrs_wsr_Used[] = {
	0x00000091L
};

ULONG A_save_ptrs_wss_Used[] = {
	0x0000008FL
};

ULONG A_io_done_Used[] = {
	0x000001B4L,
	0x000001BAL
};

ULONG A_io_done_wsr_Used[] = {
	0x000001B6L
};

ULONG A_io_done_wss_Used[] = {
	0x000001B8L
};

ULONG A_unknown_msg_Used[] = {
	0x00000085L
};

ULONG A_ext_msg_Used[] = {
	0x0000018BL
};

ULONG A_check_next_io_Used[] = {
	0x000001DAL
};

ULONG A_DIOM_Flag_Used[] = {
	0x000000C8L,
	0x00000126L,
	0x0000015EL,
	0x0000017DL
};

ULONG A_cmd_select_atn_failed_Used[] = {
	0x00000224L
};

ULONG A_err_not_ext_msg_Used[] = {
	0x00000246L,
	0x00000260L
};

ULONG A_sync_msg_reject_Used[] = {
	0x000001F2L
};

ULONG A_neg_select_failed_Used[] = {
	0x00000280L
};

ULONG A_abort_select_failed_Used[] = {
	0x000002EAL
};

ULONG A_abort_io_complete_Used[] = {
	0x000002DEL
};

ULONG A_SIOM_Flag_Used[] = {
	0x00000092L,
	0x000000A2L,
	0x000000F9L,
	0x00000139L,
	0x00000171L,
	0x000001ACL
};

ULONG A_wdtr_msg_ignored_Used[] = {
	0x0000023AL
};

ULONG A_sdtr_msg_ignored_Used[] = {
	0x00000258L
};

ULONG A_wdtr_neg_done_Used[] = {
	0x0000024CL
};

ULONG A_sdtr_neg_done_Used[] = {
	0x00000266L
};

ULONG A_wdtr_msg_reject_Used[] = {
	0x0000026CL
};

ULONG A_sdtr_msg_reject_Used[] = {
	0x00000274L
};

ULONG A_ignore_residue_Used[] = {
	0x0000008BL
};

ULONG A_NEXUS_data_base_adr3_Used[] = {
	0x000000F7L
};

ULONG A_NEXUS_data_base_adr1_Used[] = {
	0x000000F3L
};

ULONG A_NEXUS_data_base_adr2_Used[] = {
	0x000000F5L
};

ULONG A_NEXUS_data_base_adr0_Used[] = {
	0x000000F1L
};

ULONG A_save_tbl_base_adr1_Used[] = {
	0x000000B7L,
	0x00000153L
};

ULONG A_save_tbl_base_adr3_Used[] = {
	0x000000BBL,
	0x00000157L
};

ULONG A_save_tbl_base_adr2_Used[] = {
	0x000000B9L,
	0x00000155L
};

ULONG A_save_tbl_base_adr0_Used[] = {
	0x000000B5L,
	0x00000151L
};

ULONG A_Not_SIOM_Flag_Used[] = {
	0x0000009CL,
	0x000000C3L,
	0x00000124L,
	0x0000015CL,
	0x00000178L,
	0x000001B1L
};

ULONG A_Not_DIOM_Flag_Used[] = {
	0x00000094L,
	0x000000CDL,
	0x000000D1L,
	0x000000FBL,
	0x0000013BL,
	0x00000173L,
	0x00000182L,
	0x000002EBL,
	0x00000303L,
	0x00000321L
};

ULONG A_restore_patch_Used[] = {
	0x0000017AL
};

ULONG A_dptr_tbl_addr1_ref_Used[] = {
	0x00000129L
};

ULONG A_dptr_cnt_addr1_ref_Used[] = {
	0x0000012CL
};

ULONG A_dptr_cnt_addr2_ref_Used[] = {
	0x000001B0L
};

ULONG A_dptr_tbl_addr2_ref_Used[] = {
	0x00000098L
};

ULONG A_dptr_cnt_addr3_ref_Used[] = {
	0x0000009BL
};

ULONG A_dptr_cnt_addr5_ref_Used[] = {
	0x00000180L
};

ULONG A_cur_sav_tbl_addr1_ref_Used[] = {
	0x000000C7L
};

ULONG A_cur_loc_tbl_addr1_ref_Used[] = {
	0x000000C6L
};

ULONG A_dptr_cnt_addr4_ref_Used[] = {
	0x000000CBL
};

ULONG A_dptr_diff_addr1_ref_Used[] = {
	0x000000A8L
};

ULONG A_dptr_diff_addr2_ref_Used[] = {
	0x00000133L
};

ULONG A_cur_loc_tbl_addr2_ref_Used[] = {
	0x0000017CL
};

ULONG A_cur_sav_tbl_addr2_ref_Used[] = {
	0x0000017BL
};

ULONG A_dptr_diff_addr3_ref_Used[] = {
	0x00000141L
};

ULONG A_dptr_restore_addr_ref_Used[] = {
	0x00000161L
};

ULONG A_dptr_tbl_addr1_Used[] = {
	0x000000FFL
};

ULONG A_dptr_cnt_addr1_Used[] = {
	0x00000107L
};

ULONG A_dptr_cnt_addr2_Used[] = {
	0x0000010AL
};

ULONG A_dptr_tbl_addr2_Used[] = {
	0x00000102L
};

ULONG A_dptr_cnt_addr3_Used[] = {
	0x0000010DL
};

ULONG A_dptr_cnt_addr5_Used[] = {
	0x00000113L
};

ULONG A_cur_sav_tbl_addr1_Used[] = {
	0x000000BFL
};

ULONG A_cur_loc_tbl_addr1_Used[] = {
	0x000000C2L
};

ULONG A_dptr_cnt_addr4_Used[] = {
	0x00000110L
};

ULONG A_dptr_diff_addr1_Used[] = {
	0x00000118L
};

ULONG A_dptr_diff_addr2_Used[] = {
	0x0000011BL
};

ULONG A_cur_loc_tbl_addr2_Used[] = {
	0x00000144L
};

ULONG A_cur_sav_tbl_addr2_Used[] = {
	0x0000015BL
};

ULONG A_dptr_diff_addr3_Used[] = {
	0x0000011EL
};

ULONG A_dptr_restore_addr_Used[] = {
	0x00000123L
};

ULONG A_unknown_reselect_id_Used[] = {
	0x00000027L
};

ULONG A_uninitialized_reselect_Used[] = {
	0x0000003BL,
	0x0000004FL,
	0x00000063L,
	0x00000077L,
	0x0000008BL,
	0x0000009FL,
	0x000000B3L,
	0x000000C7L,
	0x000000DBL,
	0x000000EFL,
	0x00000103L,
	0x00000117L,
	0x0000012BL,
	0x0000013FL,
	0x00000153L,
	0x00000167L
};

ULONG A_suspended_Used[] = {
	0x0000016BL
};

