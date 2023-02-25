static char sccsid[] = "@(#)65  1.3  src/rspc/kernext/disp/pciwfg/config/config.c, pciwfg, rspc41J, 9521A_all 5/21/95 20:58:09";
/* config.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: wfg_config
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

#include "wfg_INCLUDES.h"

int (*v7310_reset_overlay)();
int (*v7310_power_ctl)();
extern int (**i_v7310_reset_overlay)();
extern int (**i_v7310_power_ctl)();

#ifdef	PANEL_PROTECTION
extern struct wfg_ddf *wfgDDF;
extern struct fbdds *wfgDDS;
#endif	/* PANEL_PROTECTION */

BUGXDEF (dbg_cfg);

/*-----------------------------------------------------------------------
 |
 | IDENTIFICATION:  DD_CONFIG
 |
 | DESCRIPTIVE NAME: DD_CONFIG 
 |
 | FUNCTION: Standard device driver config routine - Configures,
 | unconfigures and returns vpd to calling config method.
 |
 | Routine is passed in command and responds as follows:
 |
 | CFG_INIT - copies in dds information and invokes
 | wfg_define routine to get adapter instance
 | into system.
 |
 | CFG_TERM - frees physical display structure thus
 | unconfiguring adapter instance.
 |
 | CFG_QVPD - Returns vital product data off of adapter
 |
 | INPUTS:   Device number
 | Command ( as listed above )
 | Uiop pointer to user space data (buffer to read vpd
 | into or dds information)
 |
 | OUTPUTS:  None.
 |
 | CALLED BY: 'cfgwfg' display config method
 |
 |
 |---------------------------------------------------------------------*/

wfg_config (devno, cmd, uiop)
        dev_t           devno;
        long            cmd;
        struct uio     *uiop;
{
        struct fbdds   *dds;
        struct wfg_ddf *ddf;
        int             rc,
                        i,
                        status;
        struct phys_displays *pd;
        char           *vpdaddr;

        /*
         * This routine switches off of the command passed in and either
         * inits terminates or does a query of the VPD on the adapter.
         */

        BUGLPR(dbg_cfg, BUGNTA,("Enter the dd_config() of WFG Graphic DD\n"));
        switch (cmd)
        {
        case CFG_INIT:

                /* In this case we need to get our driver in to the system
                 * by allocating a phys display struct and filling it in then
                 * adding our driver to the devsw table.  For this level of
                 * implementation we are compiled into the devsw so all we do
                 * is invoke dd_define  to build our phys display struct and
                 * hook it in
                 */

#ifdef	PANEL_PROTECTION
		wfgDDS =
#endif	/* PANEL_PROTECTION */
                dds = (struct fbdds *) xmalloc (sizeof (struct fbdds),
                                                3, pinned_heap);

                if (dds == NULL)
                {
                BUGLPR (dbg_cfg, BUGNTA, ("DDS xmalloc failure\n"));
                        wfg_err (NULL, "PCIWFG", "CONFIG", "XMALLOC", dds, WFG_BAD_XMALLOC,
                                RAS_UNIQUE_1);
                        return (ENOMEM);
                }

                rc = uiomove ((unsigned char *)dds, sizeof (struct fbdds), UIO_WRITE, (struct uio *)uiop);
                if (rc != 0)
                        return (rc);

                BUGLPR (dbg_cfg, BUGNTA, ("Calling wfg_define\n"));

                for (i = 0; i < 16; i++)
                {
                        BUGLPR(dbg_cfg,BUGGID,("RGB[%d] = %X\n",i,dds->ksr_color_table[i]));

                }

                rc = wfg_define (devno, dds, sizeof (struct fbdds));

                for (i = 0; i < 16; i++)
                {
                        BUGLPR(dbg_cfg,BUGGID,("RGB[%d] = %X\n",i,dds->ksr_color_table[i]));

                }
                BUGLPR (dbg_cfg, BUGACT, ("wfg_define - rc = %d\n", rc));
                
                i_v7310_reset_overlay = &v7310_reset_overlay;
                i_v7310_power_ctl     = &v7310_power_ctl;

                return (rc);

        case CFG_TERM:

                /* In this case we would remove all malloc'd data
                 * and remove ourselves from the device switch table
                 * if we are the last device of this type.
                 * Since there can only be one WFG then remove
                 */
                BUGLPR(dbg_cfg,BUGNTA,("Calling wfg_undefine\n"));

                rc = wfg_undefine(devno);

                BUGLPR(dbg_cfg,BUGACT,("wfg_undefine rc = %d\n",rc));

                return(rc);

        case CFG_QVPD:

                /*
                 * This function returns the Vital Product Data from
                 * the adapter.
                 */

                devswqry (devno, (unsigned int *)&status, (unsigned char **)&pd);

                while (pd != NULL)
                {
                        if (pd->devno == devno)
                                break;
                        else
                                pd = pd->next;
                }

                if (pd == NULL)
                {
                        return (-1);   /* No display present */
                }
                else
                {
                        /*
                         * return the "not supported" string
                         */

                        vpdaddr = "No VPD is defined for this device.";
                        /*
                         * Write data out to user space
                         */

                        rc = uiomove (vpdaddr, strlen(vpdaddr), UIO_READ, uiop);
                        break;
                }
                break;

        default:
                return (-1);
        }

        return (0);
}

#ifdef	PANEL_PROTECTION
void 
get_VDD_info(struct Vddinfo *vddinfo)
{
	i_get_VDD_info(vddinfo);
}

int
wfg_lcd_control(int devno, int level)
{
	if (!wfgDDF || !wfgDDS || !wfgDDS->pm_lcd_device_id) {
		return 1;
	}

	return wfg_planar_control(wfgDDF, wfgDDS, devno, 
				  wfgDDS->pm_lcd_device_id, level);
}
#else	/* PANEL_PROTECTION */
void get_VDD_info()
{
    i_get_VDD_info();
}
#endif	/* PANEL_PROTECTION */
