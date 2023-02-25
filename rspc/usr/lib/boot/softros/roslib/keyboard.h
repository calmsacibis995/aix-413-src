/* @(#)17	1.1  src/rspc/usr/lib/boot/softros/roslib/keyboard.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:38:50  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*-------------  Keyboard Definitions ----------------------------------------*/

#define POWER_FAIL  0xA5A5              /* value of POWER_FAIL on SDF Adapter */
#define ENTER_KEY   0x1c0D              /* value of ENTER key                 */
#define BS_KEY      0x0e08              /* value of BackSapce Key             */
#define PGUP_KEY    0x4900              /* value of PAGE UP key               */
#define PGDN_KEY    0x5100              /* value of PAGE DOWN key             */
#define CRUP_KEY    0x4800              /* value of CURSOR UP key             */
#define CRDN_KEY    0x5000              /* value of CURSOR DOWN               */
#define ESC_KEY     0x011b              /* value of ESCAPE key                */
#define SPA_KEY     0x3920              /* value of SPACE BAR key             */
#define HOME_KEY    0x4700              /* value of HOME key                  */
#define END_KEY     0x4f00              /* value of END key                   */
#define CRRT_KEY    0x4D00              /* value of CURSOR RIGHT              */
#define CRLF_KEY    0x4B00              /* value of CURSOR LEFT               */
#define TABR_KEY    0x0F09              /* value of TAB RIGHT                 */
#define TABL_KEY    0x0F00              /* value of TAB LEFT                  */
#define INS_KEY     0x5200              /* value of Insert Key                */
#define DEL_KEY     0x5300              /* value of Delete Key                */
#define F1_KEY      0x3b00              /* value of F1 key                    */
#define F2_KEY      0x3c00              /* value of F2 key                    */
#define F3_KEY      0x3d00              /* value of F3 key                    */
#define F4_KEY      0x3e00              /* value of F4 key                    */
#define F5_KEY      0x3f00              /* value of F5 key                    */
#define F6_KEY      0x4000              /* value of F6 key                    */
#define F7_KEY      0x4100              /* value of F7 key                    */
#define F8_KEY      0x4200              /* value of F8 key                    */
#define F9_KEY      0x4300              /* value of F9 key                    */
#define F10_KEY     0x4400              /* value of F10 key                   */
#define F11_KEY     0x5700              /* value of F11 key                   */
#define F12_KEY     0x5800              /* value of F12 key                   */

#define EPGUP_KEY   0x49e0              /* value of PAGE UP key               */
#define EPGDN_KEY   0x51e0              /* value of PAGE DOWN key             */
#define ECRUP_KEY   0x48e0              /* value of CURSOR UP key             */
#define ECRDN_KEY   0x50e0              /* value of CURSOR DOWN               */
#define EHOME_KEY   0x47e0              /* value of HOME key                  */
#define EEND_KEY    0x4fe0              /* value of END key                   */
#define ECRRT_KEY   0x4De0              /* value of CURSOR RIGHT              */
#define ECRLF_KEY   0x4Be0              /* value of CURSOR LEFT               */
#define EINS_KEY    0x52e0              /* value of Insert Key                */
#define EDEL_KEY    0x53e0              /* value of Delete Key                */
#define EENTER_KEY  0xe00D              /* value of ENTER key                 */


#define ACRRT_KEY   0x9D00              /* value of Alt- ECURSOR RIGHT        */
#define ACRLF_KEY   0x9B00              /* value of Alt- ECURSOR LEFT         */
#define ACRUP_KEY   0x9800              /* value of Alt- ECURSOR UP           */
#define ACRDN_KEY   0xA000              /* value of Alt- ECURSOR DOWN         */
#define APGUP_KEY   0x9900              /* value of Alt- PAGE UP key          */
#define APGDN_KEY   0xA100              /* value of Alt- PAGE DOWN key        */
#define CCRRT_KEY   0x74e0              /* value of Ctl- ECURSOR RIGHT        */
#define CCRLF_KEY   0x73e0              /* value of Ctl- ECURSOR LEFT         */
#define CCRUP_KEY   0x8de0              /* value of Ctl- ECURSOR UP           */
#define CCRDN_KEY   0x91e0              /* value of Ctl- ECURSOR DOWN         */
#define CPGUP_KEY   0x84e0              /* value of Ctl- PAGE UP key          */
#define CPGDN_KEY   0x76e0              /* value of Ctl- PAGE DOWN key        */

#define left_shift_key  0x2a
#define right_shift_key 0x36

#define left_shift  0x01
#define right_shift 0x02

#define i8042_Command_Port 0x80000064
#define i8042_Data_Port    0x80000060
#define Keyboard 1
#define Mouse    2
#define out_buf_full  0x01
#define inpt_buf_full 0x02
#define aux_buf_full  0x20


int check_key_avail(void);

int getchar(void);
