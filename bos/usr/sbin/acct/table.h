/* @(#)39	1.2  src/bos/usr/sbin/acct/table.h, cmdacct, bos411, 9428A410j 6/15/90 20:02:50 */
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

struct table {
	char        **tb_base;      /* Pointer to ptr to space */
	ushort      tb_elsize;      /* Size of one element */
	int         tb_nel;         /* Number of elements */
};

#define INITTABLE(array,nel) \
	{ (char **)&array, sizeof(*array), nel }

extern char *extend();
