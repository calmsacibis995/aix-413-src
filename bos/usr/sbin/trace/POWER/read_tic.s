# @(#)75	1.2  src/bos/usr/sbin/trace/POWER/read_tic.s, cmdtrace, bos411, 9430C411a 7/21/94 02:43:52
#
#   COMPONENT_NAME: cmdtrace
#
#   FUNCTIONS: read_tic
#
#   ORIGINS: 27,83
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# 
# LEVEL 1, 5 Years Bull Confidential Information 


#	read_tic(times_array, count)
#		where times_array is pointer to an array of ints
#		and count is the number of items to get
#	This program reads the timer tbl register repeatedly
#	count times and stores the values in times_array.

		.machine "ppc"
		S_PROLOG(read_tic)

		mtctr   r4              # set up loop counter
		addi    r3,r3,-4        # back up from the array to use stwu
					# instruction
	loop:
		mftb    r5              # read time base lower to register 5
		stwu    r5,4(r3)        # increment r3 by 4 and store r5 to
					# that address.
		bdnz+   loop            # go back to the loop if not done
		S_EPILOG
		FCNDES(read_tic)
