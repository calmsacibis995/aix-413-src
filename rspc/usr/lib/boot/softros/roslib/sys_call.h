/* @(#)61	1.1  src/rspc/usr/lib/boot/softros/roslib/sys_call.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:14  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: getchar
 *		putchar
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
//************************** OCO SOURCE MATERIALS *****************************
//******* IBM CONFIDENTIAL (IBM CONFIDENTIAL-RESTRICTED when combined *********
//************** with the aggregated modules for this product) ****************
//*****************************************************************************
// FILE NAME:          sys_call.h
// FILE CREATED:       07/21/93 by D. Cronk
// FILE OWNER:         D. Cronk
//
// COPYRIGHT:          (C) Copyright IBM Corp. 1993, 1993
//
// DESCRIPTION: This file contains system service call data structures and
//              routine definitions.
//
// REVISION HISTORY:
// -----------------------Sandalfoot-----------------------------------------
// Date      Track#     Who  Description
// --------  ---------- ---  ------------------------------------------------
//
//*****************************************************************************

#ifndef _H_SYS_CALL
#define _H_SYS_CALL

#ifndef _STDIO
#include <stdio.h>
#endif

/* To define a new system service routine -                                  */
/*                                                                           */
/* - Add the routine to the end of the system_service_routine array          */
/*                                                                           */
/* - After the "else" conditional, define the three lines to allow the       */
/*   routine name to be replaced with a direct function vector.  Notice that */
/*   the array index of the routine determines what vector it can be called  */
/*   through.                                                                */
/*                                                                           */

/*---------------------------------------------------------------------------*/
/*               Variable definitions, common equates.....                   */

#define SC_CONSOLE_INIT 0x0010
#define SC_CURSOR_SIZE  0x0011
#define SC_LOAD_PALETTE 0x0012

#ifdef DEFINE_SYS_CALL_ARRAY

#ifndef DEFINE_SYS_CALL_EQUATES_ONLY

int sys_call(int function, ... );
extern void bad_int_call();
extern get_bus_speed_info();

/* The int_hndlr.c file uses the DEFINE_SYS_CALL_ARRAY equate so that the system service */
/* routine array is declared/defined/initialized here.                                   */

/* Only one file in a build should have this array defined.                              */

unsigned int system_service_routine[] = {
                                              getchar,          /* index = 0, Vector = 1000 */
                                              putchar,          /* index = 1, Vector = 1020 */
                                              scanf,            /* index = 2, Vector = 1040 */
                                              printf,           /* index = 3, Vector = 1060 */
                                              sscanf,           /* index = 4, Vector = 1080 */
                                              sprintf,          /* index = 5, Vector = 10a0 */

                                              fprintf,          /* index = 6, Vector = 10c0 */
                                              fopen,            /* index = 7, Vector = 10e0 */
                                              fflush,           /* index = 8, Vector = 1100 */
                                              fclose,           /* index = 9, Vector = 1120 */
                                              fseek,            /* index =10, Vector = 1140 */
                                              ftell,            /* index =11, Vector = 1160 */
                                              rewind,           /* index =12, Vector = 1180 */
                                              fgetc,            /* index =13, Vector = 11a0 */
                                              fputc,            /* index =14, Vector = 11c0 */
                                              fgets,            /* index =15, Vector = 11e0 */
                                              fputs,            /* index =16, Vector = 1200 */
                                              fread,            /* index =17, Vector = 1220 */
                                              fwrite,           /* index =18, Vector = 1240 */
                                              remove,           /* index =19, Vector = 1260 */
                                              rename,           /* index =20, Vector = 1280 */
                                              clearerr,         /* index =21, Vector = 12a0 */
                                              ffind_first,      /* index =22, Vector = 12c0 */
                                              ffind_next,       /* index =23, Vector = 12e0 */
         /* RESERVED for DEBUG INTERRUPT */   bad_int_call,     /* index =24, Vector = 1300 */
                                              mk_dir,           /* index =25, Vector = 1320 */
                                              rm_dir,           /* index =26, Vector = 1340 */

                                              malloc,           /* index =27, Vector = 1360 */
                                              calloc,           /* index =28, Vector = 1380 */
                                              free,             /* index =29, Vector = 13a0 */

                                              putbitmap,        /* index =30, Vector = 13c0 */
                                              rect_fill,        /* index =31, Vector = 13e0 */

                                              get_bus_speed_info,/* index =32, Vector = 1400 */
                                              change_dir,       /* index =33, Vector = 1420 */
                                              sys_call,         /* index =34, Vector = 1440 */

                                              NULL
                                            };

#define MAX_SYS_SERV_INDEX ( sizeof(system_service_routine) / sizeof(system_service_routine[0]))

#endif

#else

/* Files that don't use the DEFINE_SYS_CALL_ARRAY equate when including this file will   */
/* get the system service routine replacements.  In other words, the file might have a   */
/* call to getchar, but it will be replaced with a call to the getchar system service    */
/* vector.                                                                               */

#ifdef getchar
#undef getchar
#endif

#define GETCHAR_INDEX  0
static int (*getchar_ptr) (void) = (GETCHAR_INDEX * 0x20) + 0x1000 + 4;
#define getchar()  (*getchar_ptr)()

#ifdef putchar
#undef putchar
#endif

#define PUTCHAR_INDEX  1
static int (*putchar_ptr) (int) = (PUTCHAR_INDEX * 0x20) + 0x1000 + 4;
#define putchar(a)  (*putchar_ptr)(a)


#ifdef scanf
#undef scanf
#endif

#define SCANF_INDEX  2
static int (*scanf_ptr) (const char *, ...) = (SCANF_INDEX * 0x20) + 0x1000 + 4;
#define scanf  (*scanf_ptr)


#ifdef printf
#undef printf
#endif

#define PRINTF_INDEX  3
static int (*printf_ptr) (const char *, ...) = (PRINTF_INDEX * 0x20) + 0x1000 + 4;
#define printf  (*printf_ptr)


#ifdef sscanf
#undef sscanf
#endif

#define SSCANF_INDEX  4
static int (*sscanf_ptr) (char *, const char *, ...) = (SSCANF_INDEX * 0x20) + 0x1000 + 4;
#define sscanf  (*sscanf_ptr)

#ifdef sprintf
#undef sprintf
#endif

#define SPRINTF_INDEX  5
static int (*sprintf_ptr) (char *, const char *, ...) = (SPRINTF_INDEX * 0x20) + 0x1000 + 4;
#define sprintf  (*sprintf_ptr)

#ifdef fprintf
#undef fprintf
#endif

#define FPRINTF_INDEX  6
static int (*fprintf_ptr) (FILE *, const char *, ...) = (FPRINTF_INDEX * 0x20) + 0x1000 + 4;
#define fprintf  (*fprintf_ptr)

#ifdef fopen
#undef fopen
#endif

#define FOPEN_INDEX  7
static FILE * (*fopen_ptr) (const char *, const char *) = (FOPEN_INDEX * 0x20) + 0x1000 + 4;
#define fopen  (*fopen_ptr)

#ifdef fflush
#undef fflush
#endif

#define FFLUSH_INDEX  8
static int (*fflush_ptr) (FILE *) = (FFLUSH_INDEX * 0x20) + 0x1000 + 4;
#define fflush  (*fflush_ptr)

#ifdef fclose
#undef fclose
#endif

#define FCLOSE_INDEX  9
static int (*fclose_ptr) (FILE *) = (FCLOSE_INDEX * 0x20) + 0x1000 + 4;
#define fclose  (*fclose_ptr)

#ifdef fseek
#undef fseek
#endif

#define FSEEK_INDEX  10
static int (*fseek_ptr) (FILE *, long, int) = (FCLOSE_INDEX * 0x20) + 0x1000 + 4;
#define fseek  (*fseek_ptr)

#ifdef ftell
#undef ftell
#endif

#define FTELL_INDEX  11
static long (*ftell_ptr) (FILE *) = (FTELL_INDEX * 0x20) + 0x1000 + 4;
#define ftell  (*ftell_ptr)

#ifdef rewind
#undef rewind
#endif

#define REWIND_INDEX  12
static void (*rewind_ptr) (FILE *) = (REWIND_INDEX * 0x20) + 0x1000 + 4;
#define rewind  (*rewind_ptr)

#ifdef fgetc
#undef fgetc
#endif

#define FGETC_INDEX  13
static int (*fgetc_ptr) (FILE *) = (FGETC_INDEX * 0x20) + 0x1000 + 4;
#define fgetc  (*fgetc_ptr)

#ifdef fputc
#undef fputc
#endif

#define FPUTC_INDEX  14
static int (*fputc_ptr) (int, FILE *) = (FPUTC_INDEX * 0x20) + 0x1000 + 4;
#define fputc  (*fputc_ptr)

#ifdef fgets
#undef fgets
#endif

#define FGETS_INDEX  15
static int (*fgets_ptr) (char *, int, FILE *) = (FGETS_INDEX * 0x20) + 0x1000 + 4;
#define fgets  (*fgets_ptr)

#ifdef fputs
#undef fputs
#endif

#define FPUTS_INDEX  16
static int (*fputs_ptr) (char *, FILE *) = (FPUTS_INDEX * 0x20) + 0x1000 + 4;
#define fputs  (*fputs_ptr)

#ifdef fread
#undef fread
#endif

#define FREAD_INDEX  17
static int (*fread_ptr) (void *, int, int, FILE *) = (FREAD_INDEX * 0x20) + 0x1000 + 4;
#define fread  (*fread_ptr)

#ifdef fwrite
#undef fwrite
#endif

#define FWRITE_INDEX  18
static int (*fwrite_ptr) (void *, int, int, FILE *) = (FWRITE_INDEX * 0x20) + 0x1000 + 4;
#define fwrite  (*fwrite_ptr)

#ifdef remove
#undef remove
#endif

#define REMOVE_INDEX  19
static int (*remove_ptr) (const char *) = (REMOVE_INDEX * 0x20) + 0x1000 + 4;
#define remove  (*remove_ptr)

#ifdef rename
#undef rename
#endif

#define RENAME_INDEX  20
static int (*rename_ptr) (const char *, const char *) = (RENAME_INDEX * 0x20) + 0x1000 + 4;
#define rename  (*rename_ptr)

#ifdef clearerr
#undef clearerr
#endif

#define CLEARERR_INDEX  21
static int (*clearerr_ptr) (FILE *) = (CLEARERR_INDEX * 0x20) + 0x1000 + 4;
#define clearerr  (*clearerr_ptr)

#ifdef ffind_first
#undef ffind_first
#endif

#define FFIND_FIRST_INDEX 22
static int (*ffind_first_ptr) (FIND_T *) = (FFIND_FIRST_INDEX * 0x20) + 0x1000 + 4;
#define ffind_first  (*ffind_first_ptr)

#ifdef ffind_next
#undef ffind_next
#endif

#define FFIND_NEXT_INDEX 23
static int (*ffind_next_ptr) (FIND_T *) = (FFIND_NEXT_INDEX * 0x20) + 0x1000 + 4;
#define ffind_next  (*ffind_next_ptr)


// WARNING - #33 is reserved for the DEBUGGER

#ifdef mk_dir
#undef mk_dir
#endif

#define MK_DIR_INDEX 25
static int (*mk_dir_ptr) (const char *) = (MK_DIR_INDEX * 0x20) + 0x1000 + 4;
#define mk_dir  (*mk_dir_ptr)

#ifdef rm_dir
#undef rm_dir
#endif

#define RM_DIR_INDEX 26
static int (*rm_dir_ptr) (const char *) = (RM_DIR_INDEX * 0x20) + 0x1000 + 4;
#define rm_dir  (*rm_dir_ptr)

#ifdef malloc
#undef malloc
#endif

#define MALLOC_INDEX 27
static void *(*malloc_ptr) (const int) = (MALLOC_INDEX * 0x20) + 0x1000 + 4;
#define malloc  (*malloc_ptr)

#ifdef calloc
#undef calloc
#endif

#define CALLOC_INDEX 28
static void *(*calloc_ptr) (const int,const int) = (CALLOC_INDEX * 0x20) + 0x1000 + 4;
#define calloc  (*calloc_ptr)

#ifdef free
#undef free
#endif

#define FREE_INDEX 29
static void  (*free_ptr) (const void *) = (FREE_INDEX * 0x20) + 0x1000 + 4;
#define free  (*free_ptr)

#ifdef putbitmap
#undef putbitmap
#endif

#define PUTBITMAP_INDEX 30
static void  (*putbitmap_ptr) (const void *, const int, const int) = (PUTBITMAP_INDEX * 0x20) + 0x1000 + 4;
#define putbitmap  (*putbitmap_ptr)

#ifdef rect_fill
#undef rect_fill
#endif

#define RECT_FILL_INDEX 31
static int  (*rect_fill_ptr) (const int,
                              const int,
                              const int,
                              const int,
                              const int) = (RECT_FILL_INDEX * 0x20) + 0x1000 + 4;
#define rect_fill  (*rect_fill_ptr)

#define GET_BUS_SPD_INDEX 32
static int (*get_bus_spd_ptr) (void) = (GET_BUS_SPD_INDEX * 0x20) + 0x1000 + 4;
#define get_sys_speeds  (*get_bus_spd_ptr)

#ifdef change_dir
#undef change_dir
#endif

#define CHANGE_DIR_INDEX 33
static int (*change_dir_ptr) (const char *) = (CHANGE_DIR_INDEX * 0x20) + 0x1000 + 4;
#define change_dir  (*change_dir)

#ifdef sys_call
#undef sys_call
#endif

#define SYS_CALL_INDEX  34
static int (*sys_call_ptr) (const int, ...) = (SYS_CALL_INDEX * 0x20) + 0x1000 + 4;
#define sys_call  (*sys_call_ptr)


#ifdef get_sys_speeds
#undef get_sys_speeds
#endif

#endif /* Else ROM */



#endif  /* _H_SYS_CALL */
