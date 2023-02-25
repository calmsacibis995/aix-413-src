static char sccsid[] = "@(#)78	1.1  src/bos/usr/samples/fp/sigfpe_samp2.c, fpsamp, bos411, 9428A410j 9/8/93 10:22:22";
/*
 *   COMPONENT_NAME: BOSSAMP
 *
 *   FUNCTIONS: f_trap_sigfpe
 *		sigfpe_handler
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This demonstates a floating point exception handler,
 * and some glue code to set up floating point exception 
 * traping.  The glue code can be called from Fortran.
 */

#include <signal.h>
#include <fpxcp.h>
#include <fptrap.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * the fp_list structure will allow text descriptions
 * of each possible trap type to be tied to the mask
 * which identifies it.
 */

typedef struct 
  {
  fpflag_t mask;
  char     *text;
  } fp_list_t;

/* IEEE required trap types */

fp_list_t 
trap_list[] = 
  {
      { FP_INVALID,      "FP_INVALID"},      
      { FP_OVERFLOW,     "FP_OVERFLOW"},     
      { FP_UNDERFLOW,    "FP_UNDERFLOW"},    
      { FP_DIV_BY_ZERO,  "FP_DIV_BY_ZERO"},  
      { FP_INEXACT,      "FP_INEXACT"}      
  };

/* INEXACT detail list -- this is an IBM RISC System/6000 extension */

fp_list_t 
detail_list[] = 
  {
      { FP_INV_SNAN,    "FP_INV_SNAN" } ,     
      { FP_INV_ISI,    "FP_INV_ISI" } ,      
      { FP_INV_IDI,    "FP_INV_IDI" } ,      
      { FP_INV_ZDZ,    "FP_INV_ZDZ" } ,      
      { FP_INV_IMZ,    "FP_INV_IMZ" } ,      
      { FP_INV_CMP,    "FP_INV_CMP" } ,      
      { FP_INV_SQRT,   "FP_INV_SQRT" } ,     
      { FP_INV_CVI,    "FP_INV_CVI" } ,
      { FP_INV_VXSOFT, "FP_INV_VXSOFT" }
  };

/*
 * NAME: sigfpe_handler
 *
 * FUNCTION: a trap handler which is entered when
 *           a floating point exception occurs.  The
 *           functions determines what exception(s) caused
 *           the trap, prints this to stdout, and returns
 *           to the process which caused the trap.
 *
 * NOTES:    
 *           When entered, all floating point traps are 
 *           disabled.
 *
 *           This sample uses printf().  This should be used
 *           with caution since since printf() of a floating
 *           point number can cause a trap, and then a
 *           another printf() of a floating point number
 *           in the signal handler will corrupt the static
 *           buffer used for the conversion.
 *
 * OUTPUTS:  The type of exception(s) which caused
 *           the trap.
 */

static void 
sigfpe_handler(int sig,
	       int code,
	       struct sigcontext *SCP)
  {
  struct mstsave * state = &SCP->sc_jmpbuf.jmp_context;
  fp_sh_info_t flt_context;	/* structure for fp_sh_info() call */
  int i;			/* loop counter */
  
  /*
   * Determine which floating point exception(s) caused
   * the trap.  fp_sh_info() is used to build the floating signal
   * handler info  structure, and then the member flt_context.trap 
   * can be examined.  First the trap type is compared for the IEEE
   * required traps, and if the trap type is an invalid operation, 
   * the detail bits are examined.
   */
  
  fp_sh_info(SCP, &flt_context, FP_SH_INFO_SIZE);

  for (i = 0; i < (sizeof(trap_list) / sizeof(fp_list_t)); i++)
      {
      if (flt_context.trap & trap_list[i].mask)
	(void) printf("Trap caused by %s error\n", trap_list[i].text);
      }

  if (flt_context.trap & FP_INVALID)
      {
      for (i = 0; i < (sizeof(detail_list) / sizeof(fp_list_t)); i++)
	  {
	  if (flt_context.trap & detail_list[i].mask)
	    (void) printf("Type of invalid op is %s\n", detail_list[i].text);
	  }
      }

  /* report which trap mode was in effect */
  switch (flt_context.trap_mode)
      {
    case FP_TRAP_OFF:
      puts("Trapping Mode is OFF");
      break;
      
    case FP_TRAP_SYNC:
      puts("Trapping Mode is SYNC");
      break;
      
    case FP_TRAP_IMP:
      puts("Trapping Mode is IMP");
      break;
      
    case FP_TRAP_IMP_REC:
      puts("Trapping Mode is IMP_REC");
      break;
      
    default:
      puts("ERROR:  Invalid trap mode");
      }

  (void) fflush(stdout);

  /* clear exception bits to prevent recurrence of trap */
  fp_sh_set_stat(SCP, (flt_context.fpscr & ((fpstat_t) ~flt_context.trap)));

  /* 
   * increment the iar of the process that caused the trap, to prevent
   * re-execution of the instruction.  
   *
   * The FP_IAR_STAT bit in flt_context.flags indicates if state->iar points
   * to an instruction which has logically been started.  If this bit is true,
   * state->iar points to an operation which has been started, and if re-executed
   * will cause another exception.  In this case we want to continue execution and
   * ignore the results of that operation, so the iar is advanced to point to the
   * next instruction.  If the bit is false, the iar already points to the next
   * instruction which must be executed.
   */
  
  if ( flt_context.flags & FP_IAR_STAT )
      {
      puts("Increment IAR");
      state->iar += 4;
      }

  return;
  }
     
#define TRAP_OFF 0
#define TRAP_ON  1

/*
 * NAME: f_trap_sigfpe
 *
 * FUNCTION:  provide a sample interface for Fortran to
 *            enable/disable traping.
 *
 * DESCRIPTION:  The first argument is an action
 *               code.  If non-zero, the call will establish
 *               sigfpe_handler() as the trap handler for 
 *               for sigfpe, put the process in synchronous
 *               execution mode, and enable the trap(s) 
 *               specified by the second argument.
 *
 *               If the second argument is zero, the call
 *               will remove the sigfpe_handler (if installed),
 *               put the process in synchonous execution mode,
 *               and disable all traps.
 */

void 
f_trap_sigfpe(int *action, fptrap_t *trap_type)
  {
  struct sigaction response;
  static struct sigaction old_response;
  static int trap_status = TRAP_OFF;
  
  /* if action is zero, then turn off trapping, and 
   * also restore previous signal handler(s) if any.
   */
  
  if (! *action)
      {
      (void) fp_trap(FP_TRAP_OFF);
      fp_disable_all();
      if (trap_status)
	  {
	  trap_status = TRAP_OFF;
	  (void) sigaction(SIGFPE, &old_response, NULL);
	  }
      }
  else
      {
      if (! trap_status)
	  {
	  (void) sigaction(SIGFPE, NULL, &old_response);
	  (void) sigemptyset(&response.sa_mask);
	  response.sa_flags = FALSE;
	  response.sa_handler = (void (*)(int)) sigfpe_handler;
	  (void) sigaction(SIGFPE, &response, NULL);
	  (void) fp_trap(FP_TRAP_SYNC);
	  fp_enable(*trap_type);
          trap_status = TRAP_ON;
	  }
      }
  }
     
