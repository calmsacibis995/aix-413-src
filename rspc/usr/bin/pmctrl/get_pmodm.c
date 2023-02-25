static char sccsid[] = "@(#)93  1.2  src/rspc/usr/bin/pmctrl/get_pmodm.c, pwrcmd, rspc41J, 9513A_all 3/28/95 16:33:15";
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS: get_pmmi_data, change_pmmi_data
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <odmi.h>
#include "pmodm.h"

/*
 * NAME:      get_pmmi_data()
 *
 * FUNCTION:  Get pm odm data --  pmmi_data object class
 *
 * DATA STRUCTURES:
 *              att  -- attribute name
 *              dflt -- default value
 *              val  -- current setting
 *
 * RETURN VALUE DESCRIPTION:
 *               0: success
  *              -1  : error
 *
 * NOTE:      ODM should be initialized,
 */

int get_pmmi_data(char *att, int *dflt, int *val)
{
    struct pmmi_data pmmidata;
    char   criteria[256];

	 /* get odm data with attribute */
    sprintf(criteria, "attribute = %s", att);
    if (odm_get_obj(pmmi_data_CLASS, criteria, &pmmidata, ODM_FIRST) <= 0) {
        return(-1); 
		  /* error or not found          */
    }

    if (dflt!=NULL) *dflt = pmmidata.value1;
    if (val!=NULL) *val  = pmmidata.value2;

    return(0);
}


/*
 * NAME:      change_pmmi_data()
 *
 * FUNCTION:  Change pm odm data --  pmmi_data object class
 *
 * DATA STRUCTURES:
 *              att  -- attribute name
 *              val  -- new setting (value2 in pmmi_data)
 *
 * RETURN VALUE DESCRIPTION:
 *               0: success
 *               -1  : error
 */

int change_pmmi_data(char *att, int val)
{
    struct pmmi_data pmmidata;
    char   criteria[256];

	 /* get odm data with attribute */
    sprintf(criteria, "attribute = %s", att);
    if (odm_get_obj(pmmi_data_CLASS, criteria, &pmmidata, ODM_FIRST) <= 0) {
        return(-1);
    }

    pmmidata.value2 = val;

	 /* change odm with new value   */
    if (odm_change_obj(pmmi_data_CLASS, &pmmidata) == -1) {
        return(-1);
    }

    return(0);
}

