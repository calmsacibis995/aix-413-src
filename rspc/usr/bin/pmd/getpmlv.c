static char sccsid[] = "@(#)98  1.4  src/rspc/usr/bin/pmd/getpmlv.c, pwrcmd, rspc41J, 9524E_all 6/15/95 08:03:26";
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS: getpmlv
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


#include <stdio.h>
#include <fcntl.h>
#include <sys/lvdd.h>
#include <lvm.h>
#include <sys/pm.h>
#include <string.h>
#include <errno.h>

extern int errno;

#define  SECTOR   512

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
#endif


#define Free(pointer) if (pointer != NULL){\
                         free((caddr_t)pointer);\
                         pointer=NULL;\
                      }

#define Free_libc(pointer) if (pointer!=NULL){\
                              lvm_freebuf((caddr_t)pointer);\
                              pointer=NULL;\
                           }

/* NOTE: Since the libc_r is linked to this program, */
/*       we cannot be too careful to free memory     */
/*       which is allocated inside the libc library. */
/*       If it is the case, Free_libc() should be    */
/*       called, otherwise memoryleak would occur.   */
/*       That is, we must ensure that liberation of  */
/*       memory is done by the same library as it is */
/*       allocated by.                               */
/*       lvm_freebuf is in use to free the memory    */
/*       allocated by libc for the release 41J,      */
/*       for only shared libraries have the loader   */
/*       section to specify the library, and there   */
/*       seems no shared libraries but liblvm        */
/*       which just calls free with libc.            */
/*       ( refer to the defect 180042 for this. )    */

/*
 * NAME:  getpmlv
 *
 * FUNCTION:  get PM hibernation volume information
 *
 * DATA STRUCTURES:
 *      lv_name   PM hibernation volume logical name. (input)
 *      pmhi      PM hibernation volume information. (output)
 *      pms       PM sector list (array). (output)
 *
 * RETURN VALUE DESCRIPTION:
 *      0:  success
 *      -1: error
 *
 * NOTE:
 *      Returned PM sector list needs to be freed later.
 */
int
getpmlv(char *lv_name, pm_hibernation_t *pmhi)
{
   char        lvid_ascii[256], dev_name[256];
   struct lv_id      lvid;
   struct pp      *pp;
   struct querylv    *querylv;
   struct xlate_arg  xlate;
   struct unique_id  pv_id;
   pm_sector_t    *pms;
   int         i;
   int         fd;
   int         sector_num, sector_zero;

   Debug0("getpmlv\n");

        errno = 0;
   /* invalid logical volume name */
   if(lv_name == NULL || *lv_name == '\0'){
      return -1;
   }

   /* open logical volume */
   sprintf(dev_name, "/dev/%s", lv_name);
   if((fd = open(dev_name, O_RDONLY)) < 0){
      errno = ENOENT;
      return -1;
   }

   /* get devno of logical volume */
   xlate.lbn = 0;
   xlate.mirror = 1;
        if(ioctl(fd, XLATE, &xlate) == -1){
      return -1;
        }
   close(fd);

   /* get lvid of logical volume */
   if(get_lvid(lv_name, lvid_ascii) != 0){
      return -1;
   }

   parsepair(lvid_ascii, ".", 3, 22, &lvid.vg_id, &lvid.minor_num);

   /* get logical volume (all blocks) information */
   if(lvm_querylv(&lvid, &querylv, NULL) != 0){  
      return -1;
   }

   if((pms = (pm_sector_t *)malloc(
         querylv->currentsize * sizeof(pm_sector_t))) == NULL){
      return -1;
   }


   /* make sector list */
   pp = querylv->mirrors[0];
   pmhi->snum = 0;
   pv_id = pp[0].pv_id;

   for(i = 0; i < querylv->currentsize; i++){
 

       /* if logical volume is over several disks, we cannot handle */
       if(memcmp(&pv_id, &(pp[i].pv_id), sizeof(pv_id)) != 0){
      return -1;
       }
       

       if(i == 0){
      pms[pmhi->snum].size = 1;
      pms[pmhi->snum].start = pp[i].pp_num;
       }else{
      if(pms[pmhi->snum].start + pms[pmhi->snum].size
         == pp[i].pp_num){
          (pms[pmhi->snum].size)++;
      }else{
          (pmhi->snum)++;
          pms[pmhi->snum].size = 1;
          pms[pmhi->snum].start = pp[i].pp_num;
      }
       }
   }  
   (pmhi->snum)++;

   pmhi->devno = xlate.p_devt;
   pmhi->ppnum = querylv->currentsize;
   pmhi->ppsize = querylv->ppsize;
   pmhi->slist = pms;
   sector_num = (1 << querylv->ppsize) / SECTOR;
   sector_zero = pmhi->slist[0].start * sector_num - xlate.pbn;
   for(i = 0; i < pmhi->snum; i ++){
      pms[i].start *= sector_num;
      pms[i].start -= sector_zero;
      pms[i].size *= sector_num;
   }

   Free_libc(pp);
   Free_libc(querylv);

   return 0;
}
