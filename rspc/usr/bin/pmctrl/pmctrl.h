/* @(#)94       1.7  src/rspc/usr/bin/pmctrl/pmctrl.h, pwrcmd, rspc41J, 9519A_all 5/9/95 14:26:41 */
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS: Power Management Daemon
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------*
 * header files *
 *--------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <nl_types.h>
#include <locale.h>
#include <time.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <odmi.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <sys/pm.h>
#include <pmlib.h>

#include "pmctrl_msg.h"          /* created from .msg file */
#include "pmctrl_msg_default.h"  /* created from .msg file */

/*
 *  macro
 */
                                      /* for variable, 'ulong flag_status'   */
#define  V_FLAG   0x80000000          /* 1_______ ________ ________ ________ */
#define  B_FLAG   0x40000000          /* _1______ ________ ________ ________ */
#define  W_FLAG   0x20000000          /* __1_____ ________ ________ ________ */
#define  R_FLAG   0x10000000          /* ___1____ ________ ________ ________ */
#define  K_FLAG   0x08000000          /* ____1___ ________ ________ ________ */
#define  Y_FLAG   0x04000000          /* _____1__ ________ ________ ________ */
#define  A_FLAG   0x02000000          /* ______1_ ________ ________ ________ */
#define  L_FLAG   0x01000000          /* _______1 ________ ________ ________ */
#define  P_FLAG   0x00800000          /* ________ 1_______ ________ ________ */
#define  C_FLAG   0x00400000          /* ________ _1______ ________ ________ */
#define  U_FLAG   0x00200000          /* ________ __1_____ ________ ________ */
#define  X_FLAG   0x00100000          /* ________ ___1____ ________ ________ */
#define  E_FLAG   0x00080000          /* ________ ____1___ ________ ________ */
#define  T_FLAG   0x00040000          /* ________ _____1__ ________ ________ */
#define  LS_FLAG  0x00020000          /* ________ ______1_ ________ ________ */
#define  D_FLAG   0x00010000          /* ________ _______1 ________ ________ */
#define  G_FLAG   0x00008000          /* ________ ________ 1_______ ________ */
#define  LR_FLAG  0x00004000          /* ________ ________ _1______ ________ */
#define  S_FLAG   0x00002000          /* ________ ________ __1_____ ________ */
#define  H_FLAG   0x00001000          /* ________ ________ ___1____ ________ */

#define  CATNAME  "pmctrl.cat"        /* name of message catalog        */

                                      /* msg catolog string             */
#define MSGSTR(msgnum, string) catgets(catd, MS_PMCTRL, msgnum, string)

#define PMCTRL_ERROR -1
#define PMCTRL_SUCCESS  0

#define Free(pointer) if (pointer !=NULL){free(pointer);pointer=NULL;}

/*-----------------------*
 * Defines for Debugging *
 *-----------------------*/

#ifdef PM_DEBUG
#define Debug0(format)                fprintf(stderr,format)
#define Debug1(format,arg1)           fprintf(stderr,format,arg1)
#define Debug2(format,arg1,arg2)      fprintf(stderr,format,arg1,arg2)
#define Debug3(format,arg1,arg2,arg3) fprintf(stderr,format,arg1,arg2,arg3)
#else 
#define Debug0(format)                
#define Debug1(format,arg1)           
#define Debug2(format,arg1,arg2)      
#define Debug3(format,arg1,arg2,arg3) 
#endif /* PM_DEBUG */

/*---------------------* 
 *   Global variables  *
 *---------------------*/

extern int errno;
extern int   optind;
extern char *optarg;
extern int   optopt;

extern long int timezone;
extern int daylight;
extern char *tzname[];

nl_catd   catd;                            /* message catalog handle         */
int       in_smit_flag;                    /* if the command is executed     */
                                           /* from SMIT, this flag is set.   */

char    *b_arg, *w_arg, *r_arg, *k_arg, *y_arg, *s_arg;
char    *a_arg, *d_arg, *g_arg, *R_arg;
char    *S_args[2];
char    *t_args[3];


int rc;

int flag_status ; /* temporary */


/*------------------------*
 *  prototype definition  *
 *------------------------*/

int    get_arg(int,char **,int,char **);
int    set_on_off(char *arg, int cmd);
int    set_duration(char *arg, int cmd);
int    set_action(char *arg, int cmd);
int    set_permission(char *arg);
void   daily_alarm_localtime(time_t, int*, int*);
time_t daily_alarm_gmt(int, int);
int    set_pm_alarm(int cmd, char *actarg, char *targ);
int    query_pm_alarm(int cmd);
int    start_action(int action);
int    query_duration(int cmd);
int    chkarg_action(char *arg, int *val);
int    query_action_setting(int cmd);
int    query_permission_setting();
int    chkarg_duration(char *arg, int *result);
int    query_on_off_setting(int cmd);
int    chkarg_on_off(char *arg, int *result);
int    set_device_idle_time(char *lname, char *targ1, char *targ2, char *targ3);
int    query_device_idle_time(char *lname);
int    list_pm_dds();
int    print_current_state();
int    print_pm_setting();
void   long_usage();
void   short_usage();
void   bye();
ulong  parse(int ,char**);
void   process_h_flag(int);
void   process_v_flag(int);
void   process_a_flag(int);
void   process_d_flag(int);
void   process_e_flag(int);
void   process_lpcxu_flag(int);
void   process_tg_flag(int);
void   process_bwrkys_flag(int);
void   process_SR_flag(int);
int    most_significant_state(int);
int    find_graphical_output(char []);
int    check_time_value(struct tm *);
int    is_number(char *,int);
int    pmlib2pmstate(int);
int    pmlibonoff2boolean(int);
