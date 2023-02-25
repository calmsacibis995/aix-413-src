static char sccsid[] = "@(#)76	1.2  src/bos/usr/lib/pios/trans2/ibm.852.c, cmdpios, bos411, 9428A410j 7/8/93 11:03:04";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** ibm.852.c - generates Stage 2 translate table to print Code Page ***/
/***             852 on IBM printers               ***/

/*
   This is a Stage 2 translate table for translation from code points for the
   intermediate code page (generated by Stage 1 translation) to code points
   for the printer's code page.  There is one entry in the table for each
   intermediate code page code point, beginning at intermediate code page code
   point zero and ending with the highest intermediate code page code point
   that can be translated to a printer code page code point for this particular
   printer code page.

   Each entry in the translate table must be of the form

      { charvalue | CP | SC  [,index1 [,index2] ] }

   where | vertical bars separate mutually exclusive choices, and [] brackets
   surround options.  The values that may be specified are:

      charvalue - the code point for the printer code page
      CP - copy the intermediate code page code point to the printer code page
	   code point (i.e., use the same code point)
      SC - use a substitute character (can't print the character)

      index1 - index into cmdnames array that specifies a command to be output
	       BEFORE the code point is output.
      index2 - index into cmdnames array that specifies a command to be output
	       AFTER the code point is output.

   To build the translate table file, do the following:
      (1) Verify that the table values and the value defined for "CODEPAGE"
	  below are correct.
      (2) Verify that the name of this file is xxx.c, where "xxx" is the value
	  defined below for "CODEPAGE".
      (3) cc xxx.c  (where "xxx" is the value defined below for "CODEPAGE")
      (4) a.out dirname  (where "dirname" is the path name of the
			  directory where the file is to be created)
*/

#define CODEPAGE "ibm.852" /* name of the translate table file to be created */

#include "piostruct.h"



/*******************************************************************************
*         Table to Translate Code Points for the Intermediate Code Page        *
*                  to Code Points for the Printer's Code Page                  *
*******************************************************************************/
struct transtab table[] = {

/*        0        1        2        3        4        5        6        7
	  8        9        A        B        C        D        E        F
*/
/*00*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*08*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*10*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*18*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*20*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*28*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*30*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*38*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*40*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*48*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*50*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*58*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*60*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*68*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*70*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*78*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*80*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{SC}    ,{SC}    ,{CP}    ,
/*88*/  {SC}    ,{CP}    ,{SC}    ,{SC}    ,{CP}    ,{SC}    ,{CP}    ,{SC}    ,
/*90*/  {CP}    ,{SC}    ,{SC}    ,{CP}    ,{CP}    ,{SC}    ,{SC}    ,{SC}    ,
/*98*/  {SC}    ,{CP}    ,{CP}    ,{SC}    ,{SC}    ,{SC}    ,{CP}    ,{SC}    ,
/*A0*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,
/*A8*/  {SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{CP}    ,{CP}    ,
/*B0*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{SC}    ,
/*B8*/  {SC}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{SC}    ,{SC}    ,{CP}    ,
/*C0*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*C8*/  {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/*D0*/  {SC}    ,{CP}    ,{SC}    ,{CP}    ,{SC}    ,{SC}    ,{CP}    ,{CP}    ,
/*D8*/  {SC}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{SC}    ,{SC}    ,{CP}    ,
/*E0*/  {CP}    ,{CP}    ,{CP}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,
/*E8*/  {SC}    ,{CP}    ,{SC}    ,{SC}    ,{CP}    ,{CP}    ,{SC}    ,{CP}    ,
/*F0*/  {CP}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{CP}    ,{CP}    ,{CP}    ,
/*F8*/  {CP}    ,{CP}    ,{CP}    ,{SC}    ,{SC}    ,{SC}    ,{CP}    ,{CP}    ,
/*100*/ {SC}    ,{0x01,1},{0x02,1},{0x03,1},{0x04,1},{0x05,1},{0x06,1},{0x07,1},
/*108*/ {0x08,1},{0x09,1},{0x0a,1},{0x0b,1},{0x0c,1},{0x0d,1},{0x0e,1},{0x0f,1},
/*110*/ {0x10,1},{0x11,1},{0x12,1},{0x13,1},{0x14,1},{0x15,1},{0x16,1},{0x17,1},
/*118*/ {0x18,1},{0x19,1},{0x1a,1},{0x1b,1},{0x1c,1},{0x1d,1},{0x1e,1},{0x1f,1},
/*120*/  {SC}   ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}   ,
/*128*/  {SC}   ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}   ,
/*130*/  {SC}   ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}   ,
/*138*/  {SC}   ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}   ,
/*140*/  {SC}   ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}   ,
/*148*/  {SC}   ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{0xa4}  ,{0xf4} ,
/*150*/  {0x9d} ,{0x95}  ,{0x97}  ,{0xe6}  ,{0xb8}  ,{0x9b}  ,{0x8d}  ,{0xa6} ,
/*158*/  {0xbd} ,{0xa5}  ,{0xf2}  ,{0x88}  ,{0x96}  ,{0x98}  ,{0xf3}  ,{0xe7} ,
/*160*/  {0xad} ,{0x9c}  ,{0xab}  ,{0xf1}  ,{0xa7}  ,{0xbe}  ,{0xe8}  ,{0xc6} ,
/*168*/  {0x91} ,{0x8f}  ,{0xac}  ,{0xa8}  ,{0xb7}  ,{0xd2}  ,{0xe3}  ,{0xd5} ,
/*170*/  {0x8a} ,{0xfc}  ,{0xde}  ,{0xeb}  ,{0xdd}  ,{0xea}  ,{0xc7}  ,{0x92} ,
/*178*/  {0x86} ,{0x9f}  ,{0xa9}  ,{0xd8}  ,{0xd4}  ,{0xd0}  ,{0xe4}  ,{0xe5} ,
/*180*/  {0x8b} ,{0xfd}  ,{0x85}  ,{0xfb}  ,{0xee}  ,{0xfa}

};


/*******************************************************************************
*                Command Names Specified by the Translate Tables               *
*******************************************************************************/
char cmdnames[][2] = {
{'c', '7'},                     /* index 0 - select the code page */
{'e', 'b'},                     /* index 1 - next byte is graphic, not control*/
};


/*******************************************************************************
*                     Program To Write the Table To a File                     *
*******************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define HEADER "PIOSTAGE2XLATE00"  /* file header */

int num_commands = sizeof(cmdnames) / 2;

main(argc, argv)
unsigned argc;
char *argv[];
{
extern int errno;
int fildes;
int fmt_type = 1;
char filename[500];

/* Construct File Name */
if (argc < 2) {
    fprintf(stderr,"ERROR: directory path for output file must be specified\n");
    return(1);
}
strcpy(filename, argv[1]);
strcat(filename, "/");
strcat(filename, CODEPAGE);

/* Open the File */
if ((fildes = open(filename, O_CREAT | O_WRONLY, 0664)) < 0)
{
    fprintf(stderr,"ERROR: Unable to open file \"%s\", errno = %d\n",
      filename, errno);
    return(1);
}

/* Write the Data to the File */
if (write(fildes, HEADER, sizeof(HEADER) - 1) < 0
      || write(fildes, &num_commands, sizeof(num_commands)) < 0
      || write(fildes, cmdnames, sizeof(cmdnames)) < 0
      || write(fildes, table, sizeof(table)) < 0) {
    fprintf(stderr,"ERROR writing to file \"%s\", errno = %d\n",
      filename, errno);
    (void) close(fildes);
    return(1);
}
(void) close(fildes);
return(0);
}
