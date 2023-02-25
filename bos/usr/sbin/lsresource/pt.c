static char sccsid[] = "@(#)27  1.3  src/bos/usr/sbin/lsresource/pt.c, cmdcfg, bos41J, 9520A_all 5/7/95 11:22:21";
/*
 *   COMPONENT_NAME: (LIBCFG) PRINT TABLES Module
 *
 *   FUNCTIONS:
 *		print_user_attributes
 *		print_resource_summary
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cf.h>
#include <memory.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include "adapter.h"
#include "bt.h"

#define VALUE_FIELD_WIDTH 24

/*------------------*/
/* Global Variables */
/*------------------*/
static char Tabs[80];

/* Keep module independent of definitions for NAMESIZE, ATTRNAMESIZE */
static char Lname[80], Aname[80];

/* Column widths for printing (not counting for null character on string) */
static int Col_Type = 4;
static int Col_Adapter = NAMESIZE - 1;
static int Col_Attr = ATTRNAMESIZE - 1;
static int Col_S = 1;
static int Col_G = 1;
static int Col_Value = VALUE_FIELD_WIDTH - 1;

/*----------------------*/
/* Forward declarations */
/*----------------------*/
extern char *get_nls_msg();
void print_user_attributes();
void print_resource_summary();
static void set_tabs();
static void sprintf_values();
static void sprintf_names();
static void log_resource_stanza();
static void prt_resource_hdr();

/***************************************************************************/
/*                     EXTERNALLY VISIBLE FUNCTIONS                        */
/***************************************************************************/

/***************************************************************************/
/* Function Name : PRINT_USER_ATTRIBUTES()                                 */
/* Function Type : C function                                              */
/* Purpose       : Log current resource value information to stdout for    */
/*                 user settable parameters.                               */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/* Arguments     :                                                         */
/*  tabdepth     - tabs into beginning of each line                        */
/*  attr_head    - pointer to head of attributes list                      */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void print_user_attributes(tabdepth, attr_head)
	int tabdepth;
	attribute_t *attr_head;
{
	attribute_t *attr_p, *attr_p1;
	int modified_attrs = FALSE;
	char valstr[VALUE_FIELD_WIDTH];

	set_tabs(tabdepth, 0);

	/* Identify columns */
	printf("%s", Tabs);
	printf("%s", get_nls_msg(20));
	printf("%s", Tabs);
	printf("--------------- --------------- ------------------------\n");
	
	for (attr_p = attr_head ; attr_p ; attr_p = attr_p->next)
	{

		/* If this device didn't resolve then skip it */
		if (attr_p->adapter_ptr->unresolved)
			continue;
	
		/* Use attr_p1 to handle GROUP attributes */
	
		if (attr_p->resource == GROUP)
			attr_p1 = attr_p->group_ptr;
		else
			attr_p1 = attr_p;
	
		for ( ; attr_p1 ; attr_p1 = attr_p1->group_ptr)
		{
		
			/* If not a user attribute or not changed skip it */
			if (!attr_p1->user || !attr_p1->changed) 
				continue;

			/* If it is a wrapped attribute skip it */
			if (attr_p1->reprsent == LIST)
			{
				if (attr_p1->start.list.ptr->value == attr_p1->current)
					continue;
			}
			else /* attr_p1->reprsent == RANGE */
			{
				if (attr_p1->start.range.value == attr_p1->current)
					continue;
			}

			modified_attrs = TRUE;
			sprintf_names(attr_p1->adapter_ptr->logname, attr_p1->name);
			sprintf_values(valstr, attr_p1, attr_p1->current);
			printf("%s %s %s %s\n", Tabs, Lname, Aname, valstr);
		}
	}

	if (!modified_attrs)
		printf("%s%s", Tabs, get_nls_msg(21));
  
	return;

} /* end of print_user_attributes() */

/***************************************************************************/
/* Function Name : PRINT_RESOURCE_SUMMARY()                                */
/* Function Type : C function                                              */
/* Purpose       : Log current resource value information to the log file  */
/*                 for all attributes attributes, ordered by bus resource. */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/* Arguments     :                                                         */
/*  tabdepth     - tabs into beginning of each line                        */
/*  attr_head    - pointer to head of attributes list                      */
/*  adap_last    - OPTIONAL pointer to last adapter of attributes to print */  
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void print_resource_summary(tabdepth, attr_head, adap_last)
	int tabdepth;
	attribute_t *attr_head;
	adapter_t *adap_last;
{
	bus_resource_e resource; 
	attribute_t *attr_p, *attr_p2, *attr_last = NULL;
	int sharecnt, groupcnt;

	set_tabs(tabdepth, 0);

	/* Find the last attribute to terminate printing and print header string */
	if (adap_last != NULL)
	{

		/* If this adapter has no attributes don't print the summary table */
		if (adap_last->attributes == NULL)
		{
			printf("%s", get_nls_msg(22));
			return;
		}

		/* If this adapter is only DEFINED, don't print the summary table */
		if (attr_head->adapter_ptr->status == DEFINED)
		{
			printf("%s", get_nls_msg(26));
			return;
		}

		/* Find the attribute to stop printing at */
		for (attr_last = adap_last->attributes ; attr_last ; attr_last = attr_last->next)
			if (attr_last->adapter_ptr != adap_last)
				break;
	}
	else {
		/* If device is not listed in busresolve table, */
		/* don't print the summary table (example: hdisk0) */
		if (attr_head == NULL)
		{
			printf("%s", get_nls_msg(22));
			return;
		}

	}

	/* print headers for table listing */
	prt_resource_hdr();

	for (resource = MADDR ; resource <= DMALVL ; resource++) 
	{

		groupcnt = 0;
		sharecnt = 0;

		for (attr_p = attr_head ; attr_p != attr_last ; attr_p = attr_p->next)
		{

			if (attr_p->adapter_ptr->unresolved || attr_p->ignore)
				continue;

			/* only print AVAILABLE/DIAGNOSTIC Devices */
			if (attr_p->adapter_ptr->status == DEFINED)
				continue;

			if (attr_p->resource == GROUP)
			{
				++groupcnt;

				for (attr_p2 = attr_p->group_ptr ; attr_p2 ; attr_p2 = attr_p2->group_ptr)
				{
					if (attr_p2->specific_resource != resource) 
						continue;

					log_resource_stanza(attr_p2, 0, groupcnt);
				}

			}
			else if (attr_p->share_head)
			{
				++sharecnt;

				if (attr_p->specific_resource != resource) 
					continue;

				/* Print the share list head attribute */
				log_resource_stanza(attr_p, sharecnt, 0);

				/* Print the subsequent shared attributes */
				for (attr_p2 = attr_p->share_ptr ; attr_p2 ; attr_p2 = attr_p2->share_ptr)
				{
					if (attr_p2->adapter_ptr->unresolved)
						continue;

					/* Sync the subsequent shared attribute for logging */
					if (attr_p->changed)
					{
						if (!attr_p2->changed)
						{
							attr_p2->changed = TRUE;
							if (attr_p2->reprsent == LIST)
								attr_p2->start.list.ptr = attr_p2->values.list.current;
							else /* attr_p2->reprsent == RANGE */
								attr_p2->start.range.value = attr_p2->current;
						}
						attr_p2->current = attr_p->current;
					}
					log_resource_stanza(attr_p2, sharecnt, 0);
				}

			}
			else
			{
				if (attr_p->specific_resource != resource)
					continue;

				log_resource_stanza(attr_p, 0, 0);
			}

		}
	}

	return;
} /* end of print_resource_summary() */

/***************************************************************************/
/*                         STATIC FUNCTIONS                                */
/***************************************************************************/

/***************************************************************************/
/* Function Name : SET_TABS()                                              */
/* Function Type : Internal C function                                     */
/* Purpose       : Set up global tabstring                                 */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/* Arguments     :                                                         */
/*   tabdepth    - Tabs into beginning of each line (external parameter)   */
/*   align       - Internal format alignment tabs                          */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void set_tabs(tabdepth, align)
	int tabdepth, align;
{

	memset(Tabs, (int)'\t', sizeof(Tabs));

	tabdepth = (tabdepth > sizeof(Tabs) - align - 1) ? sizeof(Tabs) - align - 1 : tabdepth;
	Tabs[tabdepth + align] = '\0';

	return;
} /* end of set_tabs() */

/***************************************************************************/
/* Function Name : SPRINTF_NAMES()                                         */
/* Function Type : Internal C function                                     */
/* Purpose       : Set up global logical and attribute name strings, which */
/*                 are used to keep this module independent of the compiler*/
/*                 definitions for NAMESIZE and ATTRNAMESIZE.              */ 
/* Global Vars   :                                                         */
/*   Lname       - Adapter logical name string                             */
/*   Aname       - Attribute name string                                   */
/*   Tabs        - Tabstring                                               */
/*   Col_Adapter - width of column for Adapter field                       */
/*   Col_Attr    - width of column for Attr    field                       */
/* Arguments     :                                                         */
/*   lname       - Adapter logical name                                    */
/*   aname       - Attribute name                                          */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void sprintf_names(lname, aname)
	char *lname, *aname;
{

	if (lname != NULL)
	{
		memset(Lname, (int)' ', sizeof(Lname) - 1);

		if (strlen(lname) < Col_Adapter)
			memcpy(Lname, lname, strlen(lname));
		else
			memcpy(Lname, lname, Col_Adapter);

		Lname[Col_Adapter] = '\0';
	}

	if (aname != NULL)
	{
		memset(Aname, (int)' ', sizeof(Aname) - 1);

		if (strlen(aname) < Col_Attr)
			memcpy(Aname, aname, strlen(aname));
		else
			memcpy(Aname, aname, Col_Attr);

		Aname[Col_Attr] = '\0';
	}

	return;
} /* end of sprintf_names() */

/***************************************************************************/
/* Function Name : SPRINTF_VALUES()                                        */
/* Function Type : Internal C function                                     */
/* Purpose       : Log current resource value information to the log file  */
/* Arguments     :                                                         */
/*  attr_p       - pointer to attributes struct                            */
/*  string       - VALUE_FIELD_WIDTH byte string to print values to        */
/*  value        - current value to use                                    */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void sprintf_values(string, attr_p, value)
	char string[VALUE_FIELD_WIDTH];
	attribute_t *attr_p;
	unsigned long value;
{

	switch (attr_p->specific_resource)
	{
		case MADDR    :
		case BADDR    :
		case IOADDR   :
			if (attr_p->width > 1)
				sprintf(string, "0x%08x - 0x%08x", value, value + attr_p->width - 1);
			else
				sprintf(string, "      0x%08x       ", value);
			break;
		case INTLVL   :
		case NSINTLVL :
			switch ((int)INTR_CNTRLR(value))
			{
				case AT_BYTE   :
					sprintf(string, "        %s%d.%lu       ", "AT", INTR_INSTNC(value),
									INTR_PRINT(value));
					break;
				case MPIC_BYTE :
					sprintf(string, "       %s%d.%lu      ", "MPIC", INTR_INSTNC(value),
									INTR_PRINT(value));
					break;
				default        :
					sprintf(string, "         %3lu          ", INTR_PRINT(value));
			}
			break;
		case DMALVL   :
			if (attr_p->width > 1)
				sprintf(string, "        %2lu - %-2lu        ", value, value + attr_p->width - 1);
			else
				sprintf(string, "          %2lu           ", value);
		}
	
	return;
} /* end of sprintf_values() */

/***************************************************************************/
/* Function Name : LOG_RESOURCE_STANZA()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : Log a single resource allocation summary stanza.        */
/* Global Vars   :                                                         */
/*   Lname       - Adapter logical name string                             */
/*   Aname       - Attribute name string                                   */
/*   Tabs        - Tabstring                                               */
/*   Col_Type    - width of column for Type    field                       */
/*   Col_S       - width of column for S       field                       */
/*   Col_G       - width of column for G       field                       */
/* Arguments     :                                                         */
/*  attr_p       - attribute structure pointer                             */
/*  share        - share index number                                      */
/*  group        - group index number                                      */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void log_resource_stanza(attr_p, share, group)
	attribute_t *attr_p;
	int share, group;
{
	char resrc[40];
	char sharestr[40], groupstr[40], valstr[VALUE_FIELD_WIDTH];
	int i;

	sprintf_names(attr_p->adapter_ptr->logname, attr_p->name);

	memset(resrc, (int)' ', sizeof(resrc) - 1);
	resrc[Col_Type] = '\0';

	switch (attr_p->specific_resource)
	{
		case MADDR    : resrc[0] = 'M'; break;
		case BADDR    : resrc[0] = 'B'; break;
		case IOADDR   : resrc[0] = 'O'; break;
		case INTLVL   : resrc[0] = 'I'; break;
		case NSINTLVL : resrc[0] = 'N'; break;
		case DMALVL   : resrc[0] = 'A';
	}

	memset(sharestr, (int)' ', sizeof(sharestr) - 1);
	if (share)
		sprintf(sharestr, "%d", share);
	for (i=0; i<sizeof(sharestr) - 1; i++) {
		if (sharestr[i] == '\0') sharestr[i] = ' ';
	}
	sharestr[Col_S] = '\0';

	memset(groupstr, (int)' ', sizeof(groupstr) - 1);
	if (group)
		sprintf(groupstr, "%d", group);
	for (i=0; i<sizeof(groupstr) - 1; i++) {
		if (groupstr[i] == '\0') groupstr[i] = ' ';
	}
	groupstr[Col_G] = '\0';

	printf("%s", Tabs);

	printf("%s %s %s %s %s ", resrc, Lname, Aname, sharestr, groupstr);

	sprintf_values(valstr, attr_p, attr_p->current);
	printf("%s\n", valstr);

	return;
} /* end of log_resource_stanza() */


/***************************************************************************/
/* Function Name : PRT_RESOURCE_HDR()                                      */
/* Function Type : Internal C function                                     */
/* Purpose       : Print resource listing header.                          */
/* Global Vars   :                                                         */
/*   Col_Type    - width of column for Type    field                       */
/*   Col_Adapter - width of column for Adapter field                       */
/*   Col_Attr    - width of column for Attr    field                       */
/*   Col_S       - width of column for S       field                       */
/*   Col_G       - width of column for G       field                       */
/*   Col_Value   - width of column for Value   field                       */
/*   Tabs        - Tabstring                                               */
/* Arguments     :                                                         */
/*  -            -                                                         */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void prt_resource_hdr()
{
	char temp_str[80];
	int  temp_length;

	printf("%s", Tabs);

	/**********************
	 * print TYPE column
	 **********************/
	memset(temp_str, (int)' ', sizeof(temp_str) - 1);

	temp_length = strlen(get_nls_msg(23));
	if (temp_length > Col_Type) {
		if (temp_length > sizeof(temp_str)) {
			temp_length = sizeof(temp_str) - 1;
		}
		Col_Type = temp_length;
	}

	memcpy(temp_str, get_nls_msg(23), temp_length);
	temp_str[Col_Type] = '\0';
	printf("%-s ", temp_str);

	/**********************
	 * print ADAPTER column
	 **********************/
	memset(temp_str, (int)' ', sizeof(temp_str) - 1);

	temp_length = strlen(get_nls_msg(27));
	if (temp_length > Col_Adapter) {
		if (temp_length > sizeof(temp_str)) {
			temp_length = sizeof(temp_str) - 1;
		}
		Col_Adapter = temp_length;
	}

	memcpy(temp_str, get_nls_msg(27), temp_length);
	temp_str[Col_Adapter] = '\0';
	printf("%-s ", temp_str);

	/**********************
	 * print ATTRIBUTE column
	 **********************/
	memset(temp_str, (int)' ', sizeof(temp_str) - 1);

	temp_length = strlen(get_nls_msg(28));
	if (temp_length > Col_Attr) {
		if (temp_length > sizeof(temp_str)) {
			temp_length = sizeof(temp_str) - 1;
		}
		Col_Attr = temp_length;
	}

	memcpy(temp_str, get_nls_msg(28), temp_length);
	temp_str[Col_Attr] = '\0';
	printf("%-s ", temp_str);

	/**********************
	 * print S column
	 **********************/
	memset(temp_str, (int)' ', sizeof(temp_str) - 1);

	temp_length = strlen(get_nls_msg(29));
	if (temp_length > Col_S) {
		if (temp_length > sizeof(temp_str)) {
			temp_length = sizeof(temp_str) - 1;
		}
		Col_S = temp_length;
	}

	memcpy(temp_str, get_nls_msg(29), temp_length);
	temp_str[Col_S] = '\0';
	printf("%-s ", temp_str);

	/**********************
	 * print G column
	 **********************/
	memset(temp_str, (int)' ', sizeof(temp_str) - 1);

	temp_length = strlen(get_nls_msg(30));
	if (temp_length > Col_G) {
		if (temp_length > sizeof(temp_str)) {
			temp_length = sizeof(temp_str) - 1;
		}
		Col_G = temp_length;
	}

	memcpy(temp_str, get_nls_msg(30), temp_length);
	temp_str[Col_G] = '\0';
	printf("%-s ", temp_str);

	/**********************
	 * print CURRENT VALUE column
	 **********************/
	memset(temp_str, (int)' ', sizeof(temp_str) - 1);

	temp_length = strlen(get_nls_msg(31));
	if (temp_length > Col_Value) {
		if (temp_length > sizeof(temp_str)) {
			temp_length = sizeof(temp_str) - 1;
		}
		Col_Value = temp_length;
	}

	memcpy(temp_str, get_nls_msg(31), temp_length);
	temp_str[Col_Value] = '\0';
	printf("%-s\n", temp_str);

	printf("%s", Tabs);

	memset(temp_str, (int)'-', Col_Type);
	temp_str[Col_Type] = '\0';
	printf("%s ", temp_str);

	memset(temp_str, (int)'-', Col_Adapter);
	temp_str[Col_Adapter] = '\0';
	printf("%s ", temp_str);

	memset(temp_str, (int)'-', Col_Attr);
	temp_str[Col_Attr] = '\0';
	printf("%s ", temp_str);

	memset(temp_str, (int)'-', Col_S);
	temp_str[Col_S] = '\0';
	printf("%s ", temp_str);

	memset(temp_str, (int)'-', Col_G);
	temp_str[Col_G] = '\0';
	printf("%s ", temp_str);

	memset(temp_str, (int)'-', Col_Value);
	temp_str[Col_Value] = '\0';
	printf("%s\n", temp_str);

	return;
} /* end of prt_resource_hdr() */

