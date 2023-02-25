# @(#)51  1.1  src/bos/usr/sbin/trace/read_nano.s, cmdtrace, bos411, 9428A410j 8/19/93 08:28:01
# 
# COMPONENT_NAME: CMDTRACE  system trace logging and reporting facility
# 
# FUNCTIONS: read_nano
# 
# ORIGINS: 83 
# 
# LEVEL 1, 5 Years Bull Confidential Information 


#	read_nano(times_array, count)
#		where times_array is pointer to an array of ints
#		and count is the number of items to get
#	This program reads the timer nanoseconds register repeatedly
#	count times and stores the values in times_array.

	     .globl   .read_nano
 .read_nano: 
	lil	6, 0	# initialize offset
	mtctr	4	# setup loop count

loop :
	mfspr	5, 5		# read nanoseconds register into 5
	stx	5, 6, 3		# store nanoseconds to offset
	ai	6, 6, 4
	bc	16,0,loop
	br
