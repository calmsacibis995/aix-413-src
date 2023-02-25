static char sccsid[] = "@(#)36  1.7  src/rspc/kernext/disp/pciwfg/ksr/wfg_pm.c, pciwfg, rspc41J, 9521A_all 5/23/95 08:31:46";
/* wfg_pm.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: setup_powermgt, wfg_pm_handler, power_control
 *              pm_save_register, reconfig_wfg, restore_data
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
#include "wfgenv.h"

int (**i_v7310_power_ctl)();

BUGXDEF (dbg_setup_powermgt);

#ifdef	PANEL_PROTECTION

#define	OFF	0
#define	ON	1
#define	SET	2
#define	GET	3
#define	CARRERA_ENABLE	0x10
#define	CARRERA_DISABLE	0x20
#define	PMOUT00	0x01	/* LCD Backlight */
#define	PMOUT01	0x02	/* LCDOFF (VCC5P) */
#define	PMOUT02	0x04	/* LCDEN */
#define	PMOUT03	0x08	/* LCD Blight */
#define	CARRERA_ID	0x14101c00

#define	SWAPL(l)	((((l)&0xff000000)>>24)|(((l)&0x00ff0000)>>8)|\
			 (((l)&0x0000ff00)<<8)|(((l)&0x000000ff)<<24))

static int lcd_wait_for_timeout = -1;

static long
wfg_get_pci(struct wfg_ddf *ddf, unsigned long addr, unsigned long offset)
{
	struct io_map pci;
	unsigned long bus_val, val;

	pci.key = IO_MEM_MAP;
	pci.flags = 0;		/* Read/Write */
	pci.size = 256;		/* 256 bytes */
	pci.bid = ddf->pci_io_map.bid;
	pci.busaddr = 0x0CF8;	/* PCI config */
	
	bus_val = (unsigned long)iomem_att(&pci);

	*((volatile unsigned long *)(bus_val)) = SWAPL(addr+offset);
	EIEIO;
	val = *((volatile unsigned long *)(bus_val+0x0004));

	iomem_det((void *)(bus_val));

	return val;
}

static long
wfg_put_pci(struct wfg_ddf *ddf, unsigned long addr, unsigned long offset,
	    unsigned long val)
{
	struct io_map pci;
	unsigned long bus_val;

	pci.key = IO_MEM_MAP;
	pci.flags = 0;		/* Read/Write */
	pci.size = 256;		/* 256 bytes */
	pci.bid = ddf->pci_io_map.bid;
	pci.busaddr = 0x0CF8;	/* PCI config */
	
	bus_val = (unsigned long)iomem_att(&pci);

	*((volatile unsigned long *)(bus_val)) = SWAPL(addr+offset);
	EIEIO;
	*((volatile unsigned long *)(bus_val+0x0004)) = val;
	EIEIO;

	iomem_det((void *)(bus_val));

	return val;
}

unsigned char
wfg_carrera_control(struct wfg_ddf *ddf, unsigned char bit, int flag)
{
	unsigned long bus_val, bus_addr;
	unsigned char tmp=0, val, pmctrl;
	struct io_map car;
	int old_level, slot, i;

	/*
	 * Search base address of carrera
	 */
	for (slot=6; slot >= 0; slot--) {
		unsigned long addr = 0x80005000 + 0x0800 * slot;

		if (wfg_get_pci(ddf,addr,0L) == CARRERA_ID) {
			/*
			 * Enable CARRERA (I/O access enable)
			 * Write I/O accesss bit (bit1) to PCI cmd reg.
			 */
			bus_addr = SWAPL(wfg_get_pci(ddf,addr,0x04L));
			wfg_put_pci(ddf,addr,0x04L,SWAPL(bus_addr|0x01L));

			/*
			 * Read base address
			 */
			bus_addr = SWAPL(wfg_get_pci(ddf,addr,0x10L)) & ~0x3;
			break;
		} else {
			continue;
		}
	}

	if (slot < 0) {
		return (1);
	}

	car.key = IO_MEM_MAP;
	car.flags = 0;		/* Read/Write */
	car.size = 256;		/* 256 bytes */
	car.bid = ddf->pci_io_map.bid;
	car.busaddr = bus_addr & 0x0000FFFF;	/* Carerra IO address */
	
	old_level = i_disable(INTMAX);
	bus_val = (unsigned long)iomem_att(&car);

	switch (flag) {
	case CARRERA_ENABLE:
		*((volatile unsigned char *)(bus_val)) = (uchar)0x00;
		EIEIO;
		pmctrl = *((unsigned char *)(bus_val + 1));
		*((volatile unsigned char *)(bus_val + 1)) = (pmctrl | 0x80);
		EIEIO;
		break;
	case CARRERA_DISABLE:
		*((volatile unsigned char *)(bus_val)) = (uchar)0x00;
		EIEIO;
		pmctrl = *((unsigned char *)(bus_val + 1));
		*((volatile unsigned char *)(bus_val + 1)) = (pmctrl & ~0x80);
		EIEIO;
		break;
	case ON:
		*((volatile unsigned char *)(bus_val)) = (uchar)0x0C;
		EIEIO;
		tmp = *((unsigned char *)(bus_val + 1));
		val = tmp | bit;
		*((volatile unsigned char *)(bus_val + 1)) = val;
		EIEIO;
		break;
	case OFF:
		*((volatile unsigned char *)(bus_val)) = (uchar)0x0C;
		EIEIO;
		tmp = *((unsigned char *)(bus_val + 1));
		val = tmp & ~bit;
		*((volatile unsigned char *)(bus_val + 1)) = val;
		EIEIO;
		break;
	case SET:
		*((volatile unsigned char *)(bus_val)) = (uchar)0x0C;
		EIEIO;
		tmp = *((unsigned char *)(bus_val + 1));
		*((volatile unsigned char *)(bus_val + 1)) = bit;
		EIEIO;
		break;
	case GET:
	default:
		*((volatile unsigned char *)(bus_val)) = (uchar)0x0C;
		EIEIO;
		tmp = *((unsigned char *)(bus_val + 1));
		break;
	}

	iomem_det((void *)(bus_val));
	i_enable(old_level);

	return (tmp);
}

/*
 * Timeout proc
 */
void
lcd_timeout_proc(void *arg)
{
	lcd_wait_for_timeout = 0;
}


/*
 * wfg_timeoutcf()
 * This is called from wfg_define/undefine function in unpin code.
 * timeoutcf() kernel service have to be called from pinned code.
 * So this wrap interface is introduced.
 */
int
wfg_timeoutcf(int cocnt)
{
	return timeoutcf(cocnt);
}

/*
 * Control LCD backlight
 */
int
wfg_backlight_control(struct wfg_ddf *ddf, struct fbdds *dds,
		      int devno, int devid, int level)
{
	int saved_level, rc;

        saved_level = i_disable(INTMAX);
	if (level == PM_PLANAR_ON) {
		/*
		 * Turn on LCD backlight
		 */
		if (pm_planar_control(devno,devid,PM_PLANAR_QUERY) == 0) {
#ifdef NO_H8_CONTROL
                        rc = wfg_carrera_control(ddf,PMOUT00,ON);
#else
			/*
			 * Disable the Carrera to move the LCD
			 * control to the H8.
			 * The LCD backlight should be controled 
			 * H8. So I don't enable PMOUT00 here.
			 */
			rc = wfg_carrera_control(ddf,0,CARRERA_DISABLE);
#endif /* NO_H8_CONTROL */
		} else {
			/*
			 * PMMD exist. Use pm_planar_control()
			 */
			rc = pm_planar_control(devno,devid,level);
		}
	} else {
		if (pm_planar_control(devno,devid,PM_PLANAR_QUERY) == 0) {
			/*
			 * Turn off LCD backlight bit (PMOUT00) of the
			 * Carrera. But, H8 controls LCD panel signals
			 * at this time. So nothing would be happen 
			 * unless the Carrea get the control.
			 */
			rc = wfg_carrera_control(ddf,PMOUT00,OFF);
#ifdef NO_H8_CONTROL
			/*
			 * Enable the Carrera to get the LCD panel signal
			 * control from H8.
			 */
			rc |= wfg_carrera_control(ddf,0,CARRERA_ENABLE);
#endif /* NO_H8_CONTROL */
		} else {
			/*
			 * PMMD exist. Use pm_planar_control()
			 */
			rc = pm_planar_control(devno,devid,level);
		}
	}
        i_enable(saved_level);

	return (rc);
}

int
wfg_planar_control(struct wfg_ddf *ddf, struct fbdds *dds,
		   int devno, int devid, int level)
{
	int saved_level;
	static unsigned char lcden = 0;

	/*
	 * If LCD ON request comes from VCA DD, check the 
	 */
	if (devno == 0 && devid == dds->pm_lcd_device_id) {
		/*
		 * If the first time, wait for 100msec for safty reason.
		 */
		if (lcd_wait_for_timeout == -1) {
			lcd_wait_for_timeout = 1;
			timeout(lcd_timeout_proc,NULL,HZ/10);
		}

		while (lcd_wait_for_timeout) {
			delay(2);
		}
		saved_level = i_disable(INTMAX);
		if (level == PM_PLANAR_ON) {
			if (lcden & PMOUT02) {
				wfg_carrera_control(ddf,PMOUT02,ON);
			}
		} else {
			lcden= wfg_carrera_control(ddf,0,GET);
			wfg_carrera_control(ddf,PMOUT02,OFF);
		}
		i_enable(saved_level);
		lcd_wait_for_timeout = 1;
		timeout(lcd_timeout_proc,NULL,HZ/10);

		return (0);
	}

	/*
	 * If pm_planar_control is not active and target device is LCD,
	 * control CARERRA directly to turn on/off LCD backlight and 
	 * enabling signal. In other cases, use pm_planar_control() function.
	 */
	if (devid == dds->pm_lcd_device_id) {

		/*
		 * If the first time, wait for 100msec for safty reason.
		 */
		if (lcd_wait_for_timeout == -1) {
			lcd_wait_for_timeout = 1;
			timeout(lcd_timeout_proc,NULL,HZ/10);
		}

		if (level == PM_PLANAR_ON) {
			/*
			 * (1) Turn on LCD panel power (PMOUT01)
			 *     Need to wait 5msec to stable the power.
			 *     Then turn on Blight.
			 */
			wfg_carrera_control(ddf,PMOUT01,ON);
			delay(1);	/* wait for 10msec */
			wfg_carrera_control(ddf,PMOUT03,ON);

			/*
			 * (2) Turn on LCD enable signal (PMOUT02)
			 *     Need to wait 100msec for panel protection
			 *     before enable this signal.
			 */
			while (lcd_wait_for_timeout) {
				delay(2);
			}
			wfg_carrera_control(ddf,PMOUT02,ON);

			/*
			 * (3) Turn on LCD backlight (PMOUT00)
			 */
			saved_level = i_disable(INTMAX);
			if (pm_planar_control(devno,devid,PM_PLANAR_QUERY) ==
			    0) {	/* No PMMD */
#ifdef NO_H8_CONTROL
                                wfg_carrera_control(ddf,PMOUT00,ON);
#else
				/*
				 * Disable the Carrera to move the LCD
				 * control to the H8.
				 * The LCD backlight should be controled 
				 * H8. So I don't enable PMOUT00 here.
				 */
				wfg_carrera_control(ddf,0,CARRERA_DISABLE);
#endif /* NO_H8_CONTROL */
			} else {
				pm_planar_control(devno,devid,level);
			}
			i_enable(saved_level);
		} else {
			/*
			 * Turn off LCD
			 */
			while (lcd_wait_for_timeout) {
				delay(2);
			}
			saved_level = i_disable(INTMAX);
			if (pm_planar_control(devno,devid,PM_PLANAR_QUERY) ==
			    0) {	/* No PMMD */
				/*
				 * Turn off (VBL->LCDEN->VCC5P,BLIGHT)
				 */
				wfg_carrera_control(ddf,PMOUT00,OFF);
				wfg_carrera_control(ddf,PMOUT02,OFF);
				wfg_carrera_control(ddf,PMOUT01|PMOUT03,OFF);
#ifdef NO_H8_CONTROL
				/*
				 * Enable the Carrera to move the LCD
				 * control to the Carrera.
				 */
				wfg_carrera_control(ddf,0,CARRERA_ENABLE);
#endif /* NO_H8_CONTROL */
			} else {
				/*
				 * Turn off (VBL->LCDEN->VCC5P,BLIGHT)
				 */
				pm_planar_control(devno,devid,level);
				wfg_carrera_control(ddf,PMOUT02,OFF);
				wfg_carrera_control(ddf,PMOUT01|PMOUT03,OFF);
			}
			i_enable(saved_level);
		} /* End of LCD turn off */
		/*
		 * Set timeout timer (100msec)
		 */
		lcd_wait_for_timeout = 1;
		timeout(lcd_timeout_proc,NULL,HZ/10);
	} else 	if (devid == dds->pm_crt_device_id) { /* Control CRT */
		pm_planar_control(devno,devid,level);
	} else {
		pm_planar_control(devno,devid,level);
	}
        return (0);
}

#define	pm_planar_control(a,b,c)	wfg_planar_control(ddf,dds,a,b,c)

#endif	/* PANEL_PROTECTION */

#ifdef	PANEL_PROTECTION
struct wfg_ddf *wfgDDF = NULL;
struct fbdds *wfgDDS = NULL;
#endif	/* PANEL_PROTECTION */

/***********************************************************************
 *                                                                     *
 * IDENTIFICATION: SETUP_POWERMGT                                      *
 *                                                                     *
 * DESCRIPTIVE NAME: Initialize Power Management Structure             *
 *                                                                     *
 * FUNCTION:  Initialize the pm_handle structure for Power Management  *
 *                                                                     *
 * INPUTS:  - Vitrual terminal pointer                                 *
 *                                                                     *
 * OUTPUTS:   None                                                     *
 *                                                                     *
 * CALLED BY: vttinit                                                  *
 *                                                                     *
 * CALLS:     None.                                                    *
 *                                                                     *
 ***********************************************************************/

long
setup_powermgt ( vp )
        struct vtmstruc *vp;
{
        static struct wfg_pmdata Wfg_pmdata;
        struct pm_handle *pmh;
        struct wfg_data *ld;
        struct fbdds  *dds;
        int           rc=0, i;

        Wfg_pmdata.vvt  = (ulong)vp;
        dds = (struct fbdds *)    vp->display->odmdds;
        ld  = (struct wfg_data *) vp->vttld;

        pmh = (struct pm_handle *) 
                  xmalloc (sizeof (struct pm_handle), 3, pinned_heap);

        if (pmh == NULL)
        {
        BUGLPR (dbg_setup_powermgt, BUGNTA, ("PMH xmalloc failure\n"));
                wfg_err (NULL, "PCIWFG","SETUP_POWERMGT","XMALLOC", 
                        pmh, WFG_BAD_XMALLOC, RAS_UNIQUE_1);
                return (ENOMEM);
        }

        ld->wfg_pmh = (struct pm_handle *)pmh;

        pmh->activity                = PM_ACTIVITY_NOT_SET;
        pmh->mode                    = PM_DEVICE_FULL_ON;
        pmh->device_idle_time1       = dds->pm_dev_itime[0];
        pmh->device_idle_time2       = dds->pm_dev_itime[1];
        pmh->device_idle_time        = dds->pm_dev_itime[2];
        pmh->device_standby_time     = 0;
        pmh->handler                 = wfg_pm_handler;
        pmh->private                 = (unsigned char *)&Wfg_pmdata;
        pmh->devno                   = vp->display->devno;
        pmh->attribute               = dds->pm_dev_att;
        pmh->device_logical_name     = dds->component;
        pmh->pm_version              = 0x100;
        pmh->extension               = NULL;

        for ( i=0; i< sizeof(pmh->reserve); i++ )
                pmh->reserve[i] = NULL;

        rc = pm_register_handle( pmh, PM_REGISTER );

        if (rc != PM_SUCCESS ) {
            BUGLPR (dbg_setup_powermgt, BUGNTA, 
                                 ("pm_register_handle failure\n"));
            wfg_err (NULL, "PCIWFG","SETUP_POWERMGT","PM_REGISTER_HANDLE", 
                        pmh, WFG_PM_ERROR, RAS_UNIQUE_1);
            xmfree (pmh, pinned_heap);
        }

        return (rc);
}

BUGXDEF (dbg_wfg_pm_handler);
/***********************************************************************
 *                                                                     *
 * IDENTIFICATION: WFG_PM_HANDLER                                      *
 *                                                                     *
 * DESCRIPTIVE NAME: Manages/Controls the Display and Panel Power      *
 *                                                                     *
 * FUNCTION:  Manages/Controls the Display Power and Panel Power       *
 *            by using pm_planar_control kernel service and/or         *
 *            Video Capture DD's exported function.                    *
 *            This function is used for minimize display/panel         *
 *            power consumption.                                       *
 *                                                                     *
 * INPUTS:  - Private Pointer for using vtmstruc and ps_s structures   *
 *          - Power Management Mode                                    *
 *                The value must be one of the following.              *
 *                           - PM_DEVICE_FULL_ON                       *
 *                           - PM_DEVICE_ENABLE                        *
 *                           - PM_DEVICE_IDLE                          *
 *                                                                     *
 * OUTPUTS:   None                                                     *
 *                                                                     *
 * CALLED BY: Power Management Core in Kernel                          *
 *                                                                     *
 * CALLS:     None                                                     *
 *                                                                     *
 ***********************************************************************/

int 
wfg_pm_handler (private, mode)
        caddr_t  private;
        int         mode;
{
        struct wfg_pmdata *pmdata;
        struct vtmstruc   *vp;        
        struct wfg_data   *ld;
        struct pm_handle  *pmhdl;
        struct fbdds      *dds;
        struct phys_displays *pd;
        struct wfg_ddf    *ddf;
        unsigned char     pr18_val;
        ulong             save_io_base;
        int               rc=PM_SUCCESS;
        int               saved_level;

/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */
{
BUGLPR(dbg_wfg_pm_handler,BUGGID,("\n"));
BUGLPR(dbg_wfg_pm_handler,BUGGID,("Input parameters follow:\n"));
BUGLPR(dbg_wfg_pm_handler,BUGGID,("mode              = 0x%x\n",mode));
}


        pmdata = (struct wfg_pmdata *)    private;
        vp     = (struct vtmstruc *)      pmdata->vvt;
        ld     = (struct wfg_data *)      vp->vttld;
        pmhdl  = (struct pm_handle *)     ld->wfg_pmh;
        dds    = (struct fbdds *)         vp->display->odmdds;
        ddf    = (struct wfg_ddf *)       ld->ddf;
        pd     = (struct phys_displays *) vp->display;

#ifndef	PANEL_PROTECTION
        /* ------------------------------------------ *
         *          Interrrupt disable                *
         * ------------------------------------------ */
        saved_level = i_disable(INTMAX);
#endif	/* PANEL_PROTECTION */

        /* ---------------------------------------------------------------- *
         * Clear the "dpms_enabled" variable to tell driver to disable DPMS *
         * ---------------------------------------------------------------- */
        ddf->dpms_enabled = FALSE;

        switch ( mode ){

        case PM_DEVICE_FULL_ON:

                switch ( pmhdl->mode ){
                case PM_DEVICE_ENABLE:
                    pmhdl->mode = mode;
                    ddf->dpms_enabled = TRUE;
                    break;
                default:
                    rc = PM_ERROR;
                    break;
                }
                break;

        case PM_DEVICE_ENABLE:

                switch ( pmhdl->mode ){

#ifdef	PANEL_PROTECTION
                case PM_DEVICE_FULL_ON:
                    /* ------------------------------------------------ *
                     * Turn the previous output device on (CRT/&LCD)    *
                     * ------------------------------------------------ */
                    rc = power_control( vp, POWER_ON, PM_PLANAR_ON );
                    break;

                case PM_DEVICE_DPMS_STANDBY:
                case PM_DEVICE_DPMS_SUSPEND:
                case PM_DEVICE_DPMS_OFF:
                    /* ------------------------------------------------ *
                     * Turn the previous output device on (CRT/&LCD)    *
                     * ------------------------------------------------ */
		    if ( ld->crt_power == TURN_ON ) {
			    rc = device_power(pd,CRT,PM_PLANAR_ON);
		    }
		    if ( ld->lcd_power == TURN_ON ) {
			    rc = device_power(pd,LCD_BACKLIGHT,PM_PLANAR_ON);
		    }
                    break;

#else	/* PANEL_PROTECTION */
                case PM_DEVICE_FULL_ON:
                case PM_DEVICE_DPMS_STANDBY:
                case PM_DEVICE_DPMS_SUSPEND:
                case PM_DEVICE_DPMS_OFF:

                    /* ------------------------------------------------ *
                     * Turn the previous output device on (CRT/&LCD)    *
                     * ------------------------------------------------ */
                    rc = power_control( vp, POWER_ON, PM_PLANAR_ON );
                    break;
#endif	/* PANEL_PROTECTION */

                case PM_DEVICE_SUSPEND:
                case PM_DEVICE_HIBERNATION:

                    /* ------------------------------------------------ *
                     *    Reconfigure the device 'WD90C24A2'            *
                     * ------------------------------------------------ */
                      reconfig_wfg( pd );
                    break;

                default:
                    break;
                }
                pmhdl->mode = mode;
                break;

        case PM_DEVICE_DPMS_STANDBY:
                rc = device_power ( pd, CRT, PM_PLANAR_LOWPOWER1 );
#ifdef	PANEL_PROTECTION
                rc = device_power ( pd, LCD_BACKLIGHT, PM_PLANAR_OFF );
#else	/* PANEL_PROTECTION */
                rc = device_power ( pd, LCD, PM_PLANAR_OFF );
#endif	/* PANEL_PROTECTION */
                pmhdl->mode = mode;
                break;

        case PM_DEVICE_DPMS_SUSPEND:
                rc = device_power ( pd, CRT, PM_PLANAR_LOWPOWER2 );
#ifdef	PANEL_PROTECTION
                rc = device_power ( pd, LCD_BACKLIGHT, PM_PLANAR_OFF );
#else	/* PANEL_PROTECTION */
                rc = device_power ( pd, LCD, PM_PLANAR_OFF );
#endif	/* PANEL_PROTECTION */
                pmhdl->mode = mode;
                break;

        case PM_DEVICE_DPMS_OFF:
                rc = device_power ( pd, CRT, PM_PLANAR_OFF );
#ifdef	PANEL_PROTECTION
                rc = device_power ( pd, LCD_BACKLIGHT, PM_PLANAR_OFF );
#else	/* PANEL_PROTECTION */
                rc = device_power ( pd, LCD, PM_PLANAR_OFF );
#endif	/* PANEL_PROTECTION */
                pmhdl->mode = mode;
                break;

        case PM_DEVICE_SUSPEND:
        case PM_DEVICE_HIBERNATION:
                shutdown_wfg( pd );
                pmhdl->mode = mode;
                break;


        case PM_PAGE_FREEZE_NOTICE:
		break;

        default:
                break;
        }


#ifndef	PANEL_PROTECTION
        /* ------------------------------------------ *
         *          Interrrupt enable                 *
         * ------------------------------------------ */
        i_enable(saved_level);
#endif	/* PANEL_PROTECTION */

        return(rc);
} 

/***********************************************************************
 *                                                                     *
 * IDENTIFICATION: power_control()                                     *
 *                                                                     *
 * DESCRIPTIVE NAME: Control output device power                       *
 *                                                                     *
 * FUNCTION:  Device power control with previous output device check   *
 *            routine                                                  *
 *                                                                     *
 * INPUTS:  - Vitrual terminal pointer                                 *
 *          - Power Management Mode                                    *
 *          - Power management level (argument of pm_planar_control()) *
 *                                                                     *
 * OUTPUTS:   None                                                     *
 *                                                                     *
 * CALLED BY: wfg_pm_handler()                                         *
 *                                                                     *
 * CALLS:     None.                                                    *
 *                                                                     *
 ***********************************************************************/

int
power_control( vp, mode, pm_level )
        struct vtmstruc *vp;
        int             mode;
        int             pm_level;
{
        struct phys_displays  *pd;
        struct wfg_data       *ld;
        int                   rc;

        pd     = (struct phys_displays *) vp->display;
        ld     = (struct wfg_data *)      vp->vttld;

#ifdef	PANEL_PROTECTION
	/*
	 * Calling routine of v7310_power_control is moved to
	 * device_power(CRT) function.
	 */
#else	/* PANEL_PROTECTION */
        /* ------------------------------------------------- *
         *   Calls v7310 function for strict power saving    *
         * ------------------------------------------------- */
        if (ld->model_type == ADVANCED_MODEL) 
              if ( ld->lcd_power == TURN_ON && ld->crt_power == TURN_ON )
                    if ( *i_v7310_power_ctl != NULL )
                          (**i_v7310_power_ctl)( mode );
#endif	/* PANEL_PROTECTION */

        /* -------------------------------------------------------- *
         *   LCD Panel power on/off  by using pm_planar_control()   *
         * -------------------------------------------------------- */
        if ( ld->lcd_power == TURN_ON ) {
    
              if ( mode == POWER_OFF )
                   device_power ( pd, LCD, PM_PLANAR_OFF );
              else
                   device_power ( pd, LCD, pm_level );
        }
    
        /* -------------------------------------------------------- *
         *  CRT display power on/off by using pm_planar_control()   *
         * -------------------------------------------------------- */
        if ( ld->crt_power == TURN_ON ) 
                   device_power ( pd, CRT, pm_level );

        return ( PM_SUCCESS );
}

/***************************************************************************
 *                                                                         *
 * FUNCTION    : shutdown_wfg()                                            *
 *                                                                         *
 * DESCRIPTION : Shutdown the device 'WD90C24A2'                           *
 *                  1. Interrupt disable                                   *
 *                  2. Enable PCI bus                                      *
 *                  3. Save some register data                             *
 *                  4. Interrupt enable                                    *
 *                                                                         *
 ***************************************************************************/
long
shutdown_wfg (pd)
        struct phys_displays *pd;
{
        struct wfg_data    *ld;
        struct wfg_ddf     *ddf;
        struct vtmstruc    *vp;
        struct fbdds       *dds;
        struct pm_handle   *pmhdl;
        int                 rc;

        vp    = (struct vtmstruc *)  pd->visible_vt;
        ld    = (struct wfg_data *)  pd->visible_vt->vttld;
        dds   = (struct fbdds    *)  pd->odmdds;
        ddf   = (struct wfg_ddf  *)  ld->ddf;
        pmhdl = (struct pm_handle *) ld->wfg_pmh;

        if ( ld->gp_flag != WFG_GP ){
            /* ------------------------------------- *
             *   Inactivate the virtual terminal     *
             *              (set vtt_activate=0)     *
             * ------------------------------------- */
             ld->Vttenv.vtt_active = VTT_INACTIVE;
        }


#ifdef	PANEL_PROTECTION
	/*
	 * Turn off LCD panel first for panel protection
	 */
        rc = pm_planar_control (pmhdl->devno, 
                                dds->pm_lcd_device_id, PM_PLANAR_OFF );

	/*
	 * Save register data
	 */
        pm_save_register( pd );

	/*
	 * Turn off DAC
	 */
        rc = device_power ( pd, DAC, PM_PLANAR_OFF );

	/*
	 * Turn off graphics controller
	 */
        rc = pm_planar_control (pmhdl->devno, 
                                dds->pm_gcc_device_id, PM_PLANAR_OFF );
#else	/* PANEL_PROTECTION */
        /* ------------------------------------------ *
         *   Save some register data                  *
         * ------------------------------------------ */
        pm_save_register( pd );

        /* ------------------------------------------ *
         *   Graphics Controller Off                  *
         * ------------------------------------------ */
        rc = device_power ( pd, CRT, PM_PLANAR_OFF );

        rc = pm_planar_control (pmhdl->devno, 
                                dds->pm_gcc_device_id, PM_PLANAR_OFF );
        rc = pm_planar_control (pmhdl->devno, 
                                dds->pm_lcd_device_id, PM_PLANAR_OFF );
        rc = pm_planar_control (pmhdl->devno, 
                                dds->pm_crt_device_id, PM_PLANAR_OFF );
#endif	/* PANEL_PROTECTION */

        return (0);
}

/***************************************************************************
 *                                                                         *
 * FUNCTION    : reconfig_wfg()                                            *
 *                                                                         *
 * DESCRIPTION : Restore procedure routine                                 *
 *                  1. Interrupt disable                                   *
 *                  2. Power on the graphics controller                    *
 *                  3. Enable PCI bus                                      *
 *                  4. Reset the hardware registers                        *
 *                  5. Restore the saved data                              *
 *                  6. Interrupt enable                                    *
 *                                                                         *
 ***************************************************************************/
long
reconfig_wfg (pd)
        struct phys_displays *pd;
{
        struct wfg_data   *ld;
        struct wfg_ddf    *ddf;
        struct pm_handle  *pmhdl;
        struct vtmstruc   *vp;
        struct fbdds      *dds;
        int                rc;

        vp    = (struct vtmstruc *)  pd->visible_vt;
        ld    = (struct wfg_data *)  pd->visible_vt->vttld;
        dds   = (struct fbdds    *)  pd->odmdds;
        ddf   = (struct wfg_ddf  *)  ld->ddf;
        pmhdl = (struct pm_handle *) ld->wfg_pmh;

        /* ------------------------------------------------- *
         *    Without turning video sub-system off,          *
         *    PM_DEVICE_SUSPEND/PM_DEVICE_HIBERNATION and    *
         *    PM_DEVICE_ENABLE may be sent                   *
         *    alternatively again and again.                 *
         * ------------------------------------------------- */
        rc = pm_planar_control (pmhdl->devno, 
				dds->pm_gcc_device_id, PM_PLANAR_ON );

        /* ------------------------------------------ *
         * Takes care of PCI configuration space      *
         *  (Set up the "pos" regs for this adapter)  *
         * ------------------------------------------ */
        rc = enable_wfg( ddf, dds );

        if (rc != 0) {
             return rc;
        }
        
        /* ------------------------------- *
         *      Calls reset routine        *
         * ------------------------------- */
        wfg_reset( vp, ld, ddf );

        /* ------------------------------- *
         *    Restore the saved data       *
         * ------------------------------- */
        restore_data ( pd );

        /* ------------------------------------ *
         *    Reactivate VTT (only KSR mode)    *
         * ------------------------------------ */
        if ( ld->gp_flag != WFG_GP ){

            /* ----------------------------------------- *
             *   Activate the virtual terminal.          *
             *   (set vtt_activate=1)                    *
             *   Restore the saved PS context to VRAM    *
             * ----------------------------------------- */
            ld->Vttenv.vtt_active = VTT_ACTIVE;
            vttact(vp);
        
            /* ---------------------------------------------- *
             * Turn the previous output device on (CRT/&LCD)  *
             * If mode is graphic, X server calls             *
             * a system call aixgsc(DISP_PM) for turning      *
             * on the previous output device                  *
             * ---------------------------------------------- */
            rc = power_control( vp, POWER_ON, PM_PLANAR_ON );
        }

        return (0);
}

/***************************************************************************
 *                                                                         *
 * FUNCTION    : pm_save_register()                                        *
 * DESCRIPTION : 1. Get several register data to allocated area            *
 * NOTE        : Paradis Extended Registers  - Done                        *
 *               VGA Registers               -                             *
 *               Hardware Function Registers -                             *
 *                                                                         *
 **************************************************************************/
long
pm_save_register (pd)
        struct phys_displays *pd;
{
        PM_SAVING_DATA *pSaveData;
        struct wfg_data *ld;
        struct wfg_ddf  *ddf;
        struct vtmstruc *vp;
        int i;
        long rgb_data;

        vp  = (struct vtmstruc *)pd->visible_vt;
        ld  = (struct wfg_data *)pd->visible_vt->vttld;
        ddf = (struct wfg_ddf  *)ld->ddf;

        pSaveData = &pr_savedata[0];

        /* ---------------------- *
         *     IO Bus Attach      *
         * ---------------------- */
        ld->IO_base_address = 
                 PCI_BUS_ATT(&ddf->pci_io_map);

        for ( i=0;i<sizeof( pr_savedata )/sizeof( PM_SAVING_DATA );i++ ) {

            switch( pSaveData->reg_type ){

            case PR_SEQ:

                 GET_SEQ_REG ( pSaveData->reg_index, pSaveData->reg_data );
                 break;

            case PR_GRF:

                 GET_GRF_REG ( pSaveData->reg_index, pSaveData->reg_data );
                 break;

            case PR_CRTC: 

                 GET_CRTC_REG ( pSaveData->reg_index, pSaveData->reg_data );
                 break;

            case SEQ_ULK: PUT_SEQ_REG  ( PR20, PR20_UNLOCK ); break;
            case SEQ_LK : PUT_SEQ_REG  ( PR20, PR20_LOCK   ); break;
            case GRF_ULK: PUT_GRF_REG  ( PR5,  PR5_UNLOCK  ); break;
            case GRF_LK : PUT_GRF_REG  ( PR5,  PR5_LOCK    ); break;
            case ULK10  : PUT_CRTC_REG ( PR10, PR10_UNLOCK ); break;
            case LK10   : PUT_CRTC_REG ( PR10, PR10_LOCK   ); break;
            case ULK1B  : PUT_CRTC_REG ( PR1B, PR1B_UNLOCK ); break;
            case LK1B   : PUT_CRTC_REG ( PR1B, PR1B_LOCK   ); break;
            case ULK30  : PUT_CRTC_REG ( PR30, PR30_UNLOCK ); break;
            case LK30   : PUT_CRTC_REG ( PR30, PR30_LOCK   ); break;
            }
            pSaveData++;
        }

        /* ---------------------- *
         *     IO Bus Dettach     *
         * ---------------------- */
        PCI_BUS_DET(ld->IO_base_address);

        return (0);
}

/***************************************************************************
 *                                                                         *
 * FUNCTION    : restore_data()                                            *
 *                                                                         *
 * DESCRIPTION : Restore the saved data to WD90C24A2 register              *
 *                                                                         *
 ***************************************************************************/
long
restore_data (pd)
        struct phys_displays *pd;
{
        PM_SAVING_DATA *pSavedData;
        struct wfg_data *ld;
        struct wfg_ddf  *ddf;
        struct vtmstruc *vp;
        int i;
        long rgb_data;
        int register_count;

        vp  = (struct vtmstruc *)pd->visible_vt;
        ld  = (struct wfg_data *)pd->visible_vt->vttld;
        ddf = (struct wfg_ddf  *)ld->ddf;

#ifdef	PANEL_PROTECTION
	device_power ( pd, LCD, PM_PLANAR_OFF );
	device_power ( pd, CRT, PM_PLANAR_OFF );
#endif	/* PANEL_PROTECTION */

        /* ---------------------- *
         *     IO Bus Attach      *
         * ---------------------- */
        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

            pSavedData = &pr_savedata[0];
            register_count = sizeof( pr_savedata )/sizeof( PM_SAVING_DATA );

        for ( i=0;i<register_count;i++ ) {

            switch( pSavedData->reg_type ){

            case PR_SEQ:

                 PUT_SEQ_REG ( pSavedData->reg_index, pSavedData->reg_data );
                 break;

            case PR_GRF:

                 PUT_GRF_REG ( pSavedData->reg_index, pSavedData->reg_data );
                 break;

            case PR_CRTC: 

                 PUT_CRTC_REG ( pSavedData->reg_index, pSavedData->reg_data );
                 break;

            case SEQ_ULK: PUT_SEQ_REG  ( PR20, PR20_UNLOCK ); break;
            case SEQ_LK : PUT_SEQ_REG  ( PR20, PR20_LOCK   ); break;
            case GRF_ULK: PUT_GRF_REG  ( PR5,  PR5_UNLOCK  ); break;
            case GRF_LK : PUT_GRF_REG  ( PR5,  PR5_LOCK    ); break;
            case ULK10  : PUT_CRTC_REG ( PR10, PR10_UNLOCK ); break;
            case LK10   : PUT_CRTC_REG ( PR10, PR10_LOCK   ); break;
            case ULK1B  : PUT_CRTC_REG ( PR1B, PR1B_UNLOCK ); break;
            case LK1B   : PUT_CRTC_REG ( PR1B, PR1B_LOCK   ); break;
            case ULK30  : PUT_CRTC_REG ( PR30, PR30_UNLOCK ); break;
            case LK30   : PUT_CRTC_REG ( PR30, PR30_LOCK   ); break;
            }
            pSavedData++;
        }

        /* ---------------------- *
         *     IO Bus Dettach     *
         * ---------------------- */
        PCI_BUS_DET(ld->IO_base_address);

#ifdef	PANEL_PROTECTION
	device_power ( pd, CRT, PM_PLANAR_ON );
	device_power ( pd, LCD, PM_PLANAR_ON );
#endif	/* PANEL_PROTECTION */

        return (0);
}
