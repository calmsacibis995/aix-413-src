/* @(#)72	1.2  src/rspc/kernext/pci/ncr810/ncr8xx.h, pciscsi, rspc41J, 9513A_all 3/28/95 14:39:38 */
/*
 *   COMPONENT_NAME: PCISCSI
 *
 *   FUNCTIONS: None
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_NCR8XX
#define _H_NCR8XX

#include <sys/pci.h>

#include "ncr8xxcfg.h"
#include "ncr8xxpci.h"
#include "ncr8xxreg.h"
#include "ncr8xxdef.h"
#include "ncr8xxmac.h"
#include "ncr8xxstr.h"
#include "ncr8xxscr.h"


#ifndef	   _NO_PROTO

/*****************************************************************************/
/*     functions in ncr8xxt.c						     */
/*****************************************************************************/

void	p8xx_adp_str_init(ads_t *ap, struct intr *ip);
struct sc_buf * p8xx_build_command();
int	p8xx_close(dev_t devno, int offchan);
int	p8xx_config(dev_t devno, int op, struct uio *uiop);
int	p8xx_inquiry(dev_t devno, int arg, ulong devflag);
int	p8xx_ioctl(dev_t devno, int cmd, int arg, ulong devflag, int chan,
		  int ext);
void	p8xx_iowait_script_init(
	ads_t *ap,
	ulong	*iowait_vir_addr,
	uint	iowait_bus_addr);
int	p8xx_issue_abort(ads_t *ap, int dev_index);
int	p8xx_issue_BDR(ads_t *ap, int dev_index);
int	p8xx_open(dev_t devno, ulong devflag, int chan, int ext);
int	p8xx_readblk(dev_t devno, int arg, ulong devflag);
void	p8xx_script_init(
	ads_t *ap,
	dvi_t *dev_ptr,
	ulong	*script_vir_addr,
	int	dev_info_hash,
	uint	iowait_bus_addr,
	uint	script_bus_addr);
int	p8xx_start_dev(ads_t *ap, int dev_index);
int	p8xx_start_unit(dev_t devno, int arg, ulong devflag);
int	p8xx_stop_dev(ads_t *ap, int dev_index);
int	p8xx_test_unit_rdy(dev_t devno, int arg, ulong devflag);
void	p8xx_undo_open(ads_t *ap, int undo_level, int ret_code, dev_t devno);

/*****************************************************************************/
/*     functions in ncr8xxb.c						     */
/*****************************************************************************/

void	p8xx_abdr(ads_t *ap, dvi_t *dev_ptr);
void	p8xx_abdr_sel_fail(ads_t *ap, dvi_t *dev_ptr, cmd_t *com_ptr,
			   int save_interrupt);
void    p8xx_abort_current_command(ads_t *ap, cmd_t *com_ptr, uchar connected);
int     p8xx_alloc_iovec(ads_t *ap, struct cmd_info *com_ptr);
int     p8xx_alloc_resources(struct sc_buf *bp, struct dev_info *dev_ptr);
int     p8xx_alloc_tag(struct sc_buf *bp, ads_t *ap);
void	p8xx_bdr_done(ads_t *ap, dvi_t *dev_ptr);
uint	p8xx_calc_resid(ads_t *ap, ulong tim_cnt, cmd_t *com_ptr);
struct	cdt *p8xx_cdt_func(int arg);
void    p8xx_check_wfr_queue(uchar command_issued_flag, ads_t *ap);
void	p8xx_chip_register_init(ads_t *ap);
void	p8xx_chip_register_reset(ads_t *ap);
void	p8xx_cleanup_reset(ads_t *ap, int err_type);
void	p8xx_command_reset_scsi_bus(ads_t *ap, int reason);
int	p8xx_config_adapter(ads_t *ap);
void	p8xx_deq_abort_bdr(dvi_t *dev_ptr);
void    p8xx_deq_active(struct dev_info *dev_ptr, struct cmd_info *com_ptr);
void    p8xx_deq_wait(struct cmd_info *com_ptr);
void    p8xx_deq_wfr(struct sc_buf *bp, struct sc_buf *prev_bp, ads_t *ap);
void	p8xx_do_patch(dvi_t *dev_ptr, uchar sxfer, uchar scntl3);
int	p8xx_dump(dev_t devno, struct uio *uiop, int cmd, int arg,
	    int chan, int ext);
int	p8xx_dump_dev(ads_t *ap, struct sc_buf *bp, dvi_t *dev_ptr);
void	p8xx_dumpend(ads_t *ap);
int	p8xx_dump_intr(dvi_t *first_dev_ptr, int aborting_cmds);
int	p8xx_dumpstrt(ads_t *ap, int arg, int ext);
int	p8xx_dumpwrt(ads_t *ap, struct sc_buf *bp, int ext);
void	p8xx_empty_cmd_save(dvi_t *dev_ptr);
void	p8xx_enq_abort_bdr(dvi_t *dev_ptr);
void    p8xx_enq_active(struct dev_info *dev_ptr, struct cmd_info *com);
void    p8xx_enq_wait(struct cmd_info *com);
void    p8xx_enq_wfr(struct sc_buf *bp, ads_t *ap);
int	p8xx_epow(struct pintr *handler);
void	p8xx_fail_cmd(dvi_t *dev_ptr);
void    p8xx_freeze_qs(struct dev_info * dev_ptr);
void    p8xx_free_iovec(ads_t *ap, struct cmd_info *com_ptr);
void    p8xx_free_resources(struct cmd_info *com_ptr);
void	p8xx_handle_extended_messages(cmd_t *com_ptr, int interrupt_flag);
void	p8xx_ignore_residue(cmd_t *com_ptr);
int	p8xx_intr(struct pintr *handler);
void	p8xx_iodone(struct sc_buf *bp);
int	p8xx_issue_abort_bdr(dvi_t *dev_ptr, struct sc_buf *bp,
                    uint iodone_flag, uchar connected);
void	p8xx_issue_abort_script(cmd_t *com_ptr, uchar connected);
void	p8xx_issue_bdr_script(dvi_t *dev_ptr, cmd_t *com_ptr, int connected);
int	p8xx_issue_cmd(ads_t *ap);
void	p8xx_logerr(int errid, int add_halfword_status, int errnum,
	    int data1, ads_t *ap, cmd_t *com_ptr, uchar read_regs);
int     p8xx_message_out(ads_t *ap, cmd_t *com_ptr, int left_over_bytes);
void	p8xx_mode_patch(ads_t *ap, int mode, dvi_t *dev_ptr);
void    p8xx_patch_tag_changes(struct cmd_info * com_ptr, uchar q_tag_msg);
int	p8xx_phase_mismatch(ulong *script_vir_addr,struct cmd_info *com_ptr,
			    int *restart_point);
int 	p8xx_pm_handler(caddr_t private, int requested_mode);
void	p8xx_poll_for_bus_reset(ads_t *ap);
int	p8xx_prepare(struct cmd_info *com_ptr);
void	p8xx_prep_main_script(ulong *iowait_vir_addr, ulong *script_vir_addr,
	    struct cmd_info *com_ptr, uint script_real_addr);
struct sc_buf * p8xx_process_bp_save(struct dev_info *dev_ptr);
void    p8xx_process_q_clr(struct dev_info *dev_ptr, struct sc_buf *bp);
void    p8xx_q_full(struct dev_info *dev_ptr, struct cmd_info *com_ptr);
int	p8xx_read_cfg(ads_t *ap, uint offset, uint length, uint *data);
int	p8xx_read_reg(ads_t *ap, uint offset, char reg_size);
uint	p8xx_recover_byte(ads_t *ap, cmd_t *com_ptr, struct sc_buf *bp);
void	p8xx_reset_iowait_jump(scripts_t *iowait_p, dvi_t *dev_ptr,
                                uchar all_luns);
void	p8xx_restore_iowait_jump(uint *iowait_vir_addr, dvi_t *dev_ptr,
	    uint script_real_addr);
void    p8xx_save_residue(ads_t *ap, int direction, cmd_t *com_ptr);
int	p8xx_scsi_interrupt(struct cmd_info *com_ptr, dvi_t *dev_ptr,
			    int save_interrupt, int issue_abrt_bdr);
void	p8xx_scsi_reset_received(ads_t *ap);
int	p8xx_sdtr_answer(uint *script_vir_addr, dvi_t *dev_ptr);
void    p8xx_sdtr_neg_done(ads_t *ap, dvi_t *dev_ptr, cmd_t *com_ptr);
void    p8xx_sdtr_reject(ads_t *ap, dvi_t *dev_ptr, cmd_t *com_ptr);
void	p8xx_sel_glitch(ads_t *ap, cmd_t *com_ptr, int save_isr);
void	p8xx_set_disconnect(dvi_t *dev_ptr, uchar chk_disconnect);
int	p8xx_sip(ads_t *ap, int save_isr, int tag);
void	p8xx_sir_abort_error(ads_t *ap, dvi_t *dev_ptr, cmd_t *com_ptr,
		int save_interrupt, int save_isr);
void	p8xx_start(struct sc_buf *bp, dvi_t *dev_ptr);
void	p8xx_start_chip(ads_t *ap, struct cmd_info *com_ptr, 
			uint script_real_addr,
	    int entry_offset, uint tim_real_addr, uint tim_no);
int	p8xx_strategy(struct sc_buf *bp);
void	p8xx_turn_chip_on(ads_t *ap, int change_pmh_mode);
int     p8xx_unfreeze_qs(struct dev_info *dev_ptr, struct sc_buf *bp,
                         uchar check_chip);
void    p8xx_update_dataptrs(ads_t *ap, cmd_t *com_ptr, int left_over_bytes);
int     p8xx_verify_tag(ads_t *ap, struct cmd_info * com_ptr);
void	p8xx_watchdog(struct watchdog *w);
void	p8xx_wdtr_answer(uint *script_vir_addr, cmd_t *com_ptr);
void    p8xx_wdtr_reject(ads_t *ap, dvi_t *dev_ptr, cmd_t *com_ptr);
void    p8xx_wide_patch(dvi_t *dev_ptr, uchar new_script);
int	p8xx_write_cfg(ads_t *ap, uint offset, uint length, uint data);
int	p8xx_write_reg(ads_t *ap, uint offset, char reg_size, int data);


#ifdef DEBUG_8XX
void	p8xx_dump_script(ulong *script_ptr, int size);
void	p8xx_rpt_status(ads_t *ap, char *msg, int flag);
#endif
#ifdef P8_TRACE
void    p8xx_trace_1(ads_t *ap, char *string, int data);
void    p8xx_trace_2(ads_t *ap, char *string, int val1, int val2);
void    p8xx_trace_3(ads_t *ap, char *string, int data1, int data2, int data3);
#endif

#ifdef P8_TRACE
#define TRACE_1(A,B)                      {p8xx_trace_1(ap, A,B);}
#define TRACE_2(A,B,C)                    {p8xx_trace_2(ap, A,B,C);}
#define TRACE_3(A,B,C,D)                  {p8xx_trace_3(ap, A,B,C,D);}
#else
#define TRACE_1(A,B)
#define TRACE_2(A,B,C)
#define TRACE_3(A,B,C,D)
#endif



/*****************************************************************************/
/*     functions in ncr8xxu.c					             */
/*****************************************************************************/

int	p8xx_TIM_build(ads_t *ap, struct cmd_info *com_ptr, int flags, 
        char *buf, int bufl, struct xmem *dp, tim_t *tbl, int  tbln);
ads_t	*p8xx_alloc_adp(dev_t devno);
int	p8xx_del_adp(dev_t devno, ads_t *ap);
void	p8xx_free_adp(ads_t *ap);
ads_t	*p8xx_get_adp(dev_t devno);
int	p8xx_get_leftovers(dvi_t *dev_ptr, int *move_kind);
void	p8xx_intr_patch(ulong *tgt, ulong *ety, int intr);
int	p8xx_negotiate(cmd_t *com_ptr, struct sc_buf *bp);
int	p8xx_select_period(ads_t *ap, int xferpd, uchar *scntl3, uchar *sxfer);
int	p8xx_set_adp(dev_t devno, ads_t *ap);
void	p8xx_start_abort_timer(dvi_t *dev_ptr, int force);
#ifdef DEBUG_8XX
void	p8xx_TIM_print(tim_t *tbl, int tbln);
void	p8xx_hexdump(char *hdr, char *data, int size);
#endif

#else


/*****************************************************************************/
/*     functions in ncr8xxt.c					             */
/*****************************************************************************/

void	p8xx_adp_str_init();
struct	sc_buf * p8xx_build_command();
int	p8xx_close();
int	p8xx_config();
int	p8xx_inquiry();
int	p8xx_ioctl();
void	p8xx_iowait_script_init();
int	p8xx_issue_abort();
int	p8xx_issue_BDR();
int	p8xx_open();
int	p8xx_readblk();
void	p8xx_script_init();
int	p8xx_start_dev();
int	p8xx_start_unit();
int	p8xx_stop_dev();
int	p8xx_test_unit_rdy();
void	p8xx_undo_open();


/*****************************************************************************/
/*     functions in ncr8xxb.c						     */
/*****************************************************************************/

void	p8xx_abdr();
void	p8xx_abdr_sel_fail();
void    p8xx_abort_current_command();
int     p8xx_alloc_iovec();
int     p8xx_alloc_resources();
int     p8xx_alloc_tag();
void	p8xx_bdr_done();
uint	p8xx_calc_resid();
struct	cdt *p8xx_cdt_func();
void	p8xx_check_wfr_queue();
void	p8xx_chip_register_init();
void	p8xx_chip_register_reset();
void	p8xx_cleanup_reset();
void	p8xx_command_reset_scsi_bus();
int	p8xx_config_adapter();
void	p8xx_deq_abort_bdr();
void    p8xx_deq_active();
void    p8xx_deq_wait();
void    p8xx_deq_wfr();
void	p8xx_do_patch();
int	p8xx_dump();
int	p8xx_dump_dev();
void	p8xx_dumpend();
int	p8xx_dump_intr();
int	p8xx_dumpstrt();
int	p8xx_dumpwrt ();
void	p8xx_empty_cmd_save();
void	p8xx_enq_abort_bdr();
void    p8xx_enq_active();
void    p8xx_enq_wait();
void    p8xx_enq_wfr();
int	p8xx_epow();
void	p8xx_fail_cmd();
void    p8xx_freeze_qs();
void    p8xx_free_iovec();
void    p8xx_free_resources();
void	p8xx_handle_extended_messages();
void	p8xx_ignore_residue();
int	p8xx_intr();
void	p8xx_iodone();
void	p8xx_issue_abort_bdr();
void	p8xx_issue_abort_script();
void	p8xx_issue_bdr_script();
int	p8xx_issue_cmd();
void	p8xx_logerr();
int     p8xx_message_out();
void	p8xx_mode_patch();
void    p8xx_patch_tag_changes();
int	p8xx_phase_mismatch();
int 	p8xx_pm_handler();
void	p8xx_poll_for_bus_reset();
int	p8xx_prepare();
void	p8xx_prep_main_script();
struct sc_buf * p8xx_process_bp_save();
void    p8xx_process_q_clr();
void    p8xx_q_full();
int	p8xx_read_cfg();
int	p8xx_read_reg();
uint	p8xx_recover_byte();
void	p8xx_reset_iowait_jump();
void	p8xx_restore_iowait_jump();
void    p8xx_save_residue();
int	p8xx_scsi_interrupt();
void	p8xx_scsi_reset_received();
int	p8xx_sdtr_answer();
void    p8xx_sdtr_neg_done();
void    p8xx_sdtr_reject();
void	p8xx_sel_glitch();
void	p8xx_set_disconnect();
int	p8xx_sip();
void	p8xx_sir_abort_error();
void	p8xx_start();
void	p8xx_start_chip();
int	p8xx_strategy();
void	p8xx_turn_chip_on();
int     p8xx_unfreeze_qs();
void    p8xx_update_dataptrs();
int     p8xx_verify_tag();
void	p8xx_watchdog();
void	p8xx_wdtr_answer();
void    p8xx_wdtr_reject();
void    p8xx_wide_patch();
int	p8xx_write_reg();
int	p8xx_write_cfg();
#ifdef DEBUG_8XX
void	p8xx_rpt_status();
void	p8xx_dump_script();
#endif


/*****************************************************************************/
/*     functions in ncr8xxu.c					             */
/*****************************************************************************/

int	p8xx_TIM_build();
ads_t	*p8xx_alloc_adp();
int	p8xx_del_adp();
void	p8xx_free_adp();
ads_t	*p8xx_get_adp();
int	p8xx_get_leftovers();
void	p8xx_intr_patch();
int	p8xx_negotiate();
int	p8xx_select_period();
int	p8xx_set_adp();
void	p8xx_start_abort_timer();
#ifdef DEBUG_8XX
void	p8xx_TIM_print();
void	p8xx_hexdump();
#endif

#endif /* not _NO_PROTO */

#endif /* _H_NCR8XX */

