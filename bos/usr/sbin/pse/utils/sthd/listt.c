static char sccsid[] = "@(#)62	1.1  src/bos/usr/sbin/pse/utils/sthd/listt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:40";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** listt.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"

	boolean	list_check(   int fd, struct str_list * list, int ret_val, int errno_val   );
extern	char	* strrchr();

#ifndef errno
extern	int	errno;
#endif

void
listt (state)
	int	state;
{
	int	fd;
	char	buf[128];
	char	* cp;
	struct str_list	* list;

	printf("Testing list ioctl\n");
	list = (struct str_list *)buf;
	if ((fd = stream_open(nuls_name, 2)) == -1) {
		printe(true, "open of '%s' failed", nuls_name);
		return;
	}
	if (cp = strrchr(nuls_name, '/'))
		cp++;
	else
		cp = nuls_name;
	if (stream_ioctl(fd, I_PUSH, pass_name) == -1) {
		printe(true, "push of '%s' failed", pass_name);
		return;
	}
	if (stream_ioctl(fd, I_PUSH, pass_name) == -1) {
		printe(true, "second push of '%s' failed", pass_name);
		return;
	}
	if (stream_ioctl(fd, I_PUSH, pass_name) == -1) {
		printe(true, "third push of '%s' failed", pass_name);
		return;
	}

	if (state & ERROR) {
		printf("testing list in error state\n");
		if (set_error_state(fd, EIO))
			list_check(fd, list, -1, EIO);
		stream_close(fd);
		return;
	}

#ifndef NO_BOGUS_ADDRESSES
	printf("testing list with bogus address\n");
	list_check(fd, BOGUS_ADDRESS, -1, EFAULT);
#endif

	printf("testing list with bogus sl_nmods\n");
	list->sl_nmods = 0;
	list_check(fd, list, -1, EINVAL);

	printf("testing list for return value only\n");
	list_check(fd, nilp(struct str_list), 4, 0);

	printf("testing valid list with exact number of elements\n");
	list->sl_nmods = 4;
	if (!(list->sl_modlist = (struct str_mlist *)malloc(10 * sizeof(struct str_mlist)))) {
		printe(false, "couldn't malloc space for list elements");
		return;
	}
	if (list_check(fd, list, 4, 0)) {
		if (strcmp(list->sl_modlist[0].l_name, pass_name) != 0)
			printe(false, "list returned first name '%s' when expecting '%s'", list->sl_modlist[0].l_name, pass_name);
		if (strcmp(list->sl_modlist[1].l_name, pass_name) != 0)
			printe(false, "list returned second name '%s' when expecting '%s'", list->sl_modlist[1].l_name, pass_name);
		if (strcmp(list->sl_modlist[2].l_name, pass_name) != 0)
			printe(false, "list returned third name '%s' when expecting '%s'", list->sl_modlist[2].l_name, pass_name);
		if (strcmp(list->sl_modlist[3].l_name, cp) != 0)
			printe(false, "list returned fourth name '%s' when expecting '%s'", list->sl_modlist[3].l_name, cp);
	}

	printf("testing valid list with extra elements\n");
	list->sl_nmods = 10;
	list->sl_modlist[0].l_name[0] = '\0';
	list->sl_modlist[1].l_name[0] = '\0';
	list->sl_modlist[2].l_name[0] = '\0';
	if (list_check(fd, list, 4, 0)) {
		if (strcmp(list->sl_modlist[0].l_name, pass_name) != 0)
			printe(false, "list returned first name '%s' when expecting '%s'", list->sl_modlist[0].l_name, pass_name);
		if (strcmp(list->sl_modlist[1].l_name, pass_name) != 0)
			printe(false, "list returned second name '%s' when expecting '%s'", list->sl_modlist[1].l_name, pass_name);
		if (strcmp(list->sl_modlist[2].l_name, pass_name) != 0)
			printe(false, "list returned third name '%s' when expecting '%s'", list->sl_modlist[2].l_name, pass_name);
		if (strcmp(list->sl_modlist[3].l_name, cp) != 0)
			printe(false, "list returned fourth name '%s' when expecting '%s'", list->sl_modlist[3].l_name, cp);
	}

	printf("testing valid list with only 2 elements\n");
	list->sl_nmods = 2;
	list->sl_modlist[0].l_name[0] = '\0';
	list->sl_modlist[1].l_name[0] = '\0';
	list->sl_modlist[2].l_name[0] = '\0';
	if (list_check(fd, list, 2, 0)) {
		if (strcmp(list->sl_modlist[0].l_name, pass_name) != 0)
			printe(false, "list returned first name '%s' when expecting '%s'", list->sl_modlist[0].l_name, pass_name);
		if (strcmp(list->sl_modlist[1].l_name, pass_name) != 0)
			printe(false, "list returned second name '%s' when expecting '%s'", list->sl_modlist[1].l_name, pass_name);
	}
	free(list->sl_modlist);

	stream_close(fd);
}

boolean
list_check (fd, strlist, ret_val, errno_val)
	int	fd;
	struct str_list	* strlist;
	int	ret_val;
	int	errno_val;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_LIST, (char *)strlist)) {
	case 0:
		if (ret_val == 0)
			return printok("list returned 0 as expected");
		return printe(false, "list returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == errno_val)
				return printok("list failed as expected with errno %d", errno);
			return printe(true, "list failed incorrectly, expected errno %d", errno_val);
		}
		return printe(true, "list failed");
	default:
		if (ret_val == i1)
			return printok("list returned %d as expected", i1);
		return printe(false, "list returned %d when expecting %d", i1, ret_val);
	}
}
