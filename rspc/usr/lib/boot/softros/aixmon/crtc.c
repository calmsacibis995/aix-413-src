static char sccsid[] = "@(#)94	1.3  src/rspc/usr/lib/boot/softros/aixmon/crtc.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:08";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: setup
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	crtc.c - Used to setup environment prior to passing control to main.
 *
 *	Beware, this version of crtc.c relies on the hints that are placed into
 *	the monitor image by the image build tools. The old method of subtracting
 *	global variables is unreliable. The code remains here commented out for
 *	now. For debugging purposes, the global values are written back to the
 *	image. 
 */

#include <stddef.h>
#include <stand.h>

#define  HINT_SIZE	48	/* Size of hint header for AIXMON	*/

extern char edata[], end[];
extern void (*__start)();

extern void set_progp( prog_t * );

extern int  mode_control;	/* Boot mode control for AIXMON  	*/
extern char *residual_save;	/* Pointer to residual data area	*/
extern int  header_block_size;	/* Offset blocks to AIX boot image 	*/
extern char *mem_load_address;
extern unsigned int mem_jump_offset;

char * environ;                 /* Pointer to the environment    */
void (*promaddr)();             /* Start address for the monitor */

void setup(int *orig)
{
unsigned int	header_size, load_orig, jump_to;


prog_t prog =
        { (long)__start, ((long)end - (long)__start), (long)__start, {0,0,0} };

*(orig - 4) = (long)end - (long)edata;

        			/* Zero out the BSS using hints in load image*/
bzero((char *)*(orig - 3),*(orig - 2));

				/* Set the mode control for AIXMON */
mode_control = *(orig -1);	

				/* Store the offset to the boot image	*/
				/* Figure out where the jump addr is	*/
header_block_size = *(orig -5);
header_size = *(orig -6) + HINT_SIZE;	/* Adjust for hints area	*/
load_orig   = *(orig -7);

		/* For now there are two header blocks before aixmon start */
		/* so adjust size by those header blocks. We need to get the */
		/* loaded address from ROS so we don't need these funny calcs */
		/* If not, I will put the header offset into hints 	      */

jump_to = load_orig + (header_block_size * 512);
jump_to -= header_size;

*(orig - 5) = jump_to;		/* For debug put it back to hints area  */

*(uint *)(&mem_load_address) = jump_to;	/* Setup calculated jump address*/

mem_jump_offset = *(orig - 8);	/* Get jump offset to memory copy	*/

*(int *)(&residual_save) = *(orig-11); /* Store residual data address	*/

			        /* Init sbrk stuff */
set_progp(&prog);

			        /* Init promaddr */
promaddr = __start;


			        /* Ready to go */
main();

			        /* We're done lets get outta here */
__exit();
}

