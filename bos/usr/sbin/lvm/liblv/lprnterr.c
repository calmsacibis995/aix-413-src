static char sccsid[] = "@(#)22	1.13.1.1  src/bos/usr/sbin/lvm/liblv/lprnterr.c, cmdlvm, bos411, 9428A410j 7/21/92 12:19:02";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: errmsg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * FILENAME: lprnterr.c
 *
 * FILE DESCRIPTION: Source file for common function used by all the
 *      low level Logical Volume Manager programs.
 *
 * FUNCTION_NAMES: errmsg
 *
 */



#include <stdio.h>
#include <lvm.h>

/*  include file for message numbers */
#include "cmdlvm_msg.h"


/* Externals */
extern char *Prog;  /* set by higher level - the program name */
extern char *lvm_msg();


/*
 * NAME: errmsg
 *
 * FUNCTION: Common error message routine for low level lvm programs.
 *     "errnum" is the return code from a lvm library function.
 *
 * RETURN VALUE DESCRIPTION: zero is always returned
 */

int errmsg(errnum)

int errnum;   /* the error number */
{
  switch (errnum) {
      case 0:         /* No error occurred */
	  break;
      case LVM_VGDELETED:
	  fprintf(stderr, lvm_msg(VGDELETED_LVM), Prog);
	  break;
      case LVM_CHKVVGOUT:
	  fprintf(stderr, lvm_msg(CHKVVGOUT_LVM), Prog);
	  break;
      case LVM_REMRET_INCOMP:
	  fprintf(stderr, lvm_msg(REMRET_INCOMP_LVM), Prog);
	  break;
      case LVM_OFFLINE:
	  fprintf(stderr, lvm_msg(OFFLINE_LVM), Prog);
	  break;
      case LVM_LVOPEN:
	  fprintf(stderr, lvm_msg(LVOPEN_LVM), Prog);
	  break;
      case LVM_VGMEMBER:
	  fprintf(stderr, lvm_msg(VGMEMBER_LVM), Prog);
	  break;
      case LVM_PARTFND:
	  fprintf(stderr, lvm_msg(PARTFND_LVM), Prog);
	  break;
      case LVM_VGFULL:
	  fprintf(stderr, lvm_msg(VGFULL_LVM), Prog);
	  break;
      case LVM_LVEXIST:
	  fprintf(stderr, lvm_msg(LVEXISTLVM), Prog);
	  break;
      case LVM_INVALID_PARAM:
	  fprintf(stderr, lvm_msg(INVAL_PARAM_LVM), Prog);
	  break;
      case LVM_PVOPNERR:
	  fprintf(stderr, lvm_msg(PVOPNERR_LVM), Prog);
	  break;
      case LVM_NOALLOCLP:
	  fprintf(stderr, lvm_msg(NOALLOCLP_LVM), Prog);
	  break;
      case LVM_MAPFOPN:
	  fprintf(stderr, lvm_msg(MAPFOPN_LVM), Prog);
	  break;
      case LVM_LPNUM_INVAL:
	  fprintf(stderr, lvm_msg(LPNUM_INVAL_LVM), Prog);
	  break;
      case LVM_PPNUM_INVAL:
	  fprintf(stderr, lvm_msg(PPNUM_INVAL_LVM), Prog);
	  break;
      case LVM_DALVOPN:
	  fprintf(stderr, lvm_msg(DALVOPNLVM), Prog);
	  break;
      case LVM_INVALID_MIN_NUM:
	  fprintf(stderr, lvm_msg(INVAL_MIN_NUM_LVM), Prog);
	  break;
      case LVM_PVDAREAD:
	  fprintf(stderr, lvm_msg(PVDAREAD_LVM), Prog);
	  break;
      case LVM_PVSTATE_INVAL:
	  fprintf(stderr, lvm_msg(PVSTATE_INVAL_LVM), Prog);
	  break;
      case LVM_MAPFRDWR:
	  fprintf(stderr, lvm_msg(MAPFRDWR_LVM), Prog);
	  break;
      case LVM_NODELLV:
	  fprintf(stderr, lvm_msg(NODELLV_LVM), Prog);
	  break;
      case LVM_PVMAXERR:
	  fprintf(stderr, lvm_msg(PVMAXERR_LVM), Prog);
	  break;
      case LVM_VGDASPACE:
	  fprintf(stderr, lvm_msg(VGDASPACE_LVM), Prog);
	  break;
      case LVM_NOQUORUM:
	  fprintf(stderr, lvm_msg(NOQUORUM_LVM), Prog);
	  break;
      case LVM_MISSPVNAME:
	  fprintf(stderr, lvm_msg(MISSPVNAME_LVM), Prog);
	  break;
      case LVM_MISSINGPV:
	  fprintf(stderr, lvm_msg(MISSINGPV_LVM), Prog);
	  break;
      case LVM_ALLOCERR:
	  fprintf(stderr, lvm_msg(ALLOCERR_LVM), Prog);
	  break;
      case LVM_RDPVID:
	  fprintf(stderr, lvm_msg(RDPVID_LVM), Prog);
	  break;
      case LVM_LVMRECERR:
	  fprintf(stderr, lvm_msg(LVMRECERR_LVM), Prog);
	  break;
      case LVM_WRTDAERR:
	  fprintf(stderr, lvm_msg(WRTDAERR_LVM), Prog);
	  break;
      case LVM_NOTVGMEM:
	  fprintf(stderr, lvm_msg(NOTVGMEM_LVM), Prog);
	  break;
      case LVM_NOTSYNCED:
	  fprintf(stderr, lvm_msg(NOTSYNCED_LVM), Prog);
	  break;
      case LVM_PROGERR:
	  fprintf(stderr, lvm_msg(PROGERR_LVM), Prog);
	  break;
      case LVM_BADBBDIR:
	  fprintf(stderr, lvm_msg(BADBBDIR_LVM), Prog);
	  break;
      case LVM_INRESYNC:
	  fprintf(stderr, lvm_msg(INRESYNC_LVM), Prog);
	  break;
      case LVM_INVLPRED:
	  fprintf(stderr, lvm_msg(INVLPRED_LVM), Prog);
	  break;
      case LVM_INVCONFIG:
	  fprintf(stderr, lvm_msg(INVCONFIGLVM), Prog);
	  break;
      case LVM_NOTCHARDEV:
	  fprintf(stderr, lvm_msg(NOTCHARDEVLVM), Prog);
	  break;
      case LVM_INV_DEVENT:
	  fprintf(stderr, lvm_msg(INVDEVENTLVM), Prog);
	  break;
      case LVM_BELOW_QRMCNT:
	  fprintf(stderr, lvm_msg(BELOW_QRMCNT_LVM), Prog);
	  break;
      case LVM_VGDA_BB:
	  fprintf(stderr, lvm_msg(VGDA_BB_LVM), Prog);
	  break;
      case LVM_MIGRATE_FAIL:
	  fprintf(stderr, lvm_msg(MIGRATE_FAIL_LVM), Prog);
	  break;
      case LVM_MEMACTVVG:
	  fprintf(stderr, lvm_msg(MEMACTVVG_LVM), Prog);
	  break;
      case LVM_FORCEOFF:
	  fprintf(stderr, lvm_msg(FORCEOFF_LVM), Prog);
	  break;
      case LVM_NOVGDAS:
	  fprintf(stderr, lvm_msg(NOVGDAS_LVM), Prog);
	  break;
      case LVM_ALRDYMEM:
	  fprintf(stderr, lvm_msg(ALRDYMEM_LVM), Prog);
	  break;
      case LVM_MAPFBSY:
	  fprintf(stderr, lvm_msg(MAPFBSY_LVM), Prog);
	  break;
      case LVM_VONOTHMAJ:
	  fprintf(stderr, lvm_msg(VONOTHMAJ_LVM), Prog);
	  break;
      case LVM_BADVERSION:
	  fprintf(stderr, lvm_msg(BADVERSION_LVM), Prog);
	  break;
      case LVM_RESYNC_FAILED:
	  fprintf(stderr, lvm_msg(RESYNC_FAILED_LVM), Prog);
	  break;
      case LVM_LOSTPP:
	  fprintf(stderr, lvm_msg(LOSTPP_LVM), Prog);
	  break;
      case LVM_MAJINUSE:
	  fprintf(stderr, lvm_msg(MAJINUSE_LVM), Prog);
	  break;
      default:
	  fprintf(stderr, lvm_msg(UNKNERR_LVM), Prog, errnum);
	  break;
  }
  return(0);
}
