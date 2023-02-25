/* @(#)72	1.5  src/tcpip/usr/bin/telnet/tokentype.h, tcptelnet, tcpip411, GOLD410 11/4/93 08:43:26 */
/* 
 * COMPONENT_NAME: TCPIP tokentype.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  27  38 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *	Copyright 1985 by Paul Milazzo - all rights reserved
 */

/*
 *  token types for the IBM 3278 emulator keyboard profile file
 */

#define	T_IDENT	257
#define	T_STRING	258
#define	T_BIND	259
#define	T_ELSE	260
#define	T_EXIT	261
#define	T_IF	262
#define	T_LOAD	263
#define	T_OR	264
#define	T_PRINT	265
#define	T_QUIT	266
#define	T_CHARACTER	267
#define	T_INSERTMODE	268
#define	T_DELETE	269
#define	T_BACKSPACE	270
#define	T_TAB	271
#define	T_BACKTAB	272
#define	T_RETURN	273
#define	T_UP	274
#define	T_DOWN	275
#define	T_LEFT	276
#define	T_RIGHT	277
#define	T_HOME	278
#define	T_ERASEINPUT	279
#define	T_ERASEEOF	280
#define	T_DUP	281
#define	T_FIELDMARK	282
#define	T_RESET	283
#define	T_CLEAR	284
#define	T_ENTER	285
#define	T_PA1	286
#define	T_PA2	287
#define	T_PA3	288
#define	T_PENSELECT	289
#define	T_PF1	290
#define	T_PF2	291
#define	T_PF3	292
#define	T_PF4	293
#define	T_PF5	294
#define	T_PF6	295
#define	T_PF7	296
#define	T_PF8	297
#define	T_PF9	298
#define	T_PF10	299
#define	T_PF11	300
#define	T_PF12	301
#define	T_PF13	302
#define	T_PF14	303
#define	T_PF15	304
#define	T_PF16	305
#define	T_PF17	306
#define	T_PF18	307
#define	T_PF19	308
#define	T_PF20	309
#define	T_PF21	310
#define	T_PF22	311
#define	T_PF23	312
#define	T_PF24	313
#define	T_SYSREQ	314
#define T_WORDNEXT	315
#define T_WORDPREV	316
#define T_ATTENTION	317 

#define T_AUTOPUSH      318
#define T_SCRREV        319
#define T_ENG_LANG      320
#define T_NLS_LANG      321
#define T_PUSH          322
#define T_ENDPUSH       323
#define T_FLDREV        324
#define T_AUTOREV       325

#define T_AUTOSHAPE     326
#define T_ISOLATED      327
#define T_INITIAL       328
#define T_MIDDLE        329
#define T_FINAL         330
#define T_FIELDSHAPE    331
#define T_FIELDBASE     332

#define	T_ILLEGAL	333
#define	T_LOWPROT	334
#define	T_LOWUNPROT	335
#define	T_HIGHPROT	336
#define	T_HIGHUNPROT	337
#define	T_BACKGROUND	338
