static char sccsid[] = "@(#)42	1.4  src/rspc/kernext/pcmcia/ssisa/pcm_dd.c, pcmciaker, rspc41J, 9511A_all 3/13/95 20:45:38";
/*
 * COMPONENT_NAME: PCMCIAKER
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
 * This module contains 
 *     pcmcia_config
 *     pcmcia_open
 *     pcmcia_close
 *     pcmcia_ioctl
 */

#include "pcm_inc.h"

/************************************************************************/
/* Global device driver static data areas                               */
/************************************************************************/

/* array containing pointer to adapter information */
extern struct adapter_def *adapter_ptrs;

/* static variable to count number of inited adapters */
ulong   inited_adapters = 0;

/* static variable to count number of opened adapters */
ulong   opened_adapters = 0;

/* an variable to disable interrupt */
extern int pcmcia_intr_prior;

/* global adapter device dirver lock word */
lock_t pcmcia_lock = LOCK_AVAIL;

/* device specific data including ioctl */
struct pcmcia_dsd pcmcia_dsd;

/* exported routines */
int pcmcia_config( dev_t, int, struct uio *);

/* internal routines */
int pcmcia_open( dev_t, ulong, int, int);
int pcmcia_close( dev_t, ulong, int, int);
int pcmcia_ioctl( dev_t, int, int, ulong, int, int);
int pcmcia_config_adapter( struct adapter_def * );
struct adapter_def *pcmcia_alloc_adap( dev_t );
void pcmcia_free_adap( struct adapter_def * );

/* imported routines */
extern int pcmcia_ioctl_intr( dev_t, int, int, ulong, int, int);

#ifdef DEBUG
extern int pcmcia_debug;
#endif

/************************************************************************
 *
 * NAME: pcmcia_config
 *
 * FUNCTION: PCMCIA Socket Service Configuration Routine
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by a process.
 *
 * RETURNS:
 *     0       The operation is successfully completed.
 *     EIO     Bad operation, or permanent I/O error happened.
 *     EBUSY   The device is still opened on terminate.
 *     ENOMEM  Memory space is unavailable for required allocation.
 *     EINVAL  Invalid config parameter is passed.
 *
 ************************************************************************/
int pcmcia_config( dev_t devno, int op, struct uio * uiop )
{
    struct devsw pcmcia_dsw;
    struct pcmcia_ddi local_ddi;
    struct adapter_def *ap;
    int ret_code = 0;
    int rc, i;
    extern int nodev();
    
#ifdef DEBUG
    if (pcmcia_debug) 
		printf("Entering pcmcia_config( devno = 0x%X, op = 0x%X )\n",
			   devno, op );
#endif

    DDHKWD2(HKWD_PCMCIA_SS, DD_ENTRY_CONFIG, ret_code, devno, op);

    rc = lockl(&(pcmcia_lock), LOCK_SHORT);
    if (rc != LOCK_SUCC) {
        ret_code = EIO;
        goto end;
    }

    /* search adapter list for this devno */
    ap = adapter_ptrs;
    while ( ap != NULL ){
        if ( ap->devno == devno ){
            break;
        }
        ap = ap->next;
    } /* end while */

    switch (op) {

    case CFG_INIT:

        if (ap != NULL) {   /* if already in pointer table */
            ret_code = EIO; /* error--already initialized */
            goto exit;
        }

        if (inited_adapters == 0)
		{
            pcmcia_dsw.d_open = (int (*)()) pcmcia_open;
            pcmcia_dsw.d_close = (int (*)()) pcmcia_close;
            pcmcia_dsw.d_read = nodev;
            pcmcia_dsw.d_write = nodev;
            pcmcia_dsw.d_ioctl = (int (*)()) pcmcia_ioctl;
            pcmcia_dsw.d_strategy = nodev;
            pcmcia_dsw.d_ttys = 0;
            pcmcia_dsw.d_select = nodev;
            pcmcia_dsw.d_config = (int (*)()) pcmcia_config;
            pcmcia_dsw.d_print = nodev;
            pcmcia_dsw.d_dump = nodev;
            pcmcia_dsw.d_mpx = nodev;
            pcmcia_dsw.d_revoke = nodev;
			pcmcia_dsd.intr_func = (int (*)())pcmcia_ioctl_intr;
            pcmcia_dsw.d_dsdptr = (caddr_t) &pcmcia_dsd;
            pcmcia_dsw.d_selptr = 0;
            pcmcia_dsw.d_opts = 0;
            rc = devswadd(devno, &pcmcia_dsw);
            if ( rc ) {
                ret_code = EIO;
                goto exit;
            }

			/* pin pcmcia_ioctl_intr() that can be called from interrupt */
			rc = pincode( (int(*)())pcmcia_ioctl_intr);
			if (rc != 0){
				(void) devswdel(devno); /* clean up */
				ret_code = ENOMEM;
				goto exit;
			}
        } /* end of if (inited_adapters == 0) */

        /* move adapter configuration data into local area */
        rc = uiomove((caddr_t) (&local_ddi),
                     (int) sizeof(struct pcmcia_ddi),
                     UIO_WRITE, /* from uio space to kernel space */
                     (struct uio *) uiop);
        if (rc != 0) {  /* if unsuccessful copy */
            if (inited_adapters == 0) {
				(void) unpincode((int(*)())pcmcia_ioctl_intr);
                (void) devswdel(devno); /* clean up */
            }
            ret_code = EIO; /* unsuccessful uiomove */
            goto exit;
        }

#ifdef DEBUG
if (pcmcia_debug){
printf("< PCMCIA DDS >\n");
printf("bus_id         = 0x%X\n",   local_ddi.bus_id);
printf("bus_type       = %d\n",     local_ddi.bus_type);
printf("slot           = %d\n",     local_ddi.slot);
printf("mem_addr       = 0x%X\n",   local_ddi.mem_addr);
printf("mem_addr_width = 0x%X\n",   local_ddi.mem_addr_width);
printf("io_addr        = 0x%X\n",   local_ddi.io_addr);
printf("io_addr_width  = 0x%X\n",   local_ddi.io_addr_width);
printf("int_lvl        = %d\n",     local_ddi.int_lvl);
printf("int_prior      = %d\n",     local_ddi.int_prior);
printf("resource_name  = '%16s'\n", local_ddi.resource_name);
printf("int_mode       = %x\n",     local_ddi.int_mode);
printf("< End of PCMCIA DDS >\n");
}
#endif
		pcmcia_intr_prior = local_ddi.int_prior;

        ap = pcmcia_alloc_adap( devno );
        if (ap == NULL) {   /* NULL means unsuccessful allocation */
            if (inited_adapters == 0) {
				(void) unpincode((int(*)())pcmcia_ioctl_intr);
                (void) devswdel(devno); /* clean up */
            }
            ret_code = ENOMEM;  /* unable to allocate adapter space */
            goto exit;
        }

        /* copy local ddi to adapter structure */
        bcopy(&local_ddi, &ap->ddi, sizeof(struct pcmcia_ddi));
        
        /* check adapter's sockets and initialize them */
        rc = pcmcia_config_adapter( ap );
        if (rc != 0) {
            pcmcia_free_adap(ap);
            if (inited_adapters == 0) {
				(void) unpincode((int(*)())pcmcia_ioctl_intr);
                (void) devswdel(devno);
            }
            ret_code = EIO;
            goto exit;
        }
        ap->opened = FALSE; /* mark adapter not opened */
		ap->ap_lock = LOCK_AVAIL; /* init lockl variable */
        ap->inited = TRUE;  /* mark adapter inited */
        inited_adapters++;  /* increment global counter */
        break;
    
    case CFG_TERM:
        
		if (ap == NULL) {
            ret_code = 0;
            goto exit;
        }
        if (ap->opened) {
            ret_code = EBUSY;
            goto exit;
        }
		pcmcia_free_adap( ap );
		inited_adapters--; /* decrement global counter */
		/* if it is the last */
		if ( inited_adapters == 0 ){
			rc = unpincode((int(*)())pcmcia_ioctl_intr);
			if ( rc != 0 ){
				ret_code = rc;
				goto exit;
			}
			rc = devswdel(devno);
			if ( rc != 0 ){
				ret_code = EIO;
				goto exit;
			}
		}			
        break;
        
    case CFG_QVPD:  /* this is a query request */
        break;
        
    default:    /* this is an invalid request */
        ret_code = EINVAL;  /* error--invalid request */
        goto exit;
        
    }   /* end switch */
    
 exit:
    unlockl( &(pcmcia_lock) ); /* release the lock */

 end:

#ifdef DEBUG
    if (pcmcia_debug) printf("Exiting pcmcia_config() = %d\n", ret_code);
#endif


    DDHKWD2(HKWD_PCMCIA_SS, DD_EXIT_CONFIG, ret_code, devno, op);
    return ret_code;

}  /* end pcmcia_config */

/****************************************************************************
 *
 * NAME: pcmcia_open
 *
 * FUNCTION: PCMCIA Socket Service open routine
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0         The operation is successfully completed.
 *     EACCESS   Re-open with diag-mode is prohibited.
 *     EIO       lockl fails. The adapter is not initialized.
 *
 ***************************************************************************/
int pcmcia_open( dev_t devno, ulong devflag, int chan, int ext )
{
    int rc;
    int ret_code = 0;
    struct adapter_def *ap;

#ifdef DEBUG
    if (pcmcia_debug) printf("Entering pcmcia_open( devno = 0x%X )\n", devno);
#endif

    DDHKWD5(HKWD_PCMCIA_SS, DD_ENTRY_OPEN, ret_code, devno, devflag,
            chan, ext, 0);

    rc = lockl(&(pcmcia_lock), LOCK_SHORT); /* serialize this */
    if (rc != LOCK_SUCC) {
        ret_code = EIO; /* error--kernel service call failed */
        goto end;
    }

    /* search adapter list for this devno */
    ap = adapter_ptrs;
    while (ap != NULL) {
        if (ap->devno == devno)
            break;
        ap = ap->next;
    }   /* endwhile */

    if ((ap == NULL) || (!ap->inited)) {
        ret_code = EIO; /* error--adapter not inited */
        goto exit;
    }

	/* only one process can open with PCM_EXCLUSIVE mode */
    if ((ap->opened) && (ext & PCM_EXCLUSIVE)){
        ret_code = EACCES;
        goto exit;
    }
	if ( ext & PCM_EXCLUSIVE ){
		ap->adapter_mode == PCM_EXCLUSIVE;
	}

	/* increment open count */
    ap->opened ++ ; 

#ifdef DEBUG
        if (pcmcia_debug) printf("Exiting pcmcia_open() = %d\n", ret_code);
#endif

exit:
    unlockl(&(pcmcia_lock));    /* release the lock */
end:

    DDHKWD1(HKWD_PCMCIA_SS, DD_EXIT_OPEN, ret_code, devno);
    return (ret_code);
}

/****************************************************************************
 *
 * NAME: pcmcia_close
 *
 * FUNCTION: PCMCIA Socket Service close routine
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EIO      lockl fails. The adapter is not initialized.
 *
 ***************************************************************************/
int pcmcia_close( dev_t devno, ulong devflag, int chan, int ext )
{
    int rc;
    int ret_code = 0;
    struct adapter_def *ap;

#ifdef DEBUG
    if (pcmcia_debug) printf("Entering pcmcia_close( devno = 0x%X )\n", devno);
#endif

    DDHKWD5(HKWD_PCMCIA_SS, DD_ENTRY_CLOSE, ret_code, devno, devflag,
            chan, ext, 0);

    rc = lockl(&(pcmcia_lock), LOCK_SHORT); /* serialize this */
    if (rc != LOCK_SUCC) {
        ret_code = EIO; /* error--kernel service call failed */
        goto end;
    }

    /* search adapter list for this devno */
    ap = adapter_ptrs;
    while (ap != NULL) {
        if (ap->devno == devno)
            break;
        ap = ap->next;
    }   /* endwhile */

    if ((ap == NULL) || (!ap->inited)) {
        ret_code = EIO; /* error--adapter not inited */
        goto exit;
    }

	/* decrement open count */
    ap->opened -- ;

	/* if not opened, reset PCM_EXLUSIVE */
	if ( ap->opened == 0 ){
		ap->adapter_mode = 0x00;
	}

#ifdef DEBUG
        if (pcmcia_debug) printf("Exiting pcmcia_close() = %d\n", ret_code);
#endif

exit:
    unlockl(&(pcmcia_lock));    /* release the lock */
end:
    DDHKWD1(HKWD_PCMCIA_SS, DD_EXIT_CLOSE, ret_code, devno);
    return (ret_code);
}

/****************************************************************************
 *
 * NAME: pcmcia_ioctl
 *
 * FUNCTION: PCMCIA Socket Service ioctl routine
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called only by a process
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EACCESS  Re-open with diag-mode is prohibited.
 *     EINVAL   An invalid parameter is specified.
 *     EIO      lockl fails. The adapter is not initialized.
 *
 ***************************************************************************/
int
pcmcia_ioctl( dev_t devno, int op, int arg, ulong devflag, int chan, int ext )
{
    struct adapter_def *ap;
    struct pcmcia_ioreg ioreg;
    int     rc, ret_code;
    int     slot;
    uint    mem_addr_start;
    uint    mem_addr_stop;
    uint    mem_addr_width;
    uint    offset_addr;
	RETCODE pcm_retcode;

#ifdef DEBUG
    if (pcmcia_debug) printf("Entering pcmcia_ioctl( op = 0x%X )\n", op);
#endif

    ret_code = 0;   /* default to no errors found  */

    DDHKWD5(HKWD_PCMCIA_SS, DD_ENTRY_IOCTL, ret_code, devno, op, devflag,
            chan, ext);

    /* lock the global lock to serialize with open/close/config */
    rc = lockl(&(pcmcia_lock), LOCK_SHORT); /* serialize this */
    if (rc != LOCK_SUCC) {
        ret_code = EIO; /* error--kernel service call failed */
        goto end;
    }

    /* search adapter list for this devno */
    ap = adapter_ptrs;

    while (ap != NULL) {
        if (ap->devno == devno){
            break;
        }
        ap = ap->next;
    }   /* endwhile */

    if ((ap == NULL) || (! ap->opened)){
        ret_code = EIO; /* error--adapter not inited */
        unlockl(&(pcmcia_lock));    /* release the lock */
        goto end;
    }

    /* lock the adapter lock to serialize these per adapter */
    rc = lockl(&(ap->ap_lock), LOCK_SHORT); /* serialize this */
    if (rc != LOCK_SUCC) {
        ret_code = EIO; /* error--kernel service call failed */
        unlockl(&(pcmcia_lock));    /* release the lock */
        goto end;
    }

    unlockl(&(pcmcia_lock));    /* release the global lock */
    
    switch ( op )
	{
    case PCM_ACK_INTERRUPT:
		ret_code = pcmcia_AcknowledgeInterrupt( ap, arg, devflag );
		break;
    case PCM_GET_SS_INFO:
        ret_code = pcmcia_GetSSInfo( ap, arg, devflag );
        break;
    case PCM_INQ_ADAPTER:
        ret_code = pcmcia_InquireAdapter(ap, arg, devflag );
        break;
    case PCM_GET_ADAPTER:
        ret_code = pcmcia_GetAdapter( ap, arg, devflag );
        break;
    case PCM_SET_ADAPTER:
        ret_code = pcmcia_SetAdapter( ap, arg, devflag );
        break;
    case PCM_INQ_WINDOW:
        ret_code = pcmcia_InquireWindow( ap, arg, devflag );
        break;
    case PCM_GET_WINDOW:
        ret_code = pcmcia_GetWindow( ap, arg, devflag );
        break;
    case PCM_SET_WINDOW:
        ret_code = pcmcia_SetWindow( ap, arg, devflag );
        break;
    case PCM_GET_PAGE:
        ret_code = pcmcia_GetPage( ap, arg, devflag );
        break;
    case PCM_SET_PAGE:
        ret_code = pcmcia_SetPage( ap, arg, devflag );
        break;
    case PCM_INQ_SOCKET:
        ret_code = pcmcia_InquireSocket( ap, arg, devflag );
        break;
    case PCM_GET_SOCKET:
        ret_code = pcmcia_GetSocket( ap, arg, devflag );
        break;
    case PCM_SET_SOCKET:
        ret_code = pcmcia_SetSocket( ap, arg, devflag );
        break;
    case PCM_GET_STATUS:
        ret_code = pcmcia_GetStatus( ap, arg, devflag );
        break;
    case PCM_RESET_SOCKET:
        ret_code = pcmcia_ResetSocket( ap, arg, devflag );
        break;
    case PCM_GET_VENDOR_INFO:
		ret_code = pcmcia_GetVendorInfo( ap, arg, devflag );
		break;
    case PCM_VEND_SPECIFIC:
		ret_code = pcmcia_VendorSpecific( ap, arg, devflag );
		break;

	/* Unsupported Socket Services */
    case PCM_GET_ADP_CNT:
    case PCM_INQ_EDC:
    case PCM_GET_EDC:
    case PCM_SET_EDC:
    case PCM_START_EDC:
    case PCM_PAUSE_EDC:
    case PCM_RESUME_EDC:
    case PCM_STOP_EDC:
    case PCM_READ_EDC:

    case PCM_PRIOR_HANDLER:
    case PCM_SS_ADDR:
    case PCM_ACCESS_OFFSETS:
    case PCM_CARD_SERVICES:
    default:
		ret_code = pcmcia_bad_function( ap, arg, devflag, op );
		/* ret_code is EINVAL or EFAULT */
		break;
    }   /* end switch */

    unlockl(&(ap->ap_lock));    /* release the adapter lock */
 end:

#ifdef DEBUG
    if (pcmcia_debug) printf("Exiting pcmcia_ioctl() = %d\n", ret_code);
#endif

    DDHKWD1(HKWD_PCMCIA_SS, DD_EXIT_IOCTL, ret_code, devno);

    return ret_code;
}
/************************************************************************
 *                                                                      
 * NAME:        pcmcia_alloc_adap                                       
 *                                                                      
 * FUNCTION:
 *     Allocates the adapter information data.
 *
 * EXECUTION ENVIRONMENT:                                               
 *     This routine can be called by a process.
 *                                                                      
 * RETURNS:
 *     NULL     indicates an error
 *     non-NULL indicates success, and 
 *              returns the pointer to the adapter information.
 *
 ************************************************************************/
struct adapter_def *
pcmcia_alloc_adap( dev_t devno )
{
    struct adapter_def *ap;
    struct adapter_def *temp_ptr;
    int old_intr_prior;

#ifdef DEBUG
    if (pcmcia_debug) printf("Entering pcmcia_alloc_adap( 0x%X )\n", devno);
#endif

    /* malloc adapter struct space word-alligned from pinned_heap */
    ap = (struct adapter_def *)
        xmalloc((uint) ( sizeof(struct adapter_def)), 4, pinned_heap);

    old_intr_prior = i_disable( pcmcia_intr_prior );

    if (ap == NULL) {
        /* xmalloc failed -- return NULL pointer */
        goto end;
    }   /* successfully malloc'ed and pinned adapter struct */

    /* initially, clear the adapter structure */
    bzero(ap, sizeof(struct adapter_def));

    /* add this adapter pointer to adapter list */
    if (adapter_ptrs == NULL) { /* if no chain here */
        adapter_ptrs = ap;  /* store in ptr array */
    } else {    /* points to a chain */
        temp_ptr = adapter_ptrs;    /* point at first one */
        while (temp_ptr->next != NULL)  /* loop until end found */
            temp_ptr = temp_ptr->next;  /* look at next struct */
        temp_ptr->next = ap;    /* add at end of chain */
    }
    ap->next = NULL;    /* mark new end of chain */
    ap->devno = devno;  /* store the adap devno  */

 end:
    i_enable( old_intr_prior );

#ifdef DEBUG
    if (pcmcia_debug) printf("Exiting pcmcia_alloc_adap() = 0x%X\n", ap);
#endif

    return ap;
}
/* end of pcmcia_alloc_adap */

/***********************************************************************
 *                                                                      
 * NAME:        pcmcia_free_adap                                        
 *                                                                      
 * FUNCTION:    Free Adapter Information Structures                     
 *                                                                      
 *      This internal routine frees memory which was previously         
 *      allocated by the pcmcia_alloc_adap() routine for needed            
 *      adapter structures.                                             
 *                                                                      
 * EXECUTION ENVIRONMENT:                                               
 *      This routine can be called from any other routine.              
 *                                                                      
 * DATA STRUCTURES:                                                     
 *      adapter_def - adapter unique data structure (one per adapter)   
 *                                                                      
 * INPUTS:                                                              
 *      ap      - pointer to adapter structure                          
 *                                                                      
 * RETURN VALUE DESCRIPTION:  The following are the return values:      
 *      none                                                            
 *                                                                      
 * EXTERNAL PROCEDURES CALLED:                                          
 *      xmfree                                                          
 *                                                                      
 ************************************************************************/
void pcmcia_free_adap( struct adapter_def * ap )
{
    struct adapter_def *tptr;
    int    old_intr_prior;

#ifdef DEBUG
    if (pcmcia_debug) printf("Entering pcmcia_free_adap( ap = 0x%X )\n", ap);
#endif

    old_intr_prior = i_disable( pcmcia_intr_prior );

    /* it is already known that ap is contained in the list */
    if (adapter_ptrs == ap){
        adapter_ptrs = ap->next;    /* remove ap from chain */
    } else {
        tptr = adapter_ptrs;    /* starting pointer */
        while (tptr->next != NULL) {    /* follow chain     */
            if (tptr->next == ap) {
                tptr->next = ap->next;  /* remove ap from chain */
                break;
            }
            tptr = tptr->next;  /* look at next element */
        }   /* endwhile */
    }

    i_enable( old_intr_prior );

    /* unpin and deallocate the adapter structure */
    (void) xmfree((void *) ap, pinned_heap);

#ifdef DEBUG
    if (pcmcia_debug) printf("Exiting pcmcia_free_adap()\n");
#endif

} /* end pcmcia_free_adap */

/***********************************************************************
 *                                                                      
 * NAME:        pcmcia_config_adapter                                      
 *                                                                      
 * FUNCTION:    Adapter Configuration Routine                           
 *                                                                      
 *      (It counts the PCMCIA slot numbers.)
 *                                                                      
 * EXECUTION ENVIRONMENT:                                               
 *      This routine can be called by a process.
 *                                                                      
 * RETURNS:
 *     0    The operation is successful.
 *                                                                      
 ************************************************************************/
int pcmcia_config_adapter( struct adapter_def *ap )
{
	int socket;
	struct pcmcia_ioreg ioreg;
	int rc, ret_code = 0;

	for ( socket = 0; socket < MAX_SOCKETS_REAL; socket++ )
	{
		/* Identification and Revision Register (0x00) */
		ioreg.index = ioreg_index( PCM_ID_REVISION, socket );
		rc = pcmcia_ioreg( ap, PCM_IOREAD, & ioreg );
		if ( rc ){
			ret_code = -1;
			goto end;
		}
		if ( ioreg.data == 0x83 )
		{
			rc = pcmcia_init_registers( ap, socket );
			if ( rc ){
				goto end;
			}
			ap->num_sockets ++ ;
		} else {
			goto end;
		}
	}
	/* if no sockets found */
	if ( ap->num_sockets == 0 ){
		ret_code = -1;
	}

 end:

	if( ap->num_sockets > MAX_SOCKETS ) ap->num_sockets = MAX_SOCKETS;

#ifdef DEBUG
if (pcmcia_debug) printf("Exiting pcmcia_config_adapter( ap = 0x%X )\n", ap);
#endif

	return ret_code;
}

