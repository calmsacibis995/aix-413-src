# @(#)30        1.3  src/rspc/kernext/pm/pmmi/pmdump/copyreal.s, pwrsysx, rspc41J, 9514A_all 4/3/95 21:37:16
#
# COMPONENT_NAME: PWRSYSX
#
# FUNCTIONS: Power Management Kernel Extension Code
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#  
	.set CTR,		9
	.set BO_IF_NOT,		4
	.set CR0_GT,		1
	.set BO_dCTR_NZERO,	16
	.set CR0_LT,		0
	.set BO_ALWAYS,		20

#**********************************************************************
#
#  NAME: copy_real
#
#  FUNCTION: copy integers from real address to real address
#
#       copy_real(int *target, int *source, int length);
#
#  INPUT STATE:
#     r3 = target physical address
#     r4 = source physical address
#     r5 = length
#
#  NOTE:
#    The length is the number of words (not bytes) to move, and there is
#    no effort made to align the source or destination on word boundaries.
#
#  RETURNED VALUE:
#     none
#
#  EXECUTION ENVIRONMENT:
#       any
#
#**********************************************************************
	S_PROLOG(copy_real)

	mfmsr	r6
	sync
	rlwinm	r7, r6, 0, 0xffffffef		# data translation off
	rlwinm	r7, r7, 0, 0xffff0fff		# exceptions off
	mtmsr	r7
	isync

	cmpi	0, 0, r5, 0
	addi	r3, r3, -4
	addi	r4, r4, -4
	mtspr	CTR, r5
	bc	BO_IF_NOT, CR0_GT, LOOP_END
FOR_LOOP:					# copy data
	lwzu	r5, 4(r4)
	stwu	r5, 4(r3)
	bc	BO_dCTR_NZERO, CR0_LT, FOR_LOOP

LOOP_END:
	sync
	mtmsr	r6				# data translation on
	isync

	S_EPILOG(copy_real)
