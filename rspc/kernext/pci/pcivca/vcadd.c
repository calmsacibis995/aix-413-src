static char sccsid[] = "@(#)04  1.18 src/rspc/kernext/pci/pcivca/vcadd.c, pcivca, rspc41J, 9524A_all 6/10/95 12:31:02";
/* vcadd.c */
/*
 *   COMPONENT_NAME: (PCIVCA)  PCI Video Capture Adapter Device Driver
 *
 *   FUNCTIONS: vca_config, vca_open, vca_close, vca_ioctl
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
int	vca_open();
int	vca_close();
int	vca_config();
int	vca_ioctl();
int	vca_display_sw();
int	vca_ntsc_sw();
int	vca_camera_sw();
int	vca_input_sw();
int	vca_initialize();
int	vca_acquisition();
int	vca_overlay();
int	vca_capture();
int	vca_adcon_ctrl();
int	vca_vram_read();
int	vca_vram_write();
int	vca_xwin_pan();
int	vca_vram_clr();
int	vca_ccd_cfg();
int	vca_xpanel();
int	vca_register_overlay();
int	vca_set_inputsw();
int	vca_get_inputsw();
int	MotionSetupHW();
int	Enableintr();
int	_Select_BT_channel();
int	_Get_Residual_Data();
int	_Get_Channel_ID();
int	_Get_Video_Signal();

extern int	vca_intr();
extern int	vca_logerr();
extern int	V7310_PUTC();
extern uchar	V7310_GETC();
extern int	RegBt812Set();
extern uchar	RegBt812Get();
extern int	MotionResetHW();
extern int	vram_copy();
extern long	vca_setup_vptr();
extern int	reset_overlay_call();
extern int     	power_ctl_call();
extern int	vca_enable();
extern int	CfgSpaceRW();
extern int	_Acq_Waiting();
#ifdef	PM_SUPPORT
extern int	pm_setup_vca();
extern int	Existing_VCA_Request_Entry();
extern struct pm_handle *pm_vca_ptr;
#endif	/* PM_SUPPORT */



/*  exported by LFT D.D  */
extern int	get_VDD_info();
extern int	(*v7310_reset_overlay)();
extern int	(*v7310_power_ctl)();

#ifdef	PANEL_PROTECTION
extern int	wfg_lcd_control();
#endif	/* PANEL_PROTECTION */

/* ------------------
 * Define global data
 * ------------------*/
extern 	struct	vca_area *vwork;		/* working area */
lock_t 	vca_lock = LOCK_AVAIL;			/* lock word */

/*---------------------------------------------------------------------------
 * IDENTIFICATION:    vca_open()
 *
 * DESCRIPTIVE NAME:  Video Capture Adapter device open routine
 *
 * FUNCTIONS :        This routine is the standard device driver open routine
 *                    which initialzes and allocates system resourses
 *                    necessary for the device driver.
 *
 * INPUTS    :        device number
 *                    Flags for open type (ignored)
 *                    Channel number(ignored)
 *                    Extended parameter 
 *
 * OUTPUTS   :        NONE
 *
 ---------------------------------------------------------------------------*/

vca_open ( devno, devflag, chan, ext )
dev_t	devno;				/* Major and Minor device number */
ulong	devflag;
int	chan;
int	ext;
{
int	lock_ret;
int	minor_dev;
int	cnt, rc = 0;
unsigned char	fncreg, pwrreg, gioreg, gotreg;


	DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_OPEN, 0, devno, 0, 0, 0, 0);
	minor_dev = minor( devno );	/* extract the minor device number */

        /*    The following commands accessed by opening the device with the
         *    extension parameter.  
         *    User application  does not need extension parameter.
         *    1. Configuration for CCD camera  uses "VCA_CCD_CFG"
         *    2. X server use "VCA_XSERVER"
         *    3. NTSC command use "VCA_NTSC"
	 *		ie openx(name,O_RDWR, 0, VCA_CCD_CFG)
	 */

	VCADBG_2("vca_open(): vwork=%x ext=%d\n", vwork, ext);
	if ( minor_dev != 0 ) {
		DDHKWD5 (HKWD_VCA_DD, DD_EXIT_OPEN, ENXIO, devno, 0, 0, 0, 0);
		return( ENXIO );
	}

	if ( devflag & DAPPEND ) {
		DDHKWD5 (HKWD_VCA_DD, DD_EXIT_OPEN, EPERM, devno, 0, 0, 0, 0);
		return( EPERM );
	}

	switch( ext ) {
	   case VCA_CCD_CFG:	/* called by configure method */
		VCADBG_1("vca_open(): ext_ccd=%x\n", vwork->ext_ccd);
           	/*- single process use only -*/
		if (( vwork->ext_ccd == FALSE ) && 
			( vwork->device_in_use == FALSE )) {
			vwork->ext_ccd = getpid();
			VCADBG_1("vca_open(): ext_ccd=%d\n", vwork->ext_ccd);
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_EXIT_OPEN, EBUSY, 
					devno, 0, 0, 0, 0);
			return( EBUSY );	/* device busy */
		}
		break;
	   case VCA_IN_OUT:	/* This flag is debug use only */
           case VCA_XSERVER:	/* called by X-server for xpanel and xwin_pan */
        	/*-- This case do not need to check PID and not 
			have the checked flag. --*/
		break;
           case VCA_NTSC:	/* called by ntsc command */
	  	if ( vwork->ext_appl == FALSE ) {
			vwork->ext_ntsc = getpid();
			VCADBG_1("vca_open(): ext_ntsc=%d\n", vwork->ext_ntsc);
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_EXIT_OPEN, EBUSY, 
					devno, 0, 0, 0, 0);
			return( EBUSY );	/* device busy */
		}
		break;
	    default:
        	/*-- This case(except application user) do not need to check PID
			 and not have the checked flag. --*/
		VCADBG_0("vca_open():application user or chvmode/lsvmode \n");
		lock_ret = lockl(&(vca_lock), LOCK_SHORT );
		if ( lock_ret == LOCK_SUCC ) {
       		    	fncreg = V7310_GETC( VCA_FNCSEL );
			/* VGA on NTSC is pathru-mode */
		    	if (( fncreg & PATHRU_MODE ) && (!(fncreg & NTSC_BASE))) { 
				unlockl((&vca_lock));
			} else {			/* normal mode */
				unlockl ((&vca_lock));
				DDHKWD5 (HKWD_VCA_DD, DD_EXIT_OPEN, 0, 
					devno, 0, 0, 0, 0);
				return( EBUSY );	/* device busy */
			}
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_EXIT_OPEN, EBUSY, devno, 
				 0, 0, 0, 0);
			return( EBUSY );
		}
		break;
	}

	vwork->device_in_use = TRUE;
	DDHKWD5 (HKWD_VCA_DD, DD_EXIT_OPEN, 0, devno, 0, 0, 0, 0);
	return( 0 );

}


/*-----------------------------------------------------------------------
 *
 * IDENTIFICATION:    vca_close()
 *
 * DESCRIPTIVE NAME:  Video Capture Adapter device close routine
 *
 * FUNCTIONS :        This routine is the standard device driver close routine
 *                    which opposite of open in that it frees systems resources 
 *                    used by device driver
 *
 * INPUTS    :        device number
 *                    Channel number(ignored)
 *                    Extended parameter 
 *
 * OUTPUTS   :        NONE
 *
 *-------------------------------------------------------------------------*/

vca_close( devno, chan, ext )
dev_t	devno;
int	chan;
int	ext;
{
int	minor_dev;
int	lock_ret;
pid_t	user_id;
unsigned char	fncreg, pwrreg, gioreg, gotreg;

  
	DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_CLOSE, 0, devno, chan, 0, 0, 0);

	minor_dev = minor( devno );	/* extract the minor device number */
	if ( minor_dev != 0 ) {
		DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CLOSE, ENXIO, devno, 0, 0, 0, 0);
		return ( ENXIO );
	}
	user_id = getpid();
	VCADBG_1("vca_close(): user_id=%d\n", user_id);

	/*Note: This function is called by last process if multiple processes */ 
	if ( vwork->ext_ccd == user_id ) {
		vwork->ext_ccd = FALSE;
		vwork->device_in_use = FALSE;
		DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CLOSE, 0, devno, chan, 0, 0, 0);
		return( 0 );
	} else if ( vwork->ext_ntsc == user_id ) {
		if (( vwork->pmout_status ) && (!( vwork->ntsc_in_use ))) {
		        pm_planar_control(devno, vwork->ccd_id, PM_PLANAR_OFF);
			vwork->pmout_status = FALSE;
			vwork->device_in_use = FALSE;
#ifdef	PM_SUPPORT
			vwork->request_of_ccd = FALSE;
			pm_vca_ptr->activity = PM_NO_ACTIVITY;	
					/* reset PM activity flag */
#endif	/* PM_SUPPORT */
		}
		vwork->ext_ntsc = FALSE;
		DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CLOSE, 0, devno, chan, 0, 0, 0);
		return( 0 );
	} else if ( vwork->ext_appl == user_id ) {
		vwork->ext_appl = FALSE;
		lock_ret = lockl(&vca_lock, LOCK_SHORT);
		if ( lock_ret == LOCK_SUCC ) {
			/* disable interrupts */
			i_clear(&(vwork->vca_handler));
			unlockl( &vca_lock );
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CLOSE, EBUSY, devno, 0, 0, 0, 0);
			return(EBUSY);          	/* return busy */
		}
		vwork->line_width = 0;
		MotionResetHW();
   	} else {	/* X-server or chvmode/lsvmode command */
		VCADBG_0("vca_close(): X-server or chvmode/lsvmode command\n");
		if (!( vwork->ntsc_in_use ))
			vwork->device_in_use = FALSE;
		DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CLOSE, 0, devno, chan, 0, 0, 0);
		return( 0 );
	}

	if ( vwork->pmout_status ) {
	        pm_planar_control(devno, vwork->ccd_id, PM_PLANAR_OFF);
		vwork->pmout_status = FALSE;
	}
#ifdef	PM_SUPPORT
	vwork->request_of_ccd = FALSE;
	pm_vca_ptr->activity = PM_NO_ACTIVITY;	/* reset PM activity flag */
        vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
	vwork->device_in_use = FALSE;
	DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CLOSE, 0, devno, chan, 0, 0, 0);
	return( 0 );
}
  
/*-----------------------------------------------------------------------
 *
 * IDENTIFICATION:    vca_config()
 *
 * DESCRIPTIVE NAME:  Video Capture Adapter device config routine
 *
 * FUNCTIONS :        This routine is the standard device driver config routine
 *                    which initialize snd terminate for device driver
 *
 * INPUTS    :        device number
 *                    command
 *                    Uiop pointer to user space data 
 *
 * OUTPUTS   :        NONE
 *
 *-------------------------------------------------------------------------*/

vca_config( devno, cmd, uiop )
dev_t	devno;
int	cmd;
struct	uio *uiop;
{
struct	vca_dds	*dds;
struct	devsw	dev_sw;
extern 	int	nodev();
int	config_rc, i, rc;
int	minor_dev;
unsigned char	reg;
volatile PCI_cfg this_slot;
RESIDUAL	residual;


	DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_CONFIG, 0, devno, cmd, 0, 0, 0);
	minor_dev = minor( devno );
	if ( minor_dev != 0 ) {
		DDHKWD5(HKWD_VCA_DD, DD_EXIT_CONFIG, EINVAL, devno, 0, 0, 0, 0);
		return( ENXIO );
	}


	if ( lockl (&(vca_lock), LOCK_SHORT ) != LOCK_SUCC ) {
		DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, EINTR, devno, 0, 0, 0, 0);
		return ( EINTR );
	}

	/*
	 * Determine if Init, Term or Query command
         */

	switch (cmd) 
	{
	case CFG_INIT:

        	/** Determine interrupt level for adapter */
        	if (( rc = pincode( vca_intr )) != 0 ) {
			unlockl(&(vca_lock));
                	DDHKWD5 (HKWD_VCA_DD, DD_EXIT_OPEN, ENOMEM, devno,
                                 0, 0, 0, 0);
                	return( ENOMEM );
        	}

		/* Copy function pointer from LFT to VCA D.D */
		v7310_reset_overlay = reset_overlay_call;
		v7310_power_ctl = power_ctl_call;
		VCADBG_1("vca_config():v7310_reset_overlay=%x\n" ,
						&v7310_reset_overlay);
		VCADBG_1("vca_config():v7310_power_ctl=%x\n" ,&v7310_power_ctl);

		dds = (struct vca_dds *) xmalloc (sizeof (struct vca_dds),
						3, pinned_heap);
		if ( dds == NULL ) {
			unlockl(&(vca_lock));
			unpincode( vca_intr );
			DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, ENOMEM, 
						devno, 0, 0, 0, 0);
			return ( ENOMEM );
		}

		/* Move dds from caller to my area */
		config_rc =  uiomove((caddr_t)dds, (int)sizeof(struct vca_dds), 
					UIO_WRITE, (struct uio *)uiop );
		if ( config_rc != 0 ) {
			xmfree((caddr_t) dds, pinned_heap);
			unlockl(&(vca_lock));
			unpincode( vca_intr );
			DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, EINVAL, 
						devno, 0, 0, 0, 0);
			return ( EINVAL );
		}

		vwork = (struct vca_area *)vca_setup_vptr();
		
		VCADBG_1("vca_config():vwork=%x\n" ,vwork);
		VCADBG_2("io_addr=%x bus_number=%x\n", dds->bus_io_addr, 
			dds->bus_number);
		VCADBG_1("slot_number=%x \n",dds->slot_number);
		/* Initialize current vca structure */
		vwork->dev = devno;
		vwork->bus_number = dds->bus_number;
		vwork->int_priority = dds->device_intr_priority;
		vwork->intr_level = dds->device_intr_level;
		vwork->io_addr = dds->bus_io_addr;
		vwork->ccd_id = dds->pm_ccd_id;
		/* get panel resolution from LFT D.D. */
		get_VDD_info( &vwork->vddinfo );
		VCADBG_2("vca_config(): get_VDD_info():W=%d H=%d\n",
				vwork->vddinfo.lcd_w,vwork->vddinfo.lcd_h);
		vwork->xres = vwork->vddinfo.lcd_w;
		vwork->yres = vwork->vddinfo.lcd_h;
		VCADBG_3("vca_config(): get_VDD_info():x=%d y=%d vram=%d\n",
			vwork->vddinfo.offset_x,vwork->vddinfo.offset_y,vwork->vddinfo.vram_offset);
		VCADBG_3("vca_config(): get_VDD_info():clk=%d Hz=%d vl=%d\n",
			vwork->vddinfo.vram_clk,vwork->vddinfo.v_sync,vwork->vddinfo.v_total);
		vwork->offset_x = vwork->vddinfo.offset_x;
		vwork->offset_y = vwork->vddinfo.offset_y;
		vwork->vram_offset = vwork->vddinfo.vram_offset;
		vwork->vram_clk = vwork->vddinfo.vram_clk;
		vwork->v_sync = vwork->vddinfo.v_sync;
		vwork->v_total = vwork->vddinfo.v_total;	
		vwork->lcd_status = VCA_LCD_ON;
		vwork->crt_status = VCA_CRT_ON;
      
		config_rc = vca_enable(dds);
		VCADBG_1("vca_config(): config_rc=%d\n",config_rc);
    		if ( config_rc != 0 ) {
			xmfree((caddr_t) dds, pinned_heap);
			xmfree((caddr_t) vwork, pinned_heap);
			unlockl(&(vca_lock));
			unpincode( vca_intr );
			DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, EINVAL, 
						devno, 0, 0, 0, 0);
			return ( EINVAL );
    		}

		/* get DMA area(4KB) in kernel */
		for ( i = 0; i < VCA_PAGE_NUM; i++ ) {
        		vwork->dma_vaddr[i] = xmalloc ( PAGESIZE,
                                               PGSHIFT, pinned_heap);
        		if ( vwork->dma_vaddr[i] == NULL ) {
				xmfree((caddr_t) dds, pinned_heap);
				xmfree((caddr_t) vwork, pinned_heap);
				unlockl(&(vca_lock));
				unpincode( vca_intr );
				DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, 
					ENOMEM, devno, 0, 0, 0, 0);
                		return ( ENOMEM );
        		}
			/* get real address */
			vwork->dma_paddr[i] = xmemdma( &vwork->xp, 
				(caddr_t)vwork->dma_vaddr[i], XMEM_HIDE );
        		if ( vwork->dma_paddr[i] == XMEM_FAIL ) {
				for ( i = 0; i < VCA_PAGE_NUM; i++ )
					xmfree(vwork->dma_vaddr[i],pinned_heap);
				xmfree((caddr_t) dds, pinned_heap);
				xmfree((caddr_t) vwork, pinned_heap);
				unlockl(&(vca_lock));
				unpincode( vca_intr );
				DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, 
					ENOMEM, devno, 0, 0, 0, 0);
                		return ( XMEM_FAIL );
        		}
			VCADBG_2("vca_config(): dma_paddr[%d]=%x\n",i, 
					vwork->dma_paddr[i]);
		}

#ifdef	PM_SUPPORT
		pm_vca_ptr = (struct pm_handle *)pm_setup_vca(dds , devno);
		pm_planar_control(devno, vwork->ccd_id, PM_PLANAR_ON);
        	vwork->pm_event = EVENT_NULL;
#endif	/* PM_SUPPORT */
		MotionResetHW();

		/* add entry points to the devsw table */

		dev_sw.d_open 		= vca_open;
		dev_sw.d_close 		= vca_close;
		dev_sw.d_read 		= nodev;
		dev_sw.d_write 		= nodev;
		dev_sw.d_ioctl 		= vca_ioctl;
		dev_sw.d_strategy 	= nodev;
		dev_sw.d_ttys 		= NULL;
		dev_sw.d_select 	= nodev;
		dev_sw.d_config 	= vca_config;
		dev_sw.d_print 		= nodev;
		dev_sw.d_dump 		= nodev;
		dev_sw.d_mpx 		= nodev;
		dev_sw.d_revoke 	= nodev;
		dev_sw.d_dsdptr 	= (char *)vwork;
		dev_sw.d_selptr 	= NULL;
		dev_sw.d_opts 		= 0;

		VCADBG_1("devswadd devno=%d\n",devno);
		if (( config_rc = devswadd( devno, &dev_sw )) != 0 ) {
                    	pm_register_handle( pm_vca_ptr, PM_UNREGISTER );
                    	xmfree( (caddr_t) pm_vca_ptr, pinned_heap );
			xmfree((caddr_t) dds, pinned_heap);
			for ( i = 0; i < VCA_PAGE_NUM; i++ )
				xmfree( vwork->dma_vaddr[i], pinned_heap);
			xmfree((caddr_t) vwork, pinned_heap);
			unlockl(&(vca_lock));
			unpincode( vca_intr );
                        vca_logerr(ERRID_VCA_INITZ, "PCIVCA", "vcadd",
                                "vca_config", config_rc, 100, "100");
			DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, config_rc, 
						devno, 0, 0, 0, 0);
			return ( config_rc );
		}
		/* Get Video Channel Map from residual data of ROS */
		_Get_Residual_Data( &residual );
		_Get_Channel_ID( &residual );

		break;
      
	case CFG_TERM:
		VCADBG_1("vca_config(): CFG_TERM minor=%d\n", minor_dev);

		v7310_reset_overlay = NULL;
		v7310_power_ctl = NULL;
		(void)devswdel( devno );		/* clean up */
		for ( i = 0; i < VCA_PAGE_NUM; i++ )
			xmfree( vwork->dma_vaddr[i], pinned_heap);

#ifdef	PM_SUPPORT
		pm_planar_control(devno, vwork->ccd_id, PM_PLANAR_OFF);
                if ( pm_vca_ptr != NULL ) {
                    rc = pm_register_handle( pm_vca_ptr, PM_UNREGISTER );
                    if ( rc != PM_SUCCESS ) {
                       	vca_logerr(ERRID_VCA_INITZ, "PCIVCA", "vcadd",
                        	"vca_config", rc, 101, "101");
                    }
                    xmfree( (caddr_t) pm_vca_ptr, pinned_heap );
                }
#endif	/* PM_SUPPORT */

    		/* Set bits in PCI command register to disable access */
    		this_slot.command = RESET_BYTES;    /* (I/O and BMT disable) */
    		rc |= CfgSpaceRW (dds->bus_number,dds->slot_number,COMMAND_REG, 
			&this_slot.command, PCI_SHORT_AT, CFG_SPACE_WRITE );

		xmfree((caddr_t) dds, pinned_heap);
		xmfree((caddr_t) vwork, pinned_heap);
		unpincode( vca_intr );
		break;

	default:
		DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, EINVAL, 
						devno, 0, 0, 0, 0);
		unlockl(&vca_lock);
		return ( EINVAL );

	}  /* End switch(cmd) */
 
	unlockl(&vca_lock);
	DDHKWD5 (HKWD_VCA_DD, DD_EXIT_CONFIG, 0, devno, 0, 0, 0, 0);
	VCADBG_0("vca_config end\n");
	return ( 0 );
}


/*-----------------------------------------------------------------------
 *
 * IDENTIFICATION:    vca_ioctl()
 *
 * DESCRIPTIVE NAME:  Video Capture Adapter device ioctl routine
 *
 * FUNCTIONS :        This routine is the standard device driver ioctl routine
 *
 * INPUTS    :        device number
 *                    command
 *                    argument for command
 *                    device flag (ignored)
 *                    Channel number(ignored)
 *                    Extended parameter 
 *
 * OUTPUTS   :        NONE
 *
 *-------------------------------------------------------------------------*/

vca_ioctl( devno, cmd, arg, devflag, chan, ext )
dev_t	devno;
int	cmd;
int	arg;
ulong	devflag;
int	chan;
int	ext;
{
struct	reg_out	out;
struct	reg_in	in;
struct	bt_out	bto;
struct	bt_in	bti;
int	minor_dev;
int	rc = 0;
int	i;
int	loop_counter;
uchar	oldval;
uchar	value;
unsigned int	eaddr;
unsigned char	reg;


	VCADBG_1("vca_ioctl(): ext=%d\n", ext);
	DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, 0, devno, cmd, devflag, 
				chan, ext);

	minor_dev = minor(devno);   /* Get minor device number  */
	/* check if minor_dev is valid */
	if (minor_dev != 0)  {
		DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, cmd, 
			devflag, chan, ext);
		return( ENXIO );  /* Invalid minor device number */
  	}

#ifdef	PM_SUPPORT
        /*
	 * If the cmd is expected to use the DMA, turn on the DMA
	 * activity flag here. Otherwise, application hangs due to wait
	 * DMA end interrupt.
	 */
	switch (cmd) {
	case VCA_IOC_INITIALIZE:
	case VCA_IOC_CAPTURE:
	case VCA_IOC_VRAM_READ:
	case VCA_IOC_VRAM_WRITE:
	case VCA_IOC_AD_CTRL: 
	case VCA_IOC_ACQUISITION: 
	case VCA_IOC_SET_INPUTSW:
	case VCA_IOC_INPUT_SW:
		if (vwork->dma_activity != 0) {
			return (EBUSY);
		} else {
			vwork->dma_activity = 1;
		}
		break;
	}
	Existing_VCA_Request_Entry( cmd );
#endif	/* PM_SUPPORT */
	switch(cmd)
	{     /* process each command */
  
	   case IOCINFO:          /* Standard request for devinfo */
		break;
      
	    case VCA_IOC_DISPLAY_SW:
		if ( ext == VCA_XSERVER ) {
			rc = vca_display_sw( arg );
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	    case VCA_IOC_NTSC_SW:
		if ( ext == VCA_NTSC ) {
			rc = vca_ntsc_sw( arg );
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_CAMERA_SW: 
		if ( ext == 0 ) {
			rc = vca_camera_sw( arg );
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_INPUT_SW:
		if ( ext == 0 ) {
			rc = vca_input_sw( arg );
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
		} else {
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_GET_INPUTSW:
		if ( ext == 0 ) {
			rc = vca_get_inputsw( arg );
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_SET_INPUTSW:
		if ( ext == 0 ) {
			rc = vca_set_inputsw( arg );
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
		} else {
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	    case VCA_IOC_INITIALIZE:
		if ( ext == 0 ) {
			rc = vca_initialize( arg );
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
		} else {
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_ACQUISITION: 
		if ( ext == 0 ) {
			rc = vca_acquisition( arg );
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
		} else {
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_OVERLAY: 
		if ( ext == 0 ) {
			rc = vca_overlay( arg );
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_CAPTURE: 
		if ( ext == 0 ) {
			rc = vca_capture( arg );
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
		} else {
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_AD_CTRL: 
		if ( ext == 0 ) {
			rc = vca_adcon_ctrl( arg );
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
		} else {
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;


	     case VCA_IOC_VRAM_READ: 
		if ( ext == 0 ) {
			rc = vca_vram_read( arg );
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
		} else {
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_VRAM_WRITE: 
		if ( ext == 0 ) {
			rc = vca_vram_write( arg );
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
		} else {
#ifdef	PM_SUPPORT
			vwork->dma_activity = 0;
#endif	/* PM_SUPPORT */
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_XWIN_PAN: 
		if ( ext == VCA_XSERVER ) {
			rc = vca_xwin_pan( arg );
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case VCA_IOC_XPANEL: 
		if ( ext == VCA_XSERVER ) {
			rc = vca_xpanel( arg );
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;


	     case VCA_IOC_CCD_CFG:
		if ( ext == VCA_CCD_CFG ) {
			rc = vca_ccd_cfg( arg );
		} else {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
		break;

	     case  VCA_IOC_REG_OUT:
		if ( ext != VCA_IN_OUT ) {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
        	rc = copyin((char *)arg, &out,
                                sizeof(struct reg_out));
        	if (rc !=0) {
                	return( EFAULT );       /* copyin failed */
        	}
        	V7310_PUTC( out.addr, out.data );
		break;
	
	     case  VCA_IOC_REG_IN:
		if ( ext != VCA_IN_OUT ) {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
        	rc = copyin((char *)arg, &in,
                                sizeof(struct reg_in));
        	if (rc !=0) {
                	return( EFAULT );       /* copyin failed */
        	}
        	in.data = V7310_GETC( in.addr );
		copyout( &in, (char *)arg, sizeof(struct reg_in));
		break;
	
	     case  VCA_IOC_BT_OUT:
		if ( ext != VCA_IN_OUT ) {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
        	rc = copyin((char *)arg, &bto,
                                sizeof(struct bt_out));
        	if (rc !=0) {
                	return( EFAULT );       /* copyin failed */
        	}
        	RegBt812Set( bto.addr, bto.data );
		break;
	
	     case  VCA_IOC_BT_IN:
		if ( ext != VCA_IN_OUT ) {
			DDHKWD5 (HKWD_VCA_DD, DD_ENTRY_IOCTL, ENXIO, devno, 
				cmd, devflag, chan, ext);
		   	return( EINVAL );
		}
        	rc = copyin((char *)arg, &bti,
                                sizeof(struct bt_in));
        	if (rc !=0) {
                	return( EFAULT );       /* copyin failed */
        	}
        	bti.data = RegBt812Get( bti.addr );
		copyout( &bti, (char *)arg, sizeof(struct reg_in));
		break;

	     default:
		rc = ENOTTY;
                vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                	"vca_ioctl", rc, 102, "102");
		return( EINVAL );
		break;
	}

	DDHKWD5 (HKWD_VCA_DD, DD_EXIT_IOCTL, rc, 
			devno, cmd, devflag, chan, ext);
	return(rc);
}


/* ---------------------------------------------------------------
 *   Function Name : MotionSetupHW()
 *   Description   : Initialize hardware register
 *   Called by     : vca_open()
 * ----------------------------------------------------------------*/
MotionSetupHW()
{
	unsigned char	fncreg, pwrreg, gotreg, gioreg, reg;
	int	cnt, rc;

	VCADBG_1("MotionSetupHW: vwork=%x\n", vwork);
        VCADBG_2("lcd=%d crt=%d\n",vwork->lcd_status, vwork->crt_status);

#ifdef	PANEL_PROTECTION
	/*
	 * Turn off LCD for panel protection.
	 */
	if (vwork->lcd_status && vwork->xres == vwork->vddinfo.lcd_w) {
		rc = wfg_lcd_control(0,PM_PLANAR_OFF);
	}
#endif	/* PANEL_PROTECTION */

	/*
 	 * Set power control register power ON 
 	 */
	/* B0h : FIELDE, FIELD0 input */
        gioreg = V7310_GETC( VCA_GIOCTL );
        V7310_PUTC( VCA_GIOCTL, gioreg&~GIOCTL_INPUT ); 
	/* B4h : Bt812/Frame memory power on */
        gotreg = V7310_GETC( VCA_GOTDAT );
        V7310_PUTC( VCA_GOTDAT, gotreg&~GOTDAT_BT_FRAME ); 
	delay(1);
	/* 90h : normal-mode and keep the LCD status */
        if ((!vwork->lcd_status ) || (vwork->xres != vwork->vddinfo.lcd_w))
                V7310_PUTC( VCA_FNCSEL, LCD_POWER_OFF );  /* LCD OFF */
        else
                V7310_PUTC( VCA_FNCSEL, LCD_POWER_ON );  /* LCD ON */
	/* 93h : VRAM/NTSC/BMT power on and keep the CRT status */
        if ( vwork->crt_status )
                V7310_PUTC( VCA_PWRCTL, PWRCTL_DAC_ON );  /* CRT ON */
        else
                V7310_PUTC( VCA_PWRCTL, PWRCTL_DAC_OFF );  /* CRT OFF */

	/* 9Fh : VRAM/NTSC POWER-ON */
        V7310_PUTC( VCA_PWRCT2, PWRCT2_FRAME_NTSC ); 

       	for ( cnt = 0; cnt < sizeof(V7310VGAParm);) 
       		V7310_PUTC( V7310VGAParm[cnt++], V7310VGAParm[cnt++] );
        V7310_PUTC( VCA_GOTDAT, GOTDAT_V7310_DAC );   
					/*  RCAout-OFF DAC-ASCII SYNC-WD */
        V7310_PUTC( VCA_GIOCTL, GIOCTL_OUTPUT );   /*  port 0-4 output enable */
	/* FFh : Software reset to Bt812/Frame memory */
	RegBt812Set( BT_SOFT_RESET, RESET_BITS );
       	for ( cnt = 0; cnt < sizeof(NTSCInParm);) 
       		RegBt812Set( NTSCInParm[cnt++], NTSCInParm[cnt++] );

#ifdef	PANEL_PROTECTION
	/*
	 * Turn on LCD
	 */
	if (vwork->lcd_status && vwork->xres == vwork->vddinfo.lcd_w) {
		rc = wfg_lcd_control(0,PM_PLANAR_ON);
	}
#endif	/* PANEL_PROTECTION */

	return( 0 );
}


/* ---------------------------------------------------------------
 *   Function Name : Enableintr()
 *   Operation     : Initialize to the working flag
 *                 : Registrate to interrupt handler
 *   Called by     : vca_initialize()
 * ----------------------------------------------------------------*/
Enableintr()
{
int     rc;

        VCADBG_1("Enableintr() vwork=%x:\n" ,vwork);

        vwork->ovl_end = 0;
        vwork->intr_mode_set = WRITE_MODE;
        vwork->buf_number = FIRST_BUF;
        vwork->acq_event = EVENT_NULL;
        vwork->bmt_event = EVENT_NULL;
        vwork->hscan_event = EVENT_NULL;

        vwork->vca_handler.next = NULL;
        vwork->vca_handler.handler = vca_intr;
        vwork->vca_handler.bus_type = BUS_BID;
        vwork->vca_handler.bid = vwork->bus_number;
        vwork->vca_handler.level = vwork->intr_level;
        vwork->vca_handler.priority = vwork->int_priority;
        vwork->vca_handler.flags = 0;

        /* registration of interrupt handler fails */
        if ( rc = i_init( &(vwork->vca_handler)) != 0 ) {
                vca_logerr(ERRID_VCA_INTR1, "PCIVCA", "vcadd",
                	"Enableintr", rc, 103, "103");
                return ( ENXIO );
        }

        return ( 0 );
}


/* ---------------------------------------------------------------
 *   Function Name : vca_vram_clr()
 *   Operation     : clear frame memory
 *   Called by     : vca_initialize()
 * ----------------------------------------------------------------*/
int
vca_vram_clr()
{
unsigned char	*memptr;
unsigned long	trans_size, start_addr;
char	*buffer;

/***    This function is to be clear VRAM using bus master transfer  **/
	trans_size = V7310_VRAM_SIZE;
	buffer = (char *) xmalloc (VCA_MAX_TSIZ, 3, kernel_heap);
	bzero ( buffer, (unsigned char *)VCA_MAX_TSIZ );

	VCADBG_0("vca_vram_clr():\n");
	vwork->user_buff = buffer;
	for ( start_addr = 0; start_addr < V7310_VRAM_SIZE; ) {
    	   if ( trans_size > VCA_MAX_TSIZ ) {
    		if ( vram_copy( VRAM_WRITE, start_addr, 							VCA_MAX_TSIZ ) != 0 ) {
			return( EINVAL );
    		}
		start_addr += VCA_MAX_TSIZ;
		trans_size -= VCA_MAX_TSIZ;
 	   } else {
    		if ( vram_copy( VRAM_WRITE, start_addr, 
					trans_size ) != 0 ) {
			return( EINVAL );
    		}
		start_addr = V7310_VRAM_SIZE;	/* exit loop */
	    }
	}
	xmfree((caddr_t) buffer, kernel_heap);
#ifdef  PM_SUPPORT
        vwork->dma_activity = 0;
#endif  /* PM_SUPPORT */
	return( 0 );
}




/* ---------------------------------------------------------------
 *   Function Name : vca_display_sw()
 *   Operation     : Set LCD and CRT on/off
 *   Called by     : X-SERVER and chvmode command
 *   Note          : This function have to called by the X server.
 *		     The X server guarantees to turn off the LCD
 *		     (includes LCDEN signal) to change it's output
 *		     device (LCD and/or CRT). Therefore, it's not
 *		     necessary to turn on/off the LCD from this
 *		     function.
 * ----------------------------------------------------------------*/
int
vca_display_sw( arg )
int	arg;
{
struct display_sw dsp_st_ptr;
unsigned char	reg;
int	rc;

	rc = copyin((char *)arg, &dsp_st_ptr,
				sizeof(struct display_sw));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_2("vca_display_sw(): lcd = %d crt = %d\n",dsp_st_ptr.lcd_out, 
					dsp_st_ptr.crt_out);

	if ( &dsp_st_ptr == NULL ) {
		return( EINVAL );
	}

	reg = V7310_GETC( VCA_FNCSEL );
	if ( dsp_st_ptr.lcd_out == VCA_LCD_ON ) {
	   if ( vwork->vddinfo.lcd_w != vwork->xres ) {
		V7310_PUTC( VCA_FNCSEL, reg & ~LCD_POWER_ON );
	        vwork->lcd_status = VCA_LCD_OFF;
	   } else {
		V7310_PUTC( VCA_FNCSEL, reg | LCD_POWER_ON );
	        vwork->lcd_status = VCA_LCD_ON;
	   }
	} else if ( dsp_st_ptr.lcd_out == VCA_LCD_OFF ) {
		V7310_PUTC( VCA_FNCSEL, reg & ~LCD_POWER_ON );
	        vwork->lcd_status = VCA_LCD_OFF;
	} else {
		return( EINVAL );
	}

	if ( dsp_st_ptr.crt_out == VCA_CRT_ON ) {
	/* V7310-DAC is not used on no overlay mode. */
	        vwork->crt_status = VCA_CRT_ON;
	} else if ( dsp_st_ptr.crt_out == VCA_CRT_OFF ) {
	        vwork->crt_status = VCA_CRT_OFF;
	} else {
		return( EINVAL );
	}

	return (0);
}


/* ---------------------------------------------------------------
 *   Function Name : vca_initialize()
 *   Operation     : Get process id
 *                 : Init V7310 hardware register
 *                 : Enable interrupt handler
 *                 : Set VRAM line width
 *   Called by     : vca_config(), vca_close(), ntsc-off
 * ----------------------------------------------------------------*/
int
vca_initialize( arg )
int	arg;
{
	struct	vca_init_data	init_ptr;
	unsigned char	reg;
	int	rc;

	VCADBG_2("vca_initialize(): xres=%d width = %d\n"
					,vwork->xres,init_ptr.width);

	if (vwork->ext_appl != FALSE) {
		return( EINVAL );
	}

	vwork->ext_appl = getpid();		/* application user */
	/* setup vca area based on adapter configuration */
	MotionSetupHW();

	/* setup vca working flag and enable interrupt handler*/
	Enableintr();

	rc = copyin((char *)arg, &init_ptr,
				sizeof(struct vca_init_data));
	if ( rc != 0 ) {
		return( EFAULT );	/* copyin failed */
	}
	if ( &init_ptr == NULL ) {
		return( EINVAL );
	}
	if (( init_ptr.width < 1 ) || ( init_ptr.width > 32 )) {
		return( EINVAL );
	}

	if ( vwork->line_width != 0 ) {
		vca_vram_clr();
                vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                	"vca_initialize", EINVAL, 104, "104");
		return( EINVAL );

	} else {
		V7310_PUTC(VCA_VRLINW, init_ptr.width);
		vwork->line_width = init_ptr.width * 32;
		vca_vram_clr();
		VCADBG_1("vca_initialize():line_width = %d\n",vwork->line_width);
	}

        if ( vwork->xres == XGA_DISP_WIDTH )  {
                vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                	"vca_initialize", EINVAL, 105, "105");
               	return ( EINVAL );
        }

	return ( 0 );
}


/* ---------------------------------------------------------------
 *   Function Name : vca_capture()
 *   Description   : Transfer acquired data from the area specified
 *                 : by VCA_IOC_ACQUISITION to user's buffer
 *   Called by     : vca_ioctl()
 * ----------------------------------------------------------------*/
int
vca_capture( arg )
int	arg;
{
struct	capture	cap_ptr;
unsigned char	acq_mode, int_st0, int_st1, int_st2;
int	sleep_rc, rc;
unsigned long	start_addr, trans_size;
int	old_level;

	VCADBG_1("vca_capture(): read_mode = %d\n",cap_ptr.read_mode);
	rc = copyin((char *)arg, &cap_ptr,
				sizeof(struct capture));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}

	if ( &cap_ptr == NULL ) {
		return( EINVAL );
	}

	vwork->user_buff = cap_ptr.buff;

	acq_mode = V7310_GETC( VCA_ACQMOD );
	if ( cap_ptr.read_mode == SYNC_ACQ ){/* synchronous to acquisition */
	   	if ( vwork->acq_mode != START_DOUBLE_BUF ) {
			start_addr = (ulong)vwork->acq_addr1;
	    	} else {		/* double buffer */
			vwork->intr_mode_set = CAPTURE_MODE;
			if ( vwork->buf_number == SECOND_BUF ) {
				start_addr = (ulong)vwork->acq_addr1;
			} else {
				start_addr = (ulong)vwork->acq_addr2;
			}
                        V7310_PUTC(VCA_VRAQPA,(uchar)(start_addr&0x00ff));
                        V7310_PUTC(VCA_VRAQPA+1,
                                        (uchar)((start_addr>>8)&0x00ff));
                        V7310_PUTC(VCA_VRAQPA+2,
                                        (uchar)((start_addr>>16)&0x00ff));
	    	}

          	if (( vwork->acq_mode==START_ACQ )||( vwork->acq_mode==START_DOUBLE_BUF )) {
			if ( _Acq_Waiting( acq_mode ) != 0 )
         			return( EINTR );
	        }

	} else if ( cap_ptr.read_mode == ASYNC_ACQ ){	/* asynchoronous */
		start_addr = (ulong)vwork->acq_addr1;
	} else
         	return( EINTR );

	trans_size = (ulong)( vwork->acq_trans_h * vwork->acq_trans_w * 2 );

	VCADBG_4("vca_capture(): W=%d H=%d start_addr=%x trans_size=%d\n"
		,vwork->acq_trans_w ,vwork->acq_trans_h, start_addr,trans_size);
	if ( vram_copy( VRAM_READ, start_addr, trans_size ) != 0 ) {
		return( EINVAL );
	}
        if ((vwork->acq_mode==START_ACQ)||(vwork->acq_mode==START_DOUBLE_BUF)) {
		V7310_PUTC( VCA_ACQMOD, acq_mode );   	
	}
#ifdef  PM_SUPPORT
        vwork->dma_activity = 0;
#endif  /* PM_SUPPORT */
						/* restart acquisition */
	return( 0 );
}


/* ---------------------------------------------------------------
 *   Function Name : vca_adcon_ctrl()
 *   Description   : Control the BT812 AD converter
 *   Called by     : vca_ioctl()
 * ----------------------------------------------------------------*/
int
vca_adcon_ctrl( arg )
int	arg;
{
struct	adcon_ctrl	adcon_ctrl_ptr;
unsigned char   acq_mode;
int	rc;

	rc = copyin((char *)arg, &adcon_ctrl_ptr,
				sizeof(struct adcon_ctrl));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_1("vca_adcon_ctrl(): bright = %d\n",adcon_ctrl_ptr.bright);
	VCADBG_1("vca_adcon_ctrl(): contrast = %d\n",adcon_ctrl_ptr.contrast);
	VCADBG_1("vca_adcon_ctrl(): saturation = %d\n",adcon_ctrl_ptr.saturation);
	VCADBG_1("vca_adcon_ctrl(): hue = %d\n",adcon_ctrl_ptr.hue);
	if ( &adcon_ctrl_ptr == NULL ) {
		return( EINVAL );
	}

	vwork->intr_mode_set = WRITE_MODE;
        acq_mode = V7310_GETC( VCA_ACQMOD);

        if ( acq_mode & SET_BIT0 ) {
		if ( _Acq_Waiting( acq_mode ) != 0 )
         		return( EINTR );
        }

	RegBt812Set( BT_BRIGHTNESS, adcon_ctrl_ptr.bright&SET_REV_BIT0 );
	RegBt812Set( BT_CONTRAST, adcon_ctrl_ptr.contrast&SET_REV_BIT0 );
	RegBt812Set( BT_SATURATION, adcon_ctrl_ptr.saturation&SET_REV_BIT0 );
	RegBt812Set( BT_HUE, adcon_ctrl_ptr.hue&SET_REV_BIT0 );

        if ((vwork->acq_mode==START_ACQ)||(vwork->acq_mode==START_DOUBLE_BUF)) {
		V7310_PUTC( VCA_ACQMOD, acq_mode );   	
						/* restart acquisition */
	}

	return (0);
}


/* ---------------------------------------------------------------
 *   Function Name : vca_ntsc_sw()
 *   Operation     : In case of ntsc-off, reset hardware register
 *                 : In case of ntsc on, overlay to VGA on NTSC
 *   Called by     : ntsc command
 * ----------------------------------------------------------------*/
int
vca_ntsc_sw( arg )
int	arg;
{
	struct	ntsc_sw ntsc_sw_ptr;
	unsigned char	reg, fncreg, gioreg, gotreg;
	int	rc, cnt;

	rc = copyin((char *)arg, &ntsc_sw_ptr,
				sizeof(struct ntsc_sw));
	if ( rc != 0 ) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_1("vca_ntsc_sw(): ntsc_out = %d\n",ntsc_sw_ptr.ntsc_out);
	if ( &ntsc_sw_ptr == NULL ) {
		return( EINVAL );
	}

/* 
 * NTSC-OFF sequence
 */
	if ( ntsc_sw_ptr.ntsc_out == NTSC_OFF ) {	
		vwork->ntsc_in_use = FALSE;
		MotionResetHW();
	} else if ( ntsc_sw_ptr.ntsc_out == NTSC_ON ) {	/* NTSC ON */
/* 
 * NTSC-ON sequence
 */
		if ( vwork->ntsc_in_use )
			return ( EBUSY );
        	if ( vwork->xres == XGA_DISP_WIDTH ) {
                        vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                        	"vca_ntsc_sw", EINVAL, 107, "107");
               		return ( EINVAL );
        	}
		vwork->ntsc_in_use = TRUE;
		vwork->request_of_ccd = TRUE;

#ifdef	PANEL_PROTECTION
		/*
		 * Turn off LCD for panel protection.
		 */
       		if (vwork->lcd_status && vwork->xres == vwork->vddinfo.lcd_w) {
			rc = wfg_lcd_control(0,PM_PLANAR_OFF);
		}
#endif	/* PANEL_PROTECTION */

		/*
 		 * Set power control register power ON 
 		 */
		/* B0h : FIELDE, FIELD0 input and RCAout enable */
        	gioreg = V7310_GETC( VCA_GIOCTL );
        	V7310_PUTC( VCA_GIOCTL, (gioreg&~GIOCTL_INPUT)|RCAOUT_ENABLE );
		/* B4h : Bt812/Frame memory power on */
        	gotreg = V7310_GETC( VCA_GOTDAT );
        	V7310_PUTC( VCA_GOTDAT, gotreg&~GOTDAT_BT_FRAME );
		delay(1);
		/* 93h : VRAM/NTSC/BMT/CRT power on and keep the CRT status */
               	V7310_PUTC( VCA_PWRCTL, PWRCTL_DAC_ON  );  /* CRT ON */
		/* 9Fh : NTSC POWER-ON */
        	V7310_PUTC( VCA_PWRCT2, PWRCT2_FRAME_NTSC );

		if ( vwork->xres == SVGA_DISP_WIDTH )  {
       		   for ( cnt = 0; cnt < sizeof(v7310_ntsc_800);) 
       			V7310_PUTC( v7310_ntsc_800[cnt++], v7310_ntsc_800[cnt++] );
		} else {	/* VGA mode (640x480) */
       		   for ( cnt = 0; cnt < sizeof(v7310_ntsc_640);) 
       			V7310_PUTC( v7310_ntsc_640[cnt++], v7310_ntsc_640[cnt++] );
  		   if ( vwork->v_sync == HIGH_REFRESH_RATE ) 
	        	V7310_PUTC( VCA_AQLINS, ACQ_START_HIGH );
		   else 
	        	V7310_PUTC( VCA_AQLINS, ACQ_START_LOW );
		}
       		if ((!vwork->lcd_status ) || (vwork->xres != vwork->vddinfo.lcd_w))
               		V7310_PUTC( VCA_FNCSEL, POWER_OFF_NTSC );  /* LCD OFF */
       		else
               		V7310_PUTC( VCA_FNCSEL, POWER_ON_NTSC );  /* LCD ON */

		RegBt812Set( BT_SOFT_RESET, RESET_BITS );
       		for ( cnt = 0; cnt < sizeof(NTSCInParm);) 
       			RegBt812Set( NTSCInParm[cnt++], NTSCInParm[cnt++] );
		_Select_BT_channel();

#ifdef	PANEL_PROTECTION
		/*
		 * Turn on LCD, if it's needed.
		 */
       		if (vwork->lcd_status && vwork->xres == vwork->vddinfo.lcd_w) {
			rc = wfg_lcd_control(0,PM_PLANAR_ON);
		}
#endif	/* PANEL_PROTECTION */

	} else {
		return( EINVAL );
	}

	return(0);
}


/* ---------------------------------------------------------------
 *   Function Name : vca_xwin_pan()
 *   Description   : Get view port geometry
 *   Called by     : X-Server
 * ----------------------------------------------------------------*/
int
vca_xwin_pan( arg )
int	arg;
{
struct	xwin_pan xwin_pan_ptr;
unsigned short	start_x, start_y, end_x, end_y, out_x, out_y;
unsigned int	ovl_mem_addr;
int	rc;

	rc = copyin((char *)arg, &xwin_pan_ptr,
				sizeof(struct xwin_pan));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_2("vca_xwin_pan(): pan_x = %d pan_y = %d\n",xwin_pan_ptr.view_x,xwin_pan_ptr.view_y);
	if ( &xwin_pan_ptr == NULL ) {
		return( EINVAL );
	}

	vwork->pan_x = xwin_pan_ptr.view_x;
	vwork->pan_y = xwin_pan_ptr.view_y;
	if ( vwork->ext_appl != FALSE ) {
		start_x = vwork->win_sx - xwin_pan_ptr.view_x;
		start_y = vwork->win_sy - xwin_pan_ptr.view_y;
		end_x = vwork->win_ex - xwin_pan_ptr.view_x;
		end_y = vwork->win_ey - xwin_pan_ptr.view_y;
		out_x = vwork->out_x - xwin_pan_ptr.view_x;
		out_y = vwork->out_y - xwin_pan_ptr.view_y;

		vca_register_overlay( start_x, start_y, end_x, end_y, out_x, out_y );
	}

	return(0);
}


/* ---------------------------------------------------------------
 *   Function Name : vca_xpanel()
 *   Description   : Get panel resolution and information
 *   Called by     : X-Server
 * ----------------------------------------------------------------*/
int
vca_xpanel( arg )
int	arg;
{
	struct	xpanel xpanel_ptr;
	int	rc;


	rc = copyin((char *)arg, &xpanel_ptr,
				sizeof(struct xpanel));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_2("vca_xpanel():x=%d y=%d\n",xpanel_ptr.phys_w,xpanel_ptr.phys_h);
	VCADBG_2("log_w=%d log_h=%d\n",xpanel_ptr.log_w,xpanel_ptr.log_h);
	VCADBG_2("off_x=%d off_y=%d\n",xpanel_ptr.offset_x,xpanel_ptr.offset_y);
	VCADBG_2("clock=%x offset=%d\n",xpanel_ptr.vram_clk, xpanel_ptr.vram_offset);
	VCADBG_2("v_total=%d v_sync=%d\n",xpanel_ptr.v_total,xpanel_ptr.v_sync);
	if ( &xpanel_ptr == NULL ) {
		return( EINVAL );
	}


#ifdef	PANEL_PROTECTION
	if (vwork->ext_appl) {
		unsigned short  start_x, start_y, end_x, end_y, out_x, out_y;
		
		vwork->v_total = xpanel_ptr.v_total;
		vwork->v_sync = xpanel_ptr.v_sync;

		if (( vwork->v_sync == HIGH_REFRESH_RATE ) && ( vwork->ntsc_in_use ))
			V7310_PUTC( VCA_AQLINS, ACQ_START_HIGH );

		vwork->win_sx += (xpanel_ptr.offset_x - vwork->offset_x);
		vwork->win_sy += (xpanel_ptr.offset_y - vwork->offset_y);
		vwork->win_ex += (xpanel_ptr.offset_x - vwork->offset_x);
		vwork->win_ey += (xpanel_ptr.offset_y - vwork->offset_y);
		vwork->out_y += (xpanel_ptr.vram_offset - vwork->vram_offset);
		
		start_x = vwork->win_sx - vwork->pan_x;
		start_y = vwork->win_sy - vwork->pan_y;
		end_x = vwork->win_ex - vwork->pan_x;
		end_y = vwork->win_ey - vwork->pan_y;
		out_x = vwork->out_x - vwork->pan_x;
		out_y = vwork->out_y - vwork->pan_y;
		
		vwork->xres = xpanel_ptr.phys_w;
		vwork->yres = xpanel_ptr.phys_h;
		vwork->offset_x = xpanel_ptr.offset_x;
		vwork->offset_y = xpanel_ptr.offset_y;
		vwork->vram_clk = xpanel_ptr.vram_clk;
		vwork->vram_offset = xpanel_ptr.vram_offset;
		
		vca_register_overlay( start_x, start_y, end_x, end_y, out_x, out_y);
	} else {
		vwork->xres = xpanel_ptr.phys_w;
		vwork->yres = xpanel_ptr.phys_h;
		vwork->offset_x = xpanel_ptr.offset_x;
		vwork->offset_y = xpanel_ptr.offset_y;
		vwork->vram_clk = xpanel_ptr.vram_clk;
		vwork->vram_offset = xpanel_ptr.vram_offset;
		vwork->v_total = xpanel_ptr.v_total;
		vwork->v_sync = xpanel_ptr.v_sync;
		
		if (( vwork->v_sync == HIGH_REFRESH_RATE ) && ( vwork->ntsc_in_use ))
			V7310_PUTC( VCA_AQLINS, ACQ_START_HIGH );
	}
#else	/* PANEL_PROTECTION */
	vwork->xres = xpanel_ptr.phys_w;
	vwork->yres = xpanel_ptr.phys_h;
	vwork->offset_x = xpanel_ptr.offset_x;
	vwork->offset_y = xpanel_ptr.offset_y;
	vwork->vram_clk = xpanel_ptr.vram_clk;
	vwork->vram_offset = xpanel_ptr.vram_offset;
	vwork->v_total = xpanel_ptr.v_total;
	vwork->v_sync = xpanel_ptr.v_sync;

	if (( vwork->v_sync == HIGH_REFRESH_RATE ) && ( vwork->ntsc_in_use ))
	       	V7310_PUTC( VCA_AQLINS, ACQ_START_HIGH );
#endif	/* PANEL_PROTECTION */

	return(0);
}



/* -----------------------------------------------------------------------------
 *   Function Name : vca_vram_read()
 *   Description   : Read VRAM data to user I/O buffer from V7310 adapter memory
 *   Called by     : vca_ioctl()
 * ---------------------------------------------------------------------------*/
int
vca_vram_read( arg )
int	arg;
{
struct	vram_xfer vram_xfer_ptr;
unsigned long	start_addr, trans_size;
unsigned char	reg, acq_mode;
int	sleep_rc, rc;

	rc = copyin((char *)arg, &vram_xfer_ptr,
				sizeof(struct vram_xfer));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_1("vca_vram_read(): vram_x = %d\n",vram_xfer_ptr.vram_x);
	if ( &vram_xfer_ptr == NULL ) {
		return( EINVAL );
	}

	/* set transfer start address */
	start_addr = (ulong)( vwork->line_width 
		* vram_xfer_ptr.vram_y + vram_xfer_ptr.vram_x ) *2;
	
	trans_size = (ulong)( vwork->line_width * ( vram_xfer_ptr.vram_h - 1 )
				+ vram_xfer_ptr.vram_w ) * 2;

	VCADBG_2("vca_vram_read():  start_addr=%x trans_size=%d\n"
			, start_addr,trans_size);
	vwork->user_buff = vram_xfer_ptr.buff;

	if ( vram_copy( VRAM_READ, start_addr, trans_size ) != 0 ) {
		return( EINVAL );
	}
#ifdef  PM_SUPPORT
        vwork->dma_activity = 0;
#endif  /* PM_SUPPORT */

	return(0);
}


/* --------------------------------------------------------------
 *   Function Name : vca_vram_write()
 *   Description   : Write VRAM data to V7310 adapter memory
 *                 : from user I/O buffer
 *   Called by     : vca_ioctl()
 * --------------------------------------------------------------*/
int
vca_vram_write( arg )
int	arg;
{
unsigned long	start_addr, trans_size;
struct	vram_xfer vram_xfer_ptr;
unsigned char   acq_mode;
int	rc;

	rc = copyin((char *)arg, &vram_xfer_ptr,
				sizeof(struct vram_xfer));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_1("vca_vram_write(): vram_x = %d\n",vram_xfer_ptr.vram_x);
	if ( &vram_xfer_ptr == NULL ) {
		return( EINVAL );
	}

	/* set transfer start address */
	start_addr = (ulong)( vwork->line_width 
		* vram_xfer_ptr.vram_y + vram_xfer_ptr.vram_x ) * 2;
	
	trans_size = (ulong)( vwork->line_width * ( vram_xfer_ptr.vram_h - 1)
				+ vram_xfer_ptr.vram_w ) * 2;

	VCADBG_2("vca_vram_write(): start_addr = %x trans_size = %d\n"
			,start_addr ,trans_size);

	vwork->user_buff = vram_xfer_ptr.buff;

	vwork->intr_mode_set = WRITE_MODE;
        acq_mode = V7310_GETC( VCA_ACQMOD );

        if ( acq_mode & SET_BIT0 ) {
		if ( _Acq_Waiting( acq_mode ) != 0 )
         		return( EINTR );
		if (vram_copy( VRAM_WRITE, start_addr, trans_size) != 0 ) {
			return( EINVAL );
		}
        	V7310_PUTC( VCA_ACQMOD, acq_mode );
                                               /* restart acquisition */
	} else {
		if (vram_copy( VRAM_WRITE, start_addr, trans_size) != 0 ) {
			return( EINVAL );
		}
	}
#ifdef  PM_SUPPORT
        vwork->dma_activity = 0;
#endif  /* PM_SUPPORT */

	return(0);
}


/* ---------------------------------------------------------------
 *   Function Name : vca_overlay()
 *   Operation      : Set parameter for overlay
 *   Called by     : vca_ioctl()
 * ----------------------------------------------------------------*/
int
vca_overlay( arg )
int	arg;
{
struct	overlay ovl_ptr;
unsigned char	reg, zoom;
unsigned short  start_x, start_y, end_x, end_y, out_x, out_y;
unsigned long	ovl_mem_addr, acq_addr;
int	rc;

	rc = copyin((char *)arg, &ovl_ptr,
				sizeof(struct overlay));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_4("vca_overlay(): ovl_mode=%d ovl_area=%d vram_w=%d vram_h=%d\n"
		,ovl_ptr.ovl_mode,ovl_ptr.ovl_area,ovl_ptr.vram_w,ovl_ptr.vram_h);
	VCADBG_2("vca_overlay(): x:%d y:%d\n",ovl_ptr.ovl_x, ovl_ptr.ovl_y);

	if ( &ovl_ptr == NULL ) {
		return( EINVAL );
	}

	if (( ovl_ptr.ovl_mode == NOCOLOR_ALL ) || 
			( ovl_ptr.ovl_mode == COLOR_ALL ) ) {
		ovl_ptr.ovl_x = 0;
		ovl_ptr.ovl_y = 0;
		ovl_ptr.vram_w = vwork->xres;
		ovl_ptr.vram_h = vwork->yres;
	}

        if (( ovl_ptr.vram_w > vwork->xres ) || ( ovl_ptr.vram_h > vwork->yres ))
        {
                vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                	"vca_overlay", EFAULT, 110, "110");
                return( EFAULT );
        }
	vwork->win_sx = ovl_ptr.ovl_x + vwork->offset_x;
	vwork->win_sy = ovl_ptr.ovl_y + vwork->offset_y;
	vwork->win_ex = ovl_ptr.ovl_x + (u_short)ovl_ptr.vram_w-1+vwork->offset_x;
	vwork->win_ey = ovl_ptr.ovl_y + (u_short)ovl_ptr.vram_h-1+vwork->offset_y;
	start_x = vwork->win_sx - vwork->pan_x;
	start_y = vwork->win_sy - vwork->pan_y;
	end_x = vwork->win_ex - vwork->pan_x;
	end_y = vwork->win_ey - vwork->pan_y;

	/* set zooming ratio */
	zoom = SNCZOM_ACTIVE_HIGH;	/* synchronize signal: active high */
	switch( ovl_ptr.zoom_x)
	{
		case NO_ZOOM:
			zoom &= ~H_8TIMES ;
			vwork->zox = ZOOM_VALUE1;
			break;
		case ZOOM_TWICE:
			zoom = (zoom &~SET_BIT1)| SET_BIT0;
			vwork->zox = ZOOM_VALUE2;
			break;
		case ZOOM_4TIMES:
			zoom = (zoom &~SET_BIT0)| SET_BIT1;
			vwork->zox = ZOOM_VALUE3;
			break;
		case ZOOM_8TIMES:
			zoom |=  H_8TIMES;
			vwork->zox = ZOOM_VALUE4;
			break;
		default:
			return( EINVAL );
	}

	switch( ovl_ptr.zoom_y)
	{
		case NO_ZOOM:
			zoom &= ~V_8TIMES;
			vwork->zoy = ZOOM_VALUE1;
			break;
		case ZOOM_TWICE:
			zoom = (zoom &~SET_BIT3)| SET_BIT2;
			vwork->zoy = ZOOM_VALUE2;
			break;
		case ZOOM_4TIMES:
			zoom = (zoom &~SET_BIT2)| SET_BIT3;
			vwork->zoy = ZOOM_VALUE3;
			break;
		case ZOOM_8TIMES:
			zoom |= V_8TIMES;
			vwork->zoy = ZOOM_VALUE4;
			break;
		default:
			return( EINVAL );
	}
	V7310_PUTC(VCA_SNCZOM, zoom);

	/* set 60h,61h,62h */
	vwork->out_x = ovl_ptr.ovl_x - ovl_ptr.vram_x;
	vwork->out_y = ovl_ptr.ovl_y - ovl_ptr.vram_y + vwork->vram_offset;

        out_x = vwork->out_x - vwork->pan_x;
	out_y = vwork->out_y - vwork->pan_y;

	vca_register_overlay( start_x, start_y, end_x, end_y, out_x, out_y);

	/* set color keying */
	V7310_PUTC( VCA_KCLGYL, ovl_ptr.green_l );
	V7310_PUTC( VCA_KCLRCL, ovl_ptr.red_l );
	V7310_PUTC( VCA_KCLBCL, ovl_ptr.blue_l );
	V7310_PUTC( VCA_KCLGYH, ovl_ptr.green_h );
	V7310_PUTC( VCA_KCLRCH, ovl_ptr.red_h );
	V7310_PUTC( VCA_KCLBCH, ovl_ptr.blue_h );

	/* set overlay mode */
	reg = V7310_GETC(VCA_OVLCTL);
	switch(ovl_ptr.ovl_mode)
	{
		case NOCOLOR_ALL:
		case NOCOLOR_AREA:
			V7310_PUTC( VCA_OVLCTL, reg & ~COLOR_KEY_ENABLE | AREA_IN_USE );
			break;
		case COLOR_ALL:
		case COLOR_AREA:
			V7310_PUTC( VCA_OVLCTL, reg | COLOR_KEY_AREA );
			break;
		default:
			return( EINVAL );
	}

        /* select overlay area */
        reg = V7310_GETC( VCA_OVLCTL);
        if ( ovl_ptr.ovl_area == RESET_BITS ) {
                reg = RESET_BITS;
        } else {
            if ( ovl_ptr.ovl_area & SET_BIT0 )              /* F0 area */
                reg |= SET_BIT2;
            if ( ovl_ptr.ovl_area & SET_BIT1 ) {     /* F1 area */
                if (( ovl_ptr.ovl_mode == COLOR_AREA ) ||
                        ( ovl_ptr.ovl_mode == NOCOLOR_AREA )) {
                        reg |= SET_BIT3;
                } else {
                    vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                    	"vca_overlay", EINVAL, 111, "111");
                    return(EINVAL);
                }
            }
            if ( ovl_ptr.ovl_area & SET_BIT2 ) {     /* F2 area */
                if (( ovl_ptr.ovl_mode == COLOR_AREA ) ||
                        ( ovl_ptr.ovl_mode == COLOR_ALL )) {
                        reg |= SET_BIT4;
                } else {
                        vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                        	"vca_overlay", EINVAL, 112, "112");
                        return(EINVAL);
                }
            }
            if ( ovl_ptr.ovl_area & SET_BIT3 ) {     /* F3 area */
                if ( ovl_ptr.ovl_mode == COLOR_AREA ) {
                        reg |= SET_BIT5;
                } else {
                        vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                        	"vca_overlay", EINVAL, 113, "113");
                        return(EINVAL);
                }
            }
        }
	V7310_PUTC( VCA_OVLCTL, reg );

	VCADBG_1("vca_overlay(): ovlctl=%x\n",V7310_GETC(VCA_OVLCTL));
	return( 0 );
}


/* ---------------------------------------------------------------
 *   Function Name : vca_register_overlay()
 *   Operation     : Set parameter for overlay
 *   Called by     : vca_overlay() and vca_xwin_pan()
 * ----------------------------------------------------------------*/
int
vca_register_overlay( start_x, start_y, end_x, end_y, out_x, out_y )
unsigned short  start_x, start_y, end_x, end_y, out_x, out_y;
{
unsigned long	ovl_mem_addr;

	VCADBG_4("sx=%d sy=%d ex=%d ey=%d\n",(short)start_x,(short)start_y,(short)end_x,(short)end_y);
	VCADBG_2("outx=%d outy=%d\n",(short)out_x,(short)out_y);

	/* Wrap the overlay area when the wondow is outsize */
	if ((start_x < vwork->offset_x ) || (((short)start_x ) < 0 )){
		start_x = vwork->offset_x;
		out_x +=  vwork->line_width*vwork->zox;
		out_y -= vwork->zoy;
	}
	if ((start_y < vwork->offset_y ) || (((short)start_y) < 0 )) {
		start_y = vwork->offset_y;
	}
	if ((end_x < vwork->offset_x ) || (((short)end_x) < 0 )) {
		end_x = vwork->offset_x;
	}
	if ((end_y < vwork->offset_y ) || (((short)end_y) < 0 )){
		end_y = vwork->offset_y;
	}

	/* this value is set a3 and a4 regiter  on vram_copy()*/
        if ( end_y > vwork->yres+vwork->offset_y )
                vwork->ovl_end = vwork->yres+vwork->offset_y;
        else
                vwork->ovl_end = end_y;

	V7310_PUTC( VCA_OVRCHS, (uchar)(start_x&0x00ff));
	V7310_PUTC( VCA_OVRCHS+1, (uchar)((start_x>>8)&0x0000ff));
	V7310_PUTC( VCA_OVRCVS, (uchar)(start_y&0x00ff));
	V7310_PUTC( VCA_OVRCVS+1, (uchar)((start_y>>8)&0x0000ff));
	V7310_PUTC( VCA_OVRCHE, (uchar)(end_x&0x00ff));
	V7310_PUTC( VCA_OVRCHE+1, (uchar)((end_x>>8)&0x0000ff));
	V7310_PUTC( VCA_OVRCVE, (uchar)(end_y&0x00ff));
	V7310_PUTC( VCA_OVRCVE+1, (uchar)((end_y>>8)&0x0000ff));

	out_y -= out_y % vwork->zoy;
	out_x -= out_x % vwork->zox;
	ovl_mem_addr = (ulong)( out_x/vwork->zox + vwork->line_width 
				* out_y/vwork->zoy );
	VCADBG_3("zox=%d zoy=%d mem_addr=%d\n",vwork->zox,vwork->zoy,ovl_mem_addr);
	V7310_PUTC( VCA_VRROPA,
		(uchar)(((OVLMEM_START_ADDR - ovl_mem_addr) * 2 ) & 0x0000ff));
	V7310_PUTC( VCA_VRROPA+1,
		(uchar)((((OVLMEM_START_ADDR - ovl_mem_addr)*2)>>8)&0x0000ff));
	V7310_PUTC( VCA_VRROPA+2,
		(uchar)((((OVLMEM_START_ADDR - ovl_mem_addr)*2)>>16)&0x0000ff));
	if ( vwork->zox  > 1 ) 
		V7310_PUTC( VCA_VRRCST, (uchar)(vwork->vram_clk - 1) );
	else 		/* NO ZOOM */
		V7310_PUTC( VCA_VRRCST, (uchar)vwork->vram_clk);

	return ( 0 );
}


/* ---------------------------------------------------------------
 *   Function Name : vca_acquisition()
 *   Opertion      : Set parameter for acquisition
 *                 : Start or stop acquisition
 *   Called by     : vca_ioctl()
 * ----------------------------------------------------------------*/
int
vca_acquisition( arg )
int	arg;
{
struct acquisition acq_ptr;
unsigned char	reg, acq_lines, acqmode_reg;
unsigned long	acq_addr;
unsigned char	acq_ctl;
int	rc;

	rc = copyin((char *)arg, &acq_ptr,
				sizeof(struct acquisition));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_1("vca_acquisition(): acq_mode = %d\n",acq_ptr.acq_mode);
	VCADBG_4("acq_x = %x acq_y=%x acq_w=%d acq_h=%d\n",acq_ptr.acq_x, 
			acq_ptr.acq_y,acq_ptr.acq_w,acq_ptr.acq_h);
	VCADBG_2("scale_x = %d scale_y=%d\n",acq_ptr.scale_x, acq_ptr.scale_y);
	if ( &acq_ptr == NULL ) {
		return( EINVAL );
	}
	vwork->acq_mode = acq_ptr.acq_mode;

	acqmode_reg = V7310_GETC( VCA_ACQMOD );

	if ( acq_ptr.acq_mode == START_DOUBLE_BUF ) {
		VCADBG_0("vca_acquisition(): double_buffer\n");

		acq_addr = (ulong)( vwork->line_width * acq_ptr.vram_y 
	     		 + acq_ptr.vram_x ) * 2;
		V7310_PUTC( VCA_VRAQPA, (uchar)(acq_addr&0x00ff));
		V7310_PUTC( VCA_VRAQPA+1, (uchar)((acq_addr>>8)&0x00ff));
		V7310_PUTC( VCA_VRAQPA+2, (uchar)((acq_addr>>16)&0x00ff));
		acqmode_reg |= MOTION_BITS;
		V7310_PUTC( VCA_ACQMOD, acqmode_reg );
		vwork->acq_addr2 = acq_addr;
		vwork->buf_number = SECOND_BUF;
		return ( 0 );
	}

	
	/* check if acquition area position is valid or not*/
	if (( acq_ptr.acq_x < 0 ) || ( acq_ptr.acq_y < 0 ) )
		return( EINVAL );
	if ((( acq_ptr.acq_x+acq_ptr.acq_w ) > ACQ_DISP_WIDTH ) ||
		(( acq_ptr.acq_y+acq_ptr.acq_h ) > ACQ_DISP_HEIGHT )) {
		return( EINVAL );
	}

	if ( acq_ptr.acq_area == AREA_ALL ) {
		acq_ptr.acq_x = 0;
		acq_ptr.acq_y = 0;
		acq_ptr.acq_w = ACQ_DISP_WIDTH;
		acq_ptr.acq_h = ACQ_DISP_HEIGHT;
	}

	V7310_PUTC( VCA_AQRCHS, 
		(uchar)((acq_ptr.acq_x + AQRCHS_OFFSET) & 0x00ff ));
	V7310_PUTC( VCA_AQRCHS+1, 
		(uchar)(((acq_ptr.acq_x + AQRCHS_OFFSET) >> 8 ) & 0x0000ff ));
	V7310_PUTC( VCA_AQRCVS, (uchar)(acq_ptr.acq_y & 0x00ff ));
	V7310_PUTC( VCA_AQRCVS+1,(uchar)((acq_ptr.acq_y >> 8) & 0x0000ff));
	V7310_PUTC( VCA_AQRCHE, (uchar)((acq_ptr.acq_x+acq_ptr.acq_w - 1
						+ AQRCHS_OFFSET) & 0x00ff));
	V7310_PUTC( VCA_AQRCHE+1,(uchar)(((acq_ptr.acq_x+acq_ptr.acq_w - 1 
					+ AQRCHS_OFFSET) >> 8 ) & 0x0000ff));
	V7310_PUTC( VCA_AQRCVE, (uchar)((acq_ptr.acq_y+acq_ptr.acq_h - 1
					+ AQRCVE_OFFSET) & 0x00ff));
	V7310_PUTC( VCA_AQRCVE+1, (uchar)(((acq_ptr.acq_y+acq_ptr.acq_h - 1
					+ AQRCVE_OFFSET) >> 8 ) & 0x0000ff));
	
	acq_addr = (ulong)( vwork->line_width * acq_ptr.vram_y 
	    	 + acq_ptr.vram_x ) * 2;

	V7310_PUTC( VCA_VRAQPA, (uchar)(acq_addr&0x00ff));
	V7310_PUTC( VCA_VRAQPA+1, (uchar)((acq_addr>>8)&0x00ff));
	V7310_PUTC( VCA_VRAQPA+2, (uchar)((acq_addr>>16)&0x00ff));
	vwork->acq_addr1 = acq_addr;

	/* set scaling ratio */
	acq_ctl = V7310_GETC( VCA_AQRCTL);
	if (( 0 < acq_ptr.scale_x ) && ( acq_ptr.scale_x < 64 )) {
		V7310_PUTC(VCA_ACQHSC, acq_ptr.scale_x);
		vwork->acq_trans_w = acq_ptr.acq_w * acq_ptr.scale_x/64;
		acq_ctl |= HSCALE_BITS;
	} else {
		vwork->acq_trans_w = acq_ptr.acq_w;
		acq_ctl &= ~HSCALE_BITS;
	}

	if (( 1 <= acq_ptr.scale_y ) && ( acq_ptr.scale_y <= 31 )) {
		if ( vwork->video_in_flag == V7310_PALC )
	      		V7310_PUTC( VCA_AQLINS, ACQ_START_PALIN );
	        else
	      		V7310_PUTC( VCA_AQLINS, ACQ_START_HIGH );
		V7310_PUTC( VCA_ACQVSC, acq_ptr.scale_y*2 );
		vwork->acq_trans_h = acq_ptr.acq_h * acq_ptr.scale_y/64;
		acqmode_reg |= VIDEO_INT_BITS;		/* non-interlace */
		acq_ctl |= VSCALE_BITS;
	} else if (( 32 <= acq_ptr.scale_y ) && ( acq_ptr.scale_y <= 63 )) {
		if ( vwork->video_in_flag == V7310_PALC )
	      		V7310_PUTC(VCA_AQLINS, ACQ_START_PALIN2 );
							/* VDELAY 33lines */
	        else
	      		V7310_PUTC(VCA_AQLINS, ACQ_START_LOW );	
							/* VDELAY 22 lines */
		V7310_PUTC(VCA_ACQVSC, acq_ptr.scale_y);
		vwork->acq_trans_h = acq_ptr.acq_h * acq_ptr.scale_y/64;
		acqmode_reg &= ~VIDEO_INT_BITS;			/* interlace */
		acq_ctl |= VSCALE_BITS;
	} else {
		if ( vwork->video_in_flag == V7310_PALC )
	      		V7310_PUTC( VCA_AQLINS, ACQ_START_PALIN2 );	
							/* VDELAY 33lines */
	        else
	      		V7310_PUTC( VCA_AQLINS, ACQ_START_LOW );	
							/* VDELAY 22 lines */
		vwork->acq_trans_h = acq_ptr.acq_h;
		acqmode_reg &= ~VIDEO_INT_BITS;			/* interlace */
		acq_ctl &= ~VSCALE_BITS;
	}

	/* set 21h Acquisition control */

	reg = V7310_GETC( VCA_GOTDAT );
	if ( reg  & RCAOUT_ENABLE )
		acq_ctl |= INT_SIGNAL_BITS;
							/* internal signal */
	else {
		reg = V7310_GETC( VCA_GIOCTL );
		V7310_PUTC( VCA_GIOCTL, (reg | SIGNAL_PORT_OFF));
							/* port 6,7 off */
		acq_ctl |= EXT_SIGNAL_BITS;
							/* external signal */
	}

	switch(acq_ptr.acq_area) 
	{
		case AREA_IN:
			acq_ctl |= AREA_IN_USE;
			break;
		case AREA_OUT:
			acq_ctl |= AREA_OUT_USE;
			vwork->acq_trans_w = ACQ_DISP_WIDTH;
			vwork->acq_trans_h = ACQ_DISP_HEIGHT;
			break;
		case AREA_ALL:
			acq_ctl |= AREA_IN_USE;
			vwork->acq_trans_w = ACQ_DISP_WIDTH;
			vwork->acq_trans_h = ACQ_DISP_HEIGHT;
			break;
		default:
			return( EINVAL );
	}
	V7310_PUTC( VCA_AQRCTL, acq_ctl );

	/* acquisition start */
	if (( acq_ptr.acq_mode == STILL ) || ( acq_ptr.acq_mode == STOP_ACQ )) {
		vwork->intr_mode_set = WRITE_MODE;
		if ( _Acq_Waiting( acqmode_reg ) != 0 )
         		return( EINTR );
	} else {
		acqmode_reg = (acqmode_reg & ~STOP_BITS) | MOTION_BITS ;
	      	V7310_PUTC( VCA_ACQMOD, acqmode_reg );
	}

	VCADBG_2("acquisition(): 20h=%x 21h=%x\n",V7310_GETC( VCA_ACQMOD )
		,V7310_GETC( VCA_AQRCTL));
	return(0);

}


/* ---------------------------------------------------------------
 *   Function Name : vca_camera_sw()
 *   Description   : Select CCD camera's signal destination
 *   Called by     : vca_ioctl()
 * ----------------------------------------------------------------*/
int
vca_camera_sw( arg )
int	arg;
{
struct camera_sw camera_sw_ptr;
unsigned char control = 0;
int	rc;

	rc = copyin((char *)arg, &camera_sw_ptr,
				sizeof(struct camera_sw));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_1("vca_camera_sw(): camera = %d\n",camera_sw_ptr.camera);
	if ( &camera_sw_ptr == NULL ) {
		return( EINVAL );
	}

	/* video signal source select */
	switch(camera_sw_ptr.camera)
	{
		case NTSC_OUT:
		   control = V7310_GETC( VCA_GIOCTL );
		   V7310_PUTC( VCA_GIOCTL, control | RCAOUT_ENABLE  );
								/* PORT7 ON */
		   control=V7310_GETC(VCA_GOTDAT);
		   V7310_PUTC(VCA_GOTDAT, control | RCAOUT_ENABLE );
								/* PORT7 HIGH */
		   vwork->request_of_ccd = TRUE;
		   break;
		case AD_CON:
		   control=V7310_GETC(VCA_GIOCTL);
		   V7310_PUTC(VCA_GIOCTL, control | RCAOUT_ENABLE );
								/* PORT7 ON */
		   control=V7310_GETC(VCA_GOTDAT);
		   V7310_PUTC(VCA_GOTDAT, control & ~RCAOUT_ENABLE );
								/* PORT7 LOW */
		   break;
		default:
		   return(EINVAL);
	}
	return ( 0 );
}



/* ---------------------------------------------------------------
 *   Function Name : vca_get_inputsw()
 *   Description   : Get appropriate video input source 
 *   Called by     : vca_ioctl()
 * ----------------------------------------------------------------*/
int
vca_get_inputsw( arg )
int	arg;
{
struct video_info info_ptr;
int	rc, cnt;

	VCADBG_0("vca_get_inputsw():");
	for ( cnt = 0; cnt < PORTNUM; cnt ++) {
		VCADBG_2("VCMap[%d]=%x",cnt,vwork->res_chan_map[cnt]);
		info_ptr.VCMap[cnt] = vwork->res_chan_map[cnt];
	}
	rc = copyout( &info_ptr, (char *)arg, sizeof(struct video_info));
	if ( rc != 0 ) {
		return( EFAULT );	/* copyin failed */
	}
	return ( 0 );
}

/* ---------------------------------------------------------------
 *   Function Name : vca_set_inputsw()
 *   Description   : Select appropriate video input source 
 *   Called by     : vca_ioctl()
 * ----------------------------------------------------------------*/
int
vca_set_inputsw( arg )
int	arg;
{
struct input_source input_ptr;
unsigned char	acq_mode;
unsigned char control = 0, reg;
int	rc, port, cnt;

	rc = copyin((char *)arg, &input_ptr,
				sizeof(struct input_source));
	if ( rc != 0 ) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_2("vca_set_inputsw(): port_id = %d video_in\n"
				,input_ptr.port_id, input_ptr.video_in);
	if ( &input_ptr == NULL ) {
		return( EINVAL );
	}

	vwork->intr_mode_set = WRITE_MODE;
        acq_mode = V7310_GETC( VCA_ACQMOD);

        if ( acq_mode & MOTION_BITS ) {
		if ( _Acq_Waiting( acq_mode ) != 0 )
         		return( EINTR );
        }

	switch( input_ptr.video_in ) 
	{
		case V7310_CCDCamera:
		  if ( vwork->ccd_status == CCD_OFF ) {
                        vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                        	"set_input_sw", EINVAL, 115, "115");
		   	return( EINVAL );
		   }

		   switch( input_ptr.port_id )
		   {
			case	PORT0:
		   		RegBt812Set( BT_INPUT, VID0_Y );
				break;
			case	PORT1:
		   		RegBt812Set( BT_INPUT, VID1_Y );
				break;
			case	PORT2:
		   		RegBt812Set( BT_INPUT, VID2_Y );
				break;
			case	PORT3:
		   		RegBt812Set( BT_INPUT, VID3_Y );
				break;
			default:
				return( EINVAL );
		   }
        	   for ( cnt = 2; cnt < sizeof(NTSCInParm);) 
                	RegBt812Set( NTSCInParm[cnt++], NTSCInParm[cnt++] );
		   control = V7310_GETC( VCA_AQRCTL );
		   V7310_PUTC( VCA_AQRCTL, control | 0x40 );
		   vwork->video_in_flag = V7310_CCDCamera;
		   vwork->request_of_ccd = TRUE;
		   break;

		case V7310_NTSCC:
		   switch( input_ptr.port_id )
		   {
			case	PORT0:
		   		RegBt812Set( BT_INPUT, VID0_Y );
				break;
			case	PORT1:
		   		RegBt812Set( BT_INPUT, VID1_Y );
				break;
			case	PORT2:
		   		RegBt812Set( BT_INPUT, VID2_Y );
				break;
			case	PORT3:
		   		RegBt812Set( BT_INPUT, VID3_Y );
				break;
			default:
				return( EINVAL );
		    }
        	   for ( cnt = 2; cnt < sizeof(NTSCInParm);) 
                	RegBt812Set( NTSCInParm[cnt++], NTSCInParm[cnt++] );
		   control = V7310_GETC( VCA_AQRCTL );
		   V7310_PUTC( VCA_AQRCTL, control & ~0x40 );
		   vwork->video_in_flag = V7310_NTSCC;
		   vwork->request_of_ccd = FALSE;
		   break;

		case V7310_NTSCS:
		   switch( input_ptr.port_id )
		   {
			case	PORT0:
		   		RegBt812Set( BT_INPUT, VID0_Y | VID0_C );
				break;
			case	PORT1:
		   		RegBt812Set( BT_INPUT, VID1_Y | VID1_C );
				break;
			case	PORT2:
		   		RegBt812Set( BT_INPUT, VID2_Y | VID2_Y );
				break;
			case	PORT3:
		   		RegBt812Set( BT_INPUT, VID3_Y | VID3_Y );
				break;
			default:
				return( EINVAL );
		   }
        	   for ( cnt = 2; cnt < sizeof(NTSCInParm);)  {
			if ( cnt == 8 ) {
                	    RegBt812Set( 0x05, 0x80 );
			    cnt++; cnt++;
			}
			else
                	    RegBt812Set( NTSCInParm[cnt++], NTSCInParm[cnt++] );
		   }
		   control = V7310_GETC( VCA_AQRCTL );
		   V7310_PUTC( VCA_AQRCTL, control & ~0x40 );
		   vwork->video_in_flag = V7310_NTSCS;
		   vwork->request_of_ccd = FALSE;
		   break;

		case V7310_PALC:
		   switch( input_ptr.port_id )
		   {
			case	PORT0:
		   		RegBt812Set( BT_INPUT, VID0_Y );
				break;
			case	PORT1:
		   		RegBt812Set( BT_INPUT, VID1_Y );
				break;
			case	PORT2:
		   		RegBt812Set( BT_INPUT, VID2_Y );
				break;
			case	PORT3:
		   		RegBt812Set( BT_INPUT, VID3_Y );
				break;
			default:
				return( EINVAL );
		    }
        	   for ( cnt = 2; cnt < sizeof(PALInParm);) 
                	    RegBt812Set( PALInParm[cnt++], PALInParm[cnt++] );
		   control = V7310_GETC( VCA_AQRCTL );
		   V7310_PUTC( VCA_AQRCTL, control & ~0x40 );
		   vwork->video_in_flag = V7310_PALC;
		   vwork->request_of_ccd = FALSE;
		   break;

		case V7310_PALS:
		   switch( input_ptr.port_id )
		   {
			case	PORT0:
		   		RegBt812Set( BT_INPUT, VID0_Y | VID0_C );
				break;
			case	PORT1:
		   		RegBt812Set( BT_INPUT, VID1_Y | VID1_C );
				break;
			case	PORT2:
		   		RegBt812Set( BT_INPUT, VID2_Y | VID2_Y );
				break;
			case	PORT3:
		   		RegBt812Set( BT_INPUT, VID3_Y | VID3_Y );
				break;
			default:
				return( EINVAL );
		   }
        	   for ( cnt = 2; cnt < sizeof(PALInParm);) {
			if ( cnt == 8 ) {
                	    RegBt812Set( 0x05, 0xf4 );
			    cnt++; cnt++;
			}
			else
                	    RegBt812Set( PALInParm[cnt++], PALInParm[cnt++] );
		   }
		   control = V7310_GETC( VCA_AQRCTL );
		   V7310_PUTC( VCA_AQRCTL, control & ~0x40 );
		   vwork->video_in_flag = V7310_PALS;
		   vwork->request_of_ccd = FALSE;
		   break;

		default:
		   return( EINVAL );
	}

        if ((vwork->acq_mode==START_ACQ)||(vwork->acq_mode==START_DOUBLE_BUF)) {
		V7310_PUTC( VCA_ACQMOD, acq_mode );   	
						/* restart acquisition */
	}

	return( 0 );
}

/* ----------------------------------------------------------------------
 *   Function Name : _Get_Residual_Data()
 *   Description   : Get the residual date from soft ROS
 *   Called by     : vca_get_inputsw()
 * ----------------------------------------------------------------------*/
int
_Get_Residual_Data( rp )
RESIDUAL *rp;
{
IPL_DIRECTORY	ipl_ptr;
struct file     *fd;
int 	rc;
MACH_DD_IO	mdd;

	VCADBG_0("_Get_Residual_Data():\n");
        rc = fp_open("/dev/nvram", O_RDONLY, 0, 0, SYS_ADSPACE, &fd);
        if ( rc ){
                return( -1 );
        }

        mdd.md_addr = 128;
        mdd.md_data = (char *)&ipl_ptr;
        mdd.md_size = sizeof(ipl_ptr);
        mdd.md_incr = MV_BYTE;

        if(fp_ioctl(fd, MIOIPLCB, &mdd, 0)){
                return -1;
        }

        fp_close(fd);

        rc = fp_open("/dev/nvram", O_RDONLY, 0, 0, SYS_ADSPACE, &fd);
        if ( rc ){
                return( -1 );
        }

        mdd.md_addr = ipl_ptr.residual_offset;
        mdd.md_data = (char *)rp;
        mdd.md_size = ipl_ptr.residual_size;
        mdd.md_incr = MV_BYTE;

        if(fp_ioctl(fd, MIOIPLCB, &mdd, 0)){
                return -1;
        }

        fp_close(fd);
        return 0;
}

/* ----------------------------------------------------------------------
 *   Function Name : _Get_Channel_ID()
 *   Description   : Get the video channel information from residual date
 *                 : of soft ROS
 *   Called by     : vca_get_inputsw()
 * ----------------------------------------------------------------------*/

int
_Get_Channel_ID( rp )
RESIDUAL *rp;
{
VidChanInfoPack *VidPack;
int     size, i, cnt, pack_num;

	VCADBG_0("_Get_Channel_ID():\n");
	for (i=0;i<rp->ActualNumDevices;i++ ){
         if ( (rp->Devices[i].DeviceId.BusId    == PCIDEVICE ) &&
              (rp->Devices[i].DeviceId.BaseType == MultimediaController )){

                if (rp->Devices[i].AllocatedOffset) {
		    VidPack = (struct _VidChanInfoPack  *)
                            &rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset];

                    while ( (VidPack->Tag != L4_Packet) 
				|| (VidPack->Type != 0xb)){ /* Vendor defined */

                    /* ------------------------------------------- *
                     *   Look through all packets until            *
                     *        the 1st VidPack is reached.        *
                     * ------------------------------------------- */

                        if ( VidPack->Tag & 0x80 ) { /* large tag */
                                VidPack = (struct _VidChanInfoPack  *)
                                        ((caddr_t) VidPack +
                                        (((struct _L1_Pack *) VidPack)->
                                            Count1 << 8) +
                                        ((struct _L1_Pack *) VidPack)->
                                            Count0 +3);
                        } else if ((VidPack->Tag & 0xf8) == END_TAG) {
				VCADBG_1("_Get_Channel_ID(): END_TAG=%x\n", VidPack->Tag);
                                break;
                        } else {	/* small tag */
                                VidPack = (struct _VidChanInfoPack  *)
                                        ((caddr_t)VidPack +
                                        (VidPack->Tag & 0x07) +1);
                        }
                    } /* end of while */
		    VCADBG_2("_Get_Channel_ID(): Tag=%x Type=%x\n",VidPack->Tag, VidPack->Type);
                    if ( VidPack->Tag != L4_Packet) {

                      /* ------------------------------------ *
                       *  No packet found, logs the error     *
                       * ------------------------------------ */

			vwork->res_chan_map[0] = V7310_CCDCamera;
			vwork->res_chan_map[1] = V7310_NTSCC | V7310_PALC;
			vwork->res_chan_map[2] = V7310_Nosignal;
			vwork->res_chan_map[3] = V7310_Nosignal;
                       	vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                        		"_Get_Channel_ID", EINVAL, 116, "116");
                    } else {
			pack_num = VidPack->Count0;
			VCADBG_0("<======== RESIDUAL DATA =======>\n");
			for ( cnt = 0; cnt < (pack_num - 1)/5; cnt++ ) {
				VCADBG_5("%x %x %x %x %x\n",VidPack->VCMap[0].LogicalChannelID,VidPack->VCMap[0].VideoIO,VidPack->VCMap[0].PhysicalChannelID,VidPack->VCMap[0].ConnectorType,VidPack->VCMap[0].SignalType);
			   switch(VidPack->VCMap[1].PhysicalChannelID){
  			   case	0:
  			   case	1:
				vwork->res_chan_map[0] |= _Get_Video_Signal( VidPack->VCMap[0].SignalType );
				if ( vwork->res_chan_map[0] & V7310_NTSCC )
					vwork->res_chan_map[0]=V7310_CCDCamera;
				break;
  			   case	2:
  			   case	3:
				vwork->res_chan_map[1] |= _Get_Video_Signal( VidPack->VCMap[0].SignalType );
				break;
  			   case	4:
  			   case	5:
				vwork->res_chan_map[2] |= _Get_Video_Signal( VidPack->VCMap[0].SignalType );
				break;
  			   case	6:
  			   case	7:
				vwork->res_chan_map[3] |= _Get_Video_Signal( VidPack->VCMap[0].SignalType );
				break;
			   default:
				break;
			   }
                       	   VidPack = (struct _VidChanInfoPack  *)
                                      ((caddr_t)VidPack + sizeof(VidChanMap));
			}
                    }
                } else {
                        /* ------------------------------------ *
                         *  No packet found, logs the error     *
                         * ------------------------------------ */
			vwork->res_chan_map[0] = V7310_CCDCamera;
			vwork->res_chan_map[1] = V7310_NTSCC | V7310_PALC;
			vwork->res_chan_map[2] = V7310_Nosignal;
			vwork->res_chan_map[3] = V7310_Nosignal;
                       	vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                        		"_Get_Channel_ID", EINVAL, 117, "117");
                }
         }
    }
    return ( 0 );
}

/* ---------------------------------------------------------------
 *   Function Name : _Get_Video_Signal()
 *   Description   : Get the video input source format 
 *   Called by     : _Get_Channel_ID()
 * ----------------------------------------------------------------*/
int
_Get_Video_Signal( signal )
unsigned char	signal;
{
int	map_val = 0;

	VCADBG_1("_Get_Video_Signal(): signal=%x\n", signal);
	switch ( signal ) {
		case	NTSCComposite:
			map_val = V7310_NTSCC;
			break;
		case	NTSCLume:
			map_val = V7310_NTSCS;
			break;
		case	NTSCChroma:
			map_val = V7310_NTSCS;
			break;
		case	PALComposite:
			map_val = V7310_PALC;
			break;
		case	PALLume:
			map_val = V7310_PALS;
			break;
		case	PALChroma:
			map_val = V7310_PALS;
			break;
		case	NTSCTimingOnly:
			vwork->ntsc_timing_in = TRUE;
			map_val = V7310_Nosignal;
			break;
		default:
			map_val = V7310_Nosignal;
			break;
	}
	return ( map_val );
}


/* ---------------------------------------------------------------
 *   Function Name : vca_input_sw()
 *   Description   : Select NTSC input CCD Camera/External Video
 *   Called by     : vca_ioctl()
 * ----------------------------------------------------------------*/
int
vca_input_sw( arg )
int	arg;
{
struct input_sw in_sw_ptr;
unsigned char control = 0, reg;
unsigned char	acq_mode;
int	rc, cnt;

	rc = copyin((char *)arg, &in_sw_ptr,
				sizeof(struct input_sw));
	if (rc !=0) {
		return( EFAULT );	/* copyin failed */
	}
	VCADBG_1("vca_input_sw(): ntsc_in = %d\n",in_sw_ptr.ntsc_in);
	if ( &in_sw_ptr == NULL ) {
		return( EINVAL );
	}

	vwork->intr_mode_set = WRITE_MODE;
        acq_mode = V7310_GETC( VCA_ACQMOD );

        if ( acq_mode & MOTION_BITS ) {
		if ( _Acq_Waiting( acq_mode ) != 0 )
         		return( EINTR );
        }

	switch(in_sw_ptr.ntsc_in)
	{
		case CCD_IN:
		  if (vwork->ccd_status == CCD_OFF ) {
                        vca_logerr(ERRID_VCA_IOCTL1, "PCIVCA", "vcadd",
                        	"input_sw", EINVAL, 114, "114");
		   	return( EINVAL );
		   }

		   RegBt812Set( BT_INPUT, VID0_Y );
        	   for ( cnt = 2; cnt < sizeof(NTSCInParm);) 
                	RegBt812Set( NTSCInParm[cnt++], NTSCInParm[cnt++] );
		   control=V7310_GETC( VCA_AQRCTL );
		   V7310_PUTC(VCA_AQRCTL, control | SET_BIT6 );
		   vwork->video_in_flag = CCD_IN;
		   vwork->request_of_ccd = TRUE;
		   break;

		case VIDEO_EX:
		   RegBt812Set( BT_INPUT, VID1_Y );
        	   for ( cnt = 2; cnt < sizeof(NTSCInParm);) 
                	RegBt812Set( NTSCInParm[cnt++], NTSCInParm[cnt++] );
		   control=V7310_GETC(VCA_AQRCTL);
		   V7310_PUTC(VCA_AQRCTL, control & ~SET_BIT6 );
		   vwork->video_in_flag = VIDEO_EX;
		   vwork->request_of_ccd = FALSE;
		   break;

		default:
		   return( EINVAL );
	}

        if ((vwork->acq_mode==START_ACQ)||(vwork->acq_mode==START_DOUBLE_BUF)) {
		V7310_PUTC( VCA_ACQMOD, acq_mode );   	
						/* restart acquisition */
	}

	return( 0 );
}


/* -----------------------------------------------------------
 *   Function Name : vca_ccd_cfg()
 *   Description   : Determine CCD status to try 5 times
 *   Called by     : vca_enable()
 * ------------------------------------------------------------*/
vca_ccd_cfg(ccd_cfg_ptr)
struct ccd_cfg *ccd_cfg_ptr;
{
unsigned char	reg;
register int	cnt;

	VCADBG_1("vca_ccd_cfg(): Reg(0x2)=%x\n",RegBt812Get(0x02));
	if ( vwork->res_chan_map[0] & V7310_CCDCamera ) {
        	vwork->ccd_status = CCD_ON;	/* Woodfield and WF prime */
	} else {
		vwork->ccd_status = CCD_OFF;	/* Wiltwick */
	}
	ccd_cfg_ptr->ccd_status = vwork->ccd_status;
	VCADBG_1("vca_ccd_cfg(): ccd_status=%d\n",vwork->ccd_status);

	return( 0 );
}


/* --------------------------------------------------------------------
 *   Function Name : _Select_BT_channel()
 *   Description   : Distinguish Woodfield and Woodfield Prime
 *                 : If Prime, select Bt812 channel 2
 *   Called by     : vca_ntsc_sw(), vca_camera_sw()
 * ---------------------------------------------------------------------*/
_Select_BT_channel()
{
register int	cnt;
unsigned char	reg;

	VCADBG_1("_Select_BT_channel(): %d\n",vwork->ntsc_timing_in);
	if ( vwork->ntsc_timing_in ) {
       		RegBt812Set( BT_INPUT, VID2_Y|VID2_C );
	} else {				/* Woodfield */
       		RegBt812Set( BT_INPUT, VID0_Y ); 
	}

	return ( 0 );
}
