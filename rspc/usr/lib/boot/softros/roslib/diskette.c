static char sccsid[] = "@(#)20	1.1  src/rspc/usr/lib/boot/softros/roslib/diskette.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:38:57";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 * FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

 */
#ifdef DEBUG	/* only include diskette support if DEBUG on */
#include <sys/types.h>
#include <misc_sub.h>
#include <int_hndlr.h>
#include <diskette.h>
#include <io_sub.h>

#ifdef DSKT_DEBUG
#include <printf.h>
#define  PRINTF(x, y)    printf(x, y)
#else
#define  PRINTF(x, y)    ;
#endif

#define  inportb(x)      inb(x)
#define  outportb(x, y)  outb(x, y)

int      detdata(CB_PTR, unsigned char *, int, int, int, int, int, int);
int      detmedia(CB_PTR, int);
int      dskt_present(CB_PTR);
int      specify(CB_PTR, int);
int      dskt_config(CB_PTR);
int      seek_or_recal(CB_PTR, int, int, int);
int      read_id(CB_PTR, int);
int      set_mode(CB_PTR, int);
int      issue_read_floppy_cmd(CB_PTR,
                               unsigned char,
                               unsigned char,
                               unsigned char,
                               unsigned char,
                               unsigned char,
                               unsigned char,
                               unsigned char,
                               unsigned char,
                               int);
int      issue_verify_cmd(CB_PTR,
                          unsigned char,
                          unsigned char,
                          unsigned char,
                          unsigned char,
                          unsigned char,
                          unsigned char,
                          unsigned char,
                          unsigned char);
int      read_floppy_cmd_check(CB_PTR);
int      wait_to_write_first_cmd_byte(CB_PTR);
int      write_first_fifo_cmd(CB_PTR, unsigned char);
int      wait_to_write_cmd_byte(CB_PTR);
int      write_fifo_cmd(CB_PTR, unsigned char);
int      wait_for_result_byte(CB_PTR);
int      read_fifo_result(CB_PTR, unsigned char *);
int      sens_intr(CB_PTR, unsigned char *, unsigned char *);
int      wait_int(CB_PTR);
void     reset_fdc(CB_PTR, int);
void     start_motor_and_wait(CB_PTR, int);
void     dskt_stop_motor(CB_PTR);
void     setup_globals(CB_PTR, int, int);
unsigned int dskt_int_hdlr(void);

#ifdef CC_CARD
int  dma_check(CB_PTR, ulong);
void dma_setup(CB_PTR, unsigned char *, int, ulong);
void iocc_or_eq_8(CB_PTR, unsigned int, unsigned char);
void iocc_an_eq_8(CB_PTR, unsigned int, unsigned char);
#else
int      dma_check(CB_PTR);
void     dma_setup(CB_PTR, unsigned char *, int, int);
#endif

extern void     rtc_dec(uint);
extern void     outb(unsigned int, unsigned char);
extern unsigned char inb(unsigned int);

/*-------------------------------*/
/* Global Diskette Control Block */
/*-------------------------------*/
DSKT_CB dskt_cb;

rtc_value rtc_timer;       // Timer value for CPU real-time clock

/*---------------------------------------------------------------------------*
 * NAME: diskette_init()                                                     *
 *                                                                           *
 * FUNCTION: Initializes the specified diskette drive.                       *
 *                                                                           *
 * EXECUTION ENVIRONMENT:                                                    *
 *                                                                           *
 * INPUTS :                                                                  *
 *          drive     : driver number to be initialized.                     *
 *                                                                           *
 * RETURNS:                                                                  *
 *          0 when successfully initialized, otherwise !0.                   *
 *---------------------------------------------------------------------------*/
int diskette_init(int drive)
{

    int     rc;
    CB_PTR  cb_ptr;

    PRINTF("diskette_init(%d) entered.\n", drive);

    rc                     = 0;
    cb_ptr                 = &dskt_cb;
    cb_ptr->g_int_expected = FALSE;

#ifdef CC_CARD
    /*--------------------*/
    /* Setup System Board */
    /*--------------------*/
    (void)iocc_an_eq_8(cb_ptr, SYS_BD_SET_UP,  (unsigned char)0x7f);

    /*-----------------*/
    /* Enable Diskette */
    /*-----------------*/
    (void)iocc_or_eq_8(cb_ptr, NIO_POS_REG0+2, (unsigned char)0x03);

    /*---------------------*/
    /* Enable System Board */
    /*---------------------*/
    (void)iocc_or_eq_8(cb_ptr, SYS_BD_SET_UP,  (unsigned char)0x80);
#else
    /*--------------------------------------*/
    /* Set SIO GAP (Guaranteed Access Time) */
    /*--------------------------------------*/

// <rb> moved SIO stuff to SIOSETUP.C
//  SIO_PAC_REG = 0x01;

    /*----------------*/
    /* Reset DMA Chip */
    /*----------------*/
    outportb(DMC_REG, (unsigned char)0x00);
#endif

    /*----------------------------*/
    /* Register Interrupt Handler */
    /*----------------------------*/
#ifndef USE_POLLING
    rc = (int)ext_int_registrar((unsigned int)ASSIGN_INTERRUPT_HANDLER_ADDRESS,
                                (unsigned char)DSKT_INTERRUPT_LEVEL,
                                dskt_int_hdlr);
#else
    rc = GOOD_RC;        // Pretend there is an interrupt handler
#endif

    if (rc == GOOD_RC || rc == INT_TAKEN) {
        rc = detmedia(cb_ptr, drive);
        dskt_stop_motor(cb_ptr);
    }

    PRINTF("diskette_init() returns with rc = %d.\n", rc);

    return( rc );

}


/*---------------------------------------------------------------------------*
 * NAME: diskette_read()                                                     *
 *                                                                           *
 * FUNCTION: Reads data from the specified position.                         *
 *                                                                           *
 * EXECUTION ENVIRONMENT:                                                    *
 *                                                                           *
 *                                                                           *
 * INPUTS :                                                                  *
 *          cyl        : cylinder (track) number to be read.                 *
 *          head       : head number to be read.                             *
 *          sector     : sector number to be read.                           *
 *          num_of_sect: number of sectors to be read.                       *
 *          drive      : drive number to be read.                            *
 *          buf        : pointer to the buffer to store read data.           *
 *                                                                           *
 * RETURNS:                                                                  *
 *          0 when successfully initialized, else !0.                        *
 *---------------------------------------------------------------------------*/
int diskette_read(int        cyl,
                  int        head,
                  int        sector,
                  int        num_of_sect,
                  int        drive,
                  char       *buf)
{

    int        rc;
    CB_PTR     cb_ptr;


    rc     = 0;
    cb_ptr = &dskt_cb;

    if ( cb_ptr->fdc[drive].g_media_drive_flag == UNKNOWN )
        rc = detmedia(cb_ptr, drive);

    if ( rc == OK )
        rc = detdata(cb_ptr, (unsigned char *)buf,
                     drive, cyl, head, sector, num_of_sect, READ);

    dskt_stop_motor(cb_ptr);

    PRINTF("diskette_read() returns with rc = %d.\n", rc);

    return( rc );
}

/*---------------------------------------------------------------------------*
 * NAME: diskette_read_RBA()                                                     *
 *                                                                           *
 * FUNCTION: Reads data from the specified position.                         *
 *                                                                           *
 * EXECUTION ENVIRONMENT:                                                    *
 *                                                                           *
 * INPUTS :                                                                  *
 *          RBA        : Relative Block Address, zero-based                  *
 *          count      : number of sectors to be read.                       *
 *          drive      : drive number to be read.                            *
 *          buf        : pointer to the buffer to store read data.           *
 *                                                                           *
 * RETURNS:                                                                  *
 *          0 when successfully initialized, else !0.                        *
 *---------------------------------------------------------------------------*/
int diskette_read_RBA(int RBA, int count, int drive, char *buf) {
    int        rc = 0;
    int c, h, s;
    int sectors_per_track;
    CB_PTR     cb_ptr = &dskt_cb;

    if ( cb_ptr->fdc[drive].g_media_drive_flag == UNKNOWN )
        rc = detmedia(cb_ptr, drive);

    sectors_per_track = cb_ptr->fdc[drive].g_max_sect;

    s = RBA % sectors_per_track;
    RBA -= s++;
    c = RBA / (sectors_per_track*2);
    RBA -= c * sectors_per_track * 2;
    if(RBA) h = 1; else h = 0;

//  printf("Diskette: RBA = %d C = %d H = %d S = %d\n",RBA,c,h,s);

    if ( rc == OK )
        rc = detdata(cb_ptr, (unsigned char *)buf,
                     drive, c, h, s, count, READ);

    dskt_stop_motor(cb_ptr);

    PRINTF("diskette_read_RBA() returns with rc = %d.\n", rc);

    return( rc );
}


/*---------------------------------------------------------------------------*
 * NAME: diskette_write()                                                    *
 *                                                                           *
 * FUNCTION: Writes data to the specified position.                          *
 *                                                                           *
 * EXECUTION ENVIRONMENT:                                                    *
 *                                                                           *
 *                                                                           *
 * INPUTS :                                                                  *
 *          cyl        : cylinder (track) number to be read.                 *
 *          head       : head number to be read.                             *
 *          sector     : sector number to be read.                           *
 *          num_of_sect: number of sectors to be read.                       *
 *          drive      : drive number to be read.                            *
 *          buf        : pointer to the buffer to store read data.           *
 *                                                                           *
 * RETURNS:                                                                  *
 *          0 when successfully initialized, else !0.                        *
 *---------------------------------------------------------------------------*/
int diskette_write(int        cyl,
                   int        head,
                   int        sector,
                   int        num_of_sect,
                   int        drive,
                   char       *buf)
{

    int        rc;
    CB_PTR     cb_ptr;


    rc     = 0;
    cb_ptr = &dskt_cb;

    if ( cb_ptr->fdc[drive].g_media_drive_flag == UNKNOWN )
        rc = detmedia(cb_ptr, drive);

    if ( rc == OK )
        rc = detdata(cb_ptr, (unsigned char *)buf,
                     drive, cyl, head, sector, num_of_sect, WRITE);

    dskt_stop_motor(cb_ptr);

    PRINTF("diskette_write() returns with rc = %d.\n", rc);

    return( rc );
}

/*---------------------------------------------------------------------------*
 * NAME: diskette_write()                                                    *
 *                                                                           *
 * FUNCTION: Writes data to the specified position.                          *
 *                                                                           *
 * EXECUTION ENVIRONMENT:                                                    *
 *                                                                           *
 *                                                                           *
 * INPUTS :                                                                  *
 *          cyl        : cylinder (track) number to be read.                 *
 *          head       : head number to be read.                             *
 *          sector     : sector number to be read.                           *
 *          num_of_sect: number of sectors to be read.                       *
 *          drive      : drive number to be read.                            *
 *          buf        : pointer to the buffer to store read data.           *
 *                                                                           *
 * RETURNS:                                                                  *
 *          0 when successfully initialized, else !0.                        *
 *---------------------------------------------------------------------------*/
int diskette_write_RBA(int RBA, int count, int drive, char *buf) {
    int        rc = 0;
    int c, h, s;
    int sectors_per_track;
    CB_PTR     cb_ptr = &dskt_cb;

    if ( cb_ptr->fdc[drive].g_media_drive_flag == UNKNOWN )
        rc = detmedia(cb_ptr, drive);

    sectors_per_track = cb_ptr->fdc[drive].g_max_sect;

    s = RBA % sectors_per_track;
    RBA -= s++;
    c = RBA / (sectors_per_track*2);
    RBA -= c * sectors_per_track * 2;
    if(RBA) h = 1; else h = 0;

//  printf("Diskette: RBA = %d C = %d H = %d S = %d\n",RBA,c,h,s);

    if ( rc == OK )
        rc = detdata(cb_ptr, (unsigned char *)buf,
                     drive, c, h, s, count, WRITE);

    dskt_stop_motor(cb_ptr);

    PRINTF("diskette_write_RBA() returns with rc = %d.\n", rc);

    return( rc );
}


/*----------------------------------------------------------------------------*
 * NAME: detdata()                                                            *
 *                                                                            *
 * FUNCTION: Writes data to the specified position.                           *
 *           Called by diskette_read()/diskette_write().                      *
 * EXECUTION ENVIRONMENT:                                                     *
 *                                                                            *
 * (NOTES:) This routine reads the data from the diskette and stores the data *
 *      at a storage location that is passed as a parm. The floppy disk parms *
 *      used for the read are those that were determined via a call to the    *
 *      detmedia() routine                                                    *
 *                                                                            *
 * RETURNS: 0 when data is successfully read, otherwise !0.                   *
 *                                                                            *
 *----------------------------------------------------------------------------*/
int detdata(CB_PTR        cb_ptr,
            unsigned char *data_ptr,
            int           drive_num,
            int           cyl_num,
            int           head_num,
            int           sect_num,
            int           num_of_sect,
            int           rw_flag)
{
    int                  rc;
    int                  max_num;
    int                  data_count;
    int                  loop_count;
    int                  num_to_read;


    PRINTF("detdata() entered.\n", "");

    rc     = OK;

    /*------------------------------------------*/
    /* Sanity check on some input parameters.   */
    /* cb_ptr values have already been set      */
    /* by setup_globals() called by detmedia(). */
    /*------------------------------------------*/
    if (cyl_num  > cb_ptr->fdc[drive_num].g_max_cyl   ||
        head_num > cb_ptr->fdc[drive_num].g_max_head  ||
        sect_num > cb_ptr->fdc[drive_num].g_max_sect)
        return( NO ) ;

    for ( ; rc == OK && num_of_sect > 0; ++cyl_num ) {

        loop_count = 0;

        do {
            /*----------------------------*/
            /* up to three passes of this */
            /* loop can occur             */
            /*----------------------------*/

            rc          = OK;
            max_num     = ( 2 - head_num) * cb_ptr->fdc[drive_num].g_max_sect -
                sect_num + 1;
            num_to_read = (max_num < num_of_sect) ? max_num : num_of_sect;
            data_count  = num_to_read * BYTES_PER_SECT;

            if ( cyl_num != cb_ptr->g_cyl[drive_num] )
                rc = seek_or_recal(cb_ptr, SEEK_CMD, drive_num, cyl_num);
            else if (cb_ptr->g_motor_on[drive_num] == FALSE)
                start_motor_and_wait(cb_ptr, drive_num);

            if (dskt_present(cb_ptr) == NO)
            {
               rc = 8;
               cb_ptr->fdc[drive_num].g_media_drive_flag = UNKNOWN;
               PRINTF("media changed.\n","");
               break;
            }

            if ( rc == OK ) {

                cb_ptr->g_int_flag = NO_INT;
                /*------------------------------------------*/
                /* set up the fdc for a dma read operation  */
                /*------------------------------------------*/
#ifdef CC_CARD
                dma_setup(cb_ptr, data_ptr, data_count, DMA_CHAN);
#else
                dma_setup(cb_ptr, data_ptr, data_count, rw_flag);
#endif
                /*--------------------------*/
                /* Issue READ/WRITE Command */
                /*--------------------------*/
                cb_ptr->g_int_expected = TRUE;
                rc = issue_read_floppy_cmd(cb_ptr,
                           (unsigned char)(drive_num + head_num * 4),
                           (unsigned char)cyl_num,
                           (unsigned char)head_num,
                           (unsigned char)sect_num,
                           READ_DATA_CMD_BYTES_PER_SECT,
                           (unsigned char)cb_ptr->fdc[drive_num].g_max_sect,
                           (unsigned char)cb_ptr->fdc[drive_num].g_gap_length,
                           (unsigned char)0xff,
                           (int)rw_flag);
            }

            if ( rc == OK ) {

                /*----------------------------*/
                /* Wait for interrupt occurs. */
                /* Result Check for READ CMD. */
                /* Check DMA.                 */
                /*----------------------------*/
                if (wait_int(cb_ptr)              == OK &&
                    read_floppy_cmd_check(cb_ptr) == OK &&
#ifdef CC_CARD
                    dma_check(cb_ptr, DMA_CHAN)   == OK ) {
#else
                    dma_check(cb_ptr)             == OK ) {
#endif
                    /*--------------------------------*/
                    /* DMA Successful.                */
                    /* Prepare for the next transfer. */
                    /*--------------------------------*/
                    num_of_sect -= num_to_read;
                    data_ptr    += data_count;
                    head_num     = 0;
                    sect_num     = 1;
                    rc           = OK;

                }
                else {

                    (void)specify(cb_ptr, drive_num);
                    (void)seek_or_recal(cb_ptr, RECAL_CMD, drive_num, 0);
                    rc = NO;

                }
            }

            ++loop_count;

        }  while ( rc != OK && loop_count < 3 );

    }

    PRINTF("detdata() returns with rc = %d.\n", rc);

    return (rc);
}


/*
 * NAME: detmedia()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This routine determines the type of drive that is present and the
 *      type of media that is present in the drive. That is, this routine
 *      determines if the drive is a 3.5 or 5.25 and if the diskette is a
 *      360K 1.2MB etc. The floppy disk controller is set up to try a read
 *      using various drive/media parameters until a successful read occurs.
 *      These parameters are saved globally and used for calls to the read
 *      routine.
 *
 *
 * RETURNS:
 */
int detmedia(CB_PTR cb_ptr, int drive_num)
{
    int                  rc;
    int                  loop_count;
    int                  media_drive_flag;


    PRINTF("detmedia(%d) entered.\n", drive_num);

    for (media_drive_flag = 1;
         media_drive_flag <= NUM_MED_DR_COMB;
         media_drive_flag++) {

        /*-----------------------------*/
        /* Setup parameters required   */
        /* to read the diskette.       */
        /*-----------------------------*/
        setup_globals(cb_ptr, media_drive_flag, drive_num);

        /*----------------------------------*/
        /* Set Data Rate to FDC CC Register */
        /*----------------------------------*/
        outportb(CCR, (unsigned char)cb_ptr->fdc[drive_num].g_data_rate);

        loop_count = 0;
        do {
            /*----------------------------*/
            /* up to three passes of this */
            /* loop can occur             */
            /*----------------------------*/

            rc = 0;

            /*---------------------*/
            /* Reset and CONFIGURE */
            /*---------------------*/

            reset_fdc(cb_ptr, (0x10 << drive_num) + drive_num);

            if (cb_ptr->g_motor_on[drive_num] == FALSE)
                start_motor_and_wait(cb_ptr, drive_num);

            /*---------*/
            /* SPECIFY */
            /*---------*/
            (void)specify(cb_ptr, drive_num);

            /*---------------*/
            /* PERPENDICULAR */
            /*---------------*/
            if ( set_mode(cb_ptr, drive_num) != OK )
                rc = SET_MODE_FAILED;
            else
                if ( seek_or_recal(cb_ptr, RECAL_CMD, drive_num, 0) != OK )
                    rc = RECAL_FAILED;
                else {
                    if ( media_drive_flag == 1 ) {
                        if (seek_or_recal(cb_ptr, SEEK_CMD,
                                          drive_num, 1 ) != OK )
                            rc = SEEK_FAILED;
                        else
                            if ( (rc = dskt_present(cb_ptr)) == NO )
                                break;
                    }
                }

            if ( ( rc == OK ) && (read_id(cb_ptr, drive_num) != OK) )
                rc = READ_ID_FAILED;

            ++loop_count;

        }  while ( rc != OK && loop_count < 3 );

        if ( rc == OK ) {

            /*--------------------------------------*/
            /* Check the result of READ ID Command. */
            /*--------------------------------------*/
            if (((FDC_RESULT_0 & 0xc0) == 0) &&
                ( FDC_RESULT_3         == cb_ptr->g_cyl[drive_num]))
                /*------------------------------*/
                /* Successfully detected media. */
                /*------------------------------*/
                break;
            else
                rc = DONT_KNOW_MEDIA;
        }
        else
            if (rc != READ_ID_FAILED)
                break;
    }

    /*------------------------------------------------------------*/
    /* Distiguish 1.2in1.2 from 1.4in2.8 by seeking to sector 18. */
    /* Since 1.2in1.2 is max 15 sectors per track, it will fail.  */
    /*------------------------------------------------------------*/
    if (rc == OK && cb_ptr->fdc[drive_num].g_media_drive_flag == A_1p4_IN_2p8) {

        cb_ptr->g_int_flag     = NO_INT;
        cb_ptr->g_int_expected = TRUE;

        if(issue_verify_cmd(cb_ptr,
                            (unsigned char)drive_num,
                            (unsigned char)0,
                            (unsigned char)0,
                            (unsigned char)cb_ptr->fdc[drive_num].g_max_sect,
                            READ_DATA_CMD_BYTES_PER_SECT,
                            (unsigned char)cb_ptr->fdc[drive_num].g_max_sect,
                            (unsigned char)cb_ptr->fdc[drive_num].g_gap_length,
                            (unsigned char)0xff) != OK ) {
            cb_ptr->g_int_expected = FALSE;
            rc = DONT_KNOW_MEDIA;
        }
        else {
            (void)wait_int(cb_ptr);
            /*-----------------------------------------------------*/
            /* read_floppy_cmd_check() can be used for VERIFY cmd. */
            /* If failed, it must be 1.2in1.2.                     */
            /*-----------------------------------------------------*/
            if (read_floppy_cmd_check(cb_ptr) != OK)
                cb_ptr->fdc[drive_num].g_media_drive_flag = A_1p2_IN_1p2;
        }
    }

    dskt_stop_motor(cb_ptr);

    if ( rc != OK )
        cb_ptr->fdc[drive_num].g_media_drive_flag = UNKNOWN;

    PRINTF("detmedia() returns with rc = %d.\n", rc);
    PRINTF("media flag = %d.\n", cb_ptr->fdc[drive_num].g_media_drive_flag);

    return (rc);

}


/*--------------------------------------------------------------------------*/
/* Function: dskt_present()                                                 */
/*                                                                          */
/* Action: Check if a diskette is present in the drive.                     */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
int dskt_present(CB_PTR cb_ptr)
{

    if ( inportb(DIR) & DISK_CHANGED_BIT )
        return( NO );
    else
        return( YES );

}


/*--------------------------------------------------------------------------*/
/* Function: specify()                                                      */
/*                                                                          */
/* Action:   Issue specify parameters to the diskette controller            */
/*                                                                          */
/* Notes:                                                                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
int specify(CB_PTR cb_ptr, int drive_num)
{


    if (write_first_fifo_cmd(cb_ptr, SPECIFY_CMD) == OK &&

        write_fifo_cmd(cb_ptr,
               (unsigned char)((cb_ptr->fdc[drive_num].g_step_rate << 4) +
               cb_ptr->fdc[drive_num].g_head_unload_time)) == OK &&

        write_fifo_cmd(cb_ptr, (unsigned char)
               ((cb_ptr->fdc[drive_num].g_head_load_time << 1) + DMA)) == OK)

        return( OK );

    else
        return( NO );

}


/*--------------------------------------------------------------------------*/
/* Function: dskt_config()                                                  */
/*                                                                          */
/* Action:                                                                  */
/*                                                                          */
/* Notes: fifo threshold 8 bytes                                            */
/*        precomp time to zero                                              */
/*--------------------------------------------------------------------------*/
int dskt_config(CB_PTR cb_ptr)
{

    if (write_first_fifo_cmd(cb_ptr, CONFIGURE_CMD) == OK &&
        write_fifo_cmd(cb_ptr, (unsigned char)0x00) == OK &&
        write_fifo_cmd(cb_ptr, (unsigned char)0x17) == OK &&
        write_fifo_cmd(cb_ptr, (unsigned char)0x00) == OK )
        return( OK );
    else
        return( NO );

}


/*--------------------------------------------------------------------------*/
/* Function: reset_fdc()                                                    */
/*                                                                          */
/* Action:   Reset controller chip                                          */
/*                                                                          */
/* Notes:    Does not stop motor if it is running                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void reset_fdc(CB_PTR cb_ptr, int reset_data)
{

    int           drive;
    unsigned char   sr0;
    unsigned char   ptr;


    /*------------------------------------------*/
    /* Indicate interrupt has not yet occurred. */
    /*------------------------------------------*/
    cb_ptr->g_int_flag = NO_INT;

    /*------------------------------------*/
    /* Invalidate current cylinder value. */
    /*------------------------------------*/
    for ( drive = 0; drive < NUM_OF_DRIVES; drive++ )
        cb_ptr->g_cyl[drive] = 0xff;

    /*-----------*/
    /* Reset FDC */
    /*-----------*/
    cb_ptr->g_int_expected = TRUE;
    outportb(DOR, (unsigned char)(0x08 + reset_data));
//    rtc_dec(RTC_1_u_SEC);

    rtc_timer = get_timeout_value(1, MICROSECONDS);
    while (timeout(rtc_timer) == FALSE) {
    } /* endwhile */

    outportb(DOR, (unsigned char)(0x0c + reset_data));
    outportb(CCR, (unsigned char)cb_ptr->fdc[reset_data & 0x03].g_data_rate);


    (void)wait_int(cb_ptr);


    /*-------------------------------------------------------------------*/
    /* The first sense interrupt has been done in the interrupt handler. */
    /* Do three more times here.                                         */
    /*-------------------------------------------------------------------*/
    for ( reset_data = 4; reset_data > 0; --reset_data ) {

        if ( sens_intr(cb_ptr, &sr0, &ptr) == YES ) {
            cb_ptr->g_index = 2;
            cb_ptr->g_fdc_result[0] = sr0;
            cb_ptr->g_fdc_result[1] = ptr;
        }
        else {
            cb_ptr->g_index         = 0;
            cb_ptr->g_fdc_result[0] = 0;
        }

        if (((cb_ptr->g_fdc_result[0] & 0x0f) == 0x03) ||
            ( cb_ptr->g_fdc_result[0]         == 0x80) )
            break;
    }

    (void)dskt_config(cb_ptr);

}


/*--------------------------------------------------------------------------*/
/* Function: start_motor_and_wait()                                         */
/*                                                                          */
/* ACTION: Starts Motor of the specified drive.                             */
/*                                                                          */
/* RETURNS: None.                                                           */
/*--------------------------------------------------------------------------*/
void start_motor_and_wait(CB_PTR cb_ptr, int drive_num)
{

    outportb(DOR, (unsigned char)((0x10 << drive_num) + 0x0c + drive_num));

    cb_ptr->g_motor_on[drive_num] = TRUE;

}


/*--------------------------------------------------------------------------*/
/* Function: dskt_stop_motor()                                              */
/*                                                                          */
/* ACTION: Stops all the motors.                                            */
/*                                                                          */
/* RETURNS: None.                                                           */
/*--------------------------------------------------------------------------*/
void dskt_stop_motor(CB_PTR cb_ptr)
{

    int  drive;

    outportb(DOR, (unsigned char)0x0c);

    for ( drive = 0; drive < NUM_OF_DRIVES; drive++ )
        cb_ptr->g_motor_on[drive] = FALSE;

}


/*--------------------------------------------------------------------------*/
/* FUNCTION: seek_or_recal()                                                */
/*                                                                          */
/* ACTION:   This procedure does a seek or a recalibrate command            */
/*                                                                          */
/* RETURNS:  various return codes (see code)                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/

int seek_or_recal(CB_PTR     cb_ptr,
                  int        command,
                  int        drive_num,
                  int        cyl_num)
{

    cb_ptr->g_cyl[drive_num] = cyl_num;

    if ( cb_ptr->g_motor_on[drive_num] == FALSE )
        start_motor_and_wait(cb_ptr, drive_num);

    cb_ptr->g_int_flag     = NO_INT;
    cb_ptr->g_int_expected = TRUE;

    if (write_first_fifo_cmd(cb_ptr, (unsigned char)command) == OK &&
        write_fifo_cmd(cb_ptr, (unsigned char)drive_num)     == OK &&
        (command == RECAL_CMD) ?
        1 : (write_fifo_cmd(cb_ptr,
                            (unsigned char)cb_ptr->g_cyl[drive_num])  == OK) ) {

        if( wait_int(cb_ptr) == OK )

            if ((FDC_RESULT_0 == 0x20 + drive_num) &&
                (FDC_RESULT_1 == cb_ptr->g_cyl[drive_num]))
                return( OK );

    }

    cb_ptr->g_cyl[drive_num] = 0xff;

    if( command == RECAL_CMD )
        return ( RECAL_FAILED );
    else
        return ( SEEK_FAILED );

}


/*--------------------------------------------------------------------------*/
/* Function: read_id()                                                      */
/*                                                                          */
/* ACTION: Issues READ ID Command.                                          */
/*                                                                          */
/* RETURNS: OK or NO                                                        */
/*--------------------------------------------------------------------------*/
int read_id(CB_PTR cb_ptr, int drive_num)
{

    cb_ptr->g_int_flag     = NO_INT;
    cb_ptr->g_int_expected = TRUE;

    if (write_first_fifo_cmd(cb_ptr, READ_ID_CMD)        == OK &&
        write_fifo_cmd(cb_ptr, (unsigned char)drive_num) == OK &&
        wait_int(cb_ptr) == OK) {

        if (cb_ptr->g_int_flag == GET_RESULTS && cb_ptr->g_index == 7)
            return ( OK );
    }
        return ( READ_ID_FAILED );

}


/*--------------------------------------------------------------------------*/
/* Function: setup_globals()                                                */
/*                                                                          */
/* ACTION: Setups diskette parameters.                                      */
/*                                                                          */
/* RETURNS: None.                                                           */
/*--------------------------------------------------------------------------*/
void setup_globals(CB_PTR          cb_ptr,
                   int             media_drive_flag,
                   int             drive_num)
{
    switch ( media_drive_flag ) {

      case A_2p8_IN_2p8:
        /*--------------------------------*/
        /* 2.88MB Media in a 2.88MB Drive */
        /*--------------------------------*/
        cb_ptr->fdc[drive_num].g_step_rate        = 0x0a;
        cb_ptr->fdc[drive_num].g_head_unload_time =    1;
        cb_ptr->fdc[drive_num].g_head_load_time   =    1;
        cb_ptr->fdc[drive_num].g_gap_length       = 0x38;
        cb_ptr->fdc[drive_num].g_max_cyl          =   79;
        cb_ptr->fdc[drive_num].g_max_sect         =   36;
        cb_ptr->fdc[drive_num].g_max_head         =    1;
        cb_ptr->fdc[drive_num].g_data_rate        = 0x03;
        cb_ptr->fdc[drive_num].g_media_drive_flag = A_2p8_IN_2p8;
        break;

      case A_1p4_IN_2p8:
        /*--------------------------------*/
        /* 1.44MB Media in a 2.88MB Drive */
        /*--------------------------------*/
        cb_ptr->fdc[drive_num].g_step_rate        = 0x0d;
        cb_ptr->fdc[drive_num].g_head_unload_time =    1;
        cb_ptr->fdc[drive_num].g_head_load_time   =    1;
        cb_ptr->fdc[drive_num].g_gap_length       = 0x1b;
        cb_ptr->fdc[drive_num].g_max_cyl          =   79;
        cb_ptr->fdc[drive_num].g_max_sect         =   18;
        cb_ptr->fdc[drive_num].g_max_head         =    1;
        cb_ptr->fdc[drive_num].g_data_rate        = 0x00;
        cb_ptr->fdc[drive_num].g_media_drive_flag = A_1p4_IN_2p8;
        break;

      case A_720_IN_2p8:
        /*-------------------------------*/
        /* 720MB Media in a 2.88MB Drive */
        /*-------------------------------*/
        cb_ptr->fdc[drive_num].g_step_rate        = 0x0e;
        cb_ptr->fdc[drive_num].g_head_unload_time =    1;
        cb_ptr->fdc[drive_num].g_head_load_time   =    1;
        cb_ptr->fdc[drive_num].g_gap_length       = 0x2a;
        cb_ptr->fdc[drive_num].g_max_cyl          =   79;
        cb_ptr->fdc[drive_num].g_max_sect         =    9;
        cb_ptr->fdc[drive_num].g_max_head         =    1;
        cb_ptr->fdc[drive_num].g_data_rate        = 0x02;
        cb_ptr->fdc[drive_num].g_media_drive_flag = A_720_IN_2p8;
        break;

      case A_1p2_IN_1p2:
        /*------------------------------*/
        /* 1.2MB Media in a 1.2MB Drive */
        /*------------------------------*/
        cb_ptr->fdc[drive_num].g_step_rate        = 0x0d;
        cb_ptr->fdc[drive_num].g_head_unload_time =    1;
        cb_ptr->fdc[drive_num].g_head_load_time   =    1;
        cb_ptr->fdc[drive_num].g_gap_length       = 0x1b;
        cb_ptr->fdc[drive_num].g_max_cyl          =   79;
        cb_ptr->fdc[drive_num].g_max_sect         =   15;
        cb_ptr->fdc[drive_num].g_max_head         =    1;
        cb_ptr->fdc[drive_num].g_data_rate        = 0x00;
        cb_ptr->fdc[drive_num].g_media_drive_flag = A_1p2_IN_1p2;
        break;

#if 0
        /*------------------------*/
        /* This is not supported. */
        /*------------------------*/
      case A_360_IN_1p2:
        /*------------------------------*/
        /* 360MB Media in a 1.2MB Drive */
        /*------------------------------*/
        cb_ptr->fdc[drive_num].g_step_rate        = 0x0e;
        cb_ptr->fdc[drive_num].g_head_unload_time =    1;
        cb_ptr->fdc[drive_num].g_head_load_time   =    1;
        cb_ptr->fdc[drive_num].g_gap_length       = 0x2a;
        cb_ptr->fdc[drive_num].g_max_cyl          =   39;
        cb_ptr->fdc[drive_num].g_max_sect         =    9;
        cb_ptr->fdc[drive_num].g_max_head         =    0;
        cb_ptr->fdc[drive_num].g_data_rate        = 0x01;
        cb_ptr->fdc[drive_num].g_media_drive_flag = A_360_IN_1p2;
        break;
#endif

      default:
        break;
    }
}

/*--------------------------------------------------------------------------*/
/* Function: set_mode()                                                     */
/*                                                                          */
/* Action:                                                                  */
/*    Send the perpendicular mode command to chip.                          */
/*--------------------------------------------------------------------------*/
int set_mode(CB_PTR cb_ptr, int drive)
{

    if (write_first_fifo_cmd(cb_ptr, PERPENDICULAR_CMD)                 == OK &&
        write_fifo_cmd(cb_ptr, (unsigned char)(0x80 & (0x04 << drive))) == OK )
        return( OK );
    else
        return( SET_MODE_FAILED );

}


/*--------------------------------------------------------------------------*/
/* Function: wait_int()                                                     */
/*                                                                          */
/* Action:                                                                  */
/*                                                                          */
/* Notes:    Does not return until the interrupt occurs or a timeout        */
/*                                                                          */
/* Returns:  OK(0) on the right interrupt, else -1.                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef USE_POLLING
int wait_int(CB_PTR cb_ptr)
{

    int rc;
//    int counter;


//    counter = 2000;
//    do
//    {
//        rtc_dec(RTC_1_m_SEC);

//       rtc_timer = get_timeout_value(1, MILLISECONDS);
//       while (timeout(rtc_timer) == FALSE) {
//       } /* endwhile */
//    }
//    while ( (cb_ptr->g_int_flag == NO_INT) && (counter-- > 0) );

    rtc_timer = get_timeout_value(2, SECONDS);
    while ((timeout(rtc_timer) == FALSE) &&
           (cb_ptr->g_int_flag == NO_INT));

    if ( cb_ptr->g_int_flag == NO_INT )
        rc = INT_TIME_OUT;
    else
        rc = OK;

    return ( rc );
}
#else
int wait_int(CB_PTR cb_ptr)
{
    int rc;

    PRINTF("wait_int entered.\n","");

    rtc_timer = get_timeout_value(20, SECONDS);
    while ((timeout(rtc_timer) == FALSE) &&
           ((inportb(SRA) & 0x80) != 0x80));

    if ((inportb(SRA) & 0x80) != 0x80)
    {
       rc = INT_TIME_OUT;
       PRINTF("wait_int timed out.\n","");
    }
    else
       rc = dskt_int_hdlr();

    return(rc);
}
#endif

/*--------------------------------------------------------------------------*/
/* FUNCTION: issue_verify_cmd(unsigned char hd_dr,                          */
/*                            unsigned char cyl,                            */
/*                            unsigned char head,                           */
/*                            unsigned char sector,                         */
/*                            unsigned char bps,                            */
/*                            unsigned char eotsn,                          */
/*                            unsigned char igl,                            */
/*                            unsigned char term)                           */
/*                                                                          */
/* ACTION:   Issue READ DATA Command to FDC.                                */
/*                                                                          */
/* RETURNS:  0 on successfull completion                                    */
/*          -1 else                                                         */
/*--------------------------------------------------------------------------*/
int issue_verify_cmd(CB_PTR   cb_ptr,
                     unsigned char hd_dr,
                     unsigned char cyl,
                     unsigned char head,
                     unsigned char sector,
                     unsigned char bps,
                     unsigned char eotsn,
                     unsigned char igl,
                     unsigned char term)
{

    if (write_first_fifo_cmd(cb_ptr, VERIFY_CMD ) ||
        write_fifo_cmd(cb_ptr, hd_dr  ) ||
        write_fifo_cmd(cb_ptr, cyl    ) ||
        write_fifo_cmd(cb_ptr, head   ) ||
        write_fifo_cmd(cb_ptr, sector ) ||
        write_fifo_cmd(cb_ptr, bps    ) ||
        write_fifo_cmd(cb_ptr, eotsn  ) ||
        write_fifo_cmd(cb_ptr, igl    ) ||
        write_fifo_cmd(cb_ptr, term   ) )
        return( NO );
    else
        return( OK );

}


/*--------------------------------------------------------------------------*/
/* FUNCTION: issue_read_floppy_cmd(unsigned char hd_dr,                     */
/*                                 unsigned char cyl,                       */
/*                                 unsigned char head,                      */
/*                                 unsigned char sector,                    */
/*                                 unsigned char bps,                       */
/*                                 unsigned char eotsn,                     */
/*                                 unsigned char igl,                       */
/*                                 unsigned char term)                      */
/*                                                                          */
/* ACTION:   Issue READ DATA Command to FDC.                                */
/*                                                                          */
/* RETURNS:  0 on successfull completion                                    */
/*          -1 else                                                         */
/*--------------------------------------------------------------------------*/
int issue_read_floppy_cmd(CB_PTR    cb_ptr,
                          unsigned char hd_dr,
                          unsigned char cyl,
                          unsigned char head,
                          unsigned char sector,
                          unsigned char bps,
                          unsigned char eotsn,
                          unsigned char igl,
                          unsigned char term,
                          int           rw_flag)
{

    unsigned char cmd;

    switch ( rw_flag ) {
      case WRITE:
        cmd = WRITE_DATA_CMD;
        PRINTF("issue write command.\n", "");
        break;
      case READ:
        cmd = READ_DATA_CMD;
        PRINTF("issue read command.\n", "");
        break;
      default:
        return( NO );
    }

    if (write_first_fifo_cmd(cb_ptr, cmd) ||
        write_fifo_cmd(cb_ptr, hd_dr  ) ||
        write_fifo_cmd(cb_ptr, cyl    ) ||
        write_fifo_cmd(cb_ptr, head   ) ||
        write_fifo_cmd(cb_ptr, sector ) ||
        write_fifo_cmd(cb_ptr, bps    ) ||
        write_fifo_cmd(cb_ptr, eotsn  ) ||
        write_fifo_cmd(cb_ptr, igl    ) ||
        write_fifo_cmd(cb_ptr, term   ) )
        if ( rw_flag == READ )
            return( READ_CMD_FAILED );
        else
            return( WRITE_CMD_FAILED );
    else
        return( OK );

}


/*---------------------------------------------------------------------------*/
/* FUNCTION: read_floppy_cmd_check()                                         */
/*                                                                           */
/* ACTION:   Check the result bytes of READ DATA CMD.                        */
/*                                                                           */
/* RETURNS:  OK or NO                                                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int read_floppy_cmd_check(CB_PTR cb_ptr)
{

    if (cb_ptr->g_int_flag == GET_RESULTS &&
        cb_ptr->g_index    == 7           &&
        !(FDC_RESULT_0 & 0xc0))
        return ( OK );
    else {
        PRINTF("read_floppy_cmd_check error. st0=%d.\n", FDC_RESULT_0);
        return ( NO );
    }

}


/*---------------------------------------------------------------------------*/
/* FUNCTION:  wait_to-write_first_cmd_byte                                   */
/*                                                                           */
/* ACTION:    Wait until FDC is ready to write.                              */
/*                                                                           */
/* RETURNS:   OK or NO                                                       */
/*---------------------------------------------------------------------------*/
int wait_to_write_first_cmd_byte(CB_PTR cb_ptr)
{
    int i;
    int timeout_counter;
    int got_ready_status;


    got_ready_status = NO;
    timeout_counter  = 30000;

   rtc_timer = get_timeout_value(2, SECONDS);

//    for (i = 0; (i < timeout_counter) && (got_ready_status == NO); i++)
   while ( (timeout(rtc_timer) == FALSE) && (got_ready_status == NO) )
        /*--------------------------------*/
        /* Check if FDC is ready to write */
        /*--------------------------------*/
        if ( (inportb(MSR) & 0xf0) == 0x80) {
            got_ready_status = YES;
            break;
        }

   if (got_ready_status != YES)
      PRINTF("w1cb_return = %d\n", got_ready_status);

    return got_ready_status;
}


/*---------------------------------------------------------------------------*/
/* FUNCTION:  write_first_fifo_cmd()                                         */
/*                                                                           */
/* ACTION:                                                                   */
/*                                                                           */
/* RETURNS:   OK or NO                                                       */
/*---------------------------------------------------------------------------*/
int write_first_fifo_cmd(CB_PTR cb_ptr, unsigned char value)
{

    if ( wait_to_write_first_cmd_byte(cb_ptr) == YES ) {
        outportb(FIFO, value);
        return( OK );
    }
    else
        return( NO );

}


/*---------------------------------------------------------------------------*/
/* FUNCTION:  wait_to_write_cmd_byte()                                       */
/*                                                                           */
/* ACTION:                                                                   */
/*                                                                           */
/* RETURNS:   OK or NO                                                       */
/*---------------------------------------------------------------------------*/
int wait_to_write_cmd_byte(CB_PTR cb_ptr)
{
    int i;
    int timeout_counter;
    int got_ready_status;


    got_ready_status = NO;
    timeout_counter  = 30000;

   rtc_timer = get_timeout_value(2, SECONDS);

//    for (i = 0; (i < timeout_counter) && (got_ready_status == NO); i++)
   while ( (timeout(rtc_timer) == FALSE) && (got_ready_status == NO) )
        /*------------------------------------------------------------*/
        /* Check if FDC is in ready to write and command in progress. */
        /*------------------------------------------------------------*/
        if ( (inportb(MSR) & 0xf0) == 0x90) {
            got_ready_status = YES;
            break;
        }

   if (got_ready_status != YES)
      PRINTF("wc_return = %d\n", got_ready_status);

    return got_ready_status;
}


/*---------------------------------------------------------------------------*/
/* FUNCTION:  write_fifo_cmd()                                               */
/*                                                                           */
/* ACTION:                                                                   */
/*                                                                           */
/* RETURNS:   OK or NO                                                       */
/*---------------------------------------------------------------------------*/
int write_fifo_cmd(CB_PTR cb_ptr, unsigned char value )
{

    if ( wait_to_write_cmd_byte(cb_ptr) == YES ) {
        outportb(FIFO, value);
        return( OK );
    }
    else
        return( NO );

}


/*---------------------------------------------------------------------------*/
/* FUNCTION:  wait_for_result_byte()                                         */
/*                                                                           */
/* ACTION:                                                                   */
/*                                                                           */
/* RETURNS:   OK or NO                                                       */
/*---------------------------------------------------------------------------*/
int wait_for_result_byte(CB_PTR cb_ptr)
{
    int i;
    int timeout_counter;
    int got_ready_status;


    got_ready_status = NO;
    timeout_counter  = 30000;

    if ( (inportb(MSR) & 0x10) == 0)
       got_ready_status = 2;

   rtc_timer = get_timeout_value(50, MILLISECONDS);

//    for (i = 0; (i < timeout_counter) && (got_ready_status != YES); i++)
   while ( (timeout(rtc_timer) == FALSE) && (got_ready_status != YES) )
        /*----------------------------------------------------------*/
        /* Check if FDC is in ready to read and command in progress */
        /*----------------------------------------------------------*/
        if ( (inportb(MSR) & 0xf0) == 0xd0) {
            got_ready_status = YES;
            break;
        }

   if (got_ready_status != YES)
      PRINTF("rb_return = %d\n", got_ready_status);

    return got_ready_status;
}


/*---------------------------------------------------------------------------*/
/* FUNCTION:  read_fifo_result()                                             */
/*                                                                           */
/* ACTION:                                                                   */
/*                                                                           */
/* RETURNS:   OK or NO                                                       */
/*---------------------------------------------------------------------------*/
int read_fifo_result(CB_PTR cb_ptr, unsigned char *return_byte)
{

    if (wait_for_result_byte(cb_ptr) == YES) {
        *return_byte = inportb(FIFO);
        return ( OK );
    }
    else
        return ( NO );

}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/* FUNCTION: dma_setup(data_ptr, data_count, dma_chan)                       */
/*                                                                           */
/* ACTION:   Setup for DMA transfer                                          */
/*                                                                           */
/* RETURNS:  None                                                            */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#ifdef CC_CARD
void dma_setup(CB_PTR        cb_ptr,
               unsigned char *data_ptr,
               int           data_count,
               ulong         dma_chan
               int           rw_flag)
{

    int           count;
    ulong         addr;
    unsigned char cmd;
    unsigned char iol;
    unsigned char ioh;
    unsigned char page;
    unsigned char addrl;
    unsigned char addrh;
    unsigned char countl;
    unsigned char counth;


    addr  = (ulong)data_ptr;
    count = data_count - 1 ;

    iol    = (unsigned char)(((FIFO  & 0x000000ff)      ) & 0x000000ff);
    ioh    = (unsigned char)(((FIFO  & 0x0000ff00) >>  8) & 0x000000ff);
    addrl  = (unsigned char)(((addr  & 0x000000ff)      ) & 0x000000ff);
    addrh  = (unsigned char)(((addr  & 0x0000ff00) >>  8) & 0x000000ff);
    page   = (unsigned char)(((addr  & 0x00ff0000) >> 16) & 0x000000ff);
    countl = (unsigned char)(((count & 0x000000ff)      ) & 0x000000ff);
    counth = (unsigned char)(((count & 0x0000ff00) >>  8) & 0x000000ff);

    /*-----------------------------------------*/
    /* Reset Central Arbitration Control Point */
    /*-----------------------------------------*/
    outportb(ARB_CTRL_POINT, (unsigned char)0x80);

    /*---------------------*/
    /* Disable DMA Channel */
    /*---------------------*/
    cmd = (unsigned char)SET_DMA_MASK_CMD + dma_chan;
    outportb(FUNC_PORT, cmd);

    /*--------------------------------*/
    /* Set Destination Memory Address */
    /*--------------------------------*/
    cmd = (unsigned char)BASE_MEM_ADDR_CMD + dma_chan;
    outportb(FUNC_PORT, cmd);
    outportb(EXEC_PORT, addrl);
    outportb(EXEC_PORT, addrh);
    outportb(EXEC_PORT, page);

    /*--------------------*/
    /* Set Transfer Count */
    /*--------------------*/
    cmd = (unsigned char)BASE_XFER_COUNT_CMD + dma_chan;
    outportb(FUNC_PORT, cmd);
    outportb(EXEC_PORT, countl);
    outportb(EXEC_PORT, counth);

    /*-------------------------------*/
    /* Set Source Address (FDC FIFO) */
    /*-------------------------------*/
    cmd = (unsigned char)IO_ADDR_CMD + dma_chan;
    outportb(FUNC_PORT, cmd);
    outportb(EXEC_PORT, iol);
    outportb(EXEC_PORT, ioh);

    /*--------------*/
    /* Set DMA Mode */
    /*--------------*/
    cmd = (unsigned char)DMA_MODE_CMD + dma_chan;
    outportb(FUNC_PORT, cmd);
    outportb(EXEC_PORT, DMA_MODE);

    /*--------------------*/
    /* Enable DMA Channel */
    /*--------------------*/
    cmd = (unsigned char)RESET_DMA_MASK_CMD + dma_chan;
    outportb(FUNC_PORT, cmd);

}
#else
void dma_setup(CB_PTR        cb_ptr,
               unsigned char *data_ptr,
               int           data_count,
               int           rw_flag)
{

    int           count;
    ulong         addr;
    unsigned char pagel;
    unsigned char pageh;
    unsigned char addrl;
    unsigned char addrh;
    unsigned char countl;
    unsigned char counth;

    PRINTF("dma_setup.\n", "");

    addr  = (ulong)data_ptr;
    count = data_count - 1 ;

    addrl  = (unsigned char)(((addr  & 0x000000ff)      ) & 0x000000ff);
    addrh  = (unsigned char)(((addr  & 0x0000ff00) >>  8) & 0x000000ff);
    pagel  = (unsigned char)(((addr  & 0x00ff0000) >> 16) & 0x000000ff);
    pageh  = (unsigned char)(((addr  & 0xff000000) >> 24) & 0x000000ff);
    countl = (unsigned char)(((count & 0x000000ff)      ) & 0x000000ff);
    counth = (unsigned char)(((count & 0x0000ff00) >>  8) & 0x000000ff);

    /*----------------------------*/
    /* Processor Addr to PCI Addr */
    /*----------------------------*/
    pageh  = pageh | 0x80;

    /*---------------------------------------------*/
    /* Clear Mask Channel 4 to Enable Channel 0~3. */
    /*---------------------------------------------*/
    outportb(MASK_REG_2, (unsigned char)0x00);

    /*----------------------------------*/
    /* Mask the channel to disable DMA. */
    /*----------------------------------*/
    outportb(MASK_REG, SET_DMA_MASK_CMD);

    /*----------*/
    /* Set Mode */
    /*----------*/
    if ( rw_flag == READ )
        outportb(DCM_REG, SET_MODE_CMD_R);
    else
        outportb(DCM_REG, SET_MODE_CMD_W);

    /*-------------------*/
    /* Set Extended Mode */
    /*-------------------*/
    outportb(DCEM_REG, SET_EXT_MODE_CMD);

    /*--------------------------------*/
    /* Set Destination Memory Address */
    /*--------------------------------*/
    outportb(CLR_BYTE_POINTER_REG,  (unsigned char)0x00);
    outportb(BASE_ADDR_REG,   addrl);
    outportb(BASE_ADDR_REG,   addrh);
    outportb(LOW_PAGE_REG_2,  pagel);
    outportb(HIGH_PAGE_REG_2, pageh);

    /*----------------*/
    /* Set Byte Count */
    /*----------------*/
    outportb(CLR_BYTE_POINTER_REG, (unsigned char)0x00);
    outportb(BYTE_COUNT_REG, countl);
    outportb(BYTE_COUNT_REG, counth);

    /*--------------------------------------*/
    /* Clear the mask to enable the channel */
    /*--------------------------------------*/
    outportb(MASK_REG, CLR_DMA_MASK_CMD);
}
#endif


/*---------------------------------------------------------------------------*/
/*                                                                           */
/* FUNCTION: dma_check(dma_chan)                                             */
/*                                                                           */
/* ACTION:   Check DMA Status                                                */
/*                                                                           */
/* RETURNS:  OK or NO                                                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#ifdef CC_CARD
int dma_check(CB_PTR cb_ptr, ulong dma_chan)
{

    unsigned char status;
    unsigned char status1;
    unsigned char status2;


    /*--------------------------*/
    /* Read DMA Status Register */
    /*--------------------------*/
    outportb(FUNC_PORT, RD_DMA_STATUS_CMD + dma_chan);
    status1 = inportb(EXEC_PORT);
    status2 = inportb(EXEC_PORT);

    if ( dma_chan < 4 )
        status = status1;
    else
        status = status2;

    /*------------------------*/
    /* Check if TC bit is ON. */
    /*------------------------*/
    if ( status & (unsigned char)(0x01 << (dma_chan % 4)) )
        return( OK );
    else
        return( NO );

}
#else
int dma_check(CB_PTR cb_ptr)
{
    /*-----------------------------------*/
    /* Check DMA Status Register TC bit. */
    /*-----------------------------------*/
    if ( inportb(DS_REG) & (unsigned char)(0x01 << DMA_CHAN) )
        return ( OK );
    else {
        PRINTF("dma_check failed.\n", "");
        return ( NO );
    }
}
#endif


/*---------------------------------------------------------------------------*/
/*                                                                           */
/* FUNCTION: sens_intr(unsigned char *sr0, unsigned char *ptr)               */
/*                                                                           */
/* ACTION:   Sense Intrupt                                                   */
/*                                                                           */
/* RETURNS:  OK or NO                                                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int sens_intr(CB_PTR cb_ptr, unsigned char *sr0 , unsigned char *ptr)
{

    if (write_first_fifo_cmd(cb_ptr, SENSE_INTR_CMD) == OK &&
        read_fifo_result(cb_ptr, sr0)                == OK &&
        read_fifo_result(cb_ptr, ptr)                == OK   )
        return( OK );
    else
        return( NO );
}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/* FUNCTION: get_results(CB_PTR cb_ptr)                                      */
/*                                                                           */
/* ACTION:   Gets result bytes from FDC and stores them in DSKT_CB.          */
/*                                                                           */
/* RETURNS:  Number of Result Bytes                                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int get_results(CB_PTR cb_ptr)
{

    int index;


    index = 0;

    while ( read_fifo_result(cb_ptr, &(cb_ptr->g_fdc_result[index++])) == OK );

    return ( index - 1 );

}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/* FUNCTION: dskt_int_hdlr(CB_PTR cb_ptr)                                    */
/*                                                                           */
/* ACTION:   Interrupt Handler.                                              */
/*                                                                           */
/* RETURNS:  OK(0) or NO(!0)                                                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/
unsigned int dskt_int_hdlr(void)
{

    unsigned int  rc;
    unsigned char sr0, ptr;
    CB_PTR        cb_ptr;


    PRINTF("interrupt handler.\n", "");

    rc     = 0;
    cb_ptr = &dskt_cb;

    /*--------------------------------------*/
    /* Check if we are expecting interrupt. */
    /*--------------------------------------*/
    if ( cb_ptr->g_int_expected == TRUE ) {

        cb_ptr->g_msr_data = inportb(MSR) & 0xc0;
        /*------------------------------------*/
        /* Check if FIFO is ready to be read. */
        /* If so, read all.                   */
        /*------------------------------------*/
        if (cb_ptr->g_msr_data == 0xc0) {
            cb_ptr->g_int_flag = GET_RESULTS;
            cb_ptr->g_index    = get_results(cb_ptr);
        }
        else
            if ( cb_ptr->g_msr_data == 0x80 )
                /*------------------------------*/
                /* FIFO is ready to be written. */
                /*------------------------------*/
                cb_ptr->g_int_flag = RDY_TO_WRITE;
            else
                cb_ptr->g_int_flag = WRONG_INTERRUPT;

        /*-------------------------------------------*/
        /* If not ready to be read, sense interrupt. */
        /*-------------------------------------------*/
        if ( cb_ptr->g_int_flag != GET_RESULTS ) {
            if ( sens_intr(cb_ptr, &sr0, &ptr) == OK ) {
                cb_ptr->g_fdc_result[0] = sr0;
                cb_ptr->g_fdc_result[1] = ptr;
                cb_ptr->g_index         = 2;
            }
        }
        cb_ptr->g_int_expected = FALSE;
    }
    else {
        /*---------------------------------*/
        /* We are not expecting interrupt. */
        /*---------------------------------*/
        rc = -1;
    }

    return ( rc );

}


#ifdef CC_CARD
void iocc_an_eq_8(CB_PTR cb_ptr,
                  unsigned int addr,
                  unsigned char an_value)
{
    unsigned char      uc_temp;

    uc_temp  = inportb(addr);
    uc_temp &= an_value;
    outportb(addr, uc_temp);

    return;
}

void iocc_or_eq_8(CB_PTR   cb_ptr,
                  unsigned int addr,
                  unsigned char or_value)
{
    unsigned char      uc_temp;

    uc_temp  = inportb(addr);
    uc_temp |= or_value;
    outportb(addr, uc_temp);

    return;
}
#endif
#endif /* DEBUG */
