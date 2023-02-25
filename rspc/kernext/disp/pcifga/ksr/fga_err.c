static char sccsid[] = "@(#)15  1.1  src/rspc/kernext/disp/pcifga/ksr/fga_err.c, pcifga, rspc411, GOLD410 4/15/94 18:41:24";
/* fga_err.c */
/*
based on "@(#)58  1.1  src/bos/kernext/disp/wga/ksr/wga_err.c, bos, bos410 10/28/93 18:05:52";
 *
 * COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: fga_err
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/syspest.h>
BUGXDEF(dbg_fga_err)
/*------------
 * FUNCTIONAL DESCRIPTION:
 *
  FGA error logging routine.
        struct vtmstruc *vp  - pointer to the vtmstruc structure in vt.h.
        char   *res_name     - Failing sotfware component name.
        char   *dmodule      - Detecting module (the module that called the
                                failing module)
        char   *fmodule      - Failing module (the module that returned the
                                bad return code)
        int    return_code   - Return code from failing module
        int    err_indicator - Error indicator number from lftras.h
        char   *ras_unique   - Unique code used to identitify specific error
                                locations for error logging.
  ------------*/

#include <sys/types.h>
#include <sys/errids.h>
#include <sys/syspest.h>
#include <vt.h>

#ifndef ERRID_GRAPHICS
#define ERRID_GRAPHICS    0xf10d0eff
#endif /* ERRID_GRAPHICS */

ERR_REC(256) ER;

int
fga_err(vp,res_name,dmodule,fmodule,return_code,err_indicator,ras_unique)
struct vtmstruc *vp;
char   *res_name;
char   *dmodule;
char   *fmodule;
int    return_code;
int    err_indicator;
char   *ras_unique;

{
/* For debugging purposes only  */
       BUGLPR(dbg_fga_err, BUGNFO, ("Entering fga_err \n"));
if( res_name != NULL )
       BUGLPR(dbg_fga_err, BUGNFO, ("Failing component %s\n",res_name));
if( dmodule != NULL )
       BUGLPR(dbg_fga_err, BUGNFO, ("Detecting module %s \n",dmodule));
if( fmodule != NULL ) {
       BUGLPR(dbg_fga_err, BUGNFO, ("Failing module %s\n",fmodule));
       BUGLPR(dbg_fga_err, BUGNFO, ("Return code %d\n",return_code));
       }
if( err_indicator != NULL )
       BUGLPR(dbg_fga_err, BUGNFO, ("Ras error code %d\n",err_indicator));
if( ras_unique != NULL )
       BUGLPR(dbg_fga_err, BUGNFO, ("Ras_unique %s\n",ras_unique));


ER.error_id = ERRID_GRAPHICS;                   /* Template Id # */

sprintf(ER.resource_name,"%8s",res_name);

sprintf(ER.detail_data,"%8s  %8s  %4d  %4d  %s",
        dmodule,fmodule,return_code,err_indicator,ras_unique);

/* Call system error logging routine */
errsave(&ER, ERR_REC_SIZE + (strlen(ER.detail_data)+1));

/* For debug only */
BUGLPR(dbg_fga_err, BUGNFO, ("Leaving fga_err \n"));

return(0);
}
