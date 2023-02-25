/* @(#)44	1.2  src/rspc/usr/lib/boot/softros/roslib/screen_fpd.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:52:20  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
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

#include <sys/types.h>

/*=========================================================================*/
/* Function:     cursor                                                    */
/* Description:  Move cursor to absolute (x,y) position                    */
/*               (x,y) must be within screen range                         */
/* Input:        Horizontal location on screen                             */
/*               Vertical location on screen                               */
/* Ouput:        NoERROR                                                   */
/*               ERROR                                                     */
/*                                                                         */
int  cursor( int x, int y );

/*=========================================================================*/
/* Function:     rel_cursor                                                */
/* Description:  Move cursor to relative (x,y) position                    */
/* Input:        Horizontal displacement of cursor                         */
/*               Vertical displacement of cursor                           */
/* Ouput:        NoERROR                                                   */
/*                                                                         */
int  rel_cursor( int x, int y );

/*=========================================================================*/
/* Function:     get_cursor                                                */
/* Description:  Return current cursor position                            */
/* Input:        Pointer to horizontal value                               */
/*               Pointer to vertical value                                 */
/* Ouput:        NoERROR                                                   */
/*                                                                         */
int  get_cursor( int *x, int *y );

/*=========================================================================*/
/* Function:     scroll_up                                                 */
/* Description:  Scroll up the screen                                      */
/* Input:        none                                                      */
/* Ouput:        none                                                      */
/*                                                                         */
void  scroll_up( void );

/*=========================================================================*/
/* Function:     scroll_down                                               */
/* Description:  Scroll down the screen                                    */
/* Input:        none                                                      */
/* Ouput:        none                                                      */
/*                                                                         */
void  scroll_down( void );

/*=========================================================================*/
/* Function:     cls                                                       */
/* Description:  Clear screen                                              */
/* Input:        none                                                      */
/* Ouput:        none                                                      */
/*                                                                         */
void  cls( void );
