static char sccsid[] = "@(#)10  1.6  src/bos/usr/sbin/install/inusave/strip.c, cmdinstl, bos411, 9428A410j 6/10/93 18:30:52";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: strip (inusave)
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

void strip(filename)
/*
 * NAME: strip(filename)
 *
 * FUNCTION: Removes the leading character from a path name
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * NOTE: In this context, it is used to remove the leading period from a path
 *       name which is specified relatively to the root (/) directory.
 *
 * RETURNS: None.
 */
char filename[];	      /* contains the path name	of a file      */
{
    int	i;		/* index into filename	*/
    int j;		/* index into filename	*/

/* Begin strip	*/

    /**********
    ** if the first character is a '.' then strip
    **********/
    if ( filename[0] == '.' ) {
    	for	(i=0,j=1; (filename[j] != '\n')	&& (filename[j]	!= '\0'); i++,j++)
		filename[i] = filename[j];
    	filename[i]	= '\0';
    }
}
