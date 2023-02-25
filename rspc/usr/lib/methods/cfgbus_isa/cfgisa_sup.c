static char sccsid[] = "@(#)10 1.3 src/rspc/usr/lib/methods/cfgbus_isa/cfgisa_sup.c, rspccfg, rspc41J, 9523C_all 6/2/95 16:18:51";
/*
 *   COMPONENT_NAME: rspccfg
 *
 *   FUNCTIONS:
 *              init_isa_info
 *              find_isa_info
 *              cache_cudv
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/cfgodm.h>         /* odm database definitions */
#include <odmi.h>               /* odm_* subroutine definitions */
#include <cf.h>                 /* E_xxxx error codes */
#include "isa_info.h"           /* ISA_info node definition */
#include "cfgdebug.h"           /* DEBUG_x declarations */
#include "cfgresid.h"           /* definition for CFG_DEVICE */

extern struct PdDv *find_PdDv();  /* get PdDv for residual data device */

struct ISA_info *isa_info_head;
struct ISA_info *isa_info_tail;
struct listinfo cudv_info;      /* cudv list information */
struct CuDv   *isa_cudv;            /* cudv list pointer */

/*
 *   NAME:      init_isa_info
 *
 *   FUNCTION:  Initializes module global data.
 *                - gets all customized device information
 *                  for subclass = "isa_sio" or "isa"
 *                - get all residual data device information
 *                  for detectable ISA devices.
 *
 *   INPUT:     logical name of parent bus
 *   OUTPUT:    global data listed above initialized
 *              pointer to list of cudv objects for isa_sio devices
 *              pointer to linked list of residual data devices
 *   RTN CODE:  E_OK            for successful return
 *              E_ODMGET        could not get CuDv odm device entry
 *
 */

int
init_isa_info(busname)
char *busname;
{
    /* general declarations */
    int i;       /* loop counter */

    /* declarations for customized information */
    struct CuDv   *ptr;
    char query_str[256];
    char subclass[16];
    char *temp_ptr;     /* temp pointer for string operations */

    /* declarations for residual data information */
    struct ISA_info *isa_info_cur;
    struct ISA_info *node;
    ulong           num_dev;      /* number of devices in residual data */
    int             num_isa;      /* number of devices in ISA info table*/
    CFG_DEVICE      *devp;        /* DEVICES - residual data */
    int rc;                     /* return codes for subroutine calls */

    DEBUG_1("cfgbus_isa: init_isa_info, busname = %s\n",busname);

    /*
     * get customized device information
     */

    sprintf(query_str, "PdDvLn LIKE '*/isa*/*' and parent = %s", busname);
    isa_cudv = (struct CuDv *) odm_get_list (CuDv_CLASS, query_str,
					 &cudv_info, 80, 1);

    if ( (int)isa_cudv == -1 ) {
	/* error occured */
	DEBUG_0("init_isa_info: error getting isa_cudv objects\n");
	return (E_ODMGET);
    }
    if ( isa_cudv == NULL ) {
	/* customized device table empty */
	DEBUG_0("cfgbus_isa: init_isa_info, no isa_cudv objects matching this query\n");
    }

#ifdef CFGDEBUG
    ptr = isa_cudv;
    DEBUG_1("cfgbus_isa: init_isa_info, %d devices found\n", cudv_info.num);
#endif
    /* null out devices I don't care about, I only want */
    /*  subclass = isa or isa_sio */
    for (i=0; i <  cudv_info.num; i++) {
	DEBUG_2("    device in loop = %s, uniquetype = %s, ",
		 isa_cudv[i].name,isa_cudv[i].PdDvLn_Lvalue);

	/* subclass is second item, read it */
	strcpy(query_str, isa_cudv[i].PdDvLn_Lvalue);
	temp_ptr = (char *) strtok ( query_str, "/");
	temp_ptr = (char *) strtok ( NULL, "/");
	strcpy(subclass,temp_ptr);

	DEBUG_1("subclass = %s\n", subclass);

	/* null out unwanted items */
	if ( (strcmp(subclass, INTG_SUBCLASS) != 0) &&
	     (strcmp(subclass, NINTG_SUBCLASS) != 0) ) {
	    isa_cudv[i].name[0] = '\0';
	}
    }

    /*
     * get residual device information for detectable ISA devices
     */

    isa_info_head = NULL;
    isa_info_cur = NULL;

    /* get residual device list and copy into isa_info array */

    /* get the number of devices and the device info */
    DEBUG_0("cfgbus_isa: init_isa_info, calling get_resid_dev\n");
    rc = get_resid_dev(&num_dev,&devp);
    DEBUG_1("cfgbus_isa: init_isa_info, get_resid_dev rc=%d\n",rc);
    if( rc != 0 ) {
	DEBUG_1("error calling get_resid_dev, rc = %d\n",rc);
	return (rc);
    }
    DEBUG_1("cfgbus_isa: init_isa_info, got %d resid devices\n",num_dev);

    /* copy over only the ISA devices, excluding
     *  - Reserved
     *  - Bridge Controller (excluding PCMCIA)
     *  - System Peripherals  (excluding Power Management)
     *  - Memory
     *  - Memory Controllers
     */
    num_isa = 0;
    for (i=0; i<num_dev; i++){

	/* get only ISA devices */
	if ( (devp+i)->deviceid.busid == ISADEVICE ) {

	    if ( ((devp+i)->deviceid.basetype != ISA_Reserved)  &&
		 ((devp+i)->deviceid.basetype != MemoryController)  ) {

		if ( (( (devp+i)->deviceid.basetype != SystemPeripheral) &&
		      ( (devp+i)->deviceid.basetype != BridgeController)) ||

		     (( (devp+i)->deviceid.basetype == SystemPeripheral) &&
		      ( (devp+i)->deviceid.subtype == PowerManagement) &&
		      ( (devp+i)->deviceid.interface == GeneralPowerManagement)) ||

		     (( (devp+i)->deviceid.basetype == BridgeController) &&
		      ( (devp+i)->deviceid.subtype == PCMCIABridge)) ) {

		     /* create new node */

		     node = (struct ISA_info *) malloc ( sizeof ( struct ISA_info ));
		     if ( node == NULL ) {
			 DEBUG_0("cfgbus_isa: init_isa_info, no node allocated\n");
			return (E_MALLOC);
		    }

		    DEBUG_1("cfgbus_isa: init_isa_info, node = %x\n",node);
		    node->next = NULL;

		    if ( isa_info_cur == NULL ) {
			isa_info_head = node;
		    }
		    else {
			isa_info_cur->next = node;
		    }
		    isa_info_cur = node;

		    /* initialize the new node */
		    isa_info_cur->resid = ( char * ) (devp+i);
		    isa_info_cur->resid_offset = i;
		    DEBUG_2("cfgbus_isa: init_isa_info, resid = %x, offset = %d\n",
			     isa_info_cur->resid,isa_info_cur->resid_offset);
		    isa_info_cur->pddv = find_PdDv(devp+i, i);
		    isa_info_cur->cudv = NULL;
		    isa_info_cur->config = 0;
		    /* set flag for integrated or non-integrated */
		    if ( ( (devp+i)->deviceid.flags & INTEGRATED ) == INTEGRATED ) {
			isa_info_cur->integrated = 1;
		    }
		    else {
			isa_info_cur->integrated = 0;
		    }
		    num_isa++;
		}
	    }
	}
    }

#ifdef CFGDEBUG
    prt_tables();       /* debug to print tables */
#endif

    isa_info_tail = isa_info_cur;
    DEBUG_0("cfgbus_isa: init_isa_info, done\n");

    return(E_OK);

}


#ifdef CFGDEBUG
/*
 *   NAME:      prt_tables
 *
 *   FUNCTION:  Prints global table data, debug only
 *
 *   INPUT:     none
 *   OUTPUT:    tables printed to stdout
 *   RTN CODE:  E_OK            for successful return
 *
 */

int
prt_tables()
{

    int i;
    struct ISA_info *node;
    CFG_DEVICE      *devp;        /* DEVICES - residual data */

    /* list cudv array */
    DEBUG_0("cfgbus_isa: prt_tables, cudv device list:\n");
    for (i=0; i<cudv_info.num; i++) {
	if ( isa_cudv[i].name[0] != '\0') {
	    DEBUG_2("    %12s  %5s    \n",
			 isa_cudv[i].name,isa_cudv[i].connwhere);
	    continue;
	}
    }

    DEBUG_0("cfgbus_isa: prt_tables, residual devices are:\n");
    node=isa_info_head;
    DEBUG_0("    device    resid    offset     pddv        cudv    intg cfg\n");
    while ( node != NULL ) {
	devp = ( CFG_DEVICE * ) node->resid;
	DEBUG_3("   %8s   %8x %4d",
		     devp->pnpid, devp, node->resid_offset);
	DEBUG_4("   %8x    %8x    %d    %d\n",
		     node->pddv, node->cudv, node->integrated,
			       node->config);
	node = node->next;
    }

}
#endif

/*
 *   NAME:      find_isa_info
 *
 *   FUNCTION:  Matches a cudv device entry to an entry in
 *              the residual device list based on the
 *              connwhere value.
 *              (Used only for devices in the residual data table.)
 *
 *   INPUT:     pointer to ISA info entry
 *   OUTPUT:    -
 *   RTN CODE:  offset into CuDv array that the residual device matches
 *              -1 - no entry was found in CuDv list
 *
 */

int find_isa_info( info_ptr  )
struct ISA_info *info_ptr;
{
    char connwhere[16]; /* residual connwhere value to match */
    int cudv_index;          /* CuDv offset */
    CFG_DEVICE      *devp;        /* DEVICES - residual data */

    DEBUG_0("cfgbus_isa: find_isa_info, start\n");

    devp = (CFG_DEVICE *) info_ptr->resid;
    sprintf(connwhere, "%s%x",
			devp->pnpid,
			devp->deviceid.serialnum);
    DEBUG_1("cfgbus_isa: find_isa_info, resid connwhere = %s\n",connwhere);

    for ( cudv_index=0; cudv_index<cudv_info.num; cudv_index++ ) {

	if ( isa_cudv[cudv_index].name[0] == '\0') {
	    continue;
	}

	DEBUG_1("                           cudv  connwhere = %s\n",
					    isa_cudv[cudv_index].connwhere);
	if ( strcmp(isa_cudv[cudv_index].connwhere, connwhere) == 0 ) {
	    /* connwhere values compare */
	    /* also verify uniquetypes */
	DEBUG_2("                           uniquetypes are %s and %s\n",
			isa_cudv[cudv_index].PdDvLn_Lvalue, info_ptr->pddv->uniquetype);
	    if ( strcmp(isa_cudv[cudv_index].PdDvLn_Lvalue, info_ptr->pddv->uniquetype) == 0 ) {
		/* uniquetypes also compare, it is good */
		DEBUG_1("            connwhere and uniquetype match ok, index = %d\n",cudv_index);
		break;
	    }
	}
    }

    DEBUG_1("cfgbus_isa: find_isa_info, done, offset = %d\n",cudv_index);
    /* if device not found, offset will = -1 */
    if ( cudv_index != cudv_info.num ) {
	/* entry found */
	return( cudv_index );
    }
    else {
	return( -1 );
    }
}


/*
 *   NAME:      cache_cudv
 *
 *   FUNCTION:  Stores the specified cudv entry in the
 *              specified ISA_info node, then nulls the
 *              previous cudv entry.  This is done so that
 *              the map_non_detectable function knows which
 *              cudv devices have been processed, and which
 *              have not.
 *
 *   INPUT:     isa_cudv array set up by init_isa_info()
 *              isa_ptr : pointer to ISA_info node the cudv is associated
 *                        with
 *              cudv_index : index into isa_cudv array to identify cudv entry
 *   OUTPUT:    ISA_info node cudv pointer is updated
 *   RTN CODE:  E_OK            successful return
 *              E_MALLOC        malloc error
 *
 */

int
cache_cudv ( isa_ptr, cudv_index )
struct ISA_info *isa_ptr;       /* pointer to isa node for cudv entry */
int cudv_index;                 /* index into cudv array to move */
{

    DEBUG_0("cfgbus_isa: cache_cudv, start\n");
    DEBUG_2("    isa ptr = %x, cudv device = %s\n",isa_ptr,isa_cudv[cudv_index].name);
    isa_ptr->cudv = (struct CuDv *) malloc ( sizeof (struct CuDv));
    if ( isa_ptr->cudv == NULL ) {
	DEBUG_0("cfgbus_isa: cache_cudv, malloc failed\n");
	return(E_MALLOC);
    }
    memcpy ( isa_ptr->cudv, &isa_cudv[cudv_index], sizeof (struct CuDv));
    DEBUG_1("    cudv copied for device %s\n",isa_ptr->cudv->name);

    /* set this CuDv.name to NULL so that we know it has been processed */
    isa_cudv[cudv_index].name[0] = '\0';

    DEBUG_0("cfgbus_isa: cache_cudv, done\n");
    return(E_OK);
}
