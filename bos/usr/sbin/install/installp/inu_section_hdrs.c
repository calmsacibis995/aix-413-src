static char     sccsid[] = "@(#)81      1.6  src/bos/usr/sbin/install/installp/inu_section_hdrs.c, cmdinstl, bos411, 9428A410j 3/6/94 19:32:32";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_display_section_hdr
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------------------------------------------------------------------*
**                        Include Files
**--------------------------------------------------------------------------*/

#include <installp.h>
#include <instl_define.h>
#include <instl_options.h>

#define SECTION_LINE "\
+-----------------------------------------------------------------------------+\n"

/*--------------------------------------------------------------------------*
**                         Function Prototypes
**--------------------------------------------------------------------------*/

void inu_display_section_hdr (int hdr_type, int operation)
{
   switch (hdr_type)
   {
      case SECT_HDR_PI_PROC:
           instl_msg (INFO_MSG, "%s", SECTION_LINE);
           switch (operation)
           {
              case OP_APPLY:
                   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PIV_I, 
                                                             INP_PIV_D));
                 break;
              case OP_COMMIT:
                   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PCV_I, 
                                                             INP_PCV_D));
                 break;
              case OP_REJECT:
                   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PRV_I, 
                                                             INP_PRV_D));
                 break;
              case OP_DEINSTALL:
                   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PDV_I, 
                                                             INP_PDV_D));
                 break;
           }
           break;

      case SECT_HDR_INSTLLING:
           instl_msg (INFO_MSG, "%s", SECTION_LINE);
           switch (operation)
           {
              case OP_APPLY:
                 instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_INST_SW_I, 
                                                           INP_INST_SW_D));
                 break;
              case OP_COMMIT:
                 instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_COMMIT_SW_I, 
                                                           INP_COMMIT_SW_D));
                 break;
              case OP_REJECT:
                 instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_REJ_SW_I, 
                                                           INP_REJ_SW_D));
                 break;
              case OP_DEINSTALL:
                 instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_DEINST_SW_I, 
                                                           INP_DEINST_SW_D));
                 break;
           }
           break;

      case SECT_HDR_POST_PROC:
           instl_msg (INFO_MSG, "%s", SECTION_LINE);
           switch (operation)
           {
              case OP_APPLY:
                 instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PIP_I, 
                                                           INP_PIP_D));
                 break;
              case OP_COMMIT:
                 instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PCP_I, 
                                                           INP_PCP_D)); 
                 break;
              case OP_REJECT:
                 instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PRP_I, 
                                                           INP_PRP_D));
                 break;
              case OP_DEINSTALL:
                 instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_PDP_I, 
                                                           INP_PDP_D));
                 break;
           }
           break;

      case SECT_HDR_SUMMARIES:
           instl_msg (INFO_MSG, "%s", SECTION_LINE);
           instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_SUMMARIES_I, 
                                                     INP_SUMMARIES_D));
           break;
   }
   instl_msg (INFO_MSG, "%s", SECTION_LINE);
}
