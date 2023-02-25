# @(#)49	1.2  src/packages/bos/net/nfs/client/root/bos.net.nfs.client.trc.S, pkgnfs, pkg411, 9434A411a 8/18/94 20:10:11
#
# FUNCTIONS: template file for nfs trcrpt
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# LEVEL 1, 5 Years Bull Confidential Information
# 
* /etc/trcfmt
#
# FLIH and return from FLIH.
# RESUME serves as a return from FLIH.
# Level 5 flihs call the 'flih()' builtin to advance the Interrupt Depth.
# It the process that is resumed is an interrupt process, $procflg will
#   be non-zero and the mst will be the mst for the interrupt process.
#   The mst is a state save area for a process or interrupt.
# The "wait" process is written out as "IDLE".
# RESUME calls the resume() builtin to decrement the Interrupt Depth.
#
+211 1.0 "@NFS: Client VNOP read/write routines" \
        $HD%D1, \
        1  { "NFS_READ       " }, \
        2  { "NFS_WRITE      " }, \
        \* { `NFS_VOPSRW mode=$HD` $BREAK } \
        vnodetofilename($D1) `count=0x$D3  offset=0x$D4  sid=$D2`

#
# Client side VNOP routines
#
+212 1.0 "@NFS: Client VNOP routines" \
        HT & 0x7, \
        0 { \
                $D1, \
                2  { "NFS_LOOKUP     " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                9  { "NFS_CREATE     " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                10 { "NFS_REMOVE     " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                11 { "NFS_LINK       " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                0x000C { "NFS_RENAME  from: " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                0x100C { "NFS_RENAME  to:   " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                13 { "NFS_MKDIR      " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                14 { "NFS_RMDIR      " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                100010 { "NFS_SYMLINK from: " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                0x1010 { "NFS_SYMLINK to:   " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                \* { `NFS_VOPS genmode=$D1` } \
                $BREAK \
        } \
        $HD%D1, \
        1  { "NFS_SELECT     " vnodetofilename($D1) }, \
        2  { "NFS_LOOKUP     " \
		$D2, \
		0  { `file not found` }, \
		\* { lookuppninstall2($D2) vnodetofilename($D2) } },\
        3  { "NFS_OPEN       " vnodetofilename($D1) }, \
        4  { "NFS_CLOSE      " vnodetofilename($D1) }, \
        5  { "NFS_IOCTL      " vnodetofilename($D1) }, \
        6  { "NFS_GETATTR    " vnodetofilename($D1) }, \
        7  { "NFS_SETATTR    " vnodetofilename($D1) }, \
        8  { "NFS_ACCESS     " vnodetofilename($D1) }, \
        9  { "NFS_CREATE     " lookuppninstall2($D2) vnodetofilename($D2) }, \
        10 { "NFS_REMOVE     " vnodetofilename($D1) }, \
        11 { "NFS_LINK       " vnodetofilename($D2) }, \
        12 { "NFS_RENAME     " vnodetofilename($D1) }, \
        13 { "NFS_MKDIR      " vnodetofilename($D2) }, \
        14 { "NFS_RMDIR      " vnodetofilename($D1) }, \
        15 { "NFS_READDIR    " vnodetofilename($D1) }, \
        16 { "NFS_SYMLINK    " vnodetofilename($D1) }, \
        17 { "NFS_READLINK   " vnodetofilename($D1) }, \
        18 { "NFS_FSYNC      " vnodetofilename($D1) }, \
        19 { "NFS_INACTIVE   " vnodetofilename($D1) }, \
        20 { "NFS_BMAP       " vnodetofilename($D1) }, \
        21 { "NFS_BADOP" }, \
        22 { "NFS_STRATEGY   " vnodetofilename($D1) }, \
	23 { "NFS_LOCKCTL    " vnodetofilename($D1) locktype=$D2%X4 start=$D3 \
				len=$D4 pid=$D5%D4}, \
        24 { "NFS_NOOP" }, \
        26 { "NFS_CMP        " vnodetofilename($D1) }, \
        \* { `NFS_VOPS mode=$HD` }

#
# Server services
#
+213 1.0 "@NFS: Server read/write services" \
{{ \
nfsseqno(seqno) \
`seqno=$seqno%D1` \
}} \
        $HD%D1, \
        1  { "RFS_READ       " }, \
        2  { "RFS_WRITE      " }, \
        \* { `NFS_RFS mode=$HD` $BREAK } \
        nfsseqno($D1) vnodetofilename($D2) $D2 `count=0x$D3  offset=0x$D4`

+214 1.0 "@NFS: Server services" \
        HT & 0x07, \
        0 { \
                $D1, \
                5      { "RFS_LOOKUP     " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                8      { "RFS_CREATE     " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                9      { "RFS_REMOVE     " G8 $HD%S1 }, \
                0x000A { "RFS_RENAME  from: " G8 $HD%S1 }, \
                0x100A { "RFS_RENAME  to:   " G8 $HD%S1 G8 lookuppninstall1($HD%S1) }, \
                11     { "RFS_LINK       " G8 $HD%S1 }, \
                0x000C { "RFS_SYMLINK from: " G8 $HD%S1 }, \
                0x100C { "RFS_SYMLINK to:   " G8 $HD%S1 }, \
                13     { "RFS_MKDIR      " G8 $HD%S1 }, \
                14     { "RFS_RMDIR      " G8 $HD%S1 }, \
                \*     { `RFS_VOPS genmode=$D1` } \
                $BREAK \
        } \
        $HD%D1, \
        1  { "RFS_NULL       " nfsseqno($D1) }, \
        2  { "RFS_GETATTR    " nfsseqno($D1) vnodetofilename($D2) vnode=$D2 }, \
        3  { "RFS_SETATTR    " nfsseqno($D1) vnodetofilename($D2) vnode=$D2 }, \
        4  { "RFS_ERROR" }, \
        5  { "RFS_LOOKUP     " nfsseqno($D1) directory= vnodetofilename($D2) vnode=$D2 }, \
        133 { "RFS_LOOKUP     " nfsseqno($D1) lookuppninstall2($D2) object=vnodetofilename($D2) vnode=$D2 \
		error=$D3%E1 }, \
        6  { "RFS_READLINK   " nfsseqno($D1) vnodetofilename($D2) }, \
        8  { "RFS_CREATE     " nfsseqno($D1) directory=vnodetofilename($D2) }, \
        136  { "RFS_CREATE     " lookuppninstall2($D1) file=vnodetofilename($D1) error=$D2%E1 vnode=$D1 }, \
        9  { "RFS_REMOVE     " nfsseqno($D1) vnodetofilename($D2) }, \
        10 { "RFS_RENAME     " nfsseqno($D1) from dir=vnodetofilename($D2) \
                to dir=vnodetofilename($D3) }, \
        138 { "RFS_RENAME     " from file=vnodetofilename($D1) lookuppninstall2($D2) \
                to file=vnodetofilename($D2) }, \
        11 { "RFS_LINK       " nfsseqno($D1) vnodetofilename($D2) \
                vnodetofilename($D3) }, \
        12 { "RFS_SYMLINK    " nfsseqno($D1) vnodetofilename($D2) }, \
        13 { "RFS_MKDIR      " nfsseqno($D1) vnodetofilename($D2) }, \
        14 { "RFS_RMDIR      " nfsseqno($D1) vnodetofilename($D2) }, \
        15 { "RFS_READDIR    " nfsseqno($D1) vnodetofilename($D2) }, \
        16 { "RFS_STATFS     " nfsseqno($D1) vnodetofilename($D2) }, \
	30 { "NFS_LOCKFH     " vnodetofilename($D1) locktype=$D2%X4 \
		start=$D3 len=$D4 pid=$D5%D4}, \
        \* { "UNKNOWN HOOK ID " }

+216 1.0 "@NFS: Client call" \
        $HD%D1, \
        1  { \
                "NFS_CALL_ENTRY " nfsseqno($D1) server=sinaddr($D2) \
                starttimer(0x216,$D1) \
        }, \
        2  { \
                "NFS_CALL_EXIT  " nfsseqno($D1) server=sinaddr($D2) \
                endtimer(0x216,$D1) \
        }, \
        \* { `NFS CALL mode=$HD` }

+215 1.0 "@NFS: Server dispatch" \
{{ \
nfsdispcode(code) \
$code, \
        0  { NULL }, \
        1  { GETATTR }, \
        2  { SETATTR }, \
        4  { LOOKUP }, \
        5  { READLINK }, \
        6  { READ }, \
        8  { WRITE }, \
        9  { CREATE }, \
        10 { REMOVE }, \
        11 { RENAME }, \
        12 { LINK }, \
        13 { SYMLINK }, \
        14 { MKDIR }, \
        15 { RMDIR }, \
        16 { READDIR }, \
        17 { STATFS }, \
        \* { D3%D1 } \
}} \
        $HD%D1, \
        1  { "RFS_DISP_ENTRY " nfsseqno($D1) client=sinaddr($D2) \
                starttimer(0x215,$D1) \
        }, \
        2  { "RFS_DISP_EXIT  " nfsseqno($D1) client=sinaddr($D2) nfsdispcode($D3) \
                endtimer(0x215,$D1) \
        }, \
        \* { `RFS_DISPATCH mode=$HD` }

+218 1.0 "@NFS: rpc.lockd hooks" \
	$HD%D1, \
	1  { "LOCKD_KLM_PROG    " proc=$D1%D4 pid=$D2%D4 cookie=$D3%D4 port=$D4 }, \
	2  { "LOCKD_NLM_REQUEST " proc=$D1%D4 from sinaddr($D2) cookie=$D3%D4 pid=$D4%D4 }, \
	3  { "LOCKD_NLM_RESULTS " proc=$D1%D4 from sinaddr($D2) cookie=$D3%D4 result=$D4 }, \
	4  { "LOCKD_KLM_REPLY   " proc=$D1%D4 stat=$D2 port=$D3 }, \
	5  { "LOCKD_NLM_REPLY   " proc=$D1%D4 to sinaddr($D2) stat=$D3 cookie=$D4%D4 }, \
	6  { "LOCKD_NLM_CALL    " proc=$D1%D4 cookie=$D2%D4 pid=$D3%D4 retransmit=$D4 }, \
	7  { "LOCKD_CALL_UDP    " to sinaddr($D4) proc=$D1%D4 program=$D2%D4 version=$D3%D4 }, \
	\* { `LOCKD CALL mode=$HD` }

