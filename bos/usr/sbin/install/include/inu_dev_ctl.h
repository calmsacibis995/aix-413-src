/* @(#)05       1.7  src/bos/usr/sbin/install/include/inu_dev_ctl.h, cmdinstl, bos411, 9428A410j 4/14/94 17:03:28 */
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef __H_DEV_CTL
#define __H_DEV_CTL

# define LRECL  512
# define TSEEK_REL      1
# define TSEEK_ABS      2
# define NO_PROMPT      -1

int    tchg_blksize (char * media,
                   int    newsize);

int topen (char *, int, int, int);

void tclose (int);

int treset (int);

int trewind (int);

int tseek_file (int, int, int);

int ttape_size (int);

/* added for feature 43659 */
int     tchg_blksize_to_zero (int, int,int *);
int     get_block_size (int);
boolean is_it_a_tape_device (char *);
void    get_correct_interface (char **, boolean *);
void    get_dev_name (char *, char *, char *);


#endif
