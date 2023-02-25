static char sccsid[] = "@(#)07  1.17 src/rspc/kernext/pci/pcivca/vcadd_pin.c, pcivca, rspc41J, 9524A_all 6/10/95 12:30:50";
/* vcadd_pin.c */
/*
 *   COMPONENT_NAME: (PCIVCA)  PCI Video Capture Adapter Device Driver
 *
 *   FUNCTIONS: vca_intr, vca_logerr
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


/* -------------------
 * Define include file
 * -------------------*/
#include "vcastr.h"

/* ------------------------------------
 * Define routines (external and local)
 * -----------------------------------*/
int     vca_intr();
int	vca_logerr();
uchar   V7310_GETC();
int     V7310_PUTC();
uchar	RegBt812Get();
int	RegBt812Set();
int	MotionResetHW();
int	vram_copy();
long	vca_setup_vptr();
int	reset_overlay_call();
int	power_ctl_call();
int     vca_enable();
int     CfgSpaceRW();
int	_Acq_Waiting();

#ifdef	PM_SUPPORT 
long	pm_setup_vca();
int	vca_pmhandler();
int	Existing_VCA_Request_Entry();
int	Save_current_reg();
int	Restore_current_reg();
struct pm_handle *pm_vca_ptr;
static 	char	*VRegData=NULL, *BRegData=NULL;
#endif	/* PM_SUPPORT */

#ifdef	PANEL_PROTECTION
extern int	wfg_lcd_control();
#endif	/* PANEL_PROTECTION */

/* ------------------
 * Define global data
 * ------------------*/
ERR_REC(256) ER;			/* defines for vca_logerr() */

struct 	vca_area *vwork;
int	vca_lock = EVENT_NULL;


/* --------------------------------------------------------------------------
 * IDENTIFICATION:    vca_intr()
 *
 * DESCRIPTIVE NAME:  Video Capture Adaptor device interrupt handler routine
 *
 * FUNCTIONS :        This routine is the standard device driver intr routine
 *                    which interrupt handler for video buffer
 *
 * INPUTS    :        interrupt structure pointer
 *
 * OUTPUTS   :        NONE
 *
 ---------------------------------------------------------------------------*/

int vca_intr ( intr_ptr )
struct	intr *intr_ptr;
{
uchar	int_st0, int_st1, int_st2, bmt_st;

	DDHKWD5(HKWD_VCA_DD, DD_ENTRY_INTR, 0, 0, 0, 0, 0, 0);

	/* read the status register */
	int_st0 = V7310_GETC( VCA_XINTEN );
	int_st1 = V7310_GETC( VCA_INTSTA );
	int_st2 = V7310_GETC( VCA_ISTAWE );
	bmt_st = V7310_GETC( VCA_BMTCTL );

	if ((( int_st1 & INTR_USE_BITS ) == 0 ) && 
			(( bmt_st & BMT_INTR_BITS ) == 0 )) {
                vca_logerr(ERRID_VCA_INITZ, "PCIVCA", "vcadd_pin",
                	"vca_intr", INTR_FAIL, 0, "0");
					/* adapter doesn't know about it */
		return ( INTR_FAIL );	/* assume it was for somebody else */
	}

	if ( int_st1 & ACQ_INTR_BITS ) {
		VCADBG_0("vca_intr(): A");
             if ( vwork->intr_mode_set == CAPTURE_MODE ) {
	      	if ( vwork->buf_number == SECOND_BUF ) {
			vwork->buf_number = FIRST_BUF;
	      	} else {
			vwork->buf_number = SECOND_BUF;
	      	}
	     }
		V7310_PUTC( VCA_XINTEN, int_st0&~ACQ_INTR_BITS );
		V7310_PUTC( VCA_ISTAWE, int_st2|ACQ_INTR_BITS );
		V7310_PUTC( VCA_INTSTA, int_st1&~ACQ_INTR_BITS );
		V7310_PUTC( VCA_ISTAWE, int_st2&~ACQ_INTR_BITS );
		e_wakeup(&vwork->acq_event);
	}
	if ( int_st1 & HSCAN_INTR_BITS ) {
		VCADBG_0("vca_intr(): H");
		V7310_PUTC( VCA_XINTEN, int_st0&~HSCAN_INTR_BITS );
		V7310_PUTC( VCA_ISTAWE, int_st2|HSCAN_INTR_BITS );
		V7310_PUTC( VCA_INTSTA, int_st1&~HSCAN_INTR_BITS );
		V7310_PUTC( VCA_ISTAWE, int_st2&~HSCAN_INTR_BITS );
                if ( vwork->trans_flag ) {
                        V7310_PUTC( VCA_CVHSCL,
                                (uchar)(vwork->ovl_end & 0x00ff));
                        V7310_PUTC( VCA_CVHSCH,
                                (uchar)((vwork->ovl_end >> 8 ) & 0x00ff ));
                        vwork->trans_flag = FALSE;
                } else {
                   if ( vwork->ovl_end < ( vwork->yres+vwork->offset_y )) {
                        V7310_PUTC( VCA_CVHSCL,
                                (uchar)((vwork->yres+vwork->offset_y)&0x00ff));
                        V7310_PUTC( VCA_CVHSCH,
                                (uchar)(((vwork->yres+vwork->offset_y) >> 8 ) & 0x00ff ));
                        vwork->trans_flag = TRUE;
                   }
                   e_wakeup(&vwork->hscan_event);
                }
        }

	if ( bmt_st & BMT_INTR_BITS ) {
		VCADBG_0("vca_intr(): B");
		V7310_PUTC( VCA_BMTCTL, bmt_st & ~0x84 );
		e_wakeup(&vwork->bmt_event);
	}
	i_reset ( &(vwork->vca_handler) );

	DDHKWD5(HKWD_VCA_DD, DD_EXIT_INTR, 0, 0, 0, 0, 0, 0);
	return ( INTR_SUCC );
}
 

/* -------------------------------------------------------------------------
 *   Function Name : V7310_PUTC()
 *   Description   : This routine will write a 8 bit value to the specified
 *                   I/O address.
 * -------------------------------------------------------------------------*/
V7310_PUTC( addr, value )
unsigned int	addr;
unsigned char	value;
{
	int	old_level;
	unsigned long	bus_val;

	/*
	 * Disable interupts while writing registers 
         */
	old_level = i_disable(INTMAX);
	bus_val = (unsigned long)iomem_att(&vwork->pci_io_map);
	*((volatile unsigned char *)(bus_val)) = (uchar)addr;
	*((volatile unsigned char *)(bus_val + VCA_DATA_PORT)) 
					= (unsigned char)value;
   	EIEIO; 
	iomem_det((void *)(bus_val));
	/*
	 * Enable interupts while writing registers 
         */
	i_enable(old_level);
	
	return(0);
}


/* -------------------------------------------------------------------------
 *   Function Name : V7310_GETC()
 *   Description   : This routine will read a 8 bit value to the specified
 *                   I/O address.
 * -------------------------------------------------------------------------*/
uchar
V7310_GETC( addr )
unsigned int	addr;
{
int	old_level;
unsigned long	bus_val;
unsigned char	value;

	/*
	 * Disable interupts while writing registers 
         */
	old_level = i_disable(INTMAX);
	bus_val = (unsigned long)iomem_att(&vwork->pci_io_map);
	*((volatile unsigned char *)(bus_val)) = (uchar)addr;
	value = *((volatile unsigned char *)(bus_val + VCA_DATA_PORT));
   	EIEIO; 
	iomem_det((void *)(bus_val));
	/*
	 * Enable interupts while writing registers 
         */
	i_enable(old_level);
	
	return(value);
}


/* -------------------------------------------------------------------------
 *   Function Name : vca_logerr()
 *   Description   : This routine setup the detail data for errlogging and calls
 *                  the errlogging routine 'errsave' to log errors.
 * -------------------------------------------------------------------------*/
int
vca_logerr( errid, res_name, dmodule, fmodule, return_code, err_indicator, 
			ras_unique )
unsigned int	errid;
char   *res_name;
char   *dmodule;
char   *fmodule;
int    return_code;
int    err_indicator;
char   *ras_unique;
{

        /* set up info and log */
        ER.error_id =  errid;
	sprintf(ER.resource_name,"%8s",res_name);

	sprintf(ER.detail_data,"%8s  %8s  %4d  %4d  %s",
        	dmodule,fmodule,return_code,err_indicator,ras_unique);

	/* Call system error logging routine */
	errsave(&ER, ERR_REC_SIZE + (strlen(ER.detail_data)+1));

	return(0);
}


/* ---------------------------------------------------------------
 *   Function Name : MotionResetHW()
 *   Description   : Reset hardware register
 *   Called by     : vca_config(), vca_close(), vca_ntsc_sw() 
 *                   and reset_overlay_call()
 * ----------------------------------------------------------------*/
MotionResetHW()
{
	unsigned char   fncreg, pwrreg, gotreg, gioreg;
	int	cnt, rc;

        VCADBG_3("MotionResetHW: vwork=%x lcd=%d crt=%d\n"
			,vwork,vwork->lcd_status, vwork->crt_status);

#ifdef	PANEL_PROTECTION
	/*
	 * Turn off LCD for panel protection
	 */
	if (vwork->lcd_status && vwork->xres == vwork->vddinfo.lcd_w) {
		rc = wfg_lcd_control(0,PM_PLANAR_OFF);
	}
#endif	/* PANEL_PROTECTION */

        for ( cnt = 0; cnt < sizeof(V7310VGAParm);)
                V7310_PUTC( V7310VGAParm[cnt++], V7310VGAParm[cnt++] );
        V7310_PUTC( VCA_GOTDAT, GOTDAT_WD_DAC ); /* RCAout-OFF DAC-WD SYNC-WD */
        V7310_PUTC( VCA_GIOCTL, GIOCTL_OUTPUT );   /* port 0-4 output enable */

	/*
 	 * Set power control register power OFF
 	 */
	/* 90h : pathru-mode and keep the LCD status */
        if ((!vwork->lcd_status ) || (vwork->xres != vwork->vddinfo.lcd_w))
                V7310_PUTC( VCA_FNCSEL, PATHRU_MODE );  /* LCD OFF */
        else
                V7310_PUTC( VCA_FNCSEL, LCD_PATHRU_MODE );  /* LCD ON */
	/* 93h:VRAM/CLUT/NTSC/BMT/DAC power-down mode and keep the CRT status */
         V7310_PUTC( VCA_PWRCTL, PWRCTL_POWER_OFF  );  	/* CRT OFF */
	/* 9Fh : VRAM/NTSC shutdown */
        V7310_PUTC( VCA_PWRCT2, PWRCT2_POWER_OFF  );
	/* B4h : Bt812/Frame memory power off */
        gotreg = V7310_GETC( VCA_GOTDAT );
        V7310_PUTC( VCA_GOTDAT, gotreg|GOTDAT_BT_FRAME  );
	/* B0h : FIELDE, FIELD0 input and Bt812/Frame memory disable */
        gioreg = V7310_GETC( VCA_GIOCTL );
        V7310_PUTC( VCA_GIOCTL, (gioreg|GIOCTL_INPUT)&~GOTDAT_BT_FRAME  );

#ifdef	PANEL_PROTECTION
	/*
	 * Turn on LCD, if needed.
	 */
	if (vwork->lcd_status && vwork->xres == vwork->vddinfo.lcd_w) {
		rc = wfg_lcd_control(0,PM_PLANAR_ON);
	}
#endif	/* PANEL_PROTECTION */

        return( 0 );
}


/* ---------------------------------------------------------------
 *   Function Name : reset_overlay_call()
 *   Operation     : Set default flag
 *                 : Set power on LCD/CRT
 *                 : Init V7310 hardware register
 *   Called by     : Graphic D.D 
 * ----------------------------------------------------------------*/
reset_overlay_call()
{
	unsigned char	reg;

        VCADBG_1("reset_overlay_call(): vwork=%x\n",vwork);

	vwork->lcd_status = VCA_LCD_ON;
	vwork->crt_status = VCA_CRT_ON;
        vwork->xres = vwork->vddinfo.lcd_w;
        vwork->yres = vwork->vddinfo.lcd_h;
        vwork->offset_x = vwork->vddinfo.offset_x;
        vwork->offset_y = vwork->vddinfo.offset_y;
        vwork->vram_offset = vwork->vddinfo.vram_offset;
        vwork->vram_clk = vwork->vddinfo.vram_clk;
        vwork->v_total = vwork->vddinfo.v_total;
        vwork->v_sync = vwork->vddinfo.v_sync;
        vwork->line_width = 0;
	vwork->pan_x = 0;
	vwork->pan_y = 0;
        reg = V7310_GETC(VCA_FNCSEL);
        V7310_PUTC( VCA_FNCSEL, reg | LCD_PATHRU_MODE ); 	/* LCD ON */
        if ( vwork->ntsc_in_use )
                V7310_PUTC( VCA_AQLINS, ACQ_START_LOW );

        return( 0 );

}


/* ---------------------------------------------------------------
 *   Function Name : power_ctl_call()
 *   Operation     : Set LCD/VRT power on/off
 *   Called by     : Graphic D.D 
 * ----------------------------------------------------------------*/
#ifdef	PANEL_PROTECTION
power_ctl_call(mode)
	int     mode;
{
	int     rc;
	unsigned char   reg;

        VCADBG_3("power_ctl_call(): mode=%d lcd=%d crt=%d\n", 
			mode,vwork->lcd_status,vwork->crt_status);

	switch (mode) {
	case CRT_PMG_ON:
		/*
		 * Power on the V7310 DAC, if the current selected DAC 
		 * if V7310.
		 */
		if ((vwork->crt_status)&&((vwork->ntsc_in_use)||(vwork->ext_appl))) {
                	reg = V7310_GETC(VCA_PWRCTL);
                	V7310_PUTC(VCA_PWRCTL, reg & ~SET_BIT5 );
		}
		break;
	case CRT_PMG_OFF:
		reg = V7310_GETC(VCA_PWRCTL);
		V7310_PUTC( VCA_PWRCTL, reg | SET_BIT5 );
		break;
	case LCD_PMG_ON:
		/*
		 * Power on the V7310's LCD output signal, if the resolution
		 * is matched to physical LCD resolution.
		 */
	        if ((vwork->lcd_status)&&(vwork->vddinfo.lcd_w==vwork->xres)) {
                	reg = V7310_GETC(VCA_FNCSEL);
                	V7310_PUTC(VCA_FNCSEL, reg | LCD_POWER_ON );
		}
		break;
	case LCD_PMG_OFF:
                reg = V7310_GETC(VCA_FNCSEL);
                V7310_PUTC(VCA_FNCSEL, reg & ~LCD_POWER_ON );
		break;
	default:
                return( EINVAL );
		break;
	}

        return ( 0 );

}
#else	/* PANEL_PROTECTION */
power_ctl_call(mode)
int     mode;
{
int     rc;
unsigned char   reg;

        VCADBG_3("power_ctl_call(): mode=%d lcd=%d crt=%d\n", 
			mode,vwork->lcd_status,vwork->crt_status);
        if ( mode == PMG_ON ) {
		if ((vwork->crt_status)&&((vwork->ntsc_in_use)||(vwork->ext_appl)))
		{
                	reg = V7310_GETC(VCA_PWRCTL);
                	V7310_PUTC(VCA_PWRCTL, reg & ~SET_BIT5 );
		}
	        if ((vwork->lcd_status) && ( vwork->vddinfo.lcd_w == vwork->xres )) {
                	reg = V7310_GETC(VCA_FNCSEL);
                	V7310_PUTC(VCA_FNCSEL, reg | LCD_POWER_ON );
		}
        } else if ( mode == PMG_OFF ) {
                reg = V7310_GETC(VCA_PWRCTL);
                V7310_PUTC( VCA_PWRCTL, reg | SET_BIT5 );
                reg = V7310_GETC(VCA_FNCSEL);
                V7310_PUTC(VCA_FNCSEL, reg & ~LCD_POWER_ON );
        } else {
                return( EINVAL );
        }

        return ( 0 );

}
#endif	/* PANEL_PROTECTION */

/* ---------------------------------------------------------------
 *   Function Name : vca_setup_vptr()
 *   Operation     : Allocate to working area
 *   Called by     : vca_config()
 * ----------------------------------------------------------------*/
long
vca_setup_vptr( )
{
        vwork = (struct vca_area *) xmalloc (sizeof (struct vca_area),
                                               3, pinned_heap);
        if (vwork == NULL) {
               unlockl(&(vca_lock));
               DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, ENOMEM,
                                                0, 0, 0, 0, 0);
               return ( ENOMEM );
        }
        bzero (vwork, sizeof (struct vca_area));
        VCADBG_1("vca_setup_vptr() vwork=%x:\n" ,vwork);
	return ( (ulong)vwork );
}



/* ---------------------------------------------------------------
 *   Function Name : vram_copy()
 *   Operation     : Setup the bus master transfer
 *		   : Copy to/from user buffer
 *		   : transfer each 4k bytes
 *   Called by     : vca_read(), vca_write(), vca_capture()
 * ----------------------------------------------------------------*/
int
vram_copy( direction, start_addr, trans_size )
int	direction;
unsigned long start_addr, trans_size;
{
struct	uio	*uiop;
struct	iovec	*iov;
unsigned char	reg, scan_st0, scan_st1, scan_st2, fncreg, pwrreg;
register unsigned long 	addr, addr1, scat_size, line_size, tmp_size;
int	rc, cnt, sleep_rc, old_level;

	VCADBG_2("vram_copy():start_addr=%x trans_size=%x\n",start_addr,trans_size);

	/* set transfer direction , YC format and interrupt disable */
	switch(direction) {
		case VRAM_READ:	/* vram -> system memory */
			V7310_PUTC( VCA_CVHSCL, 
				(uchar)(vwork->ovl_end & 0x00ff));
			V7310_PUTC( VCA_CVHSCH, 
				(uchar)((vwork->ovl_end >> 8 ) & 0x00ff ));
			vwork->trans_flag = FALSE;
			reg = BMT_READ_BITS;
			break;
		case VRAM_WRITE:	/* system memory -> vram */
			reg = BMT_WRITE_BITS;
			break;
		default:
			return( EINVAL );
	}


	/* move to/from user buffer */
	iov = (struct iovec *)malloc(sizeof(struct iovec));
	uiop = (struct uio *)malloc(sizeof(struct uio));
	iov->iov_base = vwork->user_buff;
	iov->iov_len = trans_size;
        uiop->uio_iov = iov;
        uiop->uio_xmem = NULL;
        uiop->uio_iovcnt = 1;
        uiop->uio_iovdcnt = 0;
        uiop->uio_offset = 0;
        uiop->uio_resid = iov->iov_len;
        uiop->uio_segflg = UIO_SYSSPACE;
        uiop->uio_fmode = O_RDWR;

	/*  bus master transfer */

	for ( addr1 = 0,cnt = 0; trans_size > 0; 
	 	addr1 += PAGESIZE, cnt++, trans_size -= PAGESIZE ) {
		if ( cnt == VCA_PAGE_NUM )
			cnt = 0;
#ifdef	PM_SUPPORT
		vwork->dma_activity = 1;
	 	pm_vca_ptr->activity = PM_ACTIVITY_OCCURRED;
#endif	/* PM_SUPPORT */
	   	if ( trans_size > PAGESIZE ) 
			scat_size = PAGESIZE;
           	else
                	scat_size = trans_size;

		if ( direction == VRAM_WRITE ) {
			rc = uiomove( vwork->dma_vaddr[cnt], scat_size, 
					UIO_WRITE, uiop );
			if ( rc != 0 ) {
				DDHKWD5 (HKWD_VCA_DD, DD_EXIT_IOCTL, 
					EINVAL, 0, 0, 0, 0, 0);
				return ( EINVAL );
			}
		} 

                tmp_size = scat_size;
          for ( addr = 0; tmp_size > 0;
                        addr += MAXPATHLEN, tmp_size -= MAXPATHLEN ) {

                if ( tmp_size > MAXPATHLEN )
                        line_size = MAXPATHLEN;
                else
                        line_size = tmp_size;

		/* set transfer size */
		V7310_PUTC( VCA_BMTSIZ, 
				(uchar)(line_size & 0x000000ff));
		V7310_PUTC( VCA_BMTSIZ+1, 
				(uchar)((line_size >> 8) & 0x000000ff));
		V7310_PUTC( VCA_BMTSIZ+2, 
				(uchar)((line_size >> 16) & 0x000000ff));

		/* set vram start address */
		V7310_PUTC( VCA_BMTVPA, 
			(uchar)((start_addr+addr+addr1) & 0x000000ff));
		V7310_PUTC( VCA_BMTVPA+1, 
			(uchar)(((start_addr+addr+addr1) >> 8) & 0x000000ff));
		V7310_PUTC( VCA_BMTVPA+2, 
			(uchar)(((start_addr+addr+addr1) >> 16) & 0x000000ff));

		/* set transfer system memory address */
		V7310_PUTC( VCA_BMTMMA, 
		(uchar)((vwork->dma_paddr[cnt]+addr | 0x80000000) & 0x000000ff));
		V7310_PUTC( VCA_BMTMMA+1, 
		(uchar)(((vwork->dma_paddr[cnt]+addr | 0x80000000)>> 8) & 0x000000ff));
		V7310_PUTC( VCA_BMTMMA+2, 
		(uchar)(((vwork->dma_paddr[cnt]+addr | 0x80000000)>> 16) & 0x000000ff));
		V7310_PUTC( VCA_BMTMMA+3, 
		(uchar)(((vwork->dma_paddr[cnt]+addr | 0x80000000)>> 24) & 0x000000ff));

                if (( direction == VRAM_READ ) && (!( vwork->trans_flag ))) {
                        old_level = i_disable(vwork->int_priority);
                        /* read interrupt status register */
	    		V7310_PUTC( VCA_BMTCTL, 
				(V7310_GETC(VCA_BMTCTL)) & ~HSCAN_INTR_BITS);
						/* BMT intr disable */
                        scan_st0 = V7310_GETC( VCA_XINTEN );
                        scan_st1 = V7310_GETC( VCA_INTSTA );
                        scan_st2 = V7310_GETC( VCA_ISTAWE );
                        V7310_PUTC( VCA_XINTEN, scan_st0&~HSCAN_INTR_BITS );
                        V7310_PUTC( VCA_ISTAWE, scan_st2|HSCAN_INTR_BITS );
                        V7310_PUTC( VCA_INTSTA, scan_st1&~HSCAN_INTR_BITS );
                        V7310_PUTC( VCA_ISTAWE, scan_st2&~HSCAN_INTR_BITS );
			V7310_PUTC( VCA_XINTEN, scan_st0|HSCAN_INTR_BITS );
        		sleep_rc = e_sleep((&vwork->hscan_event), EVENT_SIGRET);
                        i_enable(old_level);
			if ( sleep_rc == EVENT_SIG ) {
				VCADBG_0("vram_copy():EVENT_SIG...\n");
				DDHKWD5 (HKWD_VCA_DD, DD_EXIT_IOCTL, 
					EINTR, 0, 0, 0, 0, 0);
				return ( EINTR );
			}
                        if ( vwork->ovl_end < (vwork->yres+vwork->offset_y)) {
                                /* enable HSCAN interrupt */
                                old_level = i_disable(vwork->int_priority);
                                V7310_PUTC(VCA_XINTEN, scan_st0|HSCAN_INTR_BITS );
                                i_enable(old_level);
                        }
		}

		/* bus master transfer start ! */
                old_level = i_disable(vwork->int_priority);
		V7310_PUTC( VCA_BMTCTL, reg | BMT_INTR_ENABLE);
		V7310_PUTC( VCA_BMTCTL, reg | BMT_START);
        	sleep_rc = e_sleep ((&vwork->bmt_event), EVENT_SIGRET);
                i_enable(old_level);
		if ( sleep_rc == EVENT_SIG ) {
			VCADBG_0("vram_copy():EVENT_SIG...\n");
			DDHKWD5 (HKWD_VCA_DD, DD_EXIT_IOCTL, 
					EINTR, 0, 0, 0, 0, 0);
			return ( EINTR );
		}

                if ( line_size < MAXPATHLEN )
                        break;
          } /* end of for */
		if ( direction == VRAM_READ ) {
			rc = uiomove( vwork->dma_vaddr[cnt], scat_size, 
					UIO_READ, uiop );
			if ( rc != 0 ) {
				DDHKWD5 (HKWD_VCA_DD, DD_EXIT_IOCTL, 
					EINVAL, 0, 0, 0, 0, 0);
				return ( EINVAL );
			}
		}
		if ( scat_size < PAGESIZE )
			break;
	} /* end of for */

	free(iov);
	free(uiop);

	return (0 );
}


/* ---------------------------------------------
 *   Function Name : RegBt812Get()
 *   Description   : Read Bt812 register data
 * ----------------------------------------------*/
uchar
RegBt812Get(index)
unsigned char	index;
{

unsigned char	data;
unsigned char	portdat, portctl;

   	/*                                          
    	 * Enable V7310-to-Bt812 interface         
    	 */                                        
   	portctl = V7310_GETC(VCA_GIOCTL);
   	portdat = V7310_GETC(VCA_GOTDAT);

   	V7310_PUTC(VCA_PXGOSL, RD_WR_BITS); 
						/* disable VRAM interface */
   	V7310_PUTC(VCA_GOTDAT, (uchar)(portdat | RD_WR_BITS)); 
						/* inactive RD#,WR# */
   	/*
    	 * Write the address register 
    	 */
   	V7310_PUTC(VCA_GINBT1, index);
   	V7310_PUTC(VCA_GINBT2, RESET_BITS);      /* RS1 = 0, RS0 = 0 */
   	V7310_PUTC(VCA_GIOCTL, (uchar)(portctl | RD_WR_BITS));  
						/* enable RD#,WR# outputs */
   	V7310_PUTC(VCA_GOTBT1, SET_BITS); /*  enable D0-D7 outputs */
   	V7310_PUTC(VCA_GOTDAT, (uchar)((portdat|SET_BIT0) & WR_STR_BITS ));
						/* strobe WR# */
   	V7310_PUTC(VCA_GOTDAT, (uchar)(portdat|RD_WR_BITS));

   	/*
    	 * Read the control register data  
    	 */ 
   	V7310_PUTC(VCA_GOTBT1, RESET_BITS);
   	V7310_PUTC(VCA_GINBT2, SET_BIT1); 
   	V7310_PUTC(VCA_GOTDAT, (uchar)((portdat|SET_BIT1) & RD_STR_BITS));
						/* strobe RD# */
   	data = V7310_GETC(VCA_PXGBT1);		 
   	V7310_PUTC(VCA_GOTDAT, (uchar)(portdat | RD_WR_BITS));/* strobe RD# */

   	/* 
    	 * Disable V7310-to-Bt812 interface     
    	 */  
   	V7310_PUTC(VCA_GIOCTL, portctl);       
   	V7310_PUTC(VCA_GOTDAT, portdat);
   	V7310_PUTC(VCA_PXGOSL, RESET_BITS); /* enable VRAM interface */

   	return( data );
}


/* ---------------------------------------------
 *   Function Name : RegBt812Set()
 *   Description   : Write Bt812 register data
 * ----------------------------------------------*/
RegBt812Set(index, data)
unsigned char	index;
unsigned char	data;
{

unsigned char	portdat, portctl;

   	/*                                                
    	 * Enable V7310-to-Bt812 interface  
    	 */ 
   	portctl = V7310_GETC(VCA_GIOCTL);
   	portdat = V7310_GETC(VCA_GOTDAT);
   	V7310_PUTC(VCA_PXGOSL, RD_WR_BITS);/* disable VRAM interface */
   	V7310_PUTC(VCA_GOTDAT, (uchar)portdat | RD_WR_BITS); 
							/* inactive RD#,WR# */
   	/*                                               
    	 * Write the address register   
    	 */ 
   	V7310_PUTC(VCA_GINBT1, index);
   	V7310_PUTC(VCA_GINBT2, RESET_BITS);   /* RS1 = 0, RS0 = 0 */
   	V7310_PUTC(VCA_GIOCTL, (uchar)portctl | RD_WR_BITS);
						  /* enable RD#,WR# outputs */
   	V7310_PUTC(VCA_GOTBT1, SET_BITS);	/* enable D0-D7 outputs */
   	V7310_PUTC(VCA_GOTDAT, (portdat | SET_BIT0) & WR_STR_BITS); 
   	V7310_PUTC(VCA_GOTDAT, portdat | RD_WR_BITS); 

   	/*                                                           
    	 * Write the control register data  
    	 */                                
   	V7310_PUTC(VCA_GINBT1, data);
   	V7310_PUTC(VCA_GINBT2, SET_BIT1);   /* RS1 = 1, RS0 = 0 */
   	V7310_PUTC(VCA_GOTDAT, portdat & WR_STR_BITS);
   	V7310_PUTC(VCA_GOTDAT, portdat | RD_WR_BITS);/* strobe WR# */

   	/*                                                           
    	 * Disable V7310-to-Bt812 interface                         
         */                                                         
   	V7310_PUTC(VCA_GIOCTL, portctl);
   	V7310_PUTC(VCA_GOTDAT, portdat);
   	V7310_PUTC(VCA_PXGOSL, RESET_BITS); 	/* enable VRAM interface */

   	return (0);
}

/* ---------------------------------------------------------------
 *   Function Name : vca_enable()
 *   Operation     : Set I/O register to PCI configuration space
 *                 : Initialize bus attach structure for PCI I/O space
 *   Called by     : vca_config()
 * ----------------------------------------------------------------*/
int
vca_enable( dds )
struct  vca_dds *dds;
{
volatile PCI_cfg this_slot;
int     rc;


        VCADBG_0("vca_enable routine\n");

        rc = CfgSpaceRW ( dds->bus_number, dds->slot_number, VENDOR_ID_REG,
                &this_slot.VendorID, PCI_SHORT_AT, CFG_SPACE_READ );
        rc |= CfgSpaceRW ( dds->bus_number, dds->slot_number, COMMAND_REG,
                &this_slot.command, PCI_SHORT_AT, CFG_SPACE_READ );
        rc |= CfgSpaceRW ( dds->bus_number, dds->slot_number, REV_ID_REG,
                &this_slot.revID, PCI_SHORT_AT, CFG_SPACE_READ );

        if ( rc != 0 ) {
                return ( -1 );
        }

        VCADBG_1("vca_enable():Vendor=%x\n", this_slot.VendorID);
        if ( this_slot.VendorID != 0xED10 ) {
                return ( -1 );
        }

        this_slot.BaseAddressRegs[0] = BYTE_REV( dds->bus_io_addr);
        rc = CfgSpaceRW ( dds->bus_number, dds->slot_number, BASE_ADDRESS_REG,
                &this_slot.BaseAddressRegs[0], PCI_WORD_AT, CFG_SPACE_WRITE );
        if ( rc != 0 ) {
                return ( -1 );
        }

        rc = CfgSpaceRW ( dds->bus_number, dds->slot_number, BASE_ADDRESS_REG,
                &this_slot.BaseAddressRegs[0], PCI_WORD_AT, CFG_SPACE_READ );
        VCADBG_1("vca_enable():BaseAddress=%x\n", this_slot.BaseAddressRegs[0]);
        if ( rc != 0 ) {
                return ( -1 );
        }

        /* initialize bus attach structure for PCI I/O space */
        vwork->pci_io_map.key   = IO_MEM_MAP;
        vwork->pci_io_map.flags = 0;          /* read/write */
        vwork->pci_io_map.size  = V7310_IOMAP_SIZE;    /* get 256byte */
        vwork->pci_io_map.bid   = dds->bus_number;
        vwork->pci_io_map.busaddr = (long long)(ulong)dds->bus_io_addr;

        VCADBG_5(" vca_enable():key=%x flag=%x size=%x bid=%x busaddr=%x\n",
        vwork->pci_io_map.key,vwork->pci_io_map.flags,vwork->pci_io_map.size,
                vwork->pci_io_map.bid,vwork->pci_io_map.busaddr);

        /* Set bits in PCI command register to enable access */
        this_slot.command = CFG_CMD_ENABLE;    /* (I/O and BMT enable) */
        rc |= CfgSpaceRW ( dds->bus_number, dds->slot_number, COMMAND_REG,
                &this_slot.command, PCI_SHORT_AT, CFG_SPACE_WRITE );
        /* Set bits in PCI latency timer register to enable access */
        this_slot.latency |= SET_BITS;      /* ( for Bus Master Transfer ) */
        rc |= CfgSpaceRW ( dds->bus_number, dds->slot_number, LATENCY_REG,
                &this_slot.latency, PCI_BYTE_AT, CFG_SPACE_WRITE );
        if ( rc != 0) {
                return ( -1 );
        }

        VCADBG_0(" end of pci config \n");
        return ( 0 );

        /* set up PCI configuration space */
}


/* ---------------------------------------------------------------
 *   Function Name : CfgSpaceRW()
 *   Description   : PCI configuration read/write
 *   Called by     : vca_enable()
 * ----------------------------------------------------------------*/
int
CfgSpaceRW( bus_num, slot, addr, p_buf, size, rw )
unsigned long   bus_num;
unsigned long   slot;
unsigned long   addr;
unsigned char   *p_buf;
int     size;
int     rw;
{
int     rc;
struct  mdio    mdd;

        mdd.md_addr = addr;
        mdd.md_data = (char *)p_buf;
        mdd.md_sla  = slot;

        switch ( size ) {
                case  4:
                        mdd.md_incr = MV_WORD;
                        mdd.md_size = size/4;
                        break;
                case  2:
                        mdd.md_incr = MV_SHORT;
                        mdd.md_size = size/2;
                        break;
                case  1:
                default:
                        mdd.md_incr = MV_BYTE;
                        mdd.md_size = size;
                        break;
        }

        rc = pci_cfgrw( bus_num, &mdd, rw );
        if ( rc != 0 )
                return ( -1 );
        else
                return ( 0 );
}


/* --------------------------------------------------------------------
 *   Function Name : _Acq_Waiting()
 *   Description   : Wait for completion of acquisition
 *   Called by     : vca_capture(), vca_adcon_ctrl(), vca_acquisition(),
 *                 : vca_vram_write() and vca_input_sw()
 * ---------------------------------------------------------------------*/
_Acq_Waiting( acq_mode )
unsigned char   acq_mode;
{
unsigned char   int_st0, int_st1, int_st2;
int     sleep_rc, rc;
int     old_level;

        VCADBG_1("_Acq_Waiting(): acq_mode=%x\n",acq_mode);
        old_level = i_disable(vwork->int_priority);
        /* read interrupt status register */
        int_st0 = V7310_GETC( VCA_XINTEN );
        int_st1 = V7310_GETC( VCA_INTSTA );
        int_st2 = V7310_GETC( VCA_ISTAWE );
        V7310_PUTC( VCA_XINTEN, int_st0&~SET_BIT3 );
        V7310_PUTC( VCA_ISTAWE, int_st2|SET_BIT3 );
        V7310_PUTC( VCA_INTSTA, int_st1&~SET_BIT3 );
        V7310_PUTC( VCA_ISTAWE, int_st2&~SET_BIT3 );
        V7310_PUTC( VCA_XINTEN, int_st0|SET_BIT3 );
                                                /* intr enable */
        V7310_PUTC( VCA_ACQMOD, acq_mode | STILL_BITS );
                                                        /* freeze */
        sleep_rc = e_sleep ((&vwork->acq_event), EVENT_SIGRET);
        i_enable(old_level);

        if ( sleep_rc == EVENT_SIG ) {
                VCADBG_0("_Acq_Waiting():EVENT_SIG...\n");
                DDHKWD5 (HKWD_VCA_DD, DD_EXIT_IOCTL,
                                EINTR, 0, 0, 0, 0, 0);
                return ( EINTR );
        }

        return ( 0 );
}


#ifdef	PM_SUPPORT
/* ---------------------------------------------------------------
 *   Function Name : pm_setup_vca()
 *   Description   : Register PM HANDLER
 *   Called by     : vca_config()
 * ----------------------------------------------------------------*/
long
pm_setup_vca( dds, devno )
struct	vca_dds	*dds;
dev_t	devno;
{
struct	pm_handle *vca_pm;
int	rc;

	vca_pm = (struct pm_handle *) xmalloc (sizeof (struct pm_handle),
						3, pinned_heap);

	if (vca_pm == NULL) {
                vca_logerr(ERRID_VCA_INITZ, "PCIVCA", "vcadd_pin",
                	"pm_setup_vca", ENOMEM, 1, "1");
                return ( ENOMEM );

	}

        /* set up power-on operation */
        vca_pm->activity    		= PM_NO_ACTIVITY;
        vca_pm->handler 		= vca_pmhandler;
        vca_pm->private 		= (caddr_t)dds;
        vca_pm->device_idle_time 	= dds->device_idle_time;
        vca_pm->device_standby_time 	= dds->device_stanby;
        vca_pm->devno 			= devno;
        vca_pm->attribute 		= 0;
        vca_pm->mode 			= PM_DEVICE_FULL_ON;
	vca_pm->pm_version		= 0x100 ;
	vca_pm->device_logical_name	= dds->logical_name;

	VCADBG_3("pm_setup_vca(): idle_time=%d stanby=%d ccd_id=%x\n"
		,dds->device_idle_time, dds->device_stanby, dds->pm_ccd_id);
	rc = pm_register_handle( vca_pm, PM_REGISTER );
	if ( rc != PM_SUCCESS ) {
                vca_logerr(ERRID_VCA_INITZ, "PCIVCA", "vcadd_pin",
                	"pm_setup_vca", ENOMEM, 2, "2");

		xmfree( (caddr_t)vca_pm, pinned_heap );
	}

	return( (ulong) vca_pm );
}

/* ---------------------------------------------------------------
 *   Function Name : vca_pmhandler()
 *   Description   : Power management
 *   Called by     : PM CORE
 * ----------------------------------------------------------------*/
int
vca_pmhandler( private, ctrl )
caddr_t private;
int     ctrl;
{
struct	vca_dds		*dds;

	VCADBG_1("vca_pmhandler(): ctrl=%x\n" ,ctrl);
	dds = (struct vca_dds *)private;
        switch( ctrl ) {

        case    PM_DEVICE_IDLE:
	   switch( pm_vca_ptr->mode ) {
	        case PM_DEVICE_ENABLE:
		case PM_DEVICE_IDLE:
			if (!( vwork->pmout_status)) 
                		pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_OFF);
                	break;
		case PM_DEVICE_SUSPEND:
		case PM_DEVICE_HIBERNATION:
			if ( vwork->pmout_status ) {
                                pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_ON);
                        } else {
                                pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_OFF);
                        }
			Restore_current_reg( dds );
			if ( vwork->request_is_blocked == TRUE ) {
				vwork->request_is_blocked = FALSE;
				e_wakeup(&vwork->pm_event);
			}
#ifdef	never
			/*
			 * The following code prevent unexpected system hang which
			 * caused by waiting DMA end interrupt.
			 */
			if (vwork->bmt_event != EVENT_NULL) {	/* check if waiting bmt_event */
				e_wakeup(&vwork->bmt_event);
			}
			if (vwork->acq_event != EVENT_NULL) {	/* check if waiting acq_event */
				e_wakeup(&vwork->acq_event);
			}
			if (vwork->hscan_event != EVENT_NULL) {	/* check if waiting hscan_event */
				e_wakeup(&vwork->hscan_event);
			}
#endif	/* never */
			break;
                case PM_DEVICE_FULL_ON:
                        break;
                default:
                        return(PM_ERROR);
	   }
           pm_vca_ptr->mode = ctrl;
           break;
        case    PM_DEVICE_ENABLE:
	   switch( pm_vca_ptr->mode ) {
		case PM_DEVICE_IDLE:
			if ( vwork->pmout_status ) {
                		pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_ON);
			}
                	break;
		case PM_DEVICE_SUSPEND:
                case PM_DEVICE_HIBERNATION:
			if ( vwork->pmout_status ) {
                		pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_ON);
                        } else {
                                pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_OFF);
			}
			Restore_current_reg( dds );
			if ( vwork->request_is_blocked == TRUE ) {
				vwork->request_is_blocked = FALSE;
				e_wakeup(&vwork->pm_event);
			}
#ifdef	never
			/*
			 * The following code prevent unexpected system hang which
			 * caused by waiting DMA end interrupt.
			 */
			if (vwork->bmt_event != EVENT_NULL) {	/* check if waiting bmt_event */
				e_wakeup(&vwork->bmt_event);
			}
			if (vwork->acq_event != EVENT_NULL) {	/* check if waiting acq_event */
				e_wakeup(&vwork->acq_event);
			}
			if (vwork->hscan_event != EVENT_NULL) {	/* check if waiting hscan_event */
				e_wakeup(&vwork->hscan_event);
			}
#endif	/* never */
			break;
		case PM_DEVICE_FULL_ON:
			if (!( vwork->pmout_status ))
                		pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_OFF);
                	break;
	        case PM_DEVICE_ENABLE:
                	break;
                default:
			return(PM_ERROR);
	   }
           pm_vca_ptr->mode = ctrl;
           break;
	case	PM_DEVICE_SUSPEND:
	case	PM_DEVICE_HIBERNATION:
	   switch( pm_vca_ptr->mode ) {
		case PM_DEVICE_ENABLE:
		case PM_DEVICE_IDLE:
			if ( vwork->dma_activity )
				return(PM_ERROR);
                	pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_OFF);
			Save_current_reg();
                        break;
                case PM_DEVICE_SUSPEND:
                        break;
                default:
                        break;
            }
           pm_vca_ptr->mode = ctrl;
           break;
        case    PM_DEVICE_FULL_ON:
#ifdef	PANEL_PROTECTION
           pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_ON);
#else	/* PANEL_PROTECTION */
           switch( pm_vca_ptr->mode ) {
                case PM_DEVICE_ENABLE:
			if ( vwork->pmout_status ) {
                        	pm_planar_control(pm_vca_ptr->devno, dds->pm_ccd_id, PM_PLANAR_ON);
			}
                        break;
                case PM_DEVICE_IDLE:
                        break;
                default:
                        break;
           }
#endif	/* PANEL_PROTECTION */
           pm_vca_ptr->mode = ctrl;
           break;
	case PM_PAGE_FREEZE_NOTICE:
		if (!VRegData) {
			VRegData = (unsigned char *)xmalloc(sizeof(VSaveReg), 3,
							    pinned_heap);
		}
		if (!VRegData) {
			return (PM_ERROR);
		}

		if (!BRegData) {
			BRegData = (unsigned char *)xmalloc(BT_IOMAP_SIZE, 3, 
							    pinned_heap);
		}
		if (!BRegData) {
			xmfree((caddr_t)VRegData, pinned_heap);
			VRegData = (unsigned char *)NULL;
			return (PM_ERROR);
		}
		break;
	case PM_PAGE_UNFREEZE_NOTICE:
		if (VRegData) {
			xmfree((caddr_t)VRegData, pinned_heap);
			VRegData = (unsigned char *)NULL;
		}
		if (BRegData) {
			xmfree((caddr_t)BRegData, pinned_heap);
			BRegData = (unsigned char *)NULL;
		}
		break;
        default:
		break;
        }

        return( PM_SUCCESS );
}


/* ---------------------------------------------------------------
 *   Function Name : Existing_VCA_Request_Entry
 *   Description   : Power management
 *   Called by     : vca_ioctl()
 * ----------------------------------------------------------------*/
int
Existing_VCA_Request_Entry( request)
int	request;
{

	VCADBG_3("Existing_VCA_Request_Entry(): request=%d mode=%x ccd_re=%d\n" 
			,request, pm_vca_ptr->mode, vwork->request_of_ccd);

	switch( pm_vca_ptr->mode ) {
	case PM_DEVICE_FULL_ON:
	case PM_DEVICE_ENABLE:
		switch( request) {
		case  VCA_IOC_CAMERA_SW:
		case  VCA_IOC_INPUT_SW:
		case  VCA_IOC_INITIALIZE:
		case  VCA_IOC_ACQUISITION:
		case  VCA_IOC_AD_CTRL:
		case  VCA_IOC_OVERLAY:
	 		pm_vca_ptr->activity = PM_ACTIVITY_NOT_SET;
			if ((vwork->request_of_ccd)&&(!(vwork->pmout_status))) {
                		pm_planar_control(pm_vca_ptr->devno, vwork->ccd_id, PM_PLANAR_ON);
				vwork->pmout_status = TRUE;
			} else if ((!(vwork->request_of_ccd))&&(vwork->pmout_status)) {
                		pm_planar_control(pm_vca_ptr->devno, vwork->ccd_id, PM_PLANAR_OFF);
				vwork->pmout_status = FALSE;
			}
			break;
		case  VCA_IOC_NTSC_SW:
			if (!( vwork->ntsc_in_use )) {
                		pm_planar_control(pm_vca_ptr->devno, vwork->ccd_id, PM_PLANAR_ON);
				vwork->pmout_status = TRUE;
	 			pm_vca_ptr->activity = PM_ACTIVITY_NOT_SET;
			}
			break;
		}
		break;
	case PM_DEVICE_IDLE:
		switch( request) {
		case  VCA_IOC_CAMERA_SW:
		case  VCA_IOC_INPUT_SW:
		case  VCA_IOC_INITIALIZE:
		case  VCA_IOC_ACQUISITION:
		case  VCA_IOC_OVERLAY:
		case  VCA_IOC_AD_CTRL:
	 		pm_vca_ptr->activity = PM_ACTIVITY_NOT_SET;
                	pm_vca_ptr->mode = PM_DEVICE_ENABLE;
			if ((vwork->request_of_ccd)&&(!(vwork->pmout_status))) {
                		pm_planar_control(pm_vca_ptr->devno, vwork->ccd_id, PM_PLANAR_ON);
				vwork->pmout_status = TRUE;
			} else if ((!(vwork->request_of_ccd))&&(vwork->pmout_status)) {
                		pm_planar_control(pm_vca_ptr->devno, vwork->ccd_id, PM_PLANAR_OFF);
				vwork->pmout_status = FALSE;
			}
			break;
		case  VCA_IOC_NTSC_SW:
			if (!( vwork->ntsc_in_use )) {
                		pm_planar_control(pm_vca_ptr->devno, vwork->ccd_id, PM_PLANAR_ON);
				vwork->pmout_status = TRUE;
	 			pm_vca_ptr->activity = PM_ACTIVITY_NOT_SET;
			}
                	pm_vca_ptr->mode = PM_DEVICE_ENABLE;
			break;
		case  VCA_IOC_VRAM_READ:
		case  VCA_IOC_VRAM_WRITE:
		case  VCA_IOC_CAPTURE:
                	pm_vca_ptr->mode = PM_DEVICE_ENABLE;
			break;
		}
		break;
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
		vwork->request_is_blocked = TRUE;
		Save_current_reg();
		e_sleep((&vwork->pm_event), EVENT_SHORT);
		switch( request) {
		case  VCA_IOC_CAMERA_SW:
		case  VCA_IOC_INPUT_SW:
		case  VCA_IOC_INITIALIZE:
		case  VCA_IOC_ACQUISITION:
		case  VCA_IOC_OVERLAY:
		case  VCA_IOC_AD_CTRL:
	 		pm_vca_ptr->activity = PM_ACTIVITY_NOT_SET;
                	pm_vca_ptr->mode = PM_DEVICE_ENABLE;
			break;
		case  VCA_IOC_NTSC_SW:
			if (!( vwork->ntsc_in_use )) {
                		pm_planar_control(pm_vca_ptr->devno, vwork->ccd_id, PM_PLANAR_ON);
				vwork->pmout_status = TRUE;
	 			pm_vca_ptr->activity = PM_ACTIVITY_NOT_SET;
			}
                	pm_vca_ptr->mode = PM_DEVICE_ENABLE;
			break;
		case  VCA_IOC_VRAM_READ:
		case  VCA_IOC_VRAM_WRITE:
		case  VCA_IOC_CAPTURE:
                	pm_vca_ptr->mode = PM_DEVICE_ENABLE;
			break;
		}
		break;
	}
	return ( 0 );
}

int
Save_current_reg()
{
int	cnt;

	VCADBG_0("Save_current_reg():\n");

	if ( VRegData == NULL) {
                vca_logerr(ERRID_VCA_INITZ, "PCIVCA", "vcadd_pin",
                	"Save_current_reg", ENOMEM, 3, "3");
                return ( ENOMEM );
	}

	if ( BRegData == NULL) {
                vca_logerr(ERRID_VCA_INITZ, "PCIVCA", "vcadd_pin",
                	"Save_current_reg", ENOMEM, 4, "4");
                return ( ENOMEM );
	}

        for ( cnt = 0; cnt < sizeof(VSaveReg); cnt++ ) {
        	VRegData[cnt] = V7310_GETC(VSaveReg[cnt]);
        }
        for ( cnt = 0; cnt < BT_IOMAP_SIZE; cnt++ ) {
        	BRegData[cnt] = RegBt812Get( cnt );
        }

	return ( 0 );
}


int
Restore_current_reg( dds )
struct	vca_dds	*dds;
{
	int	cnt, rc;
	unsigned char	offset;

	VCADBG_0("Restore_current_reg():\n");

#ifdef	PANEL_PROTECTION
	/*
	 * Turn off LCD for panel protection.
	 */
	rc = wfg_lcd_control(0,PM_PLANAR_OFF);
#endif	/* PANEL_PROTECTION */

	vca_enable( dds );
	if (( vwork->ext_appl ) || (vwork->ntsc_in_use)) {
		V7310_PUTC( VCA_GIOCTL, GIOCTL_POWER_ON);	/* B0h */
		V7310_PUTC( VCA_GOTDAT, GOTDAT_POWER_ON);	/* B4h */
		delay(1);		/* Wait until Frame memory ON */
	}
        for ( cnt = 0; cnt < sizeof(VSaveReg); cnt++ ) {
		if ( VSaveReg[cnt] == VCA_FNCSEL ) {
                	V7310_PUTC( VSaveReg[cnt], VRegData[cnt]|LCD_POWER_ON );
			vwork->lcd_status = VCA_LCD_ON;
		} else if ( VSaveReg[cnt] == VCA_BMTCTL ) {
                	V7310_PUTC( VSaveReg[cnt], VRegData[cnt]&~SET_BIT0 );
		} else {
                	V7310_PUTC( VSaveReg[cnt], VRegData[cnt] );
		}
        }
	/* FFh : Software reset to Bt812/Frame memory */
        RegBt812Set( BT_SOFT_RESET , RESET_BITS );
        for ( cnt = 0; cnt < BT_IOMAP_SIZE; cnt++ ) {
        	RegBt812Set( cnt, BRegData[cnt] );
        }

#ifdef	PANEL_PROTECTION
	/*
	 * Turn on LCD
	 */
	rc = wfg_lcd_control(0,PM_PLANAR_ON);
#endif	/* PANEL_PROTECTION */

	return ( 0 );
}
#endif	/* PM_SUPPORT */
