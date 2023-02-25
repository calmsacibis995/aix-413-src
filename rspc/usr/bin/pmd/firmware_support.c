static char sccsid[] = "@(#)97  1.3  src/rspc/usr/bin/pmd/firmware_support.c, pwrcmd, rspc41J, 9516A_all 4/18/95 14:16:05";
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS: iplcb_get, firmware_support
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

#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/residual.h>
#include <sys/iplcb.h>

/*
 * NAME:  iplcb_get
 *
 * FUNCTION:  get IPLCB data
 *
 * DATA STRUCTURES:
 *      dest      data area to be returned.
 *
 * RETURN VALUE DESCRIPTION:
 *      When success, 0 is returned.
 *      When an error occurred, a negative integer is returned.
 */
static int
iplcb_get(void *dest, int address, int num_bytes, int iocall)
{
	int		fd;
	MACH_DD_IO	mdd;

	if((fd = open("/dev/nvram", 0)) < 0){
		return(-1);
	}

	mdd.md_addr = (long)address;
	mdd.md_data = dest;
	mdd.md_size = (long)num_bytes;
	mdd.md_incr = MV_BYTE;

	if(ioctl(fd, iocall, &mdd)){
		return -1;
	}

	close(fd);
	return 0;
}

/*
 * NAME:  firmware_support
 *
 * FUNCTION:  get FirmwareSupports flag
 *
 * DATA STRUCTURES:
 *      firm      FirmwareSupports flag is returned.
 *
 * RETURN VALUE DESCRIPTION:
 *      When success, 0 is returned.
 *      When an error occurred, a negative integer is returned.
 *
 * NOTE: suspend/hibernation flags are:
 *      Suspend = 0x0200     
 *      Hibernation = 0x0400
 */
int
firmware_support(unsigned long *firmware)
{
	IPL_DIRECTORY	iplcb_dir;
	RESIDUAL	residual;
	int		i;
	char		*p;

	if(firmware == NULL){
		return -1;
	}

	/* get IPL control block */
	if(iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB)){
		return -1;
	}

	/* get residual data */
	if(iplcb_get(&residual, iplcb_dir.residual_offset,
				iplcb_dir.residual_size, MIOIPLCB)){
		return -1;
	}

	/* search for Power Management Packet */
	for(i = 0; i < residual.ActualNumDevices; i++){
		if(residual.Devices[i].DeviceId.DevId == 0x244d0005){
			break;
		}
	}
	if(i >= residual.ActualNumDevices){
		return -1;
	}

	/* search for Power Management Descriptor */
	p = residual.DevicePnPHeap;
	for(i = residual.Devices[i].AllocatedOffset; p[i] != END_TAG; i++){
		if(p[i] == L4_Packet && p[i+3] == 8){
			*firmware = (int)p[i+4] + ((int)p[i+5] << 8) +
				((int)p[i+6] << 16) + ((int)p[i+7] << 24);
			return 0;
		}
	}

	return -1;
}
