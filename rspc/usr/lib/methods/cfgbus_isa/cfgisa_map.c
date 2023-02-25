static char sccsid[] = "@(#)08 1.8 src/rspc/usr/lib/methods/cfgbus_isa/cfgisa_map.c, rspccfg, rspc41J, 9523B_all 6/6/95 14:18:24";
/*
 *   COMPONENT_NAME: rspccfg
 *
 *   FUNCTIONS:
 *              map_residual
 *              remove_cudv
 *              map_non_detectable
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

#include <stdio.h>              /* stdout definition */
#include <sys/cfgodm.h>         /* odm database definitions */
#include <odmi.h>               /* odm_* subroutine definitions */
#include <cf.h>                 /* E_xxxx error codes */
#include <string.h>             /* string operations for strchr */
#include "isa_info.h"           /* ISA_info node definition */
#include "cfgdebug.h"           /* DEBUG_x declarations */
#include "cfgresid.h"           /* definition for CFG_DEVICE */

extern struct listinfo cudv_info;
extern struct CuDv   *isa_cudv;            /* cudv list information pointer */
extern int find_isa_info();                /* match cudv <> resid by connwhere*/
extern struct ISA_info *find_ioaddr();     /* match cudv <> resid by I/O addr*/
extern struct ISA_info *isa_info_head;
extern struct ISA_info *isa_info_tail;

extern int get_io_packets();            /* get I/O address packets */
extern  int     allpkg;                 /* packaging flag */


/*
 *
 *   NAME:      map_residual
 *
 *   FUNCTION:  Tries to match each residual ISA device in the
 *              isa_info list to a corresponding entry in the
 *              CuDv database.
 *
 *   INPUT:     isa_cudv and isa_info lists have been initialized
 *   OUTPUT:    isa_cudv pointers in isa_info list point to cudv objects for
 *              the residual devices that have CuDv entries.
 *              All residual isa_info devices will be marked to configure.
 *   RTN CODE:  0 - successful return
 *              E_ODMGET - error return
 *
 */
int
map_residual()
{
    /* general declarations */
    int i;          /* loop counter */
    int offset;       /* index into cudv array */
    struct ISA_info *ptr;       /* pointer to isa info nodes */

    char query_str[256];
    char subclass[16];
    char *temp_ptr;     /* temp pointer for string operations */

    DEBUG_0("cfgbus_isa: map_residual, start\n");

    /* for each device in isa_info list */
    /*    try to match to a cudv entry using connwhere values */
    ptr = isa_info_head;
    while ( ptr != NULL ) {

	/* process all integrated devices first */
	/* select on connwhere values */
	if (ptr->integrated == 1) {
	    /* find the CuDv entry corresponding to this connwhere */
	    offset = find_isa_info ( ptr );

	    if ( offset != -1 ) {
		/* this is a match, cache the cudv entry into isa_info */
		cache_cudv( ptr, offset );
		/* set flag indicating configure method should be run */
		ptr->config = 1;
	    }
	    else {
		/* no cudv exists, if pddv exists set config bit */
		/*   so it will get configured, if no pddv, then */
		/*   software must be installed.                 */
		if ( ptr->pddv ) {
		    ptr->config = 1;
		}
	    }
	}

	ptr = ptr->next;
    }

    /*********************************************************************
     * Delete any integrated devices left in the isa_cudv array.
     * They are no longer in residual data, so something has changed
     * and these devices should be deleted so new devices will
     * be created correctly.
     *********************************************************************/
    /* for all devices in isa_cudv */
    for (i=0; i<cudv_info.num; i++) {

	DEBUG_2("cfgbus_isa: map_residual, check device %s with connwhere = %s\n",
					   isa_cudv[i].name,
					   isa_cudv[i].connwhere);


	/* get subclass from uniquetype to use below */
	strcpy(query_str, isa_cudv[i].PdDvLn_Lvalue);
	temp_ptr = (char *) strtok ( query_str, "/");
	temp_ptr = (char *) strtok ( NULL, "/");
	strcpy(subclass,temp_ptr);

	DEBUG_1("subclass = %s\n", subclass);

	/* skip entries already processed, or non-integrated */
	/*     non-integrated means subclass=isa */
	if ( ( isa_cudv[i].name[0] == '\0') ||
	     ( strcmp(subclass, NINTG_SUBCLASS) == 0) ) {
	    continue;
	}

	DEBUG_1("cfgbus_isa: map_residual, deleting devices for %s\n",
						 isa_cudv[i].name);
	/* problem, unconfigure existing device */
	remove_cudv ( &isa_cudv[i] );

    }

    /*********************************************************************
     * Now process non-integrated devices
     * All remaining devices in the cudv list should be non-integrated.
     * Try and match these devices to residual data devices based on
     * I/O address.
     *********************************************************************/
    /* for all devices in isa_cudv */
    for (i=0; i<cudv_info.num; i++) {

	/* skip entries already processed */
	if ( isa_cudv[i].name[0] == '\0') {
	    continue;
	}
	ptr = find_ioaddr(&isa_cudv[i]);
	if ( ptr != NULL ) {
	    /* this is a match, cache the cudv entry into isa_info */
	    cache_cudv( ptr, i );
	    /* set flag indicating configure method should be run */
	    ptr->config = 1;
	}

    }

    /* set config bit for any remaining residual entries */
    /* that have software installed. */
    ptr = isa_info_head;
    while (ptr != NULL) {
	if (ptr->pddv) {
	    ptr->config = 1;
	}
	ptr = ptr->next;
    }

#ifdef CFGDEBUG
    prt_tables();       /* debug to print tables */
#endif

    DEBUG_0("cfgbus_isa: map_residual, done\n");
    return(0);
}



/*
 *
 *   NAME:      remove_cudv
 *
 *   FUNCTION:  Removes device from Cu* databases
 *              Nulls out logical name in cudv object
 *
 *   INPUT:
 *   OUTPUT:
 *   RTN CODE:  0 - successful return
 *              E_ODMDELETE - error return
 *
 */
int
remove_cudv ( cudv_ptr )
struct CuDv   *cudv_ptr;        /* cudv entry to remove from odm */
{

    char query_str[256];        /* temp string to store query criteria */
    int rc;                     /* return code for function calls */

    /* delete all the customized info for this device */
    sprintf(query_str, "name=%s", cudv_ptr->name);

    /* null out this entry so we know it has been processed */
    cudv_ptr->name[0] = '\0';

    rc = odm_rm_obj (CuDep_CLASS, query_str);
    if ( rc == -1 ) {
	    DEBUG_0("cfgbus_isa: map_residual, error removing CuDep entry\n");
	    return (E_ODMDELETE);
    }
    rc = odm_rm_obj (CuVPD_CLASS, query_str);
    if ( rc == -1 ) {
	    DEBUG_0("cfgbus_isa: map_residual, error removing CuVPD entry\n");
	    return (E_ODMDELETE);
    }
    rc = odm_rm_obj (CuAt_CLASS, query_str);
    if ( rc == -1 ) {
	    DEBUG_0("cfgbus_isa: map_residual, error removing CuAt entry\n");
	    return (E_ODMDELETE);
    }
    rc = odm_rm_obj (CuDv_CLASS, query_str);
    if ( rc == -1 ) {
	    DEBUG_0("cfgbus_isa: map_residual, error removing CuDv entry\n");
	    return (E_ODMDELETE);
    }
    /* not sure how to handle this one                 <<<<<
    odmdelete -q "name=xxx[j].name" CuDvDr;
    */

    return (0);

}


/*
 *   NAME:      map_non_detectable
 *
 *   FUNCTION:  create isa_info devices for remaining cudv
 *              devices in list.
 *
 *   INPUT:     isa_cudv and isa_info lists have been initialized
 *   OUTPUT:    isa_cudv pointers in isa_info list point to cudv objects for
 *              the integrated devices that have CuDv entries.
 *   RTN CODE:  0 - successful return
 *              E_x - error return
 *
 */

int
map_non_detectable()
{
    int i;              /* loop counter */
    struct ISA_info *node;
    int nimhost=0;

#ifdef CFGDEBUG
    /* declarations for debug */
    CFG_DEVICE      *devp;        /* DEVICES - residual data */
#endif

    DEBUG_0("cfgbus_isa: map_non_detectable, start\n");

    /* since the cudv array entries which have already been processed
     * are now NULL, run through the array looking for non-NULL entries
     */
    for ( i=0; i<cudv_info.num; i++ ) {

	if ( isa_cudv[i].name[0] == '\0') {
	    continue;
	}

	node = (struct ISA_info *) malloc ( sizeof ( struct ISA_info ));
	if ( node == NULL ) {
	    DEBUG_0("cfgbus_isa: init_isa_info, no node allocated\n");
	    return (E_MALLOC);
	}

	DEBUG_1("cfgbus_isa: init_isa_info, node = %x\n",node);

	/* initialize the new node */
	node->resid = NULL;
	node->pddv = NULL;
	node->cudv = NULL;
	node->config = 1;               /* set to call its configure method */
	node->integrated = 0;
	node->next = NULL;

	cache_cudv ( node, i );

	/* link it into the list */
	isa_info_tail->next = node;
	isa_info_tail = node;

    }
        /* check if environment variable set to install all packages */
        if (strlen((char *)getenv("NIM_HOSTNAME")) != 0)
                nimhost = 1;
	
	/* If allpkg is set, then install all isa packages that */
	/* are shipped on the installation media.               */
	if (allpkg && nimhost)
    		fprintf(stdout, ":%s ", "devices.isa");


#ifdef CFGDEBUG
    prt_tables();       /* debug to print tables */
#endif

    DEBUG_0("cfgbus_isa: map_non_detectable, done\n");
    return(0);
}


/*
 *   NAME:      find_ioaddr
 *
 *   FUNCTION:  gets residual data entry based on I/O addr matching cudv entry
 *
 *   INPUT:     isa_cudv and isa_info lists have been initialized
 *   OUTPUT:    isa_cudv pointers in isa_info list point to cudv objects for
 *              the integrated devices that have CuDv entries.
 *   RTN CODE:  ptr to isa_info array - successful return
 *              NULL - successful return, no match or error
 *
 */

struct ISA_info *find_ioaddr(cudv_ptr)
    struct CuDv *cudv_ptr;            /* pointer to cudv device list */
{

    int i,j;                    /* loop counters */
    int rc;                     /* subroutine return code */
    int num_packets;            /* number of packets returned on heap call */
    CFG_iopack_t   *iopack_p;   /* I/O Addr Residual Packet structure */
    struct CuAt *cuat;           /* customized attribute structure ptr */
    ulong   tst_ioaddr;
    int     cnt;
    char    *type_ptr;
    struct ISA_info *ptr;

#ifdef CFGDEBUG
    /* declarations for debug */
    CFG_DEVICE      *devp;        /* DEVICES - residual data */
#endif

    DEBUG_1("cfgisa_map: find_ioaddr, start for device %s\n",cudv_ptr->name);

    /* get i/o addr packets from odm */
    cuat = getattr(cudv_ptr->name, NULL, TRUE, &cnt);
    for (j=0; j<cnt; j++) {
	if (cuat[j].type[0] == 'O') {

	    type_ptr = strchr (cuat[j].type, '.');
	    if (type_ptr == NULL ) {
		tst_ioaddr = strtoul(cuat[j].value, (char **)NULL, 0);
		break;
	    }
	    else {
		if ( atol(type_ptr+1) == 1 ) {
		    tst_ioaddr = strtoul(cuat[j].value, (char **)NULL, 0);
		    break;
		}
	    }
	}
    }

    /* if no cudv attribute, return */
    if ( j == cnt)  return (NULL);

    /* for each non-integrated device in isa_info list */
    /*    try to match to a cudv entry using connwhere values */
    ptr = isa_info_head;
    while ( ptr != NULL ) {

	/* if device is non-integrated, get its i/o addr */
	if (ptr->integrated == 0) {

	    rc = get_io_packets(ptr->resid_offset, 'a', &num_packets,
			&iopack_p);

	    if ( rc != 0 ) {
		/* return error from subroutine */
#ifdef CFGDEBUG
		devp = (CFG_DEVICE *) ptr->resid;
		DEBUG_3("get_io_packets() failed with rc=%d for device %s at offset %d\n",
			rc, devp->pnpid, ptr->resid_offset);
#endif
		return (NULL);
	    }

	    /* if device has at least one I/O address packet, compare */
	    if (num_packets > 0 ) {

		/* test on I/O addr and uniquetype */
		if (((iopack_p)->min == tst_ioaddr) &&
		    (!strcmp(ptr->pddv->uniquetype,cudv_ptr->PdDvLn_Lvalue)) ) {

			return (ptr);
		}
	    }

	}

	ptr = ptr->next;

    }

    /* no entries found */
    return (NULL);

}
