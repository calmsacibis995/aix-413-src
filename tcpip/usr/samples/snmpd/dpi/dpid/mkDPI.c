static char sccsid[] = "@(#)15	1.3  src/tcpip/usr/samples/snmpd/dpi/dpid/mkDPI.c, snmp, tcpip411, GOLD410 2/7/94 16:06:41";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:        /usr/samples/snmpd/dpi/dpid/mkDPI.c
 */

/*
 * INTERNATIONAL BUSINESS MACHINES CORP. PROVIDES THIS SOURCE CODE
 * EXAMPLE, ASSOCIATED LIBRARIES AND FILES "AS IS," WITHOUT WARRANTY 
 * OF ANY KIND EITHER EXPRESSED OR IMPLIED INCLUDING BUT NOT LIMITED 
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE.  The entire risk as to the quality and performance 
 * of this example is with you.  Should any part of this source code 
 * example prove defective you (and not IBM or an IBM authorized 
 * dealer) assume the entire cost of all necessary servicing, repair, 
 * or correction.
 */

/* DPI parser */

/*
 * This is the DPI library as described in rfc1228 and has the DPI 1.1 
 * enhancements.
 */

#define DPI_DEBUG

#include <stdio.h>

#include "snmp_dpi.h"

static struct dpi_get_packet *pDPIget();
static struct dpi_next_packet *pDPInext();
static struct dpi_set_packet *pDPIset();
static struct dpi_trap_packet *pDPItrap();
static struct dpi_resp_packet *pDPIresponse();

static void fDPIget();
static void fDPInext();
static void fDPIset();
static void fDPIset_packet();
static void fDPItrap();
static void fDPIresponse();

static int cDPIget();
static int cDPInext();
static int cDPIset();
static int cDPItrap();
static int cDPIresponse();

static struct snmp_dpi_hdr *mkDPIhdr();
static struct dpi_get_packet *mkDPIget();
static struct dpi_next_packet *mkDPInext();
static struct dpi_set_packet *mkDPIset();

static long int_val();

static unsigned char new_packet[4096];
static int packet_len;
static int dpi_debug;

#ifdef DPI_DEBUG
static char *dpi_type_text[] = {
         "Unknown",
         "SNMP_DPI_GET",
         "SNMP_DPI_GET_NEXT",
         "SNMP_DPI_SET",
         "SNMP_DPI_TRAP",
         "SNMP_DPI_RESPONSE",
         "SNMP_DPI_REGISTER"
};
#define SNMP_DPI_MAXTYPE ((sizeof(dpi_type_text) - 1) / sizeof(char *))
#endif /* def DPI_DEBUG */

void DPIdebug(onoff)
int onoff;
{
   dpi_debug = onoff;
}

struct snmp_dpi_hdr *pDPIpacket(packet)
unsigned char *packet;
{
	struct snmp_dpi_hdr *hdr;
	int len, offset, level_unsupported;


	hdr = (struct snmp_dpi_hdr *) 
			calloc(1, sizeof(struct snmp_dpi_hdr));
	if (hdr == 0) return(0);

	len = (packet[0] << 8) + packet[1];
	len += 2;
	offset = 2;
	hdr->proto_major = packet[offset++];
	hdr->proto_minor = packet[offset++];
	hdr->proto_release = packet[offset++];
	hdr->packet_type = packet[offset++];

#ifdef DPI_DEBUG
    if (dpi_debug) {
       int i;

       i = hdr->packet_type;
       if (i>SNMP_DPI_MAXTYPE) i = 0;
       printf("pDPIpacket: Major=%d, Minor=%d, Release=%d, Type=%s\n",
              hdr->proto_major,
              hdr->proto_minor,
              hdr->proto_release,
              dpi_type_text[i]);
    }
#endif /* def DPI_DEBUG */

    level_unsupported = 0;
    switch (hdr->proto_minor) {
    case 1:
      switch (hdr->proto_release) {
      case 0:
      case 1:
        break; /* all ok */
      default:
        level_unsupported = 1;
      } /* end switch (hdr->proto_release) */
      break;
    default:
      level_unsupported = 1;
    } /* end switch (hdr->proto_minor) */
    if (level_unsupported || (hdr->proto_major ^= SNMP_DPI_PROTOCOL)) {
#ifdef DPI_DEBUG
       if (dpi_debug)
          printf("pDPIpacket: No support for this level of DPI\n");
#endif /* def DPI_DEBUG */
       free(hdr);
       return(0);
    }

	switch (hdr->packet_type) {
	case SNMP_DPI_GET:
	case SNMP_DPI_REGISTER:
		hdr->packet_body.dpi_get = pDPIget(packet+offset,len-offset);
		break;
	case SNMP_DPI_GET_NEXT:
		hdr->packet_body.dpi_next = pDPInext(packet+offset,len-offset);
		break;
	case SNMP_DPI_SET:
		hdr->packet_body.dpi_set = pDPIset(hdr, packet+offset,len-offset);
		break;
	case SNMP_DPI_TRAP:
		hdr->packet_body.dpi_trap = pDPItrap(hdr, packet+offset,len-offset);
		break;
	case SNMP_DPI_RESPONSE:
		hdr->packet_body.dpi_response = pDPIresponse(hdr, packet+offset,
			len-offset);
		break;
	}
	return(hdr);
}

static struct dpi_get_packet *pDPIget(packet,len)
unsigned char *packet;
int len;
{
	struct dpi_get_packet *get;
	int l;

	get = (struct dpi_get_packet *) calloc(1, sizeof(struct dpi_get_packet));
	if (get == 0) return(0);
	l = strlen(packet) + 1;
	get->object_id = (char *) malloc(l);
	strcpy(get->object_id,packet);
#ifdef DPI_DEBUG
    if (dpi_debug) {
       printf("pDPIget: oid=%s\n", get->object_id);
    }
#endif /* def DPI_DEBUG */
	return(get);
}

static struct dpi_next_packet *pDPInext(packet,len)
unsigned char *packet;
int len;
{
	struct dpi_next_packet *next;
	int l;
	unsigned char *cp;

	next = (struct dpi_next_packet *) calloc(1, sizeof(struct dpi_next_packet));
	if (next == 0) return(0);
	cp = (unsigned char *) packet;
	l = strlen(cp) + 1;
	next->object_id = (char *) malloc(l);
	strcpy(next->object_id,cp);
	cp += l;
	l = strlen(cp) + 1;
	next->group_id = (char *) malloc(l);
	strcpy(next->group_id,cp);
#ifdef DPI_DEBUG
    if (dpi_debug) {
       printf("pDPInext: group_id=%s, oid=%s\n",
              next->group_id,   next->object_id);
    }
#endif /* def DPI_DEBUG */
	return(next);
}

static struct dpi_set_packet *pDPIset(hdr,packet,len)
struct snmp_dpi_hdr *hdr;
unsigned char *packet;
int len;
{
	struct dpi_set_packet *set, *first = 0, *next;
	int l; 
	long int tmp_int;
	unsigned char *cp;
	unsigned char *cpt1, *cpt2;

	if (len == 0) return(0);	/* nothing to parse */
	cp = packet;

	while (len > 0) {
		set = (struct dpi_set_packet *) 
			 calloc(1, sizeof(struct dpi_set_packet));
		if (set == 0) {
			if (first) fDPIset_packet(first);
			return(0);
		}

		if (first == 0) first = set;
		else next -> next = set;
		next = set;

		set -> next = NULL;
		l = strlen(cp) + 1;
		set->object_id = (char *) malloc(l);
		strcpy(set->object_id,cp);
		cp += l;
		set->type = *(cp++);
		l = (*(cp++) << 8);
		l += *(cp++);
		set->value_len = l;
		set->value = (char *) malloc(l);
		if (set->type & ~SNMP_TYPE_MASK) { /* an integer type */
			if (set->value_len != 4) {
				fprintf(stderr,
						"pDPIset: Invalid packet, integer length %d\n",
						set->value_len);
				if (first) fDPIset_packet(first);
				return(0);
			}
			if (set->type != SNMP_TYPE_INTERNET) { /* an integer type */
				tmp_int = int_val(cp);
				bcopy(&tmp_int,set->value,set->value_len);
			} else {
				/* By definition: IP address is in network byte order */
				bcopy(cp,set->value,set->value_len);
/*				tmp_int = int_val(cp);
 *				bcopy(&tmp_int,set->value,set->value_len);
 */			}
		} else bcopy(cp,set->value,l);     /* an octet string */
		cp += l;
		/*  calculate remaining length:
		 *  (length - (1 for type + 2 for length + l for value +
		 *             strlength object_ID + 1 for ending NULL))
		 */
		len -= (1 + 2 + l + strlen(set->object_id) + 1);
#ifdef DPI_DEBUG
		if (dpi_debug) {
			printf("pDPIset: oid=%s, type='%2.2x'H, value_len=%d\n",
					set->object_id, set->type, set->value_len);
			if (set->type & ~SNMP_TYPE_MASK) { /* an integer type */
				if (set->type == SNMP_TYPE_NUMBER)
					printf("         value=%ld [0x%x]\n",
							*((long *)set->value), *((long*)set->value));
				else
					printf("         value=%lu [0x%x]\n",
							*((unsigned long*)set->value),
							*((unsigned long*)set->value));
			} else {                           /* an octet string */
				printf("         value='");
				for (l=0; l < set->value_len; l++)
					printf("%2.2x",set->value[l]);
				printf("'H\n");
			}
		} /* end if (dpi_debug) */
#endif /* def DPI_DEBUG */
	} /* while (len > 0) */
	return(first);
}


static struct dpi_trap_packet *pDPItrap(hdr,packet,len)
struct snmp_dpi_hdr *hdr;
unsigned char *packet;
int len;
{
	struct dpi_trap_packet *trap;
	int                     epl;
	unsigned char          *cp;

	trap = (struct dpi_trap_packet *) 
			calloc(1, sizeof(struct dpi_trap_packet));
	if (trap == 0) return(0);

	switch (hdr->proto_minor) {
		case 1: /* Version 1.x */
			switch (hdr->proto_release) {
				case 0: /* Version 1.0 was initial implementation */
					trap->generic = *packet;
					trap->specific = *(packet + 1);
					trap->enterprise = 0;
#ifdef DPI_DEBUG
					if (dpi_debug) {
						printf("pDPItrap: generic=%d [0x%x], specific=%d [0x%x]\n",
								trap->generic,  trap->generic,
								trap->specific, trap->specific);
					}
#endif /* def DPI_DEBUG */
					trap->info = pDPIset(hdr,packet+2,len-2);
					return(trap);
					break; /*NOTREACHED*/
				case 1: /* From version 1.1 onwards we allow enterprise */
				default:
					trap->generic = int_val(packet);
					trap->specific = int_val(packet + 4);
					cp = (unsigned char *)(packet + 8);
					epl = strlen(cp) + 1;
					/* a NULL string (e.g epl==0) means: user wants default */
					if (epl > 1) {
						trap->enterprise = (char *) malloc(epl);
						if (trap->enterprise == 0) {
							free(trap);
							return(0);
						}
						strcpy(trap->enterprise,cp);
					}
					else trap->enterprise = 0;
#ifdef DPI_DEBUG
					if (dpi_debug) {
						printf("pDPItrap: generic=%ld [0x%x], specific=%ld [0x%x]",
								trap->generic,  trap->generic,
								trap->specific, trap->specific);
						if (trap->enterprise)
							printf(", enterpise=%s\n",trap->enterprise);
						else printf(", no enterprise specified\n");
					}
#endif /* def DPI_DEBUG */
					cp += epl;
					trap->info = pDPIset(hdr,cp,len-8-epl);
					return(trap);
					break; /*NOTREACHED*/

			} /* end switch (hdr->proto_release) */
		case 2: /* Version 2.x */
		default:
			return(0);  /* we don't know how to handle (yet) */
			break; /*NOTREACHED*/
	} /* end switch (hdr->proto_minor) */
	/*NOTREACHED*/
}

static struct dpi_resp_packet *pDPIresponse(hdr,packet,len)
struct snmp_dpi_hdr *hdr;
unsigned char *packet;
int len;
{
	struct dpi_resp_packet *resp;

	resp = (struct dpi_resp_packet *)
			calloc(1, sizeof(struct dpi_resp_packet));
	if (resp == 0) return(0);

	resp->ret_code = *packet;
#ifdef DPI_DEBUG
	if (dpi_debug) {
		printf("pDPIresponse: ret_code=%d\n", resp->ret_code);
	}
#endif /* def DPI_DEBUG */
	resp->ret_data = pDPIset(hdr,packet+1,len-1);
	return(resp);
}

void fDPIparse(hdr)
struct snmp_dpi_hdr *hdr;
{
	if (hdr == 0) return;
	switch (hdr->packet_type) {
	case SNMP_DPI_GET:
	case SNMP_DPI_REGISTER:
		fDPIget(hdr);
		break;
	case SNMP_DPI_GET_NEXT:
		fDPInext(hdr);
		break;
	case SNMP_DPI_SET:
		fDPIset(hdr);
		break;
	case SNMP_DPI_TRAP:
		fDPItrap(hdr);
		break;
	case SNMP_DPI_RESPONSE:
		fDPIresponse(hdr);
		break;
	default: /* should not occur */
		fprintf(stderr,"fDPIparse: Unknown packet_type: %d\n",
				hdr->packet_type);
		break;
	}
	free(hdr);
}

static void fDPIget(hdr)
struct snmp_dpi_hdr *hdr;
{
	struct dpi_get_packet *get;

	get = hdr->packet_body.dpi_get;
	if (get == 0) return;
	if (get->object_id) free(get->object_id);
	free(get);
}

static void fDPInext(hdr)
struct snmp_dpi_hdr *hdr;
{
	struct dpi_next_packet *next;

	next = hdr->packet_body.dpi_next;
	if (next == 0) return;
	if (next->object_id) free(next->object_id);
	if (next->group_id) free(next->group_id);
	free(next);
}

static void fDPIset(hdr)
struct snmp_dpi_hdr *hdr;
{
	fDPIset_packet(hdr->packet_body.dpi_set);
}

static void fDPIset_packet(packet)
struct dpi_set_packet *packet;
{
	struct dpi_set_packet *set, *next;

	next = packet;

	while (next) {
		set = next;
		if (set->object_id) free(set->object_id);
		if (set->value) free(set->value);
		next = set->next;
		free(set);
	} /* end while (next) */
}

static void fDPItrap(hdr)
struct snmp_dpi_hdr *hdr;
{
	struct dpi_trap_packet *trap;
	struct dpi_set_packet *set;

	trap = hdr->packet_body.dpi_trap;
	if (trap == 0) return;

	if (trap->enterprise) free(trap->enterprise);
	if (trap->info) fDPIset_packet(trap->info);
	free(trap);
}

static void fDPIresponse(hdr)
struct snmp_dpi_hdr *hdr;
{
	struct dpi_resp_packet *resp;
	struct dpi_set_packet *set;
	struct dpi_set_packet *set2;

	resp = hdr->packet_body.dpi_response;
	if (resp == 0) return;

	set = resp->ret_data;
	while (set != 0) {
		if (set->object_id) free(set->object_id);
		if (set->value) free(set->value);
		set2 = set;
		set = set -> next;
		free(set2);
	}
	free(resp);
}

unsigned char *cDPIpacket(hdr)
struct snmp_dpi_hdr *hdr;
{
	int rc, len;

	if (hdr == 0) {
		return(0);
	}
	packet_len = 2;
	new_packet[packet_len++] = hdr->proto_major;
	new_packet[packet_len++] = hdr->proto_minor;
	new_packet[packet_len++] = hdr->proto_release;
	new_packet[packet_len++] = hdr->packet_type;
	switch (hdr->packet_type) {
	case SNMP_DPI_GET:
	case SNMP_DPI_REGISTER:
		rc = cDPIget(hdr->packet_body.dpi_get);
		break;
	case SNMP_DPI_GET_NEXT:
		rc = cDPInext(hdr->packet_body.dpi_next);
		break;
	case SNMP_DPI_SET:
		rc = cDPIset(hdr->packet_body.dpi_set);
		break;
	case SNMP_DPI_TRAP:
		rc = cDPItrap(hdr->packet_body.dpi_trap);
		break;
	case SNMP_DPI_RESPONSE:
		rc = cDPIresponse(hdr->packet_body.dpi_response);
		break;
	default: /* should not occur */
		fprintf(stderr,"cDPIpacket: Unknown packet_type: %d\n",
				hdr->packet_type);
		rc = -1;
		break;
	}
	if (rc == -1) return(0);
	len = packet_len - 2;
	new_packet[1] = len & 0xff;
	len >>= 8;
	new_packet[0] = len & 0xff;
	return(new_packet);
}

static int cDPIget(get)
struct dpi_get_packet *get;
{
	if (get->object_id == 0) return(-1);

	strcpy(&new_packet[packet_len],get->object_id);
	packet_len += strlen(get->object_id) + 1;
	return(0);
}

static int cDPInext(next)
struct dpi_next_packet *next;
{
	if (next->object_id == 0) return(-1);
	if (next->group_id == 0) return(-1);

	strcpy(&new_packet[packet_len],next->object_id);
	packet_len += strlen(next->object_id) + 1;
	strcpy(&new_packet[packet_len],next->group_id);
	packet_len += strlen(next->group_id) + 1;
	return(0);
}

static int cDPIset(set)
struct dpi_set_packet *set;
{
	int len;
	unsigned long tmp_int, tmp_int2;
	struct dpi_set_packet *next;

	if (set == 0) return(-1);
	next = set;
	while (next) {
		if (next->object_id == 0) return(-1);
		if ((next->value == 0) && (next->value_len != 0)) return(-1);

		strcpy(&new_packet[packet_len],next->object_id);
		packet_len += strlen(next->object_id) + 1;
		new_packet[packet_len++] = next->type;
		len = next->value_len >> 8;
		new_packet[packet_len++] = len & 0xff;
		new_packet[packet_len++] = next->value_len & 0xff;
		if (next->type & ~SNMP_TYPE_MASK) { /* an integer type */
			if (next->value_len != 4) {
				fprintf(stderr,
						"cDPIset: Invalid packet, integer length %d\n",
						next->value_len);
				return(0);
			}
			bcopy(next->value,&tmp_int,next->value_len);
			if (next->type != SNMP_TYPE_INTERNET) {
				/* Convert integer to network byte order */
				tmp_int2 = htonl(tmp_int);
				bcopy(&tmp_int2,&new_packet[packet_len],next->value_len);
			} else {
				/* By definition IP address is in network byte order */
				bcopy(&tmp_int,&new_packet[packet_len],next->value_len);
/*				tmp_int2 = htonl(tmp_int);
 *				bcopy(&tmp_int2,&new_packet[packet_len],next->value_len);
 */			}
		} else /* an octet string */
			bcopy(next->value,&new_packet[packet_len],next->value_len);
		packet_len += next->value_len;
		next = next->next;
	} /* end while (next) */
	return(0);
}

static int cDPIresponse(resp)
struct dpi_resp_packet *resp;
{
	int rc;

	if (resp == 0) return(-1);

	new_packet[packet_len++] = resp->ret_code;
	if (resp->ret_data != 0) {
		rc = cDPIset(resp->ret_data);
	} else rc = 0;
	return(rc);
}

static int cDPItrap(trap)
struct dpi_trap_packet *trap;
{
	int rc;
	unsigned long g,s;

	/* this was V1.0 code, generic and specific were 1 character */
	/*  new_packet[packet_len++] = trap->generic;
	 *  new_packet[packet_len++] = trap->specific;
	 */

	/* this is V1.1 code, generic and specific are 4 byte intergers */
/*	bcopy(&trap->generic,&new_packet[packet_len],4);
 *	bcopy(&trap->specific,&new_packet[packet_len+4],4);
 */
	g = htonl(trap->generic);
	s = htonl(trap->specific);
	bcopy(&g,&new_packet[packet_len],4);
	bcopy(&s,&new_packet[packet_len+4],4);
	packet_len += 8;
	if (trap->enterprise) {
		strcpy(&new_packet[packet_len],trap->enterprise);
		packet_len += strlen(trap->enterprise) + 1;
	} else {
		new_packet[packet_len++] = 0;
	}

	/* the rest is the same for V1.0 and V1.1 */
	if (trap->info != 0) rc = cDPIset(trap->info);
	else rc = 0;
	return(rc);
}

unsigned char *mkMIBquery(cmd, oid_name, group_oid, type, len, value)
int cmd;
char *oid_name, *group_oid;
int type, len;
char *value;
{
	struct snmp_dpi_hdr *hdr;
	unsigned char *cp;

	hdr = mkDPIhdr(cmd);
	if (hdr == 0) return(0);
	switch (hdr->packet_type) {
	case SNMP_DPI_GET:
	case SNMP_DPI_REGISTER:
		hdr->packet_body.dpi_get = mkDPIget(oid_name);
		break;
	case SNMP_DPI_GET_NEXT:
		hdr->packet_body.dpi_next = mkDPInext(oid_name,group_oid);
		break;
	case SNMP_DPI_SET:
		hdr->packet_body.dpi_set =
			mkDPIset(oid_name,type,len,value);
		if (hdr->packet_body.dpi_next == NULL) {
			fDPIparse(hdr); /* mkDPIset failed */
			return(0);
		}
		break;
	default: /* should not occur */
		fprintf(stderr,"mkMIBquery: Unknown packet_type: %d\n",
				hdr->packet_type);
		fDPIparse(hdr);
		return(0);
		break;
	}
	cp = cDPIpacket(hdr);
	fDPIparse(hdr);
	return(cp);
}

unsigned char *mkDPIregister(oid_name)
char *oid_name;
{
	return(mkMIBquery(SNMP_DPI_REGISTER,oid_name,NULL,0,0,NULL));
}

unsigned char *mkDPIresponse(ret_code,value_list)
int ret_code;
struct dpi_set_packet *value_list;
{
	struct snmp_dpi_hdr *hdr;
	struct dpi_resp_packet *resp;
	unsigned char *cp;

	hdr = mkDPIhdr(SNMP_DPI_RESPONSE);
	resp = (struct dpi_resp_packet *)
			calloc(1, sizeof(struct dpi_resp_packet));
	if (resp == 0) {
		free(hdr);
		return(0);
	}
	hdr->packet_body.dpi_response = resp;
	resp->ret_code = ret_code;
	resp->ret_data = value_list;
	cp = cDPIpacket(hdr);
	fDPIparse(hdr);
	return(cp);
}

unsigned char *mkDPItrap(generic,specific,value_list)
int generic, specific;
struct dpi_set_packet *value_list;
{
	struct snmp_dpi_hdr *hdr;
	struct dpi_trap_packet *trap;
	unsigned char *cp;

	hdr = mkDPIhdr(SNMP_DPI_TRAP);
	if (hdr == 0) return(hdr);
	trap = (struct dpi_trap_packet *)
			calloc(1, sizeof(struct dpi_trap_packet));
	if (trap == 0) {
		free(hdr);
		return(0);
	}
	hdr->packet_body.dpi_trap = trap;
	trap->generic = generic;
	trap->specific = specific;
	trap->enterprise = 0;
	trap->info = value_list;
	cp = cDPIpacket(hdr);
	fDPIparse(hdr);
	return(cp);
}

unsigned char *mkDPItrape(generic,specific,value_list,enterprise)
long int generic, specific;
struct dpi_set_packet *value_list;
char *enterprise;
{
	struct snmp_dpi_hdr *hdr;
	struct dpi_trap_packet *trap;
	unsigned char *cp;
	int l;

	hdr = mkDPIhdr(SNMP_DPI_TRAP);
	if (hdr == 0) return(0);
	trap = (struct dpi_trap_packet *)
			calloc(1, sizeof(struct dpi_trap_packet));
	if (trap == 0) {
		free(hdr);
		return(0);
	}
	if (enterprise) {
		l = strlen(enterprise) + 1;
		trap->enterprise = (char *) malloc(l);
		if (trap->enterprise == 0) {
			free(hdr);
			return(0);
		}
		strcpy(trap->enterprise,enterprise);
	}
	else trap->enterprise = 0;
	hdr->packet_body.dpi_trap = trap;
	trap->generic = generic;
	trap->specific = specific;
	trap->info = value_list;
	cp = cDPIpacket(hdr);
	fDPIparse(hdr);
	return(cp);
}

static struct snmp_dpi_hdr *mkDPIhdr(type)
int type;
{
	struct snmp_dpi_hdr *hdr;

	hdr = (struct snmp_dpi_hdr *) 
			calloc(1, sizeof(struct snmp_dpi_hdr));
	if (hdr == 0) return(0);
	hdr->proto_major = SNMP_DPI_PROTOCOL;
	hdr->proto_minor = SNMP_DPI_VERSION;
	hdr->proto_release = SNMP_DPI_RELEASE;
	hdr->packet_type = type;
	return(hdr);
}

static struct dpi_get_packet *mkDPIget(oid_name)
char *oid_name;
{
	struct dpi_get_packet *get;
	int l;

	get = (struct dpi_get_packet *) 
			calloc(1, sizeof(struct dpi_get_packet));
	if (get == 0) return(0);

	l = strlen(oid_name) + 1;
	get->object_id = (char *) malloc(l);
	strcpy(get->object_id,oid_name);
	return(get);
}

static struct dpi_next_packet *mkDPInext(oid_name,group_oid)
char *oid_name;
char *group_oid;
{
	struct dpi_next_packet *next;
	int l;

	next = (struct dpi_next_packet *) 
			calloc(1, sizeof(struct dpi_next_packet));
	if (next == 0) return(0);
	l = strlen(oid_name) + 1;
	next->object_id = (char *) malloc(l);
	strcpy(next->object_id,oid_name);
	l = strlen(group_oid) + 1;
	next->group_id = (char *) malloc(l);
	strcpy(next->group_id,group_oid);
	return(next);
}

struct dpi_set_packet *mkDPIset(oid_name,type,len,value)
char *oid_name;
int type;
int len;
char *value;
{
	struct dpi_set_packet *set;
	int l;

	switch (type) {  /* validate type */
	case SNMP_TYPE_TEXT:          /* textual representation */
	case SNMP_TYPE_NUMBER:        /* number */
	case SNMP_TYPE_STRING:        /* text string */
	case SNMP_TYPE_OBJECT:        /* object identifier */
	case SNMP_TYPE_EMPTY:         /* no value */
	case SNMP_TYPE_INTERNET:      /* internet address */
	case SNMP_TYPE_COUNTER:       /* counter */
	case SNMP_TYPE_GAUGE:         /* gauge */
	case SNMP_TYPE_TICKS:         /* time ticks */
		break;
	default:
		fprintf(stderr,"mkDPIset: Unknown SNMP_TYPE: %d\n", type);
		return(0);
		break;
	}

	set = (struct dpi_set_packet *) 
			calloc(1, sizeof(struct dpi_set_packet));
	if (set == 0) return(0);
	
	l = strlen(oid_name) + 1;
	set->object_id = (char *) malloc(l);
	strcpy(set->object_id,oid_name);
	set->type = type;
	set->value_len = len;
	set->value = (char *) malloc(len);
	bcopy(value,set->value,len);
	set->next = 0;
	return(set);
}

struct dpi_set_packet *mkDPIlist(packet,oid_name,type,len,value)
struct dpi_set_packet *packet;
char *oid_name;
int type;
int len;
char *value;
{
	struct dpi_set_packet *next = NULL;
	struct dpi_set_packet *set = NULL;
	int l;

	switch (type) {  /* validate type */
	case SNMP_TYPE_TEXT:          /* textual representation */
	case SNMP_TYPE_NUMBER:        /* number */
	case SNMP_TYPE_STRING:        /* text string */
	case SNMP_TYPE_OBJECT:        /* object identifier */
	case SNMP_TYPE_EMPTY:         /* no value */
	case SNMP_TYPE_INTERNET:      /* internet address */
	case SNMP_TYPE_COUNTER:       /* counter */
	case SNMP_TYPE_GAUGE:         /* gauge */
	case SNMP_TYPE_TICKS:         /* time ticks */
		break;
	default:
		fprintf(stderr,"mkDPIlist: Unknown SNMP_TYPE: %d\n", type);
		return(0);
		break;
	}

	set = (struct dpi_set_packet *)
			calloc(1, sizeof(struct dpi_set_packet));
	if (set == 0) return(0);

	l = strlen(oid_name) + 1;
	set->object_id = (char *) malloc(l);
	strcpy(set->object_id,oid_name);
	set->type = type;
	set->value_len = len;
	set->value = (char *) malloc(len);
	set->next = NULL;
	bcopy(value,set->value,len);
	if (packet) {
		next = packet;
		while (next->next) next = next->next;
		next->next = set;
		return(packet);
	}
	return (set);
}

/*
 * Function: int_val
 *
 * Inputs: bytes - a character string containing a number.
 *
 * Outputs: none
 *
 * Return: The value of the 4 byte number.  The equation equals:
 *    int_val = bytes[0] * 2 ^ 24  + bytes[1] * 2 ^ 16 +
 *              bytes[2] * 2 ^ 8 + bytes[3]
 *
 */
long int_val(bytes)
unsigned char *bytes;
{
	register int i;
	unsigned long sum;

	sum = 0;
	for(i=0;i<4;i++) {
		sum <<= 8;
		sum |= *(bytes++);
	}
	return(sum);
}

