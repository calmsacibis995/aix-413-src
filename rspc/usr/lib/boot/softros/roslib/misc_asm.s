# @(#)44	1.1  src/rspc/usr/lib/boot/softros/roslib/misc_asm.s, rspc_softros, rspc412 8/5/94 16:39:38
#
#   COMPONENT_NAME: rspc_softros
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


                .csect  .misc_asm[PR]

#include(regdefs.h)
   .set    PID,           1023

   .set         RTCU,   4
   .set         RTCL,   5
   .set         RTBU,   269
   .set         RTBL,   268

   .machine	"ppc"


##################################################################
#
#    File        :     misc_asm.s
#                      Miscellaneous routines that must be
#                      performed by assembly code (ie: get special
#                      purpose registers, etc).
#
#    The file misc_asm.h contains the function prototypes for these
#    routines so that they can be called by C routines.
#
####################################################################

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void rtc_dec( uint DELAY )
#
# DESCRIPTION
#
#       Perform DELAY nanoseconds delay loop using the DEC.
#
# INPUTS
#
#       DELAY (R3) = delay duration in terms of nanoseconds.
#
# Registers modified:
#       R0, R3, R4, R5, R6, R7,  CR0, DEC
# -----------------------------------------------------------------------------

  # Number of decrementer tics = 
  #       number of nanoseconds X local bus speed x speed mult / freq_adjust


                .set   freq_adjust_601, 125
                .set   mult_max_601,    eval(0x7FFFFFFF/125)
                .set   speed_mult_601,  2

                .set   freq_adjust_603, 4000
                .set   mult_max_603,    eval(0x7FFFFFFF/4000)
                .set   speed_mult_603,  1



                .globl  .rtc_dec        #

.rtc_dec:


                mflr    r5

                addi    r6, r3, 0               # Move R3 contents to R6

                .extern .get_bus_speed_info
                bl      .get_bus_speed_info
                rlwinm  r4, r3, 16, 24, 31     # Shift over the local bus speed


                mtlr    r5

                mfpvr   r5                     # Which processor is this?
                rlwinm  r5,r5,16,16,31
                cmpi    cr0,r5,1
                beq     cr0, set_601_delay

                # Set the processor specific information for 603/604
                mulli   r4, r4, speed_mult_603

                addis   5, 0, (freq_adjust_603 > 16)
                addi    5, 5, (freq_adjust_603 & 0xFFFF)  

                addis   7, 0, (mult_max_603 > 16)
                addi    7, 7, (mult_max_603 & 0xFFFF)  

set_601_delay:
                # Set the processor specific information for 601

ifdef(`RTC_STD_FREQ',
`              # RTC_STD_FREQ - Restore decrementer count, no conversion necessary
                addi   r3, r6, 0   
                b       move_to_dec                                         ',)

                mulli   r4, r4, speed_mult_601  # Multiply local bus speed

                addis   5, 0, (freq_adjust_601 > 16)
                addi    5, 5, (freq_adjust_601 & 0xFFFF)  

                addis   7, 0, (mult_max_601 > 16)
                addi    7, 7, (mult_max_601 & 0xFFFF)  
                b       convert_to_tics

convert_to_tics:
                cmpi    cr0, r6, 0      # Check if r6 is a REALLY BIG number
                blt     div_first       # If so, do the division first

                cmp     cr0, r6, r7     # Compare number of nanoseconds to
                bgt     cr0, div_first  # determine if division should be 
                                        # done first.

                mullw   r3, r4, r6      # Multiply nanoseconds by conversion speed
                divwu   r3, r3, r5      # Divide by freqency adjuster
                b       move_to_dec

div_first:
                divwu   r3, r6, r5      # Divide by freqency adjuster
                mullw   r3, r4, r3      # Multiply nanoseconds by conversion speed

move_to_dec:
                addi    r3, r3, 1       # Increment by 1 to ensure minimum delay
                mtdec   r3              # load decrementer from r3
                cmpi    cr0,0,r3,0      # test if parameter was 0
                btlr    eq              # return if parm was 0

large_value:    bgt     loop_till_1     # branch if value > 0
                mfdec   r3              # load r3 with decrementer
                cmpi    cr0,0,r3,0      # is value still < 0
                blt     large_value     # branch if still negative
loop_till_1:    mfdec   r3              # load r3 with decrementer
                cmpi    cr0,0,r3,0      # test for a timeout occur
                bgt     loop_till_1     #


                blr                     # return if timeout occured

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void set_dec( uint VALUE )
#
# DESCRIPTION
#
#       Update content of DEC with supplied value VALUE.
#
# INPUTS
#
#       VALUE (R3) = new content of DEC for updating.
# -----------------------------------------------------------------------------

                .globl  .set_dec        #

.set_dec:       mtdec   r3              # load DEC from r3
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint read_dec( void )
#
# DESCRIPTION
#
#       Obtain current content of DEC.
#
# OUTPUT
#
#       R3 = Content of DEC.
# -----------------------------------------------------------------------------

                .globl  .read_dec       #

.read_dec:      mfdec   r3              # load r3 from DEC
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint read_rtcu( void )
#
# DESCRIPTION
#
#       Obtain current content of RTCU.
#
# OUTPUT
#
#       R3 = Content of RTCU.
# -----------------------------------------------------------------------------

                .globl  .read_rtcu      #

#.read_rtcu:     mfrtcu  r3              # load r3 from RTCU


.read_rtcu:      mfpvr   r3              # Get processor version
                 rlwinm  r3,r3,16,16,31
                 cmpi    cr0,r3,1
                 beq     rrtcu_601
                 mftbu   r3		# load r3 from RTBU
                 blr
rrtcu_601:
                 mfspr   r3, RTCU        # load r3 from RTCU
                 blr                     #

#.read_rtcu:     mfspr   r3, RTCU        # load r3 from RTCU
#                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint read_rtcl( void )
#
# DESCRIPTION
#
#       Obtain current content of RTCL.
#
# OUTPUT
#
#       R3 = Content of RTCL.
# -----------------------------------------------------------------------------

                .globl  .read_rtcl      #

#.read_rtcl:     mfrtcl  r3              # load r3 from RTCL
.read_rtcl:      mfpvr   r3              # Get processor version
                 rlwinm  r3,r3,16,16,31
                 cmpi    cr0,r3,1
                 beq     rrtcl_601
                 mftb    r3		# load r3 from RTBL
                 blr
rrtcl_601:
                 mfspr   r3, RTCL        # load r3 from RTCL
                 blr                     #

#.read_rtcl:     mfspr   r3, RTCL        # load r3 from RTCL
#                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void set_rtcu( uint )
#
# DESCRIPTION
#
#       Set upper word of RTC register to value in R3.
#
# OUTPUT
#
#       RTCU loaded with R3 value
# -----------------------------------------------------------------------------

                .globl  .set_rtcu      #

#.set_rtcu:      mtrtcu  r3              # load RTCU with R3 value

.set_rtcu:       mfpvr   r0              # Get processor version
                 rlwinm  r0,r0,16,16,31
                 cmpi    cr0,r0,1
                 beq     srtcu_601
                 mtspr   RTBU, r3        # set RTBU from r3
                 blr
srtcu_601:
                 mtspr   RTCU, r3        # set RTCU from r3
                 blr                     #

#.set_rtcu:      mtspr   RTCU, r3        # load RTCU with R3 value
#                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void set_rtcl( uint )
#
# DESCRIPTION
#
#       Set lower word of RTC register to value in R3.
#
# OUTPUT
#
#       RTCL loaded with R3 value
# -----------------------------------------------------------------------------

                .globl  .set_rtcl      #

#.set_rtcl:      mtrtcl  r3              # load RTCL with R3 value

.set_rtcl:       mfpvr   r0              # Get processor version
                 rlwinm  r0,r0,16,16,31
                 cmpi    cr0,r0,1
                 beq     srtcl_601
                 mtspr   RTBL, r3        # set RTBL from r3
                 blr
srtcl_601:
                 mtspr   RTCL, r3        # set RTCL from r3
                 blr                     #

#.set_rtcl:      mtspr   RTCL, r3        # load RTCL with R3 value
#                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint get_msr( void )
#
# DESCRIPTION
#
#       Return current content of MSR.
# -----------------------------------------------------------------------------

                .globl  .get_msr        #

.get_msr:       mfmsr   r3              # Read MSR and return it in r3.
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void set_msr( uint msr_value )
#
# DESCRIPTION
#
#       Set MSR to the specified value msr_value.
# -----------------------------------------------------------------------------

                .globl  .set_msr        #

.set_msr:       isync
                mtmsr   r3              #
                isync
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint read_msr_ee( void )
#
# DESCRIPTION
#
#       Obtain the current state of the EE-bit of MSR.
#
# OUTPUT
#
#       R3 = Current state of the EE-bit of MSR.
# -----------------------------------------------------------------------------

                .globl  .read_msr_ee    #

.read_msr_ee:   mfmsr   r3              #  Get MSR value
                rlwinm  r3,r3,17,31,31  #  Rotate bit 15 to bit 1 and isolate.
                blr                     #  return R3 = 1 if EE = 1, 0 if EE = 0

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void set_msr_ee( void )
#
# DESCRIPTION
#
#       Set the EE-bit of MSR to 1.
# -----------------------------------------------------------------------------

                .globl  .set_msr_ee     #

.set_msr_ee:    mfmsr   r3              # read MSR
                ori     r3,r3,0x8000    # set EE bit
                isync
                mtmsr   r3              # update MSR
                isync
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void reset_msr_ee( void )
#
# DESCRIPTION
#
#       Clear the EE-bit of MSR to 0.
# -----------------------------------------------------------------------------

                .globl  .reset_msr_ee   #

.reset_msr_ee:  mfmsr   r3              # read MSR
                andi.   r3,r3,0x7FFF    # clear EE bit
                isync
                mtmsr   r3              # update MSR
                isync
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint get_srr0( void )
#
# DESCRIPTION
#
#       Return current contents of SRR0.
# -----------------------------------------------------------------------------

                .globl  .get_srr0       #

.get_srr0:      mfsrr0  r3              # Read SRR0 and return it in r3.
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint get_srr1( void )
#
# DESCRIPTION
#
#       Return current contents of SRR1.
# -----------------------------------------------------------------------------

                .globl  .get_srr1       #

.get_srr1:      mfsrr1  r3              # Read SRR1 and return it in r3.
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint get_DAR( void )
#
# DESCRIPTION
#
#       Obtain the content of the DAR special register of this processor.
#
# OUTPUT
#
#       R3 = Content of DAR register.
# -----------------------------------------------------------------------------

                .globl  .get_DAR        #


.get_DAR:
                mfdar   r3              #
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint get_DSISR( void )
#
# DESCRIPTION
#
#       Obtain the content of the DSISR special register of this processor.
#
# OUTPUT
#
#       R3 = Content of DSISR register.
# -----------------------------------------------------------------------------

                .globl  .get_DSISR        #


.get_DSISR:
                mfspr   r3, dsisr         #
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint get_PID( void )
#
# DESCRIPTION
#
#       Obtain the content of the PID special register of this processor.
#
# OUTPUT
#
#       R3 = Content of PID register.
# -----------------------------------------------------------------------------

                .globl  .get_PID        #


.get_PID:       mfspr   r3,PID          #
                blr                     #

# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint get_PVR( void )
#
# DESCRIPTION
#
#       Obtain the content of the PVR special register of this processor.
#
# OUTPUT
#
#       R3 = Content of PVR register.
# -----------------------------------------------------------------------------

                .globl  .get_PVR        #

.get_PVR:       mfpvr   r3              #
                blr                     #


# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void set_seg_reg( uint seg_reg_num, uint seg_reg_value )
#
# DESCRIPTION
#
#       Update segment register specified by seg_reg_num with new contents
#       seg_reg_value. Only low 4 bits of input seg_reg_num is used as an ID.
#
# INPUTS
#
#       seg_reg_num (R3)   = ID number of the segment register to be updated.
#       seg_reg_value (R4) = value to be written into the segment register.
# -----------------------------------------------------------------------------

                .globl  .set_seg_reg    #

.set_seg_reg:

                isync
                rlwinm   r3, r3, 28, 0, 3    # Shift SR select bits to 0-3
                mtsrin   r4, r3         # Store the value in R4 in the 
                isync                   # segment register specified by
                                        # bits 0-3 of R3.
                isync

                blr                     # R3 has return value...



# -----------------------------------------------------------------------------
# FUNCTION:
#
#       uint get_seg_reg( uint seg_reg_num )
#
# DESCRIPTION
#
#       Obtain the current content of the segment register specified by
#       seg_reg_num. Only low 4 bits of input seg_reg_num is used as an ID.
#
# OUTPUT
#
#       R3 = Contents of segment register specified by seg_reg_num.
#      CR0 = flags modified by this routine.
# -----------------------------------------------------------------------------

                .globl  .get_seg_reg    #

.get_seg_reg:

                rlwinm   r3, r3, 28, 0, 3    # Shift SR select bits to 0-3
                mfsrin   r3, r3         # Get segment register specified
                                        # by bits 0-3 of R3 and put
                                        # result in R3.

                blr                     # R3 has return value...



# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void halt( void )
#
# DESCRIPTION
#
#       Halt the system by executing an infinite loop.
#
# OUTPUT
#
#       N/A
# -----------------------------------------------------------------------------

                .globl  .halt           #

.halt:          b       $               # Loop forever....



# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void _ptrgl
#
# DESCRIPTION
#
#       This allows us to use the pointers to functions construct
#       as described in K&R 5.12
#
# OUTPUT
#
#       Vectors onto specified routine...
# -----------------------------------------------------------------------------

.globl  ._ptrgl

._ptrgl:
        l       r0,0x00(r11)            # Load address of called function
        st      r2,0x14(r1)             # Store stuff in area
        mtctr   r0                      # move address of called function to CTR
        l       r2,0x04(r11)            # Get new TOC from parm area
        l       r11,0x08(r11)           # Get new LR from parm area
        bctr




# -----------------------------------------------------------------------------
# FUNCTION:
#
#       void _eieio
#
# DESCRIPTION
#
#       Execute the eieio instruction.
#
# OUTPUT
#
# -----------------------------------------------------------------------------

.globl  ._eieio

._eieio:
        eieio                           # Enforce In Order Execution of I/O
        blr                                                       

