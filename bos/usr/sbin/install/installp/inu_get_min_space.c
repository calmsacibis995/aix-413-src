static char sccsid[] = "@(#)94  1.13  src/bos/usr/sbin/install/installp/inu_get_min_space.c, cmdinstl, bos411, 9428A410j 6/3/94 09:37:13";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_get_min_space (inulib)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <pwd.h>
#include <sys/statfs.h>
#include <installp.h>
#include <instl_options.h>
#include <sys/vmount.h>

extern struct vmount *get_vmount(fsid_t *fsid);

/*--------------------------------------------------------------------------*
**
** NAME: inu_get_min_space
**
** FUNCTION: determines if there is the minimum required space to install
**           an LPP.  If there is not, the /tmp and /usr are expanded as
**           appropriate.
**
** RETURNS: SUCCESS if it was successful
**          FAILURE if a failure occurred extending the file system.
**
**--------------------------------------------------------------------------*/
int inu_get_min_space (
char *fspath, 
int req_kbytes)
{
  int err;
  long long avail_kbytes=0;
  long long total_kbytes;
  struct statfs stat;
  struct vmount *vm;

   /*-----------------------------------------------------------------------*
    * Get the file system status block structure for the specified fs.
    *-----------------------------------------------------------------------*/
    err = statfs (fspath, &stat);
    if (err != 0) {
        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                              CMN_CANT_OPEN_D), INU_INSTALLP_CMD, fspath);
        return (FAILURE);
    }

   /*-----------------------------------------------------------------------*
    * Calculate the total and availble number of blocks.
    *-----------------------------------------------------------------------*/
    total_kbytes = (long long) ((((long long)stat.f_bsize * 
                                  (long long) stat.f_blocks) 
                                                     + 
                                 (long long) 1023) / (long long) 1024);
    avail_kbytes = (long long) ((((long long)stat.f_bsize * 
                                  (long long)stat.f_bfree) 
                                                     + 
                                 (long long) 1023) / (long long) 1024);

   /*-----------------------------------------------------------------------*
    * Compare available bytes to required bytes and if more is needed then
    * expand if the -X flag was given, else return a FAILURE...
    *-----------------------------------------------------------------------*/
    if (avail_kbytes < req_kbytes) {
        if ( ! X_FLAG) {
            vm = get_vmount(&stat.f_fsid);   /* used to get real fs name. */
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NOSPACE_E, 
                              CMN_NOSPACE_D), 
                              INU_INSTALLP_CMD, 
 		              vmt2dataptr(vm, VMT_STUB), /* get real fs name */
                              (req_kbytes - (int) avail_kbytes)*2); /*512 blks*/

            return (FAILURE);
	}
        err = expand_lv (fspath, 
                         (size_t)(total_kbytes + req_kbytes - avail_kbytes));
        if (err != 0) {
            vm = get_vmount(&stat.f_fsid);
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_EXPAND_E, 
                              CMN_CANT_EXPAND_D), 
                              INU_INSTALLP_CMD, 
 		              vmt2dataptr(vm, VMT_STUB),
                              (req_kbytes - (int) avail_kbytes)*2);
            return (FAILURE);
        }
    }
    return (SUCCESS);
}
