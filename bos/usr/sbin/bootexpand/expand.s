# @(#)70	1.10  src/bos/usr/sbin/bootexpand/expand.s, bosboot, bos412, 9445C412a 11/9/94 15:44:16
#
# COMPONENT_NAME: (BOSBOOT) Kernel Assembler Code
#
# FUNCTIONS: start of bootexpand code
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

################################################################################
##                                                                            ##
## NAME: start                                                                ##
##                                                                            ##
## FUNCTION: Entry point from ROS IPL code                                    ##
##                                                                            ##
## This routine gains control via a branch from the ROS IPL		      ##
## code, which passes in GPR 3 a pointer to the IPL Control		      ##
## block. If GPR3 = -1, then this is a ROS Emulation boot and                 ##
##    GPR4 contains the IPL Control block pointer and GPR5                    ##
##    contains the Auxiliary IPL Control block pointer.			      ##
##    GPR3, GPR4, and GPR5 are preserved and passed on to                     ##
##    the "start1" routine.                                                   ##
## This program MUST appear first in the kernel, because its		      ##
## job is to copy the memory image of the kernel into		  	      ##
## its proper place starting at address zero, overlaying the		      ##
## module header which preceeds the actual executable code.		      ##
## This header is present because the kernel is just an ordinary	      ##
## load module built by the ordinary AIX tools; when the ROS IPL	      ##
## code reads it from disk or diskette into address 0, the header	      ##
## is at address zero and the code (which logically belongs		      ##
## at 0) follows it.  The header is copied into a kernel data		      ##
## structure, and the entire kernel image is shifted down to		      ##
## start at 0.  This shift is accomplished by the code at		      ##
## label "start", below.						      ##
##                                                                            ##
## This job is not exactly a trivial					      ##
## task given that the executing routine must copy itself		      ##
## as well and make sure that the instruction cache and data		      ##
## don't get in each other's way while the self-modifying		      ##
## code is running.							      ##
##                                                                            ##
## Here's the plan of attack:						      ##
## 1) Branch-and-link to discover the actual address at			      ##
##    which "start" begins executing.  This will NOT be the		      ##
##    address shown in the assembly listing or load map,		      ##
##    because the code is offset by the length of the header.		      ##
## 2) Copy the module header into a data structure further		      ##
##    on in the kernel.							      ##
## 3) Copy the FIRST PART of this routine down to location 0,		      ##
##    using a loop which is located in higher addresses and		      ##
##    thus doesn't get copied yet.					      ##
## 4) Flush the caches and branch to the newly-moved routine.		      ##
## 5) Copy the rest of the kernel to the proper place, and		      ##
##    flush the caches again.						      ##
## 6) Branch to "start1", which creates a C environment for		      ##
##    the rest of kernel initialization.				      ##
##									      ##
################################################################################

.machine "com"

include(iplcb.m4)

##page;
#####################################################################
##                                                                 ##
##    Low memory -- Beginning of the code that resides at 0        ##
##                                                                 ##
#####################################################################

        .csect  ENTRY(low[PR])     # this goes at address zero
low:

real0:


#       0 so if anyone uses this as a procedure descriptor we go to zero
#       0 is an invalid op on this machine!

         .long   0


#       These cells are set up by the tool that creates the IPL data

        .long   0xBADFCA11         # dummy TOC value in case used as proc descriptor
DATA(header_offset):
        .long   0                  # offset in IPL file to start of hdr
DATA(ram_disk_start):
        .long   0                  # pointer to RAM disk in IPL file
DATA(ram_disk_end):
        .long   0                  # pointer to RAM disk end + 1
DATA(dbg_avail):
        .long   0                  # debugger available flags
DATA(base_conf_start):
        .long   0                  # pointer to base conf area in IPL image
DATA(base_conf_end):
        .long   0                  # pointer to base conf end
DATA(base_conf_disk):
        .long   0                  # disk address of base config area
	.org	real0+0x28C		# this is a workaround for a ROS bug
					# that is out on the loose.  Defect
					# 146736 affects many rainbow3
					# systems that are on customer sites
##page;
DATA(ipl_cb):
start:
       .globl	start			# N.B. Do NOT use "ENTRY(start)" because
                                        #   the string "start" is the one that
                                        #   the kernel build utility will match.

        bl      start10                 # Find out where we are, and branch
                                        #   to move loop in SECOND part
                                        #   of this routine
start25:
                                        # N.B. this label used at start10 -
                                        #   do not move

#*******************************************************************#
#   Move latter part of kernel down; this loop gains control        #
#   AFTER it has been copied down to its final position.            #
#*******************************************************************#

start30:
        lu      r6,4(r2)                # Get next 4 bytes to move
        stu     r6,4(r1)                # Move to proper loction
        cmp     cr0,r2,r0               # Check if all data moved
        blt     start30

#       Now must insure the dcache is stored in memory

	andiu.	r6,r17,PPC_MODEL
	bne     cr0,start42		# PPC model

.machine "pwr"
        clcs    r8,0xe                  # Get minimum cache line size
.machine "com"
        cal     r7,imove(0)             # Start at end of move routine
        sf      r7,r8,r7                # Back up one cache line worth
start40:
.machine "pwr"
        clf     r7,r8                   # Flush line containing byte at
                                        #   r7+r8, update r7
.machine "com"
        cmp     cr0,r7,r0               # Check if done
        blt     start40
        dcs                             # Wait for memory to be updated
        ics                             # Also instruction cache
	b	start48

#	PPC model
#	Use one loop to flush dcache, another one to invalidate icache.
#	This is necessary because cache line sizes of these two caches may differ

start42:
	cal     r7,imove(0)
start43:				# begin flushing dcache
.machine "ppc"
	dcbf	0,r7
.machine "com"
	cmp	cr0,r7,r0
	a       r7,r7,r16
	blt	start43

	sync
	cal	r7,imove(0)
start46:				# begin invalidating icache
.machine "ppc"
	icbi	0,r7
.machine "com"
	cmp	cr0,r7,r0
	a       r7,r7,r15
	blt	start46
	sync
	isync

start48:

#       Branch to "start1"

       .extern  ENTRY(start1[PR])
        ba      ENTRY(start1[PR])

#*******************************************************************#
#   N.B. This first loop FOLLOWS the second so the first half of    #
#   the move never clobbers itself!                                 #
#*******************************************************************#
imove:
        .long   0			# defines location

start10:
        mflr	r13                     # LR contains address of start10
        ai      r13,r13,real0-start25   # r13 contains real address of low 0


#	Determine machine model.
#	If it is a PPC machine, determine instruction cache line and data cache
#	line sizes from iplcb.
#	r15: icache line size
#	r16: dcache line size
#	r17: machine model

	cal	r9,0(r3)
        cmpi    cr0,r3,-1		# r3 = -1 => ROS emulation boot
        bne     cr0,start11		# If not ROS emul, r9 has iplcb ptr
	cal	r9,0(r4)		# Else (ROS emul) get iplcb ptr from r4
start11:

	l	r8,IPLINFO(r9)		# offset of ipl_info structure
	a	r8,r8,r9		# addr of ipl_info structure
	l	r17,MODEL(r8)		# model field in ipl_info structure
	andiu.	r8,r17,PPC_MODEL	# test if PPC model
	beq	cr0,start12		# not a PPC model
	l	r8,PROCINFO(r9)		# offset of proc_info structure
	a	r8,r8,r9		# addr of proc_info structure
	l	r15,ICACHEBK(r8)	# icache block size
	l	r16,DCACHEBK(r8)	# dcache block size

#       Compute how many bytes to move in order to overlay the header at 0

start12:
	l       r1,DATA(ram_disk_start)(r13)  # addr of start of RAM disk
        cal     r0,-4(r1)               # less 4 so we stop in time
        cal     r1,-4(0)                # put -4 in r1; first "stu" will go to 0
        ai      r2,r13,-4               # set from address -4 to r2
        cal     r14,imove-real0(0)      # compute number of bytes for
                                        #  first (short) move

#       First move this routine so that we can guarantee the integrity of cache

        andiu.	r8,r17,PPC_MODEL        # test if PPC model
	bne     cr0,start22             # PPC model

start20:
        lu      r6,4(r2)                # get next 4 bytes to move
        stu     r6,4(r1)                # move to proper location
.machine "pwr"
        clf     0,r1                    # we can afford to flush every
                                        #   store for short move
.machine "com"
        cmp     cr0,r1,r14              # check if all data moved
        blt     start20

#       If here, the size of the TOC/XCOFF header in bytes has been moved to
#       location 0.  The kernel remains unchanged above this.

        dcs                             # following example from Appendix C
                                        #   of architecture
        ics                             # make sure we get the new values
                                        #   just moved above
        ba      start25                 # this instruction will branch to
                                        #   the absolute location of start25
start22:
        lu      r6,4(r2)                # get next 4 bytes to move
        stu     r6,4(r1)                # move to proper location
.machine "ppc"
        dcbf    0,r1                    # flush dcache block
	sync
        icbi    0,r1                    # invalidate icache block
.machine "com"
        cmp     cr0,r1,r14              # check if all data moved
        blt     start22
        sync
        isync
        ba      start25

#
# provide storage for g_toc..its used by start1 but not needed otherwise.
#
	.globl	DATA(ipl_cb)
	.globl	DATA(g_toc)
DATA(g_toc):	.long 0

