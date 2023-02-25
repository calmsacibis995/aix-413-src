# @(#)88        1.1  src/bos/usr/sbin/bootexpand/cache_ppc.s, bosboot, bos411, 9431A411a 8/1/94 17:12:09
#
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: vm_cflush_ppc_splt
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
#***************************************************************************
		.file	"cache_ppc.s"
#***************************************************************************
#
# INCLUDED FUNCTIONS:
#	vm_cflush_ppc_splt
#
#  NOTE: This function was copied from the file
#          src/bos/kernel/ml/POWER/cache_ppc.s  and modified slightly to
#	   work with bootexpand. The lines that were modified are marked
#	   with the string MOD. The changes were to make the icache block size
#	   and dcache block size input parameters rather than getting them
#	   out of the system configuration area, which does not exist in
#	   the bootexpand environment.
#
#***************************************************************************


#***************************************************************************
#
#  NAME: vm_cflush_ppc_splt
#
#  FUNCTION: flush I cache and D cache for a memory range
#
#       vm_cflush_ppc_splt(eaddr, nbytes, icache_blksz, dcache_blksz)   MOD
#
#  INPUT STATE:
#  	r3 = starting effective address to flush
#     	r4 = byte count
#	r5 = icache block size   MOD (added this input parameter)
#	r6 = dcache block size   MOD (added this input parameter)
#
#  OUTPUT STATE:
#     	Caches with data in the specified range are flushed
#
#  RETURN VALUE:
#	0 	if success
#	EINVAL  if nbytes < 0
#
#  EXECUTION ENVIRONMENT: 
#	Called directly from main in expndkro.c   MOD
#
#***************************************************************************

        .machine        "ppc"

        S_PROLOG(vm_cflush_ppc_splt)

        cmpi    cr0,r4,0        # see if nbytes is le 0
        ble     cr0,spec1_nbytes # branch if nbytes <= 0
	cal	r12,0(r5)	# r12 = icache block size   MOD
        ai      r7,r6,-1        # get mask of log2(dcache block size) length
                                # --this assumes cache block size is
                                #   a power of two.
	cntlz	r11,r6
	
#       num_cache_blocks =
#        (offset_into_cache_block + nbytes + cache_block_size - 1) / 
#	 cache_block_size

        and     r8,r7,r3        # mask off offset in cache block
	sfi	r11,r11,31	# r11 = log2(dcache block size)
        a       r8,r8,r7        # offset + cache block size - 1
        sf      r5,r6,r3        # subtract block size to start loop
        a       r9,r8,r4        # + nbytes
        ai      r7,r12,-1       # get mask of log2(icache block size) length
                                # -- this assumes cache block size is
                                #    a power of two.
	sr	r10,r9,r11	# / cache block size
	cntlz	r11,r12
        sri     r8,r10,2        # r8 = bigloop count
        andil.  r9,r10,0x3      # r9 = # blocks to flush in d_remainloop
        cmpi    cr1,r8,0        # chech if big loop count is 0
        beq     ppc_d_bigloop   # if nothing left for remainloop

        mtctr   r9              # setup loop count
ppc_d_remainloop:
        dcbf    r5,r6		# flush a dcache block
	a	r5,r5,r6	# increment r4 to the next cache block
        bdn     ppc_d_remainloop  # dec ctr and branch if non-zero

ppc_d_bigloop: 
        and     r10,r7,r3       # mask off offset in cache block
	sfi	r11,r11,31	# r11 = log2(icache block size)
        a       r10,r10,r4      # offset + nbytes
        beq     cr1,ppc_d_sync

        mtctr   r8              # setup loop count
ppc_d_bigloop1:
        dcbf    r5,r6		# flush a dcache block
	a	r5,r5,r6	# increment r5 to the next cache block
        dcbf    r5,r6		
	a	r5,r5,r6	
        dcbf    r5,r6		
	a	r5,r5,r6	
        dcbf    r5,r6		
	a	r5,r5,r6	
        bdn     ppc_d_bigloop1  # dec ctr and branch if non-zero

ppc_d_sync:
        a       r9,r10,r7       # + cache block size - 1
        sync                    # wait for memory updates to complete

#       num_cache_blocks =
#       (offset_into_cache_block + nbytes + cache_block_size - 1) / 
#	cache_block_size

	sr	r8,r9,r11	# / cache block size
        sf      r5,r12,r3       # subtract block size to start loop
        sri     r10,r8,2        # r8 = bigloop count
        andil.  r9,r8,0x3       # r9 = # blocks to flush in i_remainloop
        cmpi    cr1,r10,0       # chech if big loop count is 0
        beq     ppc_i_bigloop   # if nothing left for remainloop

        mtctr   r9              # setup loop count
ppc_i_remainloop:
        icbi    r5,r12		# flush a icache block
	a	r5,r5,r12	# increment r5 to the next cache block
        bdn     ppc_i_remainloop  # dec ctr and branch if non-zero

ppc_i_bigloop: 
        beq     cr1,ppc_i_sync

        mtctr   r10              # setup loop count
ppc_i_bigloop1:
        icbi    r5,r12		# flush a icache block
	a	r5,r5,r12	# increment r5 to the next cache block
        icbi    r5,r12		
	a	r5,r5,r12	
        icbi    r5,r12		
	a	r5,r5,r12	
        icbi    r5,r12		
	a	r5,r5,r12	
        bdn     ppc_i_bigloop1  # dec ctr and branch if non-zero

ppc_i_sync:
	lil	r3,0x0		# success return code
        isync                   # discard prefretched instructions
	br

spec1_nbytes:
	lil	r3,0x0		# success return code
	beqr	cr0		# return if nbytes = 0
	lil	r3,EINVAL	# error return code
        S_EPILOG

	FCNDES(vm_cflush_ppc_splt)


	include(errno.m4)
