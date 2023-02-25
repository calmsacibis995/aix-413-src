static char sccsid[] = "@(#)60  1.1  src/rspc/kernext/pm/pmmd/6030/battery.c, pwrsysx, rspc411, 9435A411a 8/29/94 18:34:32";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: pm_battery_control,battery_query,battery_discharge
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/errno.h>

#include <sys/pm.h>

extern int set_battery_discharge();
extern struct pm_battery *get_battery_status();

/*
 * NAME:  pm_battery_control
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine is entry point of the pm_control_parameter system call.
 *
 * DATA STRUCTURES:
 *        pm_battery - PM battery information structure
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS - Successfully processed
 *        PM_ERROR - Invalid cmd argument or falied to process
 *                   battery functions.
 */
int
pm_battery_control(int cmd, struct pm_battery *battery)
{
    char ep;

    if( suser( &ep ) == 0 ) {
	setuerror(ep);
	return PM_ERROR;
    }

    switch( cmd ) {
      case PM_BATTERY_DISCHARGE:
	return battery_discharge();

      case PM_BATTERY_QUERY:
	return battery_query(battery);

      default:
	setuerror(EINVAL);
	return PM_ERROR;
    }
}


/*
 * NAME:  battery_discharge
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS - Successfully processed
 *        PM_ERROR - Falied to call battery discharge function.
 */
static int
battery_discharge()
{
    if( set_battery_discharge() != PM_SUCCESS ) {
	setuerror(EIO);
	return PM_ERROR;
    } else {
	return PM_SUCCESS;
    }
}


/*
 * NAME:  battery_query
 *
 * FUNCTION:  PM system call
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *        pm_battery - PM battery information structure
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS - Successfully processed
 *        PM_ERROR - Falied to call battery status function.
 */
static int
battery_query(struct pm_battery *battery)
{
    struct pm_battery *pb;
    
    if( (pb = get_battery_status()) == NULL ) {
	setuerror(EIO);
	return PM_ERROR;
    }

    if( copyout(pb, battery, sizeof(struct pm_battery)) == 0 ) {
	return PM_SUCCESS;
    } else {
	return PM_ERROR;
    }
}
