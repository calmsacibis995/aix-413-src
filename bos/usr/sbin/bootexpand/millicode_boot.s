# @(#)36	1.1  src/bos/usr/sbin/bootexpand/millicode_boot.s, bosboot, bos411, 9428A410j 3/8/94 18:27:47
#
# COMPONENT_NAME: BOSBOOT
#
# FUNCTIONS: generic integer functions (Power/Power PC implementation)
#	MILLICODE FUNCTIONS:
#	     __mulh
#	     __mull
#	     __divss
#	     __divus
#	     __quoss
#	     __quous
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#          *** NOTE ***
#
# When using these routines, the compiler saves/restores only
# those registers which will actually be used; it has knowledge
# of the internal semantics.  Do NOT change the code to use
# any additional registers without agreement from the compiler
# architects!
#****************************************************************************


# Special purpose version of millicode only for bootexpand.  This
# relies on having a global int 'system_architecture' avaliable which
# has the same information as _system_configuration.architecture, i.e.
# POWER_RS == 0x1 and POWER_PC == 0x2.

	.set powerrs,1
	.file "millicode_boot.s"
	.machine "any"

	.toc
	TOCE(system_architecture, data)

#-------------------------------------------------------------------
# Subroutine Name: __mulh
#
# Function: calculate high-order 32 bits of the signed integer
#           product arg1 * arg2
#
# Input:
#       r3 = arg1 (signed int)
#	r4 = arg2 (signed int)
# Output:
#	r3 = high-order 32 bits of r3 * r4
# General Purpose Registers Set/Used:
#	r3, r4
# Special Purpose Registers Set/Used:
#	mq (ON POWER)
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

	S_PROLOG(__mulh)
	st	r31, -4(r1)	#save r31
	LTOC(r31, system_architecture, data)
	l	r31, 0(r31)	# load global variable
	cmpli	cr0, r31, powerrs
	l	r31, -4(r1)	# prestore r31
	bne	mulh_ppc

	mul	r3,r3,r4		# r3 = r3 * r4
	br

mulh_ppc:
	mulhw	r3,r3,r4		# r3 = (high 32 bits) r3 * r4
	S_EPILOG
	FCNDES(__mulh)

#-------------------------------------------------------------------
# Subroutine Name: __mull
#
# Function: calculate 64 bits of signed product arg1 * arg2,
#	    (returned in two 32-bit registers)
#
#
# Input:
#       r3 = arg1 (signed int)
#	r4 = arg2 (signed int)
# Output:
#	r3 = high-order 32 bits of r3 * r4
#	r4 = low-order 32 bits of r3 * r4
# General Purpose Registers Set/Used:
#	r3, r4
# Special Purpose Registers Set/Used:
#	mq (on POWER)
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

	S_PROLOG(__mull)
	st	r31, -4(r1)	#save r31
	LTOC(r31, system_architecture, data)
	l	r31, 0(r31)	# load global variable
	cmpli	cr0, r31, powerrs
	l	r31, -4(r1)	# prestore r31
	bne	mull_ppc

	mul	r3,r3,r4		# r3 = r3 * r4
	mfspr	r4,mq			# get low order part
	br

mull_ppc:
	mullw	r0,r3,r4		# low part of result
	mulhw	r3,r3,r4		# high part of result
	ori	r4,r0,0x0		# move low part to correct reg.
	S_EPILOG
	FCNDES(__mull)

#-------------------------------------------------------------------
# Subroutine Name: __divss (Power Version)
#
# Function: calculate 32-bit quotient and 32-bit remainder
#	    of arg1/arg2, where arg1 and arg2 are 32-bit signed ints
#
#
# Input:
#       r3 = arg1 - dividend (signed int)
#	r4 = arg2 - divisor  (signed int)
# Output:
#	r3 = quotient of arg1/arg2  (signed int)
#	r4 = remainder of arg1/arg2 (signed int)
#
#	For division by zero and overflow (i.e. -2^32 / -1)
#	the quotient and remainder are undefined and may
#	vary by platform.
#
# General Purpose Registers Set/Used:
#	r3, r4
# Special Purpose Registers Set/Used:
#	mq (ON POWER)
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

	S_PROLOG(__divss)
	st	r31, -4(r1)	#save r31
	LTOC(r31, system_architecture, data)
	l	r31, 0(r31)	# load global variable
	cmpli	cr0, r31, powerrs
	l	r31, -4(r1)	# prestore r31
	bne	divss_ppc

	divs	r3,r3,r4		# quotient to r3
	mfspr	r4,mq			# get remainder to r4
	br

divss_ppc:
	divw	r0,r3,r4		# r0 = r3/r4
	mullw	r4,r0,r4		# r4 = (r3/r4) * r4
	subf	r4,r4,r3		# remainder: r4 = r3 - ((r3/r4) * r4)
	ori	r3,r0,0x0		# quotient to correct reg.
	S_EPILOG
	FCNDES(__divss)

#-------------------------------------------------------------------
# Subroutine Name: __divus (Power Version)
#
# Function: calculate unsigned 32-bit quotient and 32-bit remainder
#	    of arg1/arg2, where arg1 and arg2 are unsigned ints
#
#
# Input:
#       r3 = arg1 - dividend (unsigned int)
#	r4 = arg2 - divisor  (unsigned int)
# Output:
#	r3 = quotient of arg1/arg2  (unsigned int)
#	r4 = remainder of arg1/arg2 (unsigned int)
#
#	For division by zero the quotient and remainder
#	are undefined and may vary by platform.
#
# General Purpose Registers Set/Used:
#	r0, r3, r4
# Special Purpose Registers Set/Used:
#	mq
# Condition Register Fields Set/Used:
#	cr0, cr1
#-------------------------------------------------------------------

	S_PROLOG(__divus)
	st	r31, -4(r1)	#save r31
	LTOC(r31, system_architecture, data)
	l	r31, 0(r31)	# load global variable
	cmpli	cr0, r31, powerrs
	l	r31, -4(r1)	# prestore r31
	bne	divus_ppc

	cmpl	cr0,r4,r3	# r4 > r3 (unsigned)?
	cmpi	cr1,r4,0x1	# r4 compared to 1
	mtspr	mq,r3		# for div instructions
	cal	r0,0(0)		# move zero to r0
	bgt	cr0,DIVUS.3	# for arg2 > arg1 (unsigned)
	blt	cr1,DIVUS.2	# for case arg2 < 1 (**signed**)
	beq	cr0,DIVUS.2	# for case arg1 == arg2
	beq	cr1,DIVUS.1	# for case of arg1 = 1
	div	r3,r0,r4	# (0 || mq) / r4
	mfspr	r4,mq		# get remainder
	br			# return
DIVUS.1:			# here if arg1 == 1
	cal	r4,0(0)		# move 0 to remainder
	br			# return
DIVUS.2:			# here if arg1 == arg2 or
				# (arg2 > 0x7FFFFFFF) && (arg2 < arg2)
	sf	r4,r4,r3	# r4 = r3 - r4 (quotient)
	cal	r3,1(0)		# quotient is 1
	br			# return
DIVUS.3:			# here if arg2 > arg2
	oril	r4,r3,0x0	# move r3 to r4 (remainder)
	cal	r3,0(0)		# move 0 to quotient
	br

divus_ppc:
	divwu	r0,r3,r4		# r0 = r3/r4 (unsigned)
	mullw	r4,r0,r4		# r4 = (r3/r4) * r4
	subf	r4,r4,r3		# remainder
	ori	r3,r0,0x0		# quotient to correct register
	S_EPILOG
	FCNDES(__divus)

#-------------------------------------------------------------------
# Subroutine Name: __quoss (Power Version)
#
# Function: calculate the 32-bit quotient of arg1/arg2, signed
#
# Input:
#       r3 = arg1 - dividend (signed int)
#	r4 = arg2 - divisor  (signed int)
#
#	For division by zero and overflow (i.e. -2^32 / -1)
#	the quotient and remainder are undefined and may
#	vary by platform.
#
# Output:
#	r3 = quotient of arg1/arg2 (signed int)
# General Purpose Registers Set/Used:
#	r3,r4
# Special Purpose Registers Set/Used:
#	mq (ON POWER)
# Condition Register Fields Set/Used:
#	None.
#-------------------------------------------------------------------

	S_PROLOG(__quoss)
	st	r31, -4(r1)	#save r31
	LTOC(r31, system_architecture, data)
	l	r31, 0(r31)	# load global variable
	cmpli	cr0, r31, powerrs
	l	r31, -4(r1)	# prestore r31
	bne	quoss_ppc

	divs	r3,r3,r4		# quotient to r3
	br

quoss_ppc:
	divw	r3,r3,r4
	S_EPILOG
	FCNDES(__quoss)

#-------------------------------------------------------------------
# Subroutine Name: __quous (Power Version)
#
# Function: calculate the 32-bit quotient of arg1/arg2, unsigned
#
# Input:
#       r3 = arg1 - dividend (unsigned int)
#	r4 = arg2 - divisor  (unsigned int)
# Output:
#	r3 = quotient of arg1/arg2 (unsigned int)
#
#	For division by zero the quotient and remainder
#	are undefined and may vary by platform.
#
# General Purpose Registers Set/Used:
#	r0, r3, r4
# Special Purpose Registers Set/Used:
#	mq
# Condition Register Fields Set/Used:
#	cr0, cr1
#-------------------------------------------------------------------
	S_PROLOG(__quous)
	st	r31, -4(r1)	#save r31
	LTOC(r31, system_architecture, data)
	l	r31, 0(r31)	# load global variable
	cmpli	cr0, r31, powerrs
	l	r31, -4(r1)	# prestore r31
	bne	quous_ppc

	cmpl	cr0,r4,r3	# r4 > r3 (unsigned)?
	cmpi	cr1,r4,0x01	# r4 == 1?
	mtspr	mq,r3		# for div instruction
	cal	r0,0(0)		# r0 to contain 0x0
	bgt	cr0,QUOUS.3	# for arg2 > arg1 (unsigned)
	blt	cr1,QUOUS.2	# for arg2 < 1 (**signed**)
	beq	cr0,QUOUS.2	# for case arg1 == arg2
	beq	cr1,QUOUS.1	# for case of arg1 == 1
	div	r3,r0,r4	# (0 || mq) / r4
				# falls thru to br...
QUOUS.1:			# branch here if arg2 == 1; result is arg1,
	br			# which is already in r3
QUOUS.2:			# here if arg1 == arg2 or
				# (arg2 > 0x7FFFFFFF) && (arg2 < arg2)
	cal	r3,1(0)		# quotient is 1
	br			# return
QUOUS.3:			# branch here if r4 > r3 (unsigned)
	cal	r3,0(0)		# zero is result
	br

quous_ppc:
	divwu	r3,r3,r4
	S_EPILOG
	FCNDES(__quous)
