$ @(#)67	1.12  src/tcpip/usr/sbin/named/named.msg, tcpnaming, tcpip411, GOLD410 6/3/93 14:17:16
$ 
$ COMPONENT_NAME: TCPIP named.msg
$ 
$ FUNCTIONS: data 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ messages for named
$ The range of message numbers assigned this file are 001-042
$
$len 
$quote "         define quote character.
$set MS_NAMED        Define initial set#

$ The messages with (used in new named) are for the tcpip325 version
$ of named.  The messages labeled (not used in new named) are obsolete
$ messages for the named in tcpip325.
$ new messages ar at the end of this file.
$ db_dump  
$ Do not translate %s, %d, %1$s, etc. 
CKFILNM              "%s.chk"
$ Do not translate .chk . (no longer used message) 
DMPTIM               "; Dumped at %s."
$ The time that the current database was printed out.(used in new named)
LOWMIN               "named: 0826-001 %d root hints... (too low)"
$ Do not translate named. (used in new named)
ROOTDMP              "; ---- Root hint cache dump ----\n"
$ Do not translate root, hint, cache. (used in new named)
CACHDMP              "; ---- Cache dump ----\n"
$ Do not translate Cache. (used in new named)
CACHDR               "; --- Cache and data ---\n"
$ Do not translate Cache. (used in new named)
HINTSHDR             "; --- Hints ---\n"
$ Do not translate Hints. (used in new named)
ORGMSG               "$Origin %s.\n"
$ (no longer used message) 
UNSPECDMP            "named: 0826-002 Dump T_UNSPEC: There is not enough memory available now: %m\n"
$ Do not translate T_UNSPEC, named. (used in new named)
USPECOUT             "named: 0826-003 Dump T_UNSPEC: There is an output buffer overflow.\n"
$ Do not translate T_UNSPEC, named . (used in new named)
USPECBUF             "%s\n"
$ Do not translate. 
UNK                  "???\n"
NUMBER               "%d"
$ Do not translate. 
PORTNUM              "%d"
$ Do not translate. 
CONVBUF              "%1$ld %2$lx %3$lx %4$lx"
$ Do not translate. 
CONVBUFIN            "x %1$ld %2$lx %3$lx %4$lx"
$ db_load (db_lookup has no messages)  
LOGFILERR            "%1$s: %m"
UNKTYP               "named: 0826-004 %2$s on line %1$d is not a known type.\n"
$ Do not translate named.
CPUTYPERR            "%1$s: 0826-005 The CPU type on line %2$d is too long."
$ Do not translate CPU. (used in new named) 
OSTYPERR             "%1$s: 0826-006 The OS type on line %2$d is too long."
$ Do not translate OS.  (used in new named)
INBUFERR             "named: 0826-007 Load T_UNSPEC: There is an input buffer overflow."
$ Do not translate T_UNSPEC, named. (used in new named)
USPECDATERR          "named: 0826-008 Load T_UNSPEC: Data is in an atob format that is not valid."
$ Do not translate T_UNSPEC, atob, named. (used in new named)
DBFMTERR             "%1$s: 0826-009 There is a database format error %3$s on line %2$d."
$ (used in new named)
UNKOPT               "named: 0826-010 Line %1$d: The $ option $%2$s is not valid.\n"
$ Do not translate named. (used in new named)
SEQERR               "named: 0826-011 Line %d: Specify a value."
$ Do not translate named. (used in new named)
EOFERR               "named: 0826-012 Line %d: Reached an end of file that was not expected."
$ Do not translate named. (used in new named)
UNKPROTO             "%1$s: 0826-013 Protocol %3$s on line %2$d is not known."
$ (used in new named)
UNKSERV              "%1$s: 0826-014 Service %3$s on line %2$d is not known."
$ (used in new named)
PORTNUMERR           "%1$s: 0826-015 Port number %2$d on line %3$d is too large.\n"
$ (used in new named)
PARENSERR            "%1$s: 0826-016 There is a missing ) character on line %2$d.\n"
$ (used in new named)
$ db_reload  
$ Do not translate ) . 
RELOAD               "Reloading the nameserver.\n"
$ Do not translate nameserver. (used in new named) 
$ db_save
SVNAM                "savename: %m."
$ Do not translate savename. (not used in new named)
SVDATA               "savedata: %m."
$ Do not translate savedata. (not used in new named)
SVHASH               "savehash: %m."
$ Do not translate savehash. (not used in new named)
SVINVQ               "saveinv: %m."
$ Do not translate saveinv. (not used in new named)
SVSTR                "savestr: %m."
$ Do not translate savestr. (not used in new named)
$ db_update  
ILLDATA              "named: 0826-017 %s has CNAME and other data (not permitted).\n"
$ Do not translate CNAME, and named. (used in new named)
$ ns_forw
FMEMERR              "forw: %m."
$ Do not translate forw. (not used in new named) 
$ ns_init
BOOTFILERR           "%1$s: %m."
$ (used in new named)
DIRERR               "Directory %1$s: %m.\n"
$ (used in new named)
UNKFLD               "%1$s: 0826-018 Field %3$s on line %2$d is not known.\n"
$ (used in new named)
INITZONE             "named: 0826-019 Do not specify more than %d zones.\n"
$ Do not translate named.
NOORG                "%1$s: 0826-020 There is a missing origin on line %2$d.\n"
$ (used in new named)
BADRFSH              "%1$s: 0826-021 Refresh time %3$s on line %2$d is not valid.  It is ignored.\n"
$ (used in new named)
CACHIGN              "%1$s: 0826-022 Cache refresh on line %2$d is ignored.\n"
$ (used in new named)
BADADDR              "named: 0826-023 %s is ignored.  It is not dotted quad."
$ Do not translate named. (used in new named)
NOSRCADDR            "%1$s: 0826-024 A source or an address on line %2$d is missing.\n"
$ (used in new named)
$ ns_main  
RESTRT               "Started again.\n"
$ named is started again. (used in new named)
SOCKSTRM             "socket  (SOCK_STREAM): %m."
$ Do not translate SOCK_STREAM, socket. (used in new named)
BINDERR              "bind(vs, %1$s[%2$d]): %m."
$ Do not translate vs, bind. (used in new named)
IFCONF               "get interface configuration: %m."
$ (used in new named)
IFFLGS               "get interface flags: %m."
$ (used in new named)
IFADDR               "get interface address: %m."
$ (used in new named)
MMEMERR              "main: 0826-025 There is not enough memory available now.\n"
$ (not used in new named)
NETMSK               "get netmask: %m."
$ Do not translate netmask. (used in new named)
DESTADDR             "get dst address: %m"
$ Do not translate dst. (used in new named)
DGRAMSOCK            "socket (SOCK_DGRAM): %m."
$ Do not translate socket, SOCK_DGRAM. (used in new named)
DGBIND               "bind(dfd %1$d, %2$s[%3$d]): %m."
$ Do not translate bind, dfd. (used in new named)
SETFDSZ              "named: 0826-026 The return from getdtablesize() is greater than FD_SETSIZE."
$ Do not translate getdtablesize, FD_SETSIZE, named.  (used in new named)
SELERR               "select: %m."
RCVERR               "Received from: %m."
$ (used in new named)
ACCERR               "accept: %m."
$ (used in new named)
USAGE                "Usage: named [-d DebugLevel] [-p PortNumber] [-b BootFile]\n"
$ Do not translate named.  (used in new named)
SQADDMEM             "sqadd: 0826-027 There is not enough memory available now.\n"
$ Do not translate sqadd. (used in new named)
PEERNMERR            "getpeername: %m."
$ Do not translate getpeername. (used in new named)
TODERR               "gettimeofday failed: %m."
$ Do not translate gettimeofday. (used in new named)
$ ns_maint  
MTZONE               "zoneref: socket: %m."
$ Do not translate zoneref, socket.
MKQERR               "zoneref: 0826-028 The res_mkquery system call failed."
$ Do not translate zoneref, res_mkquery.
MTNOSOA              "There is no SOA from server %1$s. zone %2$s. (len 0).\n"
$ Do not translate SOA, len.
MASTER2              "zoneref: 0826-029 Masters for secondary zone %s cannot be reached."
$ Do not translate zoneref. (used in new named)
$ ns_req
RQMEMERR             "ns_req: 0826-030 There is not enough memory available now.\n"
$ Do not translate ns_req. (used in new named)
DOMNMERR             "named: 0826-031 The domain name is too long: %s ...\n"
$ Do not translate named. (used in new named)
NAMERR               "Name_Too_Long"
$ Do not translate Name_Too_Long. (used in new named)
ZNLOOKERR            "InitDynUpdate: 0826-032 A lookup failed on zone %s.\n"
$ Do not translate InitDynUpdate. (used in new named)
RQNOSOA              "InitDynUpdate: 0826-033 Cannot find an SOA record for %s.\n"
$ Do not translate InitDynUpdate, SOA.
$ ns_resp
SERRETNXD            "A server returned NXDOMAIN && !aa."
$ Do not translate NXDOMAIN, aa.
MAXQRY               "named: 0826-034 MAXQUERIES limit is exceeded.  There is a possible data loop in resolving %s."
$ Do not translate MAXQUERIES, named. (used in new named)
RPLYERR              "named: 0826-035 Response from %s is not in a valid form.\n"
$ Do not translate named. (used in new named)
ROOTERR              "check_root: 0826-036 Cannot find root.\n"
$ Do not translate check_root, root.
MINROOTERR           "check_root: 0826-037 %d root servers after query to root server is less than minimum."
$ Do not translate check_root. (used in new named)
NOROOTCLS            "There are no root nameservers for class %s.\n"
$ Do not translate nameservers.
$ ns_stats  
STATOPEN             "named: 0826-038 Cannot open statistics file %s.\n"
$ Do not translate named.
$ SRC related messages  
SRC1                 "%s: 0826-039 ERROR: SRC is not found.  Continuing without SRC support.\n"
$ Do not translate SRC. (not used in new named)
SRC2                 "%1$s: 0826-040 ERROR: $2%d recvfrom.\n"
$ Do not translate recvfrom. (not used in new named)
SRC3                 "named: 0826-041 This option is not supported.\n"
$ Do not translate named. (used in new named)
SRC4                 "Debug."
$ Do not translate (not used in new named)
SRC5                 "Active."
$ Do not translate (not used in new named)
SRC6                 "Inactive."
$ Do not translate (not used in new named)
SRC7                 "named: 0826-042 setsockopt SO_DEBUG: error number %m."
$ Do not translate setsockopt, SO_DEBUG, named. (used in new named)
SRC8                 "Type."
$ Do not translate (not used in new named)
SRC9                 "Domain."
$ Do not translate (not used in new named)
SRC10                "Source File or Host."
$ Do not translate (not used in new named)
TXTERR            "%1$s: 0826-043 The TXT record on line %2$d truncated."
$ Do not translate TXT. (used in new named)
ZONEMEMERR	"Not enough memory to allocate initial zones array"
$ New messages for named
MMEMERR1              "getnetconf: 0826-025 There is not enough memory available now.\n"
$ New messages for named
$ New SRC related messages  
SRC1_NEW                 "%s: 0826-039 SRC is not found.  Continuing without SRC support.\n"
$ Do not translate SRC. (used in new named)
SRC2_NEW                 "$s: 0826-040 %d recvfrom.\n"
$ New messages for named. Do not translate recvfrom. (used in new named)
SRC4_NEW                 "Debug"
$ (used in new named)
SRC5_NEW                 "Active"
$ (used in new named)
SRC6_NEW                 "Inactive"
$ (used in new named)
SRC8_NEW                 "Type"
$ (used in new named)
SRC9_NEW                 "Domain"
$ (used in new named)
SRC10_NEW                "Source File or Host"
$ (used in new named)
