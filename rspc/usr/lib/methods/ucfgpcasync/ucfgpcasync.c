static char sccsid[] = "@(#)31  1.2  src/rspc/usr/lib/methods/ucfgpcasync/ucfgpcasync.c, pcmciafax, rspc41J, 9512A_all 3/20/95 22:13:16";
/*
 * COMPONENT_NAME: pcmciafax
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 27, 83
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <stdlib.h>         /* standard C library */
#include <sys/types.h>
#include <errno.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

#include <sys/str_tty.h>

#include "cfgdebug.h"
#include "ttycfg.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */

/*
 * ==============================================================================
 * External functions declarations
 * ==============================================================================
 */
extern mid_t loadext();

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       ERR_EXIT
 * -----------------------------------------------------------------------------
 * 
 * Closes any open object classes and terminates ODM.
 * Used to back out on an error.
 *
 * This routine is to be used only within this file.
 * The device specific routines for the various device
 * specific config methods  must not call this function.
 *
 * Return code: None
 * -----------------------------------------------------------------------------
 */
static void err_exit(exitcode)
int    exitcode;
{
    /* Close any open object class */
    odm_close_class(CuDv_CLASS);
    odm_close_class(PdDv_CLASS);
    odm_close_class(CuAt_CLASS);
    odm_close_class(PdAt_CLASS);

    /* Terminate the ODM */
    odm_terminate();
    exit(exitcode);
} /* End static void err_exit(...) */

/*
 * =============================================================================
 *                       MAIN
 * =============================================================================
 * 
 * This process is executed to "unconfigure" an adapter device in streams environment.
 * 
 * This process is invoked when an adapter device instance is to be 
 * "unconfigured". This involves terminating the device to the 
 *    driver and representing the unconfigured state of the device 
 *    in the database. It may also unload the appropriate device driver 
 *    if the device is the last configured device for the driver.
 *
 * Interface: ucfgpcasync -l <logical_name>
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
main(argc, argv, envp)
     int    argc;
     char    *argv[];
     char    *envp[];
{
    extern int rsadapdds();
    extern int lionadapdds();

    char * logical_name;              /* logical name to configure */
    char   sstring[256];              /* search criteria pointer */
    long   majorno;                   /* major number */
    long   minorno;                   /* minor number */
    int    return_code, how_many;     /* return code and counter */
    int    current_opt, option_error; /* used in parsing parameters */

    struct cfg_dd  cfg;               /* sysconfig command structure for dd */

    /* ODM structures declarations */
    struct Class * cus_dev_class;     /* customized devices class ptr */
    struct Class * pre_dev_class;     /* predefined devices class ptr */

    struct CuDv    child_cus_dev;  /* customized device object storage */
    struct CuDv    cus_dev;        /* customized adapter device object */
    struct PdDv    pre_dev;        /* predefined device object storage */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    option_error = 0;
    logical_name = NULL;

    /* ================ */
    /* Parse Parameters */
    /* ================ */
    while ((current_opt = getopt(argc,argv,"l:")) != EOF) {
        switch (current_opt) {
          case 'l':
            if (logical_name != NULL) {
                option_error++;
            };
            logical_name = optarg;
            break;
            
          default:
            option_error++;
        }
    }
    if (option_error) {
        /* error parsing parameters */
        DEBUG_0("ucfgpcasync: command line error\n");
        exit(E_ARGS);
    };

    /* ========================================================= */
    /* Check command-line parameters for validity */
    /* For unconfig methods this includes making sure that a */
    /*logical name was specified. */
    /* The customized devices object class must also be searched */
    /* to see if the logical name is valid. */
    /* ========================================================= */
    if (logical_name == NULL) {
        DEBUG_0("ucfgpcasync: no logical name\n");
        exit(E_LNAME);
    };

    /* start up odm */
    if (odm_initialize() == -1) {
        DEBUG_0("ucfgpcasync: bad odm initialization\n");
        exit(E_ODMINIT);
    };

    /* open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("ucfgpcasync: CuDv open class error\n");
        err_exit(E_ODMOPEN);
    };

    /* ================================================= */
    /* Get the Customized Devices Object for this device */
    /* ================================================= */
    sprintf(sstring, "name = '%s'", logical_name);
    return_code = (int)odm_get_obj(cus_dev_class, sstring, &cus_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1 ("ucfgpcasync: CuDv %s found no objects.\n",sstring);
        err_exit (E_NOCuDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1 ("ucfgpcasync: get_obj failed, %s.\n", sstring);
        err_exit (E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default: /* odm object found ==> That's OK */
        DEBUG_1 ("ucfgpcasync: CuDv %s found objects.\n",sstring);
    } /* End switch (return_code) */

    /* ===================================================== */
    /* If device state is DEFINED, then already unconfigured */
    /* so just exit. */
    /* ===================================================== */
    if (cus_dev.status == (short) DEFINED) {
        DEBUG_0("ucfgpcasync: Device is already in DEFINED state\n");
        err_exit(E_OK);
    };

    /* ======================================================= */
    /* See if this device has AVAILABLE children.  If it does, */
    /* it can not be unconfigured. */
    /* ======================================================= */
    sprintf(sstring, "parent = '%s' AND status != %d",
            logical_name, DEFINED);
    return_code = (int)odm_get_first(cus_dev_class, sstring, &child_cus_dev);
    switch (return_code) {
      case 0: /* No CuDv object found ==> That's OK */
        DEBUG_1("ucfgpcasync: %s has no configured children\n",
                logical_name);
        break;
        
      case -1: /* odm error occurred */
        DEBUG_1("ucfgpcasync: get_obj failed, %s.\n", sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default: /* odm object found */
        DEBUG_2 ("ucfgpcasync: %s has configured children: %s.\n",
                 logical_name, child_cus_dev.name);
        err_exit(E_CHILDSTATE);
    } /* End switch (return_code) */

    /* ================================================================= */
    /* Get the predefined object for this device. This object is located */
    /* searching the predefined devices object class based on the unique */
    /* type link descriptor in the customized device. */
    /* ================================================================= */

    /* open predefined devices object class (PdDv) */
    if ((int)(pre_dev_class = odm_open_class(PdDv_CLASS)) == -1) {
        DEBUG_0("ucfgpcasync: PdDv open class error\n");
        err_exit (E_ODMOPEN);
    };

    /* search Predefined devices for object with this unique type */
    sprintf(sstring, "uniquetype = '%s'", cus_dev.PdDvLn_Lvalue);
    return_code = (int)odm_get_first(pre_dev_class, sstring, &pre_dev);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1 ("ucfgpcasync: PdDv crit=%s found no objects.\n",sstring);
        err_exit (E_NOPdDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1 ("ucfgpcasync: get_obj failed, crit=%s.\n",sstring);
        err_exit (E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default:  /* odm object found ==> That's OK */
        DEBUG_1 ("ucfgpcasync: PdDv uniquetype = %s found objects.\n",
                 cus_dev.PdDvLn_Lvalue);
    } /* End switch (return_code) */
    
    /* ================================= */
    /* Unconfigure the associated driver */
    /* ================================= */
    /* An associated driver is needed */
    if (!strcmp(pre_dev.DvDr, "")) {
        DEBUG_0("ucfgpcasync: No associated driver is found\n");
        err_exit(E_LNAME);
    };
    
    if ((majorno = genmajor(pre_dev.DvDr)) == (long) -1) {
        /* error allocating major number */
        DEBUG_1("ucfgpcasync: genmajor fails for driver %s\n",
                pre_dev.DvDr);
        err_exit(E_MAJORNO);
    };
    
    /* CFG.KMID */
    if ((cfg.kmid = loadext(pre_dev.DvDr, FALSE, TRUE)) == (mid_t) NULL) {
        DEBUG_1("ucfgpcasync: Unable to get cfg.kmid for %s\n",
                pre_dev.DvDr);
        err_exit(E_UNLOADEXT);
    } /* End if ((cfg.kmid = loadext(...)) == (mid_t) NULL) */

    /* minor number is the socket number of PCMCIA */
    minorno = cus_dev.connwhere[0] - '0' - 1 ;

    /* CFG.DEVNO */
    cfg.devno = makedev(majorno, minorno);
    DEBUG_1("ucfgpcasync: devno = %x\n", cfg.devno);
    
    /* CFG.CMD */
    cfg.cmd = CFG_TERM;
    
    /* Build the DDS */
#if 0
    /* CFG.DDSPTR and CFG.DDSLEN */
    /* Only the which_dds and name fields are required */
    if (!strcmp(pre_dev.DvDr, RS_DRIVER)){
        DEBUG_0("ucfgpcasync: Trying to build DDS\n");
        if (return_code = rsadapdds(&cus_dev, &cfg.ddsptr, &cfg.ddslen)) {
            /* error building dds */
            DEBUG_1("ucfgpcasync: error building dds for %s\n", logical_name);
            err_exit (return_code);
        };
    }
    else if (!strcmp(pre_dev.DvDr, LION_DRIVER)) {
        DEBUG_0("ucfgpcasync: Trying to build DDS\n");
        if (return_code = lionadapdds(&cus_dev, &cfg.ddsptr, &cfg.ddslen)) {
            /* error building dds */
            DEBUG_1("ucfgpcasync: error building dds for %s\n", logical_name);
            err_exit (return_code);
        };
    }
    else {
        /* Unsupported driver */
        DEBUG_1("ucfgpcasync: driver %s isn't supported for this configure method.\n",
                pre_dev.DvDr);
        err_exit(E_INVATTR);
    }
#endif

    DEBUG_1("ucfgpcasync: Trying to unconfigure %s driver\n", pre_dev.DvDr);
    DEBUG_1("ucfgpcasync: cfg.kmid = 0x%x\n", cfg.kmid);
    DEBUG_1("ucfgpcasync: cfg.devno = 0x%x\n", cfg.devno);
    DEBUG_1("ucfgpcasync: cfg.cmd = 0x%x\n", cfg.cmd);
    DEBUG_1("ucfgpcasync: cfg.ddsptr = 0x%x\n", cfg.ddsptr);
    DEBUG_1("ucfgpcasync: cfg.ddslen = 0x%x\n", cfg.ddslen);
    if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd)) == -1) { 
        /* error configuring driver */ 
        DEBUG_1("ucfgpcasync: errno = %d\n",errno); 
		/* make sure the device termination for PCMCIA */
        /* device is busy. */ 
	    err_exit(E_BUSY);
    }
    DEBUG_0("ucfgpcasync: Unconfiguration complete\n");
    
    /* ========================================= */
    /* Call loadext() to unload device driver. */
    /* loadext() will unload the driver only if */
    /* device is last instance of driver configured. */
    /* ========================================= */
    DEBUG_1("ucfgpcasync: Unloading the driver: %s\n", pre_dev.DvDr);
    /* Test if driver is really loaded */
    if (loadext(pre_dev.DvDr, FALSE, FALSE) == (mid_t) NULL) {
        /* error unloading device driver */
        DEBUG_0("ucfgpcasync: error unloading driver\n");
        err_exit(E_UNLOADEXT);
    };
    DEBUG_0("ucfgpcasync: Unloaded driver\n");

    /* ============================================== */
    /* Change the status field of device to "DEFINED" */
    /* ============================================== */
    DEBUG_0("ucfgpcasync: Updating device state\n");
    cus_dev.status = (short)DEFINED;

    /* Update the Customized Object of the device */
    if ((return_code = odm_change_obj(cus_dev_class, &cus_dev)) < 0) {
        /* change object failed */
        DEBUG_1("ucfgpcasync: update %s failed.\n", logical_name);
        err_exit(E_ODMUPDATE);
    };

    /* close object classes */
    if (odm_close_class(pre_dev_class) < 0) {
        err_exit(E_ODMCLOSE);
    };
    if (odm_close_class(cus_dev_class) < 0) {
        err_exit(E_ODMCLOSE);
    };
    
    /* ======================================================== */
    /* Unconfig method is finished at this point. Terminate the */
    /* ODM, and exit with a good return code. */
    /* ======================================================== */
    odm_terminate();
    return(E_OK);
    
} /* End main(...) */
