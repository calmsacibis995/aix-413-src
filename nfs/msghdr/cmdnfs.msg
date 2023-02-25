$ @(#)58	1.37.2.4  src/nfs/msghdr/cmdnfs.msg, cmdnfs, nfs41J, 9520A_all 5/14/95 18:43:26
$
$  COMPONENT_NAME: (CMDNFS) commands that deal with NFS/NIS
$
$  FUNCTIONS: cmdnfs messages
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1991,1993
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$quote "
$ There will be a message set for each command
$ It is assumed that the command name before the colon will NOT be translated.
$ It is assumed that any words that are all capitalized will NOT be translated.

$ All messages that prompt the user for a response, should not have the
$ response choices, nor the response, translated. We restrict these responses
$ to the 7-bit ASCII code representations.

$set BIOD
BIOD1 "usage: biod [<count>]\n"
BIOD2 "async_daemon "
BIOD3 "cannot fork "

$set GFSINS
GFS1 "usage: gfsinstall [-a pathname | -d -s]\n"
GFS2 "usage: gfsinstall -a pathname\n"
GFS3 "gfsinstall: 1831-001 could not unload extension\n"
$ /dev/kmem should not be translated
GFS4 "gfsinstall: 1831-002 cannot open /dev/kmem\n"
GFS5 "SYS_KLOAD"
GFS6 "SYS_CFGKMOD"
GFS7 "SYS_KULOAD"
GFS8 "lseek"
GFS9 "read"
GFS10 "load_nfs(%1$d,%2$d): 1831-544  could not load nfs kernel extensions\n"


$set NFSD
NFSD1 "usage: nfsd [servers]\n"
NFSD2 "nfsd: 1831-003 server already active\n"
NFSD3 "nfsd: 1831-004 cannot fork" 

$set NFSMNTHLP
NMH1 "Usage:\n\
\t    nfsmnthelp B debug\n\
\tor: nfsmnthelp M debug host object stub options\n\
\tor: nfsmnthelp Q debug host\n\
\tor: nfsmnthelp U debug host object vfsno forced\n"
NMH2 "nfsmnthelp: 1831-005 Invalid number of arguments: %1$d\n"
NMH3 "nfsmnthelp: 1831-006 NFS mounts/unmounts are only allowed by\n\
\tsuperuser and by members of the system group\n"
NMH4 "nfsmnthelp: 1831-007 Invalid command argument: %1$s\n"
NMH5 "mount: backgrounding\n%1$s:%2$s\n"
NMH6 "mount: retrying\n%1$s:%2$s\n"
NMH7 "mount: 1831-008 giving up on:\n%1$s:%2$s\n"
NMH8 "mount: 1831-009 %1$s not in hosts database\n"
NMH9 "mount: 1831-010 server %1$s not responding"
NMH10 "mount: 1831-011 access denied for %1$s:%2$s\n"
NMH11 "mount: %1$s server ok\n"

$ rsize should not be translated
NMH12 "mount: 1831-012 invalid rsize specified, ignored\n"
$ wsize should not be translated
NMH13 "mount: 1831-013 invalid wsize specified, ignored\n"
$ acregmin and actimeo should not be translated 
NMH14 "mount: warning acregmin overridden by actimeo\n\
\tspecification\n"
$ acregmax and actimeo should not be translated
NMH15 "mount: warning acregmax overridden by actimeo\n\
\tspecification\n"
$ acdirmin and actimeo should not be translated
NMH16 "mount: warning acdirmin overridden by actimeo\n\
\tspecification\n"
$ acdirmax and actimeo should not be translated
NMH17 "mount: warning acdirmax overridden by actimeo\n\
\tspecification\n"

NMH18 "nfsmnthelp: 1831-014 Remote query operation not supported for NFS\n"
NMH19 "umount: 1831-015 %1$d error while unmounting %2$s:%3$s - \n"
NMH20 "nfsmnthelp: 1831-016 %1$s not in hosts database\n"
NMH21 "nfsmnthelp: 1831-017 Warning: umount: cannot do local bind.\n"
NMH22 "nfsmnthelp: 1831-018 Duplicate option %1$s, second specification\n\
\tignored\n"
NMH23 "nfsmnthelp: 1831-019 "
NMH24 "mount: fork for NFS broadcast"
NMH25 "Warning: unmount:"

$set PORTMAP
$ the udp_... words should not be translated
PM1 "portmap: 1831-020 could not do udp_create\n"
PM2 "portmap: 1831-021 could not do tcp_create\n"
$ the words with svc in them should not be translated
PM3 "portmap: 1831-022 run_svc returned unexpectedly\n"
PM4 "portmap: svc_sendreply\n"
PM5 "CALLIT: 1831-023 cannot fork.\n"
PM6 "portmap: 1831-024 cannot fork"
PM7 "portmap: 1831-025 cannot create socket"
PM8 "portmap: 1831-026 cannot bind"

$set YPBIND

YPB1 "usage: ypbind [ -s ]\n"
YPB2 "ypbind: running secure\n"
YPB3 "ypbind: 1831-027 Cannot catch broadcast process exit signal.\n"
YPB4 "ypbind: 1831-028 Cannot create socket for pinging.\n"
YPB5 "ypbind: 1831-029 Cannot create tcp service.\n"
YPB6 "ypbind: 1831-030 Cannot register tcp service.\n"
YPB7 "ypbind: 1831-031 Cannot create udp service.\n"
YPB8 "ypbind: 1831-032 Cannot register udp service.\n"
YPB9 "ypbind: 1831-033 Bad fds bits in main loop select mask.\n"
YPB10 "ypbind: 1831-034 Invalid timeout in main loop select.\n"
YPB11 "ypbind: 1831-035 Cannot reply to rpc call.\n"
YPB12 "ypbind: 1831-036 Cannot respond to rpc request.\n"
YPB13 "ypbind: 1831-037 ypbind_get_binding cannot free args.\n"
YPB14 "NIS: server for domain \"%1$s\" OK"
YPB15 "NIS: 1831-038 server not responding for domain \"%1$s\";\n\
\tstill trying"
YPB16 "ypbind: 1831-039 broadcaster fork failure.\n"
YPB17 "ypbind: 1831-040 Set domain request to host %1$s,\n\
\tfrom host %2$s, failed (bad port).\n"
YPB18 "ypbind: 1831-041 Set domain request to host %1$s,\n\
\tfrom host %2$s, failed (not root).\n" 
YPB19 "ypbind: 1831-042 Set domain request to host %1$s,\n\
\tfrom host %2$s, failed (credentials).\n" 
YPB20 "ypbind:1831-043 cannot fork"
YPB21 "ypbind: -ypset: allowing ypset! (this is insecure)\n" 
YPB22 "ypbind -ypsetme: allowing local ypset! (this is insecure)\n"
YPB23 "ypbind: Can't ignore pipe exit signal.\n"
YPB24 "xdr_ypbind_setdom fails.\n"
YPB25 "ypbind: Internal error -- no broadcaster pipe.\n"
YPB26 "ypbind: broadcaster fork failure.\n"
YPB27 "ypbind: Broadcaster pipe failure.\n"
YPB28 "ypbind: Set domain request to host %s, \n\
\tfrom host %s, failed (ypset not allowed)!\n"
YPB29 "ypbind: Set domain request to host %s, \n\
\tfrom host %s, failed (not local)!\n"
YPB30 "%s: Permission denied.  Must have root authority.\n"

$set NFS_CLN
NFSC1 "nfs_clean: Stopping NFS/NIS Daemons\n"
NFSC2 "Usage: nfs.clean [-d][-y]\n"

$set RC_NFS
RCNFS1 "Starting NFS services:\n"
RCNFS2 "Starting NIS services:\n"

$set RPC_LOCKD
$ Replace this message with LCKD81
LCKD1 "rpc.lockd -t[timeout] -g[grace_period] -d[debug]\n"
LCKD2 "rpc.lockd: 1831-044 release_le: add_mon failed\n");
LCKD3 "rpc.lockd: 1831-045 len(%1$d) greater than max len(%2$d)\n"
LCKD4 "rpc.lockd: 1831-046 kill process (%1$d)\n"
LCKD5 "rpc.lockd: 1831-047 add_reclock: insrtp = a = %1$x \n\
\tshould not happen!!!\n"
LCKD6 "rpc.lockd: 1831-048 add_mon = -1 in delete, should not happen\n"
LCKD7 "rpc.lockd: 1831-049 impossible!\n"
LCKD8 "rpc.lockd: 1831-050 no lock available\n"
LCKD9 "rpc.lockd: 1831-051 wakeup(%1$x) cannot take place\n\
\tdue to add_call malloc error\n"
LCKD10 "rpc.lockd: 1831-052 unknown contact monitor (%1$d)\n"
LCKD11 "contact_mon: 1831-053 problem with private data size (%1$d)\n\
\tto status monitor\n"
LCKD12 "rpc.lockd: 1831-054 site %1$s does not subscribe to\n\
\tstatus monitor service \n"
LCKD13 "rpc.lockd: 1831-055 cannot contact local statd\n"
LCKD14 "rpc.lockd: 1831-056 cannot create tcp service.\n"
LCKD15 "rpc.lockd: 1831-057 unable to register\n\
\t(NLM_PROG, NLM_VERS, tcp).\n"
LCKD16 "rpc.lockd: 1831-058 cannot create udp service.\n"
LCKD17 "rpc.lockd: 1831-059 unable to register\n\
\t(NLM_PROG, NLM_VERS, udp).\n"
LCKD18 "rpc.lockd: 1831-060 cannot create tcp service.\n"
LCKD19 "rpc.lockd: 1831-061 unable to register\n\
\t(NLM_PROG, NLM_VERSX, tcp).\n"
LCKD20 "rpc.lockd: 1831-062 cannot create udp service.\n"
LCKD21 "rpc.lockd: 1831-063 unable to register\n\
\t(NLM_PROG, NLM_VERSX, udp).\n"
LCKD22 "rpc.lockd: 1831-064 unable to register\n\
\t(KLM_PROG, KLM_VERS, udp).\n"
LCKD23 "rpc.lockd: 1831-065 unable to register\n\
\t(KLM_PROG, KLM_VERS, tcp).\n"
LCKD24 "rpc.lockd: 1831-066 unable to register\n\
\t(PRIV_PROG, PRIV_VERS, tcp).\n"
LCKD25 "rpc.lockd: 1831-067 svc_run returned\n"
LCKD26 "rpc.lockd: 1831-068 process issues request %1$x \n\
\t(proc = %2$d) before obtaining response for %3$x\n"
LCKD27 "rpc.lockd: 1831-069 unknown klm_reply proc(%1$d)\n"
LCKD28 "rpc.lockd: 1831-070 unknown nlm_reply proc vaule: %1$d\n"
LCKD29 "rpc.lockd: 1831-071 inconsistent  lock reply exists, ignored \n" 
LCKD30 "rpc.lockd: 1831-072 %1$d not supported in nlm_call\n"
LCKD31 "rpc.lockd: 1831-073 unable to free arguments\n"
LCKD32 "rpc.lockd: 1831-074 blocked request (%1$x) cannot \n\
\tbe found in message queue\n"
LCKD33 "rpc.lockd: 1831-075 reclaim request (%1$x) cannot\n\
\tbe sent and cannot be queued for resend later!\n"
LCKD34 "rpc.lockd: 1831-076 could not add lock to local lock table\n"
LCKD35 "rpc.lockd: 1831-077 rpc not supported\n"
LCKD36 "rpc.lockd: 1831-078 cancel request\n\
\tpre_le=%1$x pre_fe = %2$x\n"
LCKD37 "rpc.lockd: 1831-079 lock_granted message discarded\n\
\tdue to lock table inconsistencies (unlock reply may be lost!)\n"
LCKD38 "rpc.lockd: 1831-080 msg(%1$x) is deleted from message\n\
\tqueue because reply cannot be created due to malloc problem\n"
LCKD39 "rpc.lockd: 1831-081 message no longer in msg_queue\n"
LCKD40 "rpc.lockd: 1831-082 local granted: rpc not supported\n"
LCKD41 "rpc.lockd: 1831-083 no locks available\n"
LCKD42 "rpc.lockd: 1831-084 message no longer in wait_queue,\n\
\tthis may be a retransmitted msg\n"
LCKD43 "rpc.lockd: 1831-085 cont_lock: remote and local lock \n\
\ttable inconsistencies\n"
LCKD44 "rpc.lockd: 1831-086 unknown lock return: %1$d\n"
LCKD45 "rpc.lockd: 1831-087 cannot find lock in table.\n"
LCKD46 "rpc.lockd: 1831-088 unkown rpc_unlock return: %1$d\n"
LCKD47 "rpc.lockd: 1831-089 cont_test: unknown return: %1$d\n"
LCKD48 "rpc.lockd: 1831-090 cont_cancel: remote and local lock tables\n\
are inconsistent\n"
LCKD49 "rpc.lockd: 1831-091 cont_cancel: cannot find blocked lock\n\
\trequest in msg queue! \n"
LCKD50 "rpc.lockd: 1831-092 remote and local lock tables are inconsistent\n"
LCKD51 "rpc.lockd: 1831-093 unexpected remote_cancel %1$d\n"
LCKD52 "rpc.lockd: 1831-094 cont_reclaim: blocked\n"
LCKD53 "rpc.lockd: 1831-095 reclaim blocked request already granted,\n\
\timpossible\n"
LCKD54 "rpc.lockd: 1831-096 reclaim lock(%1$x) fail!\n"
LCKD55 "rpc.lockd: 1831-097 reclaim block lock has been granted\n\
\twith reclaim rejected as nolocks\n"
LCKD56 "rpc.lockd: 1831-098 blocked reclaim request already granted\n\
\tlocally, impossible\n"
LCKD57 "rpc.lockd: 1831-099 reclaim lock req(%1$x) is returned\n\
\tdue to grace period, impossible\n"
LCKD58 "rpc.lockd: 1831-100 unknown cont_reclaim return: %1$d\n"
LCKD59 "rpc.lockd: 1831-101 File sharing request from %1$s \n\
\t(mode %2$d, access %3$d) conflicts with sharing for %4$s \n\
\t(mode %5$d, access %6$d)\n"
LCKD60 "rpc.lockd: 1831-102 cannot malloc memory\n"
LCKD61 "rpc.lockd: 1831-103 malloc error, loopback wakeup\n"
LCKD62 "rpc.lockd: 1831-104 msg no longer in msg_queue\n"
LCKD63 "stat_mon: 1831-105 len(=%1$d) is greater than 16!\n"
LCKD64 "rpc.lockd: 1831-106 stat_mon proc(%1$d) not supported\n"
LCKD65 "rpc.lockd: 1831-107 Cannot contact status monitor!\n"
LCKD66 "rpc.lockd: 1831-108 cannot send because socket\n\
\tcould not be created.\n"
LCKD67 "rpc.lockd: 1831-109 udp cannot send due to out of cache\n"
LCKD68 "***** granted reclocks *****\n"
LCKD69 "***** message queue *****\n"
LCKD70 "***** no entry in message queue *****\n"
LCKD71 "***** blocked reclocks *****\n"
LCKD72 "***** no blocked reclocks *****\n"
LCKD73 "***** monitor queue on (%s, %d) *****\n"
LCKD74 "rpc.lockd: 1831-110  request discarded due to a status\n\
\tmonitor problem\n"
LCKD75 "rpc.lockd: 1831-111 killed upon request\n"
LCKD76 "rpc.lockd: 1831-112 svcudp_enablecache failed\n"
LCKD77 "malloc"
LCKD78 "rpc.lockd: cannot fork"
LCKD79 "logfile fopen:"
LCKD80 "rpc.lockd: 1831-528 hostname is not set\n"
LCKD81 "rpc.lockd -t[timeout] -g[grace_period] -d[debug] -X[aiXtensions] -b\n"
$set RPC_STATD
STATD1 "rpc.statd -d[debug] -D\n"
STATD2 "rpc.statd: 1831-113 cannot contact local lockd\n\
\ton %1$s status change, give up!\n"
STATD3 "rpc.statd: 1831-114 fopen(stat file) error\n"
STATD4 "rpc.statd: 1831-115 cannot talk to statd at %1$s\n"
STATD5 "rpc.statd: 1831-117 unable to free arguments\n"
STATD6 "rpc.statd: 1831-118 cannot create udp service.\n"
STATD7 "rpc.statd: 1831-119 unable to register\n\
\t(SM_PROG, SM_VERS, udp).\n"
STATD8 "rpc.statd: 1831-120 cannot create tcp service.\n"
STATD9 "rpc.statd: 1831-121 unable to register\n\
\t(SM_PROG, SM_VERS, tcp).\n"
STATD10 "rpc.statd: 1831-122 svc_run returned\n"
STATD11 "rpc.statd: 1831-123 open of \n"
STATD12 "rpc.statd: 1831-124 unlink of "
STATD13 "rpc.statd: 1831-125 insert_mon: malloc"
STATD14 "rpc.statd: 1831-126 fseek failed"
STATD15 "rpc.statd: 1831-127 fsync failed"
STATD16 "rpc.statd: mkdir current"
STATD17 "rpc.statd: mkdir backup"
STATD18 "rpc.statd: open current directory"
STATD19 "rpc.statd: rename"
STATD20 "rpc.statd: open backup directory"
STATD21 "rpc.statd: malloc"
STATD22 "rpc.statd: close"
STATD23 "rpc.statd: 1831-128 cannot fork"

$set CHKEY
CHKY1 "usage: %1$s [-f]\n"
CHKY2 "chkey: Generating new key for %1$s.\n"
CHKY3 "chkey: 1831-129 No NIS password found: cannot change key.\n"
CHKY4 "chkey: 1831-130 No password found: cannot change key.\n"
CHKY5 "chkey: 1831-131 Invalid password.\n"
CHKY6 "chkey: 1831-132 cannot find master of publickey database\n"
CHKY7 "chkey: 1831-133 Password incorrect.\n"
CHKY8 "chkey: Sending key change request to %1$s...\n"
CHKY9 "chkey: 1831-134 %1$s: unable to update NIS database\n\
\t(%2$u): %3$s\n" 
CHKY10 "chkey: 1831-135 Perhaps %1$s is down?\n"
CHKY11 "Done.\n"
CHKY12 "chkey: 1831-136 Unable to login with new secret key.\n"

$set KEYLOGIN
KEYLOG1 "keylogin: 1831-137 Cannot find\n\%1$s's secret key\n"
KEYLOG2 "keylogin: 1831-138 Password incorrect for %1$s\n"
KEYLOG3 "keylogin: 1831-139 Could not set\n\%1$s's secret key\n"
KEYLOG4 "keylogin: 1831-140 Maybe the keyserver is down?\n"
KEYLOG5 "keylogin: Can't find secret key. Maybe NIS is not running\n"

$set YPCAT
YPCT1 "Usage:\n\
\typcat [-k] [-d domainname] [-t] mapname\n\
\typcat -x\n"
YPCT2 "ypcat: 1831-141 Cannot get %1$s back from system call.\n"
YPCT3 "ypcat: 1831-142 the %1$s has not been set on this machine.\n"
YPCT4 "ypcat: 1831-143 cannot bind to NIS server for domain %1$s.\n\
\tReason: %2$s.\n"
YPCT5 "ypcat: 1831-144  %1$s argument is bad.\n";
YPCT6 "ypcat: 1831-145 cannot get first record from NIS.\n\
\tReason:  %1$s.\n"
YPCT7 "ypcat: 1831-146 cannot get next record from NIS.\n\
\tReason:  %1$s.\n"
YPCT8 "Use \"%1$s\" for map \"%2$s\"\n"

$set YPMATCH
YPMCH1 "Usage:\n\
\typmatch [-d domain] [-t] [-k] key [key ...] mname\n\
\typmatch -x\n\
\twhere\n\
\t\tmname may be either a mapname or a nickname for a map\n\
\t\t-t inhibits map nickname translation\n\
\t\t-k prints keys as well as values.\n\
\t\t-x dumps the map nickname translation table.\n"
YPMCH2 "ypmatch:  1831-147 %1$s argument is bad.\n"
YPMCH3 "ypmatch:  1831-148 cannot get %1$s back from system call.\n"
YPMCH4 "ypmatch:  1831-149 the %1$s has not been set on this machine.\n"
YPMCH5 "ypmatch: 1831-150 Cannot match key %1$s in map \
%2$s.\n\tReason: %3$s.\n"
YPMCH6 "Use \"%1$s\" for map \"%2$s\"\n"

$set YPPASSWD
YPPW1 "yppasswd: 1831-151 cannot get domain\n"
YPPW2 "yppasswd: 1831-152 cannot get master for passwd file\n"
YPPW3 "yppasswd: 1831-153 %1$s is not running yppasswd daemon\n"
YPPW4 "yppasswd: 1831-154 yppasswd daemon is not running on\n\
\tprivileged port\n"
YPPW5 "yppasswd: 1831-155 could not change passwd\n"
YPPW6 "NIS passwd changed on %1$s\n"
YPPW7 "yppasswd: 1831-156 Warning: cannot find public key for %1$s.\n"
YPPW8 "yppasswd: 1831-157 Warning: could not reencrypt secret key\n\
\tfor %1$s\n"
YPPW9 "Secret key reencrypted for %1$s on %2$s\n"
YPPW10 "yppasswd: 1831-158 you don't have a login name\n"
YPPW11 "Changing NIS password for %1$s\n"
YPPW12 "yppasswd: 1831-159 Not in passwd file.\n"
YPPW13 "yppasswd: 1831-160 Permission denied.\n"
YPPW14 "yppasswd: 1831-161 Password unchanged.\n"
YPPW15 "yppasswd: 1831-162 Please use at least one non-numeric\n\
\tcharacter\n"
YPPW16 "yppasswd: 1831-163 Please use a longer password\n"
YPPW17 "yppasswd: 1831-164 Mismatch - password unchanged.\n"
YPPW18 "yppasswd: 1831-165 NIS login shell changed on %s\n"
YPPW19 "yppasswd: 1831-166 reason yp_match failed on %s is %d\n"
YPPW20 "yppasswd: 1831-167  Changing NIS shell for %s\n"
YPPW21 "yppasswd: 1831-168  Error attempting to revert to real GID\n"
YPPW22 "yppasswd: 1831-169  %s: cannot change shell and finger\
\tinformation at the same time\n"
YPPW23 "yppasswd: 1831-170 Old shell: %s\nNew shell: "
YPPW24 "yppasswd: 1831-171 Login shell unchanged.\n"
YPPW25 "yppasswd: 1831-172 Error setting shell for %s to %s\n\
\tValue is invalid.\n"
YPPW26 "yppasswd: 1831-173 Error getting shell for %s.\n"
YPPW27 "yppasswd: 1831-174 Check %s file.\n"
YPPW28 "yppasswd: 1831-175 Error changing shell for %s to %s\n\
\tValue is invalid.\n"
YPPW29 "yppasswd: 1831-176 usage: %s [-s [uname [shellprog]]] | [-f [uname]]\n"
YPPW30 "yppasswd: 1831-177 couldn't change NIS login shell\n"
YPPW31 "yppasswd: 1831-178 couldn't change NIS Gecos information\n"
YPPW32 "yppasswd: 1831-179 NIS Gecos information changed on %s\n"
YPPW33 "yppasswd: 1831-180 Changing NIS Gecos information for %s\n"
YPPW34 "yppasswd: 1831-181 %s's current gecos:\n\t\t%s\n"
YPPW35 "yppasswd: 1831-182 Change (%s) or (%s)? >"
YPPW36 "yppasswd: 1831-183 NIS Gecos information not changed.\n"
YPPW37 "yppasswd: 1831-184 Current available shells:\n"
YPPW38 "yppasswd: 1831-185 %s's current login shell:\n\t\t%s\n"
YPPW39 "yppasswd: 1831-186 To?> "
YPPW40 "Password:"
YPPW41 "Retype password:"
YPPW42 "Old NIS password:"
YPPW43 "New password:"
YPPW44 "Retype new password:"
$ The following are for NIS methods 
YPPW45 "1831-549  Your password has expired.\n\
\tPlease choose a new password.\n"

$set YPWHICH
YPWCH1 "Usage:\n\
\typwhich [-d domain] [[-t] -m [mname] | [-V1 | -V2] host]\n\
\typwhich -x\n"
YPWCH2 "ypwhich: 1831-165 %1$s argument is bad.\n"
YPWCH3 "ypwhich: 1831-166 cannot get %1$s back from system call.\n"
YPWCH4 "ypwhich: 1831-167 the %1$s has not been set on this machine.\n";
YPWCH5 "ypwhich: 1831-168 cannot get first record from NIS.\n\
\tReason:  %1$s.\n"
YPWCH6 "ypwhich: 1831-169 cannot get next record from NIS.\n\
\tReason:  %1$s.\n"
YPWCH7 "ypwhich: 1831-170 cannot find %1$s\n"
YPWCH8 "ypwhich: 1831-171 %1$s is not running ypbind\n" 
YPWCH9 "ypwhich: 1831-172 %1$s is not running port mapper\n"
YPWCH10 "ypwhich: 1831-173 cannot call ypbind on %1$s"
YPWCH11 "ypwhich: 1831-174 Cannot find the master of %1$s.\n\
\tReason: %2$s.\n"
YPWCH12 "Use \"%1$s\" for map \"%2$s\"\n"
YPWCH13 "dump_ypmaps: 1831-175 Cannot bind for domain %1$s.\n\
\tReason: %2$s\n"
YPWCH14 "ypwhich: 1831-176 Cannot get maplist.\n\
\tReason:  %1$s.\n"
YPWCH15 "ypwhich: 1831-177 Cannot find the master of %1$s.\n\
\tReason: %2$s.\n"
YPWCH16 "ypwhich: 1831-178 Domain %1$s not bound.\n"
YPWCH17 "ypwhich: 1831-179 clntudp_create error"  
YPWCH18 "ypwhich: 1831-180 (v2dumpmaps) cannot get maplist"

$set EXPORTFS
$ Replace this message with EXFS18
EXFS1 "usage: %1$s [-aiuv] [-o options] [dir] ...\n"
EXFS2 "exportfs: 1831-181 options ignored for unexport\n"
EXFS3 "exportfs: 1831-182 nothing exported\n"
EXFS4 "exportfs: 1831-183 must be superuser to unexport\n"
EXFS5 "exportfs: 1831-184 unexported %1$s\n"
EXFS6 "exportfs: 1831-185 Cannot open "
EXFS7 "exportfs: 1831-186 %1$s not found in %2$s\n"
EXFS8 "exportfs: 1831-187 re-exported %1$s\n"
EXFS9 "exportfs: 1831-188 unknown address type: %1$d\n"
EXFS10 "exportfs: 1831-189 %1$s: unknown host\n"
EXFS11 "exportfs: 1831-190 unknown option: %1$s\n"
EXFS12 "exportfs: 1831-191 %1$s allows %2$s access to too many \n\
\thosts (max %3$d)\n"
EXFS13 "exportfs: 1831-192 %1$s: sub-directory (%2$s) \n\
\talready exported\n"
EXFS14 "exportfs: 1831-193 %1$s: parent-directory (%2$s)\n\
\talready exported\n"
EXFS15 "exportfs: 1831-194 Out of memory\n"
EXFS16 "exportfs: 1831-195 must be superuser to export\n"
EXFS17 "Exported %1$s\n"
$ Use this message instead of EXFS1
EXFS18 "Usage: %1$s [-aiuv] [-o options] [directory]\n"
EXFS19 "exportfs: A non-numeric UID was used with the anon option.\n  A UID of 0 will be assumed\n"

$set NFSSTAT
NFSST1 "Usage: nfsstat [-cnrsz]\n"
NFSST2 "nfsstat: 1831-196 Must be root for z flag\n"
NFSST3 "nfsstat: 1831-197 kernel is not configured with the\n\
\tserver nfs and rpc code.\n"
NFSST4 "nfsstat: 1831-198 kernel is not configured with the\n\
\tclient nfs and rpc code.\n"
NFSST15 "\nClient rpc:\n"
NFSST16 "\nServer rpc:\n"
NFSST17 "\nClient nfs:\n"
NFSST18 "\nServer nfs:\n"
NFSST19 "nfsstat: 1831-545 The client system call nfs_cntl() failed.\n"

$set RSTATD
RSTD1 "rpc.rstatd: IFNET off value = 0x%1$x\n"

$set RUSRSD
RUSRD1 "rpc.rusersd: 1831-209 svc_rpc_udp_create: error\n"
RUSRD2 "rpc.rusersd: 1831-210 svc_rpc_register: error\n"
RUSRD3 "rpc.rusersd: 1831-211 run_svc_rpc should never return\n"
RUSRD4 "rpc.rusersd: 1831-212 cannot open %1$s\n"
RUSRD5 "rpc.rusersd: 1831-213 error\n"
RUSRD6 "svc_rpc_send_results"

$set YPPWDD
$ replace the following message with yppwdd19
YPPWDD1 "usage: %1$s file [-m arg1 arg2 ...]\n"
YPPWDD2 "yppasswdd: 1831-214 cannot write %1$s\n"
YPPWDD3 "yppasswdd: 1831-215 cannot chdir to /var/yp\n"
YPPWDD4 "yppasswdd: 1831-216 cannot bind to a privileged socket\n"
YPPWDD5 "yppasswdd: 1831-217 could not create an RPC server\n"
YPPWDD6 "yppasswdd: 1831-218 could not register yppasswdd\n"
YPPWDD7 "yppasswdd: 1831-219 svc_run shouldn't have returned\n"
YPPWDD8 "yppasswdd: 1831-220 could not reply to RPC call\n"
YPPWDD9 "yppasswdd: 1831-221 no passwd for %1$s\n"
YPPWDD10 "yppasswdd: 1831-222 %1$s: bad passwd\n"
YPPWDD11 "yppasswdd: 1831-223 password file busy - try again.\n"
YPPWDD12 "yppasswdd: 1831-224 fdopen failed?\n"
YPPWDD13 "yppasswdd: 1831-225 fopen of %1$s failed?\n"
YPPWDD14 "yppasswdd: 1831-226 couldnt reply to RPC call\n"
YPPWDD15 "socket: 1831-227 All ports in use\n"
YPPWDD16 "yppasswdd: 1831-228 cannot fork"
YPPWDD17 "rename"
YPPWDD18 "socket"
YPPWDD19 "usage: %1$s file [-nopw] [-noshell] [[-m|-r] arg1 arg2 ...]\n"
YPPWDD20 "usage: %1$s file [-nopw] [-nogecos] [-noshell] [[-m|-r] arg1 arg2 ...]\n"

$set YPUPDATD
YPUPD1 "%1$s: 1831-229 warning -- options ignored\n"
YPUPD2 "%1$s: 1831-230 cannot chdir to "
YPUPD3 "%1$s: 1831-231 cannot create tcp service\n"
YPUPD4 "%1$s: 1831-232 could not register service\n"
YPUPD5 "%1$s: 1831-233 cannot create udp service\n"
YPUPD6 "ypupdated: 1831-234 cannot fork"

$set RPC_MOUNTD
RPCMNTD1 "usage: rpc.mountd [-nid]\n"
RPCMNTD2 "rpc.mountd: 1831-235 mount dispatch routine:\n"
RPCMNTD3 "rpc.mountd: 1831-236 cannot fork"

$set RPC_RWALLD
RPCRWLD1 "rpc.rwalld: 1831-237 svc_rpc_udp_create: error\n"
RPCRWLD2 "rpc.rwalld: 1831-238 svc_rpc_register: error\n"
RPCRWLD3 "rpc.rwalld: 1831-239 Error: svc_run shouldn't have returned\n"
RPCRWLD4 "rpc.rwalld: 1831-240 rusersd error\n"
RPCRWLD5 "rstat: getsockname"

$set RPC_SPRAYD
RPCSPRYD1 "%1$s: 1831-241 could not create RPC server\n"
RPCSPRYD2 "%1$s: 1831-242 could not register SPRAYPROG\n"
RPCSPRYD3 "%1$s: 1831-243 shouldn't reach this point\n"
RPCSPRYD4 "rpc.sprayd 1831-244 could not reply to RPC call\n"
RPCSPRYD5 "rstat: getsockname"

$set RPCINFO
RPCINF1 "Usage:\n\
\trpcinfo [ -n portnum ] -u host prognum [ versnum ]\n\
\trpcinfo [ -n portnum ] -t host prognum [ versnum ]\n\
\trpcinfo -p [ host ]\n\
\trpcinfo -b prognum versnum\n\
\trpcinfo -d prognum versnum\n"
RPCINF2 "rpcinfo: 1831-245 %1$s is illegal port number\n"
RPCINF3 "rpcinfo: 1831-246 program %1$lu is not available\n"
$ please use RPCINF16 in place of this message
RPCINF4 "rpcinfo: 1831-247 program %1$lu version %1$lu\n\
\tis not available\n"
$ please use RPCINF15 in place of this message
RPCINF5 "program %1$lu version %1$lu\n\
\tready and waiting\n"
RPCINF6 "1831-248 cannot contact portmapper: "
RPCINF7 "rpcinfo: 1831-249 No remote programs registered.\n"
RPCINF8 "rpcinfo: 1831-250 broadcast failed: %1$s\n"
RPCINF9 "rpcinfo: 1831-251 Sorry. You are not root\n"
$ please use RPCINF17 in place of this message
RPCINF10 "rpcinfo: 1831-252 Could not delete registration for \n\
\tprog %1$s version %1$s\n"
RPCINF11 "rpcinfo: 1831-253 %1$s is unknown service\n"
RPCINF12 "rpcinfo: 1831-254 %1$s is illegal program number\n"
RPCINF13 "rpcinfo: 1831-255 %1$s is illegal version number\n"
RPCINF14 "rpcinfo: 1831-256 %1$s is unknown host\n"
$ Please use this message in place of RPCINF5
RPCINF15 "program %1$lu version %2$lu\n\
\tready and waiting\n"
$ Please use this message in place of RPCINF4
RPCINF16 "rpcinfo: 1831-247 program %1$lu version %2$lu\n\
\tis not available\n"
$ Please use this message in place of RPCINF10
RPCINF17 "rpcinfo: 1831-252 Could not delete registration for \n\
\tprog %1$s version %2$s\n"

$set RUP
RUP1 "Usage: rup [-h] [-l] [-t] [host ...]\n"
RUP2 "collecting responses... "
RUP3 "  up"
RUP4 "  load average:"

$set RUSERS
RUSERS1 "Usage: rusers [-a] [-h] [-i] [-l] [-u] [host ...]\n"
RUSERS2 "collecting responses....\n"
RUSERS3 "rusers: 1831-257 cannot find host name!\n"
RUSERS4 "rusers: 1831-258 too many hosts on network\n"

$set RWALL
RWALL1 "Usage: rwall host .... [-n netgroup ....] [-h host ...]\n"
RWALL2 "rwall: 1831-259 Cannot open /etc/utmp\n"
RWALL3 "rwall: 1831-260 Message too long\n"
RWALL4 "rwall: 1831-261 %1$s is unknown host\n"
RWALL5 "rwall: 1831-262 Could not contact %1$s:\n "

$set SHOWMOUNT
SHWMNT1 "Usage: showmount [-a] [-d] [-e] [host]\n"
SHWMNT2 "showmount: 1831-263 %1$s\n"
SHWMNT3 "showmount: 1831-264 table overflow:\n\
\tonly %1$d entries shown\n"
SHWMNT4 "showmount: no exported file systems for %1$s\n"
SHWMNT5 "export list for %1$s:\n"
SHWMNT6 "(everyone)"

$set SPRAY
SPRAY1 "Usage: spray host [-i] [-c cnt] [-l lnth] [-d usecs]\n"
SPRAY2 "spray: 1831-265 %1$s is unknown host name\n"
SPRAY3 "sending %1$d packets of length %2$d to\n\
\t%3$s ...\n"
SPRAY4 "\n%1$d packets (%2$.3f%%) dropped\n\
\tby %3$s\n"
SPRAY5 "\n\tno packets dropped by %1$s\n"
SPRAY6 "\t%1$d packets/sec, %2$d bytes/sec\n"
SPRAY7 "%1$d is max packet size\n"
SPRAY8 "sending %1$d packets of length %2$d to\n\
\t%3$s ...\n"
SPRAY9 "\n\t%1$d packets (%2$.3f%%) dropped by %3$s\n"
SPRAY10 "\nno packets dropped by %1$s\n"
SPRAY11 "\t%1$d packets/second, %2$d bytes/second\n"
SPRAY12 "%1$d is maximum packet size\n"


$set AUTOMOUNT
AMNT1 "Usage:\n\
\tautomount [-n] [-m] [-tl s] [-tm s] [-tw s] [dir map [-mntopts]]\
...\n"
AMNT2 "automount: 1831-266 Bad timeout value\n"
AMNT3 "automount: 1831-267 Bad timeout switch\n"
AMNT4 "already_mounted: 1831-268 malloc failed\n"
AMNT5 "already_mounted: 1831-269 mntctl error\n"
AMNT6 "Usage: automount\n\
\t[-n]\t\t(no mounts)\n\
\t[-m]\t\t(ignore NIS auto.master)\n\
\t[-T]\t\t(trace nfs requests)\n\
\t[-v]\t\t(verbose error msgs)\n\
\t[-tl n]\t\t(mount duration)\n\
\t[-tm n]\t\t(attempt interval)\n\
\t[-tw n]\t\t(unmount interval)\n\
\t[-M dir]\t(temporary mount dir)\n\
\t[-D n=s]\t(define env variable)\n\
\t[-f file]\t(get mntpnts from file)\n\
\t[ dir map [-mntopts] ] ...\n"
AMNT7 "Must be root to use automount\n"
AMNT8 "already mounted %1$s:%2$s on %3$s (%4$s)\n"
AMNT9 "couldn't create mntpnt %1$s\n"
AMNT10 "%1$s:%2$s no longer mounted\n"
AMNT11 "remount FAILED: server not responding\n"
AMNT12 "renew link for %1$s\n"
AMNT13 "It's on my host\n"
AMNT14 "automount:shouldignore():getmapent() \
returned NULL for mntpnt=%1$s, map=%2$s, opts=%3$s\n"
AMNT15 "Found Internet interface: %1$s %2$s\n"
AMNT16 "automount:islocal():net drivers misconfigured\n"
AMNT17 "automount:islocal:gethostbyname() error for hostname=|%1$s|\n"



$set KEYSERV
KEYS1 "usage: %1$s [-n]\n"
KEYS2 "keyenvoy: 1831-270 This program cannot be used interactively.\n"
KEYS3 "keyserv: 1831-271 %1$s must be run as root\n"
KEYS4 "%1$s: 1831-272 unable to create udp service\n"
KEYS5 "%1$s: 1831-273 unable to register service\n"
KEYS6 "keyserv: 1831-274 Invalid %1$s\n"
KEYS7 "keyserv: 1831-275 Cannot find %1$s's secret key\n"
KEYS8 "keyserv: 1831-276 Invalid password for %1$s\n"
KEYS9 "keyserv: 1831-277 unable to reply\n"
KEYS10 "keyserv: 1831-278 unable to free arguments\n"
KEYS11 "%1$s: 1831-279 \n"
KEYS12 "write"
KEYS13 "keyserv: 1831-280 cannot fork"

$set RPC_REXD
RPCRXD1 "rpc.rexd: 1831-281 No host name in %1$s\n"
RPCRXD2 "rpc.rexd: 1831-282 mount %1$s:\n\
\t%2$s not in hosts database\n"
RPCRXD3 "rpc.rexd: 1831-283 can not map port for %1$s\n"
RPCRXD4 "rpc.rexd: 1831-284 %1$s not responding to port map request\n", 
RPCRXD5 "rexd portmap: 1831-285 %1$s not responding\n"
RPCRXD6 "rpc.rexd: 1831-286 %1$s is not running a mount daemon\n"
RPCRXD7 "rpc.rexd: 1831-287 %1$s not responding to mount request\n", 
RPCRXD8 "rexd mount: 1831-288 %1$s not responding\n"
RPCRXD9 "rexd mount: 1831-289 not in export list for %1$s\n",
RPCRXD10 "rpc.rexd: 1831-290 unable to mount %1$s on %2$s: %3$s\n"
RPCRXD11 "rpc.rexd: 1831-291 unknown error" 
RPCRXD12 "rpc.rexd: 1831-292 %1$s not in hosts database\n"
RPCRXD13 "rpc.rexd: 1831-293 Host %1$s is not running mountd\n"
RPCRXD14 "rpc.rexd: 1831-294 RPC error %1$d with host %2$s\n"
RPCRXD15 "rpc.rexd: 1831-295 %1$s not exported by %2$s\n" 
RPCRXD16 "rpc.rexd: 1831-296 out of memory in unmount\n"
RPCRXD17 "rpc.rexd: 1831-297 invalid mount revision\n"
RPCRXD18 "rpc.rexd: 1831-298 mntctl error %1$d\n"
RPCRXD19 "rpc.rexd: 1831-299 could not find %1$s to unmount\n"
RPCRXD20 "rpc.rexd: 1831-300 svctcp_create error\n"
RPCRXD21 "rpc.rexd: 1831-301 service register error\n"
RPCRXD22 "rpc.rexd: 1831-302 service rpc register: error\n"
RPCRXD23 "rpc.rexd: 1831-303 cannot create temp directory %1$s\n"
RPCRXD24 "rpc.rexd: 1831-304 Select returned zero\r\n"
RPCRXD25 "rpc.rexd: 1831-305 NULLPROC error\n"
RPCRXD26 "rpc.rexd: 1831-306 reply failed\n"
RPCRXD27 "rpc.rexd: 1831-307 mode reply failed"
RPCRXD28 "rpc.rexd: 1831-308 window change reply failed"
RPCRXD29 "rpc.rexd: 1831-309 signal reply failed"
RPCRXD30 "rpc.rexd: 1831-310 No process to wait for!\n"
RPCRXD31 "rpc.rexd: 1831-311 child killed to unmount %1$s\r\n"
RPCRXD32 "rpc.rexd: 1831-312 could not umount %1$s from %2$s\r\n"
RPCRXD33 "rpc.rexd: 1831-313 cannot allocate a pty\n"
RPCRXD34 "rpc.rexd: 1831-314 cannot fork\n"
RPCRXD35 "rpc.rexd: 1831-315 cannot chdir to %1$s\n"
RPCRXD36 "rpc.rexd: 1831-316 cannot exec shell %1$s\n"
RPCRXD37 "rpc.rexd: 1831-317 cannot exec %1$s\n"
RPCRXD38 "rpc.rexd: 1831-318 out of memory\n"
RPCRXD39 "rpc.rexd: 1831-319 root execution not allowed\n"
RPCRXD40 "rpc.rexd: 1831-320 User id %1$d not valid\n"
RPCRXD41 "ioctl: 1831-321 TIOCSETD failed\n"
RPCRXD42 "ioctl: 1831-322 TIOCSETN failed\n"
RPCRXD43 "ioctl: 1831-323 TIOCSETC failed\n"
RPCRXD44 "ioctl: 1831-324 TIOCSLTC failed\n"
RPCRXD45 "ioctl: 1831-325 TIOCLSET failed\n"
RPCRXD46 "Warning on unmount create: "
RPCRXD47 "Warning: unmount: "
RPCRXD48 "rpc.rexd: 1831-549 User login name %1$s not found\n"
RPCRXD49 "rpc.rexd: 1831-550 Can't set user credentials.\n"
$set ON
ON1 "Usage: on [-i|-n] [-d] machine cmd [args]...\n"
ON2 "on: 1831-326 Unknown option %1$s\n"
ON3 "on: 1831-327 unknown host %1$s\n"
ON4 "on: 1831-328 cannot connect to server on %1$s\n"
ON5 "on: 1831-329 cannot find . (%1$s)\n"
ON6 "on: 1831-330 cannot locate mount point for %1$s (%2$s)\n"
ON7 "where: 1831-331 Unable to get working directory (%1$s)"
ON8 "where: 1831-332 out of memory in findmount\n"
ON9 "where: 1831-333 invalid mount revision"
ON10 "where: 1831-334 mntctl error %1$d\n"

$set RPC_BOOTPARAMD
RPCBTPMD1 "usage: %1$s [-d]\n"
RPCBTPMD2 "rpc.bootparamd: 1831-335 cannot access symbols \n\
\t\"%1$s\", \"%2$s\", \"%3$s\"\ in kernel\n"
RPCBTPMD3 "rpc.bootparamd: 1831-336 cannot open %1$s\n"
RPCBTPMD4 "rthost: 1831-337 symbol not in namelist\n"
RPCBTPMD5 "rtnet: 1831-338 symbol not in namelist\n"
RPCBTPMD6 "rpc.bootparamd: 1831-339 read rthost in kmem failed.\n"
RPCBTPMD7 "rpc.bootparamd: 1831-340 read routehash from kmem failed.\n"
RPCBTPMD8 "rpc.bootparamd: 1831-341 read rt->rt_ifp from kmem failed.\n"
RPCBTPMD9 "rpc.bootparamd: 1831-342 read ifaddr from kmem failed.\n"
RPCBTPMD10 "rpc.bootparamd: 1831-343 read in_ifaddr from kmem failed.\n"
RPCBTPMD11 "rpc.bootparamd: 1831-344 read rtnet in kmem failed.\n"
RPCBTPMD12 "getfile_1: file is \"%1$s\" %2$s \"%3$s\"\n"
RPCBTPMD13 "rpc.bootparamd: 1831-345 In debug mode\n"
RPCBTPMD14 "rpc.bootparamd: 1831-346 cannot create udp service.\n"
RPCBTPMD15 "rpc.bootparamd: 1831-347 unable to register \n\
\t(BOOTPARAMPROG, BOOTPARAMVERS, udp).\n"
RPCBTPMD16 "rpc.bootparamd: 1831-348 svc_run returned\n"
RPCBTPMD17 "rpc.bootparamd: 1831-349 unable to free arguments\n"

$set SMIT
$ All shell scripts in ./com/cmd/usr.etc/smit are in this set because they
$ share these messages.
SM1 "Usage: chnfs -b num_biods -n num_nfsds [-I|-B|-N]\n"
SM2 "Usage: chypdom [-I|-B|-N] newdomainname\n"
SM3 "Usage: lsmaster [ -c | -l ]\n"
SM4 "Usage: %1$s [-c | -l ] [directory]\n"
SM5 "Usage: %1$s [-I|-B|-N]\n"
SM6 "Usage:\n\
%1$s [-s slave1[,slave2]...] [-O|-o] [-E|-e] [-P|-p] [-U|-u]\n\
\t[-C|-c] [-I|-B|-N]\n"
SM7 "Usage: %1$s -d directory [ -f exports_file ]\n\
\t[-t {rw|ro}{rm -h hostname[,hostname]...}]\n\
\t[-a uid]\n\
\t[-r hostname[,hostname]...\n\
\t[-c hostname[,hostname]...\n\
\t[-s|-n] [-I|-B|-N]\n"
SM8 "Usage: %1$s -f path_name -d remote_directory -h remote_host \n\
\t[-t {rw|ro}] \n\
\t[-m mount_type_name] \n\
\t[-w {fg|bg}] \n\
\t[{-S|-H}] \n\
\t[{-e|-E}] \n\
\t[{-s|-n}] [{-I|-B|-N}] \n\
\t[-r times_to_retry] [-b read_buffer_size] [-c write_buffer_size] \n\
\t[-o timeout] [-P port_number] \n\
\t[-u acregmin] [-U acregmax] [-v acdirmin] [-V acdirmax] [-T actimeo] \n\
\t[-p num_biods]\n"
SM9 "Usage: %1$s [-O|-o] [-I|-B|-N] master\n"
SM10 "Usage: rmnfsexp -d directory [-f exports_file] [-B|-I|-N]\n"
SM11 "Usage: rmnfsmnt -f path_name [-I|-B|-N]\n"
SM12 "Usage: rmyp {-s | -c}\n"
SM13 "%1$s: 1831-350 error in updating the domainname of the system\n"
SM14 "%1$s: 1831-351 error in updating %2$s\n"
SM15 "Slave servers for the domain %1$s\n"
SM16 "\nDomains that are being served\n"
SM17 "\nThese NIS daemons will be started.\n"
SM18 "\typserv will be started on system restart\n"
SM19 "\typbind will be started on system restart\n"
SM20 "\typpasswdd will be started on system restart\n"
SM21 "\typupdated will be started on system restart\n"
SM22 "%1$s: 1831-352 Exiting because of errors in %2$s\n"
SM23 "%1$s: 1831-353 error in adding entry to inittab\n"
SM24 "%1$s: 1831-354 encountered an error in rebuilding \n\
\tthe inetd data base\n"
SM25 "mknfsexp: 1831-355 an export for %1$s already exists\n"
SM26 "chnfsexp: 1831-356 an export for %1$s does not exist\n"
SM27 "Unable to change %1$s. Aborting action\n"
SM28 "%1$s: 1831-357 Error in unmounting %2$s for change\n"
SM29 "%1$s: 1831-358 Exiting because of errors in %2$s\n"
SM30 "%1$s: 1831-359 error in removing 'rcnfs' entry from inittab\n"
SM31 "rmnfsexp: 1831-360 %1$s is not currently exported\n\
\tand is not in %2$s\n"
SM32 "%1$s: 1831-361 %2$s not currently exported.\n"
SM33 "Unable to change %1$s. Aborting action\n"
SM34 "rmnfsmnt: 1831-362 Error in unmounting %1$s\n"
SM35 "rmnfsmnt: 1831-363 %1$s not currently mounted\n"
SM36 "ERROR: 1831-364 parameter values cannot be less than\n\
or equal to 0\n"
SM37 "Usage: %1$s [ -f exports_file ] [-c | -l ] [directory]\n"
$ Please use message SM39 instead of this one (SM38)
SM38 "Usage: %1$s -f path_name -d remote_directory -h remote_host \n\
\t[-t {rw|ro}] \n\
\t[-m mount_type_name] \n\
\t[-w {fg|bg}] \n\
\t[{-S|-H}] \n\
\t[{-e|-E}] [{-a|-A}]\n\
\t[{-s|-n}] [{-I|-B|-N}] \n\
\t[-r times_to_retry] [-b read_buffer_size] [-c write_buffer_size] \n\
\t[-o timeout] [-P port_number] \n\
\t[-u acregmin] [-U acregmax] [-v acdirmin] [-V acdirmax] [-T actimeo] \n\
\t[-p num_biods]\n"
$ Please use this message in place of SM38
SM39 "Usage: %1$s -f path_name -d remote_directory -h remote_host \n\
\t[-t {rw|ro}] \n\
\t[-m mount_type_name] \n\
\t[-w {fg|bg}] [{-X|-x}] \n\
\t[{-S|-H}] [{-Y|-y}] [{-Z|-z}]\n\
\t[{-e|-E}] [{-a|-A}]\n\
\t[{-s|-n}] [{-I|-B|-N}] \n\
\t[-r times_to_retry] [-b read_buffer_size] [-c write_buffer_size] \n\
\t[-o timeout] [-P port_number] \n\
\t[-u acregmin] [-U acregmax] [-v acdirmin] [-V acdirmax] [-T actimeo] \n\
\t[-p num_biods]\n"
SM40 "Usage: %1$s [-O|-o] [-C|-c] [-I|-B|-N] master\n"
SM41 "%s: %s: No such file or directory.\n"
SM42 "%s: Error. %s is not an NFS file system.\n"
SM43 "%s: The local host's domain name has not been set.\
Please set it.\n"

$set YPSET
YP1 "Usage:\n\
\tmakedbm -u file\n\
\tmakedbm [-b] [-s] [-i YP_INPUT_FILE] [-o YP_OUTPUT_FILE]\n\
\t        [-d YP_DOMAIN_NAME] [-m YP_MASTER_NAME] infile outfile\n"
YP2 "1831-365 items not in pairs\n"
YP3 "1831-366 cannot open database %1$s\n"
YP4 "1831-367 entry too big\n"
YP5 "1831-368 split not paired\n"
YP6 "1831-369 bad delitem\n"
YP7 "1831-370 bad block\n"
YP8 "makedbm: 1831-371 cannot open %1$s\n"
YP9 "makedbm: 1831-372 cannot create %1$s\n"
YP10 "makedbm: 1831-373 cannot init %1$s\n"
YP11 "makedbm: 1831-374 ERROR!\n"
YP12 "makedbm: 1831-375 problem storing %1$.*s %2$.*s\n"
YP13 "makedbm: 1831-376 cannot stat %1$s\n"
YP14 "ypserv: 1831-377 Map name string too long.\n"
YP15 "ypserv: 1831-378 ypcheck_domain stat error on file %1$s.\n"
YP16 "ypserv: 1831-379 %1$s execl failure.\n"
YP17 "ypserv: 1831-380 %1$s cannot respond to rpc request.\n"
YP18 "ypserv: 1831-381 %1$s cannot free args.\n"
YP19 "ypserv: 1831-382 %1$s no such map or access denied.\n"
YP20 "ypserv: 1831-383 Domain %1$s not supported\n"
YP21 "ypserv: 1831-384 Domain %1$s not supported (broadcast)\n"
YP22 "ypserv: 1831-385  bad fds bits in main loop select mask.\n"
YP23 "ypserv: 1831-386  invalid timeout in main loop select.\n"
YP24 "ypserv: 1831-387 Cannot catch process exit signal.\n"
YP25 "ypserv: 1831-388 Cannot reply to rpc call.\n"
YP26 "ypserv: 1831-389 Stat error on map file %1$s.\n"
YP27 "ypserv: 1831-390 Unable to create server for "
YP28 "ypserv: 1831-391 Unable to register service for "
YP29 "ypserv: 1831-392 Stat error on map file %1$s.\n"
YP30 "Usage:\n\
ypset [-V1] [-h <host>] [-d <domainname>] server_to_use\n\
\twhere host and server_to_use may be either names or internet \n\
addresses of form ww.xx.yy.zz\n"
YP31 "1831-393: Sorry, I cannot get an address for host %s from NIS.\n"
YP32 "1831-394: Sorry, I got a garbage address for host %s back from NIS.\n"
YP33 "1831-395: Sorry, the %s argument is bad.\n"
YP34 "1831-396: Sorry, cannot get %s back from system call.\n"
YP35 "1831-397: Sorry, the %s has not been set on this machine.\n"
YP36 "1831-398: Sorry, I cannot make use of NIS.  I give  up.\n"
YP37 "1831-399: Sorry, I cannot set up a udp connection to ypbind on host %s.\n"
YP38 "1831-400: Sorry, I could not send my rpc message to ypbind on host %s.\n"
YP39 "Usage:\n\
ypxfr [-f] [-h <host>] [-d <domainname>]\n\
\t[-s <domainname>] [-S] [-c] [-C tid prot ipadd port] map\n\n\
where\n\
\t-f forces transfer even if the master's copy is not newer.\n\
\t\thost may be either a name or an internet \n\
\t\taddress of form ww.xx.yy.zz\n\
\t-S ypxfr will check that ypserv is using \"priviledged\" ports.\n\
\t-c inhibits sending a \"Clear map\" message to the local ypserv.\n\
\t-C is used by ypserv to pass callback information.\n"
YP40 "1831-401: Sorry, I cannot set up a udp connection to ypserv on host %s.\n"
YP41 "ypxfr: 1831-402 Cannot get  order number for map %1$s\n\
from server at %2$s: use the -f flag.\n"
YP42 "Transferred map %1$s from %2$s (%3$d entries).\n"
YP43 "ypxfr: 1831-403 Cannot translate master name\n\
%1$s to an address.\n"
YP44 "bind_to_server: server is not using a privileged port\n"
YP45 "ypxfr: 1831-404 bind_to_server clntudp_create error\n"
YP46 "ypxfr: 1831-405 (info) Cannot get order number from ypserv\n\
at %1$s. Reason: %2$s\n"
YP47 "ypxfr(get_v1order) RPC call to %1$s failed" 
YP48 "ypxfr(get_v2order) RPC call to %1$s failed"
YP49 "ypxfr: 1831-406 Cannot get master name from ypserv at\n\
%1$s. Reason: %2$s.\n"
YP50 "ypxfr(get_v1master_name) RPC call to %1$s failed"
YP51 "ypxfr(get_v2master_name) RPC call to %1$s failed"
YP52 "ypxfr: 1831-407 Map %1$s at %2$s is not more\n\
recent than local.\n"
YP53 "ypxfr: 1831-408 Cannot create temp map %1$s.\n"
YP54 "ypxfr: 1831-409 Cannot dbm init temp map %1$s.\n"
YP55 "ypxfr: 1831-410 Cannot do dbm close operation on temp map %1$s.\n"
YP56 "ypxfr: 1831-411 Version skew at %1$s while\n\
transferring map %2$s.\n"
YP57 "ypxfr: 1831-412 Cannot do dbm store into temp map %1$s.\n"
YP58 "ypxfr: 1831-413 Rename error: could not move %1$s to %2$s.\n"
YP59 "ypxfr: 1831-414 Rename error: check that old %1$s is\n\
still intact.\n"
YP60 "ypxfr: 1831-415 Map name string too long.\n"
YP61 "ypxfr: 1831-416 Cannot move %1$s to %2$s.\n"
YP62 "ypxfr: 1831-417 Cannot recover from rename failure.\n"
YP63 "ypxfr: 1831-418 Stat error on map file %1$s.\n"
YP64 "ypxfr: 1831-419 Error from ypserv on %1$s\n\
(ypall_callback) = %2$s.\n"
YP65 "*** Count mismatch in dbm file %1$s: oldyy=%2$d, new=%3$d ***\n"
YP66 "ypxfr: 1831-420 Wrong version number!\n"
YP67 "*** Count mismatch after emumerate %1$s: oldyy=%2$d, new=%3$d ***\n"
YP68 "ypxfr: 1831-421 Error from ypserv on %1$s\n\
 (get first) = %2$s.\n"
YP69 "ypxfr: 1831-422 Error from ypserv on %1$s\n\
 (get next) = %2$d.\n"
YP70 "Duplicate key %1$s in map %2$s\n"
YP71 "key %1$s was not inserted into dbm file\n"
YP72 "ypxfr: 1831-423 Cannot get local machine name.\n"
YP73 "ypxfr: 1831-424 Cannot send ypclear message to \n\
ypserv on the local machine.\n"
YP74 "ypxfr: 1831-425 Cannot get master of %1$s.\n\
Reason: %2$s.\n"
YP75 "Database file %1$s -- has bad format.\n"

$set MKALIAS
MKAL1 "Usage: mkalias <input> <output>\n"
MKAL2 "mkalias: 1831-426 Unknown option %1$c\n"
MKAL3 "mkalias: 1831-427 Unable to open input database %1$s\n"
MKAL4 "mkalias: 1831-428 Unable to open output database %1$s\n"

$set NEWKEY
NWKY1 "Usage: %1$s [-u username] [-h hostname]\n" 
NWKY2 "newkey: 1831-429 must be root to run %1$s\n"
NWKY3 "newkey: 1831-430 cannot chdir to "
NWKY4 "newkey: 1831-431 unknown user: %1$s\n"
NWKY5 "newkey: 1831-432 unknown host: %1$s\n"
NWKY6 "Adding new key for %1$s.\n"
NWKY7 "newkey: 1831-433 Password incorrect.\n"
NWKY8 "Please wait for the database to get updated...\n"	
NWKY9 "%1$s: 1831-434 unable to update NIS database\n\
\t(%2$u): %3$s\n" 
NWKY10 "Your new key has been successfully stored away.\n"

$set STDETHERS
STDET1 "%1$s: 1831-435 cannot open %2$s\n"
STDET2 "%1$s: 1831-436 ignoring line: %2$s\n"

$set STDHOSTS
STHS1 "stdhosts: cannot open %s\n"
$set YPPOLL
YPPL1 "Usage: yppoll [-h <host>] [-d <domainname>] mapname\n\
\twhere host may be either a name or an internet \n\
\taddress of form ww.xx.yy.zz\n"
YPPL2 "yppoll: 1831-437 Bad %1$s argument.\n"
YPPL3 "yppoll: 1831-438 Cannot get %1$s back from system call.\n"
YPPL4 "yppoll: 1831-439 %1$s has not been set on this machine.\n"
YPPL5 "yppoll: 1831-440 Ill-formed response returned from ypserv\n\
\ton host %1$s.\n"
YPPL6 "ypwhich: 1831-441 cannot find %1$s\n"
YPPL7 "yppoll: 1831-442 Cannot create UDP connection to %1$s.\n"
YPPL8 "yppoll: 1831-443 Cannot make YPPROC_ORDER call to \n\
\typserv at %1$s.\n"
YPPL9 "yppoll: 1831-444 Cannot create V1 UDP connection to %1$s.\n"
YPPL10 "yppoll: 1831-445 Cannot make YPPROC_POLL call to \n\
\typserv at %1$s\n"
YPPL11 "yppoll: 1831-446 Cannot make YPPROC_MASTER call to\n\
\typserv at %1$s.\n"
YPPL12 "yppoll: 1831-447 Cannot get any map parameter information.\n"
YPPL13 "Domain %1$s is supported.\n"
YPPL14 "Map %1$s has order number %2$d.\n"
YPPL15 "The master server is %1$s.\n"
YPPL16 "yppoll: 1831-448 Unknown master server.\n"
YPPL17 "yppoll: 1831-449 Map %1$s is not supported.\n"
YPPL18 "yppoll: 1831-450 Map %1$s does not exist at %2$s.\n"
YPPL19 "yppoll: 1831-451 Domain %1$s is not supported.\n"
YPPL20 "yppoll: 1831-452 cannot get hostname\n"
YPPL21 "yppoll: 1831-453 could not get NIS server %1$d\n"

$set YPPUSH
YPPS1 "Usage:\n\
yppush [-d <domainname>] [-v] map\n"
YPPS2 "yppush: 1831-454 Map does not exist.\n"
YPPS3 "Finding NIS servers:"
YPPS4 "yppush: 1831-455 Cannot get an address for server %1$s.\n"
YPPS5 "yppush: 1831-456 Cannot set up as a udp server.\n"
YPPS6 "yppush: 1831-457 Cannot set up transient callback server.\n"
YPPS7 "yppush: 1831-458 Invalid timeout in main loop select.\n"
YPPS8 "yppush: 1831-459 No response from ypxfr on %1$s\n"
YPPS9 "yppush: 1831-460 Cannot reply to rpc call.\n"
YPPS10 "yppush: 1831-461 Bad server state value %1$d.\n"
YPPS11 "Status received from ypxfr on %1$s:\n\t%2$s\n"
YPPS12 "yppush listener: 1831-462 Garbage transaction status\n\
\t(value %1$d) from ypxfr on %2$s.\n"
YPPS13 "yppush: 1831-463 Bad error code passed to rpcerr_msg: %1$d.\n"
YPPS14 "yppush: 1831-464 Error talking to %1$s: \n"
YPPS15 "yppush: 1831-465 V1 Error talking to %1$s: \n"
YPPS16 "yppush: 1831-466 The %1$s argument is bad.\n"
YPPS17 "yppush: 1831-467 Cannot get %1$s from system call.\n"
YPPS18 "yppush: 1831-468 The %1$s has not been set on this machine.\n"
YPPS19 "yppush: 1831-469 Cannot find an NIS server for domain %1$s.\n\
Reason:  %2$s.\n"
YPPS20 "yppush: 1831-470 Cannot build server list\n\
from map \"ypservers\".  Reason:  %1$s.\n"
YPPS21 "yppush: 1831-471 Internal error trying to talk to %1$s."
YPPS22 "Map successfully transferred."
YPPS23 "Transfer not done : master's version isn't newer."
YPPS24 "Failed 1831-472 - ypxfr there cannot find a server for map."
YPPS25 "Failed 1831-473 - domain isn't supported."
YPPS26 "Failed 1831-474 - local resource allocation failure."
YPPS27 "Failed 1831-475 - ypxfr had an RPC failure"
YPPS28 "Failed 1831-476 - ypxfr could not get the map master's address."
YPPS29 "Failed 1831-477 - NIS server or map format error."
YPPS30 "Failed 1831-478 - args to ypxfr were bad."
YPPS31 "Failed 1831-479 - dbm operation on map failed."
YPPS32 "Failed 1831-480 - file I/O operation on map failed"
YPPS33 "Failed 1831-481 - map version skew during transfer."
YPPS34 "Map successfully transferred, but ypxfr could not send\n\
\"Clear map\" to ypserv "
YPPS35 "Failed 1831-482 - no local order number in map - use -f flag to ypxfr."
YPPS36 "Failed 1831-483 - ypxfr internal error."
YPPS37 "Failed 1831-484 - Transfer request refused."
YPPS38 "RPC success"
YPPS39 "RPC Cannot encode args"
YPPS40 "RPC Cannot decode results"
YPPS41 "RPC Cannot send"
YPPS42 "RPC Cannot receive"
YPPS43 "NIS server registered, but does not respond"
YPPS44 "RPC version mismatch"
YPPS45 "RPC auth error"
YPPS46 "RPC remote program unavailable"
YPPS47 "RPC program mismatch"
YPPS48 "RPC unknown procedure"
YPPS49 "RPC Cannot decode args"
YPPS50 "unknown host"
YPPS51 "portmap failure (host is down?)"
YPPS52 "RPC prog not registered"
YPPS53 "RPC system error"
YPPS54 "Internal error trying to talk to %1$s."
YPPS55 "%1$s has been called."
YPPS56 "%1$s (v1 ypserv) sent an old-style request."
YPPS57 "NIS server not registered at %1$s."
YPPS58 "RPC error to %1$s:  "
YPPS59 "Local resource allocation failure - cannot talk to %1$s."
YPPS60 "System error talking to %1$s:  "

$set MAKE_SCRIPT
MKSC1 "updated passwd\n"
MKSC2 "pushed passwd\n"
MKSC3 "could not find %1$s/passwd\n"
MKSC4 "updated group\n"
MKSC5 "pushed group\n"
MKSC6 "could not find %1$s/group\n"
MKSC7 "updated hosts\n"
MKSC8 "pushed hosts\n"
MKSC9 "could not find %1$s/hosts\n"
MKSC10 "updated ethers\n"
MKSC11 "pushed ethers\n"
MKSC12 "could not find %1$s/ethers\n"
MKSC13 "updated networks\n"
MKSC14 "pushed networks\n"
MKSC15 "could not find %1$s/networks\n"
MKSC16 "updated services\n"
MKSC17 "pushed services\n"
MKSC18 "could not find %1$s/services\n"
MKSC19 "updated rpc\n"
MKSC20 "pushed rpc\n"
MKSC21 "could not find %1$s/rpc\n"
MKSC22 "updated protocols\n"
MKSC23 "pushed protocols\n"
MKSC24 "could not find %1$s/protocols\n"
MKSC25 "updated netgroup\n"
MKSC26 "pushed netgroup\n"
MKSC27 "could not find %1$s/netgroup\n"
MKSC28 "updated bootparams\n"
MKSC29 "pushed bootparams\n"
MKSC30 "could not find %1$s/bootparams\n"
MKSC31 "updated aliases\n"
MKSC32 "pushed aliases\n"
MKSC33 "could not find %1$s\n"
MKSC34 "updated publickey\n"
MKSC35 "pushed publickey\n"
MKSC36 "could not find %1$s/publickey\n"
MKSC37 "updated netid\n"
MKSC38 "pushed netid\n"
MKSC39 "updated netmasks\n"
MKSC40 "pushed netmasks\n"
MKSC41 "could not find %1$s/netmasks\n"
MKSC42 "The NIS map netid may be out of date.\nPlease update the netid NIS map.\n"

$set YPINIT
$ Remember NOT to translate user choices and replys
YPI1 "Usage:\n\
\typinit -m\n\
\typinit -s master_server\n\
\typinit -q [-o] [-n] {-m [slave1 [slave2]...] | -s master_server}\n"
YPI2 "ypinit:1831-485 Cannot enumerate maps from %1$s.\n\
\tPlease check that it is running.\n"
YPI3 "ypinit:1831-486 Cannot get local host's name.\n\
\tPlease check your path.\n"
YPI4 "ypinit:1831-487 The local host's name has not been set.\n\
\tPlease set it.\n"
YPI5 "ypinit:1831-488 Cannot get local host's domain name.\n\
\tPlease check your path.\n"
YPI6 "ypinit:1831-489 The local host's domain name has not \n\
\tbeen set. Please set it.\n"
YPI7 "ypinit:1831-490 You have to be the superuser to run this.\n\
\tPlease log in as root.\n"
YPI8 "ypinit:1831-491 The directory %1$s doesn't exist.\n\
\tRestore it from the distribution.\n"
YPI9 "ypinit:1831-492 The host specified should be a running \n\
\tmaster NIS server, not this machine.\n"
YPI10 "Installing the NIS data base will require that you answer\n\
\ta few questions.\n"
YPI11 "Questions will all be asked at the beginning of the procedure.\n"
YPI12 "Do you want this procedure to quit on non-fatal errors? [y/n: n] "
YPI13 "OK, please remember to go back and redo manually\n\
whatever fails. If you don't, some part of the system \n\
(perhaps the NIS itself) won't work.\n"
YPI14 "Can we destroy the existing %1$s and its contents? [y/n: n] "
YPI15 "ypinit:1831-493 Cannot clean up old directory %1$s.\n\
\tFatal error.\n"
YPI16 "OK, please clean it up by hand and start again.  Bye\n"
YPI17 "ypinit:1831-494 Cannot make new directory %1$s.\n\
\tFatal error.\n"
YPI18 "There will be no further questions.\n\
The remainder of the procedure should take a few minutes,\n\
to copy the data bases from %1$s.\n"
YPI19 "Transferring %1$s...\n"
YPI20 "ypinit:1831-495 %1$s's NIS data base has been\n\
\tset up with errors. Please remember to figure out what went\n\
\twrong, and fix it.\n"
YPI21 "ypinit:1831-496 %1$s's NIS data base has been\n\
\tset up without any errors.\n"
YPI22 " At this point, make sure that /etc/passwd, /etc/hosts,\n\
\t/etc/networks, /etc/group, /etc/protocols, /etc/services/,\n\
\t/etc/rpc and /etc/netgroup have been edited so that when\n\
\tNIS is activated, the data bases you have just created will be \n\
\tused, instead of the /etc ASCII files.\n"
YPI23 "At this point, we have to construct a list of the\n\
hosts which will run NIS servers. %1$s is in the list of NIS \n\
server hosts.  Please continue to add the names for the other\n\
hosts, one per line. When you are done with the list, type a <control D>.\n"
YPI24 "next host to add:  %1$s\n"
YPI25 "next host to add:  "
YPI26 "The current list of NIS servers looks like this:\n"
YPI27 "Is this correct?  [y/n: y]  "
YPI28 "Let's try the whole thing again...\n"
YPI29 "There will be no further questions. The remainder\n\
of the procedure should take 5 to 10 minutes.\n"
YPI30 "Building %1$s/%2$s/ypservers...\n"
YPI31 "ypinit:1831-497 Could not build NIS data base\n\
\t%1$s/ypservers.\n"
YPI32 "Running %1$s/Makefile...\n"
YPI33 "ypinit:1831-498 Error running Makefile.\n"
YPI34 "ypinit:1831-499 %1$s has been set up as a NIS master \n\
\tserver with errors.\n\
\tPlease remember to figure out what went wrong, and fix it.\n"
YPI35 "If there are running slave NIS servers, run yppush\n\
now for any data bases which have been changed.\n\
If there are no running slaves, run ypinit on those\n\
hosts which are to be slave servers.\n"
YPI36 "%1$s has been set up as a NIS master \n\
\tserver without any errors.\n"

$set MKNETID 
MKNID1 "Usage: %1$s [-q] [-pghm filename]\n"
MKNID2 "%1$s: 1831-500 cannot open %2$s\n"
MKNID3 "%1$s: 1831-501 syntax error in file \"%2$s\", line %3$d\n" 
MKNID4 "%1$s: 1831-502 %2$s multiply defined,\n\
\tother definitions ignored\n"
MKNID5 "%1$s: 1831-503 %2$s's groups exceed %3$d\n"

$set REVNETGROUP
RVNG1 "Usage: %1$s -h (by host), %2$s -u (by user)\n"
RVNG2 "1831-504 Cycle detected in /etc/netgroup: %1$s.\n"
RVNG3 "revnetgroup: 1831-505 syntax error in /etc/netgroup\n"
RVNG4 "1831-506 fatal error: %1$s\n"

$set LIBRPCSVC
$ Please use message RPCSVC10 instead of RPCSVC1
RPCSVC1 "Invalid option(s), list of options is:\n\
\tro|rw,soft|hard,fg|bg,intr,secure,noac,rsize=n,wsize=n,timeo=n,retrans=n\n\
\tretry=n,port=n,actimeo=n,acregmin=n,acregmax=n,acdirmin=n,acrdmax=n,biods=n\n"
RPCSVC2 "%1$s: 1831-507 already mounted over\n"
RPCSVC3 "nfs_mount: 1831-508 %1$d error while mounting %2$s:"
RPCSVC4 "mount.c: 1831-509 nfsmounts malloc failed\n"
RPCSVC5 "mount.c: 1831-510 nfsmounts mntctl error\n"
RPCSVC6 "mount.c: 1831-511 automountd: out of memory in unmount\n"
RPCSVC7 "mount.c: 1831-512 automountd: invalid mount revision\n"
RPCSVC8 "mount.c: 1831-513 automountd: could not find %1$s to unmount\n"
RPCSVC9 "Cannot malloc vmount struct, FATAL ERROR"
$ Please use this message, RPCSVC10, instead of RPCSVC1
RPCSVC10 "Invalid option(s), list of options is:\n\
\tro|rw,soft|hard,fg|bg,intr,secure,noac,nodev,nosuid,bsy,shortdev,rsize=n,\n\
\twsize=n,timeo=n,retrans=n,retry=n,port=n,actimeo=n,acregmin=n,acregmax=n,\n\
\tacdirmin=n,acrdmax=n,biods=n\n"

$set PCNFSD
PCNFS1 "FREE_CHILD: process #%1$d exited with status 0X%2$x\n"
PCNFS2 "pc-nfsd error: 1831-514 -s option must be followed by a\n\
spooling directory path\n"
PCNFS3 "pc-nfsd: 1831-515 could not open /dev/null\n"
PCNFS4 "pc-nfsd: 1831-516 could not open /dev/console\n"
PCNFS5 "pc-nfsd: started from inetd\n"
PCNFS6 "pc-nfsd: 1831-517 invalid spool directory %1$s\n"
PCNFS7 "pc-nfsd: 1831-518 error - svc_run returned\n"
PCNFS8 "NULLPROC called\n"
PCNFS9 "PCNFSD_AUTH called\n"
PCNFS10 "AUTH returns %1$d %2$d %3$d\n"
PCNFS11 "PCNFSD_PR_INIT called\n"
PCNFS12 "PCNFSD_PR_START called\n"
PCNFS13 "pc-nfsd error: 1831-519 unknown function %1$d called\n\
Dispatch()\n"
PCNFS14 "AUTHPROC username=%1$s\n"
PCNFS15 "pc-nfsd: 1831-520 unable to create spool directory %1$s\n"
PCNFS16 "PR_INIT pathname=%1$s\n"
PCNFS17 "PR_START pathname=%1$s\n"
PCNFS18 "PR_START username=%1$s\n"
PCNFS19 "PR_START client= %1$s\n"
PCNFS20 "...cannot stat it.\n"
PCNFS21 "...PR_START returns PS_RES_NO_FILE\n"
PCNFS22 "...PR_START returns PS_RES_ALREADY\n"
PCNFS23 "...PR_START returns PS_RES_NULL\n"
PCNFS24 "...renamint %1$s -> %2$s\n"
PCNFS25 "... trying new spool file -> %1$s\n"
PCNFS26 "pc-nfsd: 1831-521 spool file rename (%1$s->%2$s)\n\
failed\n"
PCNFS27 "username is %1$s\n"
PCNFS28 "uid is %1$d\ngid is %2$d\n"
PCNFS29 "...run_ps630 invoked\n"
PCNFS30 "...PR_START returns PS_RES_FAIL\n"
PCNFS31 "...forked child #%1$d\n"
PCNFS32 "...PR_START returns PS_RES_OK\n"

$set RPCGEN
RGEN1 "usage:\n\
\t%1$s infile\n\
\t%1$s [-c | -h | -l | -m] [-o outfile] [infile]\n\
\t%1$s [-s udp|tcp]* [-o outfile] [infile]\n"
RGEN2 "%1$s: 1831-522 output would overwrite %2$s\n"
RGEN3 "%1$s: 1831-523 unable to open "
RGEN4 "%1$s, line %2$d: "
RGEN5 "too many files!\n"
RGEN6 "expected '%1$s'"
RGEN7 "expected '%1$s' or '%2$s'"
RGEN8 "expected '%1$s', '%2$s', or '%3$s'"

$set HANFS
HANFS1 "Usage: %1$s -s primary_hostname\n\
\t-m twin_hostname\n\
\t-N primary_net_adapter\n\
\t-n secondary_net_adapter\n\
\t-D first_primary_disk_adapter\n\
\t-H first_secondary_disk_adapter\n\
\t[-d second_primary_disk_adapter]\n\
\t[-h second_secondary_disk_adapter]\n\
\t[-u udp_port]\n\
\t[-l logifle]\n\
\t[-e exports_file]\n\
\t[-t timeout_value]\n\
\t[-g grace_period]\n\
\t[-p number_of_pings]\n\
\t[{-r | -b} daemon_start]\n"
HANFS2 "Usage: %1$s [-r |-b]\n"
HANFS3 "%1$s: 1831-524 The configuration file, %2$s, already exists.\n\
Please remove it and try again\n" 
HANFS4 "%1$s: 1831-525 %2$s not found\n"
HANFS5 "%1$s: 1831-526 Invalid UDP value %2$d\n"
HANFS6 "%1$s: 1831-527 Error in updating %2$s\n"
HANFS7 "Usage: %1$s -T Twin_primary_hostname\n\
\t-t twin_secondary_hostname\n\
\t-N primary_net_adapter\n\
\t-n secondary_net_adapter\n\
\t-D primary_disk_list\n\
\t-d secondary_disk_list\n\
\t -u udp_port \n\
\t -l logfile \n\
\t -e exports_file \n\
\t -w timeout_value \n\
\t -g grace_period \n\
\t -p number_of_pings \n"

$set NFSO
$ /dev/kmem should not be translated
NFSO1 "\nusage: nfso [-c] -o option[%1$cnewvalue] [ -o ... ]\n"
NFSO2 "\tnfso -d option\n"
NFSO3 "\tnfso [-c] -a\n"
NFSO10 "nfso: 1831-534 %1$s is not a valid variable name.\n"
NFSO11 "nfso: 1831-535 Error occurred trying to set %1$s to %2$d.\n"
NFSO12 "nfso: 1831-536 Error occurred trying to set %1$s to %2$d. (default value)"
NFSO13 "nfso: 1831-537 Value %1$s to set variable %2$s is not a number.\n"
NFSO17 "nfso: 1831-541 Some parameters were not parsed.\n"
NFSO18 "nfso: 1831-542 Unknown switch.\n"
NFSO19 "\tnfso -l hostname\n"
NFSO20 "nfso: 1831-543 The client system call nfs_cntl(%1$d,%2$d,%3$d) failed.\n"
NFSO21 "nfso: 1831-546 If nfs_socksize is greater than sb_max, this operation will fail. \nUse the \"no\" command to set sb_max.\n"
NFSO22 "nfso: 1831-547 The '%1$c' option can only be used by root.\n"
NFSO23 "nfso: 1831-548 The nfs kernel extension is not loaded.  Only root can load kernel extensions.\n"

$set KRPCXDR
KRPCXDR01 "Portmapper not responding: giving up.\n"
KRPCXDR02 "Portmapper not responding: still trying.\n"
KRPCXDR03 "Portmapper ok.\n"

$set NFSLD
NFSLOAD1 "load_nfs: 1831-548 The nfs kernel extension is not loaded.  Only root can load kernel extensions.\n"
