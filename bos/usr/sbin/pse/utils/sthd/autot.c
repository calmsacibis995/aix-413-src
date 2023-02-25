static char sccsid[] = "@(#)53	1.1  src/bos/usr/sbin/pse/utils/sthd/autot.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:21";
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
 ** autot.c 2.3, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"
#include <sys/sad.h>
#include <pse/nd.h>

staticf	char	* get_cp(   char * buf, char * name   );
staticf	int	get_major(   char * name   );
staticf	boolean	sad_check(   int fd, int cmd, char * buf, int len, int ret_val, int errno_val   );
extern	char	* strchr();
extern	char	* strrchr();

#ifndef errno
extern	int	errno;
#endif
static	int	pass = 1;

void
autot (state)
	int	state;
{
	int	fd;
	int	fd1;
	char	buf[128];
	char	* cp;
	char	name_buf[40];
	int	nuls_major;
	struct strapush	stra;
	struct strapush	stra1;
	struct str_list	sl;
	struct str_mlist	* sml;

	printf("Testing ioctls to SAD driver\n");
do_it_again:;
	switch ( pass ) {
	case 1:
		printf("First we test using I_STR SAD ioctls\n");
		break;
	case 2:
		printf("\nNow we do it all again using transparent ioctls\n");
		PAUSE_HERE();
		break;
	default:
		return;
	}
	if (cp = strrchr(nuls_name, '/'))
		cp++;
	else
		cp = nuls_name;
	if ((nuls_major = get_major(cp)) == -1) {
		printe(false, "couldn't get major number for %s", cp);
		return;
	}

	if ((fd = stream_open(sad_name, 2)) == -1) {
		printe(true, "open of '%s' failed", sad_name);
		return;
	}

#ifndef NO_BOGUS_ADDRESSES
	printf("testing SAD_VML ioctl with bogus str_mlist address\n");
	sl.sl_nmods = 2;
	sl.sl_modlist = (struct str_mlist *)BOGUS_ADDRESS;
	sad_check(fd, SAD_VML, (char *)&sl, sizeof(struct str_list), -1, EFAULT);
#endif

	printf("testing SAD_VML with bogus sl_nmods\n");
	sl.sl_nmods = 0;
	sad_check(fd, SAD_VML, (char *)&sl, sizeof(struct str_list), -1, EINVAL);

	printf("testing SAD_VML with 2 valid names\n");
	sl.sl_nmods = 2;
	sl.sl_modlist = (struct str_mlist *)buf;
	strcpy(sl.sl_modlist[0].l_name, pass_name);
	strcpy(sl.sl_modlist[1].l_name, "timod");
	sad_check(fd, SAD_VML, (char *)&sl, sizeof(struct str_list), 0, 0);

	printf("testing SAD_VML with a bad name\n");
	sl.sl_nmods = 2;
	sl.sl_modlist = (struct str_mlist *)buf;
	strcpy(sl.sl_modlist[0].l_name, pass_name);
	strcpy(sl.sl_modlist[1].l_name, "foobar");
	sad_check(fd, SAD_VML, (char *)&sl, sizeof(struct str_list), 1, 0);

	printf("testing SAD_GAP with a non-existent major number (5000)\n");
	stra.sap_cmd = 0;
	stra.sap_major = 5000;
	sad_check(fd, SAD_GAP, (char *)&stra, sizeof(struct strapush), -1, ENOSTR);

	printf("testing SAD_GAP with a major number not configured for autopushing (%d)\n", nuls_major);
	stra.sap_major = nuls_major;
	sad_check(fd, SAD_GAP, (char *)&stra, sizeof(struct strapush), -1, ENODEV);

	printf("testing SAD_SAP with a bad command value\n");
	stra.sap_cmd = SAP_ONE + 2000;
	sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EINVAL);

	printf("testing SAD_SAP with a non-existent major number (5000)\n");
	stra.sap_cmd = SAP_ONE;
	stra.sap_major = 5000;
	stra.sap_minor = 2;
	sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EINVAL);

	printf("testing SAD_SAP with an invalid number of modules\n");
	stra.sap_major = nuls_major;
	stra.sap_npush = MAXAPUSH + 10;
	sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EINVAL);

	printf("testing SAD_SAP with an invalid name in the list of modules\n");
	stra.sap_cmd = SAP_ONE;
	stra.sap_major = nuls_major;
	stra.sap_minor = 12;
	stra.sap_npush = 2;
	strcpy(stra.sap_list[0], pass_name);
	strcpy(stra.sap_list[1], "foobar");
	sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EINVAL);

	printf("testing SAD_SAP (SAP_RANGE) with an invalid minor number range\n");
	stra.sap_cmd = SAP_RANGE;
	stra.sap_major = nuls_major;
	stra.sap_minor = 12;
	stra.sap_lastminor = 2;
	stra.sap_npush = 2;
	strcpy(stra.sap_list[0], pass_name);
	strcpy(stra.sap_list[1], pass_name);
	sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, ERANGE);

	printf("testing SAD_SAP (SAP_CLEAR) on a major number not configured for autopushing\n");
	stra.sap_cmd = SAP_CLEAR;
	stra.sap_major = nuls_major;
	stra.sap_minor = 2;
	sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, ENODEV);

	PAUSE_HERE();
	printf("testing valid SAD_SAP on a range of minor numbers\n");
	stra.sap_cmd = SAP_RANGE;
	stra.sap_minor = 3;
	stra.sap_lastminor = 12;
	if (sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0)) {
		printf("testing SAD_GAP on valid range\n");
		memset((char *)&stra1, 0, sizeof(stra1));
		stra1.sap_major = stra.sap_major;
		stra1.sap_minor = 3;
		stra1.sap_lastminor = 12;
		if (sad_check(fd, SAD_GAP, (char *)&stra1, sizeof(struct strapush), 0, 0)) {
			if (stra1.sap_cmd != SAP_RANGE)
				printe(false, "SAD_GAP reported cmd %d when expecting SAP_RANGE (%d)", stra1.sap_cmd, SAP_RANGE);
			if (stra1.sap_minor != 3)
				printe(false, "SAD_GAP reported minor %d when expecting 3", stra1.sap_minor);
			if (stra1.sap_lastminor != 12)
				printe(false, "SAD_GAP reported lastminor %d when expecting 12", stra1.sap_lastminor);
			if (stra1.sap_npush != 2)
				printe(false, "SAD_GAP reported npush %d when expecting 2", stra1.sap_npush);
			if (strcmp(stra1.sap_list[0], pass_name) != 0)
				printe(false, "list returned first name '%s' when expecting '%s'", stra1.sap_list[0], pass_name);
			if (strcmp(stra1.sap_list[1], pass_name) != 0)
				printe(false, "list returned second name '%s' when expecting '%s'", stra1.sap_list[1], pass_name);
		}

		printf("testing SAD_GAP on minor number included in valid range\n");
		memset((char *)&stra1, 0, sizeof(stra1));
		stra1.sap_major = stra.sap_major;
		stra1.sap_minor = 8;
		if (sad_check(fd, SAD_GAP, (char *)&stra1, sizeof(struct strapush), 0, 0)) {
			if (stra1.sap_cmd != SAP_RANGE)
				printe(false, "SAD_GAP reported cmd %d when expecting SAP_RANGE (%d)", stra1.sap_cmd, SAP_RANGE);
			if (stra1.sap_minor != 3)
				printe(false, "SAD_GAP reported minor %d when expecting 3", stra1.sap_minor);
			if (stra1.sap_lastminor != 12)
				printe(false, "SAD_GAP reported lastminor %d when expecting 12", stra1.sap_lastminor);
			if (stra1.sap_npush != 2)
				printe(false, "SAD_GAP reported npush %d when expecting 2", stra1.sap_npush);
			if (strcmp(stra1.sap_list[0], pass_name) != 0)
				printe(false, "list returned first name '%s' when expecting '%s'", stra1.sap_list[0], pass_name);
			if (strcmp(stra1.sap_list[1], pass_name) != 0)
				printe(false, "list returned second name '%s' when expecting '%s'", stra1.sap_list[1], pass_name);
		}

		printf("testing SAD_GAP on minor number outside valid range\n");
		stra.sap_minor = 20;
		sad_check(fd, SAD_GAP, (char *)&stra, sizeof(struct strapush), -1, ENODEV);

		printf("attempting valid SAD_SAP on a minor in the range already set\n");
		stra.sap_cmd = SAP_ONE;
		stra.sap_minor = 7;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EEXIST);

		printf("attempting valid SAD_SAP on an overlapping range\n");
		stra.sap_cmd = SAP_RANGE;
		stra.sap_minor = 1;
		stra.sap_lastminor = 5;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EEXIST);

		printf("attempting valid SAD_SAP on a range included in the set range\n");
		stra.sap_cmd = SAP_RANGE;
		stra.sap_minor = 10;
		stra.sap_lastminor = 11;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EEXIST);

		printf("attempting to clear the range with an invalid minor number\n");
		stra.sap_cmd = SAP_CLEAR;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, ERANGE);

#ifdef SPECIFIC_OPENS_OK
		sprintf(name_buf, "%s5", nuls_name);
		printf("opening stream in the range set (%s)\n", name_buf);
		fd1 = stream_open(name_buf, 2);
		if (fd1 != -1) {
			sl.sl_nmods = MAXAPUSH;
			sl.sl_modlist = (struct str_mlist *)buf;
			if (list_check(fd1, &sl, 3, 0)) {
				if (strcmp(sl.sl_modlist[0].l_name, pass_name) != 0)
					printe(false, "list returned first name '%s' when expecting '%s'", sl.sl_modlist[0].l_name, pass_name);
				if (strcmp(sl.sl_modlist[1].l_name, pass_name) != 0)
					printe(false, "list returned second name '%s' when expecting '%s'", sl.sl_modlist[1].l_name, pass_name);
				if (strcmp(sl.sl_modlist[2].l_name, cp) != 0)
					printe(false, "list returned third name '%s' when expecting '%s'", sl.sl_modlist[2].l_name, cp);
			}
			stream_close(fd1);
		} else
			printe(true, "open of %s failed", name_buf);

		sprintf(name_buf, "%s2", nuls_name);
		printf("opening stream just before the range set (%s)\n", name_buf);
		fd1 = stream_open(name_buf, 2);
		if (fd1 != -1) {
			sl.sl_nmods = MAXAPUSH;
			sl.sl_modlist = (struct str_mlist *)buf;
			if (list_check(fd1, &sl, 1, 0)) {
				if (strcmp(sl.sl_modlist[0].l_name, cp) != 0)
					printe(false, "list returned name '%s' when expecting '%s'", sl.sl_modlist[0].l_name, cp);
			}
			stream_close(fd1);
		} else
			printe(true, "open of %s failed", name_buf);

		sprintf(name_buf, "%s13", nuls_name);
		printf("opening stream just after the range set (%s)\n", name_buf);
		fd1 = stream_open(name_buf, 2);
		if (fd1 != -1) {
			sl.sl_nmods = MAXAPUSH;
			sl.sl_modlist = (struct str_mlist *)buf;
			if (list_check(fd1, &sl, 1, 0)) {
				if (strcmp(sl.sl_modlist[0].l_name, cp) != 0)
					printe(false, "list returned name '%s' when expecting '%s'", sl.sl_modlist[0].l_name, cp);
			}
			stream_close(fd1);
		} else
			printe(true, "open of %s failed", name_buf);
#endif
		printf("clearing range set\n");
		stra.sap_cmd = SAP_CLEAR;
		stra.sap_minor = 3;
		stra.sap_lastminor = -1;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0);

		printf("making sure the autopush information was cleared\n");
		sad_check(fd, SAD_GAP, (char *)&stra, sizeof(struct strapush), -1, ENODEV);
	}

	PAUSE_HERE();
	printf("testing valid SAD_SAP on a specific minor number\n");
	stra.sap_cmd = SAP_ONE;
	stra.sap_minor = 3;
	stra.sap_lastminor = 12;
	if (sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0)) {
		printf("testing SAD_GAP on minor number set\n");
		memset((char *)&stra1, 0, sizeof(stra1));
		stra1.sap_major = stra.sap_major;
		stra1.sap_minor = 3;
		stra1.sap_lastminor = 12;
		if (sad_check(fd, SAD_GAP, (char *)&stra1, sizeof(struct strapush), 0, 0)) {
			if (stra1.sap_cmd != SAP_ONE)
				printe(false, "SAD_GAP reported cmd %d when expecting SAP_ONE (%d)", stra1.sap_cmd, SAP_ONE);
			if (stra1.sap_minor != 3)
				printe(false, "SAD_GAP reported minor %d when expecting 3", stra1.sap_minor);
			if (stra1.sap_lastminor != 3)
				printe(false, "SAD_GAP reported lastminor %d when expecting 3", stra1.sap_lastminor);
			if (stra1.sap_npush != 2)
				printe(false, "SAD_GAP reported npush %d when expecting 2", stra1.sap_npush);
			if (strcmp(stra1.sap_list[0], pass_name) != 0)
				printe(false, "list returned first name '%s' when expecting '%s'", stra1.sap_list[0], pass_name);
			if (strcmp(stra1.sap_list[1], pass_name) != 0)
				printe(false, "list returned second name '%s' when expecting '%s'", stra1.sap_list[1], pass_name);
		}

		printf("testing SAD_GAP on minor number not set\n");
		stra.sap_minor = 4;
		sad_check(fd, SAD_GAP, (char *)&stra, sizeof(struct strapush), -1, ENODEV);

		printf("attempting valid SAD_SAP on minor already set\n");
		stra.sap_minor = 3;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EEXIST);

		printf("attempting valid SAD_SAP on an overlapping range\n");
		stra.sap_cmd = SAP_RANGE;
		stra.sap_minor = 1;
		stra.sap_lastminor = 5;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EEXIST);

		printf("testing valid SAD_SAP on another minor number\n");
		stra.sap_cmd = SAP_ONE;
		stra.sap_minor = 12;
		if (sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0)) {
			printf("deleting autopush information for previous set\n");
			stra.sap_cmd = SAP_CLEAR;
			sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0);
		}

#ifdef SPECIFIC_OPENS_OK
		sprintf(name_buf, "%s3", nuls_name);
		printf("opening specific stream (%s)\n", name_buf);
		fd1 = stream_open(name_buf, 2);
		if (fd1 != -1) {
			sl.sl_nmods = MAXAPUSH;
			sl.sl_modlist = (struct str_mlist *)buf;
			if (list_check(fd1, &sl, 3, 0)) {
				if (strcmp(sl.sl_modlist[0].l_name, pass_name) != 0)
					printe(false, "list returned first name '%s' when expecting '%s'", sl.sl_modlist[0].l_name, pass_name);
				if (strcmp(sl.sl_modlist[1].l_name, pass_name) != 0)
					printe(false, "list returned second name '%s' when expecting '%s'", sl.sl_modlist[1].l_name, pass_name);
				if (strcmp(sl.sl_modlist[2].l_name, cp) != 0)
					printe(false, "list returned third name '%s' when expecting '%s'", sl.sl_modlist[2].l_name, cp);
			}
			stream_close(fd1);
		} else
			printe(true, "open of %s failed", name_buf);

		printf("opening clone stream which should not be autopushed\n");
		fd1 = stream_open(nuls_name, 2);
		if (fd1 != -1) {
			sl.sl_nmods = MAXAPUSH;
			sl.sl_modlist = (struct str_mlist *)buf;
			if (list_check(fd1, &sl, 1, 0)) {
				if (strcmp(sl.sl_modlist[0].l_name, cp) != 0)
					printe(false, "list returned name '%s' when expecting '%s'", sl.sl_modlist[0].l_name, cp);
			}
			stream_close(fd1);
		} else
			printe(true, "open of %s failed", nuls_name);

#endif
		printf("clearing minor number set\n");
		stra.sap_cmd = SAP_CLEAR;
		stra.sap_minor = 3;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0);

		printf("making sure the autopush information was cleared\n");
		sad_check(fd, SAD_GAP, (char *)&stra, sizeof(struct strapush), -1, ENODEV);
	}

	PAUSE_HERE();
	printf("testing SAD_SAP on all minors\n");
	stra.sap_cmd = SAP_ALL;
	if (sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0)) {
		printf("testing SAD_GAP on a specific minor\n");
		memset((char *)&stra1, 0, sizeof(stra1));
		stra1.sap_major = stra.sap_major;
		stra1.sap_minor = 16;
		if (sad_check(fd, SAD_GAP, (char *)&stra1, sizeof(struct strapush), 0, 0)) {
			if (stra1.sap_cmd != SAP_ALL)
				printe(false, "SAD_GAP reported cmd %d when expecting SAP_ALL (%d)", stra1.sap_cmd, SAP_ALL);
			if (stra1.sap_minor != -1)
				printe(false, "SAD_GAP reported minor %d when expecting -1", stra1.sap_minor);
			if (stra1.sap_lastminor != -1)
				printe(false, "SAD_GAP reported lastminor %d when expecting -1", stra1.sap_lastminor);
			if (stra1.sap_npush != 2)
				printe(false, "SAD_GAP reported npush %d when expecting 2", stra1.sap_npush);
			if (strcmp(stra1.sap_list[0], pass_name) != 0)
				printe(false, "list returned first name '%s' when expecting '%s'", stra1.sap_list[0], pass_name);
			if (strcmp(stra1.sap_list[1], pass_name) != 0)
				printe(false, "list returned second name '%s' when expecting '%s'", stra1.sap_list[1], pass_name);
		}

		printf("attempting valid SAD_SAP on an overlapping range\n");
		stra.sap_cmd = SAP_RANGE;
		stra.sap_minor = 1;
		stra.sap_lastminor = 5;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, EEXIST);

		printf("opening a clone stream which should be autopushed\n");
		fd1 = stream_open(nuls_name, 2);
		if (fd1 != -1) {
			sl.sl_nmods = MAXAPUSH;
			sl.sl_modlist = (struct str_mlist *)buf;
			if (list_check(fd1, &sl, 3, 0)) {
				if (strcmp(sl.sl_modlist[0].l_name, pass_name) != 0)
					printe(false, "list returned first name '%s' when expecting '%s'", sl.sl_modlist[0].l_name, pass_name);
				if (strcmp(sl.sl_modlist[1].l_name, pass_name) != 0)
					printe(false, "list returned second name '%s' when expecting '%s'", sl.sl_modlist[1].l_name, pass_name);
				if (strcmp(sl.sl_modlist[2].l_name, cp) != 0)
					printe(false, "list returned third name '%s' when expecting '%s'", sl.sl_modlist[2].l_name, cp);
			}
			stream_close(fd1);
		} else
			printe(true, "open of %s failed", nuls_name);

		printf("attempting to clear with a specific minor number\n");
		stra.sap_cmd = SAP_CLEAR;
		stra.sap_minor = 12;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), -1, ENODEV);

		printf("clearing autopush information\n");
		stra.sap_cmd = SAP_CLEAR;
		stra.sap_minor = 0;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0);

		printf("making sure the autopush information was cleared\n");
		sad_check(fd, SAD_GAP, (char *)&stra, sizeof(struct strapush), -1, ENODEV);
	}

	PAUSE_HERE();
	printf("testing autopush with error in pushing\n");
	stra.sap_npush = 3;
	strcpy(stra.sap_list[0], pass_name);
	strcpy(stra.sap_list[1], pass_name);
	strcpy(stra.sap_list[2], errm_name);
	stra.sap_cmd = SAP_ALL;
	if (sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0)) {
		printf("opening a clone stream which should be autopushed\n");
		fd1 = stream_open(nuls_name, 2);
		if (fd1 != -1) {
			printe(false, "open of %s succeeded", nuls_name);
			stream_close(fd1);
		} else
			printf("open of %s failed as expected\n", nuls_name);

		printf("clearing autopush information\n");
		stra.sap_cmd = SAP_CLEAR;
		stra.sap_minor = 0;
		sad_check(fd, SAD_SAP, (char *)&stra, sizeof(struct strapush), 0, 0);
	}

	stream_close(fd);
	pass++;
	goto do_it_again;
}

staticf char *
get_cp (buf, name)
	char	* buf;
	char	* name;
{
reg	char	* cp;
	int	name_len;

	name_len = strlen(name);
	for (cp = buf; cp[0]  &&  cp[1]; cp += (strlen(cp)+1)) {
		if (cp[0] == name[0]  &&  strncmp(cp, name, name_len) == 0) {
			if (cp = strchr(cp, '='))
				cp += 3;
			return cp;
		}
	}
	return nilp(char);
}

static int
get_major (name)
	char	* name;
{
	char	* cp;
	char	info_buf[128];
	int	fd;
	struct strioctl	stri;

	if ((fd = stream_open(nuls_name, 2)) == -1) {
		printe(true, "open of %s failed", nuls_name);
		return -1;
	}
	if (stream_ioctl(fd, I_PUSH, "sc") == -1) {
		printe(true, "push of 'sc' failed");
		stream_close(fd);
		return -1;
	}
	stri.ic_cmd = ND_GET;
	stri.ic_timout = 0;
	sprintf(info_buf, "info %s", name);
	info_buf[4] = '\0';
	stri.ic_dp = info_buf;
	stri.ic_len = sizeof(info_buf);
	if (stream_ioctl(fd, I_STR, &stri) == -1) {
		printe(true, "ioctl to get major number failed");
		stream_close(fd);
		return -1;
	}
	stream_close(fd);
	if (cp = get_cp(info_buf, "device number"))
		return atoi(cp);
	return -1;
}

staticf boolean
sad_check (fd, cmd, buf, len, ret_val, errno_val)
	int	fd;
	int	cmd;
	char	* buf;
	int	len;
	int	ret_val;
	int	errno_val;
{
	int	i1;
	struct strioctl	stri;

	if ( pass == 1 ) {
		/* Use I_STR ioctl */
		stri.ic_cmd = cmd;
		stri.ic_dp = buf;
		stri.ic_len = len;
		stri.ic_timout = -1;
		i1 = stream_ioctl(fd, I_STR, (char *)&stri);
	} else {
		/* Use transparent ioctl */
		i1 = stream_ioctl(fd, cmd, buf);
	}
	switch ( i1 ) {
	case 0:
		if (ret_val == 0)
			return printok("sad ioctl returned 0 as expected");
		return printe(false, "sad ioctl returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == errno_val)
				return printok("sad ioctl failed as expected with errno %d", errno);
			return printe(true, "sad ioctl failed incorrectly, expected errno %d", errno_val);
		}
		return printe(true, "sad ioctl failed");
	default:
		if (ret_val == i1)
			return printok("sad ioctl returned %d as expected", i1);
		return printe(false, "sad ioctl returned %d when expecting %d", i1, ret_val);
	}
}
