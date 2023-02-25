static char sccsid[] = "@(#)26  1.2  src/bos/usr/sbin/lsresource/lsresource.c, cmdcfg, bos411, 9438A411a 9/16/94 15:39:52";
/*
 * COMPONENT_NAME: (CMDCFG)  Device specific config support cmds
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <sys/types.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#include "bt.h"
#include "adapter.h"

#ifdef MSG
#include <nl_types.h>
#include <locale.h>
#endif /* MSG */

#define BR_NLS_CAT "cmdcfg.cat"
#define BR_NLS_SET  4

/*------------------*/
/* Global variables */
/*------------------*/

static struct _catalog_descriptor *BusResNLSCat;
extern adapter_t *AdapterList;
extern attribute_t *AttributeList;

/*----------------------*/
/* Forward declarations */
/*----------------------*/

char *get_nls_err_msg();
char *get_nls_msg();

static int br_parse();

/***************************************************************************/
/* Function Name : MAIN()                                                  */ 
/* Function Type : C program entry point                                   */
/* Purpose       : Provides a utility to print busresolve information      */ 
/* Arguments     :                                                         */ 
/*  argc          - Argument count                                         */
/*  argv          - Argument vector                                        */
/*                                                                         */
/* Return Value  : Documented in the array BusResDefltMsg in this module   */
/*                                                                         */
/***************************************************************************/
int main(argc, argv)
	int argc;
	char *argv[];
{
	int lock_id, flags, rc, resolve, listall;
	char conf_list[1024], not_res_list[1024], logname[NAMESIZE + 1];
	char *lname, *bname;

	/*------------------*/
	/* Initialize stuff */
	/*------------------*/

	/* Lock the database */
	lock_id = odm_lock("/etc/objrepos/config_lock", ODM_WAIT);
	if (lock_id == -1)
	{
		printf(get_nls_err_msg(E_ODMLOCK));
		return E_ODMLOCK;
	}

#ifdef MSG
	setlocale(LC_ALL,"");

	/* Open the NLS message catalogue */
	BusResNLSCat = catopen(BR_NLS_CAT, NL_CAT_LOCALE);
#endif /* MSG */

	/* Initialize the ODM */
	rc = odm_initialize();
	if (rc == -1)
	{
		printf(get_nls_err_msg(E_ODMINIT));
		if (BusResNLSCat != (struct _catalog_descriptor *)-1)
			catclose(BusResNLSCat);
		odm_unlock(lock_id);
		return E_ODMINIT;
	}

	/*--------------------------*/
	/* Parse & check user input */
	/*--------------------------*/

	rc = br_parse(argc, argv, logname, &resolve, &listall);
	if (rc != E_OK)
	{
		printf(get_nls_err_msg(rc));
		odm_terminate();
		if (BusResNLSCat != (struct _catalog_descriptor *)-1)
			catclose(BusResNLSCat);
		odm_unlock(lock_id);
		return rc;
	}

	/*---------------------------------------*/
	/* Call busresolve in informational mode */
	/*---------------------------------------*/

	/* Set up flags parameter */
	flags = COMMAND_MODE_FLAG;
	if (resolve)
		flags |= RESOLVE_MODE_FLAG;

	/* Call busresolve */
	rc = busresolve(logname, flags, conf_list, not_res_list, NULL);
	if (rc != E_OK)
	{
		printf(get_nls_err_msg(rc));
		if (rc != E_BUSRESOURCE)
			return rc;
	}

	/* Clean up */
	odm_terminate();
	odm_unlock(lock_id);

	/*--------------*/
	/* Print output */
	/*--------------*/
	if (resolve)
	{
		printf("%s", get_nls_msg(17));
		printf("\t%s\n", conf_list);
		printf("%s", get_nls_msg(18));
		if (not_res_list[0] == NULL) {
			printf("\t%s\n", get_nls_msg(25));
		}
		else {
			printf("\t%s\n", not_res_list);
			printf("%s", get_nls_msg(19));
			print_user_attributes(1, AttributeList);
		}
	}
	else if (listall)
		print_resource_summary(1, AttributeList, NULL);
	else
	{
		adapter_t *adap_head;
		attribute_t *attr_head, *attr_p1, *attr_p2, *attr_p3, *attr_p4;

		/* Find head of logname's attribute list */
		for (attr_head = AttributeList ; attr_head ; attr_head = attr_head->next)
			if (!strcmp(attr_head->adapter_ptr->logname, logname))
				break;

		/* Find logname's adapter node */
		for (adap_head = AdapterList ; adap_head ; adap_head = adap_head->next)
			if (!strcmp(adap_head->logname, logname))
				break;

		/* Relink all shared attributes for printing */
		for (attr_p1 = AttributeList ; attr_p1 ; attr_p1 = attr_p1->next)
		{
			if (!attr_p1->share_head)
				continue;

			if (!strcmp(attr_p1->adapter_ptr->logname, logname))
				continue;

			for (attr_p2 = attr_p1->share_ptr ; attr_p2 ; attr_p2 = attr_p2->share_ptr)
			{
				if (strcmp(attr_p2->adapter_ptr->logname, logname))
					continue;

				for (attr_p3 = attr_p1 ; attr_p3 ; attr_p3 = attr_p3->share_ptr)
				{
					if (attr_p3->share_ptr != attr_p2)
						continue;

					attr_p2->share_head = TRUE;
					attr_p1->share_head = FALSE;

					attr_p2->ignore = FALSE;
					attr_p1->ignore = TRUE;

					attr_p3->share_ptr = attr_p1;
					attr_p4            = attr_p2->share_ptr;
					attr_p2->share_ptr = attr_p1->share_ptr;
					attr_p1->share_ptr = attr_p4;
				}

				/* Only one share list head is sufficient! */
				break;
			}
		}

		print_resource_summary(1, attr_head, adap_head);
	}

	if (BusResNLSCat != (struct _catalog_descriptor *)-1)
		catclose(BusResNLSCat);

	return rc;
} /* end of main() */

/***************************************************************************/
/* Function Name : BR_PARSE()                                              */ 
/* Function Type : Internal C function                                     */
/* Purpose       : Parse command line input                                */ 
/* Arguments     :                                                         */ 
/*  argc          - Argument count                                         */
/*  argv          - Argument vector                                        */
/*  logname       - Device logical name returned (need NAMESIZE storage)   */
/*  resolve       - Bool resolve logname bus resources returned            */
/*  listall       - Bool list all devices flag                             */
/*                                                                         */
/* Return Value  :                                                         */
/*  E_OK          - No explaination needed                                 */
/*  E_ARGS        - A syntax error was detected                            */
/*                                                                         */
/***************************************************************************/
static int br_parse(argc, argv, logname, resolve, listall)
	int argc;
	char *argv[], *logname;
	int *resolve;
	int *listall;
{  
	int c, lflag = FALSE;
	extern int optind;
	extern char *optarg;

	*resolve = FALSE;
	*listall = FALSE;

	/* resolve function not to be turned on yet */
	/* while ((c = getopt(argc, argv, "rl:")) != -1) */
	while ((c = getopt(argc, argv, "al:")) != -1)
	{  
		switch (c)
		{
			case 'l' : /* logname supplied */
				lflag = TRUE;
				strncpy(logname, optarg, NAMESIZE);
				break;
			case 'a' : /* list all devices */
				*listall = TRUE;
				break;
			case 'r' : /* resolve attributes */
				*resolve = TRUE;
				break;
			case '?' : /* help */
			default  : /* syntax error */
				return E_ARGS;
		} /* switch (c) */
	} /* while getopt */

	/* Arguments left to process */
	if (optind < argc)
		return E_ARGS;

	if (!lflag)
		return E_ARGS;

	return E_OK;
}

/***************************************************************************/
/* Function Name : GET_NLS_ERR_MSG                                         */
/* Function Type : Internal C function                                     */
/* Purpose       : Convert error message number to catalog message number. */
/*                 Then gets message.                                      */
/* Arguments     :                                                         */
/*  err_num         - message number                                       */
/*                                                                         */
/* Notes         : Error messages correspond to cf.h return codes.         */
/***************************************************************************/
char *get_nls_err_msg(err_num)
	int err_num;
{
	int message_num;

	/* set msgno to correspond to correct message catalog number */
	switch(err_num)
	{
		case E_ODMINIT      :           /* cf.h value =  2  */
			message_num = 1;
			break;
		case E_ODMLOCK      :           /* cf.h value =  3  */
			message_num = 2;
			break;
		case E_ODMOPEN      :           /* cf.h value =  4  */
			message_num = 3;
			break;
		case E_ODMGET       :           /* cf.h value =  6  */
			message_num = 4;
			break;
		case E_ARGS         :           /* cf.h value = 11  */
			message_num = 5;
			break;
		case E_OPEN         :           /* cf.h value = 12  */
			message_num = 6;
			break;
		case E_TYPE         :           /* cf.h value = 14  */
			message_num = 7;
			break;
		case E_NOCuDv       :           /* cf.h value = 23  */
			message_num = 8;
			break;
		case E_NOPdDv       :           /* cf.h value = 24  */
			message_num = 9;
			break;
		case E_NOCuDvPARENT :           /* cf.h value = 25  */
			message_num = 10;
			break;
		case E_PARENTSTATE  :           /* cf.h value = 28  */
			message_num = 11;
			break;
		case E_NOATTR       :           /* cf.h value = 33  */
			message_num = 12;
			break;
		case E_BADATTR      :           /* cf.h value = 34  */
			message_num = 13;
			break;
		case E_DEVACCESS    :           /* cf.h value = 47  */
			message_num = 14;
			break;
		case E_BUSRESOURCE  :           /* cf.h value = 52  */
			message_num = 15;
			break;
		case E_MALLOC       :           /* cf.h value = 54  */
			message_num = 16;
			break;
		default             :
			message_num = 24;
	}

	return get_nls_msg(message_num);
}

/***************************************************************************/
/* Function Name : GET_NLS_MSG()                                           */ 
/* Function Type : Internal C function                                     */
/* Purpose       : Return pointer to NLS catalogue or default message text */
/*                 string for the lsresource command.                      */
/* Arguments     :                                                         */ 
/*  msgnum          - message number                                       */
/*                                                                         */
/* Notes         : The first 16 entries correspond to cf.h return codes.   */
/***************************************************************************/
static char *BusResDefltMsg[] =
{
/* index */
/* 0  */ "",  /* RESERVED, do not use */
/* 1  */ "lsresource : Device configuration database could not be initialized.\n",
/* 2  */ "lsresource : Device configuration database lock service timed out.\n",
/* 3  */ "lsresource : Device configuration database could not be opened.\n",
/* 4  */ "lsresource : Data could not be retrieved from the device configuration database\n",
/* 5  */ "lsresource : Usage error\n\tUsage : lsresource [-a] -l Name\n",
/* 6  */ "lsresource : Cannot open a required file.\n",
/* 7  */ "lsresource : Bus type not supported\n",
/* 8  */ "lsresource : The specified device was not found in the customized\n\
device configuration database.\n",
/* 9  */ "lsresource : Cannot find information in the predefined device\n\
device configuration database for the specified device.\n",
/* 10 */ "lsresource : This device is not a child of a bus, and therefore\n\
does not have any bus attributes.\n",
/* 11 */ "lsresource : A parent device is not in the AVAILABLE state\n",
/* 12 */ "lsresource : No attributes were found for this device\n",
/* 13 */ "lsresource : Bad attribute detected\n",
/* 14 */ "lsresource : Machine Device Driver ioctl failed\n",
/* 15 */ "lsresource : Insufficient bus resources for all devices\n",
/* 16 */ "lsresource : Insufficient virtual storage to proceed\n",
/* 17 */ "Devices resolvable :\n",
/* 18 */ "Devices not resolvable :\n",
/* 19 */ "Attributes which user must change to resolve devices shown above :\n",
/* 20 */ "REQUIRED VALUE",
/* 21 */ "No attributes need be modified by the user\n",
/* 22 */ "lsresource : This device has no bus resource attributes\n",
/* 23 */ "TYPE",
/* 24 */ "lsresource : Internal error\n",
/* 25 */ "NONE",
/* 26 */ "lsresource : No bus resources are assigned because device\n\
status is Defined\n",
/* 27 */ "ADAPTER",
/* 28 */ "ATTRIBUTE",
/* 29 */ "S",
/* 30 */ "G",
/* 31 */ "CURRENT VALUE",
};

char *get_nls_msg(msgnum)
	int msgnum;
{
	char *msgtext, *int_msg_err = "lsresource : Internal message error\n";

	/* set msgtext to default message string, then check for catalog */
	msgtext = BusResDefltMsg[msgnum];

	if (BusResNLSCat != (struct _catalog_descriptor *)-1)
	{
		msgtext = catgets(BusResNLSCat, BR_NLS_SET, msgnum, msgtext);
	}

	if (msgtext == NULL || !strcmp(msgtext, ""))
		msgtext = int_msg_err;

	return msgtext;
}
