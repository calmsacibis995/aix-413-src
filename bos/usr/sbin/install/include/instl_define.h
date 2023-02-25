/* @(#)00  1.16  src/bos/usr/sbin/install/include/instl_define.h, cmdinstl, bos411, 9428A410j 5/19/94 19:11:56 */
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: instl_define.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_INSTL_DEFINE
#define _H_INSTL_DEFINE


/*
 * NAME: instl_define.h
 *
 * FUNCTION: This header contains all the info needed for external
 *           access to the option structure and parsing, including
 *           Usage message defaults.
 */

/* FLAGS :
 *
 * A             list APAR's included in this update
 * a             apply the given lpps
 * b             don't do bosboot
 * B             work with only the update images
 * C             cleanup after a failed install
 * c             commit the given lpps
 * D             delete the bff when install completed
 * d device      device to be used during install
 * e file        logfile to be used during install.
 * F             ignore all prereqs and force installation
 * f file        use file for a listing of the lpps
 * g             include all the prereqs for the listed lpps
 * I             use only the install images from list of lpps
 * i             output the instructions for given lpps to stdout
 * J             outputs messages and progress in SMIT format
 * L             invoked from the GUI to list whats on the media
 * l             lists all the lpps available on the device
 * N             NOSAVE of the replaced files
 * O {[r][s][u]} portion of lpp to be installed
 * P             matchs all previously installed lpps
 * p             preview flag 
 * Q             suppress instreq failures
 * q             quiet restore mode 
 * R             force the reject of last applied lpp
 * r             reject the given lpps
 * s             list the status of all applied lpps and updates
 * T {k}         override the cleanup
 * t directory   save replaced files beneath alternate location
 * u             deinstall given product options
 * v             verify the installation before completion
 * V {V0,V1,V2,V3,V4}  verbosity level (V1 is default, not specifiable by user)
 * X             expand filesystem as needed
 * z blocks      sets blocking siZe for device
 */

/*******************************
 * Fastpath's into the structure
 *******************************/

#define A_FLAG      instl_opt.A_flag
#define a_FLAG      instl_opt.a_flag
#define B_FLAG      instl_opt.B_flag
#define b_FLAG      instl_opt.b_flag
#define C_FLAG      instl_opt.C_flag
#define c_FLAG      instl_opt.c_flag
#define D_FLAG      instl_opt.D_flag
#define d_FLAG      instl_opt.d_flag
#define DEVICE      instl_opt.d_arg
#define e_FLAG      instl_opt.e_flag
#define LOGFILE     instl_opt.e_arg 
#define F_FLAG      instl_opt.F_flag
#define f_FLAG      instl_opt.f_flag
#define USER_FILE   instl_opt.f_arg
#define g_FLAG      instl_opt.g_flag
#define I_FLAG      instl_opt.I_flag
#define i_FLAG      instl_opt.i_flag
#define J_FLAG      instl_opt.J_flag
#define k_FLAG      instl_opt.k_flag
#define L_FLAG      instl_opt.L_flag
#define l_FLAG      instl_opt.l_flag
#define N_FLAG      instl_opt.N_flag
#define O_FLAG      instl_opt.O_flag
#define O_ARG       instl_opt.O_arg
#define ROOT_PART   instl_opt.O_root
#define USR_PART    instl_opt.O_usr
#define SHARE_PART  instl_opt.O_share

#define P_FLAG      instl_opt.P_flag
#define p_FLAG      instl_opt.p_flag

#define Q_FLAG      instl_opt.Q_flag
#define q_FLAG      instl_opt.q_flag
#define R_FLAG      instl_opt.r_flag
#define r_FLAG      instl_opt.r_flag
#define s_FLAG      instl_opt.s_flag
#define TRACE_ARG   instl_opt.T_arg
#define NO_CLEANUP  instl_opt.T_argk
#define t_FLAG      instl_opt.t_flag
#define t_ARG       instl_opt.t_arg
#define t_SAVE_DIR  instl_opt.t_arg
#define u_FLAG      instl_opt.u_flag
#define v_FLAG      instl_opt.v_flag
#define V_FLAG      instl_opt.V_flag
#define X_FLAG      instl_opt.X_flag
#define z_FLAG      instl_opt.z_flag
#define BLOCK_SIZE  instl_opt.z_arg

#define LPPLIST	      instl_opt.lpplist
#define LPP_ALL       instl_opt.lpp_all
#define NEEDS_LPPS    instl_opt.needs_lpps
#define NEEDS_DEVICE  instl_opt.needs_device
#define INU_TMPDIR    instl_opt.inu_tmpdir
#define INU_LOCK_ID_U instl_opt.inu_lock_id_u
#define INU_LOCK_ID_M instl_opt.inu_lock_id_m
#define INU_LOCK_ID_S instl_opt.inu_lock_id_s
#define INU_ERR_COUNT instl_opt.inu_err_count
#define INU_ABORT_SEV instl_opt.inu_abort_sev
#define INU_LIBDIR    instl_opt.inu_libdir
#define INU_COMMAND   instl_opt.inu_command

#endif /* _H_INSTL_DEFINE */
