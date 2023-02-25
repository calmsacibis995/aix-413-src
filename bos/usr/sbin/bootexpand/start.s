# @(#)99	1.7  src/bos/usr/sbin/bootexpand/start.s, bosboot, bos411, 9428A410j 5/26/94 14:01:15
#
# COMPONENT_NAME: BOSBOOT
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file	"start.s"

#-----------------------------------------------------------------------#
#
# name: start
#
# function: This is an assembler "glue" routine which links to the
#           system initialization routine, main(). A stack is set
#           up in gpr 1, and control is transferred to main(),
#           which never returns.
#
# input state:
#	r3  =   pointer to ipl control block
#	if r3 = -1 (indicating a ROS Emulation boot) :
#		r4 = pointer to ipl control block
#		r5 = pointer to auxiliary ipl control block
#
#
# output state:
#       r1  =   address of temporary stack
#       r2  =   address of kernel TOC
#       r3  =   pointer to ipl control block
#	if r3 = -1 (indicating a ROS Emulation boot) :
#		r4 = pointer to ipl control block
#		r5 = pointer to auxiliary ipl control block
#
#-----------------------------------------------------------------------#

        .set    stacksize, 8192		# startup stack area size
#
        S_PROLOG(start1)
        cal     r2,MSR_ME|MSR_AL(0)     # set msr = disabled, xlate off
                                        #   (machine check and alignment
                                        #    enabled only)
        mtmsr   r2
	isync				# make sure mtmsr is noticed
	
	cal	r7,0(0)			# clear r7
	cmpi	cr0,r3,-1
	bne	cr0,start10
#					If r3 = -1 :
	cal	r3,0(r4)		# r3 = ipl control block pointer
	cal	r7,0(r5)		# r7 = auxiliary ipl control block ptr
start10:

#       set up toc

        bl      label
label:
        mfspr   r6,LR                   # get link register
        .using  label,r6                # get addressibility to vcon
        l       r2,ltoc                 # address vcon
#
#       set up stack
#
        .using  toc, r2
        l       r1,.mainstk		# get startup stack pointer in r1
        cal     r1,stacksize-stkmin(r1)
#
#       set up input to expandker
#
        l       r4,.ipl_cb		# address of IPL CB pointer
        st      r3,0(r4)		# save IPL control block address
	cal	r6,ipldir(r3)		# point to ipl directory
	l	r4,rammapo(r6)		# get offset to ram bit map
	a	r4,r3,r4		# point to ram bit map
	l	r5,iplinfoo(r6)		# get offset to ipl info
	a	r5,r3,r5		# point to ipl info
	l	r5,bytesperbit(r5)	# get bytes per bit
	l	r6,mapsz(r6)		# get size of map
	rlinm	r6,r6,30,0,31		# convert to words (shift right 2)
#
#       Transfer to main
#		parameters:
#		r3 = ipl control block pointer
#		r4 = pointer to ram bit map
#		r5 = bytes per bit
#		r6 = size of map
#		r7 = auxiliary ipl control block pointer (or zero if none)
#
        .extern ENTRY(main[PR])
        ba      ENTRY(main[PR])

ltoc:   .long   TOC_ORIGIN              # address of toc (see asdef.s for TOC_ORIGIN)


        .csect  mainstk[RW]
        .space  stacksize
        .extern DATA(ipl_cb[RW])

toc:            .toc
.mainstk:       .tc     .mainstk[TC], mainstk[RW]
.ipl_cb:        .tc     .ipl_cb[TC], DATA(ipl_cb[RW])

#-----------------------------------------------------------------------#
#	include files
#-----------------------------------------------------------------------#

include(scrs.m4)
include(iplcb.m4)
include(seg.m4)
include(machine.m4)
include(sys/comlink.m4)



