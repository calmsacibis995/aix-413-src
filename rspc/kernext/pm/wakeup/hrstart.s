# @(#)87        1.5  src/rspc/kernext/pm/wakeup/hrstart.s, pwrsysx, rspc41J, 9519A_all 5/9/95 06:58:46
#############################################################################
#
#   COMPONENT_NAME: PWRSYSX
#
#   FUNCTIONS: Hibernation Resume Start(__hrstart)
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#############################################################################
# hrstart.s -   Entry point for Hibernation Resume from POST
#
#        INPUT :   r3  = residual data entry address
#                        In the residual data, first entry(4bytes) shows
#                        ResidualLength.
#
#        NOTE  :
#               This procedure relies on an executable that has been
#               prepared by the boot header program to include some
#               information in a well known place in the hibernation
#               header.
#
#               Following is the hibernation header. This header begins
#               from address ( __hrstart - 1024 ) on the memory.
#
#  typedef struct _pm_hibernation_header {
#     char    signature[8];           /* "AIXPM001" */
#     unsigned int *wakeup_area;      /* physical address of wakeup area */
#     int     wakeup_area_length;     /* length of the wakeup area */
#     int     wakeup_code_offset;     /* offset of the wakeup code */
#     int     wakeup_code_length;     /* length of the wakeup code */
#     int     config_data_offset;     /* offset of the config data */
#     int     config_data_length;     /* length of the config data */
#     unsigned int *restart;          /* physical restart address */
#     int     image_offset;           /* offset of compressed memory image */
#     int     image_length;           /* length of compressed memory image */
#     int     bitmap_offset;          /* offset of the bitmap data */
#     int     bitmap_length;          /* length of the bitmap data */
#     char    reserve[12];
#  };
#
#               This module is self-relocating and will allow loading
#               to an arbitrary memory address and relocate itself
#               to the target address.
#
###########################################################################
#
#       Flow:
#               - Disable External Interrupts
#               - Translation off
#               - Get necessary residual data and check them
#               - Determine Entry point and save
#               - Relocate to target address
#               - Initialize TOC
#               - Initialize Stack
#               - Initialize BSS
#               - Call Main
#
#***********************************************************************

.machine "ppc"

include(hrstart.m4)			# file unique definition 
include(machine.m4)			# MSR related definition 


        S_PROLOG(__hrstart)
ENTRY(__hrstart):
                                        # Begin Initialization
        bl      txt_strt                # Get current location
txt_strt:
#-----------------------------------------------------------------------
#
########################################################################
#       System Initialization
########################################################################
#
# Disable interrupt and data/instruction translation
#
        mfmsr   r10
        rlwinm  r10,r10,0,~MSR_EE       # EE=0
        rlwinm  r10,r10,0,~MSR_DR       # DR=0
        rlwinm  r10,r10,0,~MSR_IR       # IR=0
        sync
        mtmsr   r10
        isync
#
########################################################################
#       Get starting address of code/header/config
########################################################################
#
        mflr    r6                      # Setup starting address
        addi    r6,r6,-4                # Adjust it back to zero base(r6)
        mr      r7,r6                   # Copy code starting address
        addi    r7,r7,-header2wakeupcode # Get header start address (r7)

        lwz     r8,config_data_offset(r7) # Get config start offset
        add     r8,r8,r7                # Get config start address (r8)
#
########################################################################
#       Get necessary residual data  (To check config change)
########################################################################
#
# Check Residual Data
#       The Segs[] array must be excluded from check-sum. The Segs[]
#       array is from the start_segs-th byte to the end_segs-th byte
#       in the residua data.
#   r3 : (Input) Residual Data Start Address. First entry is length.
#   r8 : (Input) Config Data Start Address.
#   r20 - r24 : Internaly used.

        addi    r20,0,0                 # Clear up SUM

                                        # Sum of data before Segs[] array
        addi    r22,0,start_segs        # Set loop count
        addi    r21,r3,-1               # Set r21=start-1
        mtctr   r22                     # Set counter register
calc_sum1:	
        lbzu    r23,1(r21)              # Get next byte
        add     r20,r20,r23             # Add it to sum
        bdnz    calc_sum1               # Dec counter, loop if not zero.

                                        # Sum of data after Segs[] array
        lwz     r22,0(r3)               # Get reidual data length
        addi    r21,r3,end_segs-1       # Set r21=start+end_segs-1
        addi    r22,r22,-end_segs       # Set loop count (size-end_segs)
        mtctr   r22                     # Set counter register
calc_sum2:	
        lbzu    r23,1(r21)              # Get next byte
        add     r20,r20,r23             # Add it to sum
        bdnz    calc_sum2               # Dec counter, loop if not zero.

        lwz     r24,residual_sum(r8)    # Get residual sum
        cmplw   r20,r24                 # Compare
        beq     check_ok                # If same, ok.

         .globl  .goto_reboot
.goto_reboot:
        mfpvr   r10                     #  check processor type
        srwi    r10,r10,16              #  extract processor version
        cmpwi   cr6,r10,PV601           #  is this a 601?
        beq     cr6,system_reset        #  branch if 601

                                        # 603/604 require to disable D/I-cache
        mfspr   r10,H0_603				# get HID0
        rlwinm  r11,r10,0,~(H0_603_DCE|H0_603_ICE)	# DCE=0, ICE=0
        ori     r12,r11,(H0_603_ICFI|H0_603_DCI)	# ICFI=1, DCI=1
        mtspr   H0_603,r12				# invalidate D/I-cache
        mtspr   H0_603,r11				# disable D/I-cache

system_reset:
        ba      0xfff00100              # Jump System Reset. REBOOT!
check_ok:

#
########################################################################
#       Relocate the image to the TARGET ADDRESS
########################################################################
#

                                        # Setup for relocation
        mr      r20,r7                  # Setup initial source address
        lwz     r21,wakeup_code_length(r7) # Get size in bytes of wakeup code
        addi    r21,r21,header2wakeupcode  # Add header area size     
	lwz     r22,pwakeup_area(r7)    # Setup wakeup area start address
	addi    r22,r22,0x2000          # first 8KB must be skipped
                                   # Here, r21:counter  r20:source  r22:target

            DBOUT(0xb0,r29,r30,r31) #DEBUG

xfer_img:
        lwz     r25,0(r20)              # Get source op-code
        stw     r25,0(r22)              # Write to target addr
        dcbf    0, r20                  # flush what we pulled in
        dcbf    0, r22                  # flush what we wrote
        sync
        icbi    0, r22                  # invalidate target
        addi    r20,r20,4               # Bump source addr
        addi    r22,r22,4               # Bump target addr
        addic.  r21,r21,-4              # Decrement byte count
        bge     xfer_img                # Branch if not done

        sync
        isync

            DBOUT(0xb1,r29,r30,r31) #DEBUG

# Branch to newly-relocated code
        subf    r20,r20,r22             # Get offset of target and source
        bl      still_original          # Get current address
still_original:
        mflr    r23                     # Save current address
        la      r23,relocat-still_original(r23) # Get offset in source
        add     r23,r23,r20             # Get relocat address in copy
        mtlr    r23                     # Set it in Link Register
        br                              # Jump to relocat in copy
relocat:

        isync                           # sync it up

            DBOUT(0xb2,r29,r30,r31) #DEBUG
#
########################################################################
#                      Initialize TOC register
########################################################################
#
        bl      here                    # Get current location
here:   mflr    r5                      # Put link reg. in R5

        .extern hrsetup[DS]
        la      r2,hrsetup[DS]-here(r5)
	lwz	r2,4(r2)		# get TOC pointer
#
#   To create a relocatable code, we do not use -T option of LDFLAGS.
#   So the real target address should be added into the gotten TOC value. 
#   In other words, the TOC value in the descriptor is relative. 
#
	add	r20,r20,r6		# r20 = target address 	
	add	r2,r2,r20		# r2 = calculated TOC
#
#   If C routines use many static data or global data, the following 
#   TOC listed data pointer should be maintained more than one entry.
#   The total count of TOC listed data is countable if the COFF header
#   is attached with our code but no header exists because the hibernation
#   dump program removed it.
#   Currently we permit only hrsetup.c can use $STATIC data.
# 

#   For $STATIC in hrsetup.c 
	lwz	r21,0(r2)		# get the first data in TOC
	add	r21,r21,r20		# added target address 
	stw	r21,0(r2)		# put the first data in TOC        

#   For $STATIC_BSS in hrsetup.c 
        lwz     r21,4(r2)               # get the second in TOC
        add     r21,r21,r20             # added target address
        stw     r21,4(r2)               # put the second data in TOC

            DBOUT(0xb3,r29,r30,r31) #DEBUG

#
########################################################################
#      Initialize Stack Pointer and preallocate 1st Stack frame
########################################################################
#
# We can assume 56(=64-8)kb is reserved for this wakeup code. 
# So the initial stack is set to 56kb higher than this code top
# because the required stack size is unknown for decompressing. 
# Probably all kind of hash table will be located in stack to reduce 
# the static/global TOC pointers, resulting in huge stack consuming. 
#
        la      r1,ENTRY(__hrstart)-here(r5) # Put stack_bottom location in R1 
        addi    r1,r1,-header2wakeupcode # minus header and config data size 
	addis	r1,r1,0x0001		# 64kb higher than this code top
	addi 	r1,r1,-0x2000		# -8kb
        rlinm   r1,r1,0,~7              # double-word align
        la      r1,STK_FRM_SIZ(r1)      # Allocate caller stack frame
        xor     r9,r9,r9                # clear a register
        stw     r9,0(r1)                # zero out the stack's back chain ptr
	mr	r5,r1			# set stack address as 3rd parameter
	
            DBOUT(0xb4,r29,r30,r31) #DEBUG
#
########################################################################
#              Do General initialzation, then call main()
########################################################################
#
#	r3 has already contained residual data top address
#	r5 has already contained our stack address
#
        mr      r4,r7                   # set header top address
	.extern ENTRY(hrsetup[PR])
        bl      ENTRY(hrsetup[PR])      # Does basic crt  initialization
        cror    r31,r31,r31             # and then calls main()

        ##############
        # NOTREACHED
        ##############

########################################################################
#  GetIoControlBase 
#  Function : Get I/O Control Base Address (ISA)
#  Input    : none
#
########################################################################
#
         .globl  .GetIoControlBase
.GetIoControlBase:
        lis	r3, 0x8000		# return = 0x80000000 for physical
        br
            
########################################################################
#  FlushIO
#  Function : Flush I/O buffers
#  Input    : none 
#
########################################################################
#
         .globl  .FlushIO
.FlushIO:
	eieio				# flush I/O area
        br

########################################################################
#  WriteOneByte
#  Function : Write one byte to a port
#  Input    : r3   Data 
#             r4   Port number
########################################################################
#
         .globl  .WriteOneByte
.WriteOneByte:
	SPR_BITOFF(r5,H0_603,H0_603_DCE)	# D-cache off
	oris	r4, r4, 0x8000
	stb	r3, 0(r4)
	SPR_BITON(r5,H0_603,H0_603_DCE)		# D-cache on
        br

########################################################################
#  ReadOneByte
#  Function : Read one byte from a port
#  Input    : r3   Port number 
#
########################################################################
#
         .globl  .ReadOneByte
.ReadOneByte:
	SPR_BITOFF(r4,H0_603,H0_603_DCE)	# D-cache off
	oris	r3, r3, 0x8000
	lbz	r3, 0(r3)
	SPR_BITON(r4,H0_603,H0_603_DCE)		# D-cache on
        br

########################################################################
#  hrmain
#  Function : Go to CPU resume and Resume
#  Input    : r3   header start address
#
########################################################################
#
         .globl  .hrmain
.hrmain:
#
# set address and jump
#
        lwz     r3, prestart(r3)        # Get restart address
        mtlr    r3                      # Set restart address to LR
        br
         .align  2

#
########################################################################
#  __hrstart function descriptor for its entry and TOC
########################################################################
#

        FCNDES(__hrstart)
                                                                               
