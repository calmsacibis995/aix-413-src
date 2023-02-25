# @(#)39	1.7.1.2  src/bos/usr/samples/README.S, bossamp, bos411, 9428A410j 8/11/93 11:06:22


 *
 * COMPONENT_NAME: (BOSSAMP) IBM BOS Sample Programs
 *
 * FUNCTIONS: README FILE FOR THE SOURCE CODE EXAMPLES
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 
 THE SOURCE CODE EXAMPLES PROVIDED ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED DEALER) ASSUME THE ENTIRE
 COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

 No warranty is given that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 The contents of these directories are subject exclusively to the terms set
 forth in the Notice to Users that can be found in each of the source code
 example files.



 Source code examples in these directories are made up of one or more source
 files.  The following shows the sub-directories and source file names which are
 associated with each example program.  Other sub-directories may exist but
 do not contain samples for the base operating system.  These sub-directories
 should contain their own documentation and are not included here.

 Example Program             Source Code File Names and Comments
 -----------------------     -------------------------------------------
 Audio application	     acpa/audio.c        Audio device driver utility

 Diskette device driver      fd/fd.c             top-half driver routines
			     fd/fd_bot.c         bottom-half driver routines
			     fd/fd_local.h       header file 

 CD-ROM device driver        cdrom/cdromt.c      top-half driver routines
			     cdrom/cdromb.c      bottom-half driver routines
			     cdrom/cdromdd.h     header file 

 Ethernet device driver      ent/ciodd.c         Common Communications I/O
					         device interface routines
                                                 (top-half driver routines)
			     ent/cioddi.h        Common Comm. I/O header file
			     ent/ciodds.h        Common Comm. I/O header file
			     ent/cioddhi.h       Common Comm. I/O header file
			     ent/cioddlo.h       Common Comm. I/O header file
			     ent/entds.c         device specific routines
			     ent/entxmit.c       data transmission routines
			     ent/entrecv.c       data reception routines 
			     ent/entddi.h        device specific header file  
			     ent/entdshi.h       device specific header file  
			     ent/entdslo.h       device specific header file  

 device methods              cfg/xxx.add         predefined data for adapter xxx
 			     cfg/yyy.add         predefined data for tape yyy
 			     cfg/defxxx.c        define method for adapter xxx
			     cfg/cfgxxx.c        configure method for adapter
						 xxx
 			     cfg/ucfgxxx.c       unconfigure method for adapter
						 xxx
			     cfg/udefxxx.c       undefine method for adapter xxx
			     cfg/defyyy.c        define method for tape drive
						 yyy
			     cfg/cfgyyy.c        configure method for tape drive
					         yyy
			     cfg/chgyyy.c        change method for tape drive
						 yyy.
			     cfg/ucfgyyy.c       unconfigure method for tape
						 drive yyy
			     cfg/udefyyy.c       undefine method for tape drive
						 yyy
			     cfg/zzz.add         predefined data for pseudo
						 device zzz
			     cfg/startzzz.c      node configuration program for
                                                 pseudo device zzz
			     cfg/defzzz.c        define method for pseudo device
					         zzz
			     cfg/cfgzzz.c        configure method for pseudo
						 device zzz

 psmalloc                    ker/psmalloc.c      allocate memory through malloc
					         and make sure that paging
					         space is allocated for it

 floating point		     fpe/sigfpe_samp1.c  C language example
 exception trapping	     fpe/sigfpe_samp2.c  helper functions for FORTRAN
						 example
			     fpe/sigfpe_samp3.f  FORTRAN language example


                ADDITIONAL NOTE TO METHODS WRITERS


 The sample device methods illustrate the configuration functions of two
 different device models.  One consists of an adapter card with tape devices
 connected.  The other model consists of a pseudo device.


 Model 1
 The devices consist of a micro channel adapter card xxx which controls
 from 1 to 4 tape devices of type yyy.

 The files to support this are:

 xxx.add         The predefined data for the adapter xxx.
 yyy.add         The predefined data for the tape yyy.

 defxxx.c        The define method for the adapter xxx.
 cfgxxx.c        The configure method for the adapter xxx.
 chgxxx.c        The change method for the adapter xxx. NOT INCLUDED IN
                 SAMPLES.
 ucfgxxx.c       The unconfigure method for the adapter xxx.
 udefxxx.c       The undefine method for the adapter xxx.

 defyyy.c        The define method for the tape drive yyy.
 cfgyyy.c        The configure method for the tape drive yyy.
 chgyyy.c        The change method for the tape drive yyy.
 ucfgyyy.c       The unconfigure method for the tape drive yyy.
 udefyyy.c       The undefine method for the tape drive yyy.



 Model 2
 The device consists of a pseudo device zzz.  Its configuration is initiated
 by the configuration manager invoking the startzzz program listed in one of
 the config rules.

 The files to support this are:

 zzz.add         The predefined data for the pseudo device zzz.

 startzzz.c      The node configuration program for the pseudo device zzz.
 defzzz.c        The define method for the pseudo device zzz.
 cfgzzz.c        The configure method for the pseudo device zzz.
 chgzzz.c        The change method for the pseudo device zzz.  NOT INCLUDED
                 IN SAMPLES.
 ucfgzzz.c       The unconfigure method for the pseudo device zzz.  NOT
                 INCLUDED IN SAMPLES.
 udefzzz.c       The undefine method for the pseudo device zzz.  NOT INCLUDED
                 IN SAMPLES.



                ADDITIONAL NOTE TO DEVICE DRIVER WRITERS

 If you want to add a "trap" instruction to your code, so that you can
 invoke the Low Level Debugger when a particular condition occurs,
 you can use code similar to the following in your program:

	brkpoint();

 To manually invoke the debugger, the system's keylock must be placed 
 in the SERVICE position, and then one of the prescribed key sequences
 must be entered.  To invoke the debugger from an attached serial 
 terminal, type CTRL-\ or CTRL-4, whichever works from your terminal.
 To invoke the debugger from the system console keyboard, type
 CTRL-ALT-numpad 4.  The debugger will use the first available serial
 port that it determines is in use as its input/output device.

 Before you can use the debugger, it has to be available in the system.
 To make the debugger available for use, you must execute the command:

	bosboot -a -D -d Device
 or
	bosboot -a -I -d Device

 where Device is the target physical boot device.

 The -D option makes the debugger available the next time you IPL, but 
 does not invoke the debugger during IPL.  The -I option makes the 
 debugger available at the next IPL, and also invokes it during IPL.
