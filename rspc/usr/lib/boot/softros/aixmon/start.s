# @(#)03	1.3  src/rspc/usr/lib/boot/softros/aixmon/start.s, rspc_softros, rspc411, 9432A411a 8/5/94 16:33:53
#
#   COMPONENT_NAME: RSPC_SOFTROS
#
#   FUNCTIONS: DATA
#		STK_FRM_SIZ
#		S_PROLOG
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#############################################################################
# start.s -	Entry point for System ROS 
#		This code must be linked as the first module in the 
#		AIX soft ROS for the Sandalfoot product. 
#
#		This procedure relies on an executable that has been 
#		prepared by the boot header program to include some 
#		xcoff information in a well known place in the boot
#		header. 
#
#		The xcoff information (hints) provided are:
#			__start - 16 = Number of bytes to Relocate
#			__start - 12 = Address of BSS section
#			__start -  8 = Length of BSS section
#
#		This module is self-relocating and will allow loading
#		to an arbitrary memory address and relocate itself 
#		to the target address.
#
###########################################################################
#
#	Flow:
#		- Disable External Interrupts
#		- Determine Entry point and save
#		- Relocate to TARGET_ADDR
#		- Setup segment registers for I/O access
#		- Initialize TOC
#		- Initialize Stack
#		- Initialize BSS 
#		- Call Main
#
#***********************************************************************

.machine "ppc"

#
# BAT 0&1 values for 603/604
#
#       Segment 0
#       BAT 0 - 256MB      EA   00000000 - 0FFFFFFF
#                        REAL   00000000 - 0FFFFFFF
        .set    uu0.bat_rspc_seg, 0x0000
        .set    ul0.bat_rspc_seg, 0x1FFE
        .set    lu0.bat_rspc_seg, 0x0000
        .set    ll0.bat_rspc_seg, 0x002A

#       Bus Memory registers
#       BAT 1 - 256MB      EA   C0000000 - CFFFFFFF
#                        REAL   C0000000 - CFFFFFFF
        .set    uu1.bat_rspc_seg, 0xC000
        .set    ul1.bat_rspc_seg, 0x1FFE
        .set    lu1.bat_rspc_seg, 0xC000
        .set    ll1.bat_rspc_seg, 0x002A

#       I/O segments - ISA Ports
#       BAT 2 - 256MB      EA   80000000 - 8FFFFFFF
#                        REAL   80000000 - 8FFFFFFF
        .set    uu2.bat_rspc_seg, 0x8000
        .set    ul2.bat_rspc_seg, 0x1FFE
        .set    lu2.bat_rspc_seg, 0x8000
        .set    ll2.bat_rspc_seg, 0x002A

#       I/O segments - PCI
#       BAT 3 - 256MB      EA   A0000000 - AFFFFFFF
#                        REAL   A0000000 - AFFFFFFF
        .set    uu3.bat_rspc_seg, 0xA000
        .set    ul3.bat_rspc_seg, 0x1FFE
        .set    lu3.bat_rspc_seg, 0xA000
        .set    ll3.bat_rspc_seg, 0x002A

	.set	STACK_VALUE,0x000A0000

	.set	TARGET_ADDR,0x5000
	.set    TARGET_H,   TARGET_ADDR > 16
	.set    TARGET_L,   TARGET_ADDR & 0xFFFF


        .set    STK_FRM_SIZ, -56
        .set    stacksize, 8192
        .set    stkmin, 56
	.set	HINT_SIZE, 48		# Size of AIXMON hints header
	.set	HID0, 1008		# HID0 for 603/604

        S_PROLOG(__start)
ENTRY(__start):
					# Begin Initialization
        bl      txt_strt                # Get current location
txt_strt:

#
########################################################################
#	System Initialization
########################################################################
#

	mfmsr	r20			# Shut off external interrupts
	xor	r20,r20,r20
	addi	r20,r20,0x1000		# set MC enable, DR off, IR off
	mtmsr	r20
	isync

	mfpvr	r30			# Get Processor Version
	rlwinm	r31,r30,16,0x0000FFFF	# Version is in high 16 bits
	cmpi	cr5,r31,1		# Test if 601 processor

# Invalidate the Data Cache and Invalidate the I Cache
# This will put both D and I cache in a known state before we do anything
# else.  Notice that after this we setup address translation for
# 603/604, so all code moves after that will be done cache-inhibited.  For
# 601 they are not done cache-inhibited, but it also has a combined I/D cache,
# so it doesn't matter.  Any code we move around after this, we are careful
# to flush the D cache and invalidate the appropriate targets in the I cache.

	beq	cr5, is_601

        # Otherwise 603/604

#!!!before invalidating, flush the data cache where the residual data is ...
	dcbf	0,r3			# flush first cache line 
	l	r31,0(r3)		# load length of resid data
	li	r30,0			# initialize index
flush_loop:
	addi	r30,r30,32		# increment index
	cmp	cr0,r30,r31		# check if more
	dcbf	r30,r3			# flush
	blt	flush_loop
#!!!

        mfspr   r7,HID0
        ori     r7,r7,0x0C00
        isync
        mtspr   HID0, r7
        isync
	rlinm	r7,r7,0,0xFFFFF3FF
        isync
        mtspr   HID0, r7
	sync
        isync

# Tear down ALL BATs
        li      r15,0
        mtdbatu 0,r15
        mtdbatu 1,r15
        mtdbatu 2,r15
        mtdbatu 3,r15
        mtibatu 0,r15
        mtibatu 1,r15
        mtibatu 2,r15
        mtibatu 3,r15
	isync
is_601:

	xor	r20,r20,r20			# Zero out all seg regs
	mtsr	sr0 ,r20
	mtsr	sr1 ,r20
	mtsr	sr2 ,r20
	mtsr	sr3 ,r20
	mtsr	sr4 ,r20
	mtsr	sr5 ,r20
	mtsr	sr6 ,r20
	mtsr	sr7 ,r20
	mtsr	sr8 ,r20
	mtsr	sr9 ,r20
	mtsr	sr10,r20
	mtsr	sr11,r20
	mtsr	sr12,r20
	mtsr	sr13,r20
	mtsr	sr14,r20
	mtsr	sr15,r20
	isync

#
########################################################################
#	Relocate the image to the TARGET ADDRESS
########################################################################
#
        mflr    r6			# Setup starting address
        addi    r6,r6,-4                # Adjust it back to zero base
	stw	r6,-28(r6)		# Save load origon in hints section

	stw     r3,-44(r6)		# Address of residual data
	stw	r4,-40(r6)		# Image load address
	stw	r5,-36(r6)		# Jump address

					# Setup for relocation
        lwz     r21,-16(r6)             # Get size in bytes of monitor

	mr	r20,r6			# Setup initial source address
	xor	r22,r22,r22		# Setup target address
        addi    r22,r22,TARGET_L        
        addis   r22,r22,TARGET_H       
	mr	r6,r22			# Save the Target Address
	addi    r20,r20,-HINT_SIZE	# Adjust target & source address
	addi	r22,r22,-HINT_SIZE	# to copy the hints with the image

xfer_img:
        lwz     r25,0(r20)              # Get source op-code
        stw     r25,0(r22)              # Write to target addr
	dcbf	0, r20			# flush what we pulled in
	dcbf	0, r22			# flush what we wrote
	sync
	icbi	0, r22			# invalidate target
        addi    r20,r20,4               # Bump source addr
        addi    r22,r22,4               # Bump target addr
        addic.  r21,r21,-4              # Decrement byte count
        bge     xfer_img                # Branch if not done

	sync
	isync

# Branch to newly-relocated code
        ba      relocat

relocat:

	beq	cr5, setup_601		# if 601, then go setup segment regs

setup_bats:				# else use bats for 603/604
#
########################################################################
#       Setup the BATS
########################################################################
#

        li      r15,0

        lis     r15, lu0.bat_rspc_seg
        lis     r20, uu0.bat_rspc_seg
        ori     r15, r15, ll0.bat_rspc_seg
        ori     r20, r20, ul0.bat_rspc_seg
        mtdbatl 0, r15
        mtdbatu 0, r20

        lis     r15, lu1.bat_rspc_seg
        lis     r20, uu1.bat_rspc_seg
        ori     r15, r15, ll1.bat_rspc_seg
        ori     r20, r20, ul1.bat_rspc_seg
        mtdbatl 1, r15
        mtdbatu 1, r20

        lis     r15, lu2.bat_rspc_seg
        lis     r20, uu2.bat_rspc_seg
        ori     r15, r15, ll2.bat_rspc_seg
        ori     r20, r20, ul2.bat_rspc_seg
        mtdbatl 2, r15
        mtdbatu 2, r20

        lis     r15, lu3.bat_rspc_seg
        lis     r20, uu3.bat_rspc_seg
        ori     r15, r15, ll3.bat_rspc_seg
        ori     r20, r20, ul3.bat_rspc_seg
        mtdbatl 3, r15
        mtdbatu 3, r20

        mfmsr   r20 
        ori     r20,r20,0x1010		# Turn on DR
        mtmsr   r20
        isync

	b	continue


setup_601:
#
########################################################################
#	Setup segment registers for I/O access
########################################################################
#
        cal     r5, 0x000C(0) 		# SEG REG 12 for adapter memory
        cau     r5, r5, 0x87F0
        mtsr    sr12, r5

        cal     r5, 0x000A(0)		# SEG REG 10 for PCI space
        cau     r5, r5, 0x87F0
        mtsr    sr10, r5

        cal     r5, 0x0008(0)		# SEG REG 8 for I/O 
        cau     r5, r5, 0x87F0
        mtsr    sr8, r5


continue:
        isync                                   # sync it up
#
########################################################################
#                      Initialize TOC register
########################################################################
#
        bl      here                    # Get current location
here:   mflr    r3                      # Put link reg. in R3
        .using  here,r3
        l       r2,DATA(ltoc)           # Init R2 with address of TOC
        .using  toc,r2

#
########################################################################
#      Initialize Stack Pointer and preallocate 1st Stack frame
########################################################################
#
        cal     r1, (STACK_VALUE & 0xffff)(0) # Set the initial stack ptr
        cau     r1, r1, (STACK_VALUE > 16) & 0xffff

        rlinm   r1,r1,0,~7              # double-word align
        la      r1,STK_FRM_SIZ(r1)      # Allocate caller stack frame
        xor     r9,r9,r9                # clear a register
        stw     r9,0(r1)                # zero out the stack's back chain ptr

#
########################################################################
#              Do General initialzation, then call main()
########################################################################
#
	mr	r3,r6			# Pass start Address to setup
	mr	r4,r31			# Also pass processor version 
        .extern ENTRY(setup[PR])
        bl      ENTRY(setup[PR])        # Does basic crt  initialization
        cror    r31,r31,r31             # and then calls main()

        ##############
        # NOTREACHED
        #############

#
########################################################################
#  Reserve a word for holding the TOC anchor
#  (THIS NEEDS TO BE IN .csect prog[PR] !!!)
########################################################################
#

ltoc:   .long   TOC_ORIGIN

        FCNDES(__start)

#
########################################################################
#  A couple of glue routines:
#      __exit()  Just spins for eternity. It should never be called.
#                But is here for gdb purposes so please leave it.
#      _ptrgl    This is xlc weirdness. Jumps through function pointers
#                all go through this routine.
########################################################################
#
        S_PROLOG(__exit)
        bl      $                       # Sit here forever

        ############
        # NOTREACHED
        ############

        S_EPILOG
        FCNDES(__exit)

        S_PROLOG(_ptrgl)
        lwz     r0, 0(r11)      # load addr of code of called function.
        stw     r2, 20(r1)      # save current TOC address.
        mtctr   r0              # move branch target to Count register.
        lwz     r2, 4(r11)      # set new TOC address.
        bctr                    # go to callee.

        #############
        # NOTREACHED
        #############

        S_EPILOG
        FCNDES(_ptrgl)

#
########################################################################
#       Define the symbols used as object position markers.
########################################################################
#

        # _etext = End of text area
DATA(_etext):   .csect  DATA(_etext[DB])
                .globl  DATA(_etext)
DATA(etext):    .globl  DATA( etext)

        #  _data = Beginning of data.
DATA(_data):    .csect  DATA(_data[RW])
                .globl  DATA(_data)
DATA(data):     .globl  DATA( data)

        #  _adata = Beginning of data.
DATA(_adata):   .csect  DATA(_adata[RW])
                .globl  DATA(_adata)
DATA(adata):    .globl  DATA( adata)

        # _edata = End of data area
DATA(_edata):   .csect  DATA(_edata[UA])
                .globl  DATA(_edata)
DATA(edata):    .globl  DATA( edata)

        # _end = End of BSS (common area).
                .comm   DATA( end), 0
                .comm   DATA(_end), 0

toc:            .toc
TC_etext:       .tc     _etext[TC], DATA(_etext[DB])
TC_adata:       .tc     _adata[TC], DATA(_adata[RW])
TC_edata:       .tc     _edata[TC], DATA(_edata[UA])
.etext:         .tc     _etext[TC], DATA(etext)
.edata:         .tc     _edata[TC], DATA(edata)
