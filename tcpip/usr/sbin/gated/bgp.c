static char sccsid[] = "@(#)34	1.5  src/tcpip/usr/sbin/gated/bgp.c, tcprouting, tcpip411, GOLD410 12/6/93 17:32:01";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		__PF1
 *		__PF2
 *		__PF6
 *		__PF7
 *		bgp_add_auth
 *		bgp_check_auth
 *		bgp_event
 *		bgp_get_open
 *		bgp_group_auth
 *		bgp_log_notify
 *		bgp_open_auth
 *		bgp_path_attr_error
 *		bgp_pp_notify_none1031
 *		bgp_pp_notify_word1070
 *		bgp_pp_recv
 *		bgp_read_message1209
 *		bgp_recv
 *		bgp_recv_open
 *		bgp_send
 *		bgp_send_keepalive
 *		bgp_send_notify
 *		bgp_send_notify_aspath
 *		bgp_send_notify_byte
 *		bgp_send_notify_none
 *		bgp_send_notify_word
 *		bgp_send_open
 *		bgp_trace
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
 * bgp.c,v 1.19 1993/03/22 02:38:44 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#include "include.h"
#include "inet.h"
#include "bgp.h"


/*
 * bgp_event - record and optionally log events and resulting state changes
 */
void
bgp_event(bnp, event, state)
bgpPeer *bnp;
int event;
int state;
{
    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	      "bgp_event: peer %s old state %s event %s new state %s",
	      bnp->bgp_name,
	      trace_state(bgp_state_bits, bnp->bgp_state),
	      trace_state(bgp_event_bits, event),
	      trace_state(bgp_state_bits, state));
    }
    bnp->bgp_laststate = bnp->bgp_state;
    bnp->bgp_state = state;
    bnp->bgp_lastevent = event;
#ifdef	PROTO_SNMP
    if (bnp->bgp_state == BGPSTATE_ESTABLISHED
      && bnp->bgp_laststate != BGPSTATE_ESTABLISHED) {
	bgp_trap_established(bnp);
    } else if (bnp->bgp_state < bnp->bgp_laststate) {
	bgp_trap_backward(bnp);
    }
#endif	/* PROTO_SNMP */
}


/*
 *
 *	Authentication support routines.  Mostly placeholders
 *
 */

/*
 * 16 byte block of one bits.  Used for the default authentication
 */
byte bgp_default_auth_info[BGP_HEADER_MARKER_LEN] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/*
 * bgp_set_open_auth - append bgp open authentication type and data
 *   to the end of an open message.  Return pointer past end of message.
 */
static byte *
bgp_set_open_auth __PF2(bap, bgpAuthinfo *,
			cp, byte *)
{
	/*
	 * Only type 0 defined so far.  Don't need to add anything.
	 */
	return cp;
}

/*
 * bgp_add_auth - set authentication info at start of message
 */
void
bgp_add_auth(bap, pkt, pktlen)
bgpAuthinfo *bap;
byte *pkt;
int pktlen;
{
    /*
     * The only authentication info we support now is none.  Add
     * a block of 1's to the start of the message.
     */
    bcopy((caddr_t)bgp_default_auth_info, (caddr_t)pkt, BGP_HEADER_MARKER_LEN);
}


/*
 * bgp_check_auth - check authentication at start of message.
 */
int
bgp_check_auth(bap, tp, pkt, pktlen)
bgpAuthinfo *bap;
task *tp;
byte *pkt;
int pktlen;
{
    int res;

    /*
     * At this point the only the default authentication is supported
     */
    res = bcmp((caddr_t)bgp_default_auth_info, (caddr_t)pkt,
      BGP_HEADER_MARKER_LEN);
    if (res != 0) {
	trace(TR_EXT, LOG_ERR,
	  "bgp_check_auth: synchronization failure with BGP task %s",
	  tp->task_name);
	return (0);
    }
    return (1);
}


/*
 * bgp_known_auth - return true if this is an authentication code we
 *		    know, false otherwise
 */
static int
bgp_known_auth __PF1(authcode, int)
{
    if (authcode == BGP_AUTH_NONE)
	return (1);
    return (0);
}


/*
 * bgp_open_auth - check authentication of the open message
 */
int
bgp_open_auth(name, bap, authcode, authdata, authdatalen)
char *name;
bgpAuthinfo *bap;
int authcode;
byte *authdata;
int authdatalen;
{
    /*
     * Detect type mismatch
     */
    if (bap->bgpa_type != authcode) {
	if (name != NULL) {
	    trace(TR_EXT, LOG_WARNING,
	      "bgp_open_auth: peer %s got auth type %d expected %d",
	      name, authcode, bap->bgpa_type);
	}
	return (0);
    }

    /*
     * Only check default authentication.
     */
    if (authdatalen != 0) {
	if (name != NULL)
	    trace(TR_EXT, LOG_WARNING,
	      "bgp_open_auth: non-zero authentication length from peer %s",
	      name);
	return (0);
    }
    return (1);
}


/*
 * bgp_group_auth - check to see if the authentication type and data match
 *		    this group.
 */
int
bgp_group_auth(bgp, authcode, authdata, authdatalen)
bgpPeerGroup *bgp;
int authcode;
byte *authdata;
int authdatalen;
{
    /*
     * This is essentially a nop since we don't do authentication.
     */
    if (authcode != bgp->bgpg_authtype || authdatalen != 0) {
	return (0);
    }
    return (1);
}


/*
 *
 *	Trace routines of various sorts
 *
 */

/*
 * bgp_trace - print the contents of a message to the trace file
 */
void
bgp_trace(bnp, bpp, comment, send_flag, message)
bgpPeer *bnp;
bgpProtoPeer *bpp;
const char *comment;
int send_flag;
byte *message;
{
    register byte *cp;
    register byte *endmess;
    register int i;
    int length;
    int type;
    int invalidtype;
    sockaddr_un addr;
    byte *marker;
    sockaddr_un *sendaddr, *recvaddr;

    if (bnp != NULL) {
	if (send_flag) {
	    sendaddr = bnp->bgp_myaddr;
	    recvaddr = bnp->bgp_addr;
	} else {
	    sendaddr = bnp->bgp_addr;
	    recvaddr = bnp->bgp_myaddr;
	}
    } else {
	if (send_flag) {
	    sendaddr = bpp->bgpp_myaddr;
	    recvaddr = bpp->bgpp_addr;
	} else {
	    sendaddr = bpp->bgpp_addr;
	    recvaddr = bpp->bgpp_myaddr;
	}
    }

    /*
     * We assume the length is valid looking, or we wouldn't have
     * gotten this far.  We also assume there is length worth of
     * data in there.
     */
    cp = message;
    BGP_GET_HEADER(marker, length, type, cp);

    trace(TR_BGP, 0, NULL);
    trace(TR_BGP, 0, "%s%#A -> %#A",
	comment,
	sendaddr,
	recvaddr);
    if (bcmp((caddr_t)marker, (caddr_t)bgp_default_auth_info,
      BGP_HEADER_MARKER_LEN) != 0) {
	tracef("%smarker ", comment);
	for (i = 0; i < BGP_HEADER_MARKER_LEN; i++) {
	    tracef("%02x", (u_int)marker[i]);
	}
	trace(TR_BGP, 0, NULL);
    }
    invalidtype = (type == 0 || type > BGP_KEEPALIVE);
    trace(TR_BGP, 0, "%smessage type %d (%s) length %d",
	comment,
	type,
	bgp_code(bgp_message_type_codes, type),
	length);
    endmess = message + length;

    TRACE_UPDATE(TR_BGP) {
	switch (type) {
	case BGP_KEEPALIVE:
	    /*
	     * Keepalives are just a header.  Nothing to print.
	     */
	    break;

	case BGP_NOTIFY:
	    /*
	     * Print enough about the notification that we might be able to
	     * figure out what it is about.
	     */
	    {
		int code;
		int subcode;

		if ((endmess - cp) < BGP_NOTIFY_MIN_LEN) {
		    trace(TR_BGP, 0,
			  "%sLength (%d) less than minimum for Notification (%d)",
			  comment,
			  length,
			  BGP_HEADER_LEN + BGP_NOTIFY_MIN_LEN);
		    break;
		}

		BGP_GET_NOTIFY(code, subcode, cp);
		trace(TR_BGP, 0, "%sNotification code %d (%s) subcode %d (%s)",
		      comment,
		      code,
		      bgp_code(bgp_error_codes, code),
		      subcode,
		      ((code == BGP_ERR_HEADER) ?
		       bgp_code(bgp_header_error_codes, subcode) :
		       ((code == BGP_ERR_OPEN) ?
			bgp_code(bgp_open_error_codes, subcode) :
			((code == BGP_ERR_UPDATE) ?
			 bgp_code(bgp_update_error_codes, subcode) :
			 "unused"))));
		if (endmess > cp) {
		    i = endmess - cp;
		    tracef("%sData (%d bytes):", comment, i);
		    if (i > 16)
			i = 16;
		    while (i-- > 0)
			tracef(" %02x", (int)*cp++);
		    trace(TR_BGP, 0, NULL);
		}
	    }
	    break;

	case BGP_OPEN:
	    /*
	     * An Open message.  We have to deal with both version 2 and version
	     * three Open messages here.
	     */
	    {
		int version;
		int as;
		int holdtime;
		u_long id;
		int authcode;

		version = BGP_GET_VERSION(cp);
		i = endmess - cp;
		if (version == BGP_VERSION_2)
		    i -= BGP_OPENV2_MIN_LEN;
		else if (version == BGP_VERSION_3)
		    i -= BGP_OPENV3_MIN_LEN;

		if (i < 0) {
		    trace(TR_BGP, 0,
			  "%sVersion %d: length (%d) less than minimum for open (%d)",
			  comment,
			  version,
			  length,
			  (version == BGP_VERSION_2)
			  ? BGP_OPENV2_MIN_LEN
			  : BGP_OPENV3_MIN_LEN);
		    break;
		}

		if (version == BGP_VERSION_2) {
		    id = 0;
		    BGP_GET_OPEN_V2(version, as, holdtime, authcode, cp);
		} else if (version == BGP_VERSION_3) {
		    BGP_GET_OPEN_V3(version, as, holdtime, id, authcode, cp);
		} else {
		    trace(TR_BGP, 0, "%sversion %d unsupported",
			  comment,
			  version);
		    break;
		}

		sockclear_in(&addr);
		sock2ip(&addr) = id;
		trace(TR_BGP, 0,
		      "%sversion %d as %u holdtime %d id %A authcode %d",
		      comment,
		      version,
		      as,
		      holdtime,
		      &addr,
		      authcode);

		if (i > 0) {
		    tracef("%sAuth data (%d bytes):", comment, i);
		    if (i > 16)
			i = 16;
		    while (i-- > 0)
			tracef(" %02x", (int)*cp++);
		    trace(TR_BGP, 0, NULL);
		}
	    }
	    break;

	case BGP_UPDATE:
	    /*
	     * An update message.  We won't print much about this unless
	     * update tracing is enabled.
	     */
	    {
		int attrlen;
		byte *attrp;

		i = endmess - cp;
		BGP_GET_UPDATE(attrlen, attrp, cp);
		if (attrlen > (i-2)) {
		    trace(TR_BGP, 0,
			  "%sAttribute length %d greater than data in message (%d)",
			  comment,
			  attrlen,
			  i-2);
		    break;
		}

		aspath_trace(TR_BGP,
			     comment,
#ifdef	PATH_VERSION_4
			     PATH_VERSION_2OR3,
#endif	/* PATH_VERSION_4 */
			     attrp,
			     attrlen);

		i = endmess - cp;
		if (i & 0x3) {
		    trace(TR_BGP, 0,
			  "%sAttribute length %d net data %d not multiple of 4",
			  comment,
			  attrlen,
			  i);
		    break;
		}
		if (i) {
		    register int done = 0;

		    sockclear_in(&addr);

		    while (i > 0) {
			if (done >= 4) {
			    trace(TR_BGP, 0, "");
			    done = 0;
			}
			BGP_GET_ADDR(&addr, cp);
			if (!done) {
			    tracef("%s\t%A", comment, &addr);
			} else {
			    tracef(", %A", &addr);
			}
			done++;
			i -= BGP_ROUTE_LENGTH;
		    }
		    trace(TR_BGP, 0, "");
		} else {
		    trace(TR_BGP, 0, "%sNo network data in message", comment);
		}
	    }
	    break;

	default:
	    /*
	     * Invalid type here.  Just return for now.
	     */
	    break;
	}
    }

    trace(TR_BGP, 0, NULL);
}


/*
 * bgp_log_notify - log a notify message for posterity
 */
void
bgp_log_notify(name, message, length, sendit)
char *name;
byte *message;
int length;
int sendit;
{
    register byte *cp;
    register byte *endp;
    register int i;
    int code, subcode;
    const char *fromto = (sendit ? "sent to" : "received from");

    /*
     * The header must have been right or we couldn't have got
     * this far.  Skip it.
     */
    cp = message;
    endp = cp + length;
    BGP_SKIP_HEADER(cp);

    /*
     * If we haven't got enough for a notify message, complain and
     * forget it.
     */
    if ((endp - cp) < BGP_NOTIFY_MIN_LEN) {
	trace(TR_BGP|TR_EXT, LOG_ERR,
	  "bgp_log_notify: notify message %s %s is truncated (length %d)",
	  fromto,
	  name,
	  length);
	return;
    }

    /*
     * Pick up the code and the subcode
     */
    BGP_GET_NOTIFY(code, subcode, cp);

    /*
     * For now just print the codes and any data.  Refine this later
     */
    tracef("NOTIFICATION %s %s: code %d (%s)",
      fromto,
      name,
      code,
      bgp_code(bgp_error_codes, code));

    if (subcode != BGP_ERR_UNSPEC) {
	const char *str;

	switch (code) {
	case BGP_ERR_HEADER:
	    str = bgp_code(bgp_header_error_codes, subcode);
	    break;
	case BGP_ERR_OPEN:
	    str = bgp_code(bgp_open_error_codes, subcode);
	    break;
	case BGP_ERR_UPDATE:
	    str = bgp_code(bgp_update_error_codes, subcode);
	    break;
	default:
	    str = "unknown";
	    break;
	}
	tracef(" subcode %d (%s)", subcode, str);
    }

    i = endp - cp;
    if (i == 1) {
	tracef(" value %d", (int)*cp);
    } else if (i == 2) {
	int val;

	BGP_GET_SHORT(val, cp);
	tracef(" value %d", val);
    } else {
	if (i > 6) {
	    i = 6;
	}
	tracef(" data");
	while (i-- > 0) {
	    tracef(" %02x", *cp++);
	}
    }

    trace(TR_BGP, LOG_WARNING, NULL);
}



/*
 *
 *	Routines to send messages of various sorts
 *
 */

/*
 * bgp_send - send a generic message to a task socket
 */
int
bgp_send(bnp, pkt, length)
bgpPeer *bnp;
byte *pkt;
int length;
{
    int sent_len, rc = 0;
    int times_around = 3;

    trace(TR_BGP, 0, "bgp_send: %sing %d bytes to %s",
	  ((bnp->bgp_spool == NULL) ? "send" : "spool"),
	  length,
	  bnp->bgp_name);
    TRACE_ON(TR_BGP) {
	const char *comment;

	if (bnp->bgp_spool == NULL) {
	    comment = "BGP SEND ";
	} else {
	    comment = "BGP SPOOL ";
	}
	bgp_trace(bnp, (bgpProtoPeer *)NULL, comment, 1, pkt);
    }

    /*
     * If we already have spooled write data, append this to the end of it.
     */
    if (bnp->bgp_spool != NULL) {
	return bgp_spool_write(bnp, pkt, length);
    }

    /* Give up if we get any sort of hard error.  If we get an */
    /* EWOULDBLOCK or EAGAIN error, suggesting that this might succeed if */
    /* we try again later, spool the write data internally if we've been */
    /* configured to do so.  This will set a timer and retry the write later */
    /* We do a direct system call here because it is easier, and */
    /* because partial writes are probably okay.  We try to mimic */
    /* task_send_packet(), however, since that seems like good code */
    do {
	sent_len = write(bnp->bgp_task->task_socket, (caddr_t)pkt, length);
	if (sent_len == length) {
	    bnp->bgp_out_octets += (u_long)sent_len;
	    return (TRUE);
	} else if (sent_len < 0) {
	    rc = errno;
	    switch(rc) {
	    case EHOSTUNREACH:
	    case ENETUNREACH:
		trace(TR_ALL, 0, "bgp_send: retrying write to %s: %m",
		  bnp->bgp_name);
		times_around--;
		break;

	    case EINTR:
		times_around--;
		break;

	    case EWOULDBLOCK:
#if	defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	    case EAGAIN:		/* System V style */
#endif	/* EAGAIN */
		/*
		 * The socket buffer is full.  If we are configured with
		 * enough room for the data spool it, otherwise return
		 * the error.
		 */
		if (bnp->bgp_spool_bufsize >= length) {
		    return bgp_spool_write(bnp, pkt, length);
		}
		trace(TR_ALL, LOG_ERR,
	"bgp_send: sending %d bytes to %s blocked (no spool space): %m",
		  length,
		  bnp->bgp_name);
		return (FALSE);

	    default:
		trace(TR_ALL, LOG_ERR,
		  "bgp_send: sending %d bytes to %s failed: %m",
		  length,
		  bnp->bgp_name);
		errno = rc;
		return (FALSE);
	    }
	} else if (sent_len == 0) {
	    trace(TR_ALL, LOG_ERR,
	      "bgp_send: sending %d bytes to %s: connection closed",
	      length,
	      bnp->bgp_name);
	    return (FALSE);
	} else {
	    bnp->bgp_out_octets += (u_long)sent_len;
	    length -= sent_len;
	    pkt += sent_len;
	    times_around = 3;
	}
    } while (times_around > 0);

    errno = rc;
    trace(TR_ALL, LOG_ERR,
      "bgp_send: sending to %s looping: %m",
      bnp->bgp_name);
    errno = rc;
    return (FALSE);
}

/*
 * bgp_send_open - send an open message using the current version
 */
int
bgp_send_open(bnp, hisversion)
bgpPeer *bnp;
int hisversion;
{
    register byte *cp;
    register int len;
    byte buffer[BGPMAXPACKETSIZE];
    as_t asout;
    time_t holdtimeout;
    u_long idout;
    int authcode, version;

    /*
     * Figure out what we want to send
     */
    asout = bnp->bgp_local_as;
    holdtimeout = bnp->bgp_holdtime_out;
    authcode = bnp->bgp_authtype;
    idout = bnp->bgp_out_id = sock2ip(inet_routerid);
    if (hisversion == BGP_VERSION_UNSPEC) {
	if (BIT_TEST(bnp->bgp_options, BGPO_VERSION)) {
	    version = bnp->bgp_conf_version;
	} else if (bnp->bgp_hisversion != BGP_VERSION_UNSPEC) {
	    version = bnp->bgp_hisversion;
	} else {
	    version = BGP_BEST_VERSION;
	}
    } else {
	if (BIT_TEST(bnp->bgp_options, BGPO_VERSION)) {
	    version = bnp->bgp_conf_version;
	} else {
	    version = hisversion;
	}
    }
    bnp->bgp_version = version;
    bnp->bgp_hisversion = BGP_VERSION_UNSPEC;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_send_open: sending OPEN to peer %s",
	      bnp->bgp_name);
    }

    /*
     * Skip the header for now, we'll do this last.
     */
    cp = buffer;
    BGP_SKIP_HEADER(cp);

    /*
     * Form an open message.  This is version dependent.
     */
    if (version == BGP_VERSION_3) {
	BGP_PUT_OPEN_V3(asout, holdtimeout, idout, authcode, cp);
    } else if (version == BGP_VERSION_2) {
	BGP_PUT_OPEN_V2(asout, holdtimeout, authcode, cp);
    } else {
	trace(TR_ALL, LOG_ERR,
	  "bgp_send_open: internal error! peer %s, version %d\n",
	  bnp->bgp_name, version);
	exit(1);
    }

    /*
     * Add authentication info
     */
    cp = bgp_set_open_auth(&(bnp->bgp_authinfo), cp);

    /*
     * Form header, add authentication, and send it.
     */
    len = cp - buffer;
    BGP_PUT_HDRLEN(len, buffer);
    BGP_PUT_HDRTYPE(BGP_OPEN, buffer);
    BGP_ADD_AUTH(&(bnp->bgp_authinfo), buffer, len);
    bnp->bgp_out_notupdates++;
    return bgp_send(bnp, buffer, len);
}


/*
 * bgp_send_keepalive - send a keepalive to the peer
 */
int
bgp_send_keepalive(bnp)
bgpPeer *bnp;
{
    byte buffer[BGP_HEADER_LEN];

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	      "bgp_send_keepalive: sending KEEPALIVE to peer %s",
	      bnp->bgp_name);
    }

    BGP_PUT_HDRLEN(BGP_HEADER_LEN, buffer);
    BGP_PUT_HDRTYPE(BGP_KEEPALIVE, buffer);
    BGP_ADD_AUTH(&(bnp->bgp_authinfo), buffer, BGP_HEADER_LEN);
    bnp->bgp_out_notupdates++;
    return bgp_send(bnp, buffer, BGP_HEADER_LEN);
}


/*
 * bgp_send_notify - send a notification message
 */
void
bgp_send_notify(bnp, code, subcode, data, datalen)
bgpPeer *bnp;
int code;
int subcode;
byte *data;
int datalen;
{
    register byte *cp;
    int len;
    byte buffer[BGPMAXPACKETSIZE];

    /*
     * If this is anything other than a CEASE, record it for posterity.
     */
    if (code != BGP_CEASE) {
	bnp->bgp_last_code = (byte) code;
	bnp->bgp_last_subcode = (byte) subcode;
    }

    /*
     * Skip over the header for now.
     */
    cp = buffer;
    BGP_SKIP_HEADER(cp);

    /*
     * Insert the message data.
     */
    BGP_PUT_NOTIFY(code, subcode, datalen, data, cp);

    /*
     * Now put the length and the type in the header, and
     * add authentication.
     */
    len = cp - buffer;
    BGP_PUT_HDRLEN(len, buffer);
    BGP_PUT_HDRTYPE(BGP_NOTIFY, buffer);
    BGP_ADD_AUTH(&(bnp->bgp_authinfo), buffer, len);
    bgp_log_notify(bnp->bgp_name, buffer, len, 1);
    bnp->bgp_out_notupdates++;
    (void) bgp_send(bnp, buffer, len);
}


/*
 * bgp_send_notify_none - send a notify with no data
 */
void
bgp_send_notify_none(bnp, code, subcode)
bgpPeer *bnp;
int code;
int subcode;
{
    bgp_send_notify(bnp, code, subcode, (byte *)NULL, 0);
}


/*
 * bgp_send_notify_byte - send a notify with one byte of data
 */
void
bgp_send_notify_byte(bnp, code, subcode, send_byte)
bgpPeer *bnp;
int code;
int subcode;
int send_byte;
{
    byte data;

    data = (byte)send_byte;
    bgp_send_notify(bnp, code, subcode, &data, 1);
}


/*
 * bgp_send_notify_word - send a notify with a 2 byte word of data
 */
void
bgp_send_notify_word(bnp, code, subcode, word)
bgpPeer *bnp;
int code;
int subcode;
int word;
{
    byte data[2];

    data[0] = (word >> 8) & 0xff;
    data[1] = word & 0xff;
    bgp_send_notify(bnp, code, subcode, data, 2);
}


/*
 * bgp_path_attr_error - emit a notification message based on the
 *   gunk returned by the path routines.
 */
void
bgp_path_attr_error(bnp, errcode, errdata, routine)
bgpPeer *bnp;
int errcode;
byte *errdata;
const char *routine;
{
    /*
    register byte *cp = errdata;

     * XXX Finish this
     */
}


/*
 * bgp_send_notify_aspath - send a notification which includes the AS path
 */
void
bgp_send_notify_aspath(bnp, code, subcode, asp)
bgpPeer *bnp;
int code;
int subcode;
as_path *asp;
{
    /*
     * XXX Finish this
     */
}


/*
 * bgp_pp_notify - send an error message to a protopeer, preceded
 *   by an open message.
 */
static void
bgp_pp_notify __PF7(bpp, bgpProtoPeer *,
		    bgp, bgpPeerGroup *,
		    bnp, bgpPeer *,
		    code, int,
		    subcode, int,
		    data, byte *,
		    datalen, int)
{
    register byte *cp;
    int len;
    as_t asout;
    int authcode, version;
    bgpAuthinfo *bap;
    u_long id;
    byte *notify;
    byte buffer[BGPMAXPACKETSIZE];

    /*
     * Protopeers always need to send an open message prior to
     * sending a notify message.  Try to guestimate what we should
     * be sending for the open.
     */
    if (bgp == NULL) {
	bgp = bgp_find_group_by_addr(bpp->bgpp_addr, bpp->bgpp_myaddr);
    }
    if (bgp != NULL && bnp == NULL) {
	bnp = bgp_find_peer_by_addr(bgp, bpp->bgpp_addr, bpp->bgpp_myaddr);
    }

    asout = inet_autonomous_system;
    id = sock2ip(inet_routerid);
    version = BGP_BEST_VERSION;
    authcode = BGP_AUTH_NONE;
    bap = NULL;
    if (bnp != NULL) {
	asout = bnp->bgp_local_as;
#ifdef	notdef
	id = bnp->bgp_out_id;
#endif	/* notdef */
	if (bnp->bgp_hisversion != BGP_VERSION_UNSPEC) {
	    version = bnp->bgp_hisversion;
	} else if (bnp->bgp_version != BGP_VERSION_UNSPEC) {
	    version = bnp->bgp_version;
	} else if (BIT_TEST(bnp->bgp_options, BGPO_VERSION)) {
	    version = bnp->bgp_conf_version;
	}
	authcode = bnp->bgp_authtype;
	bap = &bnp->bgp_authinfo;
    } else if (bgp != NULL) {
	asout = bgp->bgpg_local_as;
	if (BIT_TEST(bgp->bgpg_options, BGPGO_VERSION)) {
	    version = bgp->bgpg_conf_version;
	}
	authcode = bgp->bgpg_authtype;
	bap = &bgp->bgpg_authinfo;
    }

    /*
     * Skip the header for now, we'll do this last.
     */
    cp = buffer;
    BGP_SKIP_HEADER(cp);

    /*
     * Form an open message.  This is version dependent.
     */
    if (version == BGP_VERSION_3) {
	BGP_PUT_OPEN_V3(asout, BGP_HOLDTIME, id, authcode, cp);
    } else if (version == BGP_VERSION_2) {
	BGP_PUT_OPEN_V2(asout, BGP_HOLDTIME, authcode, cp);
    } else {
	/* Internal error */
	assert(FALSE);
    }

    /*
     * Add authentication info
     */
    cp = bgp_set_open_auth(bap, cp);

    /*
     * Form header and add authentication
     */
    len = cp - buffer;
    BGP_PUT_HDRLEN(len, buffer);
    BGP_PUT_HDRTYPE(BGP_OPEN, buffer);
    BGP_ADD_AUTH(bap, buffer, len);

    /*
     * Now put together the notify message
     */
    notify = cp;
    BGP_SKIP_HEADER(cp);

    /*
     * Insert the message data.
     */
    BGP_PUT_NOTIFY(code, subcode, datalen, data, cp);

    /*
     * Now put the length and the type in the header, and
     * add authentication.
     */
    len = cp - notify;
    BGP_PUT_HDRLEN(len, notify);
    BGP_PUT_HDRTYPE(BGP_NOTIFY, notify);
    BGP_ADD_AUTH(bap, notify, len);

    TRACE_ON(TR_BGP) {
	bgp_trace((bgpPeer *)NULL, bpp, "BGP SEND ", 1, buffer);
	bgp_trace((bgpPeer *)NULL, bpp, "BGP SEND ", 1, notify);
    }

    bgp_log_notify(bpp->bgpp_name, notify, len, 1);

    /*
     * Now send it.  We don't actually care much about errors.
     * XXX Maybe we should log them?
     */
    (void)write(bpp->bgpp_task->task_socket, (caddr_t)buffer, cp - buffer);
}


/*
 * bgp_pp_notify_none - send a notify to a protopeer with no data
 */
void
bgp_pp_notify_none(bpp, bgp, bnp, code, subcode)
bgpProtoPeer *bpp;
bgpPeerGroup *bgp;
bgpPeer *bnp;
int code;
int subcode;
{
    bgp_pp_notify(bpp, bgp, bnp, code, subcode, (byte *)0, 0);
}

/*
 * bgp_pp_notify_byte - send a notification to a protopeer with a byte of data
 */
static void
bgp_pp_notify_byte __PF6(bpp, bgpProtoPeer *,
			 bgp, bgpPeerGroup *,
			 bnp, bgpPeer *,
			 code, int,
			 subcode, int,
			 data, int)
{
    byte bytedata;

    bytedata = (byte)data;
    bgp_pp_notify(bpp, bgp, bnp, code, subcode, &bytedata, 1);
}

/*
 * bgp_pp_notify_word - send a notification to a protopeer with 2 bytes of data
 */
PROTOTYPE(bgp_pp_notify_word,
	  static void,
	 (bgpProtoPeer *,
	  bgpPeerGroup *,
	  bgpPeer *,
	  int,
	  int,
	  int));
static void
bgp_pp_notify_word(bpp, bgp, bnp, code, subcode, data)
bgpProtoPeer *bpp;
bgpPeerGroup *bgp;
bgpPeer *bnp;
int code;
int subcode;
int data;
{
    byte worddata[2];

    worddata[0] = (byte)((data >> 8) & 0xff);
    worddata[1] = (byte)(data & 0xff);
    bgp_pp_notify(bpp, bgp, bnp, code, subcode, worddata, 2);
}


/*
 *	BGP receive/buffer management routine
 */
int
bgp_recv(tp, bufpt, maxread, name)
task *tp;
bgpBuffer *bufpt;
int maxread;
char *name;
{
    register bgpBuffer *bp;
    int len_to_read, len_in_buffer;
    int len_read;

    /*
     * If the buffer position pointer and the read pointer are the
     * same, reset them to the start of the buffer.
     */
    bp = bufpt;
    len_in_buffer = bp->bgpb_readptr - bp->bgpb_bufpos;
    if (len_in_buffer == 0) {
	bp->bgpb_bufpos = bp->bgpb_buffer;
	bp->bgpb_readptr = bp->bgpb_buffer;
    }
 
    /*
     * If we don't have maxread characters of space in the buffer, copy current
     * data up.
     */
    if (maxread <= 0 || (bp->bgpb_endbuf - bp->bgpb_readptr) < maxread) {
	if (bp->bgpb_bufpos != bp->bgpb_buffer) {
	    bcopy((caddr_t)bp->bgpb_bufpos, (caddr_t)bp->bgpb_buffer,
	      (size_t)len_in_buffer);
	    bp->bgpb_bufpos = bp->bgpb_buffer;
	    bp->bgpb_readptr = bp->bgpb_buffer + len_in_buffer;
	}
    }
    len_to_read = bp->bgpb_endbuf - bp->bgpb_readptr;


    /*
     * If there is no space at all in the buffer, return 0 to indicate
     * that we couldn't read any more.
     */
    if (len_to_read == 0) {
	return 0;
    }

    /*
     * Read no more than maxread additional characters, if more would be
     * read.
     */
    if (maxread > 0 && maxread < len_to_read) {
	len_to_read = maxread;
    }

    /*
     * Try to read this many characters.
     */
    len_read = read(tp->task_socket, (caddr_t)bp->bgpb_readptr, len_to_read);

    /*
     * If we got a soft error (including EWOULDBLOCK, which is normal),
     * return zero.  If the error is hard, however, complain and return
     * -1.
     */
    if (len_read < 0) {
	switch (errno) {
	case EWOULDBLOCK:
#if	defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	case EAGAIN:
#endif	/* EAGAIN */
	case EINTR:
	    return 0;

	case ENETUNREACH:
	case EHOSTUNREACH:
	    trace(TR_ALL, 0, "bgp_recv: peer %s: %m", name);
	    return 0;

	default:
	    trace(TR_ALL, LOG_ERR, "bgp_recv: read from peer %s failed: %m",
		  name);
	    return -1;
	}
    } else if (len_read == 0) {
	trace(TR_ALL, LOG_ERR,
	      "bgp_recv: peer %s: received unexpected EOF",
	      name);
	return -1;
    }

    /*
     * Add new character count to read pointer and return
     */
    bp->bgpb_readptr += len_read;
    return len_read;
}


/*
 *
 *	Peer establishment support routines.  These routines are
 *	used prior to a peer connection reaching Established state,
 *	both for normal peers and for proto peers (the latter are
 *	peers which have actively connected to us, from which we
 *	await an Open message to determine appropriate additional
 *	processing).
 *
 */

/*
 * bgp_read_message - read and return a message header with data
 */
PROTOTYPE(bgp_read_message,
	  static int,
	 (bgpPeer *,
	  bgpProtoPeer *,
	  int *,
	  int *,
	  int,
	  int));
static int
bgp_read_message(bnp, bpp, length, type, expect_type, expect_type_2)
bgpPeer *bnp;
bgpProtoPeer *bpp;
int *length;
int *type;
int expect_type;
int expect_type_2;
{
    int l, t;
    int igotsome = 0;
    int err;
    int event;
    char *name;
    bgpBuffer *inbuf;
    task *tp;

    /*
     * Get this guy's name and buffer
     */
    if (bnp != NULL) {
	name = bnp->bgp_name;
	inbuf = &(bnp->bgp_inbuf);
	tp = bnp->bgp_task;
    } else {
	name = bpp->bgpp_name;
	inbuf = &(bpp->bgpp_inbuf);
	tp = bpp->bgpp_task;
    }

    /*
     * Check to see if we've got a header's worth of data
     */
    if ((inbuf->bgpb_readptr - inbuf->bgpb_bufpos) < BGP_HEADER_LEN) {
	igotsome = bgp_recv(tp, inbuf, 0, name);
	if (igotsome < 0) {
	    if (bnp) {
		bgp_peer_close(bnp, BGPEVENT_ERROR);
	    }
	    return (-1);
	}
	if ((inbuf->bgpb_readptr - inbuf->bgpb_bufpos) < BGP_HEADER_LEN) {
	    trace(TR_ALL, 0, "bgp_read_message: %s: %d bytes buffered",
	      name, inbuf->bgpb_readptr - inbuf->bgpb_bufpos);
	    return 0;
	}
    }

    /*
     * Check for a valid length
     */
    BGP_GET_HDRLEN(l, inbuf->bgpb_bufpos);
    if (l < BGP_HEADER_LEN || l > BGPMAXPACKETSIZE) {
	if (bnp != NULL) {
	    bgp_send_notify_word(bnp, BGP_ERR_HEADER, BGP_ERRHDR_LENGTH, l);
	    bgp_peer_close(bnp, BGPEVENT_CLOSED);
	} else {
	    bgp_pp_notify_word(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
	      BGP_ERR_HEADER, BGP_ERRHDR_LENGTH, l);
	}
	return (-1);
    }

    /*
     * Check to make sure we've got all message data.  If not,
     * get more.
     */
    if ((inbuf->bgpb_readptr - inbuf->bgpb_bufpos) < l) {
	if (igotsome != 0) {
	    return (0);
	}
	igotsome = bgp_recv(tp, inbuf, 0, name);
	if (igotsome < 0) {
	    if (bnp) {
		bgp_peer_close(bnp, BGPEVENT_ERROR);
	    }
	    return (-1);
	}
	if ((inbuf->bgpb_readptr - inbuf->bgpb_bufpos) < l) {
	    return (0);
	}
    }

    TRACE_ON(TR_BGP) {
	bgp_trace(bnp, bpp, "BGP RECV ", 0, inbuf->bgpb_bufpos);
    }

    /*
     * Check for an in-range type
     */
    BGP_GET_HDRTYPE(t, inbuf->bgpb_bufpos);
    if (t == 0 ||  t > BGP_KEEPALIVE) {
	if (bnp != NULL) {
	    bgp_send_notify_byte(bnp, BGP_ERR_HEADER, BGP_ERRHDR_TYPE, t);
	    bgp_peer_close(bnp, BGPEVENT_CLOSED);
	} else {
	    bgp_pp_notify_byte(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
	      BGP_ERR_HEADER, BGP_ERRHDR_TYPE, t);
	}
	return (-1);
    }


    /*
     * Check for proper minimum lengths.  Also determine the
     * event in case it is needed
     */
    err = 0;
    switch (t) {
    case BGP_OPEN:
	event = BGPEVENT_RECVOPEN;
	if (l < (BGP_HEADER_LEN+BGP_OPEN_MIN_LEN)) {
	    err = 1;
	}
	break;
    case BGP_UPDATE:
	event = BGPEVENT_RECVUPDATE;
	if (l < (BGP_HEADER_LEN+BGP_UPDATE_MIN_LEN)) {
	    err = 1;
	}
	break;
    case BGP_NOTIFY:
	event = BGPEVENT_RECVNOTIFY;
	if (l < (BGP_HEADER_LEN+BGP_NOTIFY_MIN_LEN)) {
	    err = 1;
	}
	break;
    case BGP_KEEPALIVE:
	event = BGPEVENT_RECVKEEPALIVE;
	if (l != BGP_HEADER_LEN) {
	    err = 1;
	}
	break;
    default:
	event = 0;
	assert(FALSE);
    }
    if (err) {
	trace(TR_EXT, LOG_WARNING,
	  "bgp_read_message: peer %s: %s message arrived with length %d",
	  name, trace_state(bgp_message_type_bits, t), l);
	if (bnp != NULL) {
	    bgp_send_notify_word(bnp, BGP_ERR_HEADER, BGP_ERRHDR_LENGTH, l);
	    bgp_peer_close(bnp, BGPEVENT_CLOSED);
	} else {
	    bgp_pp_notify_word(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
	      BGP_ERR_HEADER, BGP_ERRHDR_LENGTH, l);
	}
	return (-1);
    }

    /*
     * If the caller wants something in particular and
     * we didn't get it, complain.
     */
    if (expect_type != 0 && t != expect_type && t != expect_type_2) {
	trace(TR_EXT, LOG_WARNING,
	  "bgp_read_message: peer %s: %s arrived, expected %s%s%s",
	  name,
	  trace_state(bgp_message_type_bits, expect_type),
	  expect_type_2 ? " or " : "",
	  expect_type_2 ? trace_state(bgp_message_type_bits,
	    expect_type_2) : "");
	if (t != BGP_NOTIFY) {
	    if (bnp != NULL) {
		bgp_send_notify_none(bnp, BGP_ERR_FSM, 0);
		bgp_peer_close(bnp, BGPEVENT_CLOSED);
	    } else {
		bgp_pp_notify_none(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
		  BGP_ERR_FSM, 0);
	    }
	}
	/*
	 * If we're logging BGP traffic, log this for posterity
	 */
	TRACE_PROTO(TR_BGP) {
	    trace(TR_BGP, 0,
		  "bgp_read_message: received %d byte message type %d (%s) from %s",
		  l,
		  t,
		  trace_state(bgp_message_type_bits, t),
		  name);
	    if (t == BGP_NOTIFY) {
		bgp_log_notify(name, inbuf->bgpb_bufpos,
		  inbuf->bgpb_readptr - inbuf->bgpb_bufpos, 0);
	    }
	}
	return (-1);
    }

    *length = l;
    *type = t;
    return (1);
}


/*
 * bgp_get_open - check and decode an open message.  This assumes the entire
 *		  message is in the buffer.
 */
PROTOTYPE(bgp_get_open,
	  static int,
	 (bgpPeer *,
	  bgpProtoPeer *,
	  int *,
	  int *,
	  int *,
	  int *,
	  u_long *,
	  int *,
	  byte **,
	  int *));
static int
bgp_get_open(bnp, bpp, r_length, r_version, r_as, r_holdtime, r_id,
				r_authcode, r_authdata, r_authlen)
bgpPeer *bnp;
bgpProtoPeer *bpp;
int *r_length;
int *r_version;
int *r_as;
int *r_holdtime;
u_long *r_id;
int *r_authcode;
byte **r_authdata;
int *r_authlen;
{
    register byte *cp;
    int version;
    int as;
    int holdtime;
    int id;
    int authcode;
    int length;
    char *name;

    /*
     * Figure out what the name should be.  Save a pointer to his buffer
     */
    if (bnp != NULL) {
	name = bnp->bgp_name;
        cp = bnp->bgp_bufpos;
    } else {
	name = bpp->bgpp_name;
        cp = bpp->bgpp_bufpos;
    }

    /*
     * Get a fast copy of the buffer pointer and get the length out of the
     * header.  Skip the header, and make sure the length is reasonable for
     * an open message (we *know* a version 2 message is shorter here).
     */
    BGP_GET_HDRLEN(length, cp);
    BGP_SKIP_HEADER(cp);

    /*
     * Check the version to pick the proper code for reading the
     * message.  Note that we also *know* that version 2 opens are
     * shorter than version 3.
     */
    if (BGP_GET_VERSION(cp) == BGP_VERSION_2) {
	BGP_GET_OPEN_V2(version, as, holdtime, authcode, cp);
	id = 0;
    } else if (BGP_GET_VERSION(cp) == BGP_VERSION_3) {
	if (length < (BGP_HEADER_LEN+BGP_OPENV3_MIN_LEN)) {
	    trace(TR_EXT, LOG_WARNING,
	 "bgp_get_open: peer %s: received short version 3 message (%d octets)",
	      name, length);
	    if (bnp != NULL) {
	        bgp_send_notify_word(bnp, BGP_ERR_HEADER, BGP_ERRHDR_LENGTH,
		  length);
	    } else {
	        bgp_pp_notify_word(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
		  BGP_ERR_HEADER, BGP_ERRHDR_LENGTH, length);
	    }
	    return (-1);
	}
	BGP_GET_OPEN_V3(version, as, holdtime, id, authcode, cp);
    } else {
	int tellversion;

	/*
	 * This case is actually important since it is used for
	 * version negotiation.  We send back our best version
	 * unless there is a configured version number, in which
	 * case we send that back instead.
	 */
	trace(TR_EXT, LOG_WARNING,
	  "bgp_get_open: received unsupported version %d message from peer %s",
	  (int)BGP_GET_VERSION(cp), name);
	tellversion = BGP_BEST_VERSION;
	if (bnp != NULL) {
	    if ((bnp->bgp_options & BGPO_VERSION)
	      && bnp->bgp_conf_version < BGP_BEST_VERSION) {
		tellversion = bnp->bgp_conf_version;
	    }
	    bgp_send_notify_word(bnp, BGP_ERR_OPEN, BGP_ERROPN_VERSION,
	      tellversion);
	} else {
	    bgp_pp_notify_word(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
	      BGP_ERR_OPEN, BGP_ERROPN_VERSION, tellversion);
	}
	return (-1);
    }

    /*
     * So far, so good.  Now check the open parameters for basic
     * sanity.
     */
    if (holdtime < BGP_MIN_HOLDTIME) {
	/*
	 * No error for this condition, just use change value to minimum.
	 */
	trace(TR_EXT, LOG_WARNING,
	 "bgp_get_open: peer %s: hold time too small (%d), set to minimum (%d)",
	  name, holdtime, BGP_MIN_HOLDTIME);
	holdtime = BGP_MIN_HOLDTIME;
    }
    if (version == BGP_VERSION_3 && (id == 0L || id == ~0L)) {
	trace(TR_EXT, LOG_WARNING,
	  "bgp_get_open: peer %s: invalid BGP identifier 0x%lx",
	  name, id);
	if (bnp != NULL) {
	    bgp_send_notify_none(bnp, BGP_ERR_OPEN, BGP_ERROPN_BGPID);
	} else {
	    bgp_pp_notify_none(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
	      BGP_ERR_OPEN, BGP_ERROPN_BGPID);
	}
	return (-1);
    }

    if (!bgp_known_auth(authcode)) {
	trace(TR_EXT, LOG_WARNING,
	  "bgp_get_open: peer %s: unknown athentication code %d used",
	  name, authcode);
	if (bnp != NULL) {
	    bgp_send_notify_none(bnp, BGP_ERR_OPEN, BGP_ERROPN_AUTHCODE);
	} else {
	    bgp_pp_notify_none(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
	      BGP_ERR_OPEN, BGP_ERROPN_AUTHCODE);
	}
	return (-1);
    }

    /*
     * About as good as we can do.  Copy the stuff and return.
     */
    *r_length = length;
    *r_version = version;
    *r_as = as;
    *r_holdtime = holdtime;
    *r_id = id;
    *r_authcode = authcode;
    if (version == BGP_VERSION_2)
        *r_authlen = length - (BGP_OPENV2_MIN_LEN+BGP_HEADER_LEN);
    else
        *r_authlen = length - (BGP_OPENV3_MIN_LEN+BGP_HEADER_LEN);
    *r_authdata = cp;

    return (1);
}


/*
 * bgp_recv_open - processes incoming data when in OpenSent and OpenConfirm
 *		   states.
 */
void
bgp_recv_open(tp)
task *tp;
{
    bgpPeer *bnp;
    bgpPeerGroup *bgp;
    int res;
    int length, type;
    int version, as, holdtime;
    u_long id;
    int code, datalen;
    byte *datap;

    bnp = (bgpPeer *)tp->task_data;
    bgp = bnp->bgp_group;
    /* Invalid state */
    assert(bnp->bgp_state == BGPSTATE_OPENSENT ||
	   bnp->bgp_state == BGPSTATE_OPENCONFIRM);

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_recv_open: called for peer %s", bnp->bgp_name);
    }

    if (bnp->bgp_state == BGPSTATE_OPENSENT) {
	/*
	 * Waiting for open.  Acquire a message.  Note that
	 * bgp_read_message will close the connection on errors,
	 * so we don't have to.
	 */
	res = bgp_read_message(bnp, (bgpProtoPeer *)0, &length, &type,
	  BGP_OPEN, 0);
	if (res <= 0) {
	    return;
	}

	/*
	 * Got one.  Decode and validate the message.  Note that
	 * bgp_get_open() does not close the connection on errors.
	 * We must do that.
	 */
	res = bgp_get_open(bnp, (bgpProtoPeer *)0, &length, &version,
	  &as, &holdtime, &id, &code, &datap, &datalen);
	if (res <= 0) {
	    bgp_peer_close(bnp, BGPEVENT_RECVOPEN);
	    return;
	}

	/*
	 * Okdk.  Now remember that we sent an open containing a version
	 * number already, as did he.  If our version numbers don't
	 * match we've got problems.
	 *
	 * Here's what we do.  If I sent a higher version than him but
	 * support his version, I accept his open message and transition
	 * to OpenConfirm.  I expect him to send a Notify in response
	 * to my open, terminating the connection and telling me his
	 * highest version less than mine.  If he does this we open
	 * again using his best.  If he accepts my open (the son of a
	 * bitch) I send a Cease and we start over using my best.  We
	 * also start over using my best if he sends any notify message
	 * other than bad version.
	 *
	 * If I sent a lower version than him but support his version,
	 * I send a Cease message and start again with my best version.
	 *
	 * Note that if a version number was configured we essentially
	 * pretend we only support that version.
	 */
	if (bnp->bgp_version != version) {
	    if (bnp->bgp_options & BGPO_VERSION) {
		bgp_send_notify_word(bnp, BGP_ERR_OPEN, BGP_ERROPN_VERSION,
		  (int)bnp->bgp_version);
		bgp_peer_close(bnp, BGPEVENT_RECVOPEN);
		return;
	    } else if (bnp->bgp_version < version) {
		bgp_send_notify_none(bnp, BGP_CEASE, 0);
		bgp_peer_close(bnp, BGPEVENT_RECVOPEN);
		return;
	    }
	    bnp->bgp_hisversion = version;
	} else {
	    /*
	     * We agree on a version, forget that his version was
	     * ever different than ours
	     */
	    bnp->bgp_hisversion = BGP_VERSION_UNSPEC;
	}

	/*
	 * Check the AS for validity.  If an AS was configured for
	 * this guy he must be using it.
	 */
	if (as != bgp->bgpg_peer_as) {
	    trace(TR_EXT, LOG_WARNING,
	      "bgp_recv_open: peer %s claims AS %d, %d configured",
	      bnp->bgp_name, as, bnp->bgp_peer_as);
	    bgp_send_notify_none(bnp, BGP_ERR_OPEN, BGP_ERROPN_AS);
	    bgp_peer_close(bnp, BGPEVENT_RECVOPEN);
	    return;
	}

	/*
	 * Check the authentication.
	 */
        if (!bgp_open_auth(bnp->bgp_name, &bnp->bgp_authinfo, code,
	    datap, datalen)
	  || !bgp_check_auth(&bnp->bgp_authinfo, bnp->bgp_task,
	    bnp->bgp_bufpos, length)) {
	    bgp_send_notify_none(bnp, BGP_ERR_OPEN, BGP_ERROPN_AUTH);
	    bgp_peer_close(bnp, BGPEVENT_RECVOPEN);
	    return;
	}

	/*
	 * Done.  Save open info, send a keepalive to acknowlege the
	 * open, delete open message from buffer and do a state transition.
	 */
	bnp->bgp_authtype = code;
	bnp->bgp_id = id;
	bnp->bgp_hisholdtime = holdtime;
	bnp->bgp_peer_as = as;
	bnp->bgp_bufpos += length;
	bgp_send_keepalive(bnp);
	bgp_event(bnp, BGPEVENT_RECVOPEN, BGPSTATE_OPENCONFIRM);
    }

    /*
     * Here we're in OpenConfirm state.  Check to see if we have
     * a response from the remote peer.
     */
    res = bgp_read_message(bnp, (bgpProtoPeer *)0, &length, &type,
      BGP_KEEPALIVE, BGP_NOTIFY);
    if (res <= 0) {
	if (res < 0) {
	    bnp->bgp_hisversion = BGP_VERSION_UNSPEC;
	}
	return;
    }

    if (!bgp_check_auth(&bnp->bgp_authinfo, bnp->bgp_task,
      bnp->bgp_bufpos, length)) {
	/*
	 * Authentication failed, inform our pal and close.
	 */
	if (type == BGP_NOTIFY) {
	    bgp_peer_close(bnp, BGPEVENT_RECVNOTIFY);
	} else {
	    bgp_send_notify_none(bnp, BGP_ERR_HEADER, BGP_ERRHDR_UNSYNC);
	    bgp_peer_close(bnp, BGPEVENT_RECVKEEPALIVE);
	}
	return;
    }

    /*
     * Got one.  This could be a Keepalive, or it could be a Notify.
     * If it is a Notify, it may be a response to a version mismatch
     * so we've got some decoding to do.  Note that `type' is used
     * as the code, and `code' is used as the subcode.
     */
    if (type == BGP_NOTIFY) {
	datap = bnp->bgp_bufpos;
	BGP_SKIP_HEADER(datap);
	BGP_GET_NOTIFY(type, code, datap);
	datalen = length - (BGP_HEADER_LEN+BGP_NOTIFY_MIN_LEN);
	if (res > 0 && type == BGP_ERR_OPEN && code == BGP_ERROPN_VERSION
	  && datalen == 2) {
	    BGP_GET_SHORT(version, datap);
	    /*
	     * Check for a bit of insanity on his part
	     */
	    if (bnp->bgp_hisversion == BGP_VERSION_UNSPEC) {
		trace(TR_ALL, LOG_WARNING,
    "bgp_recv_open: version mismatch from %s (%d) when version %d agreed upon",
		  bnp->bgp_name, version, bnp->bgp_version);
	    } else
	    /*
	     * This is not likely to happen, but we check for it
	     * anyway.
	     */
	    if (version > bnp->bgp_hisversion && BGP_KNOWN_VERSION(version)) {
		if (version == BGP_BEST_VERSION) {
		    bnp->bgp_hisversion = BGP_VERSION_UNSPEC;
		} else {
		    bnp->bgp_hisversion = version;
		}
	    }
	} else {
	    /*
	     * Here we make sure we restart with highest version.
	     * Also, log the notify message (someday XXX).
	     */
	    bnp->bgp_hisversion = BGP_VERSION_UNSPEC;
	}
	bgp_peer_close(bnp, BGPEVENT_RECVNOTIFY);
    } else /* (type == BGP_KEEPALIVE) */ {
	/*
	 * Check to see if this guy has accepted mismatched
	 * versions(!).  Tell him to cease if so.  If not,
	 * transition to established, reset the receive routine
	 * and call the update routine for further processing.
	 */
	if (bnp->bgp_hisversion != BGP_VERSION_UNSPEC) {
	    trace(TR_ALL, LOG_WARNING,
	  "bgp_recv_open: peer %s accepted mismatched versions: his %d mine %d",
	      bnp->bgp_name, bnp->bgp_hisversion, bnp->bgp_version);
	    bgp_send_notify_none(bnp, BGP_CEASE, 0);
	    bnp->bgp_hisversion = BGP_VERSION_UNSPEC;
	    bgp_peer_close(bnp, BGPEVENT_RECVKEEPALIVE);
	} else {
	    bnp->bgp_bufpos += length;
	    bgp_peer_established(bnp);
	}
    }
}


/*
 * bgp_pp_recv - receive an open message on behalf of a protopeer, decide
 *		 what to do with it.
 */
void
bgp_pp_recv(tp)
task *tp;
{
    bgpPeer *bnp;
    bgpProtoPeer *bpp;
    bgpPeerGroup *bgp;
    int length, type;
    int version, as, holdtime;
    u_long id;
    int code, datalen;
    byte *datap;
    int res;

    bpp = (bgpProtoPeer *)tp->task_data;

    /*
     * We're waiting for an open message.  See what we get.
     */
    res = bgp_read_message((bgpPeer *)NULL, bpp, &length, &type, BGP_OPEN, 0);

    /*
     * If we didn't get a whole message, wait for more.  If
     * something screwed up, terminate.
     */
    if (res <= 0) {
	if (res < 0) {
	    bgp_pp_delete(bpp);
	}
	return;
    }

    /*
     * Got an open message.  Decode it.
     */
    res = bgp_get_open((bgpPeer *)NULL, bpp, &length, &version, &as,
      &holdtime, &id, &code, &datap, &datalen);
    if (res <= 0) {
	bgp_pp_delete(bpp);
	return;
    }

    /*
     * Try to find the group to which this peer belongs.  If there isn't
     * one he's out of luck.
     */
    bgp = bgp_find_group(bpp->bgpp_addr, bpp->bgpp_myaddr, (as_t)as, (as_t)0,
      code, datap, datalen);
    if (bgp == NULL) {
	trace(TR_EXT, LOG_WARNING,
	  "bgp_pp_recv: no group for %s found, dropping him",
	  bpp->bgpp_name);
	bgp_pp_notify_none(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
	  BGP_ERR_OPEN, BGP_ERROPN_AUTH);
	bgp_pp_delete(bpp);
	return;
    }
    if (BIT_TEST(bgp->bgpg_flags, BGPGF_IDLED)) {
	trace(TR_EXT, LOG_WARNING,
	  "bgp_pp_recv: dropping %s, group %s is idled",
	  bpp->bgpp_name, bgp->bgpg_name);
	bgp_pp_notify_none(bpp, bgp, (bgpPeer *)0, BGP_CEASE, 0);
	bgp_pp_delete(bpp);
	return;
    }

    /*
     * Got a group.  See if we can find a matching peer in this group.
     */
    bnp = bgp_find_peer(bgp, bpp->bgpp_addr, bpp->bgpp_myaddr);
    if (bnp != (bgpPeer *)NULL) {
	/*
	 * This peer is configured, or is already talking to us at least.
	 * We need to figure out what state the configured fellow is in.
	 * If he's established, in openconfirm or idle we terminate the
	 * protopeer.
	 *
	 * XXX: Note that there is a bug here when the peer's state is
	 * openconfirm.  We know the state of the remote protopeer is.
	 * opensent.  The state of the remote peer, when the local is
	 * in openconfirm, might be opensent, openconfirm or established.
	 * If the remote peer is in opensent he may yet decide to close
	 * the peer connection even though we've closed the protopeer
	 * connection.
	 *
	 * Oh, well.  If the remote is another gated I think we'd
	 * be fine since the remote would not allow us to advance
	 * to openconfirm without making a decision concerning which
	 * to close.
	 */
	if (bnp->bgp_state == BGPSTATE_ESTABLISHED
	  || bnp->bgp_state == BGPSTATE_OPENCONFIRM
	  || bnp->bgp_state == BGPSTATE_IDLE) {
	    trace(TR_BGP, LOG_WARNING,
	      "bgp_pp_recv: rejecting connection from %s, peer in state %s",
	      bnp->bgp_name, trace_state(bgp_state_bits, bnp->bgp_state));
	    bgp_pp_notify_none(bpp, bgp, bnp, BGP_CEASE, 0);
	    bgp_pp_delete(bpp);
	    return;
	}

	/*
	 * If the peer is in connect or opensent we've got trouble.
	 * If we don't like the protopeer's version at all, dump
	 * the protopeer.  If the protopeer's version is better than
	 * the one we're openning with, dump the peer, if worse dump
	 * the protopeer.  If we're both running better than version 2
	 * we compare our router ID to his to determine which to keep.
	 * If we're both running version 2 we use the interface addresses
	 * for the same purpose.  Note that we always keep the better version.
	 *
	 * If res == 0, close the protopeer.  If res == 1, close the
	 * peer.  Don't let any protopeer continue if we hate his
	 * version.
	 */
	if (!BGP_KNOWN_VERSION(version)
	  || (BIT_TEST(bnp->bgp_options, BGPO_VERSION)
	  && version != bnp->bgp_conf_version)) {
	    res  = 0;
	} else if (bnp->bgp_state != BGPSTATE_ACTIVE) {
	    res = 0;
	    if (bnp->bgp_version != version) {
		/*
		 * The versions are mismatched, close the lowest version.
		 */
		if (bnp->bgp_version < version) {
		    res = 1;
		}
	    } else if (version > BGP_VERSION_2) {
		if (ntohl(bnp->bgp_out_id) < ntohl(id)) {
		    res = 1;
		}
	    } else {
		if (ntohl(sock2ip(bnp->bgp_myaddr))
		    < ntohl(sock2ip(bnp->bgp_addr))) {
		    res = 1;
		}
	    }
	} else {
	    /*
	     * Active state here, no peer to close.
	     *
	     * XXX This could possibly allow us to end up in a version
	     * lower than we otherwise would.  This could only be caused
	     * by a really warped set of circumstances, however, so we'll
	     * ignore the possibility.
	     */
	    res = 1;
	}


	if (!res) {
	    trace(TR_EXT, LOG_INFO,
	      "bgp_pp_recv: dropping %s, connection collision prefers %s",
	      bpp->bgpp_name, bnp->bgp_name);
	    bgp_pp_notify_none(bpp, bgp, bnp, BGP_CEASE, 0);
	    bgp_pp_delete(bpp);
	    return;
	}

	if (bnp->bgp_state == BGPSTATE_OPENSENT) {
	    /*
	     * Send him a message since this is polite
	     */
	    bgp_send_notify_none(bnp, BGP_CEASE, 0);
	}
	trace(TR_EXT, LOG_INFO,
	  "bgp_pp_recv: dropping %s, connection collision prefers %s",
	  bnp->bgp_name, bpp->bgpp_name);
	bgp_peer_close(bnp, BGPEVENT_STOP);
        bgp_use_protopeer(bnp, bpp, length);
    } else {
	/*
	 * This peer isn't configured, though it matches the address-and-mask
	 * list of a group.  Check to make sure his version is acceptable.
	 * If so, start him up.
	 */
	if (!BGP_KNOWN_VERSION(version)
	  || (BIT_TEST(bgp->bgpg_options, BGPO_VERSION)
	  && version != bgp->bgpg_conf_version)) {
	    int retversion;

	    if (BIT_TEST(bgp->bgpg_options, BGPO_VERSION)) {
		retversion = bgp->bgpg_conf_version;
	    } else {
		retversion = BGP_BEST_VERSION;
	    }
	    bgp_pp_notify_word(bpp, bgp, (bgpPeer *)0,
	      BGP_ERR_OPEN, BGP_ERROPN_VERSION, retversion);
	    bgp_pp_delete(bpp);
	    return;
	}
	bnp = bgp_new_peer(bgp, bpp, length);
    }

    /*
     * We now have a peer.  There is still a possibility of problems
     * with the version here, however, if there is a configured
     * version.  Send an open in response, then terminate if there
     * is a version mismatch.  Otherwise, save open info and send a
     * keepalive to acknowlege the open.  Then delete open message
     * from buffer. Do appropriate state transition.
     */
    bnp->bgp_id = id;
    bnp->bgp_hisholdtime = holdtime;
    bgp_event(bnp, BGPEVENT_OPEN, BGPSTATE_OPENSENT);
    bgp_send_open(bnp, version);
    if (bnp->bgp_version != version) {
	bgp_send_notify_word(bnp, BGP_ERR_OPEN, BGP_ERROPN_VERSION,
	  (int)bnp->bgp_version);
	bgp_peer_close(bnp, BGPEVENT_RECVOPEN);
	return;
    }
    bgp_event(bnp, BGPEVENT_RECVOPEN, BGPSTATE_OPENCONFIRM);
    bgp_send_keepalive(bnp);
    bgp_recv_change(bnp, bgp_recv_open, "bgp_recv_open");
    bnp->bgp_bufpos += length;
    /*
     * A funny situation here.  There shouldn't be any more data in
     * the buffer because we only just sent the open message.  If
     * there is then complain about it, but try to continue processing
     * anyway.
     */
    if (bnp->bgp_readptr != bnp->bgp_bufpos) {
	trace(TR_BGP, LOG_WARNING,
	  "bgp_pp_recv: peer %s sent unexpected extra data, probably insane",
	  bnp->bgp_name);
	bgp_recv_open(bnp->bgp_task);
    }
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
