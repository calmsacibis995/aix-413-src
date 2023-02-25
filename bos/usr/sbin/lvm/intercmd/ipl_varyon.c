static char sccsid[] = "@(#)16  1.16.2.1  src/bos/usr/sbin/lvm/intercmd/ipl_varyon.c, cmdlvm, bos411, 9428A410j 7/17/92 19:50:57";
 /**************************************************************************
 *                                                                        *
 * COMPONENT_NAME: CMDLVM                                                 *
 *                                                                        *
 *                                                                        *
 * ORIGINS: 27                                                            *
 *                                                                        *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when                  *
 * combined with the aggregated modules for this product)                 *
 *                  SOURCE MATERIALS                                      *
 * (C) COPYRIGHT International Business Machines Corp. 1989               *
 * All Rights Reserved                                                    *
 *                                                                        *
 * US Government Users Restricted Rights - Use, duplication or            *
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.      *
 *                                                                        *
 **************************************************************************/
/*                                                                        *
 *                                                                        *
 * FILE : ipl_varyon.c                                                    *
 *                                                                        *
 * FILE DESCRIPTION: Source file for ipl_varyon program. "ipl_varyon"     *
 *                   is used during 1st phase IPL to bring on line the    *
 *                   root volume group.                                   *
 *                                                                        *
 * RETURN VALUE DESCRIPTION:                                              *
 *                          0  Command Successfully completed             *
 *                          1  Unsuccessful in varying on the root volume *
 *                          2  ODM Access Error                           *
 *                          3  Object not found in ODM                    *
 *                          4  Error from LVM_QUERYPV                     *
 *                          5  MALLOC Failure                             *
 *                          6  MKNOD Failure                              *
 *                          7  READ Error                                 *
 *                          8  OPEN Error                                 *
 *                          9  Error from LVM_QUERYVG                     *
 *                         10  No Kernel Module ID found via loadext      *
 *                         11  Invalid IPL record                         *
 *                         12  GENMAJOR FAILURE                           *
 *                                                                        *
 * EXTERNAL PROCEDURES CALLED : odm_get_list,odm__initialize,odm_set_path *
 *                              odm_terminate, lvm_querypv, lvm_varyonvg, *
 *                              lvm_queryvg                               *
 *                                                                        *
 * FUNCTIONS: get_config_pvnames, get_rtvgname_rtvgid, get_majornum,      *
 *            make_special_files, get_kmid, test_pvstatus, log_errors,    *
 *            get_ipldevice_pvid, make_logical_volume_special_files       *
 *            main.                                                       *
 *                                                                        *
 *                                                                        *
 **************************************************************************/



/* INCLUDE HEADER FILES */

#include <stdio.h>
#include <fcntl.h>
#include <sys/devinfo.h>
#include <ctype.h>
#include <sys/dir.h>
#include <string.h>
#include <lvm.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <memory.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/bootrecord.h>
#include <sys/ioctl.h>
#include "cmdlvm.h"
#include <sys/errids.h>



/* EXTERNAL PROCEDURES CALLED */

extern odm_error ();
extern int odmerrno;
extern int lvm_querypv();
extern int lvm_queryvg();
extern int lvm_varyonvg();

/* EXTERNAL DEFINTIONS */

extern char *optarg;
extern int optind;


/* ERROR DEFINITIONS */

#define VARYON_FAILURE    1
#define ODM_FAILURE       2
#define ODM_OBJNOTFOUND   3
#define LQUERYPV_FAILURE  4
#define MALLOC_FAILURE    5
#define MKNOD_FAILURE     6
#define READERROR         7
#define OPEN_ERROR        8
#define LQUERYVG_FAILURE  9
#define NOKMID_FOUND     10
#define INVALIDIPLREC    11
#define GENMAJOR_FAILURE 12
#define NO_ERROR          0



/* CONSTANTS */

#define NUM_OF_PVSTATUS   4


/* GLOBAL VARIABLES */

static int device_flag;
static int inquiry_flag;
static int verbose_flag;

static char *ipldevice;
static char NAME_BUFFER[50];

struct  vglist {
          struct unique_id vgid;
  };


/*************************************************************************
*                                                                        *
* NAME: getflg                                                           *
*                                                                        *
* FUNCTION: Parse the command line flags and arguments                   *
*                                                                        *
* INPUT:                                                                 *
*            argc             An INTEGER containing the number of        *
*                             command line strings.                      *
*                                                                        *
*            argv             An array of command line strings.          *
*                                                                        *
* OUTPUT:                                                                *
*            NONE                                                        *
*                                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0 Function successfully completed             *
*                         -1 Unsucessful Completion                      *                                              *
*                                                                        *
*                                                                        *
**************************************************************************/


int getflg(argc,argv)

int argc;        /* number of command line string in referenced by argv */
char **argv;     /* array of command line strings */
{
        int rc;                    /* getopt return value */

        device_flag   = 0;
        verbose_flag  = 0;
        inquiry_flag  = 0;

        /* return if no parameters have been entered */

        if (argc == 1)
          {
            return (NO_ERROR);
          }


        /* switch on each option contained in the command line */
        while ((rc = getopt(argc, argv, "d:iv")) != EOF) {
                switch (rc) {
                case 'd':
                        device_flag = 1;
                        ipldevice = optarg;
                        break;
                case 'i':
                        inquiry_flag = 1;
                        break;
                case 'v':
                        verbose_flag = 1;
                        break;
                default:
                case '?':
                        fprintf(stderr,"Usage: ipl_varyon [-d ipldevice]\
 [-i ] [-v ]\n");
                        return(-1);
                }
        } /* end while */

        return(NO_ERROR);

}



/*************************************************************************
*                                                                        *
* NAME: get_ipldevice_pvid                                               *
*                                                                        *
* FUNCTION: Get the pvid for the ipldevice, which is a linked file to    *
*           one of the physical volumes contained within the root        *
*           volume group.                                                *
*                                                                        *
* INPUT:    NONE                                                         *
*                                                                        *
* OUTPUT:                                                                *
*            pvid             A POINTER to a structured used to hold     *
*                             the pvid.                                  *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0 Function successfully completed             *
*                          7 READ system call failure                    *
*                          8 OPEN system call failure                    *
*                         11 INVALID IPL record                          *
*                                                                        *
*                                                                        *
*                                                                        *
**************************************************************************/




int get_ipldevice_pvid (ipldevice, pv_id)
char *ipldevice;
struct unique_id *pv_id;
{
  IPL_REC ipl_record;
  int fd;
  int error;
  char path[30];

  strcpy (path,"/dev/");
  mbsncat(path, ipldevice, strlen(ipldevice));
  if ((fd = open(path, O_RDONLY)) == -1)
    {
      return (OPEN_ERROR);
    }

  error = read (fd, &ipl_record, sizeof(IPL_REC));
  if (error == -1)
    {
      return (READERROR);
    }

  if (ipl_record.IPL_record_id == IPLRECID)
    {

      pv_id -> word1 = ipl_record.pv_id.word1;
      pv_id -> word2 = ipl_record.pv_id.word2;
      pv_id -> word3 = ipl_record.pv_id.word3;
      pv_id -> word4 = ipl_record.pv_id.word4;

      close(fd);
      return (NO_ERROR);
    }
  else
    {
      close(fd);
      return (INVALIDIPLREC);
    } /* endif */
}


/*************************************************************************
*                                                                        *
* NAME: get_config_pvnames                                               *
*                                                                        *
* FUNCTION: Get a list of all configured physical volumes (names)        *
*           from ODM.                                                    *
*                                                                        *
* INPUT:    NONE                                                         *
*                                                                        *
* OUTPUT:                                                                *
*            pvlist_ptr       A POINTER to a POINTER of character        *
*                             strings used to store the physical.        *
*                             volume names.                              *
*                                                                        *
*            num_of_pvnames   An INTEGER used to store the number        *
*                             of configured physical volumes.            *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0 Function successfully completed             *
*                          2 ODM Access Error                            *
*                          3 Object not found in ODM                     *
*                          5 MALLOC Failure                              *
*                                                                        *
*                                                                        *
*                                                                        *
**************************************************************************/

/* function to get a list of all configured pvnames */

int get_config_pvnames (pvlist_ptr, num_of_pvnames )

struct pvlist **pvlist_ptr;
int *num_of_pvnames;
{
  struct CuAt *cuatp;
  struct CuDv *cudvp;

  struct listinfo stat_info;
  char crit[128];
  int index;



  /* Get a list of all configured physical volumes from the CuDv class */

  strcpy (crit, "name like hdisk* AND attribute = pvid");
  cuatp =  (struct CuAt *) odm_get_list(CuAt_CLASS, crit, &stat_info,
                                        MAXNUMPVS, 1);

  if ((int)cuatp == -1) { return (ODM_FAILURE); }

  if (stat_info.num == 0) { return (ODM_OBJNOTFOUND); }
  else
    {
      *num_of_pvnames = stat_info.num;
      *pvlist_ptr = (struct pvlist *) malloc
                                       (sizeof (struct pvlist) * stat_info.num);

      if (*pvlist_ptr == NULL)
        {
          odm_free_list (cuatp, &stat_info);
          return (MALLOC_FAILURE);
        }
      else
        {
          for (index = 0; index < stat_info.num ;index ++ )
            {
              strcpy ( (*pvlist_ptr + index) -> pvname, (cuatp + index) -> name);
              sscanf((cuatp + index) -> value, "%08x%08x", &((*pvlist_ptr + index) ->pvid.word1),
                     &((*pvlist_ptr + index) ->pvid.word2));

            } /* endfor */

          odm_free_list (cuatp, &stat_info);
          return (NO_ERROR);
        } /* endif */
    } /* endif */
}

/*************************************************************************
*                                                                        *
* NAME: get_disk_list                                                    *
*                                                                        *
* FUNCTION: Gets a list of all configured physical volumes               *
*           by searching in the /dev directory. This function will       *
*           be called as opposed to get_config_pvnames when no CuPV      *
*           class exists in the /etc/objrepos directory.                 *
*                                                                        *
* INPUT:    NONE                                                         *
*                                                                        *
* OUTPUT:                                                                *
*            pvlist_ptr       A POINTER to a POINTER that points         *
*                             to a structure containing the names        *
*                             and associated pvids.                      *
*                                                                        *
*            num_of_pvnames   An INTEGER used to store the number        *
*                             of configured physical volumes.            *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0 Function successfully completed             *
*                          2 ODM Access Error                            *
*                          5 MALLOC Failure                              *
*                          8 OPEN Error                                  *
*                                                                        *
*                                                                        *
*                                                                        *
**************************************************************************/


get_disk_list(pvlist_ptr, num_of_pvnames)

struct pvlist **pvlist_ptr;
int *num_of_pvnames;

{
        int     index;
        int     diskcnt;
        char    crit[128];
        char    str[128];
        char    buf[512];
        int     fd;
        struct  devinfo devi;
        IPL_REC iplrec;
        struct  dirent *dirent;
        DIR     *dirp;
        char    *ptr;
        struct Class *cusdev;           /* customized devices class ptr */
        struct CuDv cusobj;             /* customized device object storage */



        /* open the Customized Devices class */
        if ((cusdev = odm_open_class(CuDv_CLASS))==NULL) { return(ODM_FAILURE); }

        /* get memory for pvlist structure */
        *pvlist_ptr = (struct pvlist *) malloc (sizeof(struct pvlist) * LVM_MAXPVS);
        if (*pvlist_ptr == NULL) { return (MALLOC_FAILURE); }

        /* zero out the pvlist structure */
        memset ((*pvlist_ptr), '\0', (sizeof(struct pvlist) * LVM_MAXPVS));

        /* open the /dev directory */
        dirp = opendir("/dev");
        if (dirp==NULL)
          {
           odm_close_class(cusdev);
           return(OPEN_ERROR);
          }

        /* step through the entire /dev directory, comparing the */
        /* file names with the names in the CuDv class. If a match */
        /* is found, then it is a configured device. Determine if */
        /* this device is hard disk; if so, then update pvlist  */
        /* structure */

        diskcnt =0;
        while(dirent=readdir(dirp))
          {
             /*  check in CuDv to see if this is the disk logical_name */
             strcpy(crit,"name = '");
             strncat(crit,dirent->d_name,dirent->d_namlen);
             strcat(crit,"'");
             if ((int)odm_get_obj(cusdev,crit,&cusobj,TRUE) <= 0)
               {
                 continue; /* try another */
               }

             if (strncmp("disk/",cusobj.PdDvLn_Lvalue,5))
               {
                 continue;
               }


             devi.devtype = 0;
             strcpy(str,"/dev/");
             strncat(str,dirent->d_name,dirent->d_namlen);
             if ((fd=open(str,O_RDONLY))==-1)
               {
                 continue; /* oh well */
               }



             if (ioctl(fd,IOCINFO,&devi))
               {
                 continue; /* oh well */
               }

/*           if ((devi.devtype==DD_DISK && devi.devsubtype == DS_PV) || */
             if (devi.devtype == DD_DISK || devi.devtype==DD_SCDISK)
               {
                   /* the device is a hard disk */

                   /* get the ipl record */
                   if (read(fd,&iplrec,sizeof(IPL_REC)) != sizeof(IPL_REC))
                     {
                        close(fd);
                        continue; /* try another */
                     }


                   /* check the majic number */
                   if (iplrec.IPL_record_id != IPLRECID)
                     {
                       /* bad IPL_RECORD */
                       close(fd);
                       continue; /* try another */
                     }


                   /* save the name and pvid in the pvlist structure */
                   strcpy((*pvlist_ptr + diskcnt) -> pvname,cusobj.name);
                   memcpy(&((*pvlist_ptr + diskcnt) -> pvid), &iplrec.pv_id,
                          sizeof(struct unique_id));

                   /* increment the disk count */
                   diskcnt++;
                   close(fd);
               }/* end if */
          } /* end while */

        *num_of_pvnames = diskcnt;

        closedir(dirp);
        odm_close_class(cusdev);
        return(NO_ERROR);
}




/*************************************************************************
*                                                                        *
* NAME: is_a_bootdevice                                                  *
*                                                                        *
* FUNCTION: Determine if a physical volume is a boot device              *
*                                                                        *
* INPUT:                                                                 *
*            pvname           A POINTER to the physical volume name      *
* OUTPUT:                                                                *
*            bootdevice_flag  A POINTER to an integer holding a value    *
*                             TRUE or FALSE depending if the physical    *                              *
*                             volume is a boot device.                   *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0 Function successfully completed             *
*                          7 READ system call failure                    *
*                          8 OPEN system call failure                    *
*                         11 INVALID IPL record                          *
*                                                                        *
*                                                                        *
*                                                                        *
**************************************************************************/


int is_a_bootdevice (pvname, bootdevice_flag)
char *pvname;
int  *bootdevice_flag;
{

  IPL_REC ipl_record;
  int fd;
  int error;
  char path[30];

  strcpy(path,"/dev/");
  mbsncat(path, pvname, strlen(pvname));

  if ((fd = open(path, O_RDONLY)) == -1)
    {
      return (OPEN_ERROR);
    }

  error = read (fd, &ipl_record, sizeof(IPL_REC));
  if (error == -1)
    {
      return (READERROR);
    }



  if (ipl_record.IPL_record_id == IPLRECID)
    {
       if (ipl_record.boot_code_length > 0)
         {
           *bootdevice_flag = TRUE;
         }
       else
         {
           *bootdevice_flag = FALSE;
         }

       return (NO_ERROR);
    }
  else
    {
      return (INVALIDIPLREC);
    } /* endif */
}




/*************************************************************************
*                                                                        *
* NAME: generate_volume_group_report                                     *
*                                                                        *
* FUNCTION: Generates a report that correlates physical volumes to       *          *
*           volume groups.                                               *
*                                                                        *
* INPUT:                                                                 *
*            pvlist_ptr       A POINTER to the pvlist structure          *
*                             and associated pvids.                      *
*                                                                        *
*            num_of_pvnames   An INTEGER containing the number           *
*                             of configured physical volumes.            *
*                                                                        *
* OUTPUT:    NONE                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0 Function successfully completed             *
*                          2 ODM Access Error                            *
*                          5 MALLOC Failure                              *
*                          8 OPEN Error                                  *
*                                                                        *
*                                                                        *
*                                                                        *
**************************************************************************/


generate_volume_group_report(pvlist_ptr, num_of_pvnames)
struct pvlist *pvlist_ptr;
int num_of_pvnames;
{

  struct vglist *vglist_ptr;
  int volnum;
  int error;
  int num_of_volgroups;
  int index;
  int FOUND;
  int BUFFER_SIZE;
  int VGREC_SIZE;
  char BOOTDEVICE[5];
  struct unique_id NULL_VGID;

  /* perform initialization */
  memset (&NULL_VGID, '\0', sizeof (struct unique_id));
  VGREC_SIZE = sizeof(struct vglist);
  BUFFER_SIZE = VGREC_SIZE;
  num_of_volgroups=0;
  volnum = 0;

  /* allocate memory for the vglist structure */
  vglist_ptr = (struct vglist *) malloc (BUFFER_SIZE);

  if (vglist_ptr == NULL)
    {
      return (MALLOC_FAILURE);
    }


  /* loop through the pvlist structure and determine the */
  /* unique volume groups. Store the unique volume groups */
  /* in the vglist structure */

  for (index = 0;index < num_of_pvnames ;index++ )
    {
      FOUND = FALSE;
      volnum = 0;

      /* the first volume group has to be unique by definition */
      if (num_of_volgroups == 0)
        {

          memcpy (&vglist_ptr[volnum].vgid, &((pvlist_ptr + index) -> vgid),
                  sizeof (struct unique_id));
          num_of_volgroups++;
        }
      else
        {
          /* compare volume group to the volume groups in the vglist */
          /* structure */

          while (!FOUND && (volnum < num_of_volgroups))
            {
              error =  memcmp (&((pvlist_ptr + index) -> vgid), &vglist_ptr[volnum].vgid,
                               sizeof (struct unique_id));
              if (error == NO_ERROR)
                {
                   FOUND = TRUE;
                }

              volnum ++;
            } /* end while */


          /* if a match was not found then it is unique volume group */
          /* store in vglist structure */

          if (!FOUND)
            {

              BUFFER_SIZE += VGREC_SIZE;
              vglist_ptr = (struct vglist *) realloc(vglist_ptr, BUFFER_SIZE);
              if (vglist_ptr == NULL)
                {
                  return (MALLOC_FAILURE);
                }

              memcpy (&vglist_ptr[volnum].vgid, &((pvlist_ptr + index) -> vgid),
                      sizeof (struct unique_id));
              num_of_volgroups++;
            }
        } /* end if */
    } /* endfor */

    printf ("\n\nPVNAME\t\tBOOT DEVICE\tPVID\t\t\tVOLUME GROUP ID\n");


    /* print out list of unique volume groups with their */
    /* corresponding physical volumes */

    for (volnum = 0; volnum < num_of_volgroups; volnum++ )
      {

            for (index = 0; index < num_of_pvnames ;index ++ )
              {

                 error =  memcmp (&((pvlist_ptr + index) -> vgid), &vglist_ptr[volnum].vgid,
                                  sizeof (struct unique_id));
                 if (error == NO_ERROR)
                   {

                     error = is_a_bootdevice ((pvlist_ptr + index) -> pvname,
                                              &(pvlist_ptr + index) -> boot_device);

                     if ((pvlist_ptr + index) -> boot_device)
                         strcpy(BOOTDEVICE,"YES");
                     else
                         strcpy(BOOTDEVICE,"NO");

                     printf ("%s\t\t%s\t\t%08x%08x\t",
                              (pvlist_ptr + index) -> pvname, 
                              BOOTDEVICE,
                              (pvlist_ptr + index) -> pvid.word1, 
                              (pvlist_ptr + index) -> pvid.word2); 

                     error = memcmp (&((pvlist_ptr + index) -> vgid), &NULL_VGID,
                             sizeof (struct unique_id));

					 if (error != NO_ERROR)
                            printf ("%08x%08x\n",
                               vglist_ptr[volnum].vgid.word1, 
                               vglist_ptr[volnum].vgid.word2); 
                    else
                            printf ("%s\n", "None");
                   }
              } /* endfor */
      } /* endfor */

    return (NO_ERROR);
}


/*************************************************************************
*                                                                        *
* NAME: get_majornum                                                     *
*                                                                        *
* FUNCTION: Get the major number for the root volume group given         *
*           the root volume group's name.                                *
*                                                                        *
* INPUT:                                                                 *
*           rootvgname_ptr   A POINTER to a character string             *
*                            containing the name of the root volume      *
*                            group.                                      *
*                                                                        *
* OUTPUT:                                                                *
*           major_devnum     A POINTER to an integer used to store the   *
*                            the major number of the root volume group   *
*                                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION                                               *
/*                          0 Function successfully completed             *
*                          12 Unable to get major number                 *
*                                                                        *
**************************************************************************/
int get_majornum (rootvgname_ptr, major_devnum)
char *rootvgname_ptr;
int  *major_devnum;
{
  *major_devnum = genmajor(rootvgname_ptr);
  if (*major_devnum == -1)
    {
      return(GENMAJOR_FAILURE);
    }
  else
    {
      return(NO_ERROR);
    } /* endif */
}





/*************************************************************************
*                                                                        *
* NAME: get_kmid                                                         *
*                                                                        *
* FUNCTION: Get the kernel module identification number.                 *
*                                                                        *
* INPUT:   NONE                                                          *
*                                                                        *
* OUTPUT:                                                                *
*           kmid_ptr     A POINTER to a structure used to store          *
*                        the kernel module id.                           *
*                                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION                                               *
*                          0 Function successfully completed             *
*                          1 Unsuccessfully completed                    *
*                                                                        *
*                                                                        *
**************************************************************************/

int get_kmid (kmid_ptr)
mid_t *kmid_ptr;
{
  FILE *kmid_file_ptr;

 *kmid_ptr = loadext("hd_pin",FALSE,TRUE);
 if (*kmid_ptr == NULL)
   {
     return (NOKMID_FOUND);
   }
 else
   {
     return (NO_ERROR);
   } /* endif */

}




/*************************************************************************
*                                                                        *
* NAME: make_special_files                                               *
*                                                                        *
* FUNCTION: Creates the special files in the /dev directory for the root *
*           volume group.                                                *
*                                                                        *
* INPUT:                                                                 *
*           rootvgname_ptr   A POINTER to a character string             *
*                            containing the name of the root volume      *
*                            group.                                      *
*                                                                        *
*           major_devnum     An INTEGER containing the major device      *
*                            number for the root volume group.           *
*                                                                        *
* OUTPUT:   NONE                                                         *
*                                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION                                               *
*                          0 Function successfully completed             *
*                          6 MKNOD FAILURE                               *
*                                                                        *
*                                                                        *
**************************************************************************/

int make_special_files (rootvgname_ptr, major_devnum)
char *rootvgname_ptr;
int major_devnum;
{
  char  path[50];
  dev_t dev;
  int mode;
  int error;

  mode = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
 /* make special file for root volume group  */
  strcpy (path, "/dev/");
  strcpy(&path[5], rootvgname_ptr);
  dev = makedev(major_devnum, 0);
  error = 0;
  error = mknod (path, mode, dev);
  if (error != NO_ERROR)
    {
      return (MKNOD_FAILURE);
    }

  /* make special file for root volume group used for exportvg  */

  strcpy (path, "/dev/");
  strcpy(&path[5], "IPL_rootvg");
  dev = makedev(major_devnum, 0);
  error = mknod (path, mode, dev);

  return (NO_ERROR);
}


void log_errors (varyonvg)
struct varyonvg varyonvg;
{
  int index;
  struct cmdlvm_err
  {
         struct err_rec0 err_rec0;
         char message[16];
  } cmdlvm_err;

  memset (&cmdlvm_err, '\0', sizeof(struct cmdlvm_err));
  cmdlvm_err.err_rec0.error_id = ERRID_CMDLVM;
  strcpy (cmdlvm_err.err_rec0.resource_name, "CMDLVM");

  errlog(&cmdlvm_err, sizeof(cmdlvm_err));

  return;

  for (index = 0; index < varyonvg.vvg_out.num_pvs; ++ index ) {
    if ( varyonvg.vvg_out.pv[index].pv_status & (LVM_PVMISSING | LVM_PVREMOVED)) {
       sprintf (cmdlvm_err.message, "%s", varyonvg.vvg_out.pv[index].pvname);      
    } /* endfor */
  }
}

/*************************************************************************
*                                                                        *
* NAME: make_logical_volume_special_files                                *
*                                                                        *
* FUNCTION: Create the logical volume character and block special.       *
*           files associated with the root volume group                  *
*                                                                        *
* INPUT:                                                                 *
*           root_vgid      A structure used to hold the root volume      *
*                          groups id.                                    *
*                                                                        *
*           major_devnum   An INTEGER contianing the major number device *
*                          number for the root volume group.             *
*                                                                        *
* OUTPUT:                                                                *
*           NONE                                                         *
*                                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION                                               *
*                          0 Function successfully completed             *
*                          6 MKNOD FAILURE                               *
*                          9 LQUERVG FAILURE                             *
*                                                                        *
*                                                                        *
**************************************************************************/


int make_logical_volume_special_files (root_vgid, major_devnum)
struct unique_id root_vgid;
int major_devnum;
{
  struct queryvg *queryvg;
  struct lv_array *lv_array;
  char *pvname;
  int error;
  int index;
  char path1[50];
  char path2[50];
  dev_t dev;
  int moderaw;
  int modeblock;
  int minor_devnum;



  moderaw = S_IFCHR| S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  modeblock = S_IFBLK | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  pvname = NULL;
  queryvg = NULL;



  error = lvm_queryvg(&root_vgid, &queryvg, pvname);

  if (error != NO_ERROR)
    {
      return (LQUERYVG_FAILURE);
    }
  else
    {
      lv_array = queryvg -> lvs;

      if (lv_array != NULL)
        {
           index = 0;
           while (index < (queryvg -> num_lvs))
             {
                 minor_devnum = (lv_array + index) -> lv_id.minor_num;
                 dev = makedev(major_devnum, minor_devnum);

                 strcpy(path1, "/dev/r");
                 strcpy(path2, "/dev/");
                  /* create raw special file for logical volume*/
                 mbsncat(path1, (lv_array + index) -> lvname,
                           LVM_NAMESIZ);
                 error = mknod (path1, moderaw, dev);
                 if (error != NO_ERROR)
                   {
                      return (MKNOD_FAILURE);
                   }

                /*  create block special file for logical volume  */
                 mbsncat(path2, (lv_array + index) -> lvname,
                           LVM_NAMESIZ);
                 error = mknod (path2, modeblock, dev);
                 if (error != NO_ERROR)
                   {
                     return (MKNOD_FAILURE);
                   }

                 index ++;
             } /* end while */
        } /* end if */

      return (NO_ERROR);
    } /* endif */
}


/*************************************************************************
*                                                                        *
* NAME: cleanup                                                          *
*                                                                        *
* FUNCTION: Performs cleanup functions (ie, returning memory, etc).      *
*                                                                        *
* INPUT:                                                                 *
*           pvlist_ptr    A POINTER to a POINTER of character strings    *
*                         used to store the physical volume names.       *
*                                                                        *
* OUTPUT:                                                                *
*           NONE                                                         *
*                                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION                                               *
*                                                                        *
*           NONE                                                         *
*                                                                        *
**************************************************************************/

void cleanup (pvlist_ptr)
struct pvlist *pvlist_ptr;
{
  free (pvlist_ptr);
  odm_terminate();
}

int get_Q()
{

   struct CuAt *cuatp;
   struct listinfo stat_info;
   int error;
   int num,i;

cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,
               "name = rootvg and attribute = quorum",&stat_info,1,1);

if ((int)cuatp == -1) return(0); 

/* if there were no physical volumes, print an error message */
if (stat_info.num) { return(1); }
else { return(0); }

}

/*************************************************************************
*                                                                        *
* NAME: main                                                             *
*                                                                        *
* FUNCTION: ipl_varyon sets varies on the root volume group              *
*                                                                        *
*                                                                        *
*                                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0  Command Successfully completed             *
*                          1  Unsuccessful in varying on the root volume *
*                          2  ODM Access Error                           *
*                          3  Object not found in ODM                    *
*                          4  Error from LVM_QUERYPV                     *
*                          5  MALLOC Failure                             *
*                          6  MKNOD Failure                              *
*                          7  READ Error                                 *
*                          8  OPEN Error                                 *
*                          9  LQUERYVG Failure                           *
*                          10 No Kernel Module ID found via loadext      *
*                          11 Invalid IPL record                         *
*                                                                        *
*                                                                        *
**************************************************************************/

main (argc, argv)
int argc;
char **argv;

{

  int error;                        /* return code */
  struct unique_id vg_id,            /* vg and pv id's used to request */
                   pv_id,
                   root_vgid;

                                     /* the query information  */

  struct querypv *querypv;            /* lvm record the query information */
                                     /* is returned in */
  char pvname[32];
  int bogus;

  struct pvlist *pvlist_ptr, *rootlist_ptr;
  struct pvlist pvnames_buffer [LVM_MAXPVS];
  int num_of_pvnames;
  int num_of_rootpvnames;
  char rootvg_name [32];
  int major_devnum;
  int index;
  struct varyonvg varyonvg;
  mid_t  kmid;
  int  noquorum;

  /* set up default values for vgid, pvid, and pvname, and umask */
  /* use memset function */
  memset (&vg_id,'\0',sizeof (struct unique_id));
  memset (&pv_id,'\0',sizeof (struct unique_id));
  num_of_rootpvnames = 0;
  num_of_pvnames = 0;
  error = NO_ERROR;
  pvlist_ptr = NULL;
  rootlist_ptr = NULL;
  rootvg_name[0] = '\0';
  umask(S_IRWXO);


  /* parse command line arguments (if any) */
  if ((error = getflg(argc,argv)) < 0) exit(0);

  /* perform initialization function for odm use */
  odm_initialize();


 if (!inquiry_flag)  /* skips ipldevice processing */
  {

  /* get the physical volume id for the ipl device */
  if (!device_flag)
    {
      /* if no ipldevice has been specified, then use the */
      /* default ipldevice */
      ipldevice = NAME_BUFFER;
      strcpy(ipldevice, "ipldevice");
    }

  error = get_ipldevice_pvid (ipldevice, &pv_id);
  if (error != NO_ERROR) {
      odm_terminate();
      exit (error);
  }

  strcpy(pvname,ipldevice);

  /* call lvm_querypv passing in the pvname and  pvid to  */
  /* get back the volume group id for the root volume group */

  if (error = lvm_querypv(&vg_id, &pv_id, &querypv, pvname))
    {
       odm_terminate ();
       exit (LQUERYPV_FAILURE);
    }

       root_vgid.word1 = vg_id.word1;
       root_vgid.word2 = vg_id.word2;

       /* get a list of configured physical volumes from odm */
       /* if the inquiry flag is set, then we want to walk */
       /* through the /dev directory for a more exact report */

       error =  get_config_pvnames (&pvlist_ptr, &num_of_pvnames);

       if ((error != NO_ERROR) || (num_of_pvnames == 0)) {
           get_disk_list(&pvlist_ptr, &num_of_pvnames);
       }


       /* determine which of the physical volumes in the */
       /* configured list of physical volumes belong to */
       /* the root volume group and store in the varyonvg */
       /* structure */
       for (index = 0;index < num_of_pvnames; index++ ) {

            /* copy pvid for current disk into a temp location */

            memcpy (&pv_id, &((pvlist_ptr + index) -> pvid),
                    sizeof(struct unique_id));

            memset (&vg_id,'\0', sizeof (struct unique_id));
            error = lvm_querypv (&vg_id, &pv_id, &querypv,
                                 (pvlist_ptr + index) -> pvname);

            if (error == NO_ERROR) {
                /* copy pv's vgid to pvlist structure */
                memcpy (&((pvlist_ptr + index) -> vgid), &vg_id,
                        sizeof(struct unique_id));

                error = memcmp (&vg_id, &root_vgid,
                                sizeof(struct unique_id));

                if (error == NO_ERROR)
                  {
                     memcpy (&varyonvg.vvg_in.pv[num_of_rootpvnames].pv_id,
                             &pv_id, sizeof (struct unique_id));

                     varyonvg.vvg_in.pv[num_of_rootpvnames].pvname =
                            pvnames_buffer[num_of_rootpvnames].pvname;
                     strcpy (varyonvg.vvg_in.pv[num_of_rootpvnames].pvname,
                               (pvlist_ptr + index) -> pvname);
                     num_of_rootpvnames ++;
                  }
              }
         } /* endfor */

   /* if the inquiry flag is set, we do not want to proceed with */
   /* varying on the root volume group.  Just print out the report */
   /* and exit */

  } /* if !inquiry_flag */
 
  else /* inquiry_flag is set */
	   {
       get_disk_list(&pvlist_ptr, &num_of_pvnames);
       for (index = 0;index < num_of_pvnames; index++ )
         {

            /* copy pvid for current disk into a temp location */

            memcpy (&pv_id, &((pvlist_ptr + index) -> pvid),
                    sizeof(struct unique_id));


            memset (&vg_id,'\0', sizeof (struct unique_id));
            error = lvm_querypv (&vg_id, &pv_id, &querypv,
                                 (pvlist_ptr + index) -> pvname);
            if (error == NO_ERROR)
              {
                /* copy pv's vgid to pvlist structure */
                memcpy (&((pvlist_ptr + index) -> vgid), &vg_id,
                        sizeof(struct unique_id));
              }
		 }
       error = generate_volume_group_report(pvlist_ptr, num_of_pvnames);
       cleanup (pvlist_ptr);
       exit(error);
     } /* end if !inquiry_flag */


  /* set the rootvolume groups name */
  strcpy (rootvg_name, "rootvg");

  /* hard code the major number for now */
  error = get_majornum(rootvg_name, &major_devnum);
  if (error !=NO_ERROR)
    {
      /* unable to get major number for the root volume group */
      /* try a best guess at what the major number is. */
      major_devnum = 10;
    }

  /* retrive the kernel module id from odm */
  error = get_kmid (&kmid);
  if (error != NO_ERROR)
    {
      cleanup (pvlist_ptr);
      exit (error);
    }


  /* create the special files for the root volume group */
  error = make_special_files (rootvg_name, (short) major_devnum);
  if (error != NO_ERROR)
    {
      cleanup (pvlist_ptr);
      exit (error);
    }


  /* initialize the varyonvg structure with the appropriate */
  /* information and then call lvm_varyonvg to vary on the */
  /* root volume group */
  varyonvg.kmid = kmid;
  varyonvg.vg_major = (short)major_devnum;
  varyonvg.vgname = rootvg_name;
  varyonvg.vvg_in.num_pvs = num_of_rootpvnames;
  memcpy(&varyonvg.vg_id, &root_vgid, sizeof(struct unique_id));
  varyonvg.noopen_lvs = FALSE;
  varyonvg.misspv_von = TRUE;
  varyonvg.missname_von = TRUE;

  noquorum = get_Q();
  if (noquorum) {
      varyonvg.override = TRUE;
      varyonvg.noquorum = TRUE;
  }

  error = lvm_varyonvg(&varyonvg);
  if (error < 0)
  {
      error = VARYON_FAILURE;
  }
  else 
  if (error > 0)
  {
     log_errors(varyonvg);   
     error = NO_ERROR;
  }

  /* if successful in varying on the root volume group */
  /* then create the the special files (both block and */
  /* raw) for all the logical volumes contained within */
  /* the root volume group */
  if (error == NO_ERROR)
    {
      error = make_logical_volume_special_files (root_vgid, major_devnum);
    }

  /* perform cleanup functions */
  cleanup (pvlist_ptr);
  exit (error);
}
