# @(#)14	1.3  src/rspc/usr/lib/boot/softros/lib/libstand/asmutil.s, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:22
#
#   COMPONENT_NAME: RSPC_SOFTROS
#
#   FUNCTIONS: S_PROLOG
#		lower
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

                .file   "asmutil.s"
                .machine "ppc"

#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: outbyte()
#
# FUNCTION:    Outputs a byte to the specified memory-mapped I/O address.
#              Definition: outbyte( uchar_t *address, uchar byte );
#
# EXECUTION ENVIRONMENT:
#
#
# RETURNS:     Nothing
#
# NOTES:	Assumes I/O mapping is already done (start.s)
#
#-----------------------------------------------------------------------------


                S_PROLOG( outbyte)

                eieio                   # Enforce in-order execution
                stb     r4, 0(r3)       # Output byte.
                eieio                   # Enforce in-order execution

                S_EPILOG

#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: outword()
#
# FUNCTION:    Outputs a word to the specified memory-mapped I/O address.
#              Definition: outword( uint_t *address, uint word );
#
# EXECUTION ENVIRONMENT:
#
# RETURNS:     Nothing
#
# NOTES:	Assumes I/O mapping is already done (start.s)
#-----------------------------------------------------------------------------

                S_PROLOG( outword)

                eieio                   # Enforce in-order execution
                stw     r4, 0(r3)       # Output word.
                eieio                   # Enforce in-order execution

                S_EPILOG

#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: outword_r()
#
# FUNCTION:    Outputs a 32-bit word to the specified memory-mapped I/O
#               address in little endian format.
#              Definition: outword_r( uint_t *address, uint word );
#
# EXECUTION ENVIRONMENT:
#
# RETURNS:     Nothing
#
# NOTES:	Assumes I/O mapping is already done (start.s)
#-----------------------------------------------------------------------------

                S_PROLOG( outword_r)

                eieio                   # Enforce in-order execution
                stwbrx  r4, 0, r3       # Output word byte reversed.
                eieio                   # Enforce in-order execution

                S_EPILOG

#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: inbyte()
#
# FUNCTION:    Reads in a byte from the specified memory-mapped I/O address.
#              Definition: (uchar_t)inbyte( uchar_t *address );
#
# EXECUTION ENVIRONMENT:
#
# RETURNS:     Byte read from address.
#
# NOTES:	Assumes I/O mapping is already done (start.s)
#
#-----------------------------------------------------------------------------

                S_PROLOG( inbyte)

                eieio                   # Enforce in-order execution
                lbz     r6, 0(r3)       # Input char.
                eieio                   # Enforce in-order execution

                mr      r3, r6          # return value
                S_EPILOG



#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: inword()
#
# FUNCTION:    Reads in a word from the specified memory-mapped I/O address.
#              Definition: (uint_t)inword( uint_t *address );
#
# EXECUTION ENVIRONMENT:
#
# RETURNS:     Word read from address.
#
# NOTES:	Assumes I/O mapping is already done (start.s)
#-----------------------------------------------------------------------------

                S_PROLOG( inword)

                eieio                   # Enforce in-order execution
                l       r6, 0(r3)       # Input word.
                eieio                   # Enforce in-order execution

                mr      r3, r6          # return value
                S_EPILOG


#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: inword_r()
#
# FUNCTION:    Reads in a 32-bitword from the specified memory-mapped I/O
#               address in little endian format.
#              Definition: (uint_t)inword_r( uint_t *address );
#
# EXECUTION ENVIRONMENT:
#
# RETURNS:     32-bit Word read from address.
#
# NOTES:	Assumes I/O mapping is already done (start.s)
#-----------------------------------------------------------------------------

                S_PROLOG( inword_r)

                eieio                   # Enforce in-order execution
                lwbrx   r6, 0, r3       # Input word, byte reversed.
                eieio                   # Enforce in-order execution

                mr      r3, r6          # return value
                S_EPILOG


#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: outbmem()
#
# FUNCTION:    Outputs a byte to the specified bus memory address.
#              Definition: outbmem( uchar_t *address, uchar byte );
#
# EXECUTION ENVIRONMENT:
#
#              Completely standalone, running on 601 in real mode.
#              All registers used are saved and restored upon exit.
#
# RETURNS:     Nothing
#
# NOTES:         This routine uses a special direct store segment to
#              access bus memory addresses.  The current segment value is
#              saved and restored on exit.
#                This code also assumes that segment register 12 has been
#              initialized to 0x87F0000C somewhere else.
#
#-----------------------------------------------------------------------------

                S_PROLOG(outbmem)

                oris    r3, r3, 0xC000  # Make sure we select through seg 12

                eieio                   # Enforce in-order execution
                stb     r4, 0(r3)       # Output byte.
                eieio                   # Enforce in-order execution

                S_EPILOG

#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: inbmem()
#
# FUNCTION:    Reads in a byte from the specified bus memory address.
#              Definition: (uchar_t)inbmem( uchar_t *address );
#
# EXECUTION ENVIRONMENT:
#
#              Completely standalone, running on 601 in real mode.
#              All registers used are saved and restored upon exit.
#
# RETURNS:     Byte read from address.
#
# NOTES:         This routine uses a special direct store segment to
#              access I/O addresses.  The current segment value is
#              saved and restored on exit.
#                This code also assumes that segment register 12 has been
#              initialized to 0x87F0000C somewhere else.
#
#-----------------------------------------------------------------------------

                S_PROLOG( inbmem)

                oris    r3, r3, 0xC000  # Make sure we select through seg 12

                eieio                   # Enforce in-order execution
                lbz     r6, 0(r3)       # Input char.
                eieio                   # Enforce in-order execution

                mr      r3, r6          # return value
                S_EPILOG

#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: soft_reboot()
#
# FUNCTION:    Reboots the system by jumping to ROM startup.
#
# EXECUTION ENVIRONMENT:
#
#              Completely standalone, running on 601 in real mode.
#
# RETURNS:     Never.
#
# NOTES:
#
#       This routine emulates the effects of a 601 Soft Reset (assert
#       SRESET_) as described in the PowerPC Implementation Definition
#       for the 601 Processor, Book IV, section D.2 "Soft Reset".
#       The following sequence is performed:
#
#               1) Save the address of the next instruction to be executed
#                  in SRR0.
#               2) Set MSR to all zeroes except interrupt prefix bit = 1.
#               3) Save the current value of MSR in SRR1.
#               4) Begin execution at 0xFFF00100 (ROM) for IP = 1.
#
#-----------------------------------------------------------------------------

                .set    ROM_RESET,      0xFFF00100
                .set    ROM_RESET_H,    ( ROM_RESET > 16 )
                .set    ROM_RESET_L,    ( ROM_RESET & 0xFFFF )
                .set    MSR_VALUE,      0x40
		.set	HID0,		1008

                .csect  .soft_reboot[PR]
                .globl  .soft_reboot
.soft_reboot:
                xor     r2, r2, r2              # zero r2
                addis   r1, r2, ROM_RESET_H     # load hi halfword
                ori     r1, r1, ROM_RESET_L     # or in lo halfword
                mtctr   r1                      # move target to Count register.
                mtsrr0  r1                      # copy to SRR0
                ori     r2, r2, MSR_VALUE       # set interrupt prefix
                mtmsr   r2                      # copy to MSR
                mtsrr1  r2                      # copy to SRR1
                bctr                            # jump to ROM

#
# prep_hid0
#
		S_PROLOG(prep_hid0)
		mfspr	r7,HID0
 		rlinm	r7,r7,0,0xFFFF3FFF		# disable cache
 		isync
 		mtspr	HID0, r7
		sync
		isync
		S_EPILOG

#
# read_hid0
#
		S_PROLOG(read_hid0)
		mfspr	r3,HID0
		S_EPILOG

#
# read_msr - Return the contents of the MSR register
# unsigned int read_msr()
#
                S_PROLOG(read_msr)
                mfmsr   r3
                S_EPILOG

#
# set_msr - Set the MSR register
# void set_msr(unisgned int)
#
                S_PROLOG(set_msr)
                mtmsr   r3
                S_EPILOG

#
# read_segreg - Return the contents of a segment register
# unsigned int read_segreg(int)
#
                S_PROLOG(read_segreg)
                mfsrin  r3, r3
                S_EPILOG

#
# set_segreg - Modify the value of a segment register
# void set_segreg(int, unsigned int)
#
                S_PROLOG(set_segreg)
                mtsrin  r4, r3
                S_EPILOG

#
# read_pvr - Return the contents of the processor version register
# unsigned read_pvr()
#
                .set PVR, 287
                S_PROLOG(read_pvr)
                mfspr   r3, PVR
                S_EPILOG

#
# flush_dcache - Flush the data cache line containing the address
# void flush_dcache(unsigned int)
#
                S_PROLOG(flush_dcache)
                dcbf    r0, r3
                sync
                S_EPILOG

#
# dojumpi - This routine will relocate a small piece of code to 512 bytes
#           below the IPL control block, call that code, have the called
#           code relocate a kernel image from an address passed to 0, and
#           call the kernel.
# void dojumpi(array, kernaddr, kernlen, kernjump)
#
#   array[0] - iplcb        r25 Save reg
#   array[1] - r4 of call   r26 Save reg
#   array[2] - r5 of call   r27 Save reg
#   array[3] - r6 of call   r28 Save reg
#
#   NOTE: This routine never returns
#
        .set    STARTADDR,-512-4        # Offset from iplcb of our relocater
        .set    MOVETO,0                # Address to move kernel to

        S_PROLOG(dojumpi)
        addi    r20, r3, 0              # Save original R3 (Points to params)
        lwz     r25, 0(r20)             # Setup registers for virgin ROS call
        lwz     r26, 4(r20)
        lwz     r27, 8(r20)
        lwz     r28,12(r20)
        lwz     r3, 0(r3)               # R3 points to IPLCB
        bcl     20, 31, $+4             # Taken from 601 book, addr next instr
dj1:
        # Compute relocation addresses for the relocater routine
        mflr    r13
        addi    r14, r3, STARTADDR      # r14 contains address to relocate to
        addi    r21, r14, 4             #     Remember this is the start addr
        addi    r13, r13, djreloc-dj1-4 # r13 contains addr to relocate from
        addi    r15, 0, djdone-djreloc  # r15 contains the number of bytes

        # Relocate the relocater
dj2:    lwzu    r3, 4(r13)              # Read from
        stwu    r3, 4(r14)              # Move to
	dcbf	r0, r13			# flush what we pulled in
	dcbf	r0, r14			# flush what we wrote
	sync
	icbi	r0, r14			# invalidate target
        addic.  r15, r15, -4            # Count this instruction
        bne     dj2                     # Keep going until done

        sync
        isync

        # Jump to the start of the kernel relocater
        mtctr   r21
        bctr

# Start of routine to relocate.  It will relocate the kernel.
#   r4  - Pointer to image to move
#   r5  - Size of image
#   r6  - Offset into image to jump too
#   r20 - Points to array of parameters to pass to kernel (4 elements)
djreloc:                                # Start of routine to relocate

        # Now move the kernel

	mr	r9, r5
        addis   r7, 0, (MOVETO > 16)    # Get upper half word
        addi    r7, 0, (MOVETO & 0xffff)  # and lower (cau,cal was easier)
        add     r6, r6, r7              # Adjust branch address to offset
        addi    r7, r7, -4              # Convert to word back to get 1st word
        addi    r4, r4, -4              # ditto from
djc1:   lwzu    r8, 4(r4)               # get a word of kernel image
        stwu    r8, 4(r7)               # put it in low memory
	dcbf	r0, r4			# flush what we read
	dcbf	r0, r7			# flush what we wrote
	sync
	icbi	r0, r7			# invalidate target
	addic.  r9, r9, -4
        bne     djc1                    # loop until done
        sync
        isync

        mtctr   r6
        addi    r3, r25, 0
        addi    r4, r26, 0
        addi    r5, r27, 0
        addi    r6, r28, 0
        xor     r7, r7, r7              # Build an MSR value
        addi    r9, 0, 0x1040           # MC enable, interrupt to ROM, real
        mtmsr   r9                      # Stick it in there
        sync
        isync

       	mfpvr   r30                     # Get Processor Version
        rlwinm  r31,r30,16,0x0000FFFF   # Version is in high 16 bits
	cmpi    cr5,r31,1               # Test if 601 processor
	beq     cr5, teardown_601

# Tear Down 603/604 Data BATS

	mtdbatu 0,r7
	mtdbatu 1,r7
	mtdbatu 2,r7
	mtdbatu 3,r7

	b	done	

teardown_601:
# For some reason ROS calls us with BAT translation.  AIX doesn't like this
        mtibatl 0, r7                   # Batstuff
        sync
        isync
        mtibatu 0, r7
        mtibatl 1, r7
        sync
        isync
        mtibatu 1, r7
        mtibatl 2, r7
        sync
        isync
        mtibatu 2, r7
        mtibatl 3, r7
        sync
        isync
        mtibatu 3, r7

done:

	sync
	isync

        bctr                            # Whieeeeeee
djdone:
        S_EPILOG

#-----------------------------------------------------------------------------
#
# FUNCTION_NAME: ppln_drain()
#
# FUNCTION:    Issue a eieio to drain the execution pipe.
#              Definition: ppln_drain();
#
# EXECUTION ENVIRONMENT:
#
#              Completely standalone, running on 601 in real mode.
#              All registers used are saved and restored upon exit.
#
# RETURNS:     Nothing
#
#-----------------------------------------------------------------------------

                S_PROLOG(ppln_drain)

                eieio                   # Enforce in-order execution

                S_EPILOG

#
# simple_jump - Does a simple absolute jump to address in r4
# int simple_jump(unsigned int addr)
#
                S_PROLOG(simple_jump)

       		mfpvr   r30                     # Get Processor Version
        	rlwinm  r31,r30,16,0x0000FFFF   # Version is in high 16 bits
		cmpi    cr5,r31,1               # Test if 601 processor
		beq     cr5, is_601

        	# Otherwise 603/604
		# Invalidate the Instruction Cache
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
	is_601:

       		mtctr   r4
        	bctr
                S_EPILOG

