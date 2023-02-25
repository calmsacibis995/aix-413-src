# @(#)76	1.2  src/bos/usr/sbin/trace/POWER/read_nano.s, cmdtrace, bos411, 9430C411a 7/21/94 02:43:58
#
#   COMPONENT_NAME: cmdtrace
#
#   FUNCTIONS: read_nano
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


#	read_nano(times_array, count)
#		where times_array is pointer to an array of ints
#		and count is the number of items to get
#	This program reads the timer nanoseconds register repeatedly
#	count times and stores the values in times_array.

        	.set    MF_RTCL, 05     # realtime clock lower (for mfspr)
		S_PROLOG(read_nano)

		mtctr   r4              # set up loop counter
		addi    r3,r3,-4        # back up from the array to use stwu
					# instruction
	loop:
		mfspr	r5, MF_RTCL	# read nanoseconds register into 5
		stu     r5,4(r3)        # increment r3 by 4 and store r5 to
					# that address.
		bdnz+   loop            # go back to the loop if not done
		S_EPILOG
		FCNDES(read_nano)
