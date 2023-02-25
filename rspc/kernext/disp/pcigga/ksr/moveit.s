# @(#)06  1.1  src/rspc/kernext/disp/pcigga/ksr/moveit.s, pcigga, rspc411, 9435D411a 9/2/94 16:06:51
# moveit.s
# based on @(#)44        1.4  src/bos/kernext/disp/wga/ksr/moveit.s, bos, bos410 3/16/93 20:44:42
#
#  COMPONENT_NAME: PCIFGA
#
#  FUNCTIONS: moveit
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1994
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#######################################################################
#
#  NAME: moveit
#
#  FUNCTION: Fast copies WGA memory for text scrolling
#
#  Calling sequence: _moveit
#
#       R3  Address of target string
#       R4   Address of source string
#       R5   Length of target string
#       R6   x addr increment value
#       R7   y addr increment value
#       R8   number of scan lines per row to copy
#
        S_PROLOG(moveit)
        xor     9,9,9           # clear r9
        a       12,5,9          # save off length of target string
yloop:
        a       10,3,9          # save off Address of target string
        a       11,4,9          # save off Address of source string
xloop:
        l       9,0(4)          # read adapter memory from source
        a       4,4,6           # add x address increment
        si.     5,5,1           # decrement transfer count
        st      9,0(3)          # write adapter memory to destination
        a       3,3,6           # add x address increment
        bc      0xC,1,xloop     # loop until scan line is complete
                                #
        xor     9,9,9           # re-clear r9
        a       3,10,7          # add y address increment
        a       4,11,7          # add y address increment
        a       5,12,9          # restore length of target string
        si.     8,8,1           # decrement scan line count
        bc      0xC,1,yloop     # loop through all scanlines
                                #
        S_EPILOG                #return.   Generate proc end table
