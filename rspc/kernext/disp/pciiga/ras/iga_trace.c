static char sccsid[] = "@(#)61	1.2  src/rspc/kernext/disp/pciiga/ras/iga_trace.c, pciiga, rspc41J, 9515B_all 3/22/95 17:19:09";

/*
 * COMPONENT_NAME: (pciiga) IGA Device Driver
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

#include <igatrc.h>

GS_TRC_GLB(0,2);               /* define global trace variables */

GS_TRC_MODULE(iga_define, 2);
GS_TRC_MODULE(iga_enable, 2);
GS_TRC_MODULE(iga_close, 2);
GS_TRC_MODULE(iga_open, 2);

GS_TRC_MODULE(iga_init, 2);
GS_TRC_MODULE(iga_Get_Monitor, 2);
GS_TRC_MODULE(iga_lookup_v, 2);

GS_TRC_MODULE(iga_load_font, 2);
GS_TRC_MODULE(iga_load_pal, 2);

GS_TRC_MODULE(iga_makegp, 2);
GS_TRC_MODULE(iga_unmakegp, 2);

GS_TRC_MODULE(iga_reset, 2);

GS_TRC_MODULE(iga_copy_ps, 2);

GS_TRC_MODULE(iga_vttcpl, 2);
GS_TRC_MODULE(iga_vttclr, 2);
GS_TRC_MODULE(iga_vttcfl, 2);
GS_TRC_MODULE(iga_vttact, 2);
GS_TRC_MODULE(iga_vttdact, 2);
GS_TRC_MODULE(iga_vttinit, 2);
GS_TRC_MODULE(iga_vttsetm, 2);
GS_TRC_MODULE(iga_vttterm, 2);
GS_TRC_MODULE(iga_vttmovc, 2);
GS_TRC_MODULE(iga_vttscr, 2);
GS_TRC_MODULE(iga_vtttext, 2);
GS_TRC_MODULE(iga_vttstct, 2);
GS_TRC_MODULE(iga_vttdefc, 2);
GS_TRC_MODULE(iga_vttrds, 2);
GS_TRC_MODULE(iga_vttdpm, 2);

GS_TRC_MODULE(iga_pm, 2);

GS_TRC_MODULE(iga_intr, 2);
