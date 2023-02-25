/* @(#)64	1.7  src/tcpip/usr/sbin/gated/parser.y, tcprouting, tcpip411, GOLD410 12/6/93 17:55:16 */
%{
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: default_gateway_option
 *		define_order
 *		define_statement
 *		interface_order
 *		interface_statement
 *		martian
 *		martian_list
 *		martian_option
 *		option
 *		option_list
 *		option_order
 *		option_statement
 *		parse_statement
 *		statement
 *		statements
 *		syslog_level
 *		syslog_mask
 *		trace_file
 *		trace_limits
 *		trace_option
 *		trace_option_global
 *		trace_options
 *		trace_options_none
 *		trace_replace
 *		trace_size
 *		trace_statement
 *		trace_trace_option
 *		trace_trace_options
 *		trace_trace_options_none
 *		yyerror
 *		yyparse
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  parser.y,v 1.60.2.6 1993/09/28 19:01:56 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */
#include "parse.h"
#include "krt.h"
#ifdef	PROTO_RIP
#include "rip.h"
#endif	/* PROTO_RIP */
#ifdef	PROTO_HELLO
#include "hello.h"
#endif	/* PROTO_HELLO */
#include "ospf.h"
#ifdef	PROTO_EGP
#include "egp.h"
#endif	/* PROTO_EGP */
#ifdef	PROTO_BGP
#include "bgp.h"
#endif	/* PROTO_BGP */
#ifdef	PROTO_DVMRP
#include "dvmrp.h"
#endif	/* PROTO_DVMRP */
#ifdef	PROTO_ISODE_SNMP
#include "snmp_isode.h"
#endif	/* PROTO_ISODE_SNMP */
#ifdef	 PROTO_ISIS
#include "isis.h"
#include "isis_test.h"
#include "isis_globals.h"
#endif	/* PROTO_ISIS */
  
char parse_error[LINE_MAX] = { 0 };
char *parse_filename = 0;

static	proto_t	parse_proto;			/* For correctly tagging ADV lists */
static	proto_t	parse_export_proto;		/* For checking metrics */
static	gw_entry	**parse_gwlist;
#if	defined(PROTO_SLSP) || defined(PROTO_EGP) || defined(PROTO_BGP)
static	gw_entry	*parse_gwp;		/* To fake a list for EGP and BGP */
#endif
static  char *parse_serv_proto;
static	int	parse_contig_masks = TRUE;	/* Masks must be contiguous */
static	int parse_group_index;
static	egp_neighbor *ngp, egp_group, *gr_ngp;
static	bgpPeer *bnp;
static	bgpPeerGroup *bgp;
static 	struct bgp_conf *bcp;
static struct AREA *ospf_AREA;
static if_info parse_ifi;	/* Used for configuring interfaces that do not exist */

u_int parse_state = PS_INITIAL;
proto_t protos_seen = 0;

PROTOTYPE(yyerror,
	  static void,
	  (const char *));

#define	free_string(s)	task_mem_free((task *) 0, (caddr_t) s); s = 0
#define	free_charstr(s)	task_mem_free((task *) 0, (caddr_t) (s)->ptr); (s)->ptr = (char *) 0; (s)->len = (s)->type = 0
#define	free_bytestr(s)	task_mem_free((task *) 0, (caddr_t) (s)->ptr); (s)->ptr = (byte *) 0; (s)->len = (s)->type = 0

#define	PARSE_ERROR	yyerror(parse_error); yynerrs++; YYERROR;

#define	PROTO_SEEN(proto) \
	if (BIT_TEST(protos_seen, RTPROTO_BIT(proto))) { \
	    sprintf(parse_error, "parse_proto_seen: duplicate %s clause", gd_lower(trace_state(rt_proto_bits, proto))); \
	    PARSE_ERROR; \
	} else { \
	    BIT_SET(protos_seen, RTPROTO_BIT(proto)); \
	    parse_proto = proto; \
	} \

#ifdef	PROTO_ASPATHS
static unsigned	aspath_state;
static asmatch_t *aspath_current;

#define	ASPATH_NEXT_STATE	if (aspath_state++ == ASP_BITS) { \
	sprintf(parse_error, "too many AS path regexp states"); \
	PARSE_ERROR; \
    }
#endif	/* PROTO_ASPATHS */

%}

/* Global */
%union {
    long	num;
    u_long	u_num;
    bytestr	hex;
    charstr	str;
    flag_t	flag;
    time_t	time;
    as_t	as;
    proto_t	proto;
    pmet_t	metric;
    pref_t	pref;
    if_addr	*ifap;
    if_addr_entry	*ifae;
    adv_entry	*adv;
    gw_entry	*gwp;
    sockaddr_un	*sockaddr;
#ifdef	PROTO_INET
    struct in_addr in_addr;
#endif	/* PROTO_INET */
    dest_mask	dm;
    struct {
	pmet_t metric;
	adv_entry *adv;
    } metric_adv ;
    struct {
	proto_t proto;
	adv_entry **import_list;
	adv_entry **export_list;
	gw_entry **gw_list;
    } proto_lists;
    config_entry *config;
    config_list *conflist;
#ifndef	GATED_LEXER
#ifdef	PROTO_OSPF
    ospf_config_router *ospf_router;
#endif	/* PROTO_OSPF */
#if	defined(PROTO_ASPATHS)
    asmatch_t	*asp_match;
    struct {
	u_short	begin;
	u_short end;
    } asp_range;
    struct {
	flag_t	start[1 + ASP_ACC/ASP_WORD];
	flag_t	state[1 + ASP_ACC/ASP_WORD];
    } asp_stack;
#endif	/* PROTO_ASPATHS */
#endif	/* GATED_LEXER */
}

/* Lexer tokens */
%token			L_EOS L_ERROR
%token	<num>		L_NUMBER
%token	<str>		L_STRING L_HNAME L_KEYWORD
%token	<hex>		L_HEXSTRING L_OCTETSTRING

/* Keyword tokens */    
%token	<num>		T_DIRECT T_INTERFACE T_INTERFACES T_LCLADDR
%token	<num>		T_PROTO T_METRIC T_METRICIN T_METRICOUT T_INFINITY T_DOWN
%token	<num>		T_PARSE T_ADV T_DEFAULT T_DEFAULTS
%token	<num>		T_YYDEBUG T_YYSTATE T_YYQUIT T_DEBUG
%token	<num>		T_SYSLOG T_UPTO T_EMERG T_ALERT T_CRIT T_ERR T_WARNING T_NOTICE T_INFO
%token	<num>		T_ON T_OFF T_GATEWAY T_PREFERENCE T_DEFAULTMETRIC T_NONBROADCAST T_BROADCAST T_TYPE
%token	<num>		T_POINTOPOINT T_MULTICAST T_NETMASK T_DEFINE
%token	<num>		T_NOP T_NOP1	/* Filler for conditional clauses */

%type	<num>		syslog_level syslog_mask

/* BGP and EGP */
%token	<num>		T_PEERAS T_LOCALAS T_NEIGHBOR T_GROUP
%token	<num>		T_NOGENDEFAULT T_VERSION T_DEFAULTIN T_DEFAULTOUT
%token	<num>		T_MAXUP T_SOURCENET T_PKTSIZE T_P1 T_P2
%type	<flag>		egp_trace_options
%type	<proto>		egp_prop_init    
%token	<num>		T_BGP T_HOLDTIME T_TEST T_KEEPALL T_SENDBUF T_RECVBUF T_SPOOLBUF
%type	<num>		bgp_linktype bgp_kbufsize
%type	<flag>		bgp_trace_options
%type	<proto>		bgp_prop_init    
%type	<dm>		bgp_allow
%type	<adv>		bgp_allow_list    

/* Needed for both EGP and AS paths */    
%token	<num>		T_EGP 

/* AS Paths */    
%token	<num>		T_ASPATH T_ORIGIN T_INCOMPLETE T_ANY
%type	<asp_match>	aspath_match
%type	<asp_stack>	aspath_regex aspath_sum aspath_symbol aspath_term
%type	<asp_range>	aspath_range
%type	<flag>		aspath_origin_list aspath_origins aspath_origin
%type	<proto>		aspath_prop_init    
/* Tags */
%token	<num>		T_TAG
%type	<metric>	tag_option tag_as tag_as_option tag
%type	<proto>		tag_prop_init

/* RIP and HELLO */
%token	<num>		T_TRUSTEDGATEWAYS T_SOURCEGATEWAYS
%token	<num>		T_RIP T_NORIPOUT T_NORIPIN T_NOCHECKZERO
%type	<config>	rip_interface_option rip_interface_options
%type	<flag>		rip_trace_options rip_v2_options
%type	<proto>		rip_prop_init

%token	<num>		T_HELLO T_NOHELLOOUT T_NOHELLOIN
%type	<config>	hello_interface_option hello_interface_options    
%type	<flag>		hello_trace_options
%type	<proto>		hello_prop_init

%token <num>		T_OSPF T_OSPF_ASE T_NETWORKS T_BACKBONE T_VIRTUALLINK T_STUB T_STUBHOSTS
%token <num>		T_RXMITINTERVAL T_INFTRANSDELAY
%token <num>		T_POLLINTERVAL T_ROUTERS T_NEIGHBORID T_TRANSITAREA
%token <num>		T_ENABLE T_DISABLE T_ELIGIBLE T_MONITORAUTHKEY
%token <num>		T_DD T_REQUEST T_LSU T_ACK T_RECEIVE T_LSA_BLD T_LSA_RX T_LSA_TX T_TRAP
%token <num>		T_EXPORTINTERVAL T_EXPORTLIMIT    
%type	<adv>		ospf_interface
%type	<conflist>	ospf_common ospf_nonbroadcast
%type	<config>	ospf_common_options ospf_common_option ospf_nonbroadcast_options ospf_nonbroadcast_option
%type	<metric>	ospf_cost_option ospf_cost ospf_type_option ospf_type
%type	<num>		ospf_auth_type
%type	<hex>		ospf_auth_key
%type	<u_num>		ospf_area ospf_area_option
%type	<num>		ospf_priority ospf_eligible_option
%type	<ospf_router>	ospf_router_eligible ospf_router_eligible_list
%type	<flag>		ospf_trace_options ospf_trace_option
%type	<proto>		ospf_prop_init ospf_ase_prop_init export_ospf_ase
%type	<dm>		ospf_area_range
%type	<adv>		ospf_area_range_list    


/* OSPF and ISIS */
%token	<num>		T_AREA T_SPF

/* OSPF and SLSP */
%token <num>		T_HELLOINTERVAL T_ROUTERDEADINTERVAL

/* OSPF and ISIS and SLSP */
%token	<num>		T_PRIORITY




/* SNMP */
%token	<num>		T_SNMP
%token	<port>		T_PORT
%type	<flag>		snmp_trace_options


/* Redirects */
%token	<num>		T_REDIRECT T_NOREDIRECTS T_ICMP
%type	<config>	redirect_interface_option redirect_interface_options
%type	<flag>		redirect_trace_options

/* Authentication */
%token	<num>		T_AUTHTYPE T_AUTHKEY T_NONE T_SIMPLE

/* Interface */
%token	<num>		T_PASSIVE T_SIMPLEX T_STRICTIFS T_SCANINTERVAL

/* Control */
%token	<num>		T_STATIC T_MARTIANS T_RETAIN
%token	<num>		T_EXPORT T_IMPORT
%token	<num>		T_HOST T_MASK T_MASKLEN T_REJECT T_BLACKHOLE T_RESTRICT T_ALLOW
%type	<dm>		route_dest
%type	<flag>		rts_options rts_option   

/* AS control */
%token	<num>		T_AS T_ROUTERID T_IGP

/* Tracing */
%token			T_OPTIONS T_NOINSTALL T_NOSEND T_NORESOLV
%token	<num>		T_TRACEOPTIONS T_EXCEPT T_TRACEFILE T_REPLACE T_SIZE T_FILES T_K T_M
%token	<num>		T_ALL T_GENERAL T_INTERNAL T_EXTERNAL T_ROUTE T_UPDATE T_KERNEL
%token	<num>		T_TASK T_TIMER T_NOSTAMP T_MARK

%type	<num>		number				/* A decimal, hex, or octet string number */
%type	<hex>		byte_string			/* A L_HEXSTRING or L_OCTETSTRING */
%type	<metric>	metric metric_option metric_restrict_option
%type	<time>		time
%type	<num>		onoff_option
%type	<flag>		trace_options trace_option trace_option_global trace_options_none
%type	<flag>		trace_trace_options_none trace_trace_options trace_trace_option
%type	<str>		trace_file
%type	<num>		trace_size    
%type	<config>	interface_define_options interface_define_option
%type	<num>		trace_replace
%type	<proto>		proto_exterior
%type	<proto_lists>	control_exterior import_interior export_interior proto_interior


%type	<proto> 	prop_default prop_direct prop_static prop_kernel
%type	<as>		as
%type	<adv>		as_list as_list_option
%type	<pref>		preference
%type	<metric>	preference_option preference_restrict_option
%type	<ifae>		interface_local    
%type	<sockaddr>	addr mask host network interface_addr
%token	<num>		T_INET    
%type	<in_addr>	in_addr
%token	<num>		T_GENDEFAULT
%type	<sockaddr>	default_gateway_option
%type	<gwp>		gateway
%type	<dm>		dest_mask host_mask network_mask all_mask default_mask martian
%type	<adv>		interface_all interface_all_adv interface_list interface_list_option interface_all_list
%type	<adv>		interface_gateway_list gateway_list
%type	<adv>		martian_list
%type	<flag>		martian_option
%type	<adv>		import_list import_listen
%type	<metric_adv>	import_option
%type	<adv>		prop_source prop_source_list
%type	<adv>		prop_restrict prop_restrict_list prop_restrict_list_option
%type	<metric_adv>	prop_source_option prop_metric_option
%type	<str>		string host_name
%type	<num>		port

%%

config		: /* Empty */
		| statements
		;

statements	: statement
		| statements statement
		;

statement	: parse_statement
		| trace_statement
		| option_order option_statement
		| interface_order interface_statement
		| define_order define_statement
		| proto_order proto_statement
		| route_order route_statement
		| control_order control_statement
		| error L_EOS
			{
				yyerrok;
			}
		| L_EOS
		;

/*  */

parse_statement	: T_YYDEBUG onoff_option L_EOS
			{
#if	YYDEBUG != 0
			    if ($2 == T_OFF) {
				yydebug = 0;
			    } else {
				yydebug = 1;
			    }
#endif	/* YYDEBUG */
			}
		| T_YYSTATE L_NUMBER L_EOS
			{
#if	YYDEBUG != 0
			    if ($2 < 0 || $2 > PS_MAX) {
				(void) sprintf(parse_error, "invalid yystate value: %d",
					       $2);
				PARSE_ERROR;
			    }
			    parse_state = $2;
#endif	/* YYDEBUG */
			}
		| T_YYQUIT L_EOS
			{
#if	YYDEBUG != 0
			    task_quit(0);
#endif	/* YYDEBUG */
			}
		;

/*  */

trace_statement	: T_TRACEOPTIONS trace_trace_options_none L_EOS
			{
			    if (!BIT_TEST(task_state, TASKS_TEST)) {
				if ($2) {
				    if (trace_flags) {
					trace_flags = trace_flags_save = $2;
					trace_display(trace_flags, TR_PARSE);
				    } else if (trace_opened) {
					trace_flags = trace_flags_save = $2;
					trace_display(trace_flags, TR_PARSE);
				    }
				} else {
				    if (trace_flags) {
					trace_flags = trace_flags_save = $2;
					trace_display(trace_flags, TR_PARSE);
					trace_off();
				    }
				}
			    }
			}
		| T_TRACEFILE trace_file trace_limits L_EOS
			{
			    if (!BIT_TEST(task_state, TASKS_TEST)) {
				if ($2.len) {
				    int append = (int) $2.type;

				    if (trace_flags) {
					trace_flags_save = trace_flags;
					trace_off();
				    }
				    trace_on($2.ptr, append);
				}
			    }
			}
		;

trace_file	: /* Empty */
			{
			    $$.len = 0;
			}
    		| string trace_replace
			{
			    $$ = $1;
			    $$.type = $2;
			}
    		;

trace_size	: number
		| number T_K
			{
			    $$ = $1 * 1024;
			}
		| number T_M
			{
			    $$ = $1 * 1024 * 1024;
			}
		;

trace_replace	: /* Empty */	{ $$ = TRUE; }
		| T_REPLACE	{ $$ = FALSE; }
		;

trace_limits	: /* Empty */
    		| T_SIZE trace_size T_FILES number
			{
			    if (parse_limit_check("file-size",
						  (u_int) $2,
						  TRACE_LIMIT_FILE_SIZE)) {
				PARSE_ERROR;
			    }
			    trace_limit_size = $2;

			    if (parse_limit_check("files",
						  (u_int) $4,
						  TRACE_LIMIT_FILE_COUNT)) {
				PARSE_ERROR;
			    }
			    trace_limit_files = $4;
			}
    		;

trace_trace_options_none
    		: T_NONE
			{
			    $$ = (flag_t) 0;
			}
		| trace_trace_options
			{
			    $$ = $$;
			}
		;
		
trace_trace_options
		: trace_trace_option
		| trace_trace_options trace_trace_option
			{
			    $$ = $1 | $2;
			}
		| trace_trace_options T_EXCEPT trace_trace_option
			{
			    $$ = $1 & ~$3;
			}
		;

trace_trace_option
		: trace_option
		| trace_option_global
    		;

trace_options_none
		: T_NONE
			{
			    $$ = (flag_t) 0;
			}
		| trace_options
		;

trace_options	: trace_option
		| trace_options trace_option
			{
			    $$ = $1 | $2;
			}
		;

/* Trace options that can only be specified globally */
trace_option_global
    		: T_NOSTAMP
			{
			    $$ = TR_NOSTAMP;
			}
		| T_MARK
			{
			    $$ = TR_MARK;
			}
		| T_PARSE
			{
			    $$ = TR_PARSE;
			}
		| T_ADV
			{
#ifdef	TR_ADV
			    $$ = TR_ADV;
#else	/* TR_ADV */
			    $$ = 0;
#endif	/* TR_ADV */
			}
		;

/* Trace options that can be specified anywhere */
trace_option	: T_ALL
			{
			    $$ = TR_ALL;
			}
		| T_GENERAL
			{
			    $$ = TR_INT|TR_EXT|TR_RT;
			}
		| T_INTERNAL
			{
			    $$ = TR_INT;
			}
		| T_EXTERNAL
			{
			    $$ = TR_EXT;
			}
		| T_ROUTE
			{
			    $$ = TR_RT;
			}
		| T_UPDATE
			{
			    $$ = TR_UPDATE;
			}
		| T_TASK
			{
			    $$ = TR_TASK;
			}
		| T_TIMER
			{
			    $$ = TR_TIMER;
			}
		| T_PROTO
			{
			    $$ = TR_PROTOCOL;
			}
		| T_KERNEL
			{
			    $$ = TR_KRT;
			}
		| T_BGP
			{
			    $$ = TR_BGP;
			}
		| T_EGP
			{
			    $$ = TR_EGP;
			}
		| T_HELLO
			{
			    $$ = TR_HELLO;
			}
		| T_ICMP
			{
			    $$ = TR_ICMP;
			}
		| T_OSPF
			{
			    $$ = TR_OSPF;
			}
		| T_RIP
			{
			    $$ = TR_RIP;
			}
		| T_SNMP
			{
			    $$ = TR_SNMP;
			}
		;

/*  */

define_order	: /*Empty */
			{
				if (parse_new_state(PS_DEFINE)) {
					PARSE_ERROR;
				}
			}
		;

define_statement
		: T_MARTIANS
			{
			    parse_contig_masks = FALSE;
			}
			'{' martian_list '}' L_EOS
			{
			    parse_contig_masks = TRUE;
			    if (parse_adv_append(&martian_list, $4, TRUE)) {
				PARSE_ERROR;
			    }
			}
		| T_AS as L_EOS
			{
			    if (inet_autonomous_system != $2) {
				if (inet_autonomous_system) {
				    inet_as_unlink(inet_autonomous_system);
				}
				inet_autonomous_system = $2;
				inet_as_link(inet_autonomous_system);
			    }
			}
    		| T_ROUTERID host L_EOS
			{
			    if (inet_parse_routerid($2, parse_error)) {
				PARSE_ERROR;
			    }
			}
		;


martian_list	: /* Empty */
			{
			    $$ = (adv_entry *) 0;
			}
		| martian_list martian martian_option L_EOS
			{
			    $$ = adv_alloc(ADVFT_DM | $3, (proto_t) 0);
			    $$->adv_dm = $2;
			    $$ = parse_adv_dm_append($1, $$);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		| martian_list error L_EOS
			{
			    yyerrok;
			}
		;

martian_option	: /* Empty */
			{
			    $$ = ADVF_NO;
			}
		| T_ALLOW
			{
			    $$ = ADVF_NO;
			}
		;

martian		: host_mask
		| network_mask
		| default_mask
		;

/**/

option_order	: /*Empty */
			{
				if (parse_new_state(PS_OPTIONS)) {
					PARSE_ERROR;
				}
			}
		;

option_statement
    		: T_OPTIONS option_list L_EOS
    		;

option_list	: option
		| option_list option
		;


option		: T_NOINSTALL
			{
			    krt_install = FALSE;
			}
		| T_GENDEFAULT preference_option default_gateway_option
			{
			    rt_default_needed = TRUE;
			    if (PARSE_METRIC_ISSET(&$2)) {
				rt_default_rtparms.rtp_preference = $2.metric;
			    }
			    rt_default_rtparms.rtp_router = $3;
			}
		| T_NOSEND
			{
			    task_newstate(TASKS_NOSEND, 0);
			}
		| T_NORESOLV
			{
			    task_newstate(TASKS_NORESOLV, 0);
			}
		| T_SYSLOG syslog_mask
			{
#ifdef	LOG_DAEMON
			    (void) setlogmask($2);
#else	/* LOG_DAEMON */
			    (void) sprintf(parse_error, "system does not support setlogmask()");
			    PARSE_ERROR;
#endif	/* LOG_DAEMON */
			}
		;


default_gateway_option
		: /* Empty */
			{
			    $$ = (sockaddr_un *) 0;
			}
		| T_GATEWAY
			{
			    parse_gwlist = &parse_gwp;
			}
		gateway
			{
			    $$ = sockdup($3->gw_addr);
			    gw_freelist(*parse_gwlist);
			    *parse_gwlist = parse_gwp = (gw_entry *) 0;
			}
			;

syslog_mask	:	syslog_level
			{
#ifdef	LOG_DAEMON
			    $$ = LOG_MASK($1);
#endif	/* LOG_DAEMON */
			}
		|	T_UPTO syslog_level
			{
#ifdef	LOG_DAEMON
			    $$ = LOG_UPTO($2);
#endif	/* LOG_DAEMON */
			}
		|	syslog_mask syslog_level
			{
#ifdef	LOG_DAEMON
			    $$ = $1 | LOG_MASK($2);
#endif	/* LOG_DAEMON */
			}
		;

syslog_level	: T_EMERG
			{
			    $$ = LOG_EMERG;
			}
		| T_ALERT
			{
			    $$ = LOG_ALERT;
			}
		| T_CRIT
			{
			    $$ = LOG_CRIT;
			}
		| T_ERR
			{
			    $$ = LOG_ERR;
			}
		| T_WARNING
			{
			    $$ = LOG_WARNING;
			}
		| T_NOTICE
			{
			    $$ = LOG_NOTICE;
			}
		| T_INFO
			{
			    $$ = LOG_INFO;
			}
		| T_DEBUG
			{
			    $$ = LOG_EMERG;
			}
    		;

/*  */

interface_order	: /*Empty */
			{
			    if (parse_new_state(PS_INTERFACE)) {
				PARSE_ERROR;
			    }
			}
		;

interface_statement	
		: T_INTERFACES
			{
			    PROTO_SEEN(RTPROTO_DIRECT);
			}
			'{' interface_stmts '}' L_EOS
		;

interface_stmts
		: /* Empty */
    		| interface_stmts interface_stmt L_EOS
    		| interface_stmts error L_EOS
			{
			    yyerrok;
			}
    		;

interface_stmt
		: T_OPTIONS interface_options
		| T_INTERFACE interface_all_list interface_define_options
			{
			    config_list *list = config_list_alloc($3, 0);

			    parse_adv_propagate_config($2, list, (proto_t) 0);

			    if (parse_adv_append(&int_policy, $2, TRUE)) {
				PARSE_ERROR;
			    }

			    config_list_free(list);
			}
		| T_DEFINE host
			{
			    bzero((caddr_t) &parse_ifi, sizeof (parse_ifi));

			    parse_ifi.ifi_addr = $2;
			}
			interface_type_options interface_common_options
			{
			    if (if_parse_add(&parse_ifi, parse_error)) {
				PARSE_ERROR;
			    }
			}
		;

interface_options
		: interface_option
		| interface_options interface_option
		;

interface_option
		: T_STRICTIFS
			{
			    task_newstate(TASKS_STRICTIFS, 0);
			}
		| T_SCANINTERVAL time
			{
			    if (parse_limit_check("scan-interval",
						  (u_int) $2,
						  KRT_LIMIT_SCANTIMER)) {
				PARSE_ERROR;
			    }
			    
			    timer_set(krt_task->task_timer[KRT_TIMER_IFCHECK],
				      $2,
				      (time_t) 0);
			}
		;

interface_type_options
		: T_BROADCAST host
			{
			    parse_ifi.ifi_addr_broadcast = $2;
			    BIT_SET(parse_ifi.ifi_state, IFS_BROADCAST);
			}
		| T_POINTOPOINT host
			{
			    parse_ifi.ifi_addr_local = $2;
			    BIT_SET(parse_ifi.ifi_state, IFS_POINTOPOINT);
			}
		| /* Empty */
			{
			}
		;

interface_common_options
		: /* Empty */
		| interface_common_options interface_common_option
		;

interface_common_option
		: T_NETMASK mask
			{
			    parse_ifi.ifi_subnetmask = $2;
			}
		| T_MULTICAST
			{
			    BIT_SET(parse_ifi.ifi_state, IFS_MULTICAST);
			}
		;

interface_define_options
		: interface_define_option
		| interface_define_options interface_define_option
			{
			    $$ = config_append($1, $2);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		;

interface_define_option
		: T_PREFERENCE preference
			{
			    $$ = config_alloc(IF_CONFIG_PREFERENCE_UP, (void_t) $2);
			}
		| T_DOWN T_PREFERENCE preference
			{
			    $$ = config_alloc(IF_CONFIG_PREFERENCE_DOWN, (void_t) $3);
			}
		| T_PASSIVE
			{
			    $$ = config_alloc(IF_CONFIG_PASSIVE, (void_t) TRUE);
			}
		| T_SIMPLEX
			{
			    $$ = config_alloc(IF_CONFIG_SIMPLEX, (void_t) TRUE);
			}
		| T_REJECT
			{
			    $$ = config_alloc(IF_CONFIG_REJECT, (void_t) TRUE);
			}
		| T_BLACKHOLE
			{
			    $$ = config_alloc(IF_CONFIG_BLACKHOLE, (void_t) TRUE);
			}
		;

interface_list_option
		: /* Empty */
			{
			    $$ = adv_alloc(ADVFT_ANY|ADVF_FIRST, (proto_t) 0);
			}
		| T_INTERFACE interface_list
			{
			    $$ = $2;
			}
		;

interface_gateway_list
		: interface_list_option
		| T_GATEWAY gateway_list
			{
			    $$ = $2;
			}
		;

interface_all_list
		: T_ALL
			{
			    $$ = adv_alloc(ADVFT_ANY|ADVF_FIRST, (proto_t) 0);
			}
		| interface_list
		;

interface_list
		: interface_all
			{
			    BIT_SET($1->adv_flag, ADVF_FIRST);
			    $$ = $1;
			}
		| interface_list interface_all
			{
			    $$ = $1;
			    if (parse_adv_append(&$$, $2, TRUE)) {
				PARSE_ERROR;
			    }
			}
		;

interface_all	: interface_all_adv
			{
			    switch ($1->adv_flag & ADVF_TYPE) {

			    case ADVFT_IFN:
				trace(TR_PARSE, 0, "parse: %s INTERFACE: %A*",
				      parse_where(),
				      $$->adv_ifn->ifae_addr);
				break;
				
			    case ADVFT_IFAE:
				trace(TR_PARSE, 0, "parse: %s INTERFACE: %A",
				      parse_where(),
				      $$->adv_ifae->ifae_addr);
			    }
			}
		;

interface_all_adv
		: host_name
			{
			    int valid_ifname = 0;

			    switch ($1.type) {
			    case L_KEYWORD:
			    case L_STRING:
				if ($1.len < IFNAMSIZ) {
				    valid_ifname++;
				}
				break;

			    default:
				break;
			    }

			    if (valid_ifname) {
				/* First try to look up as an interface name */

				$$ = if_parse_name($1.ptr, FALSE);
			    } else {
				/* Doesn't meet the criteria to be a name */

				$$ = (adv_entry *) 0;
			    }
			    
			    if (!$$ && !BIT_TEST(task_state, TASKS_NORESOLV)) {
				/* Try to resolve as a host name */

				sockaddr_un *addr = parse_addr_hostname($1.ptr);

				if (addr) {
				    $$ = if_parse_address(addr);
				}

			    }

			    if (!$$) {
				if (valid_ifname && !BIT_TEST(task_state, TASKS_STRICTIFS)) {
				    /* Assume it is a reference to an interface that does not exist */

				    $$ = if_parse_name($1.ptr, TRUE);
				} else {
				    /* Time to give up */
				    
				    (void) sprintf(parse_error, "Interface not found at '%s'",
						   $1.ptr);
				    PARSE_ERROR;
				}
			    }
				
			    free_charstr(&$1);
			}
		| addr
			{
			    $$ = if_parse_address($1);
			    if (!$$) {
				(void) sprintf(parse_error, "Interface not found at '%A'",
					       $1);
				PARSE_ERROR;
			    }

			    sockfree($1);
			}
		;

interface_local	: interface_addr
			{
			    if_addr *ifap;
			    
			    ifap = if_withlcladdr($1, FALSE);
			    if (ifap) {
				$$ = ifae_alloc(ifap->ifa_addrent_local);
			    } else {
				if (BIT_TEST(task_state, TASKS_STRICTIFS)) {
				    (void) sprintf(parse_error, "Invalid interface at '%A'",
						   $1);
				    PARSE_ERROR;
				} else {
				    $$ = ifae_locate($1, &if_local_list);
				}
			    }
			    sockfree($1);
			    trace(TR_PARSE, 0, "parse: %s LOCAL ADDRESS: %A",
				  parse_where(),
				  $$->ifae_addr);
			}
		;

interface_addr	: addr
		| host_name
			{
			    sockaddr_un *addr;

			    if (BIT_TEST(task_state, TASKS_NORESOLV)
				|| !(addr = parse_addr_hostname($1.ptr))) {
				PARSE_ERROR;
			    }
			    $$ = sockdup(addr);
			    free_charstr(&$1);
			}
		;

/*  */

proto_order	: /* Empty */
			{
			    if (parse_new_state(PS_PROTO)) {
				PARSE_ERROR;
			    }
			}
		;

proto_statement	: redirect_statement L_EOS
		| bgp_statement L_EOS
		| egp_statement L_EOS
		| hello_statement L_EOS
    		| ospf_statement L_EOS
		| rip_statement L_EOS
		| snmp_statement L_EOS
		;

/*  */


rip_statement	: T_RIP
			{
			    PROTO_SEEN(RTPROTO_RIP);

			    parse_gwlist = &rip_gw_list;

			    rip_trace_flags = trace_flags;
			}
    		onoff_option rip_group
			{
			    switch ($3) {
			    case T_OFF:
				BIT_RESET(rip_flags, RIPF_ON);
				break;

			    case T_ON:
				BIT_SET(rip_flags, RIPF_ON);
				break;
			    }
			}
		;

rip_group	: /* Empty */
		| '{' rip_group_stmts '}'
		;

rip_group_stmts	: /* Empty */
		| rip_group_stmts rip_group_stmt L_EOS
		| rip_group_stmts error L_EOS
			{
			    yyerrok;
			}
		;

rip_group_stmt	: T_PREFERENCE preference
			{
			    rip_preference = $2;
			}
		| T_DEFAULTMETRIC metric
			{
			    if (parse_metric_check(RTPROTO_RIP, &$2)) {
				PARSE_ERROR;
			    }
			    rip_default_metric = $2.metric;
			}
		| T_BROADCAST
			{
			    BIT_RESET(rip_flags, RIPF_CHOOSE);
			    BIT_SET(rip_flags, RIPF_BROADCAST);
			}
		| T_NONBROADCAST
			{
			    BIT_RESET(rip_flags, RIPF_CHOOSE|RIPF_BROADCAST);
			}
		| T_TRACEOPTIONS rip_trace_options
			{
			    rip_trace_flags =  $2;
			}
		| T_NOCHECKZERO
			{
			    BIT_SET(rip_flags, RIPF_NOCHECK);
			}
		| T_INTERFACE interface_all_list rip_interface_options
			{
			    config_list *list = config_list_alloc($3, 0);

			    parse_adv_propagate_config($2, list, RTPROTO_RIP);

			    if (parse_adv_append(&rip_int_policy, $2, TRUE)) {
				PARSE_ERROR;
			    }

			    config_list_free(list);
			}
		| T_TRUSTEDGATEWAYS gateway_list
			{
			    rip_n_trusted += parse_gw_flag($2, RTPROTO_RIP, GWF_TRUSTED);
			    if (!rip_n_trusted) {
				PARSE_ERROR;
			    }
			}
		| T_SOURCEGATEWAYS gateway_list
			{
			    rip_n_source += parse_gw_flag($2, RTPROTO_RIP, GWF_SOURCE);
			    if (!rip_n_source) {
				PARSE_ERROR;
			    }
			    BIT_SET(rip_flags, RIPF_SOURCE);
			}
		;

rip_interface_options
		: rip_interface_option
		| rip_interface_options rip_interface_option
			{
			    $$ = config_append($1, $2);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		;

rip_interface_option
		: T_NORIPIN
			{
			    $$ = config_alloc(RIP_CONFIG_NOIN, (void_t) TRUE);
			}
		| T_NORIPOUT
			{
			    $$ = config_alloc(RIP_CONFIG_NOOUT, (void_t) TRUE);
			}
		| T_METRICIN metric
			{
			    if (parse_metric_check(RTPROTO_RIP, &$2)) {
				PARSE_ERROR;
			    }
			    $$ = config_alloc(RIP_CONFIG_METRICIN, (void_t) $2.metric);
			}
		| T_METRICOUT metric
			{
			    if (parse_metric_check(RTPROTO_RIP, &$2)) {
				PARSE_ERROR;
			    }
			    $$ = config_alloc(RIP_CONFIG_METRICOUT, (void_t) $2.metric);
			}
		| T_VERSION L_NUMBER rip_v2_options
			{
			    flag_t flag;
			    
			    switch ($2) {
			    case 1:
				if ($3) {
				    (void) sprintf(parse_error, "options not valid with version 1");
				    PARSE_ERROR;
				}
				flag = RIP_IFPS_V1;
				break;

			    case 2:
				if ($3) {
				    flag = $3;
				} else {
				    /* This means whatever options the interface is capable of */
#ifdef	IP_MULTICAST
				    flag = RIP_IFPS_V2;
#else	/* IP_MULTICAST */
				    flag = RIP_IFPS_V2BC;
#endif	/* IP_MULTICAST */
				}
				break;

			    default:
				(void) sprintf(parse_error, "invalid version");
				PARSE_ERROR;
			    }
			    $$ = config_alloc(RIP_CONFIG_FLAG, (void_t) flag);
			}
		;

rip_v2_options	: /* Empty */
			{
			    $$ = 0;
			}
		| T_MULTICAST
			{
			    $$ = RIP_IFPS_V2MC;
			}
		| T_BROADCAST
			{
			    $$ = RIP_IFPS_V2BC;
			}
		;

rip_trace_options
		: trace_options
    		;


rip_prop_init	: T_RIP
			{
			    $$ = parse_proto = RTPROTO_RIP;
			    parse_gwlist = &rip_gw_list;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;


/*  */


hello_statement	: T_HELLO
			{
			    PROTO_SEEN(RTPROTO_HELLO);

			    parse_gwlist = &hello_gw_list;

			    hello_trace_flags = trace_flags;
			}
		onoff_option hello_group
			{
			    switch ($3) {
			    case T_OFF:
				BIT_RESET(hello_flags, HELLOF_ON);
				break;

			    case T_ON:
				BIT_SET(hello_flags, HELLOF_ON);
				break;
			    }
			}
		;

hello_group	: /* Empty */
		| '{' hello_group_stmts '}'
		;

hello_group_stmts
		: /* Empty */
		| hello_group_stmts hello_group_stmt L_EOS
		| hello_group_stmts error L_EOS
			{
			    yyerrok;
			}
		;

hello_group_stmt
		: T_PREFERENCE preference
			{
			    hello_preference = $2;
			}
		| T_DEFAULTMETRIC metric
			{
			    if (parse_metric_check(RTPROTO_HELLO, &$2)) {
				PARSE_ERROR;
			    }
			    hello_default_metric = $2.metric;
			}
		| T_BROADCAST
			{
			    hello_flags = (hello_flags & ~HELLOF_CHOOSE) | HELLOF_BROADCAST;
			}
		| T_NONBROADCAST
			{
			    hello_flags = hello_flags & ~(HELLOF_CHOOSE|HELLOF_BROADCAST);
			}
		| T_TRACEOPTIONS hello_trace_options
			{
			    hello_trace_flags =  $2;
			}
		| T_INTERFACE interface_all_list hello_interface_options
			{
			    config_list *list = config_list_alloc($3, 0);

			    parse_adv_propagate_config($2, list, RTPROTO_HELLO);

			    if (parse_adv_append(&hello_int_policy, $2, TRUE)) {
				PARSE_ERROR;
			    }

			    config_list_free(list);
			}
		| T_TRUSTEDGATEWAYS gateway_list
			{
			    hello_n_trusted += parse_gw_flag($2, RTPROTO_HELLO, GWF_TRUSTED);
			    if (!hello_n_trusted) {
				PARSE_ERROR;
			    }
			}
		| T_SOURCEGATEWAYS gateway_list
			{
			    hello_n_source += parse_gw_flag($2, RTPROTO_HELLO, GWF_SOURCE);
			    if (!hello_n_source) {
				PARSE_ERROR;
			    }
			    BIT_SET(hello_flags, HELLOF_SOURCE);
			}
		;

hello_interface_options
		: hello_interface_option
		| hello_interface_options hello_interface_option
			{
			    $$ = config_append($1, $2);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		;

hello_interface_option
		: T_NOHELLOIN
			{
			    $$ = config_alloc(HELLO_CONFIG_NOIN, (void_t) TRUE);
			}
		| T_NOHELLOOUT
			{
			    $$ = config_alloc(HELLO_CONFIG_NOOUT, (void_t) TRUE);
			}
		| T_METRICIN metric
			{
			    if (parse_metric_check(RTPROTO_HELLO, &$2)) {
				PARSE_ERROR;
			    }
			    $$ = config_alloc(HELLO_CONFIG_METRICIN, (void_t) $2.metric);
			}
		| T_METRICOUT metric
			{
			    if (parse_metric_check(RTPROTO_HELLO, &$2)) {
				PARSE_ERROR;
			    }
			    $$ = config_alloc(HELLO_CONFIG_METRICOUT, (void_t) $2.metric);
			}
		;

hello_trace_options
		: trace_options
    		;


hello_prop_init	: T_HELLO
			{
			    $$ = parse_proto = RTPROTO_HELLO;
			    parse_gwlist = &hello_gw_list;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;

    
/*  */


ospf_statement	: T_OSPF
			{
			    PROTO_SEEN(RTPROTO_OSPF);

			    parse_gwlist = &ospf.gw_list;

			    ospf.trace_flags = trace_flags;

			    /* Somewhere around here is where we will figure out which OSPF global structure we are */
			    /* refering to and initialize it once we support multiple instantations of OSPF that is. */
			}
		onoff_option ospf_group
			{
			    if ($3 == T_OFF) {
				ospf.ospf_admin_stat = OSPF_DISABLED;
			    } else {
				if (ospf_parse_valid_check(parse_error)) {
				    PARSE_ERROR;
				}
				ospf.ospf_admin_stat = OSPF_ENABLED;
			    }
			}
		;

ospf_group	: /* Empty */
		| '{' ospf_group_stmts '}'
		;

ospf_group_stmts	: /* Empty */
		| ospf_group_stmts ospf_group_stmt L_EOS
		| ospf_group_stmts error L_EOS
			{
				yyerrok;
			}
		;

ospf_group_stmt	: T_DEFAULTS '{' ospf_defaults '}'
			{
			}
		| T_TRACEOPTIONS ospf_trace_options
			{
			    ospf.trace_flags = $2;
			}
		| T_EXPORTINTERVAL time
			{
			    ospf.export_interval = $2;
			}
		| T_EXPORTLIMIT number
			{
			    ospf.export_limit = $2;
			}
		| T_MONITORAUTHKEY ospf_auth_key
			{
			    ospf.mon_authtype = OSPF_AUTH_SIMPLE;
			    bcopy((caddr_t) $2.ptr, (caddr_t) ospf.mon_authkey, OSPF_AUTH_SIZE);

			    free_bytestr(&$2);
			}
		| ospf_area_option
			{
			    ospf_AREA = ospf_parse_area_alloc($1, parse_error);
			    if (!ospf_AREA) {
				PARSE_ERROR;
			    }
			    ospf_AREA->authtype = OSPF_AUTH_NONE;		/* Default to no authentication */
			}
		'{' ospf_area_stmts '}'
			{
			    if (ospf_parse_area_check(ospf_AREA, parse_error)) {
				PARSE_ERROR;
			    }
			}
		;

ospf_defaults	: /* Empty */
		| ospf_defaults ospf_default L_EOS
    		| ospf_defaults error L_EOS
			{
			    yyerrok;
			}
		;

ospf_default	: T_PREFERENCE preference
			{
			    ospf.preference_ase = $2;
			}
		| T_METRIC ospf_cost
			{
			    ospf.export_metric = $2.metric;
			}
		| T_TAG tag_as
			{
			    switch ($2.state) {
			    case PARSE_METRICS_SET:
				ospf.export_tag = $2.metric;
				break;

			    case PARSE_METRICS_ALTERNATE:
				ospf.export_tag = PATH_OSPF_TAG_TRUSTED |
				    (($2.metric << PATH_OSPF_TAG_USR_SHIFT) & PATH_OSPF_TAG_USR_MASK);
				break;
			    }
			}
		| T_TYPE ospf_type
			{
			    ospf.export_type = $2.metric;
			}
		;

ospf_area_stmts	: /* Empty */
    		| ospf_area_stmts ospf_area_stmt L_EOS
    		| ospf_area_stmts error L_EOS
			{
			    yyerrok;
			}
		;

ospf_area_stmt	: T_AUTHTYPE ospf_auth_type
			{
			    ospf_AREA->authtype = $2;
			}
		| T_STUB ospf_cost_option
			{
			    if (ospf_AREA->area_id == OSPF_BACKBONE) {
				(void) sprintf(parse_error, "stub option not valid for backbone");
				PARSE_ERROR;
			    }

			    /* This is a stub */
			    BIT_SET(ospf_AREA->area_flags, OSPF_AREAF_STUB);

			    /* And we inject a default */
			    if (PARSE_METRIC_ISSET(&$2)) {
				BIT_SET(ospf_AREA->area_flags, OSPF_AREAF_STUB_DEFAULT);
				ospf_AREA->dflt_metric = $2.metric;
			    }
			}    
		| T_STUBHOSTS '{' ospf_stub_list '}' {}
		| T_NETWORKS '{' ospf_area_range_list '}'
			{
			    adv_entry *adv;

			    if (ospf_AREA->nrcnt) {
				(void) sprintf(parse_error, "net range already specified");
				PARSE_ERROR;
			    }

			    /* Install each of the entries */
			    ADV_LIST($3, adv) {
				ospf_parse_add_net(ospf_AREA,
						   adv->adv_dm.dm_dest,
						   adv->adv_dm.dm_mask);
			    } ADV_LIST_END($3, adv) ;

			    adv_free_list($3);
			}
		| T_INTERFACE ospf_interface
			{
			    if (parse_adv_append(&ospf_AREA->intf_policy, $2, TRUE)) {
				PARSE_ERROR;
			    }
			}
    		| T_VIRTUALLINK T_NEIGHBORID host T_TRANSITAREA ospf_area ospf_common
			{
			    struct INTF *vif;

			    vif = ospf_parse_virt_parse(ospf_AREA, $3, $5, $6, parse_error);
			    if (!vif) {
				PARSE_ERROR;
			    }

			    config_list_free($6);
			}
		;

ospf_interface
		: interface_all_list ospf_cost_option ospf_common
			{
			    config_list *list = $3;

			    if (PARSE_METRIC_ISSET(&$2)) {

				list = config_list_add(list,
						       config_alloc(OSPF_CONFIG_COST,
								    (void_t) $2.metric),
						       ospf_config_free);
				if (!list) {
				    PARSE_ERROR;
				}
			    }

			    parse_adv_propagate_config($1, list, RTPROTO_OSPF);

			    config_list_free(list);

			    $$ = $1;
			}
		| interface_all_list T_NONBROADCAST ospf_cost_option ospf_nonbroadcast
			{
			    config_list *list = $4;

			    list = config_list_add(list,
						   config_alloc(OSPF_CONFIG_TYPE,
								(void_t) NONBROADCAST),
						   ospf_config_free);
			    if (!list) {
				PARSE_ERROR;
			    }
			    
			    if (PARSE_METRIC_ISSET(&$3)) {

				list = config_list_add(list,
						       config_alloc(OSPF_CONFIG_COST,
								    (void_t) $3.metric),
						       ospf_config_free);
				if (!list) {
				    PARSE_ERROR;
				}
			    }

			    parse_adv_propagate_config($1, list, RTPROTO_OSPF);

			    config_list_free(list);

			    $$ = $1;
			}
		;

ospf_common	: /* Empty */
			{
			    $$ = (config_list *) 0;
			}
		| '{' ospf_common_options '}'
			{
			    $$ = config_list_alloc($2, ospf_config_free);
			}
		;

ospf_common_options
		: ospf_common_option L_EOS
		| ospf_common_options ospf_common_option L_EOS
			{
			    $$ = config_append($1, $2);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		| ospf_common_options error L_EOS
			{
				yyerrok;
			}
		;

ospf_nonbroadcast
		: /* Empty */
			{
			    $$ = (config_list *) 0;
			}
		| '{' ospf_nonbroadcast_options '}'
			{
			    $$ = config_list_alloc($2, ospf_config_free);
			}
		;

ospf_nonbroadcast_options
		: ospf_nonbroadcast_option L_EOS
		| ospf_nonbroadcast_options ospf_nonbroadcast_option L_EOS
			{
			    $$ = config_append($1, $2);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		| ospf_nonbroadcast_options error L_EOS
			{
				yyerrok;
			}
		;

ospf_common_option
		: T_ENABLE
			{
			    $$ = config_alloc(OSPF_CONFIG_ENABLE, (void_t) TRUE);
			}
		| T_DISABLE
			{
			    $$ = config_alloc(OSPF_CONFIG_ENABLE, (void_t) FALSE);
			}
		| T_RXMITINTERVAL time
			{
			    if (parse_limit_check("retransmit-interval",
						  (u_int) $2,
						  OSPF_LIMIT_RETRANSMITINTERVAL)) {
				PARSE_ERROR;
			    }
			    $$ = config_alloc(OSPF_CONFIG_RETRANSMIT, (void_t) $2);
			}
		| T_INFTRANSDELAY time
			{
			    if (parse_limit_check("transit-delay",
						  (u_int) $2,
						  OSPF_LIMIT_TRANSITDELAY)) {
				PARSE_ERROR;
			    }
			    $$ = config_alloc(OSPF_CONFIG_TRANSIT, (void_t) $2);
			}
		| T_PRIORITY ospf_priority
			{
			    $$ = config_alloc(OSPF_CONFIG_PRIORITY, (void_t) $2);
			}
		| T_HELLOINTERVAL time
			{
			    if (parse_limit_check("hello-interval",
						  (u_int) $2,
						  OSPF_LIMIT_HELLOINTERVAL)) {
				PARSE_ERROR;
			    }
			    $$ = config_alloc(OSPF_CONFIG_HELLO, (void_t) $2);
			}
		| T_ROUTERDEADINTERVAL time
			{
			    if (parse_limit_check("router-dead-interval",
						  (u_int) $2,
						  OSPF_LIMIT_ROUTERDEADINTERVAL)) {
				PARSE_ERROR;
			    }
			    $$ = config_alloc(OSPF_CONFIG_ROUTERDEAD, (void_t) $2);
			}
		| T_AUTHKEY ospf_auth_key
			{
			    /* Side-effect: The authentication type must be specified first */

			    switch (ospf_AREA->authtype) {
			    case OSPF_AUTH_NONE:
				sprintf(parse_error, "authentication-key specified when authentication-type is none");
				PARSE_ERROR;
				/* break; */

			    case OSPF_AUTH_SIMPLE:
				/* Simple password */
				break;

			    default:
				sprintf(parse_error, "unknown authentication type: %d",
					ospf_AREA->authtype);
				PARSE_ERROR;
			    }

			    $$ = config_alloc(OSPF_CONFIG_AUTHKEY,
					      (void_t) sockdup(sockbuild_byte($2.ptr, $2.len)));

			    free_bytestr(&$2);
			}
		;

ospf_nonbroadcast_option
		: ospf_common_option
		| T_POLLINTERVAL time
			{
			    if (parse_limit_check("poll-interval",
						  (u_int) $2,
						  OSPF_LIMIT_POLLINTERVAL)) {
				PARSE_ERROR;
			    }
			    $$ = config_alloc(OSPF_CONFIG_POLL, (void_t) $2);
			}
		| T_ROUTERS '{' ospf_router_eligible_list '}'
			{
			    $$ = config_alloc(OSPF_CONFIG_ROUTERS, (void_t) $3);
			}
		;

/* OSPF costs */
ospf_cost_option	: /* Empty */
			{
			    PARSE_METRIC_CLEAR(&$$);
			}
		| T_METRIC ospf_cost
			{
				$$ = $2;
			}
		;

ospf_cost	: metric
			{
			    $$ = $1;
			    if (parse_metric_check(RTPROTO_OSPF, &$$)) {
				PARSE_ERROR;
			    }
			}
		;

/* OSPF external route type */
ospf_type_option	: /* Empty */
			{
			    PARSE_METRIC_CLEAR(&$$);
			}
		| T_TYPE ospf_type
			{
			    $$ = $2;
			}
		;

ospf_type	: L_NUMBER
			{
			    if (parse_limit_check("export-type",
						  (u_int) $1,
						  OSPF_LIMIT_EXPORTTYPE)) {
				PARSE_ERROR;
			    }
			    PARSE_METRIC_SET(&$$, $1);
			}
		;

ospf_area_option
		: T_AREA ospf_area
			{
			    $$ = $2;
			}
		| T_BACKBONE
			{
			    $$ = OSPF_BACKBONE;
			}
		;

ospf_area	: number
			{
			    if (parse_limit_check("area-number",
						  (u_int) $1,
						  OSPF_LIMIT_AREA)) {
				PARSE_ERROR;
			    }
			    $$ = htonl($1);
			}
		;

ospf_auth_type	: L_NUMBER
			{
			    if (parse_limit_check("authentication-type",
						  (u_int) $1,
						  OSPF_LIMIT_AUTHTYPE)) {
				PARSE_ERROR;
			    }
			    $$ = $1;
			}
		| T_NONE
			{
			    $$ = OSPF_AUTH_NONE;
			}
		| T_SIMPLE
			{
			    $$ = OSPF_AUTH_SIMPLE;
			}
		;

ospf_auth_key	: string
			{
			    if ($1.len > OSPF_AUTH_SIZE) {
				sprintf(parse_error, "authentication-key \"%s\"%d longer than %d characters",
					$1.ptr,
					$1.len,
					OSPF_AUTH_SIZE);
				PARSE_ERROR;
			    }

			    $$.type = $1.type;
			    $$.len = OSPF_AUTH_SIZE;
			    $$.ptr = (byte *) task_mem_calloc((task *) 0, 1, OSPF_AUTH_SIZE);
			    
			    /* Left justify the authentication key */
			    (void) strncpy((caddr_t) $$.ptr, $1.ptr, $1.len);

			    free_charstr(&$1);
			}
		| L_NUMBER
			{
			    int offset = OSPF_AUTH_SIZE - sizeof ($1);

			    $$.type = L_NUMBER;
			    $$.len = OSPF_AUTH_SIZE;
			    $$.ptr = (byte *) task_mem_calloc((task *) 0, 1, OSPF_AUTH_SIZE);

			    /* Right justify the authentication key */
			    bcopy((caddr_t) &$1, (caddr_t) $$.ptr + offset, sizeof($1));
			}
		| byte_string
			{
			    int offset = OSPF_AUTH_SIZE - $1.len;

			    if (offset < 0) {
				sprintf(parse_error, "authentication-key longer than %d characters",
					OSPF_AUTH_SIZE);
				PARSE_ERROR;
			    }

			    $$.type = $1.type;
			    $$.len = OSPF_AUTH_SIZE;
			    $$.ptr = (byte *) task_mem_calloc((task *) 0, 1, OSPF_AUTH_SIZE);

			    /* Right justify the authentication key */
			    bcopy((caddr_t) $1.ptr, (caddr_t) $$.ptr + offset, $1.len);

			    free_bytestr(&$1);
			}
    		;



ospf_stub_list	: ospf_stub L_EOS
    		| ospf_stub_list ospf_stub L_EOS
    		| ospf_stub_list error L_EOS
			{
			    yyerrok;
			}
    		;

ospf_stub	: host T_METRIC ospf_cost
			{
			    ospf_parse_add_host(ospf_AREA, sock2ip($1), $3.metric);

			    sockfree($1);
			}
		;

ospf_router_eligible_list
		: ospf_router_eligible L_EOS
		| ospf_router_eligible_list ospf_router_eligible L_EOS
			{
			    ($$ = $2)->ocr_next = $1;
			}
		| ospf_router_eligible_list error L_EOS
			{
			    yyerrok;
			}
    		;

ospf_router_eligible
		: gateway ospf_eligible_option
			{
			    $$ = ospf_parse_router_alloc(sock2in($1->gw_addr), (u_int) $2);
			}
		;

ospf_eligible_option
		: /* Empty */
			{
			    $$ = 0;
			}
		| T_ELIGIBLE
			{
			    $$ = 1;
			}
		;

ospf_priority	: L_NUMBER
			{
			    if (parse_limit_check("priority",
						  (u_int) $1,
						  OSPF_LIMIT_DRPRIORITY)) {
				PARSE_ERROR;
			    }
			    $$ = $1;
			}
		;

ospf_prop_init	: T_OSPF
			{
			    $$ = parse_proto = RTPROTO_OSPF;
			    parse_gwlist = (gw_entry **) 0;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;

ospf_ase_prop_init
		: T_OSPF_ASE
			{
			    /* Make sure this PS type is known */
			    adv_psfunc_add(RTPROTO_OSPF_ASE, &ospf_adv_psfunc);

			    $$ = parse_proto = RTPROTO_OSPF_ASE;
			    parse_gwlist = (gw_entry **) 0;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;

ospf_trace_options
		: ospf_trace_option
		| ospf_trace_options ospf_trace_option
			{
			    $$ = $1 | $2;
			}
		| ospf_trace_options T_EXCEPT ospf_trace_option
			{
			    $$ = $1 & ~$3;
			}
		;

ospf_area_range_list
		: /* Empty */
			{
			    $$ = (adv_entry *) 0;
			}
		| ospf_area_range_list ospf_area_range L_EOS
			{
			    if (sock2ip($2.dm_dest) == INADDR_ANY) {
				sprintf(parse_error, "invalid range %A/%A",
					$2.dm_dest,
					$2.dm_mask);
				PARSE_ERROR;
			    }
			    $$ = adv_alloc(ADVFT_DM, (proto_t) 0);
			    $$->adv_dm = $2;
			    $$ = parse_adv_dm_append($1, $$);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		| ospf_area_range_list error L_EOS
			{
			    yyerrok;
			}
		
		;

ospf_area_range	: host_mask
		| network_mask
		;

ospf_trace_option
		: trace_option
		| T_DD				{ $$ = TR_OSPF_DD; }
		| T_REQUEST			{ $$ = TR_OSPF_REQ; }
		| T_LSU				{ $$ = TR_OSPF_LSU; }
		| T_ACK				{ $$ = TR_OSPF_ACK; }
		| T_RECEIVE			{ $$ = TR_OSPF_RX; }
		| T_LSA_BLD			{ $$ = TR_OSPF_LSA_BLD; }
		| T_LSA_TX			{ $$ = TR_OSPF_LSA_TX; }
		| T_LSA_RX			{ $$ = TR_OSPF_LSA_RX; }
		| T_SPF				{ $$ = TR_OSPF_SPF; }
		| T_TRAP			{ $$ = TR_OSPF_TRAP; }
		;


    
/*  */


/*  */


/*  */


/*  */


egp_statement	: T_EGP
			{
			    PROTO_SEEN(RTPROTO_EGP);

			    egp_trace_flags = trace_flags;
			}
		onoff_option egp_group
			{
			    switch ($3) {
			    case T_ON:
				doing_egp = TRUE;

				if (!inet_autonomous_system) {
				    (void) sprintf(parse_error, "autonomous-system not specified");
				    PARSE_ERROR;
				}
				if (!egp_neighbors) {
				    (void) sprintf(parse_error, "no EGP neighbors specified");
				    PARSE_ERROR;
				}

#if	defined(PROTO_SNMP)
				egp_sort_neighbors();
#endif	/* defined(PROTO_SNMP) */
				break;

			    case T_OFF:
				doing_egp = FALSE;
			    }
			}
		;

egp_group	: /* Empty */
		| '{' egp_group_stmts '}'
		;

egp_group_stmts	: /* Empty */
		| egp_group_stmts egp_group_stmt L_EOS
		| egp_group_stmts error L_EOS
			{
			    yyerrok;
			}
		;

egp_group_stmt	: T_PREFERENCE preference
			{
			    egp_preference = $2;
			}
		| T_DEFAULTMETRIC metric
			{
			    if (parse_metric_check(RTPROTO_EGP, &$2)) {
				PARSE_ERROR;
			    }
			    egp_default_metric = $2.metric;
			}
		| T_PKTSIZE number
			{
			    if (parse_limit_check("packetsize",
						  (u_int) $2,
						  EGP_LIMIT_PKTSIZE)) {
				PARSE_ERROR;
			    }
			    egp_pktsize = $2;
			}
		| T_TRACEOPTIONS egp_trace_options
			{
			    egp_trace_flags =  $2;
			}
		| T_GROUP
			{
			    /* Clear group structure and set fill pointer */
			    bzero((caddr_t) &egp_group, sizeof(egp_group));
			    ngp = &egp_group;
			    ngp->ng_trace_flags = egp_trace_flags;
			    /* First neighbor in group is head of group */
			    gr_ngp = (egp_neighbor *) 0;
			    parse_group_index++;
			}
		egp_group_options '{' egp_peer_stmts '}'
			{
			    if (gr_ngp->ng_gr_acquire > gr_ngp->ng_gr_number) {
				(void) sprintf(parse_error,
					       "maxacquire %u is greater than number of neighbors %u in group %d",
					       gr_ngp->ng_gr_acquire,
					       gr_ngp->ng_gr_number,
					       parse_group_index);
				PARSE_ERROR;
			    } else if (!gr_ngp->ng_gr_acquire) {
				gr_ngp->ng_gr_acquire = gr_ngp->ng_gr_number;
			    }

			}
		;

egp_peer_stmts	: /* Empty */
		| egp_peer_stmts egp_peer_stmt L_EOS
		| egp_peer_stmts error L_EOS
			{
			    yyerrok;
			}
		;

egp_peer_stmt	: T_NEIGHBOR
			{
			    ngp = egp_ngp_alloc(&egp_group);

			    /* This neighbor is head of the group */
			    if (!gr_ngp) {
				gr_ngp = ngp;
			    }
			    parse_gwlist = &parse_gwp;
			}
		host egp_peer_options
                        {
			    int add = TRUE;
			    egp_neighbor *ngp2;

			    /* Set neighbor's address */
			    ngp->ng_addr = $3;
			    ngp->ng_gw.gw_proto = RTPROTO_EGP;
			    (void) sprintf(ngp->ng_name, "%A",
					   ngp->ng_addr);

			    /* Set group pointer and count this neighbor */
			    ngp->ng_gr_head = gr_ngp;
			    ngp->ng_gr_index = parse_group_index;
			    gr_ngp->ng_gr_number++;

			    EGP_LIST(ngp2) {
				if (sockaddrcmp_in(ngp->ng_addr, ngp2->ng_addr)) {
				    if (BIT_TEST(ngp2->ng_flags, NGF_DELETE)) {
					if (!egp_neighbor_changed(ngp2, ngp)) {
					    BIT_RESET(ngp2->ng_flags, NGF_DELETE);
					    egp_ngp_free(ngp);
					    add = FALSE;
					    break;
					} else {
					    ngp->ng_flags = NGF_WAIT;
					}
				    } else {
					(void) sprintf(parse_error, "duplicate EGP neighbor at %A",
						       ngp->ng_addr);
					PARSE_ERROR;
				    }
				}
			    } EGP_LIST_END(ngp2) ;
			    
			    /* Add this neighbor to end of the list */
			    if (add) {
				insque((struct qelem *) ngp, (struct qelem *) egp_neighbor_head.ng_back);
				egp_neighbors++;
			    }
			}
		;

egp_group_options
		: /* Empty */
		| egp_group_options egp_group_option
		| egp_group_options egp_peer_option
		;

egp_group_option
		: T_PEERAS as
			{
			    BIT_SET(ngp->ng_options, NGO_PEERAS);
			    ngp->ng_peer_as = $2;
			}
		| T_LOCALAS as
			{
			    BIT_SET(ngp->ng_options, NGO_LOCALAS);
			    ngp->ng_local_as = $2;
			}
		| T_MAXUP L_NUMBER
			{
			    /* XXX - Limit check maxup value */
			    BIT_SET(ngp->ng_options, NGO_MAXACQUIRE);
			    ngp->ng_gr_acquire = $2;
			}
		| T_VERSION L_NUMBER
			{
			    if ( !(EGPVMASK & (1 << ($2 - 2))) ) {
				(void) sprintf(parse_error, "unsupported EGP version: %d",
					       $2);
				PARSE_ERROR;
			    }
			    BIT_SET(ngp->ng_options, NGO_VERSION);
			    ngp->ng_version = $2;
			}
		| T_PREFERENCE preference
			{
			    BIT_SET(ngp->ng_options, NGO_PREFERENCE);
			    ngp->ng_preference = $2;
			}
		| T_TRACEOPTIONS egp_trace_options
			{
			    ngp->ng_trace_flags = egp_trace_flags;
			}
		;

egp_peer_options
		: /* Empty */
		| egp_peer_options egp_peer_option
		;

egp_peer_option	: T_METRICOUT metric
			{
			    if (parse_metric_check(RTPROTO_EGP, &$2)) {
				PARSE_ERROR;
			    }
			    ngp->ng_metricout = $2.metric;
			    BIT_SET(ngp->ng_options, NGO_METRICOUT);
			}
		| T_NOGENDEFAULT
			{
			    BIT_SET(ngp->ng_options, NGO_NOGENDEFAULT);
			}
		| T_DEFAULTIN
			{
			    BIT_SET(ngp->ng_options, NGO_DEFAULTIN);
			}
		| T_DEFAULTOUT
			{
			    BIT_SET(ngp->ng_options, NGO_DEFAULTOUT);
			}
		| T_GATEWAY gateway
			{
			    BIT_SET(ngp->ng_options, NGO_GATEWAY);
			    ngp->ng_gateway = sockdup($2->gw_addr);
			    gw_freelist(*parse_gwlist);
			    parse_gwlist = (gw_entry **) 0;
			    parse_gwp = (gw_entry *) 0;
			}
		| T_LCLADDR interface_local
			{
			    BIT_SET(ngp->ng_options, NGO_LCLADDR);
			    ngp->ng_lcladdr = $2;
			}
		| T_SOURCENET network
			{
			    BIT_SET(ngp->ng_options, NGO_SADDR);
			    ngp->ng_saddr = $2;
			}
 		| T_P1 time
 			{
 			    if (parse_limit_check("P1",
						  (u_int) $2,
						  EGP_LIMIT_P1)) {
 				PARSE_ERROR;
 			    }
 			    BIT_SET(ngp->ng_options, NGO_P1);
 			    ngp->ng_P1 = $2;
 			}
 		| T_P2 time
 			{
 			    if (parse_limit_check("P2",
						  (u_int) $2,
						  EGP_LIMIT_P2)) {
 				PARSE_ERROR;
 			    }
 			    BIT_SET(ngp->ng_options, NGO_P2);
 			    ngp->ng_P2 = $2;
			}
		;

egp_trace_options
		:	trace_options
		;

egp_prop_init
		: T_EGP
			{
			    $$ = parse_proto = RTPROTO_EGP;
			    parse_gwlist = (gw_entry **) 0;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;


    
/*  */


bgp_statement	: T_BGP
			{
			    PROTO_SEEN(RTPROTO_BGP);

			    parse_gwlist = &parse_gwp;

			    bgp_default_trace_flags = trace_flags;
			}
		onoff_option bgp_group
			{
			    switch ($3) {
			    case T_ON:
				doing_bgp = TRUE;

				if (!bgp_conf_check(parse_error)) {
				    PARSE_ERROR;
				}
				break;

			    case T_OFF:
				doing_bgp = FALSE;
			    }

			    parse_gwlist = (gw_entry **) 0;
			}
		;

bgp_group	: /* Empty */
		| '{' bgp_group_stmts '}'
		;

bgp_group_stmts	: /* Empty */
		| bgp_group_stmts bgp_group_stmt L_EOS
		| bgp_group_stmts error L_EOS
			{
			    yyerrok;
			}
		;

bgp_group_stmt	: T_PREFERENCE preference
			{
			    bgp_default_preference = $2;
			}
		| T_DEFAULTMETRIC metric
			{
			    if (parse_metric_check(RTPROTO_BGP, &$2)) {
				PARSE_ERROR;
			    }
			    bgp_default_metric = $2.metric;
			}
		| T_TRACEOPTIONS bgp_trace_options
			{
			    bgp_default_trace_flags = $2;
			}
		| T_GROUP T_TYPE bgp_linktype T_PEERAS as
			{
			    /* Allocate a Group structure */
			    bgp = bgp_conf_group_alloc();

			    /* Set the type and AS */
			    bgp->bgpg_type = $3;
			    bgp->bgpg_peer_as = $5;

			    /* Point at this configuration information */
			    bcp = &bgp->bgpg_conf;

			    /* Inherit global trace flags */
			    bgp->bgpg_trace_flags = bgp_default_trace_flags;
			}
		bgp_peer_options
			{
			    /* Add this group */
			    bgp = bgp_conf_group_add(bgp, parse_error);
			    if (!bgp) {
				PARSE_ERROR;
			    }
			}
		'{' bgp_peer_stmts '}'
			{
			    /* Check group */
			    if (!bgp_conf_group_check(bgp, parse_error)) {
				PARSE_ERROR;
			    }
			}
		;

bgp_peer_options
		: /* Empty */
		| bgp_peer_options bgp_peer_option
		;

bgp_peer_stmts	: /* Empty */
		| bgp_peer_stmts bgp_peer_stmt L_EOS
		| bgp_peer_stmts error L_EOS
			{
				yyerrok;
			}
		;

bgp_peer_stmt	: T_NEIGHBOR
    			{
			    /* Allocate this peer */
			    bnp = bgp_conf_peer_alloc(bgp);

			    /* Point at this configuration information */
			    bcp = &bnp->bgp_conf;
			}
		host bgp_peer_options
			{
			    /* Set peer address */
			    bnp->bgp_addr = $3;

			    bnp = bgp_conf_peer_add(bgp, bnp, parse_error);
			    if (!bnp) {
				PARSE_ERROR;
			    }
			}
		| T_ALLOW
			{
			    parse_contig_masks = FALSE;
			}
			'{' bgp_allow_list '}'
			{
			    parse_contig_masks = TRUE;

			    if (parse_adv_append(&bgp->bgpg_allow, $4, TRUE)) {
				PARSE_ERROR;
			    }
			}
		;

bgp_peer_option	: T_METRICOUT metric
			{
			    if (parse_metric_check(RTPROTO_BGP, &$2)) {
				PARSE_ERROR;
			    }
			    BIT_SET(bcp->bgpc_options, BGPO_METRIC_OUT);
			    bcp->bgpc_metric_out = $2.metric;
			}
		| T_LOCALAS as
			{
			    BIT_SET(bcp->bgpc_options, BGPO_LOCAL_AS);
			    bcp->bgpc_local_as = $2;
			}
		| T_NOGENDEFAULT
			{
			    BIT_SET(bcp->bgpc_options, BGPO_NOGENDEFAULT);
			}
		| T_GATEWAY gateway
			{
			    if (bcp->bgpc_gateway) {
				/* Free the old one */
				sockfree(bcp->bgpc_gateway);
			    }
			    BIT_SET(bcp->bgpc_options, BGPO_GATEWAY);
			    bcp->bgpc_gateway = sockdup($2->gw_addr);
			    gw_freelist(*parse_gwlist);
			    *parse_gwlist = parse_gwp = (gw_entry *) 0;
			}
		| T_PREFERENCE preference
			{
			    BIT_SET(bcp->bgpc_options, BGPO_PREFERENCE);
			    bcp->bgpc_preference = $2;
			}
		| T_LCLADDR interface_local
			{
			    if (bcp->bgpc_lcladdr) {
				/* Free the old one */
				ifae_free(bcp->bgpc_lcladdr);
			    }
			    BIT_SET(bcp->bgpc_options, BGPO_LCLADDR);
			    bcp->bgpc_lcladdr = $2;
			}
		| T_HOLDTIME time
			{
			    if (parse_limit_check("hold time",
						  (u_int) $2,
						  BGP_LIMIT_HOLDTIME)) {
				PARSE_ERROR;
			    }
			    BIT_SET(bcp->bgpc_options, BGPO_HOLDTIME);
			    bcp->bgpc_holdtime_out = $2;
			}
		| T_TRACEOPTIONS bgp_trace_options
			{
			    bcp->bgpc_trace_flags = $2;
			}
		| T_VERSION L_NUMBER
			{
			    if (!BGP_KNOWN_VERSION($2)) {
				(void) sprintf(parse_error, "unsupported BGP version: %d",
					       $2);
				PARSE_ERROR;
			    }
			    BIT_SET(bcp->bgpc_options, BGPO_VERSION);
			    bcp->bgpc_conf_version = $2;
			}
		| T_PASSIVE
			{
			    BIT_SET(bcp->bgpc_options, BGPO_PASSIVE);
			}
		| T_DEFAULTIN
			{
			    BIT_SET(bcp->bgpc_options, BGPO_DEFAULTIN);
			}
		| T_DEFAULTOUT
			{
			    BIT_SET(bcp->bgpc_options, BGPO_DEFAULTOUT);
			}
		| T_KEEPALL
			{
			    BIT_SET(bcp->bgpc_options, BGPO_KEEPALL);
			}
		| T_SENDBUF bgp_kbufsize
			{
			    bcp->bgpc_send_bufsize = $2;
			}
		| T_RECVBUF bgp_kbufsize
			{
			    bcp->bgpc_recv_bufsize = $2;
			}
		| T_SPOOLBUF number
			{
			    if (parse_limit_check("spool buffer size",
						  (u_int) $2,
						  BGP_LIMIT_SBUF)) {
				PARSE_ERROR;
			    }
			    bcp->bgpc_spool_bufsize = $2;
			}
		;

bgp_kbufsize	: number
			{
			    if (parse_limit_check("kernel buffer size",
						  (u_int) $1,
						  BGP_LIMIT_KBUF)) {
				PARSE_ERROR;
			    }
			    $$ = $1;
			}
		;

bgp_linktype	: T_INTERNAL
			{
			    $$ = BGPG_INTERNAL;
			}
		| T_EXTERNAL
			{
			    $$ = BGPG_EXTERNAL;
			}
		| T_IGP
			{
			    $$ = BGPG_INTERNAL_IGP;
			}
		| T_TEST
			{
			    $$ = BGPG_TEST;
			}
		;

bgp_trace_options
		: trace_options
		;

bgp_prop_init
		: T_BGP
			{
			    $$ = parse_proto = RTPROTO_BGP;
			    parse_gwlist = (gw_entry **) 0;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;


bgp_allow_list	: /* Empty */
			{
			    $$ = (adv_entry *) 0;
			}
		| bgp_allow_list bgp_allow L_EOS
			{
			    $$ = adv_alloc(ADVFT_DM, (proto_t) 0);
			    $$->adv_dm = $2;
			    $$ = parse_adv_dm_append($1, $$);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		| bgp_allow_list error L_EOS
			{
			    yyerrok;
			}
		;
		
	
/* Destination and mask pair used for control lists */
bgp_allow	: all_mask
		| host_mask
		| network_mask
		;


/*  */

redirect_statement	: T_REDIRECT
			{
#if	defined(PROTO_ICMP) || defined(KRT_RT_SOCK)
			    PROTO_SEEN(RTPROTO_REDIRECT);

			    parse_gwlist = &redirect_gw_list;

			    redirect_trace_flags = trace_flags;
#endif	/* defined(PROTO_ICMP) || defined(KRT_RT_SOCK) */
			}
		onoff_option redirect_group
			{
#if	defined(PROTO_ICMP) || defined(KRT_RT_SOCK)
			    switch ($3) {
			    case T_ON:
				/* Enabled by default */
				break;

			    case T_OFF:
				redirect_disable(RTPROTO_REDIRECT);
				break;
			    }
#endif	/* defined(PROTO_ICMP) || defined(KRT_RT_SOCK) */
			}
		;

redirect_group	: /* Empty */
		| '{' redirect_group_stmts '}'
		;

redirect_group_stmts
		: /* Empty */
		| redirect_group_stmts redirect_group_stmt L_EOS
		| redirect_group_stmts error L_EOS
			{
			    yyerrok;
			}
		;

redirect_group_stmt	: T_PREFERENCE preference
			{
#if	defined(PROTO_ICMP) || defined(KRT_RT_SOCK)
			    redirect_preference = $2;
#endif	/* defined(PROTO_ICMP) || defined(KRT_RT_SOCK) */
			}
		| T_INTERFACE interface_all_list redirect_interface_options
			{
#if	defined(PROTO_ICMP) || defined(KRT_RT_SOCK)
			    config_list *list = config_list_alloc($3, 0);

			    parse_adv_propagate_config($2, list, RTPROTO_REDIRECT);

			    if (parse_adv_append(&redirect_int_policy, $2, TRUE)) {
				PARSE_ERROR;
			    }

			    config_list_free(list);
#endif	/* defined(PROTO_ICMP) || defined(KRT_RT_SOCK) */
			}
		| T_TRUSTEDGATEWAYS gateway_list
			{
#if	defined(PROTO_ICMP) || defined(KRT_RT_SOCK)
			    redirect_n_trusted += parse_gw_flag($2, RTPROTO_REDIRECT, GWF_TRUSTED);
			    if (!redirect_n_trusted) {
				PARSE_ERROR;
			    }
#endif	/* defined(PROTO_ICMP) || defined(KRT_RT_SOCK) */
			}
		| T_TRACEOPTIONS redirect_trace_options
			{
			    redirect_trace_flags = $2;
			}
		;

redirect_interface_options
		: redirect_interface_option
		| redirect_interface_options redirect_interface_option
			{
			    $$ = config_append($1, $2);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		;

redirect_interface_option
		: T_NOREDIRECTS
			{
#if	defined(PROTO_ICMP) || defined(KRT_RT_SOCK)
			    $$ = config_alloc(REDIRECT_CONFIG_NOIN, (void_t) TRUE);
#endif	/* defined(PROTO_ICMP) || defined(KRT_RT_SOCK) */
			}
		;

redirect_trace_options
		: trace_options
		;

/*  */



/*  */    


snmp_statement	: T_SNMP
		        {
			    PROTO_SEEN(RTPROTO_SNMP);

			    snmp_trace_flags = trace_flags;
			}
		onoff_option snmp_group
			{
			    /* XXX - trace options */

			    switch ($3) {
			    case T_ON:
				doing_snmp = TRUE;
				break;

			    case T_OFF:
				doing_snmp = FALSE;
				break;
			    }
			}
		;

snmp_group	: /* Empty */
		| '{' snmp_group_stmts '}'
		;

snmp_group_stmts
		: /* Empty */
		| snmp_group_stmts snmp_group_stmt L_EOS
		| snmp_group_stmts error L_EOS
			{
				yyerrok;
			}
		;

snmp_group_stmt
    		: T_PREFERENCE preference
			{
			    snmp_preference = $2;
			}
    		| T_TRACEOPTIONS snmp_trace_options
			{
			    snmp_trace_flags = $2;
			}
		| T_PORT port
			{
			    snmp_port = $2;
			}
		| T_DEBUG
			{
			    snmp_debug = TRUE;
			}
		;

snmp_trace_options
		: trace_options
		;


/*  */

route_order	: /* Empty */
			{
			    if (parse_new_state(PS_ROUTE)) {
				PARSE_ERROR;
			    }
			}
		;

route_statement : T_STATIC
			{
			    /* Need to set this for static routes, not used for interface routes */
			    parse_proto = RTPROTO_STATIC;
			    parse_gwlist = &rt_gw_list;
			    rt_open(rt_task);
			}
		'{' route_stmts '}'
			{
			    rt_close(rt_task, (gw_entry *) 0, 0, NULL);
			}
		;

route_stmts	: route_stmt L_EOS
		| route_stmts route_stmt L_EOS
		| error L_EOS
			{
				yyerrok;
			}
		;

route_stmt	: route_dest T_GATEWAY gateway_list interface_list_option preference_option rts_options
			{
			    /* Route to a gateway */

			    if (rt_parse_route($1.dm_dest,
					       $1.dm_mask,
					       $3,
					       $4,
					       PARSE_METRIC_ISSET(&$5) ? (pref_t) $5.metric : RTPREF_STATIC,
					       $6 | RTS_GATEWAY,
					       parse_error)) {
				PARSE_ERROR;
			    }

			    sockfree($1.dm_dest);
			}
		| route_dest T_INTERFACE interface_all preference_option rts_options
			{
			    /* Route to an interface */

			    if (rt_parse_route($1.dm_dest,
					       $1.dm_mask,
					       (adv_entry *) 0,
					       $3,
					       PARSE_METRIC_ISSET(&$4) ? (pref_t) $4.metric : RTPREF_STATIC,
					       $5,
					       parse_error)) {
				PARSE_ERROR;
			    }

			    sockfree($1.dm_dest);
			}
		;

route_dest	: host_mask
    		| network_mask
    		| T_DEFAULT
			{
			    $$.dm_dest = sockdup(inet_addr_default);
			    $$.dm_mask = inet_mask_default;
			    trace(TR_PARSE, 0, "parse: %s DEST: %A MASK: %A",
				  parse_where(),
				  $$.dm_dest,
				  $$.dm_mask);
			}
    		;

rts_options	: /* Empty */
			{
			    $$ = 0;
			}
		| rts_options rts_option
			{
			    $$ = $1 | $2;
			}
		;

rts_option	: T_RETAIN
			{
			    $$ = RTS_RETAIN;
			}
		| T_REJECT
			{
			    $$ = RTS_REJECT;
			}
		| T_BLACKHOLE
			{
			    $$ = RTS_BLACKHOLE;
			}
		| T_NOINSTALL
			{
			    $$ = RTS_NOTINSTALL;
			}
		;

/*  */

control_order	: /* Empty */
			{
			    if (parse_new_state(PS_CONTROL)) {
				PARSE_ERROR;
			    }
			}
		;
		
control_statement
		: T_IMPORT T_PROTO control_exterior T_AS as import_option L_EOS
			{
#if	defined(PROTO_EGP) || defined(PROTO_BGP)
			    adv_entry *adv;
			    pmet_t *pp = &$6.metric;

			    /*
			     *	Tack the list of destinations onto the end of the list
			     *	for neighbors with the specified AS.
			     */
			    adv = adv_alloc(ADVFT_AS, $3.proto);
			    adv->adv_as = $5;
			    adv->adv_list = $6.adv;
			    if (PARSE_METRIC_ISSET(pp)) {
				if (PARSE_METRIC_ISRESTRICT(pp)) {
				    BIT_SET(adv->adv_flag, ADVF_NO);
				} else {
				    BIT_SET(adv->adv_flag, ADVFOT_PREFERENCE);
				    adv->adv_result.res_preference = pp->metric;
				}
			    }

			    if (!parse_adv_as($3.import_list, adv)) {
				PARSE_ERROR;
			    }
#endif	/* defined(PROTO_EGP) || defined(PROTO_BGP) */
			}
		| T_IMPORT T_PROTO T_BGP aspath_match import_option L_EOS
			{
			    adv_entry *adv;
			    pmet_t *pp = &$5.metric;

			    /*
			     *	Tack the list of destinations onto the end of the list
			     *	for neighbors with the specified AS.
			     */
			    adv = adv_alloc(ADVFT_ASPATH, RTPROTO_BGP);
			    adv->adv_aspath = (caddr_t) $4;
			    adv->adv_list = $5.adv;
			    if (PARSE_METRIC_ISSET(pp)) {
				if (PARSE_METRIC_ISRESTRICT(pp)) {
				    BIT_SET(adv->adv_flag, ADVF_NO);
				} else {
				    BIT_SET(adv->adv_flag, ADVFOT_PREFERENCE);
				    adv->adv_result.res_preference = pp->metric;
				}
			    }

			    if (parse_adv_append(&bgp_import_aspath_list, adv, TRUE)) {
				PARSE_ERROR;
			    }
			}
		| T_IMPORT T_PROTO import_interior interface_gateway_list import_option L_EOS
			{
			    /*
			     *	Append the dest_mask list to the end of the import list
			     *	for the specified protocol.
			     */
			    if (!parse_adv_propagate_preference($4, $3.proto, &$5.metric, $5.adv)) {
				PARSE_ERROR;
			    }

			    adv_free_list($5.adv);

			    switch ($4->adv_flag & ADVF_TYPE) {
			    case ADVFT_GW:
				/* Gateway list */
			        {
				    adv_entry *adv = $4;
				    adv_entry *advn;

				    do {
					advn = adv->adv_next;
					adv->adv_next = NULL;
					/* Reset the type */
					adv->adv_flag = (adv->adv_flag & ~ADVF_TYPE) | ADVFT_ANY;
					if (parse_adv_append(&adv->adv_gwp->gw_import, adv, TRUE)) {
					    PARSE_ERROR;
					}
				    } while (adv = advn);
				}
				break;

			    case ADVFT_ANY:
				/* Global list */
				if (parse_adv_append($3.import_list, $4, TRUE)) {
				    PARSE_ERROR;
				}
				break;

			    case ADVFT_IFN:
			    case ADVFT_IFAE:
				/* Interface list */
				if (parse_adv_append(&int_import[$3.proto], $4, TRUE)) {
				    PARSE_ERROR;
				}
				break;

			    default:
				assert(FALSE);
			    }
			}
		| T_IMPORT T_PROTO T_OSPF_ASE tag_option import_option L_EOS
			{
			    adv_entry *adv;
			    pmet_t *pp = &$5.metric;

			    /* Make sure this PS type is known */
			    adv_psfunc_add(RTPROTO_OSPF_ASE, &ospf_adv_psfunc);

			    /*
			     *	Append the dest_mask list to the end of the OSPF import list.
			     */
			    adv = adv_alloc(ADVFT_PS, RTPROTO_OSPF_ASE);
			    adv->adv_list = $5.adv;
			    adv->adv_ps = (caddr_t) adv;

			    /* Set Tag */
			    if (PARSE_METRIC_ISSET(&$4)) {
				BIT_SET(adv->adv_flag, ADVFOT_METRIC);
				BIT_SET(adv->adv_result.res_flag, OSPF_EXPORT_TAG);
				adv->adv_result.res_metric = $4.metric;
			    }

			    /* Set Preference */
			    if (PARSE_METRIC_ISSET(pp)) {
				if (PARSE_METRIC_ISRESTRICT(pp)) {
				    BIT_SET(adv->adv_flag, ADVF_NO);
				} else {
				    BIT_SET(adv->adv_flag, ADVFOT_PREFERENCE);
				    adv->adv_result.res_preference = pp->metric;
				}
			    }

			    if (parse_adv_append(&ospf.import_list, adv, TRUE)) {
				PARSE_ERROR;
			    }
			}
		| T_EXPORT T_PROTO control_exterior T_AS as prop_source_option L_EOS
			{
#if	defined(PROTO_EGP) || defined(PROTO_BGP)
			    adv_entry *adv;
			    adv_entry *adv_prop = $6.adv;
			    pmet_t *mp = &$6.metric;

			    /*
			     *	Tack the list of destinations onto the end of the list
			     *	for neighbors with the specified AS.
			     */
			    adv = adv_alloc(ADVFT_AS, $3.proto);
			    adv->adv_as = $5;
			    adv->adv_list = adv_prop;
			    if (PARSE_METRIC_ISRESTRICT(mp)) {
				BIT_SET(adv->adv_flag, ADVF_NO);
			    } else if (PARSE_METRIC_ISSET(mp)) {
				BIT_SET(adv->adv_flag, ADVFOT_METRIC);
				adv->adv_result.res_metric = mp->metric;
			    }

			    if (!parse_adv_as($3.export_list, adv)) {
				PARSE_ERROR;
			    }
#endif	/* defined(PROTO_EGP) || defined(PROTO_BGP) */
			}
		| T_EXPORT T_PROTO export_interior interface_gateway_list prop_source_option L_EOS
			{
			    /*
			     *	Append the dest_mask list to the end of the export list
			     *	for the specified protocol.
			     */

			    if (!parse_adv_propagate_metric($4, $3.proto, &$5.metric, $5.adv)) {
				PARSE_ERROR;
			    }

			    adv_free_list($5.adv);

			    switch ($4->adv_flag & ADVF_TYPE) {
			    case ADVFT_GW:
				/* Gateway list */
			        {
				    adv_entry *adv = $4;
				    adv_entry *advn;
				
				    do {
					advn = adv->adv_next;
					adv->adv_next = NULL;
					if (parse_adv_append(&adv->adv_gwp->gw_export, adv, TRUE)) {
					    PARSE_ERROR;
					}
				    } while (adv = advn);
				}
				break;

			    case ADVFT_ANY:
				/* Global list for the protocol */
				if (parse_adv_append($3.export_list, $4, TRUE)) {
				    PARSE_ERROR;
				}
				break;

			    case ADVFT_IFN:
			    case ADVFT_IFAE:
				/* Interface list */
				if (parse_adv_append(&int_export[$3.proto], $4, TRUE)) {
				    PARSE_ERROR;
				}
				break;

			    default:
				assert(FALSE);
			    }
			}
    		| T_EXPORT T_PROTO export_ospf_ase ospf_type_option tag_as_option prop_source_option L_EOS
			{
			    adv_entry *adv;
			    adv_entry *adv_prop = $6.adv;
			    pmet_t *mp = &$6.metric;

			    /* If we export then we are a border router */
			    IAmASBorderRtr = TRUE;

			    /* Make sure this PS type is known */
			    adv_psfunc_add(RTPROTO_OSPF_ASE, &ospf_adv_psfunc);

			    /*
			     *	Append the dest_mask list to the end of the export list
			     *	for the specified protocol.
			     */
			    adv = adv_alloc(ADVFT_PS, RTPROTO_OSPF_ASE);
			    adv->adv_ps = (caddr_t) adv;
			    adv->adv_list = adv_prop;

			    /* Metric */
			    if (PARSE_METRIC_ISRESTRICT(mp)) {
				BIT_SET(adv->adv_flag, ADVF_NO);
			    } else if (PARSE_METRIC_ISSET(mp)) {
				BIT_SET(adv->adv_flag, ADVFOT_METRIC);
				adv->adv_result.res_metric = mp->metric;
			    }

			    /* Type */
			    if (PARSE_METRIC_ISSET(&$4)) {
				BIT_SET(adv->adv_flag, ADVFOT_FLAG);
				adv->adv_result.res_flag = $4.metric;
			    }

			    /* Tag */
			    switch ($5.state) {
			    case PARSE_METRICS_SET:
				BIT_SET(adv->adv_flag, ADVFOT_METRIC2|ADVFOT_FLAG);
				BIT_SET(adv->adv_result.res_flag, OSPF_EXPORT_TAG|OSPF_EXPORT_TAG_METRIC2);
				adv->adv_result.res_metric2 = $5.metric;
				break;

			    case PARSE_METRICS_ALTERNATE:
				BIT_SET(adv->adv_flag, ADVFOT_METRIC2|ADVFOT_FLAG);
				BIT_SET(adv->adv_result.res_flag, OSPF_EXPORT_TAG|OSPF_EXPORT_TAG_METRIC2);
				adv->adv_result.res_metric2 = PATH_OSPF_TAG_TRUSTED |
				    (($5.metric << PATH_OSPF_TAG_USR_SHIFT) & PATH_OSPF_TAG_USR_MASK);
				break;
			    }

			    if (parse_adv_append(&ospf.export_list, adv, TRUE)) {
				PARSE_ERROR;
			    }
			}
		;	

/*  */


/*  */

/* Support for import clauses */

control_exterior	: proto_exterior
			{
			    parse_export_proto = $$.proto = $1;

			    switch ($1) {
#if	defined(PROTO_EGP)
			    case RTPROTO_EGP:
				$$.import_list = &egp_import_list;
				$$.export_list = &egp_export_list;
				break;
#endif

#if	defined(PROTO_BGP)
			    case RTPROTO_BGP:
				$$.import_list = &bgp_import_list;
				$$.export_list = &bgp_export_list;
				break;
#endif

			    default:
				(void) sprintf(parse_error,
					       "unknown import protocol: %s",
					       gd_lower(trace_state(rt_proto_bits, $1)));
				PARSE_ERROR;
			    }
			}
		;


import_interior : proto_interior
			{
			    $$ = $1;
			    
			    parse_proto = $1.proto;
			    parse_gwlist = $1.gw_list;

			    if (!$1.import_list) {
				(void) sprintf(parse_error,
					       "unknown import protocol: %s",
					       gd_lower(trace_state(rt_proto_bits, $1.proto)));
				PARSE_ERROR;
			    }
			    
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $1.proto)));
			}
		;


import_option	: T_RESTRICT
			{
			    PARSE_METRIC_RESTRICT(&$$.metric);
			    $$.adv = (adv_entry *) 0;
			}
		| preference_option '{' import_list '}'
			{
			    $$.metric = $1;
			    $$.adv = $3;
			}
		;

import_list	: /* Empty */
			{
			    $$ = (adv_entry *) 0;
			}
		| import_list import_listen L_EOS
			{
			    
			    $$ = parse_adv_dm_append($1, $2);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		| import_list error L_EOS
			{
			    yyerrok;
			}
		;

import_listen	:
			{
			    parse_contig_masks = FALSE;
			}
			dest_mask preference_restrict_option
			{
			    parse_contig_masks = TRUE;

			    $$ = adv_alloc(ADVFT_DM, parse_proto);
			    $$->adv_dm = $2;
			    if (PARSE_METRIC_ISSET(&$3)) {
				if (PARSE_METRIC_ISRESTRICT(&$3)) {
				    BIT_SET($$->adv_flag, ADVF_NO);
				} else {
				    BIT_SET($$->adv_flag, ADVFOT_PREFERENCE);
				    $$->adv_result.res_preference = $3.metric;
				}
			    }
			}
		;

/*  */

/* Support for Export clauses */

export_interior : proto_interior
			{
			    $$ = $1;
			    
			    parse_export_proto = parse_proto = $1.proto;
			    parse_gwlist = $1.gw_list;

			    if (!$1.export_list) {
				(void) sprintf(parse_error,
					       "unknown export protocol: %s",
					       gd_lower(trace_state(rt_proto_bits, $1.proto)));
				PARSE_ERROR;
			    }
			    
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $1.proto)));
			}
		;


export_ospf_ase	: T_OSPF_ASE
			{
			    $$ = parse_export_proto = RTPROTO_OSPF_ASE;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;


prop_source_option
    		: T_RESTRICT
			{
			    PARSE_METRIC_RESTRICT(&$$.metric);
			    $$.adv = (adv_entry *) 0;
			}
		| metric_option '{' prop_source_list '}'
			{
			    $$.metric = $1;
			    $$.adv = $3;
			}
		;

prop_source_list
		: /* Empty */
			{
			    $$ = (adv_entry *) 0;
			}
		| prop_source_list prop_source L_EOS
			{
			    $$ = $1;
			    if (parse_adv_append(&$$, $2, TRUE)) {
				PARSE_ERROR;
			    }
			}
		| prop_source_list error L_EOS
			{
			    yyerrok;
			}
		;

prop_source	: T_PROTO proto_exterior T_AS as prop_metric_option
			{
			    $$ = adv_alloc(ADVFT_AS | ADVF_FIRST, (proto_t) 0);
			    $$->adv_as = $4;
			    $$ = parse_adv_propagate_metric($$, $2, &$5.metric, $5.adv);
			}
    /* AS paths */
		| T_PROTO aspath_prop_init aspath_match prop_metric_option
			{
			    $$ = adv_alloc(ADVFT_ASPATH | ADVF_FIRST, (proto_t) 0);
			    $$->adv_aspath = (caddr_t) $3;
			    $$ = parse_adv_propagate_metric($$, $2, &$4.metric, $4.adv);
			}
    /* Tags */
    		| T_PROTO tag_prop_init T_TAG tag prop_metric_option
			{
			    $$ = adv_alloc(ADVFT_TAG|ADVF_FIRST, (proto_t) 0);
			    $$->adv_tag = $4.metric;
			    $$ = parse_adv_propagate_metric($$, $2, &$5.metric, $5.adv);
			}
    /* Interfaces */
		| T_PROTO prop_direct interface_list_option prop_metric_option
			{
			    $$ = parse_adv_propagate_metric($3, $2, &$4.metric, $4.adv);
			}
    /* Static routes */
		| T_PROTO prop_static interface_list_option prop_metric_option
			{
			    $$ = parse_adv_propagate_metric($3, $2, &$4.metric, $4.adv);
			}
    /* Kernel static routes (routing socket) */
		| T_PROTO prop_kernel interface_list_option prop_metric_option
			{
			    $$ = parse_adv_propagate_metric($3, $2, &$4.metric, $4.adv);
			}
    /* Internal default route */
		| T_PROTO prop_default prop_metric_option
			{
			    $$ = adv_alloc(ADVFT_ANY | ADVF_FIRST, (proto_t) 0);
			    $$ = parse_adv_propagate_metric($$, $2, &$3.metric, $3.adv);
			}
    /* RIP Imports */
		| T_PROTO rip_prop_init interface_gateway_list prop_metric_option
			{
			    $$ = parse_adv_propagate_metric($3, $2, &$4.metric, $4.adv);
			}
    /* HELLO Imports */
		| T_PROTO hello_prop_init interface_gateway_list prop_metric_option
			{
			    $$ = parse_adv_propagate_metric($3, $2, &$4.metric, $4.adv);
			}
    /* OSPF imports */
		| T_PROTO ospf_prop_init prop_metric_option
			{
			    $$ = adv_alloc(ADVFT_ANY | ADVF_FIRST, (proto_t) 0);
			    $$ = parse_adv_propagate_metric($$, $2, &$3.metric, $3.adv);
			}
		| T_PROTO ospf_ase_prop_init prop_metric_option
			{
			    $$ = adv_alloc(ADVFT_ANY | ADVF_FIRST, (proto_t) 0);
			    $$ = parse_adv_propagate_metric($$, $2, &$3.metric, $3.adv);
			}
		;

prop_direct	: T_DIRECT
			{
			    $$ = parse_proto = RTPROTO_DIRECT;
			    parse_gwlist = (gw_entry **)0;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;

prop_static	: T_STATIC
			{
			    $$ = parse_proto = RTPROTO_STATIC;
			    parse_gwlist = &rt_gw_list;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;

prop_kernel	: T_KERNEL
			{
			    $$ = parse_proto = RTPROTO_KERNEL;
			    parse_gwlist = &krt_gw_list;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;

prop_default	: T_DEFAULT
			{
			    $$ = parse_proto = RTPROTO_DEFAULT;
			    parse_gwlist = (gw_entry **)0;
			    trace(TR_PARSE, 0, "parse: %s PROTO: %s",
				  parse_where(),
				  gd_lower(trace_state(rt_proto_bits, $$)));
			}
		;

prop_metric_option
    		: T_RESTRICT
			{
			    PARSE_METRIC_RESTRICT(&$$.metric);
			    $$.adv = (adv_entry *) 0;
			}
		| metric_option prop_restrict_list_option
			{
			    $$.metric = $1;
			    $$.adv = $2;
			}
		;

prop_restrict_list_option
		:	/* Empty */
			{
			    $$ = (adv_entry *) 0;
			}
		| '{' prop_restrict_list '}'
			{
			    $$ = $2;
			}
		;
    

prop_restrict_list
		: /* Empty */
			{
			    $$ = (adv_entry *) 0;
			}
		| prop_restrict_list prop_restrict L_EOS
			{
			    $$ = parse_adv_dm_append($1, $2);
			    if (!$$) {
				PARSE_ERROR;
			    }
			}
		| prop_restrict_list error L_EOS
			{
			    yyerrok;
			}
		;

prop_restrict	:
			{
			    parse_contig_masks = FALSE;
			}
			dest_mask metric_restrict_option
			{
			    parse_contig_masks = TRUE;

			    $$ = adv_alloc(ADVFT_DM, (proto_t) 0);
			    $$->adv_dm = $2;
			    if (PARSE_METRIC_ISRESTRICT(&$3)) {
				BIT_SET($$->adv_flag, ADVF_NO);
			    } else if (PARSE_METRIC_ISSET(&$3)) {
				$$->adv_result.res_metric = $3.metric;
				BIT_SET($$->adv_flag, ADVFOT_METRIC);
			    }
			}
		;

/*  */

/* Addresses */

addr		: T_NOP
			{
			    assert(FALSE);
			}
		| in_addr
			{
			    $$ = sockdup(sockbuild_in(0, $1.s_addr));

			    trace(TR_PARSE, 0, "parse: %s IP_ADDR: %A",
				  parse_where(),
				  $$);
			}
    ;


/* Destination and mask pair used for control lists */
dest_mask	: all_mask
		| default_mask
		| host_mask
		| network_mask
		;

all_mask	: T_ALL
			{
			    $$.dm_dest = $$.dm_mask = (sockaddr_un *) 0;
			    trace(TR_PARSE, 0, "parse: %s DEST: all",
				  parse_where());
			}
		;

default_mask	: T_NOP1
			{
			    assert(FALSE);
			}
		| T_DEFAULT
			{
			    $$.dm_dest = sockdup(inet_addr_default);
			    $$.dm_mask = inet_mask_host;
			    trace(TR_PARSE, 0, "parse: %s DEST: %A",
				  parse_where(),
				  $$.dm_dest);
			}
		;

mask		: T_NOP
			{
			    assert(FALSE);
			}
		| in_addr
			{
			    $$ = inet_mask_locate($1.s_addr);
			    if (!$$) {
				if (parse_contig_masks) {
				    (void) sprintf(parse_error, "mask not contiguous");
				    PARSE_ERROR;
				} else {
				    $$ = mask_locate(sockbuild_in(0, $1.s_addr));
				}
			    }
			    trace(TR_PARSE, 0, "parse: %s MASK: %A",
				  parse_where(),
				  $$);
			}
		;

/* Gateway list */
gateway_list	: gateway
			{
			    $$ = adv_alloc(ADVFT_GW | ADVF_FIRST, (proto_t) 0);
			    $$->adv_gwp = $1;
			}
		| gateway_list gateway
			{
			    adv_entry *adv = adv_alloc(ADVFT_GW, (proto_t) 0);

			    adv->adv_gwp = $2;
			    $$ = $1;
			    if (parse_adv_append(&$$, adv, TRUE)) {
				PARSE_ERROR;
			    }
			}
		;

/* A gateway is a host on an attached network */
gateway		: host
			{
			    /*
			     *	Make sure host is on a locally attached network then
			     *	find or create a gw structure for it.  Requires that
			     *	parse_proto and parse_gwlist are previously set
			     */
			    if (!if_parse_withdst($1)) {
				(void) sprintf(parse_error, "gateway not a host address on an attached network: '%A'",
					       $1);
				if (BIT_TEST(task_state, TASKS_STRICTIFS)) {
				    PARSE_ERROR;
				} else {
				    yyerror(parse_error);
				}
			    }
			    if (!parse_gwlist) {
				(void) sprintf(parse_error, "gateway specification not valid for %s",
					       trace_state(rt_proto_bits, parse_proto));
				PARSE_ERROR;
			    }
			    $$ = gw_locate(parse_gwlist,
					   parse_proto,
					   (task *) 0,
					   (as_t) 0,
					   (as_t) 0,
					   (time_t) 0,
					   $1,
					   (flag_t) 0);
			    trace(TR_PARSE, 0, "parse: %s GATEWAY: %A  PROTO: %s",
				  parse_where(),
				  $$->gw_addr,
				  gd_lower(trace_state(rt_proto_bits, $$->gw_proto)));
			    sockfree($1);
			}
		;

/* A host is a host */
host		: host_name
			{ 
			    sockaddr_un *addr;

			    if (BIT_TEST(task_state, TASKS_NORESOLV)
				|| !(addr = parse_addr_hostname($1.ptr))) {
				PARSE_ERROR;
			    }
			    $$ = sockdup(addr);
			    trace(TR_PARSE, 0, "parse: %s HOST: %A",
				  parse_where(),
				  $$);
			    free_charstr(&$1);
			}
		| addr
			{
			    $$ = $1;
			    trace(TR_PARSE, 0, "parse: %s HOST: %A",
				  parse_where(),
				  $$);
			}
		;

host_mask	: T_HOST host
			{
			    $$.dm_dest = $2;
			    $$.dm_mask = sockhostmask($2);

			    trace(TR_PARSE, 0, "parse: %s DEST: %A",
				  parse_where(),
				  $$.dm_dest);
			}
		;

/* A network is a network */
network		: host_name
			{
			    sockaddr_un *addr;

			    if (BIT_TEST(task_state, TASKS_NORESOLV)
				|| !(addr = parse_addr_netname($1.ptr))) {
				PARSE_ERROR;
			    }
			    $$ = sockdup(addr);
			    trace(TR_PARSE, 0, "parse: %s NETWORK: %A",
				  parse_where(),
				  $$);
			    free_charstr(&$1);
			}
		| addr
			{
			    $$ = $1;
			    trace(TR_PARSE, 0, "parse: %s NETWORK: %A",
				  parse_where(),
				  $$);
			}
		;

network_mask	: network
			{
			    $$.dm_dest = $1;
			    switch (socktype($1)) {
#ifdef	PROTO_INET
			    case AF_INET:
				$$.dm_mask = inet_mask_natural($1);
				if (sock2ip($$.dm_dest) & ~sock2ip($$.dm_mask)) {
				    sprintf(parse_error, "host or mask option needed");
				    PARSE_ERROR;
				}
				break;
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
			    case AF_ISO:
				$$.dm_mask = iso_mask_natural($1);
				break;
#endif	/* PROTO_ISO */
			    default:
				assert(FALSE);
			    }
			    
			    trace(TR_PARSE, 0, "parse: %s DEST: %A MASK: %A",
				  parse_where(),
				  $$.dm_dest,
				  $$.dm_mask);
			}
		| network T_MASKLEN number
			{
			    $$.dm_dest = $1;
			    switch (socktype($1)) {
#ifdef	PROTO_INET
			    case AF_INET:
				if (parse_limit_check("inet mask bits",
						      (u_int) $3,
						      INET_LIMIT_MASKLEN)) {
				    PARSE_ERROR;
				}
				$$.dm_mask = inet_masks[$3];
				break;
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
			    case AF_ISO:
				if (parse_limit_check("iso mask bits",
						      (u_int) $3,
						      ISO_LIMIT_MASKLEN)) {
				    PARSE_ERROR;
				}
				$$.dm_mask = iso_masks[$3];
				break;
#endif	/* PROTO_ISO */
			    default:
				assert(FALSE);
			    }
			    trace(TR_PARSE, 0, "parse: %s DEST: %A MASK: %A",
				  parse_where(),
				  $$.dm_dest,
				  $$.dm_mask);
			}
		| network T_MASK mask
			{
			    register byte *ap = $1->a.sa_data;
			    register byte *mp = $3->a.sa_data;
			    register byte *alp = (byte *) $1 + socksize($1);
			    register byte *mlp = (byte *) $3 + socksize($3);

			    if (socktype($1) != socktype($3)) {
				sprintf(parse_error, "Network %A and mask %A are not the same family",
					$1,
					$3);
				PARSE_ERROR;
			    }
			    
			    /* Skip zero fields */
			    while (!*ap && ap < alp) {
				ap++, mp++;
			    }
			    
			    while (ap < alp) {
				if (mp < mlp) {
				    if (*ap++ & ~*mp++) {
					break;
				    }
				} else {
				    if (*ap++) {
					break;
				    }
				}
			    }
			    if (ap < alp) {
				sprintf(parse_error, "Non-masked bits not zero for %A mask %A",
					$1,
					$3);
				PARSE_ERROR;
			    }

			    $$.dm_dest = $1;
			    $$.dm_mask = $3;
			    trace(TR_PARSE, 0, "parse: %s DEST: %A MASK: %A",
				  parse_where(),
				  $$.dm_dest,
				  $$.dm_mask);
			}
		;

/* IP address */
in_addr		: L_NUMBER
			{
			    u_int32 addr = $1;
			    
			    if (addr) {
				/* Normalize the address */
				while (!(addr & 0xff000000)) {
				    addr <<= 8;
				}
			    }
			    $$.s_addr = htonl(addr);
			}
		| byte_string
			{
			    int i;
			    u_int32 addr = 0;

			    if ($1.len > sizeof(addr)) {
				sprintf(parse_error, "octet or hex string too long to be an IP address");
				PARSE_ERROR;
			    }

			    for (i = 0; i < sizeof(addr); i++) {
				addr <<= 8;
				if (i < $1.len) {
				    BIT_SET(addr, $1.ptr[i]);
				}
			    }
			    $$.s_addr = htonl(addr);
			    free_bytestr(&$1);
			}
		;

/* Host name */
host_name	: L_HNAME
			{
			    $$ = $1;
			    $$.ptr = task_mem_strdup((task *) 0, $1.ptr);
			}
		| L_KEYWORD
			{
			    $$ = $1;
			    $$.ptr = task_mem_strdup((task *) 0, $1.ptr);
			}
		| string
		;

/* Protocols */
proto_interior	: T_REDIRECT
			{
#if	defined(PROTO_ICMP) || defined(KRT_RT_SOCK)
			    $$.proto = RTPROTO_REDIRECT;
			    $$.import_list = &redirect_import_list;
			    $$.export_list = (adv_entry **) 0;
			    $$.gw_list = &redirect_gw_list;
#endif	/* defined(PROTO_ICMP) || defined(KRT_RT_SOCK) */
			}
		| T_HELLO
			{
			    $$.proto = RTPROTO_HELLO;
			    $$.import_list = &hello_import_list;
			    $$.export_list = &hello_export_list;
			    $$.gw_list = &hello_gw_list;
			}
		| T_RIP
			{
			    $$.proto = RTPROTO_RIP;
			    $$.import_list = &rip_import_list;
			    $$.export_list = &rip_export_list;
			    $$.gw_list = &rip_gw_list;
			}
		;


proto_exterior	: T_NOP
			{
			    assert(FALSE);
			}
		| egp_prop_init
		| bgp_prop_init
		;

onoff_option	: T_ON		{ $$ = T_ON; }
		| T_OFF		{ $$ = T_OFF; }
		;

/* Metric */

metric		: number
			{
			    PARSE_METRIC_SET(&$$, $1);
			    trace(TR_PARSE, 0, "parse: %s METRIC: %d",
				  parse_where(),
				  $$.metric);
			}
		| T_INFINITY
			{
			    PARSE_METRIC_INFINITY(&$$);
			    trace(TR_PARSE, 0, "parse: %s METRIC: infinity",
				  parse_where());
			}
		;

metric_option	: /* Empty */
			{
			    PARSE_METRIC_CLEAR(&$$);
			}
		| T_METRIC metric
			{
			    $$ = $2;
			}
		;
		
metric_restrict_option
		: /* Empty */
			{
			    PARSE_METRIC_CLEAR(&$$);
			}
		| T_RESTRICT
			{
			    PARSE_METRIC_RESTRICT(&$$);
			}
		| T_METRIC metric
			{
			    if (parse_metric_check(parse_export_proto, &$2)) {
				PARSE_ERROR;
			    }
			    $$ = $2;
			}
		;
		
/* Preference */
preference_option
		: /* Empty */
			{
			    PARSE_METRIC_CLEAR(&$$);
			}
		| T_PREFERENCE preference
			{
			    PARSE_METRIC_SET(&$$, $2);
			}
		;

preference_restrict_option
		: preference_option
		| T_RESTRICT
			{
			    PARSE_METRIC_RESTRICT(&$$);
			}
		;

preference	: L_NUMBER
			{
			    if (parse_limit_check("preference",
						  (u_int) $1,
						  LIMIT_PREFERENCE)) {
				PARSE_ERROR;
			    }
			    $$ = $1;
			}
		;


/* Numbers and such */
string		: L_STRING
			{
			    /* Remove quotes from the string if present */
			    char *cp;

			    $$ = $1;
				
			    switch (*$$.ptr) {
			    case '<':
				*$$.ptr = '>';
				/* Fall through */

			    case '"':
				cp = &$$.ptr[$$.len-1];
				if (*cp == *$$.ptr) {
				    *cp = (char) 0;
				    $$.len -= 2;
				    cp = $$.ptr + 1;
				    break;
				}
				/* Fall Through */

			    default:
				cp = $$.ptr;
			    }

			    $$.ptr = task_mem_strdup((task *) 0, cp);
			    trace(TR_PARSE, 0, "parse: %s STRING: \"%s\" length: %d",
				  parse_where(),
				  $$.ptr,
				  $$.len);
			}
		;


number		: L_NUMBER
		| byte_string
			{
			    int i = 0;
			    int len = sizeof($$);

			    $$ = 0;
			    if ($1.len > len) {
				sprintf(parse_error, "octet or hex string too long to be a number");
				PARSE_ERROR;
			    } else if ($1.len < len) {
				len = $1.len;
			    }

			    while (i < len) {
				$$ = ($$ << 8) | $1.ptr[i++];
			    }
			    free_bytestr(&$1);
			}
		;


byte_string	: L_HEXSTRING
		| L_OCTETSTRING
		;

time		: L_NUMBER
			{
			    if (parse_limit_check("seconds",
						  (u_int) $1,
						  LIMIT_NATURAL)) {
				PARSE_ERROR;
			    };
			    $$ = $1;
			}
		| L_NUMBER ':' L_NUMBER
			{
			    if (parse_limit_check("minutes",
						  (u_int) $1,
						  LIMIT_NATURAL)) {
				PARSE_ERROR;
			    }
			    if (parse_limit_check("seconds",
						  (u_int) $3,
						  LIMIT_SECONDS)) {
				PARSE_ERROR;
			    }
			    $$ = ($1 * 60) + $3;
			}
		| L_NUMBER ':' L_NUMBER ':' L_NUMBER
			{
			    if (parse_limit_check("hours",
						  (u_int) $1,
						  LIMIT_NATURAL)) {
				PARSE_ERROR;
			    }
			    if (parse_limit_check("minutes",
						  (u_int) $3,
						  LIMIT_MINUTES)) {
				PARSE_ERROR;
			    }
			    if (parse_limit_check("seconds",
						  (u_int) $5,
						  LIMIT_SECONDS)) {
				PARSE_ERROR;
			    }
			    $$ = (($1 * 60) + $3) * 60 + $5;
			}
		;

/* Internet ports */
port		: L_NUMBER
			{
			    if (parse_limit_check("port",
						  (u_int) $1,
						  LIMIT_PORT)) {
				PARSE_ERROR;
			    }
			    $$ = htons($1);
			}
		| L_KEYWORD
			{
			    const struct servent *sp;

			    if (!(sp = getservbyname($1.ptr, parse_serv_proto))) {
				(void) sprintf(parse_error, "unknown protocol %s/%s",
					$1.ptr, parse_serv_proto);
				PARSE_ERROR;
			    }

			    $$ = sp->s_port;

			    trace(TR_PARSE, 0, "parse: %s PORT %s (%d)",
				  parse_where(),
				  $1.ptr,
				  ntohs($$));
			}
		;

/**/
/* Autonomous systems */

as		: L_NUMBER
			{
			    if (parse_limit_check("autonomous system",
						  (u_int) $1,
						  LIMIT_AS)) {
				PARSE_ERROR;
			    }
			    $$ = $1;
			}
		;

as_list		: as
			{
			    $$ = adv_alloc(ADVFT_AS | ADVF_FIRST, (proto_t) 0);
			    $$->adv_as = $1;
			}
		| as_list as
			{
			    adv_entry *adv = adv_alloc(ADVFT_AS, (proto_t) 0);

			    adv->adv_as = $2;
			    $$ = $1;
			    if (parse_adv_append(&$$, adv, TRUE)) {
				PARSE_ERROR;
			    }
			}
		;

as_list_option	: /* Empty */
			{
			    $$ = (adv_entry *) 0;
			}
		| T_AS as_list
			{
			    $$ = $2;
			}
		;

/**/
/* Tags */

/* Route tags */
tag_as_option
		: /* Empty */
			{
			    PARSE_METRIC_CLEAR(&$$);
			}
		| T_TAG tag_as
			{
			    $$ = $2;
			}
		;

tag_as		: tag
		| T_AS
			{
			    PARSE_METRIC_ALTERNATE(&$$, 0);
			}
		| T_AS number
			{
			    if (parse_limit_check("external-route-tag",
						  (u_int) $2,
						  PATH_OSPF_TAG_USR_LIMIT)) {
				PARSE_ERROR;
			    }
			    PARSE_METRIC_ALTERNATE(&$$, $2);
			}
		;

tag_option	: /* Empty */
			{
			    PARSE_METRIC_CLEAR(&$$);
			}
		| T_TAG tag
			{
			    $$ = $2;
			}
		;

tag	: number
			{
			    if (parse_limit_check("external-route-tag",
						  (u_int) $1,
						  PATH_OSPF_TAG_LIMIT)) {
				PARSE_ERROR;
			    }
			    PARSE_METRIC_SET(&$$, $1);
			}
/* XXX - various tag options */
		;

tag_prop_init
		: proto_exterior
		| rip_prop_init 
		| hello_prop_init 
		| ospf_prop_init
		| ospf_ase_prop_init
		;

/**/
/* AS paths */

aspath_prop_init
		: proto_exterior
		| rip_prop_init 
		| hello_prop_init 
		| ospf_prop_init
		| ospf_ase_prop_init
		;

aspath_match	: T_ASPATH
			{
			    aspath_state = 0;
			    aspath_current = (asmatch_t *) task_block_alloc(aspath_match_index);
			}
		aspath_regex T_ORIGIN aspath_origin
    			{
			    $$ = aspath_current;
			    $$->origin_mask = $5;
			    trace(TR_PARSE, 0, "%s: path %X origins %s",
				  parse_where(),
				  $$,
				  aspath_adv_origins($$->origin_mask));
			    aspath_current = (asmatch_t *) 0;
			}
    		;

aspath_origin	: T_ANY
			{
			    /* Set all bits on */
			    $$ = -1;
			}
		| aspath_origin_list
		;

aspath_origin_list
		: aspath_origins
		| aspath_origin_list '|' aspath_origins
			{
			    $$ = $1 | $3;
			}
		;

aspath_origins	: T_IGP
			{
			    $$ = (1 << PATH_ORG_IGP);
			}
		| T_EGP
			{
			    $$ = (1 << PATH_ORG_EGP);
			}
		| T_INCOMPLETE
    			{
			    $$ = (1 << PATH_ORG_XX);
			}
		;
    
aspath_regex	: aspath_sum
			{
			    NFA_ASG($$.start, $1.start);
			    NFA_ASG(aspath_current->start, $$.start);
			    trace(TR_PARSE, 0, "%s: REDUCED:	regex : sum",
				  parse_where());
			}
		| aspath_sum '|' aspath_regex
			{
			    NFA_OR($$.start, $1.start, $3.start);
			    NFA_OR($$.state, $1.state, $3.state);
			    NFA_ASG(aspath_current->start, $$.start);
			    trace(TR_PARSE, 0, "%s: REDUCED:	regex : sum | regex",
				  parse_where());
			}
		;

aspath_sum	: aspath_term
			{
			    trace(TR_PARSE, 0, "%s: REDUCED:	sum : term",
				  parse_where());
			}
		| aspath_term aspath_sum
			{
			    int i;
			    
			    NFA_OR($$.state, $1.state, $2.state);

			    for (i = 0; i < ASP_ACC; i++) {
				int nonzero;
				
				if (NFA_ISSET($1.state, i)) {
				    if (NFA_ISSET(aspath_current->nfa[i].next, ASP_ACC)) {
					NFA_CLR(aspath_current->nfa[i].next, ASP_ACC);
					NFA_OR(aspath_current->nfa[i].next, aspath_current->nfa[i].next, $2.start);
				    }
				    NFA_CLR($1.state, i);
				    NFA_NONZERO($1.state, nonzero);
				    if (nonzero) {
					break;
				    }
				}
			    }
			    if (i > ASP_ACC) {
				YYERROR;
			    }
			    if (NFA_ISSET($1.start, ASP_ACC)) {
				NFA_CLR($1.start, ASP_ACC);
				NFA_OR($1.start, $1.start, $2.start);
			    }
			    NFA_ASG($$.start, $1.start);
			    trace(TR_PARSE, 0, "%s: REDUCED:	sum : term sum",
				  parse_where());
			}
		;

aspath_term	: aspath_symbol
			{
			    trace(TR_PARSE, 0, "%s: REDUCED:	term : symbol",
				  parse_where());
			}
		| aspath_symbol '*'
			{
			    int i;
			    
			    for (i = 0; i < ASP_ACC; i++) {
				if (NFA_ISSET($1.state, i)) {
				    int nonzero;
				
				    if (NFA_ISSET(aspath_current->nfa[i].next, ASP_ACC)) {
					NFA_OR(aspath_current->nfa[i].next, aspath_current->nfa[i].next, $1.start);
				    }
				    NFA_CLR($1.state, i);
				    NFA_NONZERO($1.state, nonzero);
				    if (nonzero) {
					break;
				    }
				}
			    }
			    NFA_ASG($$.start, $1.start);
			    NFA_SET($$.start, ASP_ACC);
			    trace(TR_PARSE, 0, "%s: REDUCED:	term : symbol *",
				  parse_where());
			}
		| aspath_symbol '+'
			{
			    int i;
			    
			    for (i = 0; i < ASP_ACC; i++) {
				if (NFA_ISSET($1.state, i)) {
				    int nonzero;
				    
				    if (NFA_ISSET(aspath_current->nfa[i].next, ASP_ACC)) {
					NFA_OR(aspath_current->nfa[i].next, aspath_current->nfa[i].next, $1.start);
				    }
				    NFA_CLR($1.state, i);
				    NFA_NONZERO($1.state, nonzero);
				    if (nonzero) {
					break;
				    }
				}
			    }
			    NFA_ASG($$.start, $1.start);
			    trace(TR_PARSE, 0, "%s: REDUCED:	term : symbol +",
				  parse_where());
			}
		| aspath_symbol '?'
			{
			    NFA_ASG($$.start, $1.start);
			    NFA_SET($$.start, ASP_ACC);
			    trace(TR_PARSE, 0, "%s: REDUCED:	term : symbol ?",
				  parse_where());
			}
		| aspath_symbol '{' aspath_range '}'
			{
			    int copy;
			    flag_t finalstate[1 + ASP_ACC / ASP_WORD];
			    flag_t newstate[1 + ASP_ACC / ASP_WORD];
			    flag_t oldstate[1 + ASP_ACC / ASP_WORD];
			    flag_t newstart[1 + ASP_ACC / ASP_WORD];
			    
			    NFA_ASG(finalstate, $1.state);
			    NFA_ASG(newstate, $1.state);
			    NFA_ASG($$.start, $1.start);
			    NFA_ASG(newstart, $1.start);
			    if (!$3.begin) {
				NFA_SET($$.start, ASP_ACC);
			    }
			    for (copy = 1; copy < $3.begin; copy++) {
				int i;
				
				NFA_ASG(oldstate, $1.state);
				NFA_ZERO(newstart);
				NFA_ZERO(newstate);
				for (i = 0; i < ASP_ACC; i++) {
				    if (NFA_ISSET($1.state, i)) {
					aspath_current->nfa[aspath_state].as = aspath_current->nfa[i].as;
					NFA_ASG(aspath_current->nfa[aspath_state].next, aspath_current->nfa[i].next);
					NFA_SHIFT(aspath_current->nfa[aspath_state].next, aspath_state-i);
					NFA_SET(newstate, aspath_state);
					if (NFA_ISSET($1.start, i)) {
					    int nonzero;
					    
					    NFA_SET(newstart, aspath_state);
					    ASPATH_NEXT_STATE;
					    NFA_CLR($1.state, i);
					    NFA_NONZERO($1.state, nonzero);
					    if (nonzero) {
						break;
					    }
					}
				    }
				    if (NFA_ISSET($$.start, ASP_ACC)) {
					NFA_OR($$.start,$$.start,newstart);
				    }
				    for (i = 0; i < ASP_ACC; i++) {
					if (NFA_ISSET(oldstate, i)) {
					    int nonzero;
					    
					    if (NFA_ISSET(aspath_current->nfa[i].next, ASP_ACC)) {
						if (copy < $3.begin) {
						    NFA_CLR(aspath_current->nfa[i].next, ASP_ACC);
						}
						NFA_OR(aspath_current->nfa[i].next, aspath_current->nfa[i].next, newstart);
					    }
					    NFA_CLR(oldstate, i);
					    NFA_NONZERO(oldstate, nonzero);
					    if (nonzero) {
						break;
					    }
					}
				    }
				    NFA_OR(finalstate, finalstate, newstate);
				    NFA_ASG($1.start, newstart);
				    NFA_ASG($1.state, newstate);
				}
				if ($3.end) {
				    for (i = 0; i < ASP_ACC; i++) {
					if (NFA_ISSET(newstate, i)) {
					    int nonzero;
					    
					    if (NFA_ISSET(aspath_current->nfa[i].next, ASP_ACC)) {
						NFA_OR(aspath_current->nfa[i].next, aspath_current->nfa[i].next, newstart);
					    }
					    NFA_CLR(newstate, i);
					    NFA_NONZERO(newstate, nonzero);
					    if (nonzero) {
						break;
					    }
					}
				    }
				}	
				NFA_ASG($$.state, finalstate);
				trace(TR_PARSE, 0, "%s: REDUCED:	term : symbol { range }",
				      parse_where());
			    }
			}
		;

aspath_symbol	: as
			{
			    NFA_ZERO($$.start);
			    NFA_ZERO($$.state);
			    NFA_SET($$.start, aspath_state);
			    NFA_SET($$.state, aspath_state);
			    aspath_current->nfa[aspath_state].as = htons($1);
			    NFA_SET(aspath_current->nfa[aspath_state].next, ASP_ACC);
			    ASPATH_NEXT_STATE;
			    trace(TR_PARSE, 0, "%s: REDUCED:	symbol : %d",
				  parse_where(),
				  $1);
			}
		| '.'
			{
			    NFA_ZERO($$.start);
			    NFA_ZERO($$.state);
			    NFA_SET($$.start, aspath_state);
			    NFA_SET($$.state, aspath_state);
			    NFA_SET(aspath_current->nfa[aspath_state].next, ASP_ACC);
			    aspath_current->nfa[aspath_state].as = 0;
			    ASPATH_NEXT_STATE;
			    trace(TR_PARSE, 0, "%s: REDUCED:	symbol : .",
				  parse_where());
			}
		| '(' aspath_regex ')'
			{
			    NFA_ASG($$.state, $2.state);
			    NFA_ASG($$.start, $2.start);
			    trace(TR_PARSE, 0, "%s: REDUCED:	symbol : ( regex )",
				  parse_where());
			}

aspath_range	: number
			{
			    $$.begin = $1;
			    $$.end = 0;
			    if (!$1) {
				sprintf(parse_error, "invalid range start: %d",
					$1);
				PARSE_ERROR;
			    }
			    trace(TR_PARSE, 0, "%s: RANGE: {%d}",
				  parse_where(),
				  $$.begin);
			}
		| number ','
			{
			    $$.begin = $1;
			    $$.end = 1;
			    if (!$1) {
				sprintf(parse_error, "invalid range start: %d",
					$1);
				PARSE_ERROR;
			    }
			    trace(TR_PARSE, 0, "%s: RANGE: {%d,}",
				  parse_where(),
				  $$.begin);
			}
		| number ',' number
			{
			    $$.begin = $1;
			    $$.end = $3;
			    if (!$1) {
				sprintf(parse_error, "invalid range start: %d",
					$1);
				PARSE_ERROR;
			    }
			    if ($3 < $1 || !$3) {
				sprintf(parse_error, "invalid range end: %d",
					$3);
				PARSE_ERROR;
			    }
			    trace(TR_PARSE, 0, "%s: RANGE: {%d,%d}",
				  parse_where(),
				  $$.begin,
				  $$.end);
			}
		;
%%

/*
 *	Log any parsing errors
 */
static void
yyerror(s)
const char *s;
{
    byte *cp;

    trace(TR_ALL, 0, NULL);
    tracef("parse: %s ",
	   parse_where());

    switch (yychar) {
    case L_HEXSTRING:
	tracef("%s at '",
	       s);
	for (cp = yylval.hex.ptr; (cp - yylval.hex.ptr) < yylval.hex.len; cp++) {
	    tracef("%02x", *cp);
	}
	tracef("'");
	break;

    case L_OCTETSTRING:
	tracef("%s at '",
	       s);
	for (cp = yylval.hex.ptr; (cp - yylval.hex.ptr) < yylval.hex.len; cp++) {
	    tracef("%s%u",
		   (cp == yylval.hex.ptr) ? "" : ".",
		   *cp);
	}
	tracef("'");
	break;

    case L_NUMBER:
	tracef("%s at '%d'",
	       s,
	       yylval.num);
	break;

    case L_EOS:
	tracef("%s at 'end-of-statement'",
	       s);
	break;

    case L_ERROR:
	tracef("%s",
	       parse_error);
	break;

#ifdef	YYEOF
    case YYEOF:
#else	/* YYEOF */
    case 0:
#endif	/* YYEOF */
	tracef("%s at 'end-of-file'",
		   s);
	break;

    default:
	if (!parse_keyword_lookup(yychar)) {
#if	YYDEBUG != 0
#ifdef	YYTRANSLATE
	    tracef("%s at '%s'",
		   s,
		   yytname[YYTRANSLATE(yychar)]);
#else	/* YYTRANSLATE */
	    tracef("%s at '%s'",
		   s,
		   yyname[yychar]);
#endif	/* YYTRANSLATE */
#else	/* YYDEBUG */
	    tracef("%s",
		   s);
#endif	/* YYDEBUG */
	    break;
	}
	/* Fall Through */

    case L_STRING:
    case L_HNAME:
    case L_KEYWORD:
	tracef("%s at '%s' length %d",
	       s,
	       yylval.str.ptr,
	       yylval.str.len);
	break;

#ifdef	YYEMPTY
    case YYEMPTY:
	tracef("%s",
	       s);
	break;
#endif	/* YYEMPTY */
    }
    trace(TR_ALL, LOG_ERR, NULL);
    trace(TR_ALL, 0, NULL);
}



/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
