static char sccsid[] = "@(#)54	1.1  src/bos/usr/sbin/lvm/highcmd/lsvgfs.c, cmdlvm, bos411, 9428A410j 9/30/91 08:51:55";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Manager Commands
 *
 * FILE: lsvgfs.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <malloc.h>
#include <string.h>
#include <string.h>
#include <stdio.h>
#include <IN/AFdefs.h>
#include <sys/cfgodm.h>

#define TRUE		1
#define FALSE		0
#define FAILED		-1
#define SUCCEEDED	0
#define MAXRECS		4096
#define MAXATTR		128
#define FSYS		"/etc/filesystems"
#define MAXDEVICE	256
#define CRITERIA	256
#define LISTINCR	128
#define LOCK_PATH	"/dev/cfglock"
#define ODM_PATH	"/etc/objrepos"
#define DEVICE		"/dev/"

static int OdmLock;

static char *FsysList = (char *)NULL;
static int   FsysListLength  = 0;
static int   FsysListUsed  = 0;

static char *
GetInvFsys(char *Attribute,char *Value) {
  register AFILE_t AttrFile;
  ATTR_t Record;
  char *Fsys = (char *)NULL,*FsysPtr,*AttrPtr;

  if((!Attribute) ||
     (!Value))
    return((char *)NULL);

  if(!(AttrFile = AFopen(FSYS,MAXRECS,MAXATTR)))
    return((char *)NULL);

  while(Record = AFnxtrec(AttrFile)) {
    FsysPtr = Record->AT_value;
    AttrPtr = AFgetatr(Record,Attribute);
    if(strcmp(Value,AttrPtr))
      continue;
    Fsys = malloc(strlen(FsysPtr)+1);
    (void)strcpy(Fsys,FsysPtr);
    break;
    }

  (void)AFclose(AttrFile);
  return(Fsys);
  }

static int
InitODM(void) {
  if((FAILED == odm_set_path(ODM_PATH)) ||
     (FAILED == (OdmLock = odm_lock(LOCK_PATH,ODM_WAIT))))
    return(FAILED);
  else
    return(SUCCEEDED);
  }

static void
ExitODM(void) {
  (void)odm_unlock(OdmLock);
  }

static char *
LocateDependList(char *Name) {
  char *String;
  char Criteria[CRITERIA];
  struct listinfo ListInfo;
  struct CuDep *CuDep;
  int Length,Used,StrLen;

  (void)sprintf(Criteria,"name = \'%s\'",Name);
  if(FAILED == (CuDep = odm_get_first(CuDep_CLASS,Criteria,&ListInfo)))
    return(NULL);
  if(CuDep) {
    StrLen = strlen(CuDep->dependency);
    Length = LISTINCR>StrLen+2?LISTINCR:StrLen+2;
    if((String = malloc(Length))) {
      (void)strcpy(String,CuDep->dependency);
      Used = StrLen+1;
      *(String+Used) = '\0';
      free(CuDep);
      while(CuDep = odm_get_next(CuDep_CLASS,&ListInfo)) {
        StrLen = strlen(CuDep->dependency);
        if(Used+StrLen+1 > Length)
          Length = ((Used+StrLen+LISTINCR)/LISTINCR)*LISTINCR;
        if((String = realloc(String,Length))) {
          (void)strcpy(String+Used,CuDep->dependency);
          Used = Used+StrLen+1;
          *(String+Used) = '\0';
          free(CuDep);
          }
        }
      }
    return(String);
    }
  else 
    return(NULL);
  }

static void
AddFsysList(char *Name) {
  if(FsysListLength-FsysListUsed < strlen(Name)+2) {
    if(!FsysListLength)
      FsysList = malloc(LISTINCR);
    else
      FsysList = realloc(FsysList,FsysListLength+LISTINCR);
    FsysListLength += LISTINCR;
    }
  (void)strcpy(FsysList+FsysListUsed,Name);
  FsysListUsed += strlen(Name)+1;
  *(FsysList+FsysListUsed) = '\0';
  }

int
SearchODM(char *VgroupName) {
  char *LvolList,*Fsys,*DeviceName[MAXDEVICE];
  int I;

  if(VgroupName && *VgroupName) {
    LvolList = LocateDependList(VgroupName);
    for(I=0;strlen(LvolList+I);I+=strlen(LvolList+I)+1) {
      strcpy(DeviceName,DEVICE);
      strcat(DeviceName,LvolList+I);
      Fsys = GetInvFsys("dev",DeviceName);
      if(Fsys && *Fsys) {
        AddFsysList(Fsys);
        free(Fsys);
        }
      }
    free(LvolList);
    return(SUCCEEDED);
    }
  else
    return(FAILED);
  }

int
main(int Argc,char *Argv[]) {
  int RetCode,I;

  if(Argc != 2)
    exit(1);

  InitODM();

  RetCode = SearchODM(Argv[1]);

  ExitODM();

  for(I=0;strlen(FsysList+I);I+=strlen(FsysList+I)+1)
    fprintf(stdout,"%s\n",FsysList+I);

  return(RetCode);
  }

