/* @(#)59	1.2  src/rspc/kernext/disp/pciiga/inc/igatrc.h, pciiga, rspc41J, 9515B_all 3/22/95 17:04:25 */

/*
 * COMPONENT_NAME: (pciiga)  Device Driver
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_IGATRC
#define _H_IGATRC

/* Include the new Graphics Systems system trace facility */
#include <graphics/gs_trace.h>

/*
 * This file is based off the principles formulated and applied
 * to the new Graphics Systems system trace facility defined in
 * the gs_trace.h file which is included into this header file.
 * Basically, there are 2 types of trace hook types defined in
 * the standard gs_trace.h file, one for debug only and one for
 * ship hooks. They operate the same way when GS_DEBUG is enabled
 * where run-time checking is used to determine whether or not
 * a given trace hook is written or not. These run-time priority
 * numbers can be changed with a debugger on the fly. When GS_DEBUG
 * is not defined (this is done in production ship code), then
 * all debug hooks turn into comments and the ship hooks turn
 * into direct system trace calls with no run-time check for
 * performance reasons. This is all explained further in the
 * comments contained within the gs_trace.h header file.
 *
 * In the Icecube device driver, an in-memory trace buffer is
 * setup during config time and utilized throughout the driver.
 * This form of tracing is fast and especiallu useful for
 * debugging CET and field problems because the customer does
 * not have to run anything special or do anything out of the
 * ordinary except for enabling the system low-level debugger
 * which is fairly simple and straightforward.
 *
 * To make the tracing statements as simple and small as possible
 * to allow less cluttler in the real code, it becomes
 * almost necessary to have only one trace call at any one
 * point in the code where tracing is needed no matter whether
 * it is debug system trace, ship system trace, in-memory trace,
 * or whatever... So, in order to do this effectively for the
 * Icecube device driver, this header file defines a complete
 * set of trace hook macros which are very similar to the ones
 * found in the gs_trace.h header file with a couple of
 * exceptions:
 *
 * 1) A major hook parameter is not required. This hook is
 *    defined globally (enum in gshkid.h)
 * 2) A ddf pointer is needed to access the in-memory trace
 *    buffer information
 * 3) A third type of trace hook was added alongside the existing
 *    debug and ship hook
 *
 * The third type of trace hook is the in-memory trace hook. This
 * hook will only produce in-memory traces and no system traces
 * when GS_DEBUG is not enabled (ie. in production code). However,
 * when GS_DEBUG is enabled, the system trace is active as well
 * for these new type of hooks.
 *
 * The following table shows how the trace hooks are used:
 *
 *              GS_DEBUG defined           GS_DEBUG not defined
 * Hook Type    (debug level code)         (production ship code)
 * ---------    ------------------         ---------------------
 *
 * DBG          system trace with runtime      nothing
 *              priority checks and also
 *              in-memory trace
 *
 * TRC          system trace with runtime      system trace with no
 *              priority checks and also       runtime priority checks
 *              in-memory trace                and in-memory trace
 *
 * MTRC         system trace with runtime      in-memory trace only
 *              priority checks and also
 *              in-memory trace
 *
 * Of course if the GS_REMOVE_TRACE is enabled, then no trace is
 * done at all including the in-memory trace. In fact, the in-memory
 * trace buffer is not even set up if the GS_REMOVE_TRACE is enabled.
 * This flag should not be used except for verification of
 * performance impacts of the tracing facility for production code as
 * it defeats the whole purpose of the tracing mechanism.
 *
 */

extern long iga_memtrace();

#ifndef GS_DEBUG_TRACE_SWITCH

#define IGA_ENTER_DBG0(ddf, module, priority, routine)
#define IGA_ENTER_DBG1(ddf, module, priority, routine, p1)
#define IGA_ENTER_DBG(ddf, module, priority,                    \
                routine, p1, p2, p3, p4, p5)

#define IGA_EXIT_DBG0(ddf, module, priority, routine)
#define IGA_EXIT_DBG1(ddf, module, priority, routine, p1)
#define IGA_EXIT_DBG(ddf, module, priority,                     \
                routine, p1, p2, p3, p4, p5)

#define IGA_PARM_DBG0(ddf, module, priority, routine, subfunc)
#define IGA_PARM_DBG1(ddf, module, priority, routine,           \
                subfunc, p1)
#define IGA_PARM_DBG(ddf, module, priority,                     \
                routine, subfunc, p1, p2, p3, p4, p5)

#ifdef GS_REMOVE_TRACE

#define IGA_ENTER_TRC0(ddf, module, priority, routine)
#define IGA_ENTER_TRC1(ddf, module, priority, routine, p1)
#define IGA_ENTER_TRC(ddf, module, priority,                    \
                routine, p1, p2, p3, p4, p5)

#define IGA_EXIT_TRC0(ddf, module, priority, routine)
#define IGA_EXIT_TRC1(ddf, module, priority, routine, p1)
#define IGA_EXIT_TRC(ddf, module, priority,                     \
                routine, p1, p2, p3, p4, p5)

#define IGA_PARM_TRC0(ddf, module, priority, routine, subfunc)
#define IGA_PARM_TRC1(ddf, module, priority, routine,           \
                subfunc, p1)
#define IGA_PARM_TRC(ddf, module, priority,                     \
                routine, subfunc, p1, p2, p3, p4, p5)

#define IGA_ENTER_MTRC0(ddf, module, priority, routine)
#define IGA_ENTER_MTRC1(ddf, module, priority, routine, p1)
#define IGA_ENTER_MTRC(ddf, module, priority,                   \
                routine, p1, p2, p3, p4, p5)

#define IGA_EXIT_MTRC0(ddf, module, priority, routine)
#define IGA_EXIT_MTRC1(ddf, module, priority, routine, p1)
#define IGA_EXIT_MTRC(ddf, module, priority,                    \
                routine, p1, p2, p3, p4, p5)

#define IGA_PARM_MTRC0(ddf, module, priority, routine, subfunc)
#define IGA_PARM_MTRC1(ddf, module, priority, routine,          \
                subfunc, p1)
#define IGA_PARM_MTRC(ddf, module, priority,                    \
                routine, subfunc, p1, p2, p3, p4, p5)

#else /* ! GS_REMOVE_TRACE */

#define IGA_ENTER_TRC0(ddf, module, priority, routine)          \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, ENTER), 0, 0, 0);                      \
        GS_ENTER_TRC0(HKWD_GS_IGA_DD, module,                   \
                priority, routine);                             \
}

#define IGA_ENTER_TRC1(ddf, module, priority, routine, p1)      \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, ENTER), p1, 0, 0);                     \
        GS_ENTER_TRC1(HKWD_GS_IGA_DD, module,                   \
                priority, routine, p1);                         \
}

#define IGA_ENTER_TRC(ddf, module, priority, routine,           \
                 p1, p2, p3, p4, p5)                            \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, ENTER), p1, p2, p3);                   \
        GS_ENTER_TRC(HKWD_GS_IGA_DD, module,                    \
                priority, routine, p1, p2, p3, p4, p5);         \
}

#define IGA_EXIT_TRC0(ddf, module, priority, routine)           \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, EXIT), 0, 0, 0);                       \
        GS_EXIT_TRC0(HKWD_GS_IGA_DD, module,                    \
                priority, routine);                             \
}

#define IGA_EXIT_TRC1(ddf, module, priority, routine, p1)       \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, EXIT), p1, 0, 0);                      \
        GS_EXIT_TRC1(HKWD_GS_IGA_DD, module,                    \
                priority, routine, p1);                         \
}

#define IGA_EXIT_TRC(ddf, module, priority, routine,            \
                p1, p2, p3, p4, p5)                             \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, EXIT), p1, p2, p3);                    \
        GS_EXIT_TRC(HKWD_GS_IGA_DD, module,                     \
                priority, routine, p1, p2, p3, p4, p5);         \
}

#define IGA_PARM_TRC0(ddf, module, priority, routine, subfunc)  \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, subfunc), 0, 0, 0);                    \
        GS_PARM_TRC0(HKWD_GS_IGA_DD, module,                    \
                priority, routine, subfunc);                    \
}

#define IGA_PARM_TRC1(ddf, module, priority, routine,           \
                subfunc, p1)                                    \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, subfunc), p1, 0, 0);                   \
        GS_PARM_TRC1(HKWD_GS_IGA_DD, module,                    \
                priority, routine, subfunc, p1);                \
}

#define IGA_PARM_TRC(ddf, module, priority, routine,            \
                subfunc, p1, p2, p3, p4, p5)                    \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, subfunc), p1, p2, p3);                 \
        GS_PARM_TRC(HKWD_GS_IGA_DD, module,                     \
                priority, routine, subfunc,                     \
                p1, p2, p3, p4, p5);                            \
}

#define IGA_ENTER_MTRC0(ddf, module, priority, routine)         \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, ENTER), 0, 0, 0);                      \
}

#define IGA_ENTER_MTRC1(ddf, module, priority, routine, p1)     \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, ENTER), p1, 0, 0);                     \
}

#define IGA_ENTER_MTRC(ddf, module, priority, routine,          \
                 p1, p2, p3, p4, p5)                            \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, ENTER), p1, p2, p3);                   \
}

#define IGA_EXIT_MTRC0(ddf, module, priority, routine)          \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, EXIT), 0, 0, 0);                       \
}

#define IGA_EXIT_MTRC1(ddf, module, priority, routine, p1)      \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, EXIT), p1, 0, 0);                      \
}

#define IGA_EXIT_MTRC(ddf, module, priority, routine,           \
                p1, p2, p3, p4, p5)                             \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, EXIT), p1, p2, p3);                    \
}

#define IGA_PARM_MTRC0(ddf, module, priority, routine, subfunc) \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, subfunc), 0, 0, 0);                    \
}

#define IGA_PARM_MTRC1(ddf, module, priority, routine,          \
                subfunc, p1)                                    \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, subfunc), p1, 0, 0);                   \
}

#define IGA_PARM_MTRC(ddf, module, priority, routine,           \
                subfunc, p1, p2, p3, p4, p5)                    \
{                                                               \
        iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,           \
                routine, subfunc), p1, p2, p3);                 \
}

#endif /* ! GS_REMOVE_TRACE */
#else /* GS_DEBUG_TRACE_SWITCH */

#define IGA_ENTER_TRC0(ddf, module, priority, routine)          \
        IGA_ENTER_DBG0(ddf, module, priority, routine)
#define IGA_ENTER_TRC1(ddf, module, priority, routine, p1)      \
        IGA_ENTER_DBG1(ddf, module, priority, routine, p1)
#define IGA_ENTER_TRC(ddf, module, priority,                    \
                routine, p1, p2, p3, p4, p5)                    \
        IGA_ENTER_DBG(ddf, module, priority,                    \
                routine, p1, p2, p3, p4, p5)

#define IGA_EXIT_TRC0(ddf, module, priority, routine)           \
        IGA_EXIT_DBG0(ddf, module, priority, routine)
#define IGA_EXIT_TRC1(ddf, module, priority, routine, p1)       \
        IGA_EXIT_DBG1(ddf, module, priority, routine, p1)
#define IGA_EXIT_TRC(ddf, module, priority,                     \
                routine, p1, p2, p3, p4, p5)                    \
        IGA_EXIT_DBG(ddf, module, priority,                     \
                routine, p1, p2, p3, p4, p5)

#define IGA_PARM_TRC0(ddf, module, priority, routine, subfunc)  \
        IGA_PARM_DBG0(ddf, module, priority, routine, subfunc)
#define IGA_PARM_TRC1(ddf, module, priority, routine,           \
                subfunc, p1)                                    \
        IGA_PARM_DBG1(ddf, module, priority, routine,           \
                subfunc, p1)
#define IGA_PARM_TRC(ddf, module, priority, routine,            \
                subfunc, p1, p2, p3, p4, p5)                    \
        IGA_PARM_DBG(ddf, module, priority, routine,            \
                subfunc, p1, p2, p3, p4, p5)

#define IGA_ENTER_MTRC0(ddf, module, priority, routine)         \
        IGA_ENTER_DBG0(ddf, module, priority, routine)
#define IGA_ENTER_MTRC1(ddf, module, priority, routine, p1)     \
        IGA_ENTER_DBG1(ddf, module, priority, routine, p1)
#define IGA_ENTER_MTRC(ddf, module, priority,                   \
                routine, p1, p2, p3, p4, p5)                    \
        IGA_ENTER_DBG(ddf, module, priority,                    \
                routine, p1, p2, p3, p4, p5)

#define IGA_EXIT_MTRC0(ddf, module, priority, routine)          \
        IGA_EXIT_DBG0(ddf, module, priority, routine)
#define IGA_EXIT_MTRC1(ddf, module, priority, routine, p1)      \
        IGA_EXIT_DBG1(ddf, module, priority, routine, p1)
#define IGA_EXIT_MTRC(ddf, module, priority,                    \
                routine, p1, p2, p3, p4, p5)                    \
        IGA_EXIT_DBG(ddf, module, priority,                     \
                routine, p1, p2, p3, p4, p5)

#define IGA_PARM_MTRC0(ddf, module, priority, routine, subfunc) \
        IGA_PARM_DBG0(ddf, module, priority, routine, subfunc)
#define IGA_PARM_MTRC1(ddf, module, priority, routine,          \
                subfunc, p1)                                    \
        IGA_PARM_DBG1(ddf, module, priority, routine,           \
                subfunc, p1)
#define IGA_PARM_MTRC(ddf, module, priority, routine,           \
                subfunc, p1, p2, p3, p4, p5)                    \
        IGA_PARM_DBG(ddf, module, priority, routine,            \
                subfunc, p1, p2, p3, p4, p5)

#define IGA_ENTER_DBG0(ddf, module, priority, routine)          \
{                                                               \
        GS_TRACE_PRIORITY_TEST(module, priority){               \
            iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,       \
                routine, ENTER), 0, 0, 0);                      \
        }                                                       \
        GS_ENTER_DBG0(HKWD_GS_IGA_DD, module,                   \
                priority, routine);                             \
}

#define IGA_ENTER_DBG1(ddf, module, priority, routine, p1)      \
{                                                               \
        GS_TRACE_PRIORITY_TEST(module, priority){               \
            iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,       \
                routine, ENTER), p1, 0, 0);                     \
        }                                                       \
        GS_ENTER_DBG1(HKWD_GS_IGA_DD, module,                   \
                priority, routine, p1);                         \
}

#define IGA_ENTER_DBG(ddf, module, priority, routine,           \
                 p1, p2, p3, p4, p5)                            \
{                                                               \
        GS_TRACE_PRIORITY_TEST(module, priority){               \
            iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,       \
                routine, ENTER), p1, p2, p3);                   \
        }                                                       \
        GS_ENTER_DBG(HKWD_GS_IGA_DD, module,                    \
                priority, routine, p1, p2, p3, p4, p5);         \
}

#define IGA_EXIT_DBG0(ddf, module, priority, routine)           \
{                                                               \
        GS_TRACE_PRIORITY_TEST(module, priority){               \
            iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,       \
                routine, EXIT), 0, 0, 0);                       \
        }                                                       \
        GS_EXIT_DBG0(HKWD_GS_IGA_DD, module,                    \
                priority, routine);                             \
}

#define IGA_EXIT_DBG1(ddf, module, priority, routine, p1)       \
{                                                               \
        GS_TRACE_PRIORITY_TEST(module, priority){               \
            iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,       \
                routine, EXIT), p1, 0, 0);                      \
        }                                                       \
        GS_EXIT_DBG1(HKWD_GS_IGA_DD, module,                    \
                priority, routine, p1);                         \
}

#define IGA_EXIT_DBG(ddf, module, priority, routine,            \
                p1, p2, p3, p4, p5)                             \
{                                                               \
        GS_TRACE_PRIORITY_TEST(module, priority){               \
            iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,       \
                routine, EXIT), p1, p2, p3);                    \
        }                                                       \
        GS_EXIT_DBG(HKWD_GS_IGA_DD, module,                     \
                priority, routine, p1, p2, p3, p4, p5);         \
}

#define IGA_PARM_DBG0(ddf, module, priority, routine, subfunc)  \
{                                                               \
        GS_TRACE_PRIORITY_TEST(module, priority){               \
            iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,       \
                routine, subfunc), 0, 0, 0);                    \
        }                                                       \
        GS_PARM_DBG0(HKWD_GS_IGA_DD, module,                    \
                priority, routine, subfunc);                    \
}

#define IGA_PARM_DBG1(ddf, module, priority, routine,           \
                subfunc, p1)                                    \
{                                                               \
        GS_TRACE_PRIORITY_TEST(module, priority){               \
            iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,       \
                routine, subfunc), p1, 0, 0);                   \
        }                                                       \
        GS_PARM_DBG1(HKWD_GS_IGA_DD, module,                    \
                priority, routine, subfunc, p1);                \
}

#define IGA_PARM_DBG(ddf, module, priority, routine,            \
                subfunc, p1, p2, p3, p4, p5)                    \
{                                                               \
        GS_TRACE_PRIORITY_TEST(module, priority){               \
            iga_memtrace(ddf, GS_TRC_HKWD(HKWD_GS_IGA_DD,       \
                routine, subfunc), p1, p2, p3);                 \
        }                                                       \
        GS_PARM_DBG(HKWD_GS_IGA_DD, module,                     \
                priority, routine, subfunc,                     \
                p1, p2, p3, p4, p5);                            \
}

#define IGA_DATA_DBG(ddf, module, priority, routine,            \
                subfunc, addr, len)                             \
        GS_DATA_DBG(HKWD_GS_IGA_DD, module, priority,           \
                routine, subfunc, addr, len)                    \

#endif /* GS_DEBUG_TRACE_SWITCH */


/* Define the routine ID's used by the IGA device driver */

                                                               /* ENTER EXIT */

#define hkwd_GS_IGA_DEFINE                      1               /* 0001 8001 */
#define hkwd_GS_IGA_ENABLE                      2               /* 0002 8002 */
#define hkwd_GS_IGA_OPEN                       	3               /* 0003 8003 */
#define hkwd_GS_IGA_CLOSE                       4               /* 0004 8004 */
#define hkwd_GS_IGA_INIT                        5               /* 0005 8005 */
#define hkwd_GS_IGA_GET_MONITOR                 6               /* 0006 8006 */
#define hkwd_GS_IGA_LOOKUP_V                    7               /* 0007 8007 */
#define hkwd_GS_IGA_MAKE_GP                     8               /* 0008 8008 */
#define hkwd_GS_IGA_UNMAKE_GP                   9               /* 0009 8009 */
#define hkwd_GS_IGA_LOAD_FONT                   10              /* 000A 800A */
#define hkwd_GS_IGA_LOAD_PALETTE                11              /* 000B 800B */
#define hkwd_GS_IGA_VTTACT                      12              /* 000C 800C */
#define hkwd_GS_IGA_VTTCFL                      13              /* 000D 800D */
#define hkwd_GS_IGA_VTTCLR                      14              /* 000E 800E */
#define hkwd_GS_IGA_VTTCPL                      15              /* 000F 800F */
#define hkwd_GS_IGA_VTTDACT                     16              /* 0010 8010 */
#define hkwd_GS_IGA_VTTINIT                     17              /* 0011 8011 */
#define hkwd_GS_IGA_VTTRDS                      18              /* 0012 8012 */
#define hkwd_GS_IGA_VTTTERM                     19              /* 0013 8013 */
#define hkwd_GS_IGA_VTTTEXT                     20              /* 0014 8014 */
#define hkwd_GS_IGA_VTTDEFC                     21              /* 0015 8015 */
#define hkwd_GS_IGA_VTTMOVC                     22              /* 0016 8016 */
#define hkwd_GS_IGA_VTTSCR                      23              /* 0017 8017 */
#define hkwd_GS_IGA_VTTSETM                     24              /* 0018 8018 */
#define hkwd_GS_IGA_VTTSTCT                     25              /* 0019 8019 */
#define hkwd_GS_IGA_COPY_PS                     26              /* 001A 801A */
#define hkwd_GS_IGA_RESET           		27              /* 001B 801B */
#define hkwd_GS_IGA_VTTDPM			28              /* 001C 801C */
#define hkwd_GS_IGA_PM				29              /* 001D 801D */
#define hkwd_GS_IGA_SHUTDOWN			30              /* 001E 801E */
#define hkwd_GS_IGA_RECONFIG			31              /* 001F 801F */

                                                                /* HOOK */

/* Define the subfunc ID's used by the IGA_ENABLE routine */
#define hkwd_GS_IGA_FAILED_RD_PCI_CFG_REGS	1              	/* 0402 */
#define hkwd_GS_IGA_UNKNOWN_VENDOR_ID		2              	/* 0802 */
#define hkwd_GS_IGA_FAILED_WR_PCI_CFG_REGS	3              	/* 0C02 */

/* Define the subfunc ID's used by the IGA_CLOSE routine */
#define hkwd_GS_IGA_DISPLAY_NOT_OPEN		1		/* 0404 */
#define hkwd_GS_IGA_DISPLAY_IN_USE		2		/* 0804 */

/* Define the subfunc ID's used by the IGA_INIT routine */
#define hkwd_GS_IGA_FLAT_PANNEL			1              	/* 0405 */
#define hkwd_GS_IGA_INVAL_DSP_MODE		2              	/* 0805 */

/* Define the subfunc ID's used by the IGA_GET_MONITOR routine */
#define hkwd_GS_IGA_MONITOR_ID			1              	/* 0406 */

/* Define the subfunc ID's used by the IGA_LOOKUP_V routine */
#define hkwd_GS_IGA_LOOKUP_V_FOUND_MODE		1              	/* 0407 */
#define hkwd_GS_IGA_LOOKUP_V_NOT_FOUND_MODE	2              	/* 0807 */



/* Define the subfunc ID's used by the IGA_MAKE_GP routine */
#define hkwd_GS_IGA_IO_ATTACH_FAILED		1		/* 0408 */

/* Define the subfunc ID's used by the IGA_UNMAKE_GP routine */
#define hkwd_GS_IGA_IO_DETACH			1		/* 0409 */
#define hkwd_GS_IGA_MEM_DETACH			2		/* 0809 */

/* Define the subfunc ID's used by the IGA_RESET routine */
#define hkwd_GS_IGA_UNLOCK_S3			1		/* 041B */
#define hkwd_GS_IGA_DEV_TYPE			2		/* 081B */
#define hkwd_GS_IGA_INIT_EXT_REGS_0		3		/* 0C1B */
#define hkwd_GS_IGA_FB_SIZE			4		/* 101B */
#define hkwd_GS_IGA_INIT_EXT_REGS_1		5		/* 141B */
#define hkwd_GS_IGA_INIT_ATTR			6		/* 181B */
#define hkwd_GS_IGA_INIT_CRTC			7		/* 1C1B */
#define hkwd_GS_IGA_INIT_ECRTC			8		/* 201B */
#define hkwd_GS_IGA_INIT_SDAC			9		/* 241B */
#define hkwd_GS_IGA_UNKNOWN_DEVICE		10		/* 281B */
#define hkwd_GS_IGA_CLEAR_VRAM			11		/* 2C1B */

/* Define the subfunc ID's used by the IGA_VTTACT routine */
#define hkwd_GS_IGA_VTTACT_KSR_MODE             1       	/* 040C */

/* Define the subfunc ID's used by the IGA_VTTCFL routine */
#define hkwd_GS_IGA_VTTCFL_UPDATE_FB            1       	/* 040D */
#define hkwd_GS_IGA_VTTCFL_CALL_MOVC            2       	/* 080D */

/* Define the subfunc ID's used by the IGA_VTTCLR routine */
#define hkwd_GS_IGA_VTTCLR_UPDATE_FB            1       	/* 040E */
#define hkwd_GS_IGA_VTTCLR_CALL_MOVC            2       	/* 040E */

/* Define the subfunc ID's used by the IGA_VTTCPL routine */
#define hkwd_GS_IGA_VTTCPL_UPDATE_FB             1       	/* 040F */
#define hkwd_GS_IGA_VTTCPL_CALL_MOVC             2       	/* 080F */

/* Define the subfunc ID's used by the IGA_VTTTEXT routine */
#define hkwd_GS_IGA_VTTTEXT_CALL_DRAW_TEXT            1       	/* 0414 */
#define hkwd_GS_IGA_VTTTEXT_CALL_MOVC                 2       	/* 0814 */

/* Define the subfunc ID's used by the IGA_VTTDEFC routine */
#define hkwd_GS_IGA_VTTDEFC_CHANGE_CURSOR        1       	/* 0415 */
#define hkwd_GS_IGA_VTTDEFC_CALL_MOVC            2       	/* 0815 */

/* Define the subfunc ID's used by the IGA_VTTMOVC routine */
#define hkwd_GS_IGA_VTTMOVC_CALL_DRAW_CHAR              1       /* 0416 */

/* Define the subfunc ID's used by the IGA_VTTSCR routine */
#define hkwd_GS_IGA_VTTSCR_CLR_WHOLE_FB              1      	/* 0417 */
#define hkwd_GS_IGA_VTTSCR_CLR_PART_FB_FOR_SCRL_UP   2       	/* 0817 */
#define hkwd_GS_IGA_VTTSCR_CLR_PART_FB_FOR_SCRL_DOWN 3       	/* 0C17 */
#define hkwd_GS_IGA_VTTSCR_CALL_MOVC                 4       	/* 1017 */

/* Define the subfunc ID's used by the IGA_VTTSETM routine */
#define hkwd_GS_IGA_VTTSETM_CALL_VTTACT              1       	/* 0418 */

/* Define the subfunc ID's used by the IGA_SHUTDOWN routine */
#define hkwd_GS_IGA_WAKEUP_FROM_E_SLEEP       	     1       	/* 041E */
#define hkwd_GS_IGA_IO_ACTIVE       	     	     2       	/* 081E */
#define hkwd_GS_IGA_TURN_OFF_DEV       	     	     3       	/* 0C1E */

/* Define the subfunc ID's used by the IGA_RECONFIG routine */
#define hkwd_GS_IGA_ENABLE_FAILED       	     1       	/* 041F */
#define hkwd_GS_IGA_RESET_FAILED       	             2       	/* 081F */


#endif /* _H_IGATRC */
