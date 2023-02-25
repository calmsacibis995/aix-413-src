/* @(#)35	1.4  src/tcpip/usr/sbin/gated/bgp.h, tcprouting, tcpip411, GOLD410 12/6/93 17:32:37 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: BGPBUF_COMPACT
 *		BGPBUF_DATA
 *		BGPBUF_FULL
 *		BGPBUF_LEFT
 *		BGPBUF_SPACE
 *		BGPCONN_DONE
 *		BGPCONN_INTERVAL
 *		BGPCONN_NEXTSLOT
 *		BGPCONN_SLOT
 *		BGPSORT_ADDR
 *		BGPSORT_LT
 *		BGP_ADD_AUTH
 *		BGP_CHECK_AUTH
 *		BGP_GET_ADDR
 *		BGP_GET_BYTE
 *		BGP_GET_HDRLEN
 *		BGP_GET_HDRTYPE
 *		BGP_GET_HEADER
 *		BGP_GET_LONG
 *		BGP_GET_NETLONG
 *		BGP_GET_NOTIFY
 *		BGP_GET_OPEN_V2
 *		BGP_GET_OPEN_V3
 *		BGP_GET_SHORT
 *		BGP_GET_UPDATE
 *		BGP_GET_VERSION
 *		BGP_GROUP_LIST_END
 *		BGP_KNOWN_VERSION
 *		BGP_MAKE_CODES
 *		BGP_NEEDS_SHAREDIF
 *		BGP_OPTIONAL_SHAREDIF
 *		BGP_PEER_LIST
 *		BGP_PEER_LIST_END
 *		BGP_PUT_ADDR
 *		BGP_PUT_BYTE
 *		BGP_PUT_HDRLEN
 *		BGP_PUT_HDRTYPE
 *		BGP_PUT_LONG
 *		BGP_PUT_NETLONG
 *		BGP_PUT_NOTIFY
 *		BGP_PUT_OPEN_V2
 *		BGP_PUT_OPEN_V3
 *		BGP_PUT_SHORT
 *		BGP_SKIP_ATTRLEN
 *		BGP_SKIP_HEADER
 *		BGP_SORT_LIST_END
 *		BGP_SPBUF_ENDBUF
 *		BGP_SPBUF_HEADER_LEN
 *		BGP_SPBUF_STARTBUF
 *		BGP_USES_SHAREDIF
 *		PROTOTYPE
 *		bgp_code
 *		bgp_find_peer_by_addr
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
 * bgp.h,v 1.16 1993/04/15 16:06:08 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	BGP_PORT		179	/* Port number to use with BGP */
#define	BGPMAXPACKETSIZE	4096	/* Maximum packet size */
#define	BGPMAXSENDPKTSIZE	1024	/* Largest we try to send (<= 4096) */
#define	BGPRECVBUFSIZE		4096	/* Size of receive buffers (>= 4096) */

/*
 * The default size we set receive and send buffers to, before we have
 * figured this out by looking at the peer/group configuration
 */
#define	BGP_RECV_BUFSIZE	BGPMAXPACKETSIZE
#define	BGP_SEND_BUFSIZE	BGPMAXPACKETSIZE

/*
 * The number of bytes we read out of the TCP socket at once before
 * allowing other things to happen.  I think we can get 20kb out on
 * an RT in a few seconds if we're not tracing, so this may be okay.
 */
#define	BGPMAXREAD		20000

/*
 * XXX I'm not sure what this is for.  Oh, well.
 */
#define	BGP_CLOSE_TIMER		45

/*
 * We schedule connect attempts into slots to try to minimize the number
 * of peers which we attempt to connect to simultaneously.  The slot size
 * is the basic unit of time granularity for specifying connect times.
 */
#define	BGPCONN_SLOTSIZE	4	/* 4 second slots, must be power of 2 */
#define	BGPCONN_SLOTSHIFT	2	/* 4 == 2^2 */
#define	BGPCONN_INSLOTMASK	(BGPCONN_SLOTSIZE - 1)
#define	BGPCONN_N_SLOTS		64	/* 256 seconds worth of slots */
#define	BGPCONN_SLOTMASK	(BGPCONN_N_SLOTS - 1)
#define	BGPCONN_SLOTDELAY	5	/* delay by up to 5 slots (20 sec) */

#define	BGPCONN_SLOT(interval) \
    ((((interval)+bgp_time_sec) >> BGPCONN_SLOTSHIFT) & BGPCONN_SLOTMASK)
#define	BGPCONN_NEXTSLOT(slot) (((slot) + 1) & BGPCONN_SLOTMASK)
#define	BGPCONN_DONE(bnp) \
    do { \
	if (bnp->bgp_connect_slot != 0) { \
	    bgp_connect_slots[bnp->bgp_connect_slot - 1]--; \
	    bnp->bgp_connect_slot = 0; \
	} \
    } while (0)

/*
 * How long to wait before retrying connect (should be multple of slot size)
 */
#define	BGPCONN_SHORT		32
#define	BGPCONN_MED		64
#define	BGPCONN_LONG		148
#define	BGPCONN_INIT		12	/* after initialization */
#define	BGPCONN_IFUP		4	/* after interface comes up */

#define	BGPCONN_F_SHORT		2
#define	BGPCONN_F_MED		5

#define	BGPCONN_INTERVAL(bnp) \
    (((bnp)->bgp_connect_failed <= BGPCONN_F_SHORT) ? BGPCONN_SHORT : \
    (((bnp)->bgp_connect_failed <= BGPCONN_F_MED) ? BGPCONN_MED : BGPCONN_LONG))

/*
 * Hold times.  We advertise a hold time of 180 seconds and decline to
 * accept a hold time of less than 20 seconds (as this would have us
 * sending a keepalive every 6 seconds).
 */
#define	BGP_HOLDTIME		180	/* What we advertise */
#define	BGP_MIN_HOLDTIME	20	/* What we'll accept */
#define	BGP_TIME_ERROR		5	/* We are about this accurate in time */

#define	bgp_time_sec	time_sec	/* use system time */

/*
 * Timeout for open message reception.  Set this equal to the holdtime for now.
 */
#define	BGP_OPEN_TIMEOUT	BGP_HOLDTIME

/*
 * Timeout for listening after initialization.  Should be
 * shorter than BGPCONN_INIT
 */
#define	BGP_LISTEN_TIMEOUT	10

/* For parser */
#define	BGP_METRIC_MAX		65535
#define	BGP_METRIC_MIN		0
#define	BGP_METRIC_NONE		((metric_t) -1)
#define	BGP_LIMIT_METRIC	BGP_METRIC_MIN,BGP_METRIC_MAX

#define	BGP_HOLDTIME_MIN	0
#define	BGP_HOLDTIME_MAX	65535

#define	BGP_LIMIT_HOLDTIME	BGP_HOLDTIME_MIN, BGP_HOLDTIME_MAX

#define	BGP_LIMIT_KBUF		BGPMAXPACKETSIZE, (metric_t) -1
#define	BGP_LIMIT_SBUF		0, (metric_t) -1

/*
 * BGP message types
 */
#define	BGP_OPEN	1		/* open message */
#define	BGP_UPDATE	2		/* update message */
#define	BGP_NOTIFY	3		/* notification message */
#define	BGP_KEEPALIVE	4		/* keepalive message */


/*
 * BGP versions.  For the moment we understand two, version 2 and
 * version 3.  The differences in the protocol are slight.
 */
#define	BGP_VERSION_UNSPEC	0
#define	BGP_VERSION_2		2
#define	BGP_VERSION_3		3
#define	BGP_KNOWN_VERSION(version) \
	((version) == BGP_VERSION_2 || (version) == BGP_VERSION_3)
#define	BGP_BEST_VERSION	BGP_VERSION_3
#define	BGP_WORST_VERSION	BGP_VERSION_2

/*
 * Minimum length of a BGP message is the length of the header.  If you
 * haven't got this, you haven't got anything.
 */
#define	BGP_HEADER_LEN	19

/*
 * Authentication types.  We only know one type at this point.
 */
#define	BGP_AUTH_NONE	0


/*
 * AS numbers of significance to BGP
 */
#define	BGP_AS_ANON	0
#define	BGP_AS_HIGH	65535


/*
 * BGP message parsing.  We assume no particular alignment anywhere
 * so that we can parse a stream.  Because of this we use no structure
 * overlays.  Instead we use macros which know the packet formats to
 * pull values out of the stream.  This avoids most byte order issues
 * as well.
 */

/*
 * The length of a BGP route in the message (4 bytes currently).
 */
#define	BGP_ROUTE_LENGTH	4

/*
 * Macros to get various length values from the stream.  cp must be a
 * (byte *)
 */
#define	BGP_GET_BYTE(val, cp)	((val) = *(cp)++)

#define	BGP_GET_SHORT(val, cp) \
	do { \
		register unsigned int Xv; \
		Xv = (*(cp)++) << 8; \
		Xv |= *(cp)++; \
		(val) = Xv; \
	} while (0)

#define	BGP_GET_LONG(val, cp) \
	do { \
		register u_long Xv; \
		Xv = (*(cp)++) << 24; \
		Xv |= (*(cp)++) << 16; \
		Xv |= (*(cp)++) << 8; \
		Xv |= *(cp)++; \
		(val) = Xv; \
	} while (0)

#define	BGP_GET_NETLONG(val, cp) \
	do { \
		register u_char *Xvp; \
		u_long Xv; \
		Xvp = (u_char *) &Xv; \
		*Xvp++ = *(cp)++; \
		*Xvp++ = *(cp)++; \
		*Xvp++ = *(cp)++; \
		*Xvp++ = *(cp)++; \
		(val) = Xv; \
	} while (0)

/*
 * Extract the BGP header from the stream.  Note that a pointer to
 * the marker is returned, rather than the marker itself.
 *
 * The header is a 16 byte marker, followed by a 2 byte length and
 * a 1 byte message type code.
 */
#define	BGP_HEADER_MARKER_LEN	16

#define	BGP_GET_HEADER(marker, length, type, cp) \
	do { \
		(marker) = (cp); \
		(cp) += BGP_HEADER_MARKER_LEN; \
		BGP_GET_SHORT((length), (cp)); \
		BGP_GET_BYTE((type), (cp)); \
	} while (0)

#define	BGP_GET_HDRLEN(length, cp) \
	do { \
		register int Xlen; \
		Xlen = ((int)*((cp) + 16)) << 8; \
		Xlen |= (int)*((cp) + 17); \
		(length) = Xlen; \
	} while (0)

#define	BGP_GET_HDRTYPE(type, cp)	((type) = *((cp) + 18))


/*
 * Extract open message data.  We treat version 2 and version 3
 * open messages separately since they are different lengths.
 * The following extracts the version from the current point.
 */
#define	BGP_GET_VERSION(cp)	(*(cp))

/*
 * For version two we have a one byte version, a 2 byte AS number,
 * a 2 byte hold time and a one byte auth code.  This totals 6 bytes.
 */
#define	BGP_OPENV2_MIN_LEN	6
#define	BGP_GET_OPEN_V2(version, as, holdtime, authcode, cp) \
	do { \
		BGP_GET_BYTE((version), (cp)); \
		BGP_GET_SHORT((as), (cp)); \
		BGP_GET_SHORT((holdtime), (cp)); \
		BGP_GET_BYTE((authcode), (cp)); \
	} while (0)

/*
 * Version three is nearly the same, with the addition of a 4 byte
 * ID field after the hold time.
 */
#define	BGP_OPENV3_MIN_LEN	10
#define	BGP_GET_OPEN_V3(version, as, holdtime, id, authcode, cp) \
	do { \
		BGP_GET_BYTE((version), (cp)); \
		BGP_GET_SHORT((as), (cp)); \
		BGP_GET_SHORT((holdtime), (cp)); \
		BGP_GET_NETLONG((id), (cp)); \
		BGP_GET_BYTE((authcode), (cp)); \
	} while (0)

/*
 * The overall minimum length for all open messages
 */
#define	BGP_OPEN_MIN_LEN	BGP_OPENV2_MIN_LEN


/*
 * The update message is essentially a 2 byte length for the attribute
 * data, followed by the attribute buffer, followed by a list of
 * networks.  The attribute data is dealt with by separate code,
 * so all we really need to do is to return the length and a pointer
 * to the attribute buffer.
 */
#define	BGP_UPDATE_MIN_LEN	2	/* make sure we've enough for length */
#define	BGP_GET_UPDATE(attrlen, attrp, cp) \
	do { \
		register int Xlen; \
		Xlen = (*(cp)++) << 8; \
		Xlen |= *(cp)++; \
		(attrlen) = Xlen; \
		(attrp) = (cp); \
		(cp) += Xlen; \
	} while (0)

/*
 * The size of the total path attribute length in an UPDATE
 */
#define	BGP_ATTR_SIZE_LEN	2

/*
 * The following macro extracts network addresses from the stream.  It
 * is used to decode the end of update messages, and understands that
 * network numbers are stored internally in network byte order.
 */
#define	BGP_GET_ADDR(addr, cp) \
	do { \
		register byte *Xap; \
		Xap = (byte *)&(sock2ip(addr)); \
		*Xap++ = *(cp)++; \
		*Xap++ = *(cp)++; \
		*Xap++ = *(cp)++; \
		*Xap++ = *(cp)++; \
	} while (0)

/*
 * The notification message contains a 2 byte code/subcode followed by
 * optional data.  Return the code and subcode.
 */
#define	BGP_NOTIFY_MIN_LEN	2	/* code/subcode pair */
#define	BGP_GET_NOTIFY(code, subcode, cp) \
	do { \
		BGP_GET_BYTE((code), (cp)); \
		BGP_GET_BYTE((subcode), (cp)); \
	} while (0)


/*
 * That is it for incoming messages.  The next set of macroes are used
 * for forming outgoing messages.
 */
#define	BGP_PUT_BYTE(val, cp) 	(*(cp)++ = (byte)(val))

#define	BGP_PUT_SHORT(val, cp) \
	do { \
		register u_short Xv; \
		Xv = (u_short)(val); \
		*(cp)++ = (byte)(Xv >> 8); \
		*(cp)++ = (byte)Xv; \
	} while (0)

#define	BGP_PUT_LONG(val, cp) \
	do { \
		register u_long Xv; \
		Xv = (u_long)(val); \
		*(cp)++ = (byte)(Xv >> 24); \
		*(cp)++ = (byte)(Xv >> 16); \
		*(cp)++ = (byte)(Xv >>  8); \
		*(cp)++ = (byte)Xv; \
	} while (0)

#define	BGP_PUT_NETLONG(val, cp) \
	do { \
		register u_char *Xvp; \
		u_long Xv = (u_long)(val); \
		Xvp = (u_char *)&Xv; \
		*(cp)++ = *Xvp++; \
		*(cp)++ = *Xvp++; \
		*(cp)++ = *Xvp++; \
		*(cp)++ = *Xvp++; \
	} while (0)


/*
 * During normal message formation the header is inserted after
 * the rest of the message is fully formed.  The following just
 * leaves space for the header.
 */
#define	BGP_SKIP_HEADER(cp)    ((cp) += BGP_HEADER_LEN)

/*
 * The following are used to insert the length and type into the
 * header given a pointer to the beginning of the message.
 */
#define	BGP_PUT_HDRLEN(length, cp) \
	do { \
		register u_short Xlen; \
		Xlen = (u_short)(length); \
		*((cp) + 16) = (byte)(Xlen >> 8); \
		*((cp) + 17) = (byte)Xlen; \
	} while (0)

#define BGP_PUT_HDRTYPE(type, cp)	(*((cp) + 18) = (byte)(type))

/*
 * Output a version 2 open message.
 */
#define	BGP_PUT_OPEN_V2(as, holdtime, authcode, cp) \
	do { \
		BGP_PUT_BYTE(BGP_VERSION_2, (cp)); \
		BGP_PUT_SHORT((as), (cp)); \
		BGP_PUT_SHORT((holdtime), (cp)); \
		BGP_PUT_BYTE((authcode), (cp)); \
	} while (0)

/*
 * Version three is nearly the same, with the addition of a 4 byte
 * ID field after the hold time.
 */
#define	BGP_PUT_OPEN_V3(as, holdtime, id, authcode, cp) \
	do { \
		BGP_PUT_BYTE(BGP_VERSION_3, (cp)); \
		BGP_PUT_SHORT((as), (cp)); \
		BGP_PUT_SHORT((holdtime), (cp)); \
		BGP_PUT_NETLONG((id), (cp)); \
		BGP_PUT_BYTE((authcode), (cp)); \
	} while (0)

/*
 * The attributes in the update message are another case where we
 * must insert the data before inserting the length.  This just
 * skips the length field, it will be filled in later.
 */
#define	BGP_SKIP_ATTRLEN(cp)	((cp) += BGP_UPDATE_MIN_LEN)


/*
 * The following puts a network address into the buffer in the
 * form a BGP update message would like.  We know the address
 * is in network byte order already.
 */
#define	BGP_PUT_ADDR(addr, cp) \
	do { \
		register byte *Xap; \
		Xap = (byte *)&(sock2ip(addr)); \
		*(cp)++ = *Xap++; \
		*(cp)++ = *Xap++; \
		*(cp)++ = *Xap++; \
		*(cp)++ = *Xap++; \
	} while (0)

/*
 * The notification message contains a 2 byte code/subcode followed by
 * optional data.  Insert the code/subcode and optional data, if any.
 * Note that we evaluate macro arguments more than once here.
 */
#define	BGP_PUT_NOTIFY(code, subcode, datalen, data, cp) \
	do { \
		BGP_PUT_BYTE((code), (cp)); \
		BGP_PUT_BYTE((subcode), (cp)); \
		if ((datalen) > 0) { \
			register int Xi = (datalen); \
			register byte *Xdp = (byte *)(data); \
			while (Xi-- > 0) \
				*(cp)++ = *Xdp++; \
		} \
	} while (0)


/*
 * BGP error processing is a little tedious.  The following are
 * the error codes/subcodes we use.
 */
#define	BGP_ERR_HEADER		1	/* message header error */
#define	BGP_ERR_OPEN		2	/* open message error */
#define	BGP_ERR_UPDATE		3	/* update message error */
#define	BGP_ERR_HOLDTIME	4	/* hold timer expired */
#define	BGP_ERR_FSM		5	/* finite state machine error */
#define	BGP_CEASE		6	/* cease message (not an error) */

/*
 * The unspecified subcode is sent when we don't have anything better.
 */
#define	BGP_ERR_UNSPEC		0

/*
 * These subcodes are sent with header errors
 */
#define	BGP_ERRHDR_UNSYNC	1	/* loss of connection synchronization */
#define	BGP_ERRHDR_LENGTH	2	/* bad length */
#define	BGP_ERRHDR_TYPE		3	/* bad message type */

/*
 * These are used with open errors
 */
#define	BGP_ERROPN_VERSION	1	/* a version we don't do */
#define	BGP_ERROPN_AS		2	/* something wrong with the AS */
#define	BGP_ERROPN_BGPID	3	/* we don't like BGP ID (V.3 only) */
#define	BGP_ERROPN_AUTHCODE	4	/* unsupported auth code */
#define	BGP_ERROPN_AUTH		5	/* authentication failure */

/*
 * These are used with update errors.  N.B. the path support code also
 * knows these errors.  The lists should be kept in sync.
 */
#define	BGP_ERRUPD_ATTRLIST	1	/* screwed up attribute list */
#define	BGP_ERRUPD_UNKNOWN	2	/* don't know well known attribute */
#define	BGP_ERRUPD_MISSING	3	/* missing well known attribute */
#define	BGP_ERRUPD_FLAGS	4	/* attribute flags error */
#define	BGP_ERRUPD_LENGTH	5	/* attribute length error */
#define	BGP_ERRUPD_ORIGIN	6	/* origin given is bad */
#define	BGP_ERRUPD_ASLOOP	7	/* routing loop in AS path */
#define	BGP_ERRUPD_NEXTHOP	8	/* next hop screwed up */
#define	BGP_ERRUPD_OPTATTR	9	/* error with optional attribute */
#define	BGP_ERRUPD_BADNET	10	/* bad network */

/*
 * The length of a BGP peer name
 */
#define	BGPPEERNAMELENGTH	40
#define	BGPPEERTASKNAMELENGTH	24
#define	BGPGROUPNAMELENGTH	48
#define	BGPGROUPTASKNAMELENGTH	28


/*
 * This shitty macro determines if a peer sorts less than another
 * peer for SNMP purposes.  Sort by remote address, then remote AS,
 * then local AS.
 */
#define	BGPSORT_ADDR(addr)	ntohl(sock2ip(addr))
#define	BGPSORT_LT(p1, addr, p2) \
    (((addr) < BGPSORT_ADDR((p2)->bgp_addr)) ? TRUE : \
      (((addr) > BGPSORT_ADDR((p2)->bgp_addr)) ? FALSE : \
	(((p1)->bgp_peer_as < (p2)->bgp_peer_as) ? TRUE : \
	  (((p1)->bgp_peer_as > (p2)->bgp_peer_as) ? FALSE : \
	    (((p1)->bgp_local_as < (p2)->bgp_local_as) ? TRUE : FALSE)))))

/*
 * Macroes to determine if a peer group is a type which requires a
 * shared network, or if it is a group which can use this information
 */
#define	BGP_NEEDS_SHAREDIF(bgp) \
    ((bgp)->bgpg_type == BGPG_EXTERNAL || (bgp)->bgpg_type == BGPG_INTERNAL)

#define	BGP_USES_SHAREDIF(bgp) \
    ((bgp)->bgpg_type != BGPG_INTERNAL_IGP)

#define	BGP_OPTIONAL_SHAREDIF(bgp) \
    ((bgp)->bgpg_type == BGPG_TEST)

/*
 * BGP buffer management structure
 */
typedef struct _bgpBuffer {
    byte *bgpb_buffer;		/* Receive buffer for incoming pkts */
    byte *bgpb_bufpos;		/* Start of unconsumed chars in buf */
    byte *bgpb_readptr;		/* Pointer past end of data in buffer */
    byte *bgpb_endbuf;		/* Pointer to end of buffer */
} bgpBuffer;

/*
 * BGP authentication structure (nothing for now)
 */
typedef struct _bgpAuthinfo {
    int bgpa_type;			/* type of authentication */
} bgpAuthinfo;

/*
 *	BGP configuration structure
 *
 * This structure is imbedded in the peer and group structures and
 * contains information learned from parsing the config file.
 */
struct bgp_conf {
    flag_t	bgpc_options;
    flag_t	bgpc_trace_flags;
    sockaddr_un	*bgpc_gateway;
    if_addr_entry	*bgpc_lcladdr;
    time_t	bgpc_holdtime_out;
    metric_t	bgpc_metric_out;
    pref_t	bgpc_preference;
    as_t	bgpc_local_as;
    bgpAuthinfo	bgpc_authinfo;
#define	bgpc_authtype	bgpc_authinfo.bgpa_type
    u_int	bgpc_conf_version;
    size_t	bgpc_recv_bufsize;
    size_t	bgpc_send_bufsize;
    size_t	bgpc_spool_bufsize;
} ;


/*
 *  BGP statistics structure
 *
 * This contains message and octet counters used to keep track of traffic
 */
struct bgp_stats {
    u_long	bgps_in_updates;	/* incoming update messages */
    u_long	bgps_out_updates;	/* outgoing update messages */
    u_long	bgps_in_notupdates;	/* incoming other than updates */
    u_long	bgps_out_notupdates;	/* outgoing other than updates */
    u_long	bgps_in_octets;		/* incoming total octets */
    u_long	bgps_out_octets;	/* outgoing total octets */
};

/*
 *  BGP write spool header and buffer
 *
 * If configured to do so we buffer write data internally when the
 * kernel buffer fills, using select() to tell us when we can write
 * more.  We allocate near-page-size buffers for this,
 * and keep track of data spooled for a peer in the header.
 */
typedef struct _bgp_spool_buffer {
    struct _bgp_spool_buffer *bgp_sp_next;	/* next buffer in chain */
    byte *bgp_sp_startp;			/* pointer to start of data */
    byte *bgp_sp_endp;				/* pointer to end of data */
    byte bgp_sp_data[4];			/* where the data goes */
} bgp_spool_buffer;

#define	BGP_SPBUF_HEADER_LEN(sbp) \
    (((caddr_t)&((sbp)->bgp_sp_data[0])) - ((caddr_t)(sbp)))

#define	BGP_SPBUF_STARTBUF(sbp)	(&((sbp)->bgp_sp_data[0]))

#define	BGP_SPBUF_ENDBUF(sbp, lenbuf)	(((byte *)(sbp)) + ((size_t)(lenbuf)))

typedef struct _bgp_spool_header {
    struct _bgpPeer *bgp_sph_next;		/* next peer in chain */
    struct _bgp_spool_buffer *bgp_sph_head;	/* head of buffer chain */
    struct _bgp_spool_buffer *bgp_sph_tail;	/* tail of buffer chain */
    size_t bgp_sph_count;			/* total bytes buffered */
} bgp_spool_header;


/*
 *  BGP peer structure
 *
 * A note on input.  The maximum BGP packet size is 4096.  To receive a
 * packet this large each peer must have an input buffer of that size
 * allocated to it (sigh).  We try to keep system call overhead down
 * by always trying to fill the 4096 byte buffer on each read.  We then
 * process each complete message in the buffer in batch fashion.  Any
 * incomplete message fragment is then copied to the start of the
 * buffer (message fragments should occur infrequently) and attempt
 * to fill the remainder of the buffer again.  We entirely empty the
 * socket before closing the routing table and returning, as this
 * provides good processing efficiency when routing traffic is heavy.
 */
typedef struct _bgpPeer {
    struct _bgpPeer *bgp_next;		/* Pointer to next bgpPeer struct */
    struct _bgpPeer *bgp_sort_next;	/* Pointer to next sorted bgpPeer */
    struct _bgpPeerGroup *bgp_group;	/* Back pointer to group header */

    flag_t bgp_flags;			/* Protocol Flags */

    struct bgp_conf	bgp_conf;	/* Configuration information */
#define	bgp_options		bgp_conf.bgpc_options
#define	bgp_conf_version	bgp_conf.bgpc_conf_version
#define	bgp_authtype		bgp_conf.bgpc_authtype
#define	bgp_authinfo		bgp_conf.bgpc_authinfo
#define	bgp_trace_flags	      	bgp_conf.bgpc_trace_flags
#define	bgp_gateway		bgp_conf.bgpc_gateway
#define	bgp_lcladdr		bgp_conf.bgpc_lcladdr
#define	bgp_holdtime_out	bgp_conf.bgpc_holdtime_out
#define	bgp_metric_out		bgp_conf.bgpc_metric_out
#define	bgp_conf_local_as	bgp_conf.bgpc_local_as
#define	bgp_preference		bgp_conf.bgpc_preference
#define	bgp_recv_bufsize	bgp_conf.bgpc_recv_bufsize
#define	bgp_send_bufsize	bgp_conf.bgpc_send_bufsize
#define	bgp_spool_bufsize	bgp_conf.bgpc_spool_bufsize

#define	bgp_type		bgp_group->bgpg_type

    u_int bgp_version;			/* Actual version in use */
    u_int bgp_hisversion;		/* Version he likes, if not ours */

    char bgp_name[BGPPEERNAMELENGTH];	/* Name of this peer */
    char bgp_task_name[BGPPEERTASKNAMELENGTH];	/* Name for peer's task */

    gw_entry bgp_gw;			/* GW block for this peer */
#define	bgp_proto	bgp_gw.gw_proto
#define	bgp_import	bgp_gw.gw_import
#define	bgp_export	bgp_gw.gw_export
#define	bgp_local_as	bgp_gw.gw_local_as
#define	bgp_peer_as	bgp_gw.gw_peer_as
#define	bgp_last_rcvd	bgp_gw.gw_time
#define	bgp_task	bgp_gw.gw_task
#define	bgp_addr	bgp_gw.gw_addr

#define	bgp_rtbit	bgp_gw.gw_task->task_rtbit

    u_int32 bgp_id;			/* BGP ID this peer told us */
    u_int32 bgp_out_id;			/* BGP ID we sent to peer */

    sockaddr_un *bgp_myaddr;		/* local address we talk to him via */
    if_addr *bgp_ifap;			/* the local interface for the peer */

    /* Information related to holdtime/keepalive timing */
    time_t bgp_hisholdtime;		/* hold time he told us */
    time_t bgp_last_sent;		/* last time we sent anything */
    time_t bgp_last_checked;		/* last time we checked status */
    time_t bgp_traffic_interval;	/* current interval we're using */

    /* Information about current and previous states */
    byte bgp_state;			/* Protocol State */
    byte bgp_laststate;			/* previous protocol state */
    byte bgp_lastevent;			/* last event */

    byte bgp_lasterror[2];		/* last error with this peer */
#define	bgp_last_code		bgp_lasterror[0]
#define	bgp_last_subcode	bgp_lasterror[1]

    /* Event counters and times */
    u_int bgp_connect_failed;		/* number of times connect has failed */
    int bgp_connect_slot;		/* slot number for connection */

    /* Traffic counters */
    struct bgp_stats		bgp_stats;
#define	bgp_in_updates		bgp_stats.bgps_in_updates
#define	bgp_out_updates		bgp_stats.bgps_out_updates
#define	bgp_in_notupdates	bgp_stats.bgps_in_notupdates
#define	bgp_out_notupdates	bgp_stats.bgps_out_notupdates
#define	bgp_in_octets		bgp_stats.bgps_in_octets
#define	bgp_out_octets		bgp_stats.bgps_out_octets

    /* Received packet buffer */
    bgpBuffer bgp_inbuf;
#define	bgp_buffer	bgp_inbuf.bgpb_buffer
#define	bgp_bufpos	bgp_inbuf.bgpb_bufpos
#define	bgp_readptr	bgp_inbuf.bgpb_readptr
#define	bgp_endbuf	bgp_inbuf.bgpb_endbuf

    /* Spooled write data */
    bgp_spool_header *bgp_spool;	/* only non-NULL when data spooled */
} bgpPeer;


/*
 * BGP peer group structure.  BGP peers are sorted into groups, where
 * the peers in a single group generally receive a single set of
 * advertisements (i.e. implement identical policy).  The group structures
 * are chained together, with each heading a list of peers.
 */
typedef struct _bgpPeerGroup {
    struct _bgpPeerGroup *bgpg_next;	/* chain pointer for groups */
    struct _bgpPeer *bgpg_peers;	/* pointer to head of list of peers */
    char	bgpg_name[BGPGROUPNAMELENGTH];	/* group name */
    char	bgpg_task_name[BGPGROUPTASKNAMELENGTH];
    u_int	bgpg_type;		/* internal|external|test */
    as_t	bgpg_peer_as;		/* AS this group operates in */
    flag_t	bgpg_flags;		/* Group peers */
    adv_entry	*bgpg_allow;		/* Popup - peers list */
    task	*bgpg_task;		/* Task for rtbit, internal only */
    u_int	bgpg_igp_rtbit;		/* IGP's rtbit */
    proto_t	bgpg_igp_proto;		/* IGP's protocol */
    u_int	bgpg_n_peers;		/* number of peers in group */
    u_int	bgpg_n_established;	/* number of established peers */
    struct bgp_conf bgpg_conf;		/* Configured information */
#define	bgpg_options		bgpg_conf.bgpc_options
#define	bgpg_conf_version	bgpg_conf.bgpc_conf_version
#define	bgpg_authtype		bgpg_conf.bgpc_authtype
#define	bgpg_authinfo		bgpg_conf.bgpc_authinfo
#define	bgpg_trace_flags	bgpg_conf.bgpc_trace_flags
#define	bgpg_gateway		bgpg_conf.bgpc_gateway
#define	bgpg_lcladdr		bgpg_conf.bgpc_lcladdr
#define	bgpg_holdtime_out	bgpg_conf.bgpc_holdtime_out
#define	bgpg_metric_out		bgpg_conf.bgpc_metric_out
#define	bgpg_local_as		bgpg_conf.bgpc_local_as
#define	bgpg_preference		bgpg_conf.bgpc_preference
#define	bgpg_recv_bufsize	bgpg_conf.bgpc_recv_bufsize
#define	bgpg_send_bufsize	bgpg_conf.bgpc_send_bufsize
#define	bgpg_spool_bufsize	bgpg_conf.bgpc_spool_bufsize
} bgpPeerGroup;

/*
 * Types of peer groups
 */
#define	BGPG_EXTERNAL		0	/* external peer group */
#define	BGPG_INTERNAL		1	/* internal peer group */
#define	BGPG_INTERNAL_IGP	2	/* internal peer group running IGP */
#define	BGPG_TEST		3	/* anonymous watcher */

/*
 *	Group flags
 */
#define	BGPGF_DELETE	0x01		/* Group is being deleted */
#define	BGPGF_IDLED	0x02		/* Group is temporarily idled */

/*
 * When we are connected to by a remote server we create a proto-peer
 * structure and new task.  This bit of goo holds us until we receive
 * an open packet and can figure out what to do.
 */
typedef struct _bgpProtoPeer {
    struct _bgpProtoPeer *bgpp_next;	/* pointer to next in chain */
    task *bgpp_task;			/* task we're using for this */
#define	bgpp_addr	bgpp_task->task_addr
    char bgpp_name[BGPPEERNAMELENGTH];	/* name for this peer */
    sockaddr_un *bgpp_myaddr;		/* local address peer connected to */
    time_t bgpp_connect_time;		/* time connect was done */
    bgpBuffer bgpp_inbuf;		/* received packet buffer */
#define	bgpp_buffer	bgpp_inbuf.bgpb_buffer
#define	bgpp_bufpos	bgpp_inbuf.bgpb_bufpos
#define	bgpp_readptr	bgpp_inbuf.bgpb_readptr
#define	bgpp_endbuf	bgpp_inbuf.bgpb_endbuf
} bgpProtoPeer;


/*
 * Buffer space computations
 */
#define	BGPBUF_SPACE(bnp)	((bnp)->bgp_endbuf - (bnp)->bgp_readptr)
#define	BGPBUF_LEFT(bnp, cp)	((bnp)->bgp_readptr - (byte *)(cp))
#define	BGPBUF_DATA(bnp)	BGPBUF_LEFT((bnp), (bnp)->bgp_bufpos)
#define	BGPBUF_FULL(bnp)	((bnp)->bgp_readptr == (bnp)->bgp_endbuf)


/*
 * BGP send list structure.  This is used for sorting routes which
 * we intend to advertise by AS path.  This is actually used for
 * two purposes.  There is a list (chained by bgpl_asnext) which
 * holds a pointer to an AS path (bgpl_aspath) and a pointer to
 * a list of routes with that path (bgpl_next).  The latter is
 * chained together (bgpl_next) and has a pointer to the route
 * (bgpl_route) and a corresponding metric (bgpl_metric).
 *
 * Also added in some shit so this can be used to build interface
 * lists, just to add confusion.
 */
typedef struct _bgp_send_list {
    struct _bgp_send_list *bgpl_next;	/* next route with same AS path */
    union {
	struct _rt_entry *Xbgpl_route;		/* route with this AS path */
	struct _bgp_send_list *Xbgpl_asnext;	/* pointer to next list head */
	struct _if_addr *Xbgpl_ifap;		/* pointer to interface */
    } bgpl_X1;
#define	bgpl_route	bgpl_X1.Xbgpl_route
#define	bgpl_asnext	bgpl_X1.Xbgpl_asnext
#define	bgpl_ifap	bgpl_X1.Xbgpl_ifap
    union {
	metric_t Xbgpl_metric;		/* metric for this route */
	struct _as_path *Xbgpl_aspath;	/* AS path for this list of routes */
    } bgpl_X2;
#define	bgpl_metric	bgpl_X2.Xbgpl_metric
#define	bgpl_aspath	bgpl_X2.Xbgpl_aspath
} bgp_send_list;



/*
 * Macro to sqeeze data to the beginning of the buffer.  This recognizes
 * most special cases, and so may be used for routine cleanup.
 */
#define	BGPBUF_COMPACT(bnp, cp)	\
	do { \
		if ((cp) != (bnp)->bgp_buffer) { \
			if ((cp) == (bnp)->bgp_readptr) { \
				(bnp)->bgp_readptr = (bnp)->bgp_buffer; \
			} else { \
				register int Xlen = BGPBUF_LEFT((bnp), (cp)); \
				bcopy((cp), (bnp)->bgp_buffer, (size_t)Xlen); \
				(bnp)->bgp_readptr = (bnp)->bgp_buffer + Xlen; \
			} \
		} \
	} while (0)

/*
 * Quicker authentication checking for the default type
 */
#define	BGP_CHECK_AUTH(bap, tp, pkt, pktlen) \
    ((((bap) != NULL && (bap)->bgpa_type != BGP_AUTH_NONE) \
      || (bcmp((caddr_t)bgp_default_auth_info, (caddr_t)(pkt), \
	BGP_HEADER_MARKER_LEN) != 0)) \
    ? bgp_check_auth((bap), (tp), (pkt), (pktlen)) : (1))

#define	BGP_ADD_AUTH(bap, pkt, pktlen) \
    if ((bap) == NULL || (bap)->bgpa_type == BGP_AUTH_NONE) {\
        bcopy((caddr_t)bgp_default_auth_info, (caddr_t)(pkt), BGP_HEADER_MARKER_LEN); \
    } else { \
        bgp_add_auth((bap), (pkt), (pktlen)); \
    }

/* Timers */
#define	BGPTIMER_TRAFFIC	0	/* Connection traffic timer */
#define	BGPTIMER_CONNECT	1	/* Fire when time to try connect */
#define	BGP_PEER_TIMER_MAX	2	/* number of timers a peer needs */

#define	BGPTIMER_LISTEN		0	/* Timer for listen task */
#define	BGP_LISTEN_TIMER_MAX	1	/* number of timers listen task wants */

/* Protocol States */
#define	BGPSTATE_IDLE		1	/* idle state - ignore everything */
#define	BGPSTATE_CONNECT	2	/* connect state - trying to connect */
#define BGPSTATE_ACTIVE		3	/* waiting for a connection */
#define	BGPSTATE_OPENSENT	4	/* open packet has been sent */
#define	BGPSTATE_OPENCONFIRM	5	/* waiting for a keepalive or notify */
#define	BGPSTATE_ESTABLISHED	6	/* connection has been established */

/* Events */
#define	BGPEVENT_START		1	/* Start */
#define	BGPEVENT_STOP		2	/* Stop */
#define	BGPEVENT_OPEN		3	/* Transport connection open */
#define	BGPEVENT_CLOSED		4	/* Transport connection closed */
#define	BGPEVENT_OPENFAIL	5	/* Transport connection open failed */
#define	BGPEVENT_ERROR		6	/* Transport error */
#define	BGPEVENT_CONNRETRY	7	/* Connection retry timer expired */
#define	BGPEVENT_HOLDTIME	8	/* Holdtime expired */
#define	BGPEVENT_KEEPALIVE	9	/* KeepAlive timer */
#define	BGPEVENT_RECVOPEN	10	/* Receive Open message */
#define	BGPEVENT_RECVKEEPALIVE	11	/* Receive KeepAlive message */
#define	BGPEVENT_RECVUPDATE	12	/* Receive Update message */
#define	BGPEVENT_RECVNOTIFY	13	/* Receive Notification message */

/* Flags */
#define	BGPF_DELETE		0x01	/* Delete this peer */
#define	BGPF_UNCONFIGURED	0x02	/* This is an unconfigured peer */
#define	BGPF_TRAFFIC		0x04	/* Traffic timer exists */
#define	BGPF_CONNECT		0x08	/* Connect timer allocated */
#define	BGPF_TRY_CONNECT	0x10	/* Time to try another connect */
#define	BGPF_WRITEFAILED	0x20	/* Attempt to write this peer failed */
#define	BGPF_IDLED		0x40	/* This peer is permanently idled */
#define	BGPF_GENDEFAULT		0x80	/* Requested default generation */

/* Options */
#define BGPO_METRIC_OUT		0x01	/* Use an outbound metric */
#define BGPO_LOCAL_AS		0x02	/* Use this outbound AS number */
#define BGPO_NOGENDEFAULT	0x04	/* Don't consider this peer for default generation */
#define	BGPO_GATEWAY		0x08	/* Address of local gateway to Source Network */
#define	BGPO_PREFERENCE		0x10	/* Preference for this AS */
#define	BGPO_LCLADDR		0x20	/* Our local address was specified */
#define	BGPO_HOLDTIME		0x40	/* Holdtime was specified */
#define	BGPO_PASSIVE		0x80	/* Don't actively try to connect */
#define	BGPO_VERSION		0x0100	/* Version number specified */
#define	BGPO_DEFAULTIN		0x0200	/* Accept default from our buddy */
#define	BGPO_DEFAULTOUT		0x0400	/* Accept default from our buddy */
#define	BGPO_KEEPALL		0x0800	/* Keep all routes from peer */

/* Group options */
#define BGPGO_METRIC_OUT	0x01	/* Use an outbound metric */
#define BGPGO_ASOUT		0x02	/* Use this outbound AS number */
#define BGPGO_NOGENDEFAULT	0x04	/* don't use group peers for default generation */
#define	BGPGO_PREFERENCE	0x10	/* Preference for this AS */
#define	BGPGO_VERSION		0x0100	/* Version number specified */
#define	BGPGO_DEFAULTIN		0x0200	/* Accept default from our buddy */
#define	BGPGO_DEFAULTOUT	0x0400	/* Accept default from our buddy */
#define	BGPGO_KEEPALL		0x0800	/* Keep all routes from peer */


/*
 * I don't like trace_state much because it doesn't do bounds checking.
 * Try this instead.  Should use token pasting for the macro.
 */
typedef struct _bgp_code_string {
    u_int bgp_n_codes;
    const bits *bgp_code_bits;
} bgp_code_string;

#define	BGP_MAKE_CODES(codevar, bitvar) \
    const bgp_code_string codevar = { (sizeof(bitvar)/sizeof(bits))-1, bitvar }

#define	bgp_code(codes, code) \
    (((code) >= (codes).bgp_n_codes) ? "invalid" : \
      trace_state((codes).bgp_code_bits, (code)))

/*
 * tracing/debugging variables, in bgp_init.c
 */
extern const bits bgp_flag_bits[];
extern const bits bgp_group_flag_bits[];
extern const bits bgp_option_bits[];
extern const bits bgp_state_bits[];
extern const bits bgp_event_bits[];
extern const bits bgp_message_type_bits[];
extern const bits bgp_error_bits[];
extern const bits bgp_header_error_bits[];
extern const bits bgp_open_error_bits[];
extern const bits bgp_update_error_bits[];
extern const bits bgp_group_bits[];

extern const bgp_code_string bgp_state_codes;
extern const bgp_code_string bgp_event_codes;
extern const bgp_code_string bgp_message_type_codes;
extern const bgp_code_string bgp_error_codes;
extern const bgp_code_string bgp_header_error_codes;
extern const bgp_code_string bgp_open_error_codes;
extern const bgp_code_string bgp_update_error_codes;
extern const bgp_code_string bgp_group_codes;

/*
 * Variables in bgp_init.c
 */
extern int doing_bgp;			/* Are we running BGP protocols? */
extern flag_t bgp_default_trace_flags;	/* Trace flags from parser */
extern pref_t bgp_default_preference;	/* Preference for BGP routes */
extern metric_t bgp_default_metric;	/* Default BGP metric to use */

extern bgpPeerGroup *bgp_groups;	/* group list */
extern int bgp_n_groups;		/* number of groups in list */
extern int bgp_n_peers;			/* total number of peers */
extern int bgp_n_unconfigured;		/* number of unconfigured peers */
extern int bgp_n_established;		/* number of established peers */

extern bgpPeer *bgp_sort_list;		/* sorted list of peers */
extern adv_entry *bgp_import_list;	/* List of BGP advise entries */
extern adv_entry *bgp_import_aspath_list;	/* List of BGP import aspath policy */
extern adv_entry *bgp_export_list;	/* List of BGP export entries */

extern block_t bgp_buf_blk_index;	/* Block index for input buffers */
extern task *bgp_listen_task;		/* The task we listen on */

/*
 * Variables from bgp.c
 */
extern byte bgp_default_auth_info[];	/* 16 bytes of 1's, should be const */


#define	BGP_PEER_LIST(bgp, bnp)	for (bnp = (bgp)->bgpg_peers; bnp; bnp = bnp->bgp_next)
#define	BGP_PEER_LIST_END(bgp, bnp)

#define	BGP_GROUP_LIST(bgp)	for (bgp = bgp_groups; bgp; bgp = bgp->bgpg_next)
#define	BGP_GROUP_LIST_END(bgp)

#define	BGP_SORT_LIST(bnp)	for (bnp = bgp_sort_list; bnp; bnp = bnp->bgp_sort_next)
#define	BGP_SORT_LIST_END(bnp)

/*
 * Routines in bgp_rt.c
 */
PROTOTYPE(bgp_rt_send_init, extern void, (bgpPeer *));
PROTOTYPE(bgp_flash, extern void, (task *, rt_list *));
PROTOTYPE(bgp_aux_flash, extern void, (task *, rt_list *));
PROTOTYPE(bgp_rt_terminate, extern void, (bgpPeer *));
PROTOTYPE(bgp_recv_update, extern void, (task *));
PROTOTYPE(bgp_rt_reinit, extern void, (bgpPeer *));
PROTOTYPE(bgp_newpolicy, extern void, (task *, rt_list *));
PROTOTYPE(bgp_aux_newpolicy, extern void, (task *, rt_list *));

/*
 * Routines in bgp_init.c
 */
PROTOTYPE(bgp_pp_delete, extern void, (bgpProtoPeer *));
PROTOTYPE(bgp_recv_change, extern void,
	 (bgpPeer *, _PROTOTYPE(recv_rtn, void, (task *)), const char *));
PROTOTYPE(bgp_find_group, extern bgpPeerGroup *,
	 (sockaddr_un *, sockaddr_un *, as_t, as_t, int, byte *, int));
PROTOTYPE(bgp_find_peer, extern bgpPeer *,
	 (bgpPeerGroup *, sockaddr_un *, sockaddr_un *));
PROTOTYPE(bgp_find_group_by_addr, extern bgpPeerGroup *,
	 (sockaddr_un *, sockaddr_un *));
PROTOTYPE(bgp_new_peer, extern bgpPeer *,
	 (bgpPeerGroup *, bgpProtoPeer *, int));
PROTOTYPE(bgp_use_protopeer, extern void, (bgpPeer *, bgpProtoPeer *, int));
PROTOTYPE(bgp_peer_close, extern void, (bgpPeer *, int));
PROTOTYPE(bgp_peer_established, extern void, (bgpPeer *));
PROTOTYPE(bgp_conf_check,
	  extern int,
	  (char *));
PROTOTYPE(bgp_conf_group_alloc, extern bgpPeerGroup *, (void));
PROTOTYPE(bgp_conf_group_add, extern bgpPeerGroup *, (bgpPeerGroup *, char *));
PROTOTYPE(bgp_conf_group_check, extern int, (bgpPeerGroup *, char *));
PROTOTYPE(bgp_conf_peer_alloc, extern bgpPeer *, (bgpPeerGroup *));
PROTOTYPE(bgp_conf_peer_add, extern bgpPeer *,
	 (bgpPeerGroup *, bgpPeer *, char *));
PROTOTYPE(bgp_var_init, extern void, (void));
PROTOTYPE(bgp_init, extern void, (void));
PROTOTYPE(bgp_spool_write, int, (bgpPeer *, byte *, int));

#define	bgp_find_peer_by_addr(bgp, addr, lcladdr) \
	bgp_find_peer((bgp), (addr), (lcladdr))

/*
 * Routines in bgp.c
 */
PROTOTYPE(bgp_event, extern void, (bgpPeer *, int, int));
PROTOTYPE(bgp_add_auth, extern void, (bgpAuthinfo *, byte *, int));
PROTOTYPE(bgp_check_auth, extern int, (bgpAuthinfo *, task *, byte *, int));
PROTOTYPE(bgp_group_auth, extern int, (bgpPeerGroup *, int, byte *, int));
PROTOTYPE(bgp_open_auth, extern int, (char *, bgpAuthinfo *, int, byte *, int));
PROTOTYPE(bgp_trace, extern void,
	 (bgpPeer *, bgpProtoPeer *, const char *, int, byte *));
PROTOTYPE(bgp_log_notify, extern void, (char *, byte *, int, int));
PROTOTYPE(bgp_send, extern int, (bgpPeer *, byte *, int));
PROTOTYPE(bgp_send_open, extern int, (bgpPeer *, int));
PROTOTYPE(bgp_send_keepalive, extern int, (bgpPeer *));
PROTOTYPE(bgp_send_notify, extern void, (bgpPeer *, int, int, byte *, int));
PROTOTYPE(bgp_send_notify_none, extern void, (bgpPeer *, int, int));
PROTOTYPE(bgp_send_notify_byte, extern void, (bgpPeer *, int, int, int));
PROTOTYPE(bgp_send_notify_word, extern void, (bgpPeer *, int, int, int));
PROTOTYPE(bgp_send_notify_aspath, extern void,
	 (bgpPeer *, int, int, as_path *));
PROTOTYPE(bgp_path_attr_error, extern void, (bgpPeer *, int, byte *, const char *));
PROTOTYPE(bgp_pp_notify_none, extern void,
	 (bgpProtoPeer *, bgpPeerGroup *, bgpPeer *, int, int));
PROTOTYPE(bgp_recv, extern int, (task *, bgpBuffer *, int, char *));
PROTOTYPE(bgp_recv_open, extern void, (task *));
PROTOTYPE(bgp_pp_recv, extern void, (task *));

/*
 * Stuff in bgp_mib.c, for SNMP
 */
#ifdef  PROTO_SNMP
PROTOTYPE(bgp_init_mib, extern void, (int));
PROTOTYPE(bgp_trap_established, extern void, (bgpPeer *));
PROTOTYPE(bgp_trap_backward, extern void, (bgpPeer *));
#endif  /* PROTO_SNMP */


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
