/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/fddi.c
 */

/* INCLUDES */
#include "snmpd.h"
#include "interfaces.h"
#include "fddi.h"
#include <isode/snmp/io.h>

extern struct interface *ifs;
extern int SNMP_PDU_variable_position;
extern int fdditimeout;
extern int open_device();
extern void close_device();

int do_fddi();
void init_fddi();
void free_SMT_frame();
struct interface *get_fddi();
void get_fdditokens();

#ifdef DEBUG 
void print_fddi();
void print_fddi_frame();
#endif /* DEBUG */

int smtnumber = 0;
int macnumber = 0;
int portnumber = 0;
int pathnumber = 0;

uchar SetCount[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

SMT_impl *SMT_list = NULL;

/* This is a dummy is structure to place in the timer list for fddi config */
struct interface isdummy;
struct interface *isdummyptr = &isdummy;

#define PMFGET	0
#define PMFSET	1

#define RESEND  5

/* SMT STUFF */
#define MIBSMTNUMBER			  1

#define MIBSMTINDEX			  2
#define MIBSMTSTATIONID			  3
#define MIBSMTOPVERSIONID		  4
#define MIBSMTHIVERSIONID		  5
#define MIBSMTLOVERSIONID		  6
#define MIBSMTUSERDATA			  7
#define MIBSMTMIBVERSIONID		  8
#define MIBSMTMACCTS			  9
#define MIBSMTNONMASTERCT		 10
#define MIBSMTMASTERCT			 11
#define MIBSMTAVAILABLEPATHS		 12
#define MIBSMTCONFIGCAPABILITIES	 13
#define MIBSMTCONFIGPOLICY		 14
#define MIBSMTCONNECTIONPOLICY		 15
#define MIBSMTTNOTIFY			 16
#define MIBSMTSTATRPTPOLICY		 17
#define MIBSMTTRACEMAXEXPIRATION	 18
#define MIBSMTBYPASSPRESENT		 19
#define MIBSMTECMSTATE			 20
#define MIBSMTCFSTATE			 21
#define MIBSMTREMOTEDISCONNECTFLAG	 22
#define MIBSMTSTATIONSTATUS		 23
#define MIBSMTPEERWRAPFLAG		 24
#define MIBSMTTIMESTAMP			 25
#define MIBSMTTRANSITIONTIMESTAMP	 26
#define MIBSMTSTATIONACTION		 27
 
/* MAC STUFF */
#define MIBMACNUMBER			 28

#define MIBMACSMTINDEX			 29
#define MIBMACINDEX			 30
#define MIBMACIFINDEX			 31
#define MIBMACFRAMESTATUSFUNCTIONS	 32
#define MIBMACTMAXCAPABILITY		 33
#define MIBMACTVXCAPABILITY		 34
#define MIBMACAVAILABLEPATHS		 35
#define MIBMACCURRENTPATH		 36
#define MIBMACUPSTREAMNBR		 37
#define MIBMACDOWNSTREAMNBR		 38
#define MIBMACOLDUPSTREAMNBR		 39
#define MIBMACOLDDOWNSTREAMNBR		 40
#define MIBMACDUPADDRESSTEST		 41
#define MIBMACREQUESTEDPATHS		 42
#define MIBMACDOWNSTREAMPORTTYPE	 43
#define MIBMACSMTADDRESS		 44
#define MIBMACTREQ			 45
#define MIBMACTNEG			 46
#define MIBMACTMAX			 47
#define MIBMACTVXVALUE			 48
#define MIBMACFRAMECTS			 49
#define MIBMACCOPIEDCTS			 50
#define MIBMACTRANSMITCTS		 51
#define MIBMACERRORCTS			 52
#define MIBMACLOSTCTS			 53
#define MIBMACFRAMEERRORTHRESHOLD	 54
#define MIBMACFRAMEERRORRATIO		 55
#define MIBMACRMTSTATE			 56
#define MIBMACDAFLAG			 57
#define MIBMACUNADAFLAG			 58
#define MIBMACFRAMEERRORFLAG		 59
#define MIBMACMAUNITDATAAVAILABLE	 60
#define MIBMACHARDWAREPRESENT		 61
#define MIBMACMAUNITDATAENABLE		 62
 
#define MIBMACTOKENCTS			 63
#define MIBMACTVXEXPIREDCTS		 64
#define MIBMACNOTCOPIEDCTS		 65
#define MIBMACLATECTS			 66
#define MIBMACRINGOPCTS			 67
#define MIBMACNOTCOPIEDRATIO		 68
#define MIBMACNOTCOPIEDFLAG		 69
#define MIBMACNOTCOPIEDTHRESHOLD	 70
 
/* PATH STUFF */
#define MIBPATHNUMBER			 71

#define MIBPATHSMTINDEX			 72
#define MIBPATHINDEX			 73
#define MIBPATHTVXLOWERBOUND		 74
#define MIBPATHTMAXLOWERBOUND		 75
#define MIBPATHMAXTREQ			 76
 
#define MIBPATHCONFIGSMTINDEX		 77
#define MIBPATHCONFIGPATHINDEX		 78
#define MIBPATHCONFIGTOKENORDER		 79
#define MIBPATHCONFIGRESOURCETYPE	 80
#define MIBPATHCONFIGRESOURCEINDEX	 81
#define MIBPATHCONFIGCURRENTPATH	 82

/* PORT STUFF */
#define MIBPORTNUMBER			 83

#define MIBPORTSMTINDEX			 84
#define MIBPORTINDEX			 85
#define MIBPORTMYTYPE			 86
#define MIBPORTNEIGHBORTYPE		 87
#define MIBPORTCONNECTIONPOLICIES	 88
#define MIBPORTMACINDICATED		 89
#define MIBPORTCURRENTPATH		 90
#define MIBPORTREQUESTEDPATHS		 91
#define MIBPORTMACPLACEMENT		 92
#define MIBPORTAVAILABLEPATHS		 93
#define MIBPORTPMDCLASS			 94
#define MIBPORTCONNECTIONCAPABILITIES	 95
#define MIBPORTBSFLAG			 96
#define MIBPORTLCTFAILCTS		 97
#define MIBPORTLERESTIMATE		 98
#define MIBPORTLEMREJECTCTS		 99
#define MIBPORTLEMCTS			100
#define MIBPORTLERCUTOFF		101
#define MIBPORTLERALARM			102
#define MIBPORTCONNECTSTATE		103
#define MIBPORTPCMSTATE			104
#define MIBPORTPCWITHHOLD		105
#define MIBPORTLERFLAG			106
#define MIBPORTHARDWAREPRESENT		107
#define MIBPORTACTION			108


/* ACTUAL FDDI SMT PACKET BYTES FOR EACH CODE */
#define PMFMIBSMTSTATIONID		0x100B
#define PMFMIBSMTOPVERSIONID		0x100D
#define PMFMIBSMTHIVERSIONID		0x100E
#define PMFMIBSMTLOVERSIONID		0x100F
#define PMFMIBSMTUSERDATA		0x1011
#define PMFMIBSMTMIBVERSIONID		0x1012
#define PMFMIBSMTMACCTS			0x1015
#define PMFMIBSMTNONMASTERCT		0x1016
#define PMFMIBSMTMASTERCT		0x1017 
#define PMFMIBSMTAVAILABLEPATHS		0x1018
#define PMFMIBSMTCONFIGCAPABILITIES	0x1019 
#define PMFMIBSMTCONFIGPOLICY		0x101A
#define PMFMIBSMTCONNECTIONPOLICY	0x101B
#define PMFMIBSMTTNOTIFY		0x101D
#define PMFMIBSMTSTATRPTPOLICY		0x101E
#define PMFMIBSMTTRACEMAXEXPIRATION	0x101F
#define PMFMIBSMTBYPASSPRESENT		0x1022
#define PMFMIBSMTECMSTATE		0x1029
#define PMFMIBSMTCFSTATE		0x102A
#define PMFMIBSMTREMOTEDISCONNECTFLAG	0x102C
#define PMFMIBSMTSTATIONSTATUS		0x102D
#define PMFMIBSMTPEERWRAPFLAG		0x102E
#define PMFMIBSMTTIMESTAMP		0x1033
#define PMFMIBSMTTRANSITIONTIMESTAMP	0x1034
#define PMFMIBSMTSTATIONACTION		0x103C
 
/* MAC STUFF */
#define PMFMIBMACINDEX			0x2022
#define PMFMIBMACFRAMESTATUSFUNCTIONS	0x200B
#define PMFMIBMACTMAXCAPABILITY		0x200D
#define PMFMIBMACTVXCAPABILITY		0x200E
#define PMFMIBMACAVAILABLEPATHS		0x2016
#define PMFMIBMACCURRENTPATH		0x2017
#define PMFMIBMACUPSTREAMNBR		0x2018
#define PMFMIBMACDOWNSTREAMNBR		0x2019
#define PMFMIBMACOLDUPSTREAMNBR		0x201A
#define PMFMIBMACOLDDOWNSTREAMNBR	0x201B
#define PMFMIBMACDUPADDRESSTEST		0x201D
#define PMFMIBMACREQUESTEDPATHS		0x2020
#define PMFMIBMACDOWNSTREAMPORTTYPE	0x2021
#define PMFMIBMACSMTADDRESS		0x2029
#define PMFMIBMACTREQ			0x2033
#define PMFMIBMACTNEG			0x2034
#define PMFMIBMACTMAX			0x2035
#define PMFMIBMACTVXVALUE		0x2036
#define PMFMIBMACFRAMECTS		0x2047
#define PMFMIBMACCOPIEDCTS		0x2048
#define PMFMIBMACTRANSMITCTS		0x2049
#define PMFMIBMACERRORCTS		0x2051
#define PMFMIBMACLOSTCTS		0x2052
#define PMFMIBMACFRAMEERRORTHRESHOLD	0x205F
#define PMFMIBMACFRAMEERRORRATIO	0x2060
#define PMFMIBMACRMTSTATE		0x206F
#define PMFMIBMACDAFLAG			0x2070
#define PMFMIBMACUNADAFLAG		0x2071
#define PMFMIBMACFRAMEERRORFLAG		0x2072
#define PMFMIBMACMAUNITDATAAVAILABLE	0x2074
#define PMFMIBMACHARDWAREPRESENT	0x2075
#define PMFMIBMACMAUNITDATAENABLE	0x2076
 
#define PMFMIBMACTOKENCTS		0x204A
#define PMFMIBMACTVXEXPIREDCTS		0x2053
#define PMFMIBMACNOTCOPIEDCTS		0x2054
#define PMFMIBMACLATECTS		0x2055
#define PMFMIBMACRINGOPCTS		0x2056
#define PMFMIBMACNOTCOPIEDRATIO		0x2069
#define PMFMIBMACNOTCOPIEDFLAG		0x2073
#define PMFMIBMACNOTCOPIEDTHRESHOLD	0x2067
 
/* PATH STUFF */
#define PMFMIBPATHINDEX			0x320B
#define PMFMIBPATHTVXLOWERBOUND		0x3215
#define PMFMIBPATHTMAXLOWERBOUND	0x3216
#define PMFMIBPATHMAXTREQ		0x3217

#define PMFMIBPATHCONFIGTABLE		0x3212
 
/* PORT STUFF */
#define PMFMIBPORTINDEX			0x401D
#define PMFMIBPORTMYTYPE		0x400C
#define PMFMIBPORTNEIGHBORTYPE		0x400D
#define PMFMIBPORTCONNECTIONPOLICIES	0x400E
#define PMFMIBPORTMACINDICATED		0x400F
#define PMFMIBPORTCURRENTPATH		0x4010
#define PMFMIBPORTREQUESTEDPATHS	0x4011
#define PMFMIBPORTMACPLACEMENT		0x4012
#define PMFMIBPORTAVAILABLEPATHS	0x4013
#define PMFMIBPORTPMDCLASS		0x4016
#define PMFMIBPORTCONNECTIONCAPABILITIES 0x4017
#define PMFMIBPORTBSFLAG		0x4021
#define PMFMIBPORTLCTFAILCTS		0x402A
#define PMFMIBPORTLERESTIMATE		0x4033
#define PMFMIBPORTLEMREJECTCTS		0x4034
#define PMFMIBPORTLEMCTS		0x4035	
#define PMFMIBPORTLERCUTOFF		0x403A
#define PMFMIBPORTLERALARM		0x403B
#define PMFMIBPORTCONNECTSTATE		0x403D
#define PMFMIBPORTPCMSTATE		0x403E
#define PMFMIBPORTPCWITHHOLD		0x403F
#define PMFMIBPORTLERFLAG		0x4040
#define PMFMIBPORTHARDWAREPRESENT	0x4041	
#define PMFMIBPORTACTION		0x4046

#define PMFREASONCODE			0x0012
#define PMFTIMESTAMP			0x1033
#define PMFSETCOUNT			0x1035
#define PMFLASTSETSTATIONID		0x1036
#define PMFMIBMACINDEXES		0x1021
#define PMFMIBPORTINDEXES		0x1020
#define PMFMIBPATHINDEXES		0x320B
#define PMFMIBPATHCONFIGURATION		0x3212

#define USERDATASIZE			32

/*  */

unsigned short in_maclist(macnum, fddinum)
int macnum;
int fddinum;
{
    int i;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered in_maclist");
#endif /* DEBUG */

    for (i = 0; i < SMT_list[fddinum - 1].nummacs; i++)
	if (macnum == SMT_list[fddinum - 1].macidnumbers[i])
	    break;

    if (i == SMT_list[fddinum - 1].nummacs)
	return(0);

    return(i + 1);
}


unsigned short next_macnum(macnum, fddinum)
int macnum;
int fddinum;
{
    int i;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered next_macnum");
#endif /* DEBUG */

    for (i = 0; i < SMT_list[fddinum - 1].nummacs; i++)
	if (macnum < SMT_list[fddinum - 1].macidnumbers[i])
	    break;

    if (i == SMT_list[fddinum - 1].nummacs)
	return(0);

    return(SMT_list[fddinum - 1].macidnumbers[i]);
}


unsigned short in_portlist(portnum, fddinum)
int portnum;
int fddinum;
{
    int i;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered in_portlist");
#endif /* DEBUG */

    for (i = 0; i < SMT_list[fddinum - 1].numports; i++)
	if (portnum == SMT_list[fddinum - 1].portidnumbers[i])
	    break;

    if (i == SMT_list[fddinum - 1].numports)
	return(0);

    return(i + 1);
}


unsigned short next_portnum(portnum, fddinum)
int portnum;
int fddinum;
{
    int i;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered next_portnum");
#endif /* DEBUG */

    for (i = 0; i < SMT_list[fddinum - 1].numports; i++)
	if (portnum < SMT_list[fddinum - 1].portidnumbers[i])
	    break;

    if (i == SMT_list[fddinum - 1].numports)
	return(0);

    return(SMT_list[fddinum - 1].portidnumbers[i]);
}


unsigned short in_pathlist(pathnum, fddinum)
int pathnum;
int fddinum;
{
    int i;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered in_pathlist");
#endif /* DEBUG */

    for (i = 0; i < SMT_list[fddinum - 1].numpaths; i++)
	if (pathnum == SMT_list[fddinum - 1].pathidnumbers[i])
	    break;

    if (i == SMT_list[fddinum - 1].numpaths)
	return(0);

    return(i + 1);
}


unsigned short next_pathnum(pathnum, fddinum)
int pathnum;
int fddinum;
{
    int i;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered next_pathnum");
#endif /* DEBUG */

    for (i = 0; i < SMT_list[fddinum - 1].numpaths; i++)
	if (pathnum < SMT_list[fddinum - 1].pathidnumbers[i])
	    break;

    if (i == SMT_list[fddinum - 1].numpaths)
	return(0);

    return(SMT_list[fddinum - 1].pathidnumbers[i]);
}


unsigned short in_tokenlist(tokennum, pathnum, fddinum)
int tokennum;
int pathnum;
int fddinum;
{
    token_path *ptr = SMT_list[fddinum - 1].token_paths;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered in_tokenlist");
#endif /* DEBUG */

    while (ptr)
    {
	if (ptr -> path_id > pathnum)
	    return(0);
	if ((ptr -> path_id == pathnum) && (ptr -> token_num == tokennum))
	    return(1);
        ptr = ptr -> next;
    }

    return(0);
}


unsigned short next_token(tokennum, pathnum, fddinum)
int tokennum;
int pathnum;
int fddinum;
{
    token_path *ptr = SMT_list[fddinum - 1].token_paths;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered next_token");
#endif /* DEBUG */

    while (ptr)
    {
	if (ptr -> path_id > pathnum)
	    return(0);

	if (ptr -> path_id == pathnum && ptr -> token_num == tokennum)
	    break;

	ptr = ptr -> next;
    }

    if (ptr && ptr -> next && ptr -> next -> path_id == pathnum)
        return(ptr -> next -> token_num);
    return(0);
}


token_path *get_tokenptr(tokennum, pathnum, fddinum)
int tokennum;
int pathnum;
int fddinum;
{
    token_path *ptr = SMT_list[fddinum - 1].token_paths;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered get_tokenptr");
#endif /* DEBUG */

    while (ptr)
    {
	if (ptr -> path_id > pathnum)
	    return(NULL);

	if (ptr -> path_id == pathnum && ptr -> token_num == tokennum)
	    break;

	ptr = ptr -> next;
    }

    return(ptr);
}


/*  */

static int o_fddismt (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar = (int) ot -> ot_info;
    int fddinum;
    int status = NOTOK;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered o_fddismt");
#endif /* DEBUG */

    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1)
	      return int_SNMP_error__status_noSuchName;
	    fddinum = oid -> oid_elements[oid -> oid_nelem - 1];
	    if ((fddinum <= 0) || (fddinum > smtnumber))
	      return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
            if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	      OID     new;

	      fddinum = 1;

	      if (fddinum > smtnumber)
		return NOTOK;

	      if ((new = oid_extend (oid, 1)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 1] = fddinum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
	    else {
              int     i = ot -> ot_name -> oid_nelem;
	      OID new;

	      if ((i >= oid -> oid_nelem) ||
		  ((fddinum = oid -> oid_elements[i]) <= 0)) {
		fddinum = 1;
	      }
	      else fddinum++;

	      if (fddinum > smtnumber)
	        return NOTOK;
	
	      if ((new = oid_extend (ot -> ot_name, 1)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 1] = fddinum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
            break;

	default:
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_1,
		   "%s: invalid operation: %d"), "o_fddismt",
		   offset);
	    return int_SNMP_error__status_genErr;
    }

o_smtstartover: ;
    /* Get the variables that are held internally in SNMP */
    switch (ifvar) {
	case MIBSMTINDEX:
		return o_integer(oi, v, fddinum);
		break;
	case MIBSMTSTATIONACTION: /* Always returns other (1) */
		return o_integer(oi, v, 1);
		break;
	case MIBSMTMACCTS:
		return o_integer(oi, v, SMT_list[fddinum - 1].nummacs);
		break;
    }

    /* From here on, the variables are on the FDDI device */
    /* So, create a header and a variable for the packet */
    if ((SMT_list[fddinum - 1].msgtosend == NULL) &&
        (make_SMT_header(&SMT_list[fddinum - 1], PMFGET) == NOTOK))
    {
        if (offset == type_SNMP_PDUs_get__next__request)
            return (NOTOK);
        else return int_SNMP_error__status_noSuchName;
    }

    switch (ifvar) {
	case MIBSMTSTATIONID:
                status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTSTATIONID, -1, OCTETSTRING, 
			     0, 0, PMFGET);
		break;
	case MIBSMTOPVERSIONID:
                status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTOPVERSIONID, -1, INTEGER, 
			     0, 0, PMFGET);
		break;
	case MIBSMTHIVERSIONID:
                status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTHIVERSIONID, -1, INTEGER, 
			     0, 0, PMFGET);
		break;
	case MIBSMTLOVERSIONID:
                status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTLOVERSIONID, -1, INTEGER, 
			     0, 0, PMFGET);
		break;
	case MIBSMTUSERDATA:
                status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTUSERDATA, -1, OCTETSTRING, 
			     0, 0, PMFGET);
		break;
	case MIBSMTMIBVERSIONID:
                status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTMIBVERSIONID, -1, INTEGER, 
			     0, 0, PMFGET);
		break;
	case MIBSMTNONMASTERCT:
                status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTNONMASTERCT, -1, INTEGER, 
			     0, 0, PMFGET);
		break;
	case MIBSMTMASTERCT:
                status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTMASTERCT, -1, INTEGER, 
			     0, 0, PMFGET);
		break;
	case MIBSMTAVAILABLEPATHS: /* This may be constant */
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTAVAILABLEPATHS, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTCONFIGCAPABILITIES:
                status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTCONFIGCAPABILITIES, -1, INTEGER, 
			     0, 0, PMFGET);
		break;
	case MIBSMTCONFIGPOLICY:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTCONFIGPOLICY, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTCONNECTIONPOLICY:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTCONNECTIONPOLICY, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTTNOTIFY:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTTNOTIFY, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTSTATRPTPOLICY:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTSTATRPTPOLICY, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTTRACEMAXEXPIRATION:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTTRACEMAXEXPIRATION, -1, FDDITIMEMILLI, 
			    0, 0, PMFGET);
		break;
	case MIBSMTBYPASSPRESENT:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTBYPASSPRESENT, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTECMSTATE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTECMSTATE, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTCFSTATE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTCFSTATE, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTREMOTEDISCONNECTFLAG:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTREMOTEDISCONNECTFLAG, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTSTATIONSTATUS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTSTATIONSTATUS, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTPEERWRAPFLAG:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTPEERWRAPFLAG, -1, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBSMTTIMESTAMP:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTTIMESTAMP, -1, FDDITIMEMILLI, 
			    0, 0, PMFGET);
		break;
	case MIBSMTTRANSITIONTIMESTAMP:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTTRANSITIONTIMESTAMP, -1, FDDITIMEMILLI, 
			    0, 0, PMFGET);
		break;
    }

    if (status == NOTOK)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
        if (offset == type_SNMP_PDUs_get__next__request)
            return (NOTOK);
        else return int_SNMP_error__status_noSuchName;
    }

    if (do_fddi(&SMT_list[fddinum - 1], oi, v, offset) == NOTOK)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
        if (offset == type_SNMP_PDUs_get__next__request)
	{
	    OID new;

	    fddinum++;

	    if (fddinum > smtnumber)
	        return NOTOK;
	
	    if ((new = oid_extend (ot -> ot_name, 1)) == NULLOID)
		return NOTOK;
            new -> oid_elements[new -> oid_nelem - 1] = fddinum;

	    if (v -> name)
	        free_SMI_ObjectName (v -> name);
	    v -> name = new;

            goto o_smtstartover;
	}
        else return int_SNMP_error__status_noSuchName;
    }

    if (SMT_list[fddinum - 1].msgtosend)
	free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
    SMT_list[fddinum - 1].msgtosend = NULL;

    return int_SNMP_error__status_noError;
}


static int s_fddismt (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    register OS		os = ot -> ot_syntax;
    int ifvar = (int) ot -> ot_info;
    int fddinum;
    int status = int_SNMP_error__status_noError;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered s_fddismt");
#endif /* DEBUG */

    switch (offset) 
    {
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1)
	      return int_SNMP_error__status_noSuchName;
	    fddinum = oid -> oid_elements[oid -> oid_nelem - 1];
	    if ((fddinum <= 0) || (fddinum > smtnumber))
	      return int_SNMP_error__status_noSuchName;
	    break;

        case type_SNMP_PDUs_rollback:
	    return int_SNMP_error__status_noError;

	default:
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_1, 
		   "%s: invalid operation: %d"), "s_fddismt",
		   offset);
	    return int_SNMP_error__status_genErr;
    }

    /* From here on, the variables are on the FDDI device */
    /* So, create a header and a variable for the packet */
    if ((offset == type_SNMP_PDUs_commit) &&
	(SMT_list[fddinum - 1].msgtosend == NULL) &&
        (make_SMT_header(&SMT_list[fddinum - 1], PMFSET) == NOTOK))
	return int_SNMP_error__status_genErr;

    status = (offset == type_SNMP_PDUs_set__request) ? 
	     int_SNMP_error__status_noError : NOTOK;
    switch (ifvar) 
    {
	case MIBSMTUSERDATA:
	    {
		char *strig;
		char strig2[USERDATASIZE] = { 
			 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		if (ot -> ot_save)
		    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
		if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		    return int_SNMP_error__status_badValue;

                if ((strig = qb2str((struct qbuf *) ot -> ot_save)) == NULLCP)
		{
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_2,
		            "GENERR in %s: s_generic failed"), "s_fddismt");
                    if (ot -> ot_save)
		        (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
		    status = int_SNMP_error__status_genErr;
		    break;
		}
                if (ot -> ot_save)
		    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;

		if (strlen(strig) > USERDATASIZE)
		{
		    status = int_SNMP_error__status_badValue;
		    free(strig);
		    break;
		}
		bcopy(strig, strig2, strlen(strig));

		if (offset == type_SNMP_PDUs_commit)
               	    status =  add_SMT_variable(&SMT_list[fddinum - 1], 
			     PMFMIBSMTUSERDATA, -1, OCTETSTRING, 
			     strig2, USERDATASIZE, PMFSET);

		free(strig);
	    }
	    break;
	case MIBSMTCONFIGPOLICY:
	        if ((v -> value -> un.number < 0) || 
		    (v -> value -> un.number > 1))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
		    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBSMTCONFIGPOLICY, -1, INTEGER, 
				v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBSMTCONNECTIONPOLICY:
	        if ((v -> value -> un.number < 32768) || 
		    (v -> value -> un.number > 65535))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBSMTCONNECTIONPOLICY, -1, INTEGER, 
				v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBSMTTNOTIFY:
	        if ((v -> value -> un.number < 2) || 
		    (v -> value -> un.number > 30))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBSMTTNOTIFY, -1, INTEGER, 
				v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBSMTSTATRPTPOLICY:
	        if ((v -> value -> un.number < 1) || 
		    (v -> value -> un.number > 2))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
		{
		    int num = (v -> value -> un.number == 1) ? 1 : 0;

                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBSMTSTATRPTPOLICY, -1, INTEGER, 
				num, sizeof(int), PMFSET);
		}
		break;
	case MIBSMTTRACEMAXEXPIRATION:
		if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBSMTTRACEMAXEXPIRATION, -1, FDDITIMEMILLI, 
			    v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBSMTSTATIONACTION:
	        if ((v -> value -> un.number < 2) || 
		    (v -> value -> un.number > 8))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBSMTSTATIONACTION, -1, INTEGER, 
				v->value->un.number - 2, sizeof(int), PMFSET);
		break;
    }

    if (status == NOTOK || status == int_SNMP_error__status_badValue)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
	if (status == NOTOK)
	    status = int_SNMP_error__status_genErr;
        return status;
    }

    if (offset == type_SNMP_PDUs_commit)
    {
	if (do_fddi(&SMT_list[fddinum - 1], oi, v, offset) == NOTOK)
	{
	    if (SMT_list[fddinum - 1].msgtosend)
		free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	    SMT_list[fddinum - 1].msgtosend = NULL;
	    return int_SNMP_error__status_noSuchName;
	}

	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
    }

    return int_SNMP_error__status_noError;
}


static int o_fddimac (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar = (int) ot -> ot_info;
    int fddinum;
    int macnum;
    int status = NOTOK;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered o_fddimac");
#endif /* DEBUG */

    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 2)
	      return int_SNMP_error__status_noSuchName;
	    fddinum = oid -> oid_elements[oid -> oid_nelem - 2];
	    if ((fddinum <= 0) || (fddinum > smtnumber))
	      return int_SNMP_error__status_noSuchName;
	    macnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    if (!in_maclist(macnum, fddinum))
	      return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
            if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	      OID     new;

	      fddinum = 1;
	      if (fddinum > smtnumber)
		return NOTOK;
	      while (SMT_list[fddinum - 1].nummacs == 0)
	      {
	          fddinum++;
		  if (fddinum > smtnumber)
	      	      return NOTOK;
	      }
	      macnum = SMT_list[fddinum - 1].macidnumbers[0];

	      if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 2] = fddinum;
              new -> oid_elements[new -> oid_nelem - 1] = macnum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
	    else {
              int     i = ot -> ot_name -> oid_nelem;
	      OID new;

	      if ((i >= oid -> oid_nelem) || 
		  ((fddinum = oid -> oid_elements[i]) <= 0)) {
		fddinum = 1;
		macnum = -1;
	        while ((fddinum <= smtnumber) && (macnum == -1)) {
	          if (SMT_list[fddinum - 1].nummacs != 0)
	            macnum = SMT_list[fddinum - 1].macidnumbers[0];
		  else {
		    fddinum++;
		    if (fddinum > smtnumber)
		      return NOTOK;
                  }
		}
		if (macnum == -1) return NOTOK;
	      }
	      else {
		if (fddinum > smtnumber)
		  return NOTOK;
                if (i + 2 != oid -> oid_nelem) {
		  macnum = -1;
	          while ((fddinum <= smtnumber) && (macnum == -1)) {
	            if (SMT_list[fddinum - 1].nummacs != 0)
	              macnum = SMT_list[fddinum - 1].macidnumbers[0];
		    else {
		      fddinum++;
		      if (fddinum > smtnumber)
		        return NOTOK;
                    }
		  }
		  if (macnum == -1) return NOTOK;
                }
                else {
		  macnum = oid -> oid_elements[i + 1];

		  if ((macnum = next_macnum(macnum, fddinum)) == 0)
		  {
                    fddinum++;
		    if (fddinum > smtnumber)
		      return NOTOK;
		    macnum = -1;
	            while ((fddinum <= smtnumber) && (macnum == -1)) {
	              if (SMT_list[fddinum - 1].nummacs != 0)
	                macnum = SMT_list[fddinum - 1].macidnumbers[0];
		      else {
		        fddinum++;
		        if (fddinum > smtnumber)
		          return NOTOK;
                      }
		    }
		    if (macnum == -1) return NOTOK;
		  }
		}
              }

	      if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 2] = fddinum;
              new -> oid_elements[new -> oid_nelem - 1] = macnum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
            break;

	default:
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_1,
		   "%s: invalid operation: %d"), "o_fddimac",
		   offset);
	    return int_SNMP_error__status_genErr;
    }

o_macstartover: ;
    switch (ifvar) {
	case MIBMACSMTINDEX:
		return o_integer(oi, v, fddinum);
		break;
	case MIBMACINDEX:
		return o_integer(oi, v, macnum);
		break;
	case MIBMACIFINDEX:
		return o_integer(oi, v, SMT_list[fddinum-1].ifentry->ifn_index);
		break;
    }

    /* From here on, the variables are on the FDDI device */
    /* So, create a header and a variable for the packet */
    if ((SMT_list[fddinum - 1].msgtosend == NULL) &&
        (make_SMT_header(&SMT_list[fddinum - 1], PMFGET) == NOTOK))
    {
        if (offset == type_SNMP_PDUs_get__next__request)
            return (NOTOK);
        else return int_SNMP_error__status_noSuchName;
    }

    switch (ifvar) {
	case MIBMACFRAMESTATUSFUNCTIONS: 
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACFRAMESTATUSFUNCTIONS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACTMAXCAPABILITY:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACTMAXCAPABILITY, macnum, FDDITIMENANO, 
			    0, 0, PMFGET);
		break;
	case MIBMACTVXCAPABILITY:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACTVXCAPABILITY, macnum, FDDITIMENANO, 
			    0, 0, PMFGET);
		break;
	case MIBMACAVAILABLEPATHS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACAVAILABLEPATHS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACCURRENTPATH:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACCURRENTPATH, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACUPSTREAMNBR:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACUPSTREAMNBR, macnum, OCTETSTRING, 
			    0, 0, PMFGET);
		break;
	case MIBMACDOWNSTREAMNBR:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACDOWNSTREAMNBR, macnum, OCTETSTRING, 
			    0, 0, PMFGET);
		break;
	case MIBMACOLDUPSTREAMNBR:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACOLDUPSTREAMNBR, macnum, OCTETSTRING, 
			    0, 0, PMFGET);
		break;
	case MIBMACOLDDOWNSTREAMNBR:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACOLDDOWNSTREAMNBR, macnum, OCTETSTRING, 
			    0, 0, PMFGET);
		break;
	case MIBMACDUPADDRESSTEST:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACDUPADDRESSTEST, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACREQUESTEDPATHS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACREQUESTEDPATHS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACDOWNSTREAMPORTTYPE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACDOWNSTREAMPORTTYPE, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACSMTADDRESS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACSMTADDRESS, macnum, OCTETSTRING, 
			    0, 0, PMFGET);
		break;
	case MIBMACTREQ:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACTREQ, macnum, FDDITIMENANO, 
			    0, 0, PMFGET);
		break;
	case MIBMACTNEG:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACTNEG, macnum, FDDITIMENANO, 
			    0, 0, PMFGET);
		break;
	case MIBMACTMAX:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACTMAX, macnum, FDDITIMENANO, 
			    0, 0, PMFGET);
		break;
	case MIBMACTVXVALUE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACTVXVALUE, macnum, FDDITIMENANO, 
			    0, 0, PMFGET);
		break;
	case MIBMACFRAMECTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACFRAMECTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACCOPIEDCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACCOPIEDCTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACTRANSMITCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACTRANSMITCTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACERRORCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACERRORCTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACLOSTCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACLOSTCTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACFRAMEERRORTHRESHOLD:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACFRAMEERRORTHRESHOLD, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACFRAMEERRORRATIO:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACFRAMEERRORRATIO, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACRMTSTATE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACRMTSTATE, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACDAFLAG:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACDAFLAG, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACUNADAFLAG:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACUNADAFLAG, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACFRAMEERRORFLAG:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACFRAMEERRORFLAG, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACMAUNITDATAAVAILABLE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACMAUNITDATAAVAILABLE, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACHARDWAREPRESENT:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACHARDWAREPRESENT, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACMAUNITDATAENABLE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACMAUNITDATAENABLE, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;

	/* This is the beginning of the Mac Counters table */
	/* They are in the same routine, because their indices are the same */
	case MIBMACTOKENCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACTOKENCTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACTVXEXPIREDCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACTVXEXPIREDCTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACNOTCOPIEDCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACNOTCOPIEDCTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACLATECTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACLATECTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACRINGOPCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACRINGOPCTS, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACNOTCOPIEDRATIO:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACNOTCOPIEDRATIO, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACNOTCOPIEDFLAG:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACNOTCOPIEDFLAG, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBMACNOTCOPIEDTHRESHOLD:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBMACNOTCOPIEDTHRESHOLD, macnum, INTEGER, 
			    0, 0, PMFGET);
		break;
    }

    if (status == NOTOK)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
        if (offset == type_SNMP_PDUs_get__next__request)
            return (NOTOK);
        else return int_SNMP_error__status_noSuchName;
    }

    if (do_fddi(&SMT_list[fddinum - 1], oi, v, offset) == NOTOK)
    {
	OID new;

	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
        SMT_list[fddinum - 1].msgtosend = NULL;
        if (offset == type_SNMP_PDUs_get__next__request)
	{
	    if ((macnum = next_macnum(macnum, fddinum)) == 0)
	    {
		fddinum++;
		if (fddinum > smtnumber)
		    return NOTOK;
		macnum = -1;
		while ((fddinum <= smtnumber) && (macnum == -1)) 
		{
		    if (SMT_list[fddinum - 1].nummacs != 0)
			macnum = SMT_list[fddinum - 1].macidnumbers[0];
		    else 
		    {
			fddinum++;
			if (fddinum > smtnumber)
			    return NOTOK;
		    }
		}
		if (macnum == -1) return NOTOK;
	    }

	    if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		return NOTOK;
            new -> oid_elements[new -> oid_nelem - 2] = fddinum;
            new -> oid_elements[new -> oid_nelem - 1] = macnum;

	    if (v -> name)
	        free_SMI_ObjectName (v -> name);
	    v -> name = new;

            goto o_macstartover;
	}
        else return int_SNMP_error__status_noSuchName;
    }

    if (SMT_list[fddinum - 1].msgtosend)
	free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
    SMT_list[fddinum - 1].msgtosend = NULL;

    return int_SNMP_error__status_noError;
}


static int s_fddimac (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int fddinum = 0;
    int macnum = -1;
    int ifvar = (int) ot -> ot_info;
    int status = int_SNMP_error__status_noError;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered s_fddimac");
#endif /* DEBUG */

    switch (offset) 
    {
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 2)
	      return int_SNMP_error__status_noSuchName;
	    fddinum = oid -> oid_elements[oid -> oid_nelem - 2];
	    if ((fddinum <= 0) || (fddinum > smtnumber))
	      return int_SNMP_error__status_noSuchName;
	    macnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    if (!in_maclist(macnum, fddinum))
	      return int_SNMP_error__status_noSuchName;
	    break;

        case type_SNMP_PDUs_rollback:
	    return int_SNMP_error__status_noError;

	default:
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_1,
		   "%s: invalid operation: %d"), "s_fddimac",
		   offset);
	    return int_SNMP_error__status_genErr;
    }

    /* From here on, the variables are on the FDDI device */
    /* So, create a header and a variable for the packet */
    if ((offset == type_SNMP_PDUs_commit) &&
	(SMT_list[fddinum - 1].msgtosend == NULL) &&
        (make_SMT_header(&SMT_list[fddinum - 1], PMFSET) == NOTOK))
	return int_SNMP_error__status_genErr;

    status = (offset == type_SNMP_PDUs_set__request) ? 
	     int_SNMP_error__status_noError : NOTOK;
    switch (ifvar) 
    {
	case MIBMACREQUESTEDPATHS:
		if ((v -> value -> un.number < 0) || 
		    (v -> value -> un.number > 255))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBMACREQUESTEDPATHS, macnum, INTEGER, 
				v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBMACFRAMEERRORTHRESHOLD:
		if ((v -> value -> un.number < 0) || 
		    (v -> value -> un.number > 65535))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBMACFRAMEERRORTHRESHOLD, macnum, INTEGER, 
				v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBMACMAUNITDATAENABLE:
		if ((v -> value -> un.number < 1) || 
		    (v -> value -> un.number > 2))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
		{
		    int num = (v -> value -> un.number == 1) ? 1 : 0;

                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBMACMAUNITDATAENABLE, macnum, INTEGER, 
				num, sizeof(int), PMFSET);
		}
		break;
	case MIBMACNOTCOPIEDTHRESHOLD:
		if ((v -> value -> un.number < 0) || 
		    (v -> value -> un.number > 65535))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBMACNOTCOPIEDTHRESHOLD, macnum, INTEGER, 
				v -> value -> un.number, sizeof(int), PMFSET);
		break;
    }

    if (status == NOTOK || status == int_SNMP_error__status_badValue)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
	if (status == NOTOK)
	    status = int_SNMP_error__status_genErr;
        return status;
    }

    if (offset == type_SNMP_PDUs_commit)
    {
	if (do_fddi(&SMT_list[fddinum - 1], oi, v, offset) == NOTOK)
	{
	    if (SMT_list[fddinum - 1].msgtosend)
		free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	    SMT_list[fddinum - 1].msgtosend = NULL;
	    return int_SNMP_error__status_noSuchName;
	}

	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
    }

    return int_SNMP_error__status_noError;
}


static int o_fddipath (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar = (int) ot -> ot_info;
    int fddinum;
    int pathnum;
    int status = NOTOK;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered o_fddipath");
#endif /* DEBUG */

    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 2)
	      return int_SNMP_error__status_noSuchName;
	    fddinum = oid -> oid_elements[oid -> oid_nelem - 2];
	    if ((fddinum <= 0) || (fddinum > smtnumber))
	      return int_SNMP_error__status_noSuchName;
	    pathnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    if (!in_pathlist(pathnum, fddinum))
	      return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
            if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	      OID     new;

	      fddinum = 1;
	      if (fddinum > smtnumber)
		return NOTOK;
	      while (SMT_list[fddinum - 1].numpaths == 0)
	      {
		  fddinum++;
		  if (fddinum > smtnumber)
		      return NOTOK;
	      }
	      pathnum = SMT_list[fddinum - 1].pathidnumbers[0];

	      if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 2] = fddinum;
              new -> oid_elements[new -> oid_nelem - 1] = pathnum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
	    else {
              int     i = ot -> ot_name -> oid_nelem;
	      OID new;

	      if ((i >= oid -> oid_nelem) ||
		  ((fddinum = oid -> oid_elements[i]) <= 0)) {
		fddinum = 1;
		if (fddinum > smtnumber)
		  return NOTOK;
	        while (SMT_list[fddinum - 1].numpaths == 0)
	        {
		    fddinum++;
		    if (fddinum > smtnumber)
		        return NOTOK;
	        }
		pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
	      }
	      else {
		if (fddinum > smtnumber)
		  return NOTOK;
                if (i + 2 != oid -> oid_nelem)
		  pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
                else {
		  pathnum = oid -> oid_elements[i + 1];
		  if ((pathnum = next_pathnum(pathnum, fddinum)) == 0)
		  {
                    fddinum++;
		    if (fddinum > smtnumber)
		      return NOTOK;
	            while (SMT_list[fddinum - 1].numpaths == 0)
	            {
		        fddinum++;
		        if (fddinum > smtnumber)
		            return NOTOK;
	            }
		    pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
		  }
		}
              }

	      if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 2] = fddinum;
              new -> oid_elements[new -> oid_nelem - 1] = pathnum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
            break;

	default:
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_1, 
		   "%s: invalid operation: %d"), "o_fddipath",
		   offset);
	    return int_SNMP_error__status_genErr;
    }

o_pathstartover: ;
    switch (ifvar) {
	case MIBPATHSMTINDEX:
		return o_integer(oi, v, fddinum);
		break;
	case MIBPATHINDEX: 
		return o_integer(oi, v, pathnum);
		break;
    }

    /* From here on, the variables are on the FDDI device */
    /* So, create a header and a variable for the packet */
    if ((SMT_list[fddinum - 1].msgtosend == NULL) &&
        (make_SMT_header(&SMT_list[fddinum - 1], PMFGET) == NOTOK))
    {
        if (offset == type_SNMP_PDUs_get__next__request)
            return (NOTOK);
        else return int_SNMP_error__status_noSuchName;
    }

    status = NOTOK;
    switch (ifvar) {
	case MIBPATHTVXLOWERBOUND:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPATHTVXLOWERBOUND, pathnum, FDDITIMENANO, 
			    0, 0, PMFGET);
		break;
	case MIBPATHTMAXLOWERBOUND:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPATHTMAXLOWERBOUND, pathnum, FDDITIMENANO, 
			    0, 0, PMFGET);
		break;
	case MIBPATHMAXTREQ:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPATHMAXTREQ, pathnum, FDDITIMENANO, 
			    0, 0, PMFGET);
		break;
    }

    if (status == NOTOK)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
        if (offset == type_SNMP_PDUs_get__next__request)
            return (NOTOK);
        else return int_SNMP_error__status_noSuchName;
    }

    if (do_fddi(&SMT_list[fddinum - 1], oi, v, offset) == NOTOK)
    {
	OID new;

	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
        SMT_list[fddinum - 1].msgtosend = NULL;
        if (offset == type_SNMP_PDUs_get__next__request)
	{
	    if ((pathnum = next_pathnum(pathnum, fddinum)) == 0)
	    {
		fddinum++;
		if (fddinum > smtnumber)
		    return NOTOK;
	        while (SMT_list[fddinum - 1].numpaths == 0)
	        {
		    fddinum++;
		    if (fddinum > smtnumber)
		        return NOTOK;
	        }
		pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
	    }

	    if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		return NOTOK;
            new -> oid_elements[new -> oid_nelem - 2] = fddinum;
            new -> oid_elements[new -> oid_nelem - 1] = pathnum;

	    if (v -> name)
	        free_SMI_ObjectName (v -> name);
	    v -> name = new;

            goto o_pathstartover;
	}
        else return int_SNMP_error__status_noSuchName;
    }

    if (SMT_list[fddinum - 1].msgtosend)
	free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
    SMT_list[fddinum - 1].msgtosend = NULL;

    return int_SNMP_error__status_noError;
}


static int s_fddipath (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar = (int) ot -> ot_info;
    int fddinum;
    int pathnum;
    int status = int_SNMP_error__status_noError;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered s_fddipath");
#endif /* DEBUG */

    switch (offset) 
    {
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 2)
	      return int_SNMP_error__status_noSuchName;
	    fddinum = oid -> oid_elements[oid -> oid_nelem - 2];
	    if ((fddinum <= 0) || (fddinum > smtnumber))
	      return int_SNMP_error__status_noSuchName;
	    pathnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    if (!in_pathlist(pathnum, fddinum))
	      return int_SNMP_error__status_noSuchName;
	    break;

        case type_SNMP_PDUs_rollback:
	    return int_SNMP_error__status_noError;

	default:
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_1,
		   "%s: invalid operation: %d"), "s_fddipath",
		   offset);
	    return int_SNMP_error__status_genErr;
    }

    /* From here on, the variables are on the FDDI device */
    /* So, create a header and a variable for the packet */
    if ((offset == type_SNMP_PDUs_commit) &&
	(SMT_list[fddinum - 1].msgtosend == NULL) &&
        (make_SMT_header(&SMT_list[fddinum - 1], PMFSET) == NOTOK))
	return int_SNMP_error__status_genErr;

    status = (offset == type_SNMP_PDUs_set__request) ? 
	     int_SNMP_error__status_noError : NOTOK;
    switch (ifvar) 
    {
	case MIBPATHTVXLOWERBOUND:
		if (offset == type_SNMP_PDUs_commit)
		    status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPATHTVXLOWERBOUND, pathnum, FDDITIMENANO, 
			    v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBPATHTMAXLOWERBOUND:
		if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPATHTMAXLOWERBOUND, pathnum, FDDITIMENANO, 
			    v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBPATHMAXTREQ:
		if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPATHMAXTREQ, pathnum, FDDITIMENANO, 
			    v -> value -> un.number, sizeof(int), PMFSET);
		break;
    }

    if (status == NOTOK)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
        return int_SNMP_error__status_genErr;
    }

    if (offset == type_SNMP_PDUs_commit)
    {
	if (do_fddi(&SMT_list[fddinum - 1], oi, v, offset) == NOTOK)
	{
	   if (SMT_list[fddinum - 1].msgtosend)
		free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	    SMT_list[fddinum - 1].msgtosend = NULL;
	    return int_SNMP_error__status_noSuchName;
	}

	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
    }

    return int_SNMP_error__status_noError;
}


static int o_fddiconpath (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar = (int) ot -> ot_info;
    int fddinum;
    int pathnum;
    int tokennum;
    token_path *ptr;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered o_fddiconpath");
#endif /* DEBUG */

    if (isdummyptr -> flush_cache)
	get_fdditokens();

    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 3)
	      return int_SNMP_error__status_noSuchName;
	    fddinum = oid -> oid_elements[oid -> oid_nelem - 3];
	    if ((fddinum <= 0) || (fddinum > smtnumber))
	      return int_SNMP_error__status_noSuchName;
	    pathnum = oid -> oid_elements[oid -> oid_nelem - 2];
	    if (!in_pathlist(pathnum, fddinum))
	      return int_SNMP_error__status_noSuchName;
	    tokennum = oid -> oid_elements[oid -> oid_nelem - 1];
	    if (!in_tokenlist(tokennum, pathnum, fddinum))
	      return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
            if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	      OID     new;

	      fddinum = 1;
	      if (fddinum > smtnumber)
		return NOTOK;
	      while (SMT_list[fddinum - 1].numpaths == 0)
	      {
		  fddinum++;
		  if (fddinum > smtnumber)
		      return NOTOK;
	      }
	      pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
	      tokennum = 1;
	      while (!in_tokenlist(tokennum, pathnum, fddinum))
	      {
		  if ((pathnum = next_pathnum(pathnum, fddinum)) == 0)
		  {
		      fddinum++;
		      if (fddinum > smtnumber)
			return NOTOK;
	              while (SMT_list[fddinum - 1].numpaths == 0)
	      	      {
			  fddinum++;
			  if (fddinum > smtnumber)
			      return NOTOK;
		      }
		      pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
		  }
	      }

	      if ((new = oid_extend (ot -> ot_name, 3)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 3] = fddinum;
              new -> oid_elements[new -> oid_nelem - 2] = pathnum;
              new -> oid_elements[new -> oid_nelem - 1] = tokennum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
	    else 
	    {
              int     i = ot -> ot_name -> oid_nelem;
	      OID new;

	      if ((i >= oid -> oid_nelem) ||
		  ((fddinum = oid -> oid_elements[i]) <= 0)) 
	      {
		  fddinum = 1;
		  if (fddinum > smtnumber)
		      return NOTOK;
	          while (SMT_list[fddinum - 1].numpaths == 0)
	          {
		      fddinum++;
		      if (fddinum > smtnumber)
		          return NOTOK;
	          }
	          pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
	          tokennum = 1;
	          while (!in_tokenlist(tokennum, pathnum, fddinum))
	          {
		      if ((pathnum = next_pathnum(pathnum, fddinum)) == 0)
		      {
		          fddinum++;
		          if (fddinum > smtnumber)
			      return NOTOK;
	                  while (SMT_list[fddinum - 1].numpaths == 0)
	      	          {
			      fddinum++;
			      if (fddinum > smtnumber)
			          return NOTOK;
		          }
		          pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
		      }
	          }
	      }
	      else 
	      {
		if (fddinum > smtnumber)
		  return NOTOK;

                if (i + 3 == oid -> oid_nelem)
		{
		  pathnum = oid -> oid_elements[i + 1];
		  tokennum = oid -> oid_elements[i + 2];

		  if ((tokennum = next_token(tokennum, pathnum, fddinum)) == 0)
		  {
		    if ((pathnum = next_pathnum(pathnum, fddinum)) == 0)
		    {
		      fddinum++;
		      if (fddinum > smtnumber)
			  return NOTOK;
	              while (SMT_list[fddinum - 1].numpaths == 0)
	      	      {
			  fddinum++;
			  if (fddinum > smtnumber)
			      return NOTOK;
		      }
		      pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
		      tokennum = 1;
	              while (!in_tokenlist(tokennum, pathnum, fddinum))
	              {
		          if ((pathnum = next_pathnum(pathnum, fddinum)) == 0)
		          {
		              fddinum++;
		              if (fddinum > smtnumber)
			        return NOTOK;
	                      while (SMT_list[fddinum - 1].numpaths == 0)
	      	              {
			           fddinum++;
			           if (fddinum > smtnumber)
			               return NOTOK;
		              }
		              pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
		          }
	              }
		    }
		    else
		    {
		      tokennum = 1;
	              while (!in_tokenlist(tokennum, pathnum, fddinum))
	              {
		          if ((pathnum = next_pathnum(pathnum, fddinum)) == 0)
		          {
		              fddinum++;
		              if (fddinum > smtnumber)
			        return NOTOK;
	                      while (SMT_list[fddinum - 1].numpaths == 0)
	      	              {
			           fddinum++;
			           if (fddinum > smtnumber)
			               return NOTOK;
		              }
		              pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
		          }
	              }
		    }
		  }
		}
                else if (i + 2 == oid -> oid_nelem)
		{
		  pathnum = oid -> oid_elements[i + 1];
		  tokennum = 1;
	          while (!in_tokenlist(tokennum, pathnum, fddinum))
	          {
		      if ((pathnum = next_pathnum(pathnum, fddinum)) == 0)
		      {
		          fddinum++;
		          if (fddinum > smtnumber)
			      return NOTOK;
	                  while (SMT_list[fddinum - 1].numpaths == 0)
	      	          {
			      fddinum++;
			      if (fddinum > smtnumber)
			          return NOTOK;
		          }
		          pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
		      }
	          }
		}
		else 
		{
		  pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
		  tokennum = 1;
	          while (!in_tokenlist(tokennum, pathnum, fddinum))
	          {
		      if ((pathnum = next_pathnum(pathnum, fddinum)) == 0)
		      {
		          fddinum++;
		          if (fddinum > smtnumber)
			      return NOTOK;
	                  while (SMT_list[fddinum - 1].numpaths == 0)
	      	          {
			      fddinum++;
			      if (fddinum > smtnumber)
			          return NOTOK;
		          }
		          pathnum = SMT_list[fddinum - 1].pathidnumbers[0];
		      }
	          }
		}
              }

	      if ((new = oid_extend (ot -> ot_name, 3)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 3] = fddinum;
              new -> oid_elements[new -> oid_nelem - 2] = pathnum;
              new -> oid_elements[new -> oid_nelem - 1] = tokennum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
            break;

	default:
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_1,
		   "%s: invalid operation: %d"),"o_fddiconpath",
		   offset);
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	case MIBPATHCONFIGSMTINDEX:
		return o_integer(oi, v, fddinum);
		break;
	case MIBPATHCONFIGPATHINDEX: 
		return o_integer(oi, v, pathnum);
		break;
	case MIBPATHCONFIGTOKENORDER: 
		return o_integer(oi, v, tokennum);
		break;
    }

    ptr = get_tokenptr(tokennum, pathnum, fddinum);

    switch (ifvar) {
	case MIBPATHCONFIGRESOURCETYPE:
		return o_integer(oi, v, ptr -> resource_type);
		break;
	case MIBPATHCONFIGRESOURCEINDEX: 
		return o_integer(oi, v, ptr -> resource_id);
		break;
	case MIBPATHCONFIGCURRENTPATH: 
		return o_integer(oi, v, ptr -> current_path + 1);
		break;
    }

    return int_SNMP_error__status_noSuchName;
}


static int o_fddiport (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar = (int) ot -> ot_info;
    int fddinum;
    int portnum;
    int status = NOTOK;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered o_fddiport");
#endif /* DEBUG */

    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 2)
	      return int_SNMP_error__status_noSuchName;
	    fddinum = oid -> oid_elements[oid -> oid_nelem - 2];
	    if ((fddinum <= 0) || (fddinum > smtnumber))
	      return int_SNMP_error__status_noSuchName;
	    portnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    if (!in_portlist(portnum, fddinum))
	      return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
            if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
	      OID     new;

	      fddinum = 1;
	      if (fddinum > smtnumber)
		return NOTOK;
	      while (SMT_list[fddinum - 1].numports == 0)
	      {
		  fddinum++;
		  if (fddinum > smtnumber)
	              return NOTOK;
	      }
	      portnum = SMT_list[0].portidnumbers[0];

	      if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 2] = fddinum;
              new -> oid_elements[new -> oid_nelem - 1] = portnum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
	    else {
              int     i = ot -> ot_name -> oid_nelem;
	      OID new;

	      if ((i >= oid -> oid_nelem) ||
		  ((fddinum = oid -> oid_elements[i]) <= 0)) {
		fddinum = 1;
		if (fddinum > smtnumber)
		  return NOTOK;
	        while (SMT_list[fddinum - 1].numports == 0)
	        {
		    fddinum++;
		    if (fddinum > smtnumber)
	                return NOTOK;
	        }
		portnum = SMT_list[0].portidnumbers[0];
	      }
	      else {
		if (fddinum > smtnumber)
		  return NOTOK;
                if (i + 2 != oid -> oid_nelem)
		  portnum = SMT_list[fddinum - 1].portidnumbers[0];
                else {
		  portnum = oid -> oid_elements[i + 1];
		  if ((portnum = next_portnum(portnum, fddinum)) == 0)
		  {
                    fddinum++;
		    if (fddinum > smtnumber)
		      return NOTOK;
	            while (SMT_list[fddinum - 1].numports == 0)
	            {
		        fddinum++;
		        if (fddinum > smtnumber)
	                    return NOTOK;
	            }
		    portnum = SMT_list[fddinum - 1].portidnumbers[0];
		  }
		}
              }

	      if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		return NOTOK;
              new -> oid_elements[new -> oid_nelem - 2] = fddinum;
              new -> oid_elements[new -> oid_nelem - 1] = portnum;

	      if (v -> name)
	        free_SMI_ObjectName (v -> name);
	      v -> name = new;
	    }
            break;

	default:
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_1,
		   "%s: invalid operation: %d"), "o_fddiport",
		   offset);
	    return int_SNMP_error__status_genErr;
    }

o_portstartover: ;
    switch (ifvar) {
	case MIBPORTSMTINDEX:
		return o_integer(oi, v, fddinum);
		break;
	case MIBPORTINDEX: 
		return o_integer(oi, v, portnum);
		break;
	case MIBPORTACTION: /* This always returns other(1) */
		return o_integer(oi, v, 1);
		break;
    }

    /* From here on, the variables are on the FDDI device */
    /* So, create a header and a variable for the packet */
    if ((SMT_list[fddinum - 1].msgtosend == NULL) &&
        (make_SMT_header(&SMT_list[fddinum - 1], PMFGET) == NOTOK))
    {
        if (offset == type_SNMP_PDUs_get__next__request)
            return (NOTOK);
        else return int_SNMP_error__status_noSuchName;
    }

    switch (ifvar) {
	case MIBPORTMYTYPE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTMYTYPE, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTNEIGHBORTYPE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTNEIGHBORTYPE, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTCONNECTIONPOLICIES:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTCONNECTIONPOLICIES, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTMACINDICATED:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTMACINDICATED, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTCURRENTPATH:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTCURRENTPATH, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTREQUESTEDPATHS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTREQUESTEDPATHS, portnum, OCTETSTRING, 
			    0, 0, PMFGET);
		break;
	case MIBPORTMACPLACEMENT:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTMACPLACEMENT, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTAVAILABLEPATHS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTAVAILABLEPATHS, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTPMDCLASS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTPMDCLASS, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTCONNECTIONCAPABILITIES:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTCONNECTIONCAPABILITIES, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTBSFLAG:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTBSFLAG, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTLCTFAILCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTLCTFAILCTS, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTLERESTIMATE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTLERESTIMATE, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTLEMREJECTCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTLEMREJECTCTS, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTLEMCTS:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTLEMCTS, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTLERCUTOFF:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTLERCUTOFF, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTLERALARM:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTLERALARM, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTCONNECTSTATE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTCONNECTSTATE, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTPCMSTATE:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTPCMSTATE, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTPCWITHHOLD:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTPCWITHHOLD, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTLERFLAG:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTLERFLAG, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
	case MIBPORTHARDWAREPRESENT:
                status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTHARDWAREPRESENT, portnum, INTEGER, 
			    0, 0, PMFGET);
		break;
    }
    if (status == NOTOK)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
        if (offset == type_SNMP_PDUs_get__next__request)
            return (NOTOK);
        else return int_SNMP_error__status_noSuchName;
    }

    if (do_fddi(&SMT_list[fddinum - 1], oi, v, offset) == NOTOK)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
        SMT_list[fddinum - 1].msgtosend = NULL;
        if (offset == type_SNMP_PDUs_get__next__request)
	{
	    OID new;

	    if ((portnum = next_portnum(portnum, fddinum)) == 0)
	    {
		fddinum++;
		if (fddinum > smtnumber)
		    return NOTOK;
	        while (SMT_list[fddinum - 1].numports == 0)
	        {
		    fddinum++;
		    if (fddinum > smtnumber)
	                return NOTOK;
	        }
		portnum = SMT_list[fddinum - 1].portidnumbers[0];
	    }

	    if ((new = oid_extend (ot -> ot_name, 2)) == NULLOID)
		return NOTOK;
            new -> oid_elements[new -> oid_nelem - 2] = fddinum;
            new -> oid_elements[new -> oid_nelem - 1] = portnum;

	    if (v -> name)
	        free_SMI_ObjectName (v -> name);
	    v -> name = new;

            goto o_portstartover;
	}
        else return int_SNMP_error__status_noSuchName;
    }

    if (SMT_list[fddinum - 1].msgtosend)
	free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
    SMT_list[fddinum - 1].msgtosend = NULL;

    return int_SNMP_error__status_noError;
}


static int s_fddiport (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    register OS		os = ot -> ot_syntax;
    int ifvar = (int) ot -> ot_info;
    int fddinum;
    int portnum;
    int status = int_SNMP_error__status_noError;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered s_fddiport");
#endif /* DEBUG */

    switch (offset) 
    {
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 2)
	      return int_SNMP_error__status_noSuchName;
	    fddinum = oid -> oid_elements[oid -> oid_nelem - 2];
	    if ((fddinum <= 0) || (fddinum > smtnumber))
	      return int_SNMP_error__status_noSuchName;
	    portnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    if (!in_portlist(portnum, fddinum))
	      return int_SNMP_error__status_noSuchName;
	    break;

        case type_SNMP_PDUs_rollback:
	    return int_SNMP_error__status_noError;

	default:
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_1,
		   "%s: invalid operation: %d"), "s_fddiport",
		   offset);
	    return int_SNMP_error__status_genErr;
    }

    /* From here on, the variables are on the FDDI device */
    /* So, create a header and a variable for the packet */
    if ((offset == type_SNMP_PDUs_commit) &&
	(SMT_list[fddinum - 1].msgtosend == NULL) &&
        (make_SMT_header(&SMT_list[fddinum - 1], PMFSET) == NOTOK))
	return int_SNMP_error__status_genErr;

    status = (offset == type_SNMP_PDUs_set__request) ? 
	     int_SNMP_error__status_noError : NOTOK;
    switch (ifvar) 
    {
	case MIBPORTCONNECTIONPOLICIES:
		if ((v -> value -> un.number < 0) || 
		    (v -> value -> un.number > 3))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBPORTCONNECTIONPOLICIES, portnum, INTEGER, 
				v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBPORTREQUESTEDPATHS:
	    {
		char *strig;

		if (ot -> ot_save)
		    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
		if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		    return int_SNMP_error__status_badValue;

                if ((strig = qb2str((struct qbuf *) ot -> ot_save)) == NULLCP)
		{
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_2,
		            "GENERR in %s: s_generic failed"), "s_fddiport");
                    if (ot -> ot_save)
		        (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
		    status = int_SNMP_error__status_genErr;
		    break;
		}
                if (ot -> ot_save)
		    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;

		if (strlen(strig) > 3)
		{
		    free(strig);
		    status = int_SNMP_error__status_badValue;
		    break;
		}

		if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
			    PMFMIBPORTREQUESTEDPATHS, portnum, OCTETSTRING, 
			    strig, strlen(strig), PMFSET);

		free(strig);
	    }
	    break;
	case MIBPORTLERCUTOFF:
		if ((v -> value -> un.number < 4) || 
		    (v -> value -> un.number > 15))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBPORTLERCUTOFF, portnum, INTEGER, 
				v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBPORTLERALARM:
		if ((v -> value -> un.number < 4) || 
		    (v -> value -> un.number > 15))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBPORTLERALARM, portnum, INTEGER, 
				v -> value -> un.number, sizeof(int), PMFSET);
		break;
	case MIBPORTACTION:
		if ((v -> value -> un.number < 2) || 
		    (v -> value -> un.number > 6))
		    status = int_SNMP_error__status_badValue;
		else if (offset == type_SNMP_PDUs_commit)
                    status = add_SMT_variable(&SMT_list[fddinum - 1], 
				PMFMIBPORTACTION, portnum, INTEGER, 
				v->value->un.number - 2, sizeof(int), PMFSET);
		break;
    }

    if (status == NOTOK || status == int_SNMP_error__status_badValue)
    {
	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1); 
	SMT_list[fddinum - 1].msgtosend = NULL;
	if (status == NOTOK)
	    status = int_SNMP_error__status_genErr;
        return status;
    }

    if (offset == type_SNMP_PDUs_commit)
    {
	if (do_fddi(&SMT_list[fddinum - 1], oi, v, offset) == NOTOK)
	{
	    if (SMT_list[fddinum - 1].msgtosend)
		free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	    SMT_list[fddinum - 1].msgtosend = NULL;
	    return int_SNMP_error__status_noSuchName;
	}

	if (SMT_list[fddinum - 1].msgtosend)
	    free_SMT_frame(SMT_list[fddinum - 1].msgtosend, 1);
	SMT_list[fddinum - 1].msgtosend = NULL;
    }

    return int_SNMP_error__status_noError;
}


static int o_fddinumbers (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    register OID	oid = oi -> oi_name;
    int ifvar = (int) ot -> ot_info;
    int rc;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered o_fddinumbers");
#endif /* DEBUG */

    if ((rc = o_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;

    switch (ifvar)
    {
	case MIBSMTNUMBER:
		return o_integer(oi, v, smtnumber);
		break;
	case MIBMACNUMBER:
		return o_integer(oi, v, macnumber);
		break;
	case MIBPORTNUMBER:
		return o_integer(oi, v, portnumber);
		break;
	case MIBPATHNUMBER:
		return o_integer(oi, v, pathnumber);
		break;
    }
}

/*  */
void init_fddi ()
{
    register OT	    ot;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered init_fddi");
#endif /* DEBUG */

    if (ot = text2obj ("fddimibSMTNumber")) {
	ot -> ot_getfnx = o_fddinumbers;
        ot -> ot_info = (caddr_t) MIBSMTNUMBER;
    }
    if (ot = text2obj ("fddimibSMTIndex")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTINDEX;
    }
    if (ot = text2obj ("fddimibSMTStationId")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTSTATIONID;
    }
    if (ot = text2obj ("fddimibSMTOpVersionId")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTOPVERSIONID;
    }
    if (ot = text2obj ("fddimibSMTHiVersionId")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTHIVERSIONID;
    }
    if (ot = text2obj ("fddimibSMTLoVersionId")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTLOVERSIONID;
    }
    if (ot = text2obj ("fddimibSMTUserData")) {
	ot -> ot_getfnx = o_fddismt;
	ot -> ot_setfnx = s_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTUSERDATA;
    }
    if (ot = text2obj ("fddimibSMTMIBVERSIONID")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTMIBVERSIONID;
    }
    if (ot = text2obj ("fddimibSMTMACCts")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTMACCTS;
    }
    if (ot = text2obj ("fddimibSMTNonMasterCts")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTNONMASTERCT;
    }
    if (ot = text2obj ("fddimibSMTMasterCts")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTMASTERCT;
    }
    if (ot = text2obj ("fddimibSMTAvailablePaths")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTAVAILABLEPATHS;
    }
    if (ot = text2obj ("fddimibSMTConfigCapabilities")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTCONFIGCAPABILITIES;
    }
    if (ot = text2obj ("fddimibSMTConfigPolicy")) {
	ot -> ot_getfnx = o_fddismt;
	ot -> ot_setfnx = s_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTCONFIGPOLICY;
    }
    if (ot = text2obj ("fddimibSMTConnectionPolicy")) {
	ot -> ot_getfnx = o_fddismt;
	ot -> ot_setfnx = s_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTCONNECTIONPOLICY;
    }
    if (ot = text2obj ("fddimibSMTTNotify")) {
	ot -> ot_getfnx = o_fddismt;
	ot -> ot_setfnx = s_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTTNOTIFY; 
    }
    if (ot = text2obj ("fddimibSMTStatRptPolicy")) {
	ot -> ot_getfnx = o_fddismt;
	ot -> ot_setfnx = s_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTSTATRPTPOLICY;
    }
    if (ot = text2obj ("fddimibSMTTraceMaxExpiration")) {
	ot -> ot_getfnx = o_fddismt;
	ot -> ot_setfnx = s_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTTRACEMAXEXPIRATION;
    }
    if (ot = text2obj ("fddimibSMTBypassPresent")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTBYPASSPRESENT;
    }
    if (ot = text2obj ("fddimibSMTECMState")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTECMSTATE;
    }
    if (ot = text2obj ("fddimibSMTCFState")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTCFSTATE;
    }
    if (ot = text2obj ("fddimibSMTRemoteDisconnectFlag")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTREMOTEDISCONNECTFLAG;
    }
    if (ot = text2obj ("fddimibSMTStationStatus")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTSTATIONSTATUS;
    }
    if (ot = text2obj ("fddimibSMTPeerWrapFlag")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTPEERWRAPFLAG;
    }
    if (ot = text2obj ("fddimibSMTTimeStamp")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTTIMESTAMP;
    }
    if (ot = text2obj ("fddimibSMTTransitionTimeStamp")) {
	ot -> ot_getfnx = o_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTTRANSITIONTIMESTAMP;
    }
    if (ot = text2obj ("fddimibSMTStationAction")) {
	ot -> ot_getfnx = o_fddismt;
	ot -> ot_setfnx = s_fddismt;
        ot -> ot_info = (caddr_t) MIBSMTSTATIONACTION;
    }

    if (ot = text2obj ("fddimibMACNumber")) {
	ot -> ot_getfnx = o_fddinumbers;
        ot -> ot_info = (caddr_t) MIBMACNUMBER;
    }
    if (ot = text2obj ("fddimibMACSMTIndex")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACSMTINDEX;
    }
    if (ot = text2obj ("fddimibMACIndex")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACINDEX;
    }
    if (ot = text2obj ("fddimibMACIfIndex")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACIFINDEX;
    }
    if (ot = text2obj ("fddimibMACFrameStatusFunctions")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACFRAMESTATUSFUNCTIONS;
    }
    if (ot = text2obj ("fddimibMACTMaxCapability")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACTMAXCAPABILITY;
    }
    if (ot = text2obj ("fddimibMACTVXCapability")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACTVXCAPABILITY;
    }
    if (ot = text2obj ("fddimibMACAvailablePaths")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACAVAILABLEPATHS;
    }
    if (ot = text2obj ("fddimibMACCurrentPath")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACCURRENTPATH;
    }
    if (ot = text2obj ("fddimibMACUpstreamNbr")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACUPSTREAMNBR;
    }
    if (ot = text2obj ("fddimibMACDownstreamNbr")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACDOWNSTREAMNBR;
    }
    if (ot = text2obj ("fddimibMACOldUpstreamNbr")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACOLDUPSTREAMNBR;
    }
    if (ot = text2obj ("fddimibMACOldDownstreamNbr")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACOLDDOWNSTREAMNBR;
    }
    if (ot = text2obj ("fddimibMACDupAddressTest")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACDUPADDRESSTEST;
    }
    if (ot = text2obj ("fddimibMACRequestedPaths")) {
	ot -> ot_getfnx = o_fddimac;
	ot -> ot_setfnx = s_fddimac;
        ot -> ot_info = (caddr_t) MIBMACREQUESTEDPATHS;
    }
    if (ot = text2obj ("fddimibMACDownstreamPORTType")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACDOWNSTREAMPORTTYPE;
    }
    if (ot = text2obj ("fddimibMACSMTAddress")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACSMTADDRESS;
    }
    if (ot = text2obj ("fddimibMACTReq")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACTREQ;
    }
    if (ot = text2obj ("fddimibMACTNeg")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACTNEG;
    }
    if (ot = text2obj ("fddimibMACTMax")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACTMAX;
    }
    if (ot = text2obj ("fddimibMACTvxValue")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACTVXVALUE;
    }
    if (ot = text2obj ("fddimibMACFrameCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACFRAMECTS;
    }
    if (ot = text2obj ("fddimibMACCopiedCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACCOPIEDCTS;
    }
    if (ot = text2obj ("fddimibMACTransmitCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACTRANSMITCTS;
    }
    if (ot = text2obj ("fddimibMACErrorCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACERRORCTS;
    }
    if (ot = text2obj ("fddimibMACLostCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACLOSTCTS;
    }
    if (ot = text2obj ("fddimibMACFrameErrorThreshold")) {
	ot -> ot_getfnx = o_fddimac;
	ot -> ot_setfnx = s_fddimac;
        ot -> ot_info = (caddr_t) MIBMACFRAMEERRORTHRESHOLD;
    }
    if (ot = text2obj ("fddimibMACFrameErrorRatio")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACFRAMEERRORRATIO;
    }
    if (ot = text2obj ("fddimibMACRMTState")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACRMTSTATE;
    }
    if (ot = text2obj ("fddimibMACDaFlag")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACDAFLAG;
    }
    if (ot = text2obj ("fddimibMACUnaDaFlag")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACUNADAFLAG;
    }
    if (ot = text2obj ("fddimibMACFrameErrorFlag")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACFRAMEERRORFLAG;
    }
    if (ot = text2obj ("fddimibMACMAUnitdataAvailable")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACMAUNITDATAAVAILABLE;
    }
    if (ot = text2obj ("fddimibMACHardwarePresent")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACHARDWAREPRESENT;
    }
    if (ot = text2obj ("fddimibMACMAUnitdataEnable")) {
	ot -> ot_getfnx = o_fddimac;
	ot -> ot_setfnx = s_fddimac;
        ot -> ot_info = (caddr_t) MIBMACMAUNITDATAENABLE;
    }

    if (ot = text2obj ("fddimibMACTokenCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACTOKENCTS;
    }
    if (ot = text2obj ("fddimibMACTvxExpiredCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACTVXEXPIREDCTS;
    }
    if (ot = text2obj ("fddimibMACNotCopiedCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACNOTCOPIEDCTS;
    }
    if (ot = text2obj ("fddimibMACLateCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACLATECTS;
    }
    if (ot = text2obj ("fddimibMACRingOpCts")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACRINGOPCTS;
    }
    if (ot = text2obj ("fddimibMACNotCopiedRatio")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACNOTCOPIEDRATIO;
    }
    if (ot = text2obj ("fddimibMACNotCopiedFlag")) {
	ot -> ot_getfnx = o_fddimac;
        ot -> ot_info = (caddr_t) MIBMACNOTCOPIEDFLAG;
    }
    if (ot = text2obj ("fddimibMACNotCopiedThreshold")) {
	ot -> ot_getfnx = o_fddimac;
	ot -> ot_setfnx = s_fddimac;
        ot -> ot_info = (caddr_t) MIBMACNOTCOPIEDTHRESHOLD;
    }

    if (ot = text2obj ("fddimibPATHNumber")) {
	ot -> ot_getfnx = o_fddinumbers;
        ot -> ot_info = (caddr_t) MIBPATHNUMBER;
    }
    if (ot = text2obj ("fddimibPATHSMTIndex")) {
	ot -> ot_getfnx = o_fddipath;
        ot -> ot_info = (caddr_t) MIBPATHSMTINDEX;
    }
    if (ot = text2obj ("fddimibPATHIndex")) {
	ot -> ot_getfnx = o_fddipath;
        ot -> ot_info = (caddr_t) MIBPATHINDEX;
    }
    if (ot = text2obj ("fddimibPATHTVXLowerBound")) {
	ot -> ot_getfnx = o_fddipath;
	ot -> ot_setfnx = s_fddipath;
        ot -> ot_info = (caddr_t) MIBPATHTVXLOWERBOUND;
    }
    if (ot = text2obj ("fddimibPATHTMaxLowerBound")) {
	ot -> ot_getfnx = o_fddipath;
	ot -> ot_setfnx = s_fddipath;
        ot -> ot_info = (caddr_t) MIBPATHTMAXLOWERBOUND;
    }
    if (ot = text2obj ("fddimibPATHMaxTReq")) {
	ot -> ot_getfnx = o_fddipath;
	ot -> ot_setfnx = s_fddipath;
        ot -> ot_info = (caddr_t) MIBPATHMAXTREQ;
    }

    if (ot = text2obj ("fddimibPATHConfigSMTIndex")) {
	ot -> ot_getfnx = o_fddiconpath;
        ot -> ot_info = (caddr_t) MIBPATHCONFIGSMTINDEX;
    }
    if (ot = text2obj ("fddimibPATHConfigPATHIndex")) {
	ot -> ot_getfnx = o_fddiconpath;
        ot -> ot_info = (caddr_t) MIBPATHCONFIGPATHINDEX;
    }
    if (ot = text2obj ("fddimibPATHConfigTokenOrder")) {
	ot -> ot_getfnx = o_fddiconpath;
        ot -> ot_info = (caddr_t) MIBPATHCONFIGTOKENORDER;
    }
    if (ot = text2obj ("fddimibPATHConfigResourceType")) {
	ot -> ot_getfnx = o_fddiconpath;
        ot -> ot_info = (caddr_t) MIBPATHCONFIGRESOURCETYPE;
    }
    if (ot = text2obj ("fddimibPATHConfigResourceIndex")) {
	ot -> ot_getfnx = o_fddiconpath;
        ot -> ot_info = (caddr_t) MIBPATHCONFIGRESOURCEINDEX;
    }
    if (ot = text2obj ("fddimibPATHConfigCurrentPath")) {
	ot -> ot_getfnx = o_fddiconpath;
        ot -> ot_info = (caddr_t) MIBPATHCONFIGCURRENTPATH;
    }

    if (ot = text2obj ("fddimibPORTNumber")) {
	ot -> ot_getfnx = o_fddinumbers;
        ot -> ot_info = (caddr_t) MIBPORTNUMBER;
    }
    if (ot = text2obj ("fddimibPORTSMTIndex")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTSMTINDEX;
    }
    if (ot = text2obj ("fddimibPORTIndex")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTINDEX;
    }
    if (ot = text2obj ("fddimibPORTMyType")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTMYTYPE;
    }
    if (ot = text2obj ("fddimibPORTNeighborType")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTNEIGHBORTYPE;
    }
    if (ot = text2obj ("fddimibPORTConnectionPolicies")) {
	ot -> ot_getfnx = o_fddiport;
	ot -> ot_setfnx = s_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTCONNECTIONPOLICIES;
    }
    if (ot = text2obj ("fddimibPORTMACIndicated")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTMACINDICATED;
    }
    if (ot = text2obj ("fddimibPORTCurrentPath")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTCURRENTPATH;
    }
    if (ot = text2obj ("fddimibPORTRequestedPaths")) {
	ot -> ot_getfnx = o_fddiport;
	ot -> ot_setfnx = s_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTREQUESTEDPATHS;
    }
    if (ot = text2obj ("fddimibPORTMACPlacement")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTMACPLACEMENT;
    }
    if (ot = text2obj ("fddimibPORTAvailablePaths")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTAVAILABLEPATHS;
    }
    if (ot = text2obj ("fddimibPORTPMDClass")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTPMDCLASS;
    }
    if (ot = text2obj ("fddimibPORTConnectionCapabilities")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTCONNECTIONCAPABILITIES;
    }
    if (ot = text2obj ("fddimibPORTBSFlag")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTBSFLAG;
    }
    if (ot = text2obj ("fddimibPORTLCTFailCts")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTLCTFAILCTS;
    }
    if (ot = text2obj ("fddimibPORTLerEstimate")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTLERESTIMATE;
    }
    if (ot = text2obj ("fddimibPORTLemRejectCts")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTLEMREJECTCTS;
    }
    if (ot = text2obj ("fddimibPORTLemCts")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTLEMCTS;
    }
    if (ot = text2obj ("fddimibPORTLerCutoff")) {
	ot -> ot_getfnx = o_fddiport;
	ot -> ot_setfnx = s_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTLERCUTOFF;
    }
    if (ot = text2obj ("fddimibPORTLerAlarm")) {
	ot -> ot_getfnx = o_fddiport;
	ot -> ot_setfnx = s_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTLERALARM;
    }
    if (ot = text2obj ("fddimibPORTConnectState")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTCONNECTSTATE;
    }
    if (ot = text2obj ("fddimibPORTPCMState")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTPCMSTATE;
    }
    if (ot = text2obj ("fddimibPORTPCWithhold")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTPCWITHHOLD;
    }
    if (ot = text2obj ("fddimibPORTLerFlag")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTLERFLAG;
    }
    if (ot = text2obj ("fddimibPORTHardwarePresent")) {
	ot -> ot_getfnx = o_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTHARDWAREPRESENT;
    }
    if (ot = text2obj ("fddimibPORTAction")) {
	ot -> ot_getfnx = o_fddiport;
	ot -> ot_setfnx = s_fddiport;
        ot -> ot_info = (caddr_t) MIBPORTACTION;
    }
}


void free_SMT_frame(frame, num)
SMT_frame *frame;
int num;
{
    SMT_var_type *ptr;
    int i;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered free_SMT_frame");
#endif /* DEBUG */

    if (!frame || num <= 0)
	return;

    for (i = 0; i < num; i++)
	while (frame[i].list) 
	{
	    ptr = frame[i].list;
	    frame[i].list = (SMT_var_type *)frame[i].list -> next;
	    if (ptr -> variableinfo)
		free(ptr -> variableinfo);
	    free(ptr);
	}

    free(frame);
}


/* Allocate a number of frames */
SMT_frame *allocate_SMT_frame(int num)
{
    SMT_frame *frames;
    int i;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered allocate_SMT_frame");
#endif /* DEBUG */

    if ((frames = (SMT_frame *)calloc(num, sizeof(SMT_frame))) == NULL)
    {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL,
		GENERA_1, "%s: Out of Memory"), "allocate_SMT_frame");
	return (NULL);
    }

    for (i = 0; i < num; i++)
    {
	frames[i].list = NULL;
	frames[i].endlist = NULL;
	frames[i].numvariable = 0;
	frames[i].buflen = 0;
    }
    return(frames);
}


int make_SMT_header(SMT_inst, setorget)
SMT_impl *SMT_inst;
int setorget;
{
    register fddi_smt_hdr_t *ptr;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered make_SMT_header");
#endif /* DEBUG */

    free_SMT_frame(SMT_inst -> msgtosend, 1);

    if ((SMT_inst -> msgtosend = allocate_SMT_frame(1)) == NULL)
        return (NOTOK);

    SMT_inst -> msgtosend -> buflen = sizeof(fddi_smt_hdr_t);
    ptr = &(SMT_inst -> msgtosend -> un.header);
    ptr -> flag = CFDDI_TX_PROC_ONLY;  /* flag to hold the TRF flags */
    ptr -> res[0] = 0;                 /* 2 reserved bytes */
    ptr -> res[1] = 0;                 /* 2 reserved bytes */
    ptr -> fc = FDDI_FC_SMT;           /* Frame control field */
    ptr -> frame_class = (setorget == PMFGET) ? FDDI_SMT_FC_PMF_GET :
			 FDDI_SMT_FC_PMF_CHG; /* SMT Frame Class */
    ptr -> frame_type = FDDI_SMT_FT_REQUEST;  /* SMT Frame Type */
    ptr -> vid = FDDI_SMT_VID;         /* SMT version ID */
    ptr -> tid = FDDI_TID_SNMP;        /* Transaction ID */
    ptr -> pad = 0x0000;               /* padding */
    ptr -> l_info = 0;                 /* length of SMT info */

    bcopy(SMT_inst -> ifentry -> ndd_physaddr, ptr -> dest, 6); /* Dest addr */
    bcopy(SMT_inst -> ifentry -> ndd_physaddr, ptr -> src, 6);  /* Src addr  */
    ptr -> station_id[0] = 0;
    ptr -> station_id[1] = 0;
    bcopy(SMT_inst -> ifentry -> ndd_physaddr, &ptr -> station_id[2], 6);

    return (OK);
}


int add_SMT_variable(SMT_inst, varnum, resource, syntax, var, len, setorget)
SMT_impl *SMT_inst;
int varnum;
int resource;
int syntax;
caddr_t var;
int len;
int setorget;
{
    fddi_smt_info_t *varptr;
    SMT_var_type *listelem;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered add_SMT_variable");
#endif /* DEBUG */

    if ((listelem = (SMT_var_type *)calloc(1, sizeof(SMT_var_type))) == NULL)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
	       "%s: Out of Memory"), "add_SMT_variable");
        free_SMT_frame(SMT_inst -> msgtosend, 1);
        SMT_inst -> msgtosend = NULL;
        return(NOTOK);
    }
    if ((varptr=(fddi_smt_info_t *)calloc(1, sizeof(fddi_smt_info_t))) == NULL)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1, 
	       "%s: Out of Memory"), "add_SMT_variable");
	free(listelem);
        free_SMT_frame(SMT_inst -> msgtosend, 1);
        SMT_inst -> msgtosend = NULL;
        return(NOTOK);
    }

    listelem -> variableinfo = varptr;
    listelem -> SNMPvarsyntax = syntax;
    listelem -> next = NULL;
    varptr -> type = varnum;
    if (resource != -1) 
    {
        varptr -> len = sizeof(int);
	listelem -> reso = 1;
	bcopy(&resource, varptr -> data, sizeof(int));
    }
    else
    {
	varptr -> len = 0;
	listelem -> reso = 0;
    }

    if (setorget == PMFSET)
    {
	switch (syntax)
	{
	    case FDDITIMENANO:
		{
		    int temp = (int)var;

		    temp = temp / 80;
		    temp = ~temp + 1;

		    bcopy((char *)&temp, &(varptr -> data[varptr -> len]), len);
		}
		break;

	    case INTEGER:
	        {
		    int temp = (int)var;

		    bcopy((char *)&temp, &(varptr->data[varptr -> len]), len);
	        }
		break;

	    case FDDITIMEMILLI:
		{
		    int temp = (int)var;

		    temp = temp * 10 / 8;

		    bcopy((char *)&temp, &(varptr -> data[varptr -> len]), len);
		}
		break;

	    default:
	        bcopy(var, &(varptr -> data[varptr -> len]), len);
		break;
	}
	varptr -> len += len;
    }

    SMT_inst -> msgtosend -> buflen += varptr -> len + sizeof(int);

    if (SMT_inst -> msgtosend -> list == NULL)
        SMT_inst -> msgtosend -> list = listelem;
    else 
        SMT_inst -> msgtosend -> endlist -> next = 
					      (struct SMT_var_type *)listelem;
    SMT_inst -> msgtosend -> endlist = listelem;

    return (OK);
}


int complete_SMT_frame(SMT_inst)
SMT_impl *SMT_inst;
{
    char *buf = SMT_inst -> msgtosend -> un.buffer;
    SMT_var_type *ptr = SMT_inst -> msgtosend -> list;
    int length = sizeof(fddi_smt_hdr_t);

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered complete_SMT_frame");
#endif /* DEBUG */

    while (ptr) 
    {
        bcopy((char *)(ptr -> variableinfo), &buf[length], 
	      4 + ptr -> variableinfo -> len);
        length += 4 + ptr -> variableinfo -> len;
	ptr = (SMT_var_type *)ptr -> next;
    }
    SMT_inst->msgtosend->un.header.l_info = length - sizeof(fddi_smt_hdr_t);

    return (length);
}

/* 
 * FDDI code works like this:
 * 1. Create a packet to send.  This is done during the normal loop through the
 *    PDU message.
 * 2. Send the packet.  
 * 3. Get a bunch of frames throwing away non-SNMP frames
 * 4. Parse to see if it is for our message we sent 
 *
 * For configuration, use steps 1-3 and parse our on packets.
 */
 
/* 
 * Use this function to remove non-SNMP SMT frames from the list 
 */

int parse_frame(SMT_frame *frame)
{
    int done = frame -> un.header.l_info;
    int idx = sizeof(fddi_smt_hdr_t);
    SMT_var_type *ptr;
    int timestamp = 0;  /* Used to check for multiple timestamp values */
    fddi_smt_info_t *varptr;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered parse_frame");
#endif /* DEBUG */

    if (frame -> un.header.fc != FDDI_FC_SMT)
	return(NOTOK);
    if (frame -> un.header.tid != FDDI_TID_SNMP)
	return(NOTOK);

    frame -> numvariable = 0;
    while (done > 0)
    {
	if ((ptr = (SMT_var_type *)calloc(1, sizeof(SMT_var_type))) == NULL)
	{
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
		   "%s: Out of Memory"), "parse_frame");
	    goto exit_parse_frames;
	}

	if ((varptr=(fddi_smt_info_t *)calloc(1,sizeof(fddi_smt_info_t)))==NULL)
	{
	     advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
	            "%s: Out of Memory"), "parse_frame");
	    free(ptr);
	    goto exit_parse_frames;
        }

	ptr -> next = NULL;
	ptr -> variableinfo = varptr;
	varptr->type = *(ushort *)(&(frame -> un.buffer[idx]));
	varptr->len = *(ushort *)(&(frame->un.buffer[idx + 2]));

	bcopy((char *)(&(frame -> un.buffer[idx + 4])),
	      varptr -> data, varptr -> len);

	idx += 4 + varptr -> len;
	done -= 4 + varptr -> len;

	if (varptr -> type == PMFSETCOUNT)
	{
	    bcopy(varptr -> data, SetCount, 12);
	    free(varptr);
	    free(ptr);
	    continue;
	}

	if (varptr -> type == PMFREASONCODE)
	{
	    bcopy(varptr -> data, &(frame -> rc), 4);
	    free(varptr);
	    free(ptr);
	    continue;
	}

	if (varptr -> type == PMFTIMESTAMP && !timestamp)
	{
	    free(varptr);
	    free(ptr);
	    timestamp++;
	    continue;
	}

	if (varptr -> type == PMFLASTSETSTATIONID)
	{
	     free(varptr);
	     free(ptr);
	     continue;
	}

	if (frame -> list == NULL)
	    frame -> list = ptr;
	else frame -> endlist -> next = (struct SMT_var_type *)ptr;
	frame -> endlist = ptr;
        frame -> numvariable++;
    }

    if (done != 0)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_9, 
	       "%s: malformed packet"), "parse_frame");

exit_parse_frames: ;
	ptr = frame -> list;
	while (ptr)
	{
	    SMT_var_type *ptr2;

	    if (ptr -> variableinfo)
		free(ptr -> variableinfo);
	    ptr2 = ptr;
	    ptr = (SMT_var_type *)ptr -> next;
	    free(ptr2);
	}

	return(NOTOK);
    }

    return(OK);
}


SMT_frame *get_frames(SMT_impl *SMT_inst, int *numframes)
{
	int n = SMT_inst -> s + 1;
        struct fd_set rfds;
	struct timeval to;
	int rc;
	SMT_frame *frames = NULL;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered get_frames");
#endif /* DEBUG */

	*numframes = 0;
	to.tv_sec = fdditimeout;        /* seconds       */
	to.tv_usec = 0;                 /* microseconds  */

	FD_ZERO(&rfds);
	FD_SET(SMT_inst -> s, &rfds);

	rc = select(n, &rfds, 0, 0, &to);

	if (rc == 0) /* timeout */
	{
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_3,
		   "%s: time out on response"), "get_frames");
            return NULL;
	}
	else if (rc == -1) /* Error check it and see */ 
	{
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_4,
		   "%s: select failed"), "get_frames");
            return NULL;
	}
	else /* read it and parse it */
	{
	    char buffer[CFDDI_MAX_SMT_PACKET];
	    int buflen = 0;
	    int done = 0;

            while (!done)
	    {
		int bufpos = 0;
		SMT_frame *oldframes; 

	        buflen = read(SMT_inst -> s, buffer, CFDDI_MAX_SMT_PACKET);

		if ((buflen < 0 && errno == EAGAIN) || (buflen == 0))
		    return(frames);

		if (buflen < 0)
		{
		    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_5,
			   "%s: read error"), "get_frames");
		    if (frames)
			free_SMT_frame(frames, *numframes);
		    *numframes = 0;
		    return(NULL);
		}

                oldframes = frames;
		if ((frames = allocate_SMT_frame(*numframes + 1)) == NULL)
		{
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL,
			GENERA_1, "%s: Out of Memory"), "get_frames");
		    return(oldframes);
		} 

		if (oldframes) 
		    bcopy(oldframes, frames, *numframes*sizeof(SMT_frame));

		frames[*numframes].buflen = sizeof(fddi_smt_hdr_t) + 
			       ((fddi_smt_hdr_t *)buffer) -> l_info;
		bcopy(buffer, frames[*numframes].un.buffer, 
		      frames[*numframes].buflen);

		if (parse_frame(&frames[*numframes]) != NOTOK)
		{
		    if (oldframes)
		        free(oldframes);
		    (*numframes)++;
		}
		else 
		{
		    free(frames);
		    frames = oldframes;
		} 
	    }
	}

	return (frames);
}


SMT_frame *send_out_packet(SMT_inst, numframes)
SMT_impl *SMT_inst;
int *numframes;
{
    int rc;
    register SMT_frame *SMTmsg = SMT_inst -> msgtosend;
    SMT_frame *frames = NULL;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered send_out_packet");
#endif /* DEBUG */

    if (open_device(SMT_inst -> ifentry) == NOTOK)
	return(NULL);

    SMT_inst -> s = SMT_inst -> ifentry -> fi_fd;
    rc = write(SMT_inst -> s, SMTmsg -> un.buffer, SMTmsg -> buflen);

    if (rc == SMTmsg -> buflen) 
	frames = get_frames(SMT_inst, numframes);
    else /* Error writing */
    {
	if (errno == EAGAIN)
	{
	    /* Do one retry */
	    rc = write(SMT_inst -> s, SMTmsg -> un.buffer, SMTmsg -> buflen);

	    if (rc == SMTmsg -> buflen) 
	    {
		frames = get_frames(SMT_inst, numframes);
		SMT_inst -> s = NOTOK;
		close_device(SMT_inst -> ifentry);
		return(frames);
	    }
        }

        if (rc == -1)
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_6,
		   "%s: write error"), "send_out_packet");
	else advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_7,
	           "%s: write error, partial write"), "send_out_packet");
	*numframes = 0;
	frames = NULL;
    }

    close_device(SMT_inst -> ifentry);
    SMT_inst -> s = NOTOK;
    return (frames);
}


/* calls send_out_packet expecting it to take care of errors on read/write */
/* This routine is called when sending a packet associated with a PDU */
int do_fddi (SMT_inst, oi, v, offset)
SMT_impl *SMT_inst;
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    int		i;	
    int numframes = 0;
    SMT_frame *frames = NULL;
    int k;
	
#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered do_fddi");
#endif /* DEBUG */

    if (SMT_inst -> msgtosend == NULL)
	return OK;

    if (complete_SMT_frame(SMT_inst) == NOTOK)
	return NOTOK;

resend:;
    if ((frames = send_out_packet(SMT_inst, &numframes)) == NULL)
	return NOTOK;
 
    if ((k = parse_SMT_packet(frames, SMT_inst, oi, v, offset, numframes))
	== NOTOK)
    {
	free_SMT_frame(frames, numframes);
	return NOTOK;
    }

    if (k == RESEND)
    {
	free_SMT_frame(frames, numframes);
	goto resend;
    }

    free_SMT_frame(frames, numframes);
    return OK;
}


int parse_SMT_packet(frames, SMT_inst, oi, v, offset, numframes)
SMT_frame *frames;
SMT_impl *SMT_inst;
OI	oi;
register struct type_SNMP_VarBind *v;
int offset;
int numframes;
{
    register OS		os = oi -> oi_type -> ot_syntax;
    SMT_var_type *ptr = NULL;
    int 	i;
    SMT_frame  *frame;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered parse_SMT_packet");
#endif /* DEBUG */

    for (i = 0; i < numframes; i++)
    {
	frame = &frames[i];

        /*
         * The frame is for us, check for a valid response to the query.
	 */
	if ((SMT_inst -> msgtosend -> un.header.frame_class != 
	    frame -> un.header.frame_class) ||
	    (frame -> un.header.frame_type != FDDI_SMT_FT_RESPONSE))
	{
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_8,
	           "%s: Invalid response, check for another packet"), 
		   "parse_SMT_packet");
	    continue;
	}

	if ((frame -> rc != FDDI_SMT_RC_SUCC) && 
	    ((frame -> rc != FDDI_SMT_RC_SC) || 
	     (offset != type_SNMP_PDUs_commit)))
	{
	    /* 
	     * Test to see if it is responding to a not supported request 
	     */
	    if (frame -> rc != 6)
	        advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_10,
	               "%s: unexpected packet"), "parse_SMT_packet");
	    continue;
	}

	if ((offset == type_SNMP_PDUs_commit) && 
	    (frame -> rc == FDDI_SMT_RC_SC))
	    return (RESEND);
	else if (offset == type_SNMP_PDUs_commit) /* successful set */
	    return (OK);
	else /* successful get */
	{
	    if (frame -> list == NULL)
	    {
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_11,
		       "%s: packet does not contain enough sections"), 
		       "parse_SMT_packet");
		continue;
	    }

	    ptr = frame -> list;    

	    while (ptr)
	    {
		SMT_var_type *ptr2;
		int error = 0;

		ptr2 = SMT_inst -> msgtosend -> list;
		while (ptr2 && 
		       (ptr2->variableinfo->type!=ptr->variableinfo->type))
	            ptr2 = (SMT_var_type *)ptr2 -> next;

		if (ptr2 == NULL) 
		{
		    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_12,
			   "%s: Unrequested frame"), "parse_SMT_packet");
		    break;
		}

		switch (ptr2 -> SNMPvarsyntax)
		{
		    case FDDITIMENANO:
		    case FDDITIMEMILLI:
		    case INTEGER:
			{
			    unsigned int temp;
			    int result;

			    if (ptr2 -> reso)
				bcopy(&(ptr->variableinfo->data[4]), &temp, 4);
			    else bcopy(ptr -> variableinfo -> data, &temp, 4);

			    if (v -> value) 
			    {
				free_SMI_ObjectSyntax (v -> value);
				v -> value = NULL;
			    }

			    if (ptr2 -> SNMPvarsyntax == FDDITIMENANO)
			    {
				temp = ~(temp - 1);
				temp = temp * 80;
			    }

			    if (ptr2 -> SNMPvarsyntax == FDDITIMEMILLI)
				temp = temp * 8 / 10;

			    if (ptr->variableinfo->type==PMFMIBPORTMACINDICATED)
			    {
				int tflag = ((temp & 0x0000ff00) >> 8);
				int rflag = temp & 0x000000ff;

				temp = ((tflag << 1) | rflag) + 1;
			    }

		    /*
		     * This corrects for the enumeration of the type
		     * The mib uses a 1 base numbering and the card
		     * uses a 0 base numbering.
		     */
		    if (ptr->variableinfo->type==PMFMIBSMTECMSTATE ||
		        ptr->variableinfo->type==PMFMIBSMTCFSTATE ||
		        ptr->variableinfo->type==PMFMIBSMTSTATIONSTATUS ||
		        ptr->variableinfo->type==PMFMIBMACCURRENTPATH ||
		        ptr->variableinfo->type==PMFMIBMACDUPADDRESSTEST ||
		        ptr->variableinfo->type==PMFMIBMACDOWNSTREAMPORTTYPE ||
		        ptr->variableinfo->type==PMFMIBMACRMTSTATE ||
		        ptr->variableinfo->type==PMFMIBPORTMYTYPE ||
		        ptr->variableinfo->type==PMFMIBPORTNEIGHBORTYPE ||
		        ptr->variableinfo->type==PMFMIBPORTCURRENTPATH ||
		        ptr->variableinfo->type==PMFMIBPORTPMDCLASS ||
		        ptr->variableinfo->type==PMFMIBPORTCONNECTSTATE ||
		        ptr->variableinfo->type==PMFMIBPORTPCMSTATE ||
		        ptr->variableinfo->type==PMFMIBPORTPCWITHHOLD)
				temp++;

		    /*
		     * This corrects for the true false definitions in the
		     * card and the mib.  In the mib, true = 1 and false = 2.
		     * On the card, true = 1 and false = 0.
		     */
		    if (ptr->variableinfo->type==PMFMIBSMTSTATRPTPOLICY ||
		        ptr->variableinfo->type==PMFMIBSMTBYPASSPRESENT ||
		        ptr->variableinfo->type==PMFMIBSMTREMOTEDISCONNECTFLAG||
		        ptr->variableinfo->type==PMFMIBSMTPEERWRAPFLAG ||
		        ptr->variableinfo->type==PMFMIBMACDAFLAG ||
		        ptr->variableinfo->type==PMFMIBMACUNADAFLAG ||
		        ptr->variableinfo->type==PMFMIBMACFRAMEERRORFLAG ||
		        ptr->variableinfo->type==PMFMIBMACMAUNITDATAAVAILABLE ||
		        ptr->variableinfo->type==PMFMIBMACHARDWAREPRESENT ||
		        ptr->variableinfo->type==PMFMIBMACMAUNITDATAENABLE ||
		        ptr->variableinfo->type==PMFMIBMACNOTCOPIEDFLAG ||
		        ptr->variableinfo->type==PMFMIBPORTBSFLAG ||
		        ptr->variableinfo->type==PMFMIBPORTLERFLAG ||
		        ptr->variableinfo->type==PMFMIBPORTHARDWAREPRESENT)
				temp = (temp == 1) ? 1 : 2;

			    result=(*os->os_encode)(&temp, &v -> value);

			    if (result == NOTOK)
			    {
				advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_14,
		                       "%s: unable to encode value"), 
				       "parse_SMT_packet");
			        return (NOTOK);
			    }
			}
			break;

		    case OCTETSTRING:
			{
			    struct qbuf *value;
			    int result;
			    int len;
			    char strig[40];

			    if (ptr2 -> reso)
			    {
				if (ptr->variableinfo->type!=PMFMIBSMTUSERDATA)
				    bcopy(&(ptr->variableinfo->data[4]), strig,
					  8);
				else bcopy(&(ptr->variableinfo->data[4]), strig,
					   32);
			    }
			    else 
			    {
				if (ptr->variableinfo->type!=PMFMIBSMTUSERDATA)
				    bcopy(ptr->variableinfo -> data, strig, 8);
				else
				    bcopy(ptr->variableinfo -> data, strig, 32);
			    }

			    /*
			     * Handle the cases where string is of length 
			     * 3, 6, 8, or 32
			     */
		    if ((ptr->variableinfo->type==PMFMIBMACSMTADDRESS) ||
			(ptr->variableinfo->type==PMFMIBMACUPSTREAMNBR) ||
			(ptr->variableinfo->type==PMFMIBMACDOWNSTREAMNBR) ||
			(ptr->variableinfo->type==PMFMIBMACOLDUPSTREAMNBR)||
			(ptr->variableinfo->type==PMFMIBMACOLDDOWNSTREAMNBR))
			value = str2qb (&strig[2], 6, 1);
		    else if ((ptr->variableinfo->type!=PMFMIBSMTUSERDATA) &&
			    (ptr->variableinfo->type!=PMFMIBPORTREQUESTEDPATHS))
			value = str2qb (strig, 8, 1);
		    else if (ptr->variableinfo->type==PMFMIBSMTUSERDATA)
			value = str2qb (strig, 32, 1);
		    else value = str2qb (strig, 3, 1);

			    if (value == NULL) 
			    {
			        advise (SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL,
					GENERA_1, "%s: Out of Memory"), 
					"parse_SMT_packet");
			        return(NOTOK);
			    }

			    if (v -> value)
			    {
				free_SMI_ObjectSyntax (v -> value);
				v -> value = NULL;
			    }
			    result = (*os->os_encode)(value, &v->value);

			    /* free the memory used by value */
			    qb_free (value);

			    if (result == NOTOK)
			    {
				advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_14,
				       "%s: unable to encode value"),
				       "parse_SMT_packet");
			        return (NOTOK);
	    		    }
		        }
		        break;

		    default:
			advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_11,
			       "%s: packet does not contain enough sections"), 
			       "parse_SMT_packet");
			error = 1;
		        break;
		}
		if (error)
		    break;
		ptr = (SMT_var_type *)ptr -> next;
	    }
	    if (ptr == NULL)
		return(OK);
	}
    }

    return(NOTOK);
}


int parse_out_packet(frames, SMT_inst, numframes)
SMT_frame *frames;
SMT_impl *SMT_inst;
int numframes;
{
    SMT_var_type *ptr = NULL;
    int		done = 0;
    int		idx = 0;
    int		i;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered parse_out_packet");
#endif /* DEBUG */

    for (i = 0; i < numframes; i++)
    {
	/*
	 * The frame is for us, check for a valid response to the query.
	 */
	if ((SMT_inst -> msgtosend -> un.header.frame_class != 
	     frames[i].un.header.frame_class) ||
	    (frames[i].un.header.frame_type != FDDI_SMT_FT_RESPONSE))
	{
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_8,
		   "%s: Invalid response, check for another packet"),
		   "parse_out_packet");
	    continue;
	}

	if (frames[i].rc != FDDI_SMT_RC_SUCC)
	{
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_10,
		   "%s: unexpected packet"), "parse_out_packet");
	    continue;
	}

	if (frames[i].list == NULL) 
	{
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_11,
		   "%s: packet does not contain enough sections"), 
		   "parse_out_packet");
	    continue;
	}

	free_SMT_frame(SMT_inst -> msgtosend, 1);
	SMT_inst -> msgtosend = &frames[i];
        return(OK);
    }	

    return(NOTOK);
}

/* Initial fddi parameter get function */
struct interface *get_fddi(struct interface *is)
{
    int i;
    SMT_impl *newspace;
    SMT_impl *ptr;
    SMT_var_type *varlist;
    SMT_frame *frames = NULL;
    int numframes = 0;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered get_fddi");
#endif /* DEBUG */

    if ((newspace = (SMT_impl *)calloc(smtnumber+1, sizeof(SMT_impl))) == NULL)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
		"%s: Out of Memory"), "get_fddi");
	return(NULL);
    }

    if (smtnumber != 0)
	bcopy((char *)SMT_list, (char *)newspace, smtnumber*sizeof(SMT_impl));
    ptr = SMT_list;
    SMT_list = newspace;
    if (ptr)
	free (ptr);

    ptr = &SMT_list[smtnumber++];
    ptr -> snmpFddiSMTIndex = smtnumber;
    ptr -> ifentry = is;
    ptr -> msgtosend = NULL;
    ptr -> nummacs = 0;
    ptr -> numpaths = 0;
    ptr -> numports = 0;

    if (make_SMT_header(ptr, PMFGET) == NOTOK)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
	       "%s: %s failed for device, %s"), "get_fddi", "make_SMT_header",
	       is -> ndd_name);
	return(NULL);
    }
    
    /* Add entry for mac indexes */
    if (add_SMT_variable(ptr, PMFMIBMACINDEXES, -1, INTEGER, 1, NULL) == NOTOK)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
	       "%s: %s failed for device, %s"), "get_fddi", "add_SMT_variable",
	       is -> ndd_name);
	return(NULL);
    }

    /* Add entry for port indexes */
    if (add_SMT_variable(ptr, PMFMIBPORTINDEXES, -1, INTEGER, 1, NULL) == NOTOK)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
	       "%s: %s failed for device, %s"), "get_fddi", "add_SMT_variable",
	       is -> ndd_name);
	return(NULL);
    }

    if (complete_SMT_frame(ptr) == NOTOK)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
	       "%s: %s failed for device, %s"), "get_fddi", 
	       "complete_SMT_frame", is -> ndd_name);
	return(NULL);
    }

    if ((frames = send_out_packet(ptr, &numframes)) == NULL)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_18,
	       "%s: %s failed to transmit the packet"), "get_fddi", 
	       is -> ndd_name);
	return(NULL);
    }

    if (parse_out_packet(frames, ptr, numframes) == NOTOK)
    {
        free_SMT_frame(frames, numframes);
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_19,
	       "%s: %s failed to parse the packet"), "get_fddi", is->ndd_name);
	return(NULL);
    }

    varlist = ptr -> msgtosend -> list;
    while (varlist)
    {
	if (varlist -> variableinfo -> len % 2 == 1)
	{
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_16,
		   "get_fddi: Bad packet length"));
	    break;
	}

	switch (varlist -> variableinfo -> type)
	{
	    case PMFMIBMACINDEXES:
		ptr -> nummacs = varlist -> variableinfo -> len / 2;
		macnumber += ptr -> nummacs;
		if ((ptr -> macidnumbers = 
		    (unsigned short *)calloc(varlist -> variableinfo -> len / 2,
				    sizeof(unsigned short))) == NULL)
		{
		    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
			   "%s: Out of memory"), "get_fddi");
		    return(NULL);
		}

		bcopy(varlist -> variableinfo -> data, ptr -> macidnumbers, 
		      varlist -> variableinfo -> len);
		if (ptr -> macidnumbers[ptr -> nummacs - 1] == 0)
		{
		    ptr -> nummacs--;
		    macnumber--;
		}
		break;

	    case PMFMIBPORTINDEXES:
		ptr -> numports = varlist -> variableinfo -> len / 2;
		portnumber += ptr -> numports;
		if ((ptr -> portidnumbers = 
		    (unsigned short *)calloc(varlist->variableinfo -> len / 2,
				    sizeof(unsigned short))) == NULL)
		{
		    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
			   "%s: Out of memory"), "get_fddi");
		    return(NULL);
		}

		bcopy(varlist -> variableinfo -> data, ptr -> portidnumbers, 
		      varlist -> variableinfo -> len);
		if (ptr -> portidnumbers[ptr -> numports - 1] == 0)
		{
		    ptr -> numports--;
		    portnumber--;
		}
		break;
	}
	varlist = (SMT_var_type *)varlist -> next;
    }

    free_SMT_frame(frames, numframes);
    ptr -> msgtosend = NULL;

    if (make_SMT_header(ptr, PMFGET) == NOTOK)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
	       "%s: %s failed for device, %s"), "get_fddi", "make_SMT_header",
	       is -> ndd_name);
	return(NULL);
    }

    /* Add entry for path indexes */
    if (add_SMT_variable(ptr, PMFMIBPATHINDEXES, 0, INTEGER, 1, NULL) == NOTOK)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
	       "%s: %s failed for device, %s"), "get_fddi", "add_SMT_variable",
	       is -> ndd_name);
	return(NULL);
    }

    if (complete_SMT_frame(ptr) == NOTOK)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
	       "%s: %s failed for device, %s"), "get_fddi", 
	       "complete_SMT_frame", is -> ndd_name);
	return(NULL);
    }

    if ((frames = send_out_packet(ptr, &numframes)) == NULL)
    {
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_18,
	       "%s: %s failed to transmit the packet"), "get_fddi", 
	       is -> ndd_name);
	return(NULL);
    }

    if (parse_out_packet(frames, ptr, numframes) == NOTOK)
    {
        free_SMT_frame(frames, numframes);
	advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_19,
	       "%s: %s failed to parse the packet"), "get_fddi", is->ndd_name);
	return(NULL);
    }

    varlist = ptr -> msgtosend -> list;
    while (varlist)
    {
	switch (varlist -> variableinfo -> type)
	{
	    case PMFMIBPATHINDEXES:
		{
		    unsigned int *temppathids;

		    if ((temppathids = 
			(unsigned int *)calloc(ptr->numpaths + 1, 
					       sizeof(int))) == NULL)
		    {
			advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
				"%s: Out of Memory"), "get_fddi");
			return(NULL);
		    }

		    if (ptr -> numpaths)
			bcopy(ptr -> pathidnumbers, temppathids, 
			      ptr -> numpaths * sizeof(int));
		    bcopy(&varlist -> variableinfo -> data[4], 
			  &temppathids[ptr -> numpaths], sizeof(int));
		    ptr -> numpaths++;
		    if (ptr -> pathidnumbers)
			free(ptr -> pathidnumbers);
		    ptr -> pathidnumbers = temppathids;
		}
		break;
	}
	varlist = (SMT_var_type *)varlist -> next;
    }

    free_SMT_frame(frames, numframes);
    ptr -> msgtosend = NULL;

    if (smtnumber == 1)
    {
	/* Make sure the first fddi request, gets the path/token table */
	isdummyptr -> flush_cache = 1;
	isdummyptr -> timeout = 0;
	isdummyptr -> datarcvtable = NULL;
	isdummyptr -> time_next = NULL;
	return(isdummyptr);
    }
    return(NULL);
}


/* Used to get the token order on fddi device */
void get_fdditokens(void)
{
    register int i;
    SMT_frame *save_frame;

#ifdef DEBUG
advise(SLOG_EXCEPTIONS, "Entered get_fdditokens");
#endif /* DEBUG */

    update_timers(isdummyptr, fdditimeout, 0);

    for (i = 0; i < smtnumber; i++)
    {
	token_path *ptr = SMT_list[i].token_paths;

	while (ptr)
	{
	    token_path *ptr2 = ptr;

	    ptr = ptr -> next;
	    free(ptr2);
	}

	SMT_list[i].token_paths = NULL;
    }

    for (i = 0; i < smtnumber; i++)
    {
	register int j;
	register token_path *lastpath = NULL;
	SMT_frame *save_frame = SMT_list[i].msgtosend;
	SMT_frame *frames = NULL;
	int numframes = 0;

	SMT_list[i].msgtosend = NULL;
	for (j = 0; j < SMT_list[i].numpaths; j++)
	{
	    register token_path *ptr = NULL;
	    ushort len;
	    uchar *data;
	    u_short count = 0;
	    u_short pos = 4;

	    if (make_SMT_header(&SMT_list[i], PMFGET) == NOTOK)
	    {
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
		       "%s: %s failed for device, %s"), "get_fdditokens", 
		       "make_SMT_header", SMT_list[i].ifentry -> ndd_name);
		break;
	    }
    
	    /* Add entry for path config indexes */
	    if (add_SMT_variable(&SMT_list[i], PMFMIBPATHCONFIGURATION, 
				 SMT_list[i].pathidnumbers[j], 
				 INTEGER, 1, NULL) == NOTOK)
	    {
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
		       "%s: %s failed for device, %s"), "get_fdditokens", 
		       "add_SMT_variable", SMT_list[i].ifentry -> ndd_name);
		break;
	    }

	    if (complete_SMT_frame(&SMT_list[i]) == NOTOK)
	    {
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
		       "%s: %s failed for device, %s"), "get_fdditokens", 
		       "complete_SMT_frame", SMT_list[i].ifentry -> ndd_name);
		break;
	    }

	    if ((frames = send_out_packet(&SMT_list[i], &numframes)) == NULL)
	    {
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
		       "%s: %s failed for device, %s"), "get_fdditokens", 
		       "send_out_packet", SMT_list[i].ifentry -> ndd_name);
		break;
	    }

	    if (parse_out_packet(frames, &SMT_list[i], numframes) == NOTOK)
	    {
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_15,
		       "%s: %s failed for device, %s"), "get_fdditokens", 
		       "parse_out_packet", SMT_list[i].ifentry -> ndd_name);
		break;
	    }

	    if (SMT_list[i].msgtosend -> list -> variableinfo -> type !=
		PMFMIBPATHCONFIGURATION)
	    {
		free_SMT_frame(frames, numframes);
		SMT_list[i].msgtosend = save_frame;
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_FDDI, FDDI_17,
			"Bad packet for get_tokenpaths"));
		return;
	    }

	    len = SMT_list[i].msgtosend -> list -> variableinfo -> len - 4;
	    data = SMT_list[i].msgtosend -> list -> variableinfo -> data;
	    while (len > 0)
	    {
		if ((ptr = (token_path *)calloc(1, sizeof(token_path))) == NULL)
		{
		    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
			   "%s: Out of memory"), "get_tokenpaths");
		    free_SMT_frame(frames, numframes);
		    return;
		}

		ptr -> path_id = SMT_list[i].pathidnumbers[j];
		ptr -> token_num = ++count;
		bcopy(&data[pos],(char *)&ptr -> resource_type, sizeof(u_int));
		bcopy(&data[pos+4],(char *)&ptr->resource_id, sizeof(u_short));
		bcopy(&data[pos+6],(char *)&ptr->current_path, sizeof(u_short));
		ptr -> next = NULL;

		len -= 8;
		pos += 8;
		if (lastpath)
		{
		    lastpath -> next = ptr;
		    lastpath = ptr;
		}
		else
		{
		    SMT_list[i].token_paths = ptr;
		    lastpath = ptr;
		}
	    }

	    free_SMT_frame(frames, numframes);
	    SMT_list[i].msgtosend = NULL;
	}

	SMT_list[i].msgtosend = save_frame;
    }
}


#ifdef DEBUG
void print_fddi(instance)
SMT_impl *instance;
{
    int i;
    token_path *ptr;

    advise(SLOG_EXCEPTIONS, "FDDI INSTANCE DUMP");
    advise(SLOG_EXCEPTIONS, "  snmpFddiSMTIndex = %d", 
	   instance -> snmpFddiSMTIndex);
    advise(SLOG_EXCEPTIONS, "  nummacs = %d", 
	   instance -> nummacs);
    for (i = 0; i < instance -> nummacs; i++)
	advise(SLOG_EXCEPTIONS, "    %d - %d", i, instance -> macidnumbers[i]);

    advise(SLOG_EXCEPTIONS, "  numports = %d", 
	   instance -> numports);
    for (i = 0; i < instance -> numports; i++)
	advise(SLOG_EXCEPTIONS, "    %d - %d", i, instance -> portidnumbers[i]);

    advise(SLOG_EXCEPTIONS, "  numpaths = %d", 
	   instance -> numpaths);
    for (i = 0; i < instance -> numpaths; i++)
	advise(SLOG_EXCEPTIONS, "    %d - %d", i, instance -> pathidnumbers[i]);

    ptr = instance -> token_paths;
    advise(SLOG_EXCEPTIONS, "  token paths:");

    while (ptr)
    {
	advise(SLOG_EXCEPTIONS, "    tk path id: %d", ptr -> path_id);
	advise(SLOG_EXCEPTIONS, "    tk tok num: %d", ptr -> token_num);

	advise(SLOG_EXCEPTIONS, "      res_type: %d", ptr -> resource_type);
	advise(SLOG_EXCEPTIONS, "      res_id: %d", ptr -> resource_id);
	advise(SLOG_EXCEPTIONS, "      curr_path: %d", ptr -> current_path);

	ptr = ptr -> next;
    }

    advise(SLOG_EXCEPTIONS, "  Socket number: %d", instance -> s);
    advise(SLOG_EXCEPTIONS, "  ifentry = %ld", (long)instance -> ifentry);

    advise(SLOG_EXCEPTIONS, "  msgtosend = %ld", (long)instance -> msgtosend);
    print_fddi_frame(instance -> msgtosend, 1);
}

void print_fddi_frame(msgtosend, num)
SMT_frame *msgtosend;
int num;
{
    int i;
    int m;

    if (!msgtosend)
	return;

    for (m = 0; m < num; m++)
    {
	SMT_var_type *ptr = msgtosend[m].list;

	for (i = 0; i <= sizeof(fddi_smt_hdr_t) / 8; i++)
	{
	    int j;
	    char tmp2[80] = "";
	    char tmp[80] = "";

	    for (j = 0; j < 8; j++)
	    {
		if (i * 8 + j >= sizeof(fddi_smt_hdr_t))
		    break;
		sprintf(tmp, "%3x", ((char *)(&msgtosend[m].un.header))[i*8+j]);
		strcat(tmp2, tmp);
	    }	
	    advise(SLOG_EXCEPTIONS, "msgtosend->header %s", tmp2);
	}

	while (ptr)
	{
	    advise(SLOG_EXCEPTIONS, "Variable def code");
	    advise(SLOG_EXCEPTIONS, "reso=%d syntax=%d", ptr -> reso,
	       ptr -> SNMPvarsyntax);
	    advise(SLOG_EXCEPTIONS, "type=%d len =%d", 
	       ptr -> variableinfo -> type, ptr -> variableinfo -> len);

	    for (i = 0; i <= ptr -> variableinfo -> len / 8; i++)
	    {
		int j;
		char tmp2[80] = "";
		char tmp[80] = "";

		for (j = 0; j < 8; j++)
		{
		    if (i * 8 + j >= ptr -> variableinfo -> len)
			break;
		    sprintf(tmp, "%3x", ptr -> variableinfo -> data[i * 8 + j]);
		    strcat(tmp2, tmp);
		}
		advise(SLOG_EXCEPTIONS, "varinfo->data: %s", tmp2);
	    }

	    ptr = (SMT_var_type *)ptr -> next;
	}

	for (i = 0; i <= msgtosend[m].buflen / 8; i++)
	{
	    int j;
	    char tmp2[80] = "";
	    char tmp[80] = "";

	    for (j = 0; j < 8; j++)
	    {
		if (i * 8 + j >= msgtosend[m].buflen)
		    break;
		sprintf(tmp, "%3x", msgtosend[m].un.buffer[i * 8 + j]);
		strcat(tmp2, tmp);
	    }
	    advise(SLOG_EXCEPTIONS, "  SMT_frame buffer: %s", tmp2);
	}
    }
}
#endif /* DEBUG */

