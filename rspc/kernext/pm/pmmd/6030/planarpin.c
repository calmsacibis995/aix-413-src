static char sccsid[] = "@(#)63  1.10  src/rspc/kernext/pm/pmmd/6030/planarpin.c, pwrsysx, rspc41J, 9520A_all 5/12/95 13:42:48";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Planar control routines for Power Management (PM)
 *              < Pinned Routines >
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 *   STATUS:
 *      07/13/94
 *      08/03/94  add cpu_idle, unconfig_pm_planar_control
 *      08/04/94  remove unnecessary initialize
 *      08/05/94  separate I/O routine  w, w/o lock
 *      08/05/94  separate pinned part and unpinned part
 *      08/24/94  L2 cache support
 *      09/07/94  support LCD ON request reject during lid open, change
 *                CRT low power for 6020/6030, support inform_slave_of_standby.
 *      09/09/94  remove lcd half brightness
 *      09/12/94  if system state is suspend(partial), the request of turning
 *                CRT/LCD on from X-server/LFT is rejected.
 *      10/04/94  No use CD awake control for partial suspend
 *      11/09/94  add Doze Disable for FullOn, no L2 cache control for Doze
 *      03/08/95  L2 cache support is dropped since it can't support write-back.
 */

/* INCLUDE */

#include <fcntl.h>
#include <sys/scsi.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/inline.h>
#include <sys/pm.h>
#include "planar.h"
#include "pdplanar.h"
#include "pdppcfnc.h"
#include "pm_6030.h"
#include "../../pmmi/pmmi.h"


/* DATA,STRUCTURE DEFINITION */

struct _pm_pmc_data     pm_pmc_data;

struct pm_planar_control_handle ppch[planar_control_handle_max];

struct _pm_isadev_data  pm_isadev_data;


/* EXTERN */

extern void inform_slave_of_suspend();
extern void inform_slave_of_standby();
extern int pm_planar_control_psumain(dev_t devno, int devid, int cmd);
extern  struct  _pm_core_data   pm_core_data;
extern  struct  _pm_md_data     pm_md_data;
extern	pm_control_data_t	pm_control_data;
extern pm_HW_state_data_t pm_HW_state_data;  /* HW state data */



/******************************************************************************
 * NAME: read_pmc
 *
 * FUNCTION: read Power Management Controller data
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      index = data to Power Management Controller index register
 *
 * RETURN VALUE:
 *      data from Power Management Controller PMDATA register
 */
int
read_pmc_nolock(int index)
{
        volatile char   *io;
        int             data;

        /* I/O access */
        io = (volatile char *)iomem_att(&(pm_pmc_data.map))
                                                + pm_pmc_data.base;

        *((volatile char *)io) = (char)index;
	eieio();
        data = *((volatile char *)(io + 1));
	eieio();

        /* I/O access end */
        iomem_det((void *)io);

        return data;
}

int
read_pmc(int index)
{
        int             data;
        int             opri;

        /* serialize access to pm_pmc_data */
        opri = disable_lock(INTMAX, &(pm_pmc_data.lock));

        /* I/O access */
        data = read_pmc_nolock(index);

        /* deserialize access to pm_pmc_data */
        unlock_enable(opri, &(pm_pmc_data.lock));

        return data;
}

/******************************************************************************
 * NAME: write_pmc
 *
 * FUNCTION: write Power Management Controller data
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      index = data to Power Management Controller PMINDEX register
 *      data  = data to Power Management Controller PMDATA register
 *
 * RETURN VALUE:
 *      written data to Power Management Controller PMDATA register
 */
int
write_pmc_nolock(int index, char data)
{
        volatile char   *io;

        /* I/O access */
        io = (volatile char *)iomem_att(&(pm_pmc_data.map))
                                                + pm_pmc_data.base;

        *((volatile char *)io) = (char)index;
	eieio();
        *((volatile char *)(io + 1)) = data;
	eieio();

        /* I/O access end */
        iomem_det((void *)io);

        return (int)data;
}

int
write_pmc(int index, char data)
{
        int             opri;

        /* serialize access to pm_pmc_data */
        opri = disable_lock(INTMAX, &(pm_pmc_data.lock));

        /* I/O access */
        data = write_pmc_nolock(index, data);

        /* deserialize access to pm_pmc_data */
        unlock_enable(opri, &(pm_pmc_data.lock));

        return (int)data;
}

/******************************************************************************
 * NAME: modify_pmc
 *
 * FUNCTION: modify Power Management Controller data bits
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      index       = data to pmc PMINDEX register
 *      modify_bits = Bit(s) to be modified
 *      data_bits   = Bit(s) to be set or reset
 *
 * RETURN VALUE:
 *      written data to Power Management Controller PMDATA register
 */
int
modify_pmc_nolock(int index, int modify_bits, int data_bits)
{
        volatile char   *io;
        char            data;

        /* I/O access */
        io = (volatile char *)iomem_att(&(pm_pmc_data.map))
                                                + pm_pmc_data.base;

        *((volatile char *)io) = (char)index;
	eieio();
        data = *((volatile char *)(io + 1));
	eieio();

        data &= ~modify_bits;
        data |= (data_bits & modify_bits);

        *((volatile char *)(io + 1)) = data;
	eieio();

        /* I/O access end */
        iomem_det((void *)io);

        return (int)data;
}

int
modify_pmc(int index, int modify_bits, int data_bits)
{
        char            data;
        int             opri;

        /* serialize access to pm_pmc_data */
        opri = disable_lock(INTMAX, &(pm_pmc_data.lock));

        /* I/O access */
        data = modify_pmc_nolock(index, modify_bits, data_bits);

        /* deserialize access to pm_pmc_data */
        unlock_enable(opri, &(pm_pmc_data.lock));

        return (int)data;
}

/******************************************************************************
 * NAME: read_isadev
 *
 * FUNCTION: read isa device port
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      addr = address of isa device
 *
 * RETURN VALUE:
 *      data from isa device
 */
int
read_isadev_nolock(int addr)
{
        volatile char   *io;
        int             data;

        /* I/O access */
        io = (volatile char *)iomem_att(&(pm_isadev_data.map)) + addr;

        data = *((volatile char *)(io));

        /* I/O access end */
        iomem_det((void *)io);

        return data;
}

int
read_isadev(int addr)
{
        int             data;
        int             opri;

        /* serialize access to pm_isadev_data */
        opri = disable_lock(INTMAX, &(pm_isadev_data.lock));

        /* I/O access */
        data = read_isadev_nolock(addr);

        /* deserialize access to pm_isadev_data */
        unlock_enable(opri, &(pm_isadev_data.lock));

        return data;
}

/******************************************************************************
 * NAME: write_isadev
 *
 * FUNCTION: write data into isa device port
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      addr  = address of isa device
 *      data  = data to write
 *
 * RETURN VALUE:
 *      written data to isa device port
 */
int
write_isadev_nolock(int addr, char data)
{
        volatile char   *io;

        /* I/O access */
        io = (volatile char *)iomem_att(&(pm_isadev_data.map)) + addr;

        *((volatile char *)(io)) = data;

        /* I/O access end */
        iomem_det((void *)io);

        return (int)data;
}

int
write_isadev(int addr, char data)
{
        int             opri;

        /* serialize access to pm_isadev_data */
        opri = disable_lock(INTMAX, &(pm_isadev_data.lock));

        /* I/O access */
        data = write_isadev_nolock(addr, data);

        /* deserialize access to pm_isadev_data */
        unlock_enable(opri, &(pm_isadev_data.lock));

        return (int)data;
}

/******************************************************************************
 * NAME: modify_isadev
 *
 * FUNCTION: modify isa device data bits
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      addr        = address of isa device port
 *      modify_bits = Bit(s) to be modified
 *      data_bits   = Bit(s) to be set or reset
 *
 * RETURN VALUE:
 *      written data to isa device port
 */
int
modify_isadev_nolock(int addr, int modify_bits, int data_bits)
{
        volatile char   *io;
        char            data;

        /* I/O access */
        io = (volatile char *)iomem_att(&(pm_isadev_data.map)) + addr;

        data = *((volatile char *)(io));
	eieio();

        data &= ~modify_bits;
        data |= (data_bits & modify_bits);

        *((volatile char *)(io)) = data;
	eieio();

        /* I/O access end */
        iomem_det((void *)io);

        return (int)data;
}

int
modify_isadev(int addr, int modify_bits, int data_bits)
{
        char            data;
        int             opri;

        /* serialize access to pm_isadev_data */
        opri = disable_lock(INTMAX, &(pm_isadev_data.lock));

        /* I/O access */
        data = modify_isadev_nolock(addr, modify_bits, data_bits);

        /* deserialize access to pm_isadev_data */
        unlock_enable(opri, &(pm_isadev_data.lock));

        return (int)data;
}


/******************************************************************************/
/******************************************************************************/
/**  DEVICE POWER CONTROL SUBROUTINES  ****************************************/
/******************************************************************************/
/******************************************************************************
 * NAME: pm_planar_control_lcd
 *
 * FUNCTION: LCD control for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      devno
 *      devid  =PMDEV_LCD
 *      cmd    = PM_PLANAR_QUERY
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *               PM_PLANAR_LOWPOWER1 (= no action)
 *               PM_PLANAR_LOWPOWER2 (= no action)
 *		 PM_PLANAR_CURRENT_LEVEL
 *
 * RETURN VALUE:
 *      if (cmd = PM_PLANAR_QUERY)
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *	else if (cmd = PM_PLANAR_CURRENT_LEVEL)
 *		 the last setup level
 *      else
 *               PM_SUCCESS,  PM_ERROR
 */
int
pm_planar_control_lcd( dev_t devno, int devid, int cmd )
{
    int     data;

    switch( cmd ){

      case PM_PLANAR_ON :
        DBG(LCD ON request has just been accepted.)
	
	pm_HW_state_data.video_dd_request = PM_PLANAR_ON;

        /*  Reject if system is in suspend(partial) state. */
        if ( pm_core_data.system_state == PM_SYSTEM_FULL_ON
          || pm_core_data.system_state == PM_SYSTEM_ENABLE
          || pm_core_data.system_state == PM_SYSTEM_STANDBY
          || pm_control_data.phase_1_only == FALSE ) {

                /*  If LID is closed, LCD ON request is rejected. */
                if (pm_control_data.phase_1_only == FALSE) {
                        if (pm_HW_state_data.LID_state == PM_LID_CLOSE) {
                                break; /* not needed to turn on. */
			}
                } else {
                        if( pm_core_data.lid_state == PM_LID_CLOSE ) {
                                break; /* note needed to turn on */
                        }
                }
                DBG(LCD ON is done.)

                modify_pmc( INDEXPMC_PMCO0,
                                   BITPMCPMCO0_LCDBACKLIGHTOFF, SET_BITS );
		/* After all, planar_control only operates LCD backlight */
        }
        break;

      case PM_PLANAR_OFF :
	pm_HW_state_data.video_dd_request = PM_PLANAR_OFF;

        modify_pmc( INDEXPMC_PMCO0,
                           BITPMCPMCO0_LCDBACKLIGHTOFF, RESET_BITS );
        break;

      case PM_PLANAR_LOWPOWER1 :
	break;

      case PM_PLANAR_LOWPOWER2 :
        break;

      case PM_PLANAR_QUERY:
	return (PM_PLANAR_ON | PM_PLANAR_OFF);

      case PM_PLANAR_CURRENT_LEVEL:
        data = read_pmc( INDEXPMC_PMCO0 );
        if( ( data & BITPMCPMCO0_LCDBACKLIGHTOFF ) == 0 ) {
            return PM_PLANAR_OFF;
        } else {
            return PM_PLANAR_ON;
        }

      default :
        return PM_ERROR;
    }

    return PM_SUCCESS;
}

/******************************************************************************
 * NAME: pm_planar_control_crt
 *
 * FUNCTION: CRT control for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      devno
 *      devid  = PMDEV_CRT
 *      cmd    = PM_PLANAR_QUERY
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *               PM_PLANAR_LOWPOWER1
 *               PM_PLANAR_LOWPOWER2
 *		 PM_PLANAR_CURRENT_LEVEL
 *
 * RETURN VALUE:
 *      if (cmd = PM_PLANAR_QUERY)
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *               PM_PLANAR_LOWPOWER1
 *               PM_PLANAR_LOWPOWER2
 *	else if (cmd = PM_PLANAR_CURRENT_LEVEL)
 *		 the last setup level
 *      else
 *               PM_SUCCESS,  PM_ERROR
 */
int
pm_planar_control_crt( dev_t devno, int devid, int cmd )
{
    int     data;

    switch( cmd ){

      case PM_PLANAR_ON :
                    /*  Reject if system is in suspend(partial) state. */
        if ( pm_core_data.system_state == PM_SYSTEM_FULL_ON
          || pm_core_data.system_state == PM_SYSTEM_ENABLE
          || pm_core_data.system_state == PM_SYSTEM_STANDBY
          || pm_control_data.phase_1_only == FALSE ) {

            DBG(planar_control: CRT ON is done.)
            modify_pmc( INDEXPMC_PMCO1,
                           BITPMCPMCO1_CRTON,       SET_BITS );
        }
        break;

      case PM_PLANAR_OFF :
	DBG(planar_control: CRT OFF is done.)
        modify_pmc( INDEXPMC_PMCO1,
                           BITPMCPMCO1_CRTLOWPOWER, RESET_BITS );
        break;

      case PM_PLANAR_LOWPOWER1 :
	DBG(planar_control: CRT LOWPOWER1 is done.)
        modify_pmc( INDEXPMC_PMCO1,
                        BITPMCPMCO1_CRTLOWPOWER, RESET_BITS );
        modify_pmc( INDEXPMC_PMCO1,
                        BITPMCPMCO1_CRTSUSPEND,  SET_BITS );
        break;

      case PM_PLANAR_LOWPOWER2 :
	DBG(planar_control: CRT LOWPOWER2 is done.)
        modify_pmc( INDEXPMC_PMCO1,
                        BITPMCPMCO1_CRTLOWPOWER, RESET_BITS );
        modify_pmc( INDEXPMC_PMCO1,
                        BITPMCPMCO1_CRTSTANDBY,  SET_BITS );
        break;

      case PM_PLANAR_QUERY:
	return (PM_PLANAR_ON | 
		PM_PLANAR_OFF | 
		PM_PLANAR_LOWPOWER1 | 
		PM_PLANAR_LOWPOWER2);

      case PM_PLANAR_CURRENT_LEVEL:
        data = read_pmc( INDEXPMC_PMCO1 );
        data &= BITPMCPMCO1_CRTLOWPOWER ;
        switch( data ){
          case BITPMCPMCO1_CRTOFF :
            return PM_PLANAR_OFF;
          case BITPMCPMCO1_CRTSUSPEND :
            return PM_PLANAR_LOWPOWER1;
          case BITPMCPMCO1_CRTSTANDBY :
            return PM_PLANAR_LOWPOWER2;
          case BITPMCPMCO1_CRTON :
            return PM_PLANAR_ON;
          default :
            return PM_ERROR;
        }
        break;

      default :
        return PM_ERROR;
    }

    return PM_SUCCESS;
}


/******************************************************************************
 * NAME: pm_planar_control_gcc
 *
 * FUNCTION: Graphic Controller control for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      devno
 *      devid  = PMDEV_GCC
 *      cmd    = PM_PLANAR_QUERY
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *               PM_PLANAR_LOWPOWER1 ( no action )
 *               PM_PLANAR_LOWPOWER2 ( no action )
 *		 PM_PLANAR_CURRENT_LEVEL
 *
 * RETURN VALUE:
 *      if (cmd = PM_PLANAR_QUERY)
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *	else if (cmd = PM_PLANAR_CURRENT_LEVEL)
 *		 the last setup level
 *      else
 *               PM_SUCCESS,  PM_ERROR
 */
int
pm_planar_control_gcc( dev_t devno, int devid, int cmd )
{
    int     data;

    switch( cmd ){

      case PM_PLANAR_ON :
        modify_pmc( INDEXPMC_PMCO0,
                           BITPMCPMCO0_VIDEOLOWPOWER, SET_BITS );
        break;

      case PM_PLANAR_OFF :
	/* In Woodfield, assertion of GCC low power(system idle mode)
           causes a trouble. But graphisc DD still calls this planar_
	   control according to the design for future expandability.
 	   Hence, in Woodfield, this feature results in being nop. */
        break;

      case PM_PLANAR_LOWPOWER1 :
        modify_pmc( INDEXPMC_PMCO0,
                           BITPMCPMCO0_VIDEOLOWPOWER, RESET_BITS );
        break;

      case PM_PLANAR_LOWPOWER2 :
        modify_pmc( INDEXPMC_PMCO0,
                           BITPMCPMCO0_VIDEOLOWPOWER, RESET_BITS );
        break;

      case PM_PLANAR_QUERY:
	return (PM_PLANAR_ON | 
		PM_PLANAR_OFF );

      case PM_PLANAR_CURRENT_LEVEL:
        data = read_pmc( INDEXPMC_PMCO0 );
        if( ( data & BITPMCPMCO0_VIDEOLOWPOWER ) == 0 ) {
            return PM_PLANAR_OFF;
        } else {
            return PM_PLANAR_ON;
        }

      default :
        return PM_ERROR;
    }

    return PM_SUCCESS;
}


/******************************************************************************
 * NAME: pm_planar_control_camera
 *
 * FUNCTION: CCD Camera control for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      devno
 *      devid  = PMDEV_CAMERA
 *      cmd    = PM_PLANAR_QUERY
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *               PM_PLANAR_LOWPOWER1 ( no action )
 *               PM_PLANAR_LOWPOWER2 ( no action )
 *		 PM_PLANAR_CURRENT_LEVEL
 *
 * RETURN VALUE:
 *      if (cmd = PM_PLANAR_QUERY)
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *	else if (cmd = PM_PLANAR_CURRENT_LEVEL)
 *		 the last setup level
 *      else
 *               PM_SUCCESS,  PM_ERROR
 */
int
pm_planar_control_camera( dev_t devno, int devid, int cmd )
{
    int     data;

    switch( cmd ){

      case PM_PLANAR_ON :
        modify_pmc( INDEXPMC_PMCO0,
                           BITPMCPMCO0_CAMERAOFF, SET_BITS );
        break;

      case PM_PLANAR_OFF :
        modify_pmc( INDEXPMC_PMCO0,
                           BITPMCPMCO0_CAMERAOFF, RESET_BITS );
        break;

      case PM_PLANAR_LOWPOWER1 :
        break;

      case PM_PLANAR_LOWPOWER2 :
        break;

      case PM_PLANAR_QUERY:
	return (PM_PLANAR_ON | 
		PM_PLANAR_OFF );

      case PM_PLANAR_CURRENT_LEVEL:
        data = read_pmc( INDEXPMC_PMCO0 );
        if( ( data & BITPMCPMCO0_CAMERAOFF ) == 0 ) {
            return PM_PLANAR_OFF;
        } else {
            return PM_PLANAR_ON;
        }

      default :
        return PM_ERROR;
    }

    return PM_SUCCESS;
}


/******************************************************************************
 * NAME: pm_planar_control_audio
 *
 * FUNCTION: Audio control for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      devno
 *      devid  = PMDEV_AUDIO
 *      cmd    = PM_PLANAR_QUERY
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *               PM_PLANAR_LOWPOWER1 ( no action )
 *               PM_PLANAR_LOWPOWER2 ( no action )
 *		 PM_PLANAR_CURRENT_LEVEL
 *
 * RETURN VALUE:
 *      if (cmd = PM_PLANAR_QUERY)
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *	else if (cmd = PM_PLANAR_CURRENT_LEVEL)
 *		 the last setup level
 *      else
 *               PM_SUCCESS,  PM_ERROR
 */
int
pm_planar_control_audio( dev_t devno, int devid, int cmd )
{
    int     data;

    switch( cmd ){

      case PM_PLANAR_ON :
        modify_pmc( INDEXPMC_PMCO1,
                           BITPMCPMCO1_AUDIOLOWPOWER, SET_BITS );
	DBG(Set Audio chip into normal mode)
        break;

      case PM_PLANAR_OFF :
	if (!((pm_control_data.system_state == PM_SYSTEM_SUSPEND) ||
	      (pm_control_data.system_state == PM_SYSTEM_HIBERNATION))) { 
	
        	modify_pmc( INDEXPMC_PMCO1,
                           BITPMCPMCO1_AUDIOLOWPOWER, RESET_BITS );
	} /* endif */
	DBG(Set Audio chip into low power mode)
        break;

      case PM_PLANAR_LOWPOWER1 :
        break;

      case PM_PLANAR_LOWPOWER2 :
        break;

      case PM_PLANAR_QUERY:
	return (PM_PLANAR_ON | 
		PM_PLANAR_OFF );

      case PM_PLANAR_CURRENT_LEVEL:
        data = read_pmc( INDEXPMC_PMCO1 );
        if( ( data & BITPMCPMCO1_AUDIOLOWPOWER ) == 0 ) {
            return PM_PLANAR_OFF;
        } else {
            return PM_PLANAR_ON;
        }

      default :
        return PM_ERROR;
    }

    return PM_SUCCESS;
}


/******************************************************************************
 * NAME: pm_planar_control_audio_mute
 *
 * FUNCTION: Audio mute external control for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      devno
 *      devid  = PMDEV_AUDIO
 *      cmd    = PM_PLANAR_QUERY
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *               PM_PLANAR_LOWPOWER1 ( no action )
 *               PM_PLANAR_LOWPOWER2 ( no action )
 *		 PM_PLANAR_CURRENT_LEVEL
 *
 * RETURN VALUE:
 *      if (cmd = PM_PLANAR_QUERY)
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *	else if (cmd = PM_PLANAR_CURRENT_LEVEL)
 *		 the last setup level
 *      else
 *               PM_SUCCESS,  PM_ERROR
 */
int
pm_planar_control_audio_mute( dev_t devno, int devid, int cmd )
{
    int     data;

    switch( cmd ){

      case PM_PLANAR_ON :
        modify_isadev( PORTXIOC_AUDIOSUPPORT,
                           BITXIOCAUDIOSUPPORT_SPEAKERMUTE, RESET_BITS );
	DBG(Speaker mute is set off)
        break;

      case PM_PLANAR_OFF :
        modify_isadev( PORTXIOC_AUDIOSUPPORT,
                           BITXIOCAUDIOSUPPORT_SPEAKERMUTE, SET_BITS );
	DBG(Speaker mute is set on)
        break;

      case PM_PLANAR_LOWPOWER1 :
        break;

      case PM_PLANAR_LOWPOWER2 :
        break;

      case PM_PLANAR_QUERY:
	return (PM_PLANAR_ON | 
		PM_PLANAR_OFF );

      case PM_PLANAR_CURRENT_LEVEL:
	data = read_isadev(PORTXIOC_AUDIOSUPPORT);
        if( ( data & BITXIOCAUDIOSUPPORT_SPEAKERMUTE ) == 1 ) {
            return PM_PLANAR_OFF;
        } else {
            return PM_PLANAR_ON;
        }

      default :
        return PM_ERROR;
    }

    return PM_SUCCESS;
}


/******************************************************************************
 * NAME: pm_planar_control_cpu
 *
 * FUNCTION: CPU power control for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      devno
 *      devid  = PMDEV_CPU
 *      cmd    = PM_PLANAR_QUERY
 *               PM_PLANAR_ON        ( no action )
 *               PM_PLANAR_OFF       ( no action )
 *               PM_PLANAR_LOWPOWER1
 *               PM_PLANAR_LOWPOWER2 ( no action )
 *		 PM_PLANAR_CURRENT_LEVEL
 *
 * RETURN VALUE:
 *      if (cmd = PM_PLANAR_QUERY || PM_PLANAR_CURRENT_LEVEL)
 *               PM_PLANAR_ON
 *      else
 *               PM_SUCCESS,  PM_ERROR
 */
int
pm_planar_control_cpu( dev_t devno, int devid, int cmd )
{
    int     data;

    switch( cmd ){

      case PM_PLANAR_ON :
        break;

      case PM_PLANAR_OFF :
        break;

      case PM_PLANAR_LOWPOWER1 :
        /* Set doze mode */
        SetHid0Power(BITPOWERMODE_DOZE);

        do {
           SetMsrPow();
        } while(ReadMsrPow() == POWERMGT_ON); /* enddo */

        SetHid0Power(0);
        break;

      case PM_PLANAR_LOWPOWER2 :
        break;

      case PM_PLANAR_QUERY:
	return ( PM_PLANAR_LOWPOWER1 );

      case PM_PLANAR_CURRENT_LEVEL:
        return PM_PLANAR_ON;

      default :
        return PM_ERROR;
    }

    return PM_SUCCESS;
}


/******************************************************************************
 * NAME: pm_planar_control_psusus
 *
 * FUNCTION: Suspend power control for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      devno
 *      devid  = PMDEV_PSUSUS
 *      cmd    = PM_PLANAR_QUERY
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *               PM_PLANAR_LOWPOWER1 ( no action )
 *               PM_PLANAR_LOWPOWER2 ( no action )
 *		 PM_PLANAR_CURRENT_LEVEL
 *
 * RETURN VALUE:
 *      if (cmd = PM_PLANAR_QUERY)
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *	else if (cmd = PM_PLANAR_CURRENT_LEVEL)
 *		 the last setup level
 *      else
 *               PM_SUCCESS,  PM_ERROR
 */
int
pm_planar_control_psusus( dev_t devno, int devid, int cmd )
{
    int     data;

    switch( cmd ){

      case PM_PLANAR_ON :
      case PM_PLANAR_OFF :
      case PM_PLANAR_LOWPOWER1 :
      case PM_PLANAR_LOWPOWER2 :
        break;

      case PM_PLANAR_QUERY:
	return (0);		
	/*  In phase 2, pseudo device: psusus is not supported at all. */

      case PM_PLANAR_CURRENT_LEVEL:
        return PM_PLANAR_ON;

      default :
        return PM_ERROR;
    }

    return PM_SUCCESS;
}


/******************************************************************************
 * NAME: pm_planar_control_cdrom
 *
 * FUNCTION: SCSI terminator control for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      devno
 *      devid  = PMDEV_CDROM
 *      cmd    = PM_PLANAR_QUERY
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *               PM_PLANAR_LOWPOWER1 ( no action )
 *               PM_PLANAR_LOWPOWER2 ( no action )
 *		 PM_PLANAR_CURRENT_LEVEL
 *
 * RETURN VALUE:
 *      if (cmd = PM_PLANAR_QUERY)
 *               PM_PLANAR_ON
 *               PM_PLANAR_OFF
 *	else if (cmd = PM_PLANAR_CURRENT_LEVEL)
 *		 the last setup level
 *      else
 *               PM_SUCCESS,  PM_ERROR
 */
int
pm_planar_control_cdrom( dev_t devno, int devid, int cmd )
{
    int     data;

    switch( cmd ){

      case PM_PLANAR_ON :
        modify_pmc( INDEXPMC_PMCO1,
                           BITPMCPMCO1_CDROMDRIVELOWPOWER, SET_BITS );
        break;

      case PM_PLANAR_OFF :
        modify_pmc( INDEXPMC_PMCO1,
                           BITPMCPMCO1_CDROMDRIVELOWPOWER, RESET_BITS );
        break;

      case PM_PLANAR_LOWPOWER1 :
        break;

      case PM_PLANAR_LOWPOWER2 :
        break;

      case PM_PLANAR_QUERY:
	return (PM_PLANAR_ON | 
		PM_PLANAR_OFF );

      case PM_PLANAR_CURRENT_LEVEL:
        data = read_pmc( INDEXPMC_PMCO1 );
        if( ( data & BITPMCPMCO1_CDROMDRIVELOWPOWER ) == 0 ) {
            return PM_PLANAR_OFF;
        } else {
            return PM_PLANAR_ON;
        }

      default :
        return PM_ERROR;
    }

    return PM_SUCCESS;
}


/******************************************************************************
 * NAME: cpu_idle
 *
 * FUNCTION: Make CPU idle
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      none
 *
 * RETURN VALUE:
 *      none
 */
void
cpu_idle()
{
    /*  Reject if system is in FULL ON state. */
    if (pm_control_data.phase_1_only == FALSE) {
	
	if ( pm_control_data.system_state == PM_SYSTEM_FULL_ON ) {
		return ;
	}

    } else {

    	if ( pm_core_data.system_state == PM_SYSTEM_FULL_ON ) {
      		return ;
    	}
    } 

    pm_planar_control_cpu(0,0, PM_PLANAR_LOWPOWER1) ;
    return ;
}


/******************************************************************************
 * NAME: turn_power_off
 *
 * FUNCTION: Turn off main power
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      none
 *
 * RETURN VALUE:
 *      none
 */
void
turn_power_off()
{
    pm_planar_control_psumain(0,0, PM_PLANAR_OFF) ;
    return ;	/* never come here */
}
