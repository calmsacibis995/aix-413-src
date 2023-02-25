* /etc/trcfmt
# @(#)58  1.2 src/packages/devices/mca/8fd4/rte/root/devices.mca.8fd4.rte.trc.S, pkgdevbase, pkg411, GOLD410  1/25/94  17:27:05
#
# COMPONENT_NAME: (SYSXLZ1) Compression Adapter - Makefile
#
# FUNCTIONS: N/A
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Data Compression Device format
+ 231 1.0 "HLZ1DD" \
	{{ $loc = X1 }} \
	$loc, \
	0x01 { "entry_config:" X1 \
		"op:" X4 \
		} , \
	0x02 { "exit_config:" "errno:" X1 } , \
	0x03 { "entry_ioctl:" \
		"op:" X4 "flag:" X4 "chan:" X4 "ext:" X4 \
		} , \
	0x04 { "exit_ioctl:" "errno:" X1 } , \
	0x05 { "entry_hlz1:" X1 \
		"inbuf:" X4 "inlen: "X4 "outbuf:" X4 "outlen:" X4 \
		"flags:" X4 } , \
	0x06 { "exit_hlz1:" "return code:" X1 } , \
	0x07 { "entry_intr:" X1 } , \
	0x08 { "exit_intr:" "return code:" X1 } , \
	0x09 { "entry_startio:" X1 }, \
	0x0a { "exit_startio:" "return code:" X1 }, \
	0x0b { "entry_term:" X1 }, \
	0x0c { "exit_term:" "return code:" X1 }, \
	0x20 { "debug:" " var1:" X1 " var2:" X1 }










