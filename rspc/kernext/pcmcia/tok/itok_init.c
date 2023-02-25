static char sccsid[] = "@(#)60  1.2  src/rspc/kernext/pcmcia/tok/itok_init.c, pcmciatok, rspc41J, 9511A_all 3/9/95 02:37:07";
/*
static char sccsid[] = "@(#)43	1.2  src/rspc/kernext/isa/itok/itok_init.c, isatok, rspc41J 5/21/94 15:32:03";
 */
/*
 * COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS:  tok_setup_op_params(), tok_reset_adapter()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "itok_proto.h"

extern dd_ctrl_t dd_ctrl;

/*****************************************************************************/
/*
 * NAME:                  tok_setup_op_params
 *
 * FUNCTION:              Setup the open parameters for the adapter open cmd.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:                 If using more than 2 transmit buffers, the adapter
 *                        might not work correctly, using 2 does not seem to
 *                        improve throughput.
 *
 * RETURNS:                nothing
 *
 */
/*****************************************************************************/
void tok_setup_op_params(dds_t *p_dds)       /* Device structure ptr. */
{
  int i;                              /* Simple loop counter. */

  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"sopB",(ulong)DDI.ring_speed,
    (ulong)WRK.command_to_do,(ulong)0);

  /* Setup the default open options. */
  /* Don't merge them into a short, since we have to support 8 bit adapters */
  /* as well as 16 bit ones. */
  WRK.open_cfg.option1 = ITOK_ISA_DEFAULT_OPEN_OPTION1;
  WRK.open_cfg.option2 = ITOK_ISA_DEFAULT_OPEN_OPTION2;

  if (DDI.attn_mac) {
    NDD.ndd_flags        |= TOK_ATTENTION_MAC;
    WRK.open_cfg.option1 |= OP_PASS_ATTN_MAC_FRAMES;
  }

  if (DDI.beacon_mac) {
    NDD.ndd_flags        |= TOK_BEACON_MAC;
    WRK.open_cfg.option2 |= OP_PASS_BEACON_FRAMES;
  }

  /* Adjust variables according to ring-speed. */
  NDD.ndd_mintu = CTOK_MIN_PACKET;
  if (DDI.ring_speed == 1) {
    /* 16 mbits/sec. */
    NDD.ndd_mtu       = TOK_16M_MAX_DATA;
    WRK.max_frame_len = TOK_16M_MAX_DATA;
  } else {
      NDD.ndd_mtu       = TOK_4M_MAX_DATA;
      WRK.max_frame_len = TOK_4M_MAX_DATA;
    }

  WRK.open_cfg.cmd            = SRB_OPEN;
  for (i = 0; i < 7; i++)
     WRK.open_cfg.reserved[i] = 0;
  if (DDI.use_alt_addr) {
    bcopy(&(DDI.alt_addr[0]),&(WRK.open_cfg.cur_tok_addr[0]),CTOK_NADR_LENGTH);
  } else { /* Use the burned in address. */
      for (i = 0; i < CTOK_NADR_LENGTH;i++) {
         WRK.open_cfg.cur_tok_addr[i] = 
           dd_ctrl.adap_cfg[WRK.minor_no].aip.encoded_hw_addr[i]; 
      }
    }
  /* Setup the physical address for the ndd. */
  NDD.ndd_physaddr = (caddr_t)(&WRK.open_cfg.cur_tok_addr);

  /* Setup the group and functional addresses. Only set them to zero if */
  /* we have not gotten a request to set some values (during the open phase). */
  if (!WRK.command_to_do) {
    for (i = 0; i < 4; i++) /* Don't optimize this to a word.. 8/16 bit access*/
       WRK.open_cfg.grp_addr[i]    = 0;
    for (i = 0; i < 4; i++)
       WRK.open_cfg.func_addr[i]   = 0;
  } else {
      if (!(WRK.command_to_do & TOK_GROUP_ADDR_EVENT_BIT)) {
        for (i = 0; i < 4; i++) /* Don't optimize this to a word */
           WRK.open_cfg.grp_addr[i]    = 0;
      }
      if (!(WRK.command_to_do & TOK_FUNC_ADDR_EVENT_BIT)) {
        for (i = 0; i < 4; i++) /* Don't optimize this to a word */
           WRK.open_cfg.func_addr[i]    = 0;
      }
      WRK.command_to_do = 0;
      /* The open config func and/or group addr. has been saved earlier. */
    }
  WRK.open_cfg.num_rcv_buf[0] = (uchar)((OPEN_RCV_BUF_COUNT >> 8) & 0xff);
  WRK.open_cfg.num_rcv_buf[1] = (uchar)(OPEN_RCV_BUF_COUNT & 0xff);
  WRK.open_cfg.rcv_buf_len[0] =(uchar)((OPEN_RCV_BUF_LENGTH >> 8) & 0xff);
  WRK.open_cfg.rcv_buf_len[1] =(uchar)(OPEN_RCV_BUF_LENGTH & 0xff);
  WRK.open_cfg.dhb_length[0] =(uchar)((OPEN_XMIT_BUF_LENGTH  >> 8) & 0xff);

  WRK.open_cfg.dhb_length[1] =(uchar)(OPEN_XMIT_BUF_LENGTH & 0xff);
  WRK.open_cfg.num_dhb        = OPEN_XMIT_BUF_COUNT;
  WRK.open_cfg.reserved2      = 0;
  WRK.open_cfg.dlc_max_sap    = OPEN_CONFIG_NBR_SAPS;
  WRK.open_cfg.dlc_max_sta    = OPEN_CONFIG_NBR_STATIONS;
  WRK.open_cfg.dlc_max_gsap   = OPEN_CONFIG_NBR_GRP_SAPS;
  WRK.open_cfg.dlc_max_gmem   = OPEN_CONFIG_MAX_GMEM;
  WRK.open_cfg.dlc_t1_tick_1  = OPEN_T1_40MS_TICK_1;
  WRK.open_cfg.dlc_t2_tick_1  = OPEN_T2_40MS_TICK_1;
  WRK.open_cfg.dlc_ti_tick_1  = OPEN_Ti_40MS_TICK_1;
  WRK.open_cfg.dlc_t1_tick_2  = OPEN_T1_40MS_TICK_2;
  WRK.open_cfg.dlc_t2_tick_2  = OPEN_T2_40MS_TICK_2;
  WRK.open_cfg.dlc_ti_tick_2  = OPEN_Ti_40MS_TICK_2;
  for (i = 0; i < PRODUCT_ID_LEN; i++)
     WRK.open_cfg.product_id[i]  = 0;

  TRACE_DBG(HKWD_ITOK_ISA_OTHER,"sopE",(ulong)0,(ulong)0,(ulong)0);
}


/*****************************************************************************/
/*
 * NAME:                  tok_reset_adapter
 *
 * FUNCTION:              Hard resets the adapter. 
 *
 * EXECUTION ENVIRONMENT: process or interrupt level
 *
 * NOTES:                 Delays 50 ms between latch and release, can be
 *                        changed to a timeout if need be.
 *
 * RETURNS:               0 or errno
 *
 */
/*****************************************************************************/
int tok_reset_adapter (dds_t *p_dds)          /* Device structure ptr. */
{
  volatile uchar *ioaddr;                     /* IO-bus handle. */

  TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"RSTb",(ulong)dd_ctrl.num_adapters,
                (ulong)p_dds,(ulong)0); 

  /* Only try to reset if we found an adapter during the initial config. */
  if (!dd_ctrl.num_adapters)
    return(-1);

  /* Latch the reset on the adapter. */
  ioaddr = iomem_att(dd_ctrl.io_map_ptr);
  *(ioaddr + dd_ctrl.adap_cfg[WRK.minor_no].io_base + RESET_LATCH_OFFSET) =
     RESET_VALUE;
  iomem_det(ioaddr);

  /* Set the state variable for the startup function. */
  WRK.open_status = OPEN_STATUS0;

  /* Hold this latch for at least 50 ms. */
  INITWDT.count   = 0;
  INITWDT.restart = 1;
  w_start(&INITWDT);

 TRACE_BOTH(HKWD_ITOK_ISA_OTHER,"RSTe",(ulong)dd_ctrl.num_adapters,(ulong)p_dds,
            (ulong)0); 
 return(0);
}
