static char sccsid[] = "@(#)27	1.5  src/tcpip/usr/sbin/snmpinfo/processt.c, snmp, tcpip411, GOLD410 3/3/93 08:32:32";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: processtrap()
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/sbin/snminfo/processt.c
 */
#ifdef LOCAL

#include <isode/pepsy/SNMP-types.h>
#include <isode/snmp/objects.h>
#include <isode/snmp/io.h>
#include <stdio.h>
#include <isode/snmp/logging.h>


int  
process_trap ()
{
    struct type_SNMP_Message *msg;
    int	    status, rc;
    register struct type_SNMP_Trap__PDU *parm;
    struct fdinfo *fi;
    struct qbuf *qb;
    long    clock;
    struct tm *tm;
    unsigned long addr;

    if ((fi = type2fi (FI_UDP, (struct fdinfo *) NULL)) 
	    == (struct fdinfo *) NULL) {
	advise (SLOG_EXCEPTIONS, "no active UDP port");
	return NOTOK;
    }

    if ((status = rcv_msg (&msg, fi -> fi_ps)) != OK)
	return status;

    switch (msg -> data -> offset) {
	case type_SNMP_PDUs_trap:
	    parm = msg -> data -> un.trap;
	    break;
	default:
	    advise (SLOG_EXCEPTIONS, "unexpected message type %d",
		    msg -> data -> offset);
	    status = NOTOK;
	    goto out;
    }

    rc = parm -> generic__trap;
    if (qb_pullup(parm -> agent__addr) == NOTOK || 
       (qb = parm -> agent__addr -> qb_forw) -> qb_len != 4)
       return NOTOK;

    addr = *((unsigned long *)qb -> qb_data);
    (void) time (&clock);
    tm = localtime (&clock);
    printf ("%2d/%02d %2d:%02d:%02d  %s (%s)\n",
	    tm -> tm_mon + 1, tm -> tm_mday,
	    tm -> tm_hour, tm -> tm_min, tm -> tm_sec,
	    lookup_addr (&addr), sprintoid (parm -> enterprise));
    printf ("\t\tTicks: <%d>  Type: ",
	    parm -> time__stamp -> parm);

    switch (rc) {
	case 0:
		printf("[coldStart]");
		break;
	case 1:
		printf("[warmStart]");
		break;
	case 2:
		printf("[linkDown]");
		break;
	case 3:
		printf("[linkUp]");
		break;
	case 4:
		printf("[authenticationFailure]");
		break;
	case 5:
		printf("[egpNeighborLoss]");
		break;
	case 6:
		printf("[enterpriseSpecific] [%d]",
		    parm -> specific__trap);
		break;
	default :
		printf("[ERROR: trap code]");
		break;
	}
    printf ("\n");

    status = process_varbinds (parm -> variable__bindings, NULLOID);

 out: ;
    if (msg)
	free_SNMP_Message (msg);
    return (status);
}
#endif /* LOCAL */
