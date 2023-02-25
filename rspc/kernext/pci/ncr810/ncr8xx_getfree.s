# @(#)54	1.1  src/rspc/kernext/pci/ncr810/ncr8xx_getfree.s, pciscsi, rspc41J, 9507A 1/25/95 14:22:52
#
#   COMPONENT_NAME: pciscsi
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
        .file "ncr8xx_getfree.s"

#-----------------------------------------------------------------------------
#
# NAME: ncr8xx_getfree
#
# FUNCTION: This routine returns the index of the first free element of a list
#	    by counting the leading zeroes of the word
#
# CALL: (int)ncr8xx_getfree(word)
#       uint word;            /* 32-bit word             */
#
# EXECUTION ENVIORNMENT:
#       This service can be called on either the process or
#       interrupt level
#
#       It only page faults on the stack when under a process
#
# RETURN VALUE:
#       0..31   =       Valid index to free element
#       32      =       No free element
#
#-----------------------------------------------------------------------------


	.csect .p8xx_getfree[PR]
       .globl .p8xx_getfree[PR]
# 	S_PROLOG(p8xx_getfree)
	cntlz	3, 3		# count the leading zeros and return result
# 	S_EPILOG(p8xx_getfree)
	br
        .align  2
 	 .byte   0xdf,2 ,0xdf,0

