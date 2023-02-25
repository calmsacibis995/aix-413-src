static char sccsid[] = "@(#)12  1.7  src/rspc/usr/lib/methods/cfgbus_pci/adaptcfg.c, rspccfg, rspc41J, 9520A_all 5/11/95 16:10:01";
/*
 *   COMPONENT_NAME: rspccfg    adaptcfg.c - common bus code
 *
 *   FUNCTIONS: 
 *		load_kernel_mod
 *		mk_sp_file
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <cf.h>
#include <sys/device.h>
#include <sys/sysconfig.h>
#include <sys/iplcb.h>

#include "bcm.h"


/*
 * NAME     : load_kernel_mod
 * FUNCTION : Loads a kernel extension and initializes it with
 *            BUC structure.
 * INPUTS   :
 *      buc : the BUC structure for the bus device.
 *      mod_name : the load module name.
 *
 * RETURNS : 0     => successfully set "bus_id" attribute.
 *           other => error code indicating failure (capable of being
 *                    returned by the caller).
 */
int
load_kernel_mod(buc,mod_name)
struct	buc_info	*buc;		/* BUC structure for bus device */
char			*mod_name;	/* Load module name */

{
	mid_t    kmid;			/* KMID for DMA kernel extension */
        struct cfg_kmod km;            /* sysconfig cmd struct for kernel ext*/

	/****************************************
	 Call loadext() to load dma driver
	*****************************************/
	if ((kmid = loadext(mod_name, TRUE, FALSE)) == NULL) {
		/* error loading DMA driver */
		return(E_LOADEXT);
	}

	/*******************************************
	 Now call sysconfig() to initialize the kernel extension
	********************************************/
	km.mdiptr = (caddr_t)buc;
	km.kmid = kmid;
	km.mdilen = buc->struct_size;
	km.cmd = CFG_INIT;
	if (sysconfig(SYS_CFGKMOD, &km, sizeof(struct cfg_kmod))==-1) {
		return(E_CFGINIT);
	}

	return(0);
}


/* 
 * NAME     : mk_sp_file
 * FUNCTION : generates the special file for the bus. It selects       
 *            the minor number based on the bus number passed in
 *            (see below for more information).
 * INPUTS   :
 *      Fname  : file name to be created.
 *      BusNum : 1 byte bus number
 * NOTES    :
 *      Note that the minor number is tied to the bus 
 *      number as follows (this relationship is also 
 *      documented in sys/io/machdd/md.c for bos325 and
 *      src/bos/kernel/io/machdd/md.c for bos410) : 
 *       
 *       	 BUS_NUM    MINOR_NUMBER
 *        	  '0'	         0
 *        	  '1'           16
 *        	  '2'    	32
 *        	  '3'	        48
 * RETURNS : FALSE => could not create special file
 * 	     TRUE  => successfully created special file.
 */
int
mk_sp_file(Fname, bus_num)

char	*Fname;		/* name of special file to create (include /dev/) */
int	bus_num;	/* bus number                                     */

{
	int	rc;	/* return code returned by this routine.  */
	int	mode;	/* file mode mask used to create file.    */
	dev_t	devno;	/* device number to use when creating file*/
	int	minor;	/* device minor number                    */


	/* Compute device minor number and device number */
	minor = bus_num << 4;
	devno = (dev_t)(0x030000 | minor);   

	/* Make the bus special file */
	mode = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP ;
	rc = unlink(Fname) ;
	rc = (mknod(Fname, mode, devno) == 0) ;

	return(rc) ;
}

