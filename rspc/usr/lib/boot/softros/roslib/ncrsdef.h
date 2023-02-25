/* @(#)46	1.1  src/rspc/usr/lib/boot/softros/roslib/ncrsdef.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:41  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define trace 0xDDDD0000
#define Ext_Count  15
#define E_cmd_buf	0x00000000
#define E_cmd_byte_cnt	0x00000000
#define E_data_buffer	0x00000000
#define E_data_byte_cnt	0x00000000
#define E_identify_msg_buf	0x00000000
#define E_mem_bytes	0x00000000
#define E_mem_target	0x00000000
#define E_mem_source	0x00000000
#define E_msg_in_buf	0x00000000
#define E_resel_id	0x00000000
#define E_status_byte	0x00000000
#define E_sync_msg_in_buf	0x00000000
#define E_sync_msg_resp	0x00000000
#define E_target_id	0x00000000
#define Abs_Count 30
#define A_cmd_cmp_flag	0x00000008
#define A_status_flag	0x00000010
#define A_cmd_flag	0x00000020
#define A_neg_msg_in_flag	0x00000040
#define A_Complete	0x00000050
#define A_Save_PTR	0x00000053
#define A_Restore_PTR	0x00000054
#define A_Modify_PTR	0x00000055
#define A_Disconnecting	0x00000056
#define A_msg_out_flag	0x00000080
#define A_trace_select_1	0xDDDD0001
#define A_trace_msgout_1	0xDDDD0002
#define A_trace_cmd	0xDDDD0003
#define A_trace_data	0xDDDD0004
#define A_trace_status	0xDDDD0005
#define A_trace_msgin_1	0xDDDD0006
#define A_trace_select_2	0xDDDD0007
#define A_trace_msgout_2	0xDDDD0008
#define A_trace_msgin_2	0xDDDD0009
#define A_trace_msgin_3	0xDDDD0010
#define A_trace_msgin_4	0xDDDD0011
#define A_trace_check_xfer	0xDDDD0012
#define A_trace_resel	0xDDDD0013
#define A_not_Msg_Out	0xEEEE0001
#define A_not_Cmd	0xEEEE0002
#define A_not_Data	0xEEEE0003
#define A_not_Status	0xEEEE0004
#define A_not_Msg_In	0xEEEE0005
#define A_Select_Failed	0xEEEE0051
#define A_reenter_phase_phault	0xEEEE0052
#define Ent_common_restart   	0x000001C0
#define Ent_common_script    	0x00000000
#define Ent_data_phase       	0x00000070
#define Ent_data_xfer        	0x000001F0
#define Ent_data_xfer_inst   	0x00000080
#define Ent_dev_disconnect   	0x00000200
#define Ent_mem_test         	0x00000360
#define Ent_negotiate_restart_not_sdtr	0x00000318
#define Ent_negotiate_restart	0x00000310
#define Ent_negotiate_script 	0x00000260
#define Ent_script_test      	0x00000348
#define Ent_sdtr_response    	0x000002A8
#define Ent_sdtr_request     	0x00000280
#define Ent_sync_neg         	0x00000298
#define Ent_sync_neg_in      	0x000002E8
#define Ent_sync_reenter     	0x00000278
#define Ent_trace_check_xfer_ret	0x00000300
#define Ent_trace_data_ret   	0x00000080
#define Ent_trace_cmd_ret    	0x00000050
#define Ent_trace_msgin_1_ret	0x000000D0
#define Ent_trace_msgin_2_ret	0x000002B8
#define Ent_trace_msgin_3_ret	0x000002D0
#define Ent_trace_msgin_4_ret	0x000002E8
#define Ent_trace_msgout_1_ret	0x00000028
#define Ent_trace_msgout_2_ret	0x00000290
#define Ent_trace_resel_ret  	0x00000248
#define Ent_trace_select_1_ret	0x00000008
#define Ent_trace_select_2_ret	0x00000268
#define Ent_trace_status_ret 	0x000000B8
