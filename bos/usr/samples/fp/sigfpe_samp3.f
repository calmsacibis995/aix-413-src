* @(#)79	1.1  src/bos/usr/samples/fp/sigfpe_samp3.f, fpsamp, bos411, 9428A410j 9/8/93 10:22:32
*
*   COMPONENT_NAME: BOSSAMP
*
*   FUNCTIONS: div
*		overflow
*		underflow
*		zero
*
*   ORIGINS: 27
*
*
*   (C) COPYRIGHT International Business Machines Corp. 1993
*   All Rights Reserved
*   Licensed Materials - Property of IBM
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*

* This program demonstates two approaches to generating
* floating point traps in FORTRAN programs.  One approach
* is to call a C language routine to establish a trap 
* handler and to enable traps.  The second approach is 
* to call the appropriate routines to serialize the process
* and enable traps directly from the FORTRAN program.
* With this approach there is no signal handler, so the
* first trap will cause the program to abort and produce
* a core file, which can be examined with a debugger such
* as dbx to find the cause of the trap.

* The routine f_trap_sigfpe() is found in sigfpe_samp2.c.  
* This routine takes two arguments:  If the first argument is
* one (1), then a trap handler is established, and the trap(s) 
* defined by the second argument are enabled.   If the
* first argument is zero (0), then the trap handler is
* removed and all traps are disabled.

*************************************************************

* The include file fp_fort_c.f defines a named common
* which contains argument values for use by fp_enable()
* and fp_trap().  Notice that these routines are also
* declared as external by the calling FORTRAN program.

      include(fp_fort_c.f)

      external f_trap_sigfpe
      external fp_enable
      external fp_trap
      integer enable_flags

* Establish the signal handler and enable some traps.

      enable_flags = trp_invalid + trp_overflow + 
     $               trp_underflow + trp_div_by_zero
      call f_trap_sigfpe(1, enable_flags)

* The following routines will cause floating point traps
* to occur.  The signal handler should report them.

      write(*,*) "with traps ENABLED"
      call div_by_zero()
      call overflow()
      call underflow()
      call zero_over_zero()

* Remove the signal handler and disable all floating point
* traps.

      call f_trap_sigfpe(0, 0)

* The following routines cause floating point exceptions, but
* should not trap this time.

      write(*,*) "with traps DISABLED"
      call div_by_zero()
      call overflow()
      call underflow()
      call zero_over_zero()

* Demonstrate enabling traps and serializing the process
* within a FORTRAN program, without use of any C programming.
* This will cause the program to terminiate and produce a 
* core file as soon as the first enabled trap occurs.

* fp_trap() is called to place the process in serial execution
* mode; fp_enable() is called to enable the desired trap(s).
* Note that these routines use call-by-value parameter passing,
* so the %VAL macro is required.

      write(*,*) "and now, should produce a core file"
      call fp_trap(%VAL(fp_trap_sync))
      call fp_enable(%VAL(trp_div_by_zero))
      call div_by_zero()

      stop
      end

************************************************************

* The following subprograms perform operations which
* will cause floating point exceptions to occur.

      subroutine div_by_zero()
      double precision zero, a
      zero = 0.0d0
      write(*,*) "divide by zero:"
      a = 1.0 / zero
      return
      end
      
      subroutine overflow()
      double precision a, b
      a = 1.0d300
      write(*,*) "overflow:"
      b = a * a
      return
      end

      subroutine underflow()
      double precision a, b
      a = 100.0d-300
      write(*,*) "underflow:"
      b = a * a
      return
      end

      subroutine zero_over_zero()
      double precision a, b
      a = 0.0d0
      write(*,*) " zero / zero:"
      b = a / a
      return
      end

*****************************************************************

* The following BLOCK DATA subprogram initializes
* the common area which contains the arguments used
* by fp_trap() and fp_enable().

      block data float_block
      include(fp_fort_c.f)
      include(fp_fort_t.f)
      end
