$ ENGLISH_SOURCE_VERSION= 1.3.1.2
$ @(#)46	1.3  src/msg/bos/base/snmpd.msg, bosmsg, msg411, 9437B411a 9/13/94 17:08:20
$
$
$ COMPONENT_NAME: SNMPD 
$
$ ORIGINS: 27
$
$ FUNCTIONS: N/A
$
$ (C) COPYRIGHT International Business Machines Corp.  1992, 1994
$ All Rights Reserved
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Licensed Materials - Property of IBM
$
$ messages for snmp daemon
$
$ Do not translate %s, %c, %d, etc.  These will be replaced by file names, 
$ commands, numbers, etc.
$
$ Also please note that the use of the word "argument" is the strict 
$ interpretation of ANSI 'C' syntax.  No other meaning is to be used or 
$ allowed in translation.
$
$ The following words are not to be translated.  They have specific meanings
$ to either snmp, communication protocol, mosy, 'C', or UNIX.
$ 
$ WORDS NOT TO BE TRANSLATED:
$ ARP ADP DGR GENERR INF IP MIB NOTOK NetAddress Netmask OBJ OID ObjectSyntax 
$ PDU PDUs PS PY RTE SIOCADDRT SIOCDELRT SIOCGNETOPT SIOCSNETOPT SMI SMUX 
$ SND SNDBUF SNMP SNMPD buffer SRC SRV SXS SYS 
$ TCP TRP UDP UNX VAL VEW VPD VarBindList addfd adios alloc arginit arp 
$ arptab auth aux ava badOperation badversion bfr compar config  
$ datagram db defViewTrapDest dest dev dgram envinit  
$ errstr fd fd2fi fdinfo fdx fi genErr getds gethostname getkmem getrlimit 
$ getsockopt gettimeofday hostname icheck id ifa ifindex ifwithnet importobjects
$ init insize io ioctl ip ipaddr isode kmem len lib localAgent logfile 
$ logname loopback lseek maxpacket mib moresyntax msg netaddress netmask 
$ newlogsize nfds nosuchsubtree obj oid oids ot pb pdu pe pe2ps pepy physaddress
$ process ps ps2pe pullup qb qb2str qbstring qbuf querytimeout rcv readconfig 
$ recvfrom resolveot rfx rmtab rn rtaddr rtentry netmask sethostname 
$ setmaxlogsize setrlimit setsocketopt setsockopt settab showmib sigvec 
$ simpleOpen smux smuxPeer smuxTree smuxio smuxtimeout snd sndrcv snmp 
$ snmpd snmpin sockaddr sodmerrno src srchdr stat statcode stopsrc str2elem 
$ str2qb str2sa tcp tcptab text2inst text2obj text2oid ts ttl udptab ut util 
$ vmunix vu community
$
$ AND any words that end in an extension (.c, .o) or are joined by 
$ underscores (ps_io) are not to be translated.

$quote "
$set MS_GENERAL

GENERA_1 "%s: Out of Memory"

GENERA_2 "Out of Memory"

GENERA_3 "Out of Memory (%d needed)"

$ dgram.c in libsnmp.a
$set MS_DGRAM

DGRAM_1 "%s called with %d, but no fdinfo"

DGRAM_2 "dgram_rfx, qbuf malloc failed"

DGRAM_3 "dgram_rfx, recvfrom failed: %s"

DGRAM_4 "received %d bytes from %s"

DGRAM_5 "sending %d bytes to %s"


$ io.c in libsnmp.a
$set MS_IO

IO_1 "cannot allocate fdinfo structure"

IO_2 "cannot allocate sockaddr_in structure"

IO_3 "fd2fi called with %d, but no fdinfo"

IO_4 "addfd failed"

IO_5 "cannot open snmp socket: %s"

IO_6 "cannot bind snmp port: %s"

IO_7 "cannot getsockopt SO_SNDBUF"

IO_8 "cannot open PS for snmp port"

IO_9 "cannot setup PS dgram service"

IO_10 "bound file descriptor %d to port %d %s %s"

IO_11 "listening"
IO_12 "host:"


$ moresyntax.c in libsnmp.a
$set MS_MORES

MORES_1 "moresyntax: no syntax defined for object \"%s\"\n"

MORES_2 "moresyntax: lost syntax for object \"%s\"\n"

MORES_3 "moresyntax: no \"%s\" object\n"


$ new_messages.c in libsnmp.a
$set MS_NEWMESS

NEWMESS_1 "value unnecessary for get operation"

NEWMESS_2 "need variable=value for set operation"

NEWMESS_3 "unknown variable \"%s\""

NEWMESS_4 "no syntax defined for object \"%s\""

NEWMESS_5 "invalid value for variable \"%s\": \"%s\""

NEWMESS_6 "encoding error for variable \"%s\""


$ objects.c in libsnmp.a
$set MS_OBJECTS

OBJECTS_1 "duplicate objects, different syntax: \"%s\" and \"%s\""

OBJECTS_2 "duplicate objects: \"%s\" and \"%s\""

OBJECTS_3 "object \"%s\" has unknown syntax \"%s\""

OBJECTS_4 "object \"%s\" has unknown access \"%s\""

OBJECTS_5 "object \"%s\" has unknown status \"%s\"\n"

OBJECTS_6 "no distant parent for %s"

OBJECTS_7 "str2elem failure on \"%s\""

OBJECTS_8 "got name \"%s\", but not object"

OBJECTS_9 "unable to read %s: %s\n"

OBJECTS_10 "out of memory allocating %d objects"

OBJECTS_11 "out of memory allocating %d octets"

OBJECTS_12 "out of memory allocating %d sub-identifiers"

OBJECTS_13 "out of memory allocating %d identifiers"

OBJECTS_15 "only one compiled file is allowed"

OBJECTS_16 "malformed line: \"%s\""

OBJECTS_17 "unable to locate object parent \"%s\" for \"%s\""

OBJECTS_18 "Unresolved parent for \"%s\""

OBJECTS_19 "str2elem failure on \"%s\""

OBJECTS_20 "duplicate object name: \"%s\""

OBJECTS_21 "invalid OID (%s) for \"%s\" in compiled file"


$ smux.c in libsnmp.a
$set MS_SMUX

SMUX_1 "%s: unknown host"

SMUX_2 "failed"

SMUX_3 "%s: missing parameter"

SMUX_4 "%s: SMUX not initialized"

SMUX_5 "%s: SMUX not opened"

SMUX_6 "SMUX error %s%d"


$ smuxentry.c in libsnmpd.a
$set MS_SMUXENTRY

ENTRY_1 "WARNING: unable to read %s\n"


$ sndrcv.c in libsnmp.a
$set MS_SNDRCV

SNDRCV_1 "no active UDP port"

SNDRCV_2 "request-id mismatch (received %d, expected %d)"

SNDRCV_3 "No response"

SNDRCV_4 "receive timed out%s"


$ syntax.c in libsnmp.a
$set MS_SYN

SYN_1 "%u days, " 

SYN_2 "%u hours, "

SYN_3 "%u minutes, "

SYN_4 " seconds (%u timeticks)"


$ values.c in libsnmp.a
$set MS_VALUES

VALUES_1 "no syntax defined for object \"%s\""

VALUES_2 "no value defined for object \"%s\""

VALUES_3 "encoding error for variable \"%s\""

VALUES_4 "GENERR in s_icheck: bad offset %d"

VALUES_5 "GENERR in s_icheck: no syntax defined for object \"%s\""


$ config.c in snmpd
$set MS_CONFIG

CONFIG_1 "malformed directive: \"%s\""

CONFIG_2 "unknown directive: \"%s\""

CONFIG_3 "refreshing %s (%d)"

CONFIG_4 "%s: host address %s mis-formed"

CONFIG_5 "%s: netmask %s mis-formed"

CONFIG_6 "invalid access mode \"%s\""

CONFIG_7 "%s: unknown OID \"%s\""

CONFIG_8 "unknown object \"%s\""

CONFIG_9 "no getfnx for object \"%s\""

CONFIG_10 "non-initializable getfnx for object \"%s\""

CONFIG_11 "no syntax defined for object \"%s\""

CONFIG_12 "invalid value for variable \"%s.0\": \"%s\""

CONFIG_13 "Log file \"size\" value, %s, cannot be converted into an integer.  The default value 0 (unlimited) will be used."

CONFIG_14 "Log file \"size\" value was not specified.  The default value 0 (unlimited) will be used."

CONFIG_15 "Invalid debug level in %s: %s. The default value 0 (minimal) will be used."

CONFIG_16 "Log file \"level\" value was not specified.  The default value 0 (minimal) will be used."

CONFIG_17 "duplicate view \"%s\" for trap sink \"%s\""

CONFIG_18 "too many views starting with \"%s\""

CONFIG_19 "view \"%s\" is for traps"

CONFIG_20 "duplicate view \"%s\""

$ Used in one advise statement as a comparison of a variable in a tree struct
CONFIG_21 "already under" 
CONFIG_22 "succeeding"

CONFIG_23 "f_smux: duplicate smux peer entry: %s"

CONFIG_24 "%s: host address %s unknown"

CONFIG_25 "maxpacket must be larger than %d"

CONFIG_26 "%s: invalid value %s"

CONFIG_27 "%s: value must be 0, or greater than %d"

CONFIG_28 "setsockopt(SO_SNDBUF) failed: %s"

CONFIG_29 "cannot read %s: %s"

CONFIG_30 "%s not owned by root"

CONFIG_31 "lost fi for fd %d"

CONFIG_32 "cannot open PS for snmp port"

CONFIG_33 "cannot setup PS dgram service"

$ devices.c in snmpd
$set MS_DEVICES

DEVICES_1 "open_device: could not allocate an fdinfo structure"

DEVICES_2 "%s called for %s doing a %d"

DEVICES_3 "open_device: Unable to open AF_NDD socket."

DEVICES_4 "open_device: Unable to connect to device driver."

DEVICES_5 "%s: Unknown method for device %s"

DEVICES_6 "%s: ioctl on device %s failed with errno = %d"

DEVICES_7 "%s: Bad file descripter for device %s"

DEVICES_8 "open_device: errno = %d, Unable to bind to device driver."

$ ethernet.c in snmpd
$set MS_ETHERNET

ETHERNET_1 "o_ethernet: device data and/or device info are NULL"

$ fddi.c in snmp
$set MS_FDDI

FDDI_1 "%s: invalid operation: %d"

FDDI_2 "GENERR in %s: s_generic failed"

FDDI_3 "%s: time out on response"

FDDI_4 "%s: select failed"

FDDI_5 "%s: read error"

FDDI_6 "%s: write error"

FDDI_7 "%s: write error, partial write"

FDDI_8 "%s: Invalid response, check for another packet"

FDDI_9 "%s: malformed packet"

FDDI_10 "%s: unexpected packet"

FDDI_11 "%s: packet does not contain enough sections"

FDDI_12 ""%s: Unrequested frame"

FDDI_13 "%s: SMT Frame/SNMP PDU mismatch"

FDDI_14 "%s: unable to encode value"

FDDI_15 "%s: %s failed for device, %s"

FDDI_16 "get_fddi: Bad packet length"

FDDI_17 "Bad packet for get_tokenpaths"

FDDI_18 "%s: %s failed to transmit the packet"

FDDI_19 "%s: %s failed to parse the packet"

$ generic.c in snmpd
$set MS_GENERIC

GENERIC_1 "%s: device data and/or device info are NULL"

GENERIC_2 "s_genera: unexpected operation %d"

GENERIC_3 "s_genera: Promiscuous ioctl failed"

GENERIC_4 "s_genera: Could not rollback promiscuous set"

GENERIC_5 "%s: bad value"

GENERIC_6 "s_genera: Promiscuous get failed"

GENERIC_7 "s_rcvtable: deleting an address failed"

GENERIC_8 "s_rcvtable: retrieval of the receive table failed"

GENERIC_9 "s_rcvtable: adding an address failed"

$ interfaces.c in snmpd
$set MS_INTER

INTER_1 "gettimeofday"

INTER_2 "GENERR: s_ifAdminStatus: get_interfaces failed"

INTER_3 "s_ifAdminStatus: unexpected operation %d"

INTER_4 "GENERR: s_ifAdminStatus: no syntax defined for \"%s\""

INTER_5 "GENERR: s_ifAdminStatus: ioctl (SIOCGIFFLAGS) for \"%s\" failed: %s"

INTER_6 "interfaces group disabled"

INTER_7 "cannot determine token speed for \"%s\": %s"

INTER_8 "add interface %d: %s 0x%x with mask 0x%x"

INTER_9 "base ifnet pointer changed. re-initializing interfaces"

INTER_10 "interface pointers changed. re-initializing interfaces"

INTER_11 "SIOCGIFNETMASK on %s"

INTER_12 "add address: %d/%s with mask 0x%x"

INTER_13 "trap_interface: lost OI for %s"

INTER_14 "trap_interface: o_integer failed"

INTER_15 "no such interface as \"%s\""

INTER_16 "malformed attribute \"%s = %s\""

INTER_17 "unknown attribute \"%s = %s\""

INTER_18 "base ndd pointer changed. re-initializing interfaces"

INTER_19 "doit_device: file descripter not related to a known interface"

INTER_20 "doit_device: file descripters are confused.  Closing to start over."

INTER_21 "doit_device: file descripter cannot be made non-blocking"

INTER_22 "doit_device: recvfrom has an error of %d"

$ ip.c in snmpd
$set MS_IP

IP_1 "GENERR: %s: ioctl (%s) for \"%s\" failed: %s"

IP_2 "GENERR: %s: get_interfaces failed"

IP_3 "GENERR: gettimeofday"

IP_4 "GENERR: s_ip_route: get_routes failed"

IP_5 "GENERR in s_ip_route: bad offset %d"

IP_6 "GENERR: s_address: no syntax defined for \"%s\""

IP_7 "GENERR: sr_commit: SIOCDELRT of dest: %s gateway: %s failed: %s"

IP_8 "GENERR: sr_commit: SIOCADDRT of dest: %s gateway: %s failed: %s"

IP_9 "GENERR: s_address: get_arptab failed"

IP_10 "no interface for arp entry %s"

IP_11 "GENERR in s_address: bad offset %d"

IP_12 "GENERR: s_address: no syntax defined for \"%s\""

IP_13 "GENERR: sa_commit: %s of %s failed: %s"

IP_14 "sa_ifindex: ifa_ifwithnet failed on %s"

IP_15 "cannot locate address for arp entry %s"


$ logging.c in snmpd
$set MS_LOG

LOG_1 "invalid logname: \"%s\""

LOG_2 "%s cannot be opened for logging: %s"

LOG_3 "%s is not configured for logging"

LOG_4 "logging started at level %d"

LOG_5 "logging to %s is terminating"

LOG_6 "%s is not configured for logging"

LOG_7 "current system max limit on the log file (%d) is less than the requested max size (%d)"

LOG_8 "getrlimit failed: %s"

LOG_9 "setrlimit failed: %s"

LOG_10 "%s cannot write to log file %s: %s"

LOG_11 "%srotating log files"

$ These must remain consecutive and in order
LOG_12 ""
LOG_13 "EMERGENCY: "
LOG_14 "ALERT: "
LOG_15 "CRITICAL: "
LOG_16 "FATAL: "
LOG_17 "EXCEPTIONS: "
LOG_18 "WARNING: "
LOG_19 "NOTICE: "
LOG_20 "PDUS: "
LOG_21 "TRACE: "
LOG_22 "DEBUG: "
LOG_23 "DEFAULT: "


$ mib.c in snmpd
$set MS_MIB

MIB_1 "\"%s\" not in %s (warning)"

MIB_2 "variable \"%s.0\" has no value (warning)"

MIB_3 "gettimeofday"

MIB_4 "lseek of 0x%x for \"%s\" in kmem failed"

MIB_5 "read of \"%s\" from kmem failed"

MIB_6 "GENERR: parse_set: new_set failed"

MIB_7 "%s, cannot nlist"

MIB_8 "%s, cannot read"

MIB_9 "text2oid(\"0.0\") failed"


$ routes.c in snmpd
$set MS_ROUTE

ROUTE_1 "duplicate routes for destination %d/%s"

ROUTE_2 "add route: %d/%s%s%s on interface 0x%x with flags %d"


$ smux_g.c in snmpd
$set MS_SMUXG

SMUXG_1 "no syntax defined for object \"%s\""


$ smuxserv.c in snmpd
$set MS_SMUXS

SMUXS_1 "addfd failed"

SMUXS_2 "start_tcp_server failed for SMUX: %s"

SMUXS_3 "bound SMUX fd %d to port %d"

SMUXS_4 "%s: out of memory (SMUX %s)"

SMUXS_5 "fdx_setup: %s (SMUX %s)"

SMUXS_6 "SMUX relation started with (%s)"

SMUXS_7 "lost smuxPeer block for %d"

SMUXS_8 "lost peer%s"

SMUXS_9 "SMUX packet from (%s)"

SMUXS_10 "under"

SMUXS_11 "contains"

SMUXS_12 "delete"

SMUXS_13 "sending register response to (SMUX %s)"

SMUXS_14 "sending request to %s"

SMUXS_15 "no response after %d seconds %s"

SMUXS_16 "received response from %s"

SMUXS_17 "missing variable in get response %s"

SMUXS_18 "simpleOpen rejected (%s): %s (SMUX %s)"

SMUXS_19 "bad version: %d (SMUX %s)"

SMUXS_20 "SMUX open: %d %s \"%s\" (%d/ %s)"

SMUXS_21 "SMUX close: %s (%s)"

SMUXS_22 "SMUX trap: (%d %d) (%s)"

SMUXS_23 "bad operation: %d (SMUX %s)"

SMUXS_24 "unexpected operation: %d%s(SMUX %s)"

SMUXS_25 "reserved subtree: %s %s %s (SMUX %s)"

SMUXS_26 "no such subtree: %s (SMUX %s)"

SMUXS_27 "no such registration: %s (SMUX %s)"

SMUXS_28 "SMUX register: %s %s in = %d out = %d (%s)"

SMUXS_29 "bad subtree: %s (SMUX %s)"

SMUXS_30 "unexpected response: %d%s"

SMUXS_31 ", missing identity "

SMUXS_32 ", identity already exists "

$ snmpd.c in snmpd
$set MS_SNMPD

SNMPD_1 "bad version: %d (%s)"

SNMPD_2 "%s: out of memory (%s)"

SNMPD_3 "authentication error: invalid community name: %s"

SNMPD_4 "too big on a too_big response"

SNMPD_5 "bad operation: %d (%s)"

SNMPD_6 "invalid debug level: %s"

SNMPD_7 "invalid port: %s"

SNMPD_8 "invalid smux_port: %s"

SNMPD_9 "invalid configfile: %s"

SNMPD_10 "Usage: %s [-c config_file] [-d level] [-f logfile]"

SNMPD_11 "processing signal %d"

SNMPD_12 "%s is not configured for logging"

SNMPD_13 "odm_initialize failed"

SNMPD_14 "init_smuxio failed: %s"

SNMPD_15 "%s (%d) is starting"

SNMPD_16 "setsockopt SO_LINGER failed on SMUX_CLIENT %d, errno %d"

SNMPD_17 "setsockopt SO_LINGER failed on SMUX %d, errno %d"

SNMPD_18 "%s (%d) is terminating"

SNMPD_19 "no listening fd's"

SNMPD_20 "select failed: %s"

SNMPD_21 "selected fd %d, but no fi"

SNMPD_22 "cannot fork"

SNMPD_23 "cannot open /dev/null"

SNMPD_24 "cannot initialize mib objects"

SNMPD_25 "init_io failed: %s"

SNMPD_26 "sigvec failed for sig %d: %s"

SNMPD_27 "snmp_receive_signal: Set timer failed."

SNMPD_28 "snmp_receive_signal: Set timer is reseting an existing timer."

$ src.c in snmpd
$set MS_SRC

SRC_1 "src_status: cannot execute stat()"

SRC_2 "not root user, cannot access long status"

SRC_3 "\nNot root user, cannot access long status.\n\n"

SRC_4 "stopsrc issued"

SRC_5 "traceson: logging is already enabled"

SRC_6 "\ntraceson: logging is already enabled.\n\n"

SRC_7 "is not configured for logging.\n\n"

SRC_8 "\ntracesoff: logging is already disabled.\n\n"

SRC_9 "tracesoff: logging is already disabled"

SRC_10 "\nStop trace.\n\n"

SRC_11 "cannot add fd"

SRC_12 "doit_src: recvfrom"

SRC_13 "\nStart trace.\n\n"

SRC_14 "\nError: cannot determine the user.\n\n"

SRC_15 "\nStart subserver: not supported.\n\n"

SRC_16 "\nStop subserver: not supported.\n\n"

SRC_17 "\nStatus of subserver: not supported.\n\n"

SRC_18 "\nTrace subserver: not supported.\n\n"

SRC_19 "\nIllegal command.\n\n"

SRC_20 "No COMMUNITY configured"

SRC_21 "No TRAP configured"

SRC_22 "No SMUX configured"


$ system.c in snmpd
$set MS_SYSTEM

SYSTEM_1 "GENERR: gettimeofday"

SYSTEM_2 "GENERR: gethostname"

SYSTEM_3 "GENERR in o_system: unable to open config file: error %d"

SYSTEM_4 "GENERR in o_system: could not remove tmp file"

SYSTEM_5 "GENERR in s_system: qb2str failed"

SYSTEM_6 "GENERR: sethostname failed: %s"

SYSTEM_7 "GENERR in s_system: unable to open config file: error %d"

SYSTEM_8 "GENERR in s_system: unable to write config file: error %d"

SYSTEM_9 "Machine Type:"

SYSTEM_10 "Processor id:"

SYSTEM_11 "Networking Software:"

SYSTEM_12 "Operating System Software:"

SYSTEM_13 "version:"


$ tcp.c in snmpd
$set MS_TCP
 
TCP_1 "add connection: %s"

$ timers.c in snmpd
$set MS_TIMERS

TIMERS_1 "Ran out of items in timer list."

$ token_ring.c in snmpd
$set MS_TOKENRING

TOKENRING_1 "%s: device data and/or device info are NULL"

TOKENRING_2 "s_tokenring: unexpected operation %d"

$ trap.c in snmpd
$set MS_TRAP

TRAP_1 "cannot initialize trap structure: out of memory"

TRAP_2 "cannot get host name, so no traps"

TRAP_3 "%s: unknown host, so no traps"

TRAP_4 "cannot initialize loopback address: out of memory"

TRAP_5 "cannot send trap: no such object: \"%s\""

TRAP_6 "cannot send trap: no value defined for object \"%s\""

TRAP_7 "gettimeofday"

TRAP_8 "sent trap (%s:%d) to %s"

TRAP_9 "sent trap (%s) to %s"


$ udp.c in snmpd
$set MS_UDP

UDP_1 "duplicate listeners: %s"

UDP_2 "error:  add listener: %s"


$ unix_g.c in snmpd
$set MS_UNIXG

UNIXG_1 "cannot nlist"

UNIXG_2 "\"%s\" not in /vmunix (warning)"


$ view.c in snmpd
$set MS_VIEW

VIEW_1 "no such view as %s for community \"%s\""

VIEW_2 "address type unsupported"

VIEW_3 "unknown OID \"%s\""

VIEW_4 "init_def_view: Cannot add default views"


$ Mosy Messages
$set MS_INFO

INFO_1 "invalid inputfile \"-\"\n"

INFO_2 "unable to write "

INFO_3 "-- automatically generated by %s, do not edit!\n\n"

INFO_4 "unable to read "

INFO_5 "yyparse failed, rc = %d"

INFO_6 "warning: \"%s\" not created: mibII.my must follow smi.my \n"

INFO_7 "warning: \'%s\' not created\n"

INFO_8 "cannot remove file %s\n"

INFO_9  "usage: mosy -o output_defs_file [-s] inputfile...\n"
INFO_10 "       mosy -x output_desc_file [-o output_defs_file] [-s] inputfile...\n"
INFO_11 "       mosy -c output_c_file [-x output_desc_file] [-o output_defs_file] [-s] inputfile...\n"

INFO_12 "Warning: %s"

INFO_13 "line %d: "

INFO_14 "last token read was \"%s\"\n"

INFO_15 "-- object definitions compiled from %s"

INFO_16 "object %s contains a `-' in its descriptor"

INFO_17 "descriptor of counter object %s doesn't end in `s'"

INFO_18 "no module name associated with symbol"

INFO_19 "write error - %s"

INFO_20 "no syntax associated with object type"

INFO_21 "need an object identifier"

INFO_22 "need a sub-identifier, not a boolean"

INFO_23 "need a sub-identifier, not a string"

INFO_24 "haven't written symbol table for values yet"

INFO_25 "need a sub-identifier, not a list of values"

INFO_26 "need a sub-identifier, not an object identifier"

INFO_27 "need a sub-identifier, not NULL"

INFO_28 "unknown value: %d"

INFO_29 "need an integer, not a string"

INFO_30 "haven't written symbol table for values yet"

INFO_31 "need an integer, not a list of values"

INFO_32 "need an integer, not an object identifier"

INFO_33 "need an integer, not NULL"

INFO_34 "TRUE"
INFO_35 "FALSE"

INFO_36 "NULL"

INFO_37 "# MIB variable descriptions\n"

INFO_38 "/* a mapping to syntax name */\n\n"

INFO_39 "/* automatically generated by %s, do not edit! */\n"

INFO_40 "Warning Null oid in defineoid"

INFO_41 "OID name clash %s => %s & %s"

INFO_42 "unknown Object Identifier '%s'"

INFO_43 "Warning: %s imported & exported!"


$ Snmpinfo Messages
$set MS_SINFO

SINFO_1 "Usage: %s [-d] host community var[.inst]...\n"

SINFO_2 "Usage: %s [-d] host community var[.inst][=value]...\n"

SINFO_3 "%s: unknown host: %s\n"

SINFO_4 "%s: could not bind UDP port for requests\n"

SINFO_5 "%s: cannot determine local hostname\n"

SINFO_6 "Version: %s: %s\n"

SINFO_7 "Invalid debug level: %s\n"

SINFO_8 "Invalid port: %s\n"

SINFO_9 "Invalid func \"%s\"\n"

SINFO_10 "Invalid number of tries: %s\n"

SINFO_11 "value unnecessary for get operation\n"

SINFO_12 "need variable=value for set operation\n"

SINFO_13  "unexpected message type %d"

SINFO_14 "%s at position %d\n"

SINFO_15 "init_io failed"

SINFO_16 "Usage: %s [-d level] [-o objectsfile]... [-p port] [-v]\n"

SINFO_17 "unknown operation \"%s\""

SINFO_18 "ambiguous operation, it could be one of:%s"

SINFO_19 "Operations:\n"

SINFO_20 "help [commands ...]\n"  $ do not translate help

SINFO_21 "    with no arguments, lists operations which may be invoked\n"

SINFO_22 "    otherwise prints help for each operation given\n"

SINFO_23 "compile [-f] [file]\n"

SINFO_24 "    -f:   brief output (default to stdout)\n"

SINFO_25 "    file: output file (default %s)\n"

SINFO_26 "unable to write: %s"

SINFO_27 "%d objects written to %s"

SINFO_28 "error %d"

$ These need to be in order and consecutive
SINFO_29 "dump a portion of the MIB"
SINFO_30 "perform get operation"
SINFO_31 "print help information"
SINFO_32 "perform powerful get-next operation"
SINFO_33 "perform set operation"
SINFO_34 "write compiled objects file"

SINFO_35 "Usage:  "
SINFO_36 "[-v] [-c community] [-d level] [-h hostname]"
SINFO_37 "[-o objectsfile]... "
SINFO_38 "[-t tries] "
$ This following lines should not be translated.  It is already in words that 
$ are protocol meaningful, when untranslated.
SINFO_39 "variable.instance..."
SINFO_40 "variable.instance=value"

