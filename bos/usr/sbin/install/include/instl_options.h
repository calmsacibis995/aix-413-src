/* @(#)38  1.18  src/bos/usr/sbin/install/include/instl_options.h, cmdinstl, bos411, 9428A410j 5/19/94 19:12:44 */
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: instl_options.h
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_INSTL_OPTIONS
#define _H_INSTL_OPTIONS
#include <instl_define.h>
#include <inudef.h>

/* NAME: instl_options.h
 *
 *
 * FUNCTION: This header contains the structure used throughout the installp
 *           command and its initialization.
 *
 * FLAGS :
 *
 *   A             - list APAR's included in this update a apply the given lpps.
 *   B             - work with only the update images.
 *   b             - don't perform bosboot
 *   C             - cleanup after a failed install.
 *   c             - commit the given lpps.
 *   D             - delete the bff when install completed.
 *   d <device>    - device to be used during install.
 *   e <file>      - logfile to be used during install. 
 *   F             - ignore all prereqs and force installation.
 *   f             - file use file for a listing of the lpps
 *   g             - include all the requisites for the listed lpps.
 *   I             - use only the install images from list of lpps.
 *   i             - output the instructions for given lpps to stdout.
 *   J             - outputs messages and progress in SMIT format.
 *   k             - outputs size info in colon format for preview ops.
 *   L             - invoked from the GUI to list whats on the media
 *   l             - lists all the lpps available on the device.
 *   N             - NOSAVE of the replaced files.
 *   O {[r][s][u]} - portion of lpp to be installed.
 *   P             - matchs all previously installed lpps.
 *   p             - preview flag 
 *   Q             - suppress instreq failures
 *   q             - quiet restore mode.
 *   R             - force the reject of last applied lpp.
 *   r             - reject the given lpps.
 *   s             - list the status of all applied lpps and updates.
 *   T {k}         - override the cleanup.
 *   t {directory} - alternate save path prefix for files replaced during update
 *   u             - deinstall the given product [options].
 *   V {V0,V1,V2,V3,V4} - verbosity level (V1 is deflt, not specifiable by user)
 *   v             - verify the installation before completion.
 *   X             - expand filesystem as needed.
 *   z <blocks>    - sets block size for device */


/****************************
 * Define instl_opt structure
 ****************************/

struct FLAGS
{
   int   A_flag;      /* APAR listing */
   int   a_flag;      /* apply */
   int   B_flag;      /* update images only */
   int   b_flag;      /* don't do bosboot */
   int   C_flag;      /* cleanup */
   int   c_flag;      /* commit */
   int   D_flag;      /* delete bff when complete */
   int   d_flag;      /* device to use */
   char  d_arg[PATH_MAX + 1]; /* device specified */
   int   e_flag;      /* device to use */
   char  e_arg[PATH_MAX + 1]; /* device specified */
   int   F_flag;      /* force */
   int   f_flag;      /* file with list of lpps */
   char  f_arg[PATH_MAX + 1]; /* file name with list of lpps */
   int   g_flag;      /* get prereqs (include) */
   int   I_flag;      /* install images only */
   int   i_flag;      /* instructions listed to stdout */
   int   J_flag;      /* SMIT */
   int   k_flag;      /* colon format for size preview */
   int   L_flag;      /* invoked from the GUI to list whats on media*/
   int   l_flag;      /* list all avaliable options */
   int   N_flag;      /* NOSAVE */
   int   O_flag;      /* one piece to install */
   char  O_arg[MAX_PARTS + 1];        /* piece to install */
   int   O_root;      /* ROOT specified for install */
   int   O_usr;       /* USR specified for install */
   int   O_share;     /* SHARE specified for install */
   int   P_flag;      /* previously installed lpp match */
   int   p_flag;      /* preview function */
   int   Q_flag;      /* suppress instreq failures */
   int   q_flag;      /* quiet mode on restore */
   int   r_flag;      /* reject */
   int   s_flag;      /* status listing of applied options */
   char  T_arg[2];    /* for no cleanup flag parsing */
   int   T_argk;      /* no cleanup */
   int   t_flag;      /* reassign save directory */
   char	 t_arg[PATH_MAX + 1];   /* alternate save directory */
   int   u_flag;      /* deinstall */
   int   v_flag;      /* verify installation */
   V_flag_type
         V_flag;      /* verbosity level */
   int   X_flag;      /* expand as needed */
   int   z_flag;      /* block siZe for backup media */
   int   z_arg;       /* actual value of the siZe */

   char  lpplist[PATH_MAX + 1];    /* the file containing the list of lpps */
   char  lpp_all;                  /* "all" was selected for lpp */
   char  needs_lpps;               /* option need a lpp list? */
   char  needs_device;             /* option need a device? */
   char  inu_tmpdir[L_tmpnam + 1]; /* temporary directory used throughout */
   int   inu_lock_id_u;            /* lock for SPOT installs */
   int   inu_lock_id_m;            /* lock for ROOT installs */
   int   inu_lock_id_s;            /* lock for SHARE installs */
   int   inu_err_count;            /* count of non-fatel errors */
   int   inu_abort_sev;            /* severity level for current operation */
   char  inu_libdir[PATH_MAX + 1];
   char  inu_command[PATH_MAX + 1];
};

#ifdef MAIN_PROGRAM
struct FLAGS   instl_opt = { /* intialize the structure */
   FALSE,        /* A_flag */
   FALSE,        /* a_flag */
   FALSE,        /* B_flag */
   FALSE,        /* b_flag */
   FALSE,        /* C_flag */
   FALSE,        /* c_flag */
   FALSE,        /* D_flag */
   FALSE,        /* d_flag */
   "/dev/rfd0",  /* d_arg  */
   FALSE,        /* e_flag */
   "",           /* e_arg  */
   FALSE,        /* F_flag */
   FALSE,        /* f_flag */
   "",           /* f_arg  */
   FALSE,        /* g_flag */
   FALSE,        /* I_flag */
   FALSE,        /* i_flag */
   FALSE,        /* J_flag */
   FALSE,        /* k_flag */
   FALSE,        /* L_flag */
   FALSE,        /* l_flag */
   FALSE,        /* N_flag */
   FALSE,        /* O_flag */
   "",           /* O_arg  */
   FALSE,        /* O_root */
   FALSE,        /* O_usr  */
   FALSE,        /* O_share */
   FALSE,        /* P_flag */
   FALSE,        /* p_flag */
   FALSE,        /* Q_flag */
   FALSE,        /* q_flag */
   FALSE,        /* r_flag */
   FALSE,        /* s_flag */
   "",           /* T_arg  */
   FALSE,        /* T_argk */
   FALSE,        /* t_flag */
   "",           /* t_arg */
   FALSE,        /* u_flag */
   FALSE,        /* v_flag */
   V1,           /* V_flag */
   FALSE,        /* X_flag */
   FALSE,        /* z_flag */
   512,          /* z_arg */

   "",           /* lpplist */
   FALSE,        /* lpp_all */
   TRUE,         /* needs_lpps */
   TRUE,         /* needs_device */
   "",           /* inu_tmpdir */
   0,            /* inu_lock_id_u */
   0,            /* inu_lock_id_m */
   0,            /* inu_lock_id_s */
   0,            /* inu_err_count */
   0,            /* inu_abort_sev */
   "",           /* inu_libdir */
   "",           /* inu_command */

};/* end intialization */

#else
extern struct FLAGS instl_opt;

#endif

#endif  /* _H_INSTL_OPTIONS */
