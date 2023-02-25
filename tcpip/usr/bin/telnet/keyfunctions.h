/* @(#)67	1.4  src/tcpip/usr/bin/telnet/keyfunctions.h, tcptelnet, tcpip411, GOLD410 1/30/93 09:44:39 */
/* 
 * COMPONENT_NAME: TCPIP keyfunctions.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* declarations for routines implementing keyboard functions */

extern int	fs_keyboard_locked;

extern void	fs_alarm ();
extern int	fs_backspace ();
extern int	fs_backtab ();
extern int	fs_character ();
extern int	fs_clear ();
extern int	fs_delete ();
extern int	fs_down ();
extern int	fs_dup ();
extern int	fs_enter ();
extern int	fs_eraseeof ();
extern int	fs_eraseinput ();
extern int	fs_fieldmark ();
extern int	fs_home ();
extern int	fs_insertmode ();
extern int	fs_left ();
extern int	fs_pa1 ();
extern int	fs_pa2 ();
extern int	fs_pa3 ();
extern int	fs_penselect ();
extern int	fs_pf1 ();
extern int	fs_pf10 ();
extern int	fs_pf11 ();
extern int	fs_pf12 ();
extern int	fs_pf13 ();
extern int	fs_pf14 ();
extern int	fs_pf15 ();
extern int	fs_pf16 ();
extern int	fs_pf17 ();
extern int	fs_pf18 ();
extern int	fs_pf19 ();
extern int	fs_pf2 ();
extern int	fs_pf20 ();
extern int	fs_pf21 ();
extern int	fs_pf22 ();
extern int	fs_pf23 ();
extern int	fs_pf24 ();
extern int	fs_pf3 ();
extern int	fs_pf4 ();
extern int	fs_pf5 ();
extern int	fs_pf6 ();
extern int	fs_pf7 ();
extern int	fs_pf8 ();
extern int	fs_pf9 ();
extern int	fs_reset ();
extern int	fs_return ();
extern int	fs_right ();
extern int	fs_sysreq ();
extern int	fs_tab ();
extern int	fs_up ();
extern int	fs_wordnext ();
extern int	fs_wordprev ();
extern int	fs_attention ();
