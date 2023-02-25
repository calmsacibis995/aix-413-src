static char sccsid[] = "@(#)31	1.2  src/rspc/kernext/disp/pcigga/ras/gga_trace.c, pcigga, rspc41J, 9513A_all 3/22/95 11:25:21";

/*
 * COMPONENT_NAME: (pcigga) Glacier Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <ggatrc.h>

GS_TRC_GLB(0,2);               /* define global trace variables */

GS_TRC_MODULE(gga_define, 2);
GS_TRC_MODULE(gga_enable, 2);
GS_TRC_MODULE(gga_close, 2);
GS_TRC_MODULE(gga_open, 2);

GS_TRC_MODULE(gga_init, 2);
GS_TRC_MODULE(gga_Prog_ICD, 2);

GS_TRC_MODULE(gga_load_font, 2);
GS_TRC_MODULE(gga_load_pal, 2);

GS_TRC_MODULE(gga_makegp, 2);
GS_TRC_MODULE(gga_unmakegp, 2);

GS_TRC_MODULE(gga_reset, 2);

GS_TRC_MODULE(gga_copy_ps, 2);

GS_TRC_MODULE(gga_vttcpl, 2);
GS_TRC_MODULE(gga_vttclr, 2);
GS_TRC_MODULE(gga_vttcfl, 2);
GS_TRC_MODULE(gga_vttact, 2);
GS_TRC_MODULE(gga_vttdact, 2);
GS_TRC_MODULE(gga_vttinit, 2);
GS_TRC_MODULE(gga_vttsetm, 2);
GS_TRC_MODULE(gga_vttterm, 2);
GS_TRC_MODULE(gga_vttmovc, 2);
GS_TRC_MODULE(gga_vttscr, 2);
GS_TRC_MODULE(gga_vtttext, 2);
GS_TRC_MODULE(gga_vttstct, 2);
GS_TRC_MODULE(gga_vttdefc, 2);
GS_TRC_MODULE(gga_vttrds, 2);
GS_TRC_MODULE(gga_vttdpm, 2);

GS_TRC_MODULE(gga_pm, 2);

GS_TRC_MODULE(gga_intr, 2);
