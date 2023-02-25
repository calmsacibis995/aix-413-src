# @(#)89        1.1  src/bos/usr/sbin/bootexpand/cache_pwr.s, bosboot, bos411, 9431A411a 8/1/94 17:12:27
#
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: vm_cflush_pwr	 
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
		.file	"cache_pwr.s"
#****************************************************************************
#
# INCLUDED FUNCTIONS:
#	vm_cflush_pwr	      
#
# NOTE: This function was copied from the file
#        src/bos/kernel/ml/POWER/cache_pwr.s  to run with bootexpand.
#	 No modifications were made to the actual code.
#	 One comment was changed. It is marked with the string MOD.
#****************************************************************************

#       cache line size selector
        .set    MCLSZ,0xe       # minimum cache line size selector
                                # selected by "01110"=0xe


#****************************************************************************
#
#  NAME: vm_cflush_pwr
#
#  FUNCTION: flush I cache and D cache for a memory range
#
#       vm_cflush_pwr(eaddr, nbytes)
#
#  INPUT STATE:
#       r3 = starting effective address to sync
#       r4 = byte count
#
#  OUTPUT STATE:
#       Caches with data in the specified range are flushed
#
#  RETURN VALUE:
#	0 	if success
#	EINVAL  if nbytes < 0
#
#  EXECUTION ENVIRONMENT: 
#	Called directly from main in expndkro.c   MOD
#
#****************************************************************************

        .machine        "pwr"

	S_PROLOG(vm_cflush_pwr)

        clcs    r6,MCLSZ        # r6 = minimum cache line size,
        cmpi    cr0,r4,0        # see if nbytes is le 0

        ai      r7,r6,-1        # get mask of log2(clsize) length
                                # --this assumes cache line size is
                                #   a power of two.
	cntlz	r11,r6
        ble     cr0,spec_nbytes # branch if nbytes <= 0

#       num_cache_lines =
#       (offset_into_cache_line + nbytes + cache_line - 1) / cache_line_size

        and     r8,r7,r3        # mask off offset in cache line
	sfi	r12,r11,31	# r12 = log2(cache line size)

	# r10 = (offset + nbytes + cache line size - 1) / cache line size
        a       r8,r8,r4        # offset + nbytes
        sf      r5,r6,r3        # subtract line size to start loop
        a       r9,r8,r7        # + cache line size - 1 
	sr	r10,r9,r12	# / cache line size
        sri     r8,r10,2        # r8 = bigloop count
        andil.  r9,r10,0x3      # r9 = # lines to flush in power_remainloop
        cmpi    cr1,r8,0        # check if big loop count is 0
        beq     power_bigloop   # branch if nothing left for remainloop

        mtctr   r9              # setup loop count
power_remainloop:
        clf     r5,r6
        bdn     power_remainloop        # dec ctr and branch if non-zero

power_bigloop: 
        beq     cr1,power_sync

        mtctr   r8              # setup loop count

power_bigloop1:
        clf     r5,r6
        clf     r5,r6
        clf     r5,r6
        clf     r5,r6
        bdn     power_bigloop1  # dec ctr and branch if non-zero

power_sync:
	lil	r3,0x0		# success return code
        dcs                     # wait for memory updates to complete
        ics                     # discard prefretched instructions
	br

spec_nbytes:
	lil	r3,0x0		# success return code
	beqr	cr0		# return if nbytes = 0
	lil	r3,EINVAL	# error return code
	S_EPILOG

	FCNDES(vm_cflush_pwr)

        include(vmdefs.m4)
        include(errno.m4)
	include(machine.m4)


