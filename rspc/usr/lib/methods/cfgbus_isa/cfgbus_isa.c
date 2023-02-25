static char sccsid[] = "@(#)17 1.14 src/rspc/usr/lib/methods/cfgbus_isa/cfgbus_isa.c, rspccfg, rspc41J, 9523C_all 6/2/95 16:17:42";
/*
 *   COMPONENT_NAME: rspccfg     define_children for isa bus
 *
 *   FUNCTIONS:
 *              define_children
 *              bus_depend
 *              set_attributes
 *              bus_dependent_cfg
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <cf.h>                 /* E_xxxx return declarations */
#include <sys/cfgodm.h>         /* odm database definitions */
#include <sys/cfgdb.h>          /* status flags */
#include <odmi.h>               /* odm_* subroutine definitions */
#include <sys/iplcb.h>          /* buc table entries */
#include <sys/ioacc.h>          /* BID_ALTREG define */
#include "cfgdebug.h"           /* DEBUG_x declarations */
#include "isa_info.h"           /* ISA_info node definition */
#include "cfgresid.h"           /* definition for CFG_DEVICE */
#include "bcm.h"                /* RSPC definitions */

extern struct ISA_info *isa_info_head;
extern struct ISA_info *isa_info_tail;
extern struct PdDv *find_PdDv();  /* get PdDv for residual data device */

extern int get_chipid_descriptor();     /* get chip id from residual data */
extern int get_irq_packets();
extern int get_mem_packets();
extern int get_io_packets();
extern int get_dma_packets();

/*
 * NAME: define_children
 *
 * FUNCTION: this routine determines the set of ISA devices to be
 *           configured and writes their names to standard out.
 *           It queries the CuDv database for currently defined
 *           children of the bus and residual data for machine devices.
 *
 *   INPUT:     isa bus device defined
 *              bus_devname : Special file device name (/dev/<busname>)
 *              phase       : IPL Phase
 *              busname     : Logical device name of ISA bus.
 *   OUTPUT:    ISA child devices defined
 *              Logical names of child devices output to stdout
 *              Install package names of unidentified devices to stdout
 *   RTN CODE:  E_OK            successful return
 *              E_ODMGET        could not get device CuDv object
 *              E_NOCuDv        no CuDv entry created when should have
 */
int
define_children(bus_devname, phase, busname)
char *bus_devname;
int  phase;
char *busname;
{

    int     rc;
    struct ISA_info *ptr;
    char    connwhere[16];
    char    loc_code[16];
    char    loc_key[4];         /* temp - setup location code */
    char    sstr[256];
    char    *outp, *errp;
    struct  CuDv cuobj;
    int     have_cudv;          /* flag to state cudv is valid or not */
    CFG_DEVICE      *devp;        /* DEVICES - residual data */
    int     resid_offset;       /* offset for intg isa resid entries */


    DEBUG_0("cfgbus_isa: in define_children\n");

    rc = init_isa_info ( busname );
    if ( rc != E_OK) {
	/* error returned, cancel config */
	return(rc);
    }
    DEBUG_1("isa info head = %x\n",isa_info_head);
    DEBUG_1("isa info tail = %x\n",isa_info_tail);

    /* match customized devices to residual devices */
    rc = map_residual ( );
    if ( rc != E_OK) {
	/* error returned, cancel config */
	return(rc);
    }

    /* initialize info for non-detectable, pre-defined ISA devices */
    rc = map_non_detectable ( );
    if ( rc != E_OK) {
	/* error returned, cancel config */
	return(rc);
    }

#ifdef CFGDEBUG
    prt_tables();       /* debug to print tables */
#endif


    /*********************************************************************
     * At this point, the ISA_info linked list is fully initialized.
     * Run through the list and define or configure the devices.
     *********************************************************************/

    resid_offset = -1;          /* initialize resid offset value */
    ptr = isa_info_head;
    DEBUG_0("cfgbus_isa: define children, list of devices found\n");
    while ( ptr != NULL ) {
	DEBUG_1("    device in loop = %s",ptr->pddv->uniquetype);

#ifdef CFGDEBUG
	if ( ptr->cudv != NULL ) {
	    DEBUG_2("    name = %s, config = %d\n",
		     ptr->cudv->name,ptr->config);
	}
	else DEBUG_0("\n");
#endif

	/* set residual data entry offset for location code */
	if (ptr->integrated == 1) {
	    resid_offset++;
	}

	if ( ptr->config ) {
	    /* set flag for having cudv unless Define fails below */
	    have_cudv = 1;

	    if ( ptr->cudv != NULL ) {
		/* Successfully obtained the card's matching CuDv object */
		/* Set the chgstatus field */
		if (ptr->cudv->chgstatus == MISSING) {
			ptr->cudv->chgstatus = SAME;
			if (odm_change_obj(CuDv_CLASS, ptr->cudv) == -1)
				return(E_ODMUPDATE);
		}

		/* device has already been defined, write name to stdout */
		/* (do not print name if bus extender, */
		/* its config method will be called below) */
		if (!ptr->pddv->bus_ext){
		    fprintf(stdout, "%s\n", ptr->cudv->name);
		}
	    }
	    else {  /* no cudv, device needs to be defined */
		/* define method needs to be invoked */
		/* construct the connwhere and location fields */

		if (ptr->integrated == 1) {
		    /* connwhere = PNP+serial_num if integrated */
		    devp = (CFG_DEVICE *) ptr->resid;
		    sprintf(connwhere, "%s%x",
			devp->pnpid,
			devp->deviceid.serialnum);

		    sprintf(loc_code,"%02d-%c0",ISABridge,('A'+resid_offset));

		    /* set up the define method */
		    /* -d flag needed so connwhere is not generated */
		    sprintf(sstr,"-d -c %s -s %s -t %s -p %s -w %s -L %s",
			    ptr->pddv->class, ptr->pddv->subclass,
			    ptr->pddv->type, busname, connwhere,
			    loc_code);

		}
		else {
		    /* if non-integrated, connwhere and location */
		    /* code is generated by define_rspc        */

		    /* set up the define method */
		    sprintf(sstr,"-c %s -s %s -t %s -p %s",
			    ptr->pddv->class, ptr->pddv->subclass,
			    ptr->pddv->type, busname);
		}


		DEBUG_1("cfgbus_isa: run define with parms: %s\n",
			sstr);

		/* run define method with the loc code already set */
		rc = odm_run_method(ptr->pddv->Define,sstr,&outp,&errp);
			/* ignore error return, need to see if */
			/* rest of machine configures */

		DEBUG_2("cfgbus_isa: define ran for %s, rc=%d\n",
			sstr,rc);

		/* get cudv object to check define working */
		sprintf(sstr,"name = %s",outp);
		rc = (int)odm_get_first(CuDv_CLASS,sstr,&cuobj);
		if ( rc == -1) {
			return(E_ODMGET);
		}
		/* if rc = 0, I could return a E_NOCuDv error */
		/* this is not done so rest of machine can configure */
		/* we should continue to next device here */
		if ( rc == 0 ) have_cudv = 0;
		if ( ptr->cudv == NULL) ptr->cudv = &cuobj;

		/* print name to stdout */
		/* (do not print name if bus extender, */
		/* its config method will be called below) */
		if (!ptr->pddv->bus_ext){
		    fprintf(stdout, "%s\n", outp);
		}
	    }

	    /* if cudv was created, then continue, else do next device */
	    if ( have_cudv == 1) {

		/* If card is a bus extender, need to invoke its    */
		/* configure method in order to define its children */
		/* Note : ignore errors from invoke_adapt_cfg() to  */
		/* ensure all devices are processed if a Configure  */
		/* method fails.                                    */

		if (ptr->pddv->bus_ext){
			DEBUG_0("cfgbus_isa: found bus extender\n");
			rc =invoke_adapt_cfg ( phase, ptr->pddv, ptr->cudv );
		}

		/* set residual data attributes */
		if ( ptr->resid ) {

		    if ( ptr->cudv == NULL ) {

			ptr->cudv = (struct CuDv *) malloc ( sizeof (struct CuDv));
			if ( ptr->cudv == NULL ) {
			    DEBUG_0("cfgbus_isa: define_children, malloc failed\n");
			    return(E_MALLOC);
			}
			memcpy ( ptr->cudv, &cuobj, sizeof (struct CuDv));
			DEBUG_1("    cudv copied for device %s\n",ptr->cudv->name);

		    }
		    if ( ptr->cudv->status != AVAILABLE ) {
			set_attributes ( ptr );
		    }
		}
	    }

	}
	/* process next ISA_info node */
	ptr = ptr->next;
    }

    return(E_OK);
}


/*
 * NAME     : bus_depend()
 * FUNCTION : For the isa bus configurator, nothing
 *            needs to be done.
 *
 * INPUTS   :
 *      busname : bus being configured
 *      devname : bus's special file name
 *
 * RETURNS : 0
 *
 */
int
bus_depend(busname, devname)
char    *busname;
char    *devname;
{


	return(0);
}


/*
 *   NAME:      set_attributes
 *
 *   FUNCTION:  sets residual information to odm for a device
 *
 *   INPUT:     isa_cudv and isa_info lists have been initialized
 *   OUTPUT:    isa_cudv pointers in isa_info list point to cudv objects for
 *              the integrated devices that have CuDv entries.
 *   RTN CODE:  0 - successful return
 *              E_x - error return
 *
 */

int
set_attributes(isa_info_ptr)
struct ISA_info *isa_info_ptr;          /* pointer to isa info nodes */
{

    int rc;                     /* return code from function calls */
    int resid_offset;               /* loop counter control */
    int odm_offset;                 /* loop counter control */
    int odm_other;                  /* loop counter control */
    int num_packets;            /* number of packets returned on heap call */
    char    sstr[256];
    struct CuAt *cusattobj;         /* device customized attribute storage */
    struct PdAt pdattobj;         /* device predefined attribute storage */
    int     how_many;               /* storage for getattr command */
    int seq_num;                /* sequence number on PdAt.type field */

    ulong  cuat_lvalue;                 /* value of cuat in ulong */
    char   cuat_svalue[64];             /* value of cuat in char  */
    CFG_irqpack_t  *irqpack_p;      /* IRQ Residual Packet structure */
    CFG_dmapack_t  *dmapack_p;      /* DMA Residual Packet structure */
    CFG_iopack_t   *iopack_p;       /* I/O Addr Residual Packet structure */
    CFG_mempack_t  *mempack_p;      /* Memory Residual Packet structure */

    DEBUG_0("cfgbus_isa: set_attributes, start\n");

    /* for specified device in isa_info list */
    /*    try to match bus attributes */

    /* get odm attributes */
    cusattobj = getattr( isa_info_ptr->cudv->name, "", TRUE,&how_many );
    if (cusattobj == NULL) {
	/* no attributes to be changed */
	return(0);
    }
    else {
	DEBUG_1("cfgbus_isa: set_attributes, got %d attrs\n",how_many);
    }

    /* read i/o address packets from resid "O"*/
    /* read packets, determine how many read */

    DEBUG_0("cfgbus_isa: set_attributes, calling get_io_packets\n");
    rc = get_io_packets(isa_info_ptr->resid_offset, 'a', &num_packets, &iopack_p);

    DEBUG_2("cfgbus_isa: set_attributes, get_io_packets rc = %d, returned %d packets\n",
				    rc,num_packets);
    if ( rc != 0 ) {
	/* return error from subroutine */
	return (rc);
    }

    /*
     * match resources to PdAt/CuAt entries
     *   (remember for multiple, append ".x")
     * update odm if necessary
     */
    for (resid_offset=0; resid_offset<num_packets; resid_offset++) {
	DEBUG_1("cfgbus_isa: set_attributes, i/o addr = 0x%x\n",
					     (iopack_p+resid_offset)->min);
	for ( odm_offset=0; odm_offset<how_many; odm_offset++ ) {
	    /* skip all packets with TYPE!=O */
	    if ( (cusattobj+odm_offset)->type[0] != 'O' ) {
		/* if no attributes found matching this type */
		/* NOTE: if more residual entries than odm objects */
		/* ignore the rest of residual entries for now */
		/* may store these somewhere else in odm later */
		continue;
	    }
	    /* check if Type should have sequence number */
	    if ( num_packets == 1 ) {
		/* make sure no other attributes of this type */
		for ( odm_other=odm_offset+1; odm_other<how_many; odm_other++) {
		    if ( (cusattobj+odm_other)->type[0] == 'O' ) {
			/* error, multiple objects to handle */
			return ( E_BADATTR );
		    }
		}
	    }
	    else {
		/* make sure we got the correct object */
		seq_num = atol ( strchr ((cusattobj+odm_offset)->type, '.')+1);
		DEBUG_1("cfgbus_isa: set_attributes, object seq_num = %d\n",seq_num);
		/* make sure we have a valid sequence number */
		if ( seq_num < 1 ) {
		    /* bad entry */
		    return ( E_BADATTR );
		}
		if ( resid_offset != seq_num-1 ) {
		    /* wrong entry, get another */
		    /* NOTE: if more residual entries than odm objects */
		    /* ignore the rest of residual entries for now */
		    /* may store these somewhere else in odm later */
		    continue;
		}
	    }

	    /* check for both values to be equal and update odm if req */
	    DEBUG_1("cfgbus_isa: set_attributes, i/o odm addr = %s\n",
					     (cusattobj+odm_offset)->value);

	    cuat_lvalue = strtoul((cusattobj+odm_offset)->value, (char **)NULL, 0);

	    /* if values are different and odm updateable */
	    if (cuat_lvalue != (iopack_p+resid_offset)->min) {

		sprintf((cusattobj+odm_offset)->value, "0x%x", (iopack_p+resid_offset)->min);
		putattr((cusattobj+odm_offset)) ;

	    }

	    break;
	}
    }

    /* read memory packets from resid "M/B" */
    /* read packets, determine how many read */

    DEBUG_0("cfgbus_isa: set_attributes, calling get_mem_packets\n");
    rc = get_mem_packets(isa_info_ptr->resid_offset, 'a', &num_packets, &mempack_p);

    DEBUG_2("cfgbus_isa: set_attributes, get_mem_packets rc = %d, returned %d packets\n",
				    rc,num_packets);
    if ( rc != 0 ) {
	/* return error from subroutine */
	return (rc);
    }

    /*
     * match resources to PdAt/CuAt entries
     *   (remember for multiple, append ".x")
     * update odm if necessary
     */
    for (resid_offset=0; resid_offset<num_packets; resid_offset++) {
	DEBUG_1("cfgbus_isa: set_attributes, mem addr = 0x%x\n",
					     (mempack_p+resid_offset)->min);
	for ( odm_offset=0; odm_offset<how_many; odm_offset++ ) {
	    /* skip all packets with TYPE!=M or B */
	    if ( (cusattobj+odm_offset)->type[0] != 'M' &&
		 (cusattobj+odm_offset)->type[0] != 'B'    ) {
		/* if no attributes found matching this type */
		/* NOTE: if more residual entries than odm objects */
		/* ignore the rest of residual entries for now */
		/* may store these somewhere else in odm later */
		continue;
	    }

	    /* check if Type should have sequence number */
	    if ( num_packets == 1 ) {
		/* make sure no other attributes of this type */
		for ( odm_other=odm_offset+1; odm_other<how_many; odm_other++) {
		    if ( (cusattobj+odm_other)->type[0] == 'M' ||
			 (cusattobj+odm_other)->type[0] == 'B' ) {
			/* error, multiple objects to handle */
			return ( E_BADATTR );
		    }
		}
	    }
	    else {
		/* make sure we got the correct object */
		seq_num = atol ( strchr ((cusattobj+odm_offset)->type, '.')+1);
		DEBUG_1("cfgbus_isa: set_attributes, object seq_num = %d\n",seq_num);
		/* make sure we have a valid sequence number */
		if ( seq_num < 1 ) {
		    /* bad entry */
		    return ( E_BADATTR );
		}
		if ( resid_offset != seq_num-1 ) {
		    /* wrong entry, get another */
		    /* NOTE: if more residual entries than odm objects */
		    /* ignore the rest of residual entries for now */
		    /* may store these somewhere else in odm later */
		    continue;
		}
	    }

	    /* check for both values to be equal and update odm if req */
	    DEBUG_1("cfgbus_isa: set_attributes, mem odm addr = %s\n",
					     (cusattobj+odm_offset)->value);

	    cuat_lvalue = strtoul((cusattobj+odm_offset)->value, (char **)NULL, 0);

	    /* if values are different and odm updateable */
	    if (cuat_lvalue != (mempack_p+resid_offset)->min) {

		sprintf((cusattobj+odm_offset)->value, "0x%x", (mempack_p+resid_offset)->min);
		putattr((cusattobj+odm_offset)) ;
	    }

	    /* check if width field is adjustable and smaller than resid */
	    sprintf(sstr,"attribute=%s AND uniquetype=%s",
	       (cusattobj+odm_offset)->attribute,
	       isa_info_ptr->cudv->PdDvLn_Lvalue);
	    rc=(int)odm_get_first(PdAt_CLASS,sstr,&pdattobj);
	    if (rc == 0 )  return E_NOCuDv;
	    else if (rc == -1)  return E_ODMGET;

	    /* if width field is empty */
	    if ( !strcmp(pdattobj.width,"") ) {
		/* locate width attribute */
		sprintf(sstr,"width=%s AND uniquetype=%s",
				 pdattobj.attribute,
				 pdattobj.uniquetype);
		rc=(int)odm_get_first(PdAt_CLASS,sstr,&pdattobj);
		if (rc == 0 )  return E_NOCuDv;
		else if (rc == -1)  return E_ODMGET;

		/* locate this cudv entry in array */
		for ( odm_other=0; odm_other<how_many; odm_other++) {
		    if ( strcmp((cusattobj+odm_other)->attribute,
				pdattobj.attribute) == 0) {
			/* check if update is required */
			cuat_lvalue = strtoul((cusattobj+odm_other)->value, (char **)NULL, 0);
			if (cuat_lvalue != (mempack_p+resid_offset)->width) {

			    DEBUG_1("cfgbus_isa: set_attributes, update width on attr %s\n",
					  (cusattobj+odm_offset)->attribute);
			    sprintf((cusattobj+odm_other)->value, "0x%x",
					  (mempack_p+resid_offset)->width);
			    putattr((cusattobj+odm_other)) ;
			}
			break;
		    }
		}
	    }
	    break;
	}
    }

    /* read irq packets from resid "I/N" */
    /* read packets, determine how many read */

    DEBUG_0("cfgbus_isa: set_attributes, calling get_irq_packets\n");
    rc = get_irq_packets(isa_info_ptr->resid_offset, 'a', &num_packets, &irqpack_p);

    DEBUG_2("cfgbus_isa: set_attributes, get_irq_packets rc = %d, returned %d packets\n",
				    rc,num_packets);
    if ( rc != 0 ) {
	/* return error from subroutine */
	return (rc);
    }

    /*
     * match resources to PdAt/CuAt entries
     *   (remember for multiple, append ".x")
     * update odm if necessary
     */
    for (resid_offset=0; resid_offset<num_packets; resid_offset++) {
	DEBUG_1("cfgbus_isa: set_attributes, irq level = %d\n",
					     (irqpack_p+resid_offset)->value);
	for ( odm_offset=0; odm_offset<how_many; odm_offset++ ) {
	    /* skip all packets with TYPE!=N or I */
	    if ( (cusattobj+odm_offset)->type[0] != 'N' &&
		 (cusattobj+odm_offset)->type[0] != 'I'    ) {
		/* if no attributes found matching this type */
		/* NOTE: if more residual entries than odm objects */
		/* ignore the rest of residual entries for now */
		/* may store these somewhere else in odm later */
		continue;
	    }

	    /* check if Type should have sequence number */
	    if ( num_packets == 1 ) {
		/* make sure no other attributes of this type */
		for ( odm_other=odm_offset+1; odm_other<how_many; odm_other++) {
		    if ( (cusattobj+odm_other)->type[0] == 'N' ||
			 (cusattobj+odm_other)->type[0] == 'I' ) {
			/* error, multiple objects to handle */
			return ( E_BADATTR );
		    }
		}
	    }
	    else {
		/* make sure we got the correct object */
		seq_num = atol ( strchr ((cusattobj+odm_offset)->type, '.')+1);
		DEBUG_1("cfgbus_isa: set_attributes, object seq_num = %d\n",seq_num);
		/* make sure we have a valid sequence number */
		if ( seq_num < 1 ) {
		    /* bad entry */
		    return ( E_BADATTR );
		}
		if ( resid_offset != seq_num-1 ) {
		    /* wrong entry, get another */
		    /* NOTE: if more residual entries than odm objects */
		    /* ignore the rest of residual entries for now */
		    /* may store these somewhere else in odm later */
		    continue;
		}
	    }

	    /* check for both values to be equal and update odm if req */
	    DEBUG_1("cfgbus_isa: set_attributes, irq odm level = %s\n",
					     (cusattobj+odm_offset)->value);

	    /*
	     * Generate value from residual that should be in odm.
	     * If level triggered, append 'L' to string.
	     * Compare values and update odm if necessary.
	     */
	    if (((irqpack_p+resid_offset)->flags == HIGH_LEVEL) ||
		((irqpack_p+resid_offset)->flags == LOW_LEVEL))  {

		sprintf(cuat_svalue, "%d,L", (irqpack_p+resid_offset)->value);
	    }
	    else {
		sprintf(cuat_svalue, "%d", (irqpack_p+resid_offset)->value);
	    }

	    /* compare generated string with odm current value */
	    /* update if required */
	    if (strcmp(cuat_svalue,(cusattobj+odm_offset)->value)) {
		strcpy((cusattobj+odm_offset)->value,cuat_svalue);
		putattr((cusattobj+odm_offset)) ;
	    }
	    break;
	}
    }


    /* read DMA packets from resid "A" */
    /* read packets, determine how many read */

    DEBUG_0("cfgbus_isa: set_attributes, calling get_dma_packets\n");
    rc = get_dma_packets(isa_info_ptr->resid_offset, 'a', &num_packets, &dmapack_p);

    DEBUG_2("cfgbus_isa: set_attributes, get_dma_packets rc = %d, returned %d packets\n",
				    rc,num_packets);
    if ( rc != 0 ) {
	/* return error from subroutine */
	return (rc);
    }

    /*
     * match resources to PdAt/CuAt entries
     *   (remember for multiple, append ".x")
     * update odm if necessary
     */
    for (resid_offset=0; resid_offset<num_packets; resid_offset++) {
	DEBUG_1("cfgbus_isa: set_attributes, DMA space = 0x%x\n",
					     (dmapack_p+resid_offset)->value);
	for ( odm_offset=0; odm_offset<how_many; odm_offset++ ) {
	    /* skip all packets with TYPE!=A */
	    if ( (cusattobj+odm_offset)->type[0] != 'A' ) {
		/* if no attributes found matching this type */
		/* NOTE: if more residual entries than odm objects */
		/* ignore the rest of residual entries for now */
		/* may store these somewhere else in odm later */
		continue;
	    }

	    /* check if Type should have sequence number */
	    if ( num_packets == 1 ) {
		/* make sure no other attributes of this type */
		for ( odm_other=odm_offset+1; odm_other<how_many; odm_other++) {
		    if ( (cusattobj+odm_other)->type[0] == 'A' ) {
			/* error, multiple objects to handle */
			return ( E_BADATTR );
		    }
		}
	    }
	    else {
		/* make sure we got the correct object */
		seq_num = atol ( strchr ((cusattobj+odm_offset)->type, '.')+1);
		DEBUG_1("cfgbus_isa: set_attributes, object seq_num = %d\n",seq_num);
		/* make sure we have a valid sequence number */
		if ( seq_num < 1 ) {
		    /* bad entry */
		    return ( E_BADATTR );
		}
		if ( resid_offset != seq_num-1 ) {
		    /* wrong entry, get another */
		    /* NOTE: if more residual entries than odm objects */
		    /* ignore the rest of residual entries for now */
		    /* may store these somewhere else in odm later */
		    continue;
		}
	    }

	    /* check for both values to be equal and update odm if req */
	    DEBUG_1("cfgbus_isa: set_attributes, DMA odm space = %s\n",
					     (cusattobj+odm_offset)->value);

	    cuat_lvalue = strtoul((cusattobj+odm_offset)->value, (char **)NULL, 0);

	    /* if values are different and odm updateable */
	    if (cuat_lvalue != (dmapack_p+resid_offset)->value) {

		sprintf((cusattobj+odm_offset)->value, "%d", (dmapack_p+resid_offset)->value);
		putattr((cusattobj+odm_offset)) ;
	    }
	    break;
	}
    }


    DEBUG_0("cfgbus_isa: set_attributes, done\n");
    return(0);
}

/*
 *
 *   NAME:      bus_dependent_cfg
 *
 *   FUNCTION:  Gets the bus id from BUC table and configured DMA
 *              kernel extension.
 *
 *   INPUT:     buc table entry
 *   OUTPUT:    buid value set
 *
 *   RTN CODE:  0 - successful return
 *              E_x - error return
 *
 */
int
bus_dependent_cfg(buc,index,bus_devname)
struct buc_info *buc;           /* IPL cntrl block buc info section */
int             index;          /* Index of BUC entry in BUC table */
char            *bus_devname;   /* /dev name for bus */
{
    int             rc;
    ulong           buid;
    struct CuAt     *cuat;
    ulong           cuat_busnumber;
    int     cnt;            /* Return value from getattr() */


    /*
     * Generate the buid, given the buid and region.
     */
    buid = (ulong)BID_ALTREG(buc->buid_data[0].buid_value, 0);

    /* Load DMA kernel extension and initialize it */
    rc = load_kernel_mod(buc,RSPC_DMA_KERNEL_EXT);
    if (rc)
	    return(rc);

    /* Set the bus's 'bus_id' attribute */
    rc = set_busid(buid);
    if (rc)
	    return(rc);

    /*
     *  Make the special file for the bus.  Remember that
     *  the Bus Id is the same as the connwhere.
     */
    if (!mk_sp_file(bus_devname, index))
	    return(E_MKSPECIAL);  /* Couldn't make special files */

    return(0);
}
