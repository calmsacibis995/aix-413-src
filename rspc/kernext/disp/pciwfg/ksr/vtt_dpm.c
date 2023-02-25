static char sccsid[] = "@(#)62  1.6  src/rspc/kernext/disp/pciwfg/ksr/vtt_dpm.c, pciwfg, rspc41J, 9521A_all 5/22/95 06:41:22";
/* vtt_dpm.c */
/*
 *
 * COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: vttdpm() device_power()
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "wfg_INCLUDES.h"
#include "wfg_funct.h"
#include <sys/m_param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include "IO_defs.h"

#ifdef	PANEL_PROTECTION
#define	pm_planar_control(a,b,c)	wfg_planar_control(ddf,dds,a,b,c)
#endif	/* PANEL_PROTECTION */

#define LCD_ON		5	
#define LCD_OFF		6
#define CRT_ON		7
#define CRT_OFF		8

int (**i_v7310_power_ctl)();

BUGXDEF (dbg_vttdpm);
/**************************************************************************
 *                                                                        *
 * IDENTIFICATION:   vttdpm()                                             *
 *                                                                        *
 * DESCRIPTIVE NAME:  Display Power Management fuction                    *
 *                                                                        *
 * FUNCTIONS:  support display power management (on and off mode)         *
 *             by turning on and off both the vertical and horizontal     *
 *             syns to attached display.                                  *
 *                                                                        *
 * INPUTS: physical display structure and                                 *
 *         display power saving phase -- display full-on = 1              *
 *                                       display standby = 2              *
 *                                       display suspend = 3              *
 *                                       display off     = 4              *
 *                                       LCD panel on    = 5              *
 *                                       LCD panel off   = 6              *
 *                                       CRT display on  = 7              *
 *                                       CRT display off = 8              *
 *                                                                        *
 * OUTPUTS                                                                *
 *                                                                        *
 * CALLED BY:  X window's screen saver which calls loadddx                *
 *                                     which calls us (dd)                *
 *                          Or                                            *
 *             LFT Display Power Management timer                         *
 *                          Or                                            *
 *             PM core (kernel) if it is installed                        *
 *                                                                        *
 * REFERENCES:  Graphics Adaptor Specification                            *
 *              Feature 166380 --- Power Management Support               *
 *                                                                        *
 **************************************************************************/

long 
vttdpm (pd,phase)
        struct phys_displays *pd;
        int phase;
{
        struct wfg_data   *ld;
        struct pm_handle  *pmhdl;
        struct fbdds      *dds;
	struct wfg_ddf    *ddf;
        int               rc=PM_SUCCESS;

        BUGLPR (dbg_vttdpm, BUGNFO, ("Entering vttpwrphase\n"));

        ld    = (struct wfg_data *)  pd->visible_vt->vttld;
        pmhdl = (struct pm_handle *) ld->wfg_pmh;
        dds   = (struct fbdds *)     pd->odmdds;
        ddf   = (struct wfg_ddf *)   ld->ddf;

        /* ---------------------------------------------------------------- *
         *   Woodfield Unique Power saving functions                        *
         *   These functions are provided for more strict power control     *
         * ---------------------------------------------------------------- */

        switch ( phase ) {

        case LCD_ON: 
                /* --------------------------------------- * 
		 *          5:   LCD ON                    *
                 * --------------------------------------- */
                io_delay(100);
#ifdef	PANEL_PROTECTION
		/*
		 * v7310_power_ctl is moved to device_power()
		 */
#else	/* PANEL_PROTECTION */
                if (ld->model_type == ADVANCED_MODEL)
                      if ( *i_v7310_power_ctl != NULL ) {
                             (**i_v7310_power_ctl)( POWER_ON );
                      }
#endif	/* PANEL_PROTECTION */
                rc = device_power ( pd, LCD, PM_PLANAR_ON ); 
                ld->lcd_power = TURN_ON;
                break;

        case LCD_OFF:
                /* --------------------------------------- * 
		 *          6:   LCD OFF                   *
                 * --------------------------------------- */
                rc = device_power ( pd, LCD, PM_PLANAR_OFF ); 

#ifdef	PANEL_PROTECTION
		/*
		 * v7310_power_ctl is moved to device_power()
		 */
#else	/* PANEL_PROTECTION */
                /* ------------------------------------------------- *
                 *   Calls v7310 function for strict power saving    *
                 * ------------------------------------------------- */
                if (ld->model_type == ADVANCED_MODEL)
                      if ( *i_v7310_power_ctl != NULL ) {
                                  (**i_v7310_power_ctl)( POWER_OFF );
                      }
#endif	/* PANEL_PROTECTION */
                ld->lcd_power = TURN_OFF;
                break;

        case CRT_ON: 
                /* --------------------------------------- * 
		 *          7:   CRT ON                    *
                 * --------------------------------------- */
                rc = device_power ( pd, CRT, PM_PLANAR_ON ); 
                ld->crt_power = TURN_ON;
                break;

        case CRT_OFF: 
                /* --------------------------------------- * 
		 *          8:   CRT OFF                   *
                 * --------------------------------------- */
                rc = device_power ( pd, CRT, PM_PLANAR_OFF ); 
                ld->crt_power = TURN_OFF;
                break;

        default:
                if ( ddf->dpms_enabled == FALSE )
                        return (PM_SUCCESS);
        
#ifdef	PANEL_PROTECTION

                switch ( phase ) {
        
                case DPMS_ON:
                        /* --------------------------------------- * 
		         *          1:   DPMS ON                   *
                         * --------------------------------------- */
			/*
			 * The core X server calls DPMS ON when its start up.
			 * If you set CRT only resolution (i.e. XGA), 
			 * we have to prevent to turn on the LCD panel for 
			 * panel protection.
			 * We have to check the current video mode uses 
			 * the LCD, then turn on the LCD planar power.
			 */
			if (ld->lcd_power == TURN_ON) {
				rc = device_power (pd, LCD_BACKLIGHT, 
						   PM_PLANAR_ON ); 
			}
			if (ld->crt_power == TURN_ON) {
				rc = device_power ( pd, CRT, PM_PLANAR_ON ); 
			}
                        pmhdl->mode = PM_DEVICE_ENABLE;
                        break;

                case DPMS_STANDBY:
                        /* --------------------------------------- *
		         *          2:   DPMS STANDBY              *
                         * --------------------------------------- */

			if (ld->lcd_power == TURN_ON) {
				rc = device_power (pd, LCD_BACKLIGHT, 
						   PM_PLANAR_OFF ); 
			}
			if (ld->crt_power == TURN_ON) {
				rc = device_power ( pd, CRT, PM_PLANAR_OFF ); 
			}
                        pmhdl->mode = PM_DEVICE_DPMS_STANDBY;
                        break;

                case DPMS_SUSPEND:
                        /* --------------------------------------- *
		         *          3:   DPMS SUSPEND              *
                         * --------------------------------------- */
			if (ld->lcd_power == TURN_ON) {
				rc = device_power (pd, LCD_BACKLIGHT, 
						   PM_PLANAR_OFF ); 
			}
			if (ld->crt_power == TURN_ON) {
				rc = device_power ( pd, CRT, PM_PLANAR_OFF ); 
			}
                        pmhdl->mode = PM_DEVICE_DPMS_SUSPEND;
                        break;

                case DPMS_OFF:
                        /* --------------------------------------- *
		         *          4:   DPMS OFF                  *
                         * --------------------------------------- */
			if (ld->lcd_power == TURN_ON) {
				rc = device_power (pd, LCD_BACKLIGHT, 
						   PM_PLANAR_OFF ); 
			}
			if (ld->crt_power == TURN_ON) {
				rc = device_power ( pd, CRT, PM_PLANAR_OFF ); 
			}
                        pmhdl->mode = PM_DEVICE_DPMS_SUSPEND;
                        break;
                default:
                        return (PM_ERROR);
                }
#else	/* PANEL_PROTECTION */
                switch ( phase ) {
        
                case DPMS_ON:
                        /* --------------------------------------- * 
		         *          1:   DPMS ON                   *
                         * --------------------------------------- */

                        rc = device_power ( pd, LCD, PM_PLANAR_ON ); 
                        rc = device_power ( pd, CRT, PM_PLANAR_ON ); 
                        pmhdl->mode = PM_DEVICE_ENABLE;
                        break;

                case DPMS_STANDBY:
                        /* --------------------------------------- *
		         *          2:   DPMS STANDBY              *
                         * --------------------------------------- */

                        rc = device_power ( pd, LCD, PM_PLANAR_OFF ); 
                        rc = device_power ( pd, CRT, PM_PLANAR_OFF ); 
                        pmhdl->mode = PM_DEVICE_DPMS_STANDBY;
                        break;

                case DPMS_SUSPEND:
                        /* --------------------------------------- *
		         *          3:   DPMS SUSPEND              *
                         * --------------------------------------- */
                        rc = device_power ( pd, LCD, PM_PLANAR_OFF ); 
                        rc = device_power ( pd, CRT, PM_PLANAR_OFF ); 
                        pmhdl->mode = PM_DEVICE_DPMS_SUSPEND;
                        break;

                case DPMS_OFF:
                        /* --------------------------------------- *
		         *          4:   DPMS OFF                  *
                         * --------------------------------------- */
                        rc = device_power ( pd, LCD, PM_PLANAR_OFF ); 
                        rc = device_power ( pd, CRT, PM_PLANAR_OFF ); 
                        pmhdl->mode = PM_DEVICE_DPMS_SUSPEND;
                        break;
                default:
                        return (PM_ERROR);
                }
#endif	/* PANEL_PROTECTION */
		break;
        }

        return (rc);
}


/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : device_power()                                       *
 *                                                                         *
 * DESCRIPTIVE NAME : Device power control routine                         *
 *                                                                         *
 * FUNCTIONS : Control the specified device power.                         *
 *                                                                         *
 * INPUTS    : Pointer to phys_displays structure                          *
 *             Device (LCD or CRT)                                         *
 *             Power On/Off flag (PM_PLANAR_ON/PM_PLANAR_OFF)              *
 *                                                                         *
 * OUTPUTS   : -1 if error occured, otherwise 0                            *
 * CALLED BY : wfg_reset()  and vttdpm()                                   *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

int
device_power( pd, device, pm_level )
        struct phys_displays *pd;
	int                  device;
	int                  pm_level;
{
        struct wfg_pmdata *pmdata;
        struct vtmstruc   *vp;        
        struct wfg_data   *ld;
        struct pm_handle  *pmhdl;
        struct fbdds      *dds;
        struct wfg_ddf    *ddf;
        unsigned char     pr19_val;
        ulong             save_io_base;
        int               rc;

        ld    = (struct wfg_data *)  pd->visible_vt->vttld;
        pmhdl = (struct pm_handle *) ld->wfg_pmh;
        dds   = (struct fbdds *)     pd->odmdds;
        ddf   = (struct wfg_ddf *)   ld->ddf;


        switch ( device ){

#ifdef	PANEL_PROTECTION
        case LCD_BACKLIGHT:

                /* ====================================================== *
                 *  LCD Panel power on/off  by using pm_planar_control()  *
                 * ====================================================== */
                rc = wfg_backlight_control (ddf, dds, pmhdl->devno, 
					    dds->pm_lcd_device_id, pm_level );

                if ( rc != PM_SUCCESS ) {
                        BUGLPR (dbg_vttdpm, BUGNTA, 
                                        ("wfg_backlight_control failure\n"));
                        wfg_err (NULL, "PCIWFG","DEVICE_POWER",
                                             "LCD_BACKLIGHT", pmhdl, 
                                                 WFG_PM_ERROR, RAS_UNIQUE_1);
                }
                break;
#endif	/* PANEL_PROTECTION */

        case LCD:

#ifdef	PANEL_PROTECTION
		if (pm_level == PM_PLANAR_ON) {
			if (ld->model_type == ADVANCED_MODEL && 
			    *i_v7310_power_ctl != NULL ) {
				(**i_v7310_power_ctl)( LCD_POWER_ON );
			}
			rc = pm_planar_control (pmhdl->devno, 
						dds->pm_lcd_device_id, pm_level );
		} else {
			rc = pm_planar_control (pmhdl->devno, 
						dds->pm_lcd_device_id, pm_level );
			if (ld->model_type == ADVANCED_MODEL && 
			    *i_v7310_power_ctl != NULL ) {
				(**i_v7310_power_ctl)( LCD_POWER_OFF );
			}
		}
#else	/* PANEL_PROTECTION */
                /* ====================================================== *
                 *  LCD Panel power on/off  by using pm_planar_control()  *
                 * ====================================================== */
                rc = pm_planar_control (pmhdl->devno, 
                                             dds->pm_lcd_device_id, pm_level );
#endif	/* PANEL_PROTECTION */

                if ( rc != PM_SUCCESS ) {
                        BUGLPR (dbg_vttdpm, BUGNTA, 
                                        ("pm_planar_control failure\n"));
                        wfg_err (NULL, "PCIWFG","DEVICE_POWER",
                                             "LCD", pmhdl, 
                                                 WFG_PM_ERROR, RAS_UNIQUE_1);
                }
                break;
    
        case CRT:


#ifdef	PANEL_PROTECTION
		if (pm_level == PM_PLANAR_ON) {
			if (ld->model_type == ADVANCED_MODEL && 
			    *i_v7310_power_ctl != NULL ) {
				(**i_v7310_power_ctl)( CRT_POWER_ON );
			}
			rc = pm_planar_control (pmhdl->devno, 
						dds->pm_crt_device_id, pm_level);
		} else {
			rc = pm_planar_control (pmhdl->devno, 
						dds->pm_crt_device_id, pm_level);
			if (ld->model_type == ADVANCED_MODEL && 
			    *i_v7310_power_ctl != NULL ) {
				(**i_v7310_power_ctl)( CRT_POWER_OFF );
			}
		}
#else	/* PANEL_PROTECTION */
                /* ======================================================= *
                 *  CRT display power on/off by using pm_planar_control()  *
                 * ======================================================= */
                rc = pm_planar_control (pmhdl->devno, 
                                           dds->pm_crt_device_id, pm_level);
#endif	/* PANEL_PROTECTION */

                if ( rc != PM_SUCCESS ) {
                        BUGLPR (dbg_vttdpm, BUGNTA, 
                                            ("pm_planar_control failure\n"));
                        wfg_err (NULL, "PCIWFG","DEVICE_POWER",
                                             "CRT", pmhdl, 
                                                 WFG_PM_ERROR, RAS_UNIQUE_1);
                }

	case DAC:
		
                if ( pm_level == PM_PLANAR_ON ) {
          
                        /* ------------------------------------ *
                         *          WD90C24A2 DAC on            *
                         * ------------------------------------ */
                        save_io_base = ld->IO_base_address;
                        ld->IO_base_address = 
                                    PCI_BUS_ATT(&ddf->pci_io_map);
              
                        PUT_CRTC_REG(PR1B, PR1B_UNLOCK);
              
                        /* --------------------------------- *
                         *    PR18 Flat Panel Control reg    *
                         * --------------------------------- */
                        PCI_PUTC(CRTC_INDX, PR19);
                        pr19_val = PCI_GETC(CRTC_DATA);
              
                        /* ----------------------------------------- *
                         *   Set bit 7 to 0 for CRT Display Enable   *
                         * ----------------------------------------- */
                        PCI_PUTC(CRTC_DATA, pr19_val | 0x20);
                        PUT_CRTC_REG(PR1B, PR1B_LOCK);
              
                        PCI_BUS_DET(ld->IO_base_address);
                        ld->IO_base_address = save_io_base;
                        break;
          
                } else {
          
                        /* ------------------------------------ *
                         *          WD90C24A2 DAC off           *
                         * ------------------------------------ */
                        save_io_base = ld->IO_base_address;
                        ld->IO_base_address = 
                                 PCI_BUS_ATT(&ddf->pci_io_map);
          
                        PUT_CRTC_REG(PR1B, PR1B_UNLOCK);
          
                        /* --------------------------------- *
                         *    PR18 Flat Panel Control reg    *
                         * --------------------------------- */
                        PCI_PUTC(CRTC_INDX, PR19);
                        pr19_val = PCI_GETC(CRTC_DATA);
          
                        /* ----------------------------------------- *
                         *   Set bit 7 to 1 for CRT Display Disable  *
                         * ----------------------------------------- */
                        PCI_PUTC(CRTC_DATA, pr19_val & ~0x20);
                        PUT_CRTC_REG(PR1B, PR1B_LOCK);
          
                        PCI_BUS_DET(ld->IO_base_address);
                        ld->IO_base_address = save_io_base;
                        break;
          
                }
        }

        return ( rc );
}
