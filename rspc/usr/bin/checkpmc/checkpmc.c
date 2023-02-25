static char sccsid[] = "@(#)74  1.4  src/rspc/usr/bin/checkpmc/checkpmc.c, pwrcmd, rspc41J, 9521A_all 5/23/95 14:34:22";
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS: iplcb_get, get_residual, decode_dev, main
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
#include <sys/iplcb.h>
#include <sys/residual.h>

#define _KERNSYS
#define _RSPC
#include <sys/systemcfg.h>

/*
 * NAME:  iplcb_get
 *
 * FUNCTION:  gets data from IPLCB (IPL control block).
 *
 * DATA STRUCTURES:
 *      void *dest      points a buffer
 *      int address     offset in IPLCB
 *      int num_bytes   data length
 *      int iocall      command to pass fp_ioctl()
 *
 * RETURN VALUE DESCRIPTION:
 *      0 : success
 *      -1: access error
 */
static int
iplcb_get(void *dest, int address, int num_bytes, int iocall)
{
	int		fd;
	MACH_DD_IO	mdd;

	if((fd = open("/dev/nvram",0)) < 0){
		return(-1);
	}

	mdd.md_addr = (long)address;
	mdd.md_data = dest;
	mdd.md_size = (long)num_bytes;
	mdd.md_incr = MV_BYTE;

	if(ioctl(fd,iocall,&mdd)){
		return -1;
	}

	close(fd);
	return 0;
}


/*
 * NAME:  get_residual
 *
 * FUNCTION:  gets residual data.
 *
 * DATA STRUCTURES:
 *      RESIDUAL *rp    points a residual structure
 *      int *size       returns the size of the residual data
 *
 * RETURN VALUE DESCRIPTION:
 *      0 : success
 *      -1: access error
 */
static int
get_residual(RESIDUAL *rp, int *size)
{
	IPL_DIRECTORY	iplcb_dir;

	if(iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB)){
		return -1;
	}

	if(iplcb_get(rp, iplcb_dir.residual_offset, iplcb_dir.residual_size,
								MIOIPLCB)){
		return -1;
	}
	*size = iplcb_dir.residual_size;
	return 0;
}


/*
 * NAME:  decode_dev
 *
 * FUNCTION:  decodes the devid into a string.
 *
 * DATA STRUCTURES:
 *      uint devid     specifies residual.Devices[?].DeviceId.DevId
 *
 * RETURN VALUE DESCRIPTION:
 *      a poiner to the decoded string.
 */
static char *
decode_dev(uint devid)
{
	int		i, j;
	static char	code[8];

	code[0] = ((devid >> 26) & 0x0000001F)+'@';
	code[1] = ((devid >> 21) & 0x0000001F)+'@';
	code[2] = ((devid >> 16) & 0x0000001F)+'@';

	for(i = 3, j = 3; i < 7; i++, j--){
		if(((devid >> (j*4)) & 0x0000000F) > 9){
			code[i] = (((devid >> (j*4)) & 0x0000000F)-9)+'@';
		}else{
			code[i] = ((devid >> (j*4)) & 0x0000000F)+'0';
		}
        }
	code[7] = '\0';

        return code;
}


/*
 * NAME:  main
 *
 * FUNCTION:  checks whether the machin has a power management controller.
 *
 * DATA STRUCTURES:
 *      none
 *
 * RETURN VALUE DESCRIPTION:
 *      0: the machine has a power management controller.
 *      1: the machine doesn't have a power management controller.
 */
main()
{
	RESIDUAL	residual;
	int		i, size;

	/* is this a rspc machine? */
	if(!__rspc()){
		return 1;
	}

	/* get residual data */
	if(get_residual(&residual, &size)){
		return 1;
	}

	/* check all device data. PNPPM is "IBM0005" */
	for(i = 0; i < residual.ActualNumDevices; i++){
		if(strcmp(decode_dev(residual.Devices[i].DeviceId.DevId),
						PNPPM) == 0 &&
			residual.Devices[i].DeviceId.Interface
					 == GeneralPowerManagement){
			return 0;
		}
	}

	return 1;
}
