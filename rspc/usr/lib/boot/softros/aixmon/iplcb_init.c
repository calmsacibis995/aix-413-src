static char sccsid[] = "@(#)98	1.40  src/rspc/usr/lib/boot/softros/aixmon/iplcb_init.c, rspc_softros, rspc41J, 9523A_all 6/5/95 15:17:41";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: iplcb_init
 *		mem_find
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* iplcb_init.c  - Construct an IPL control block 			*/

#include <sys/ioacc.h>
#include "aixmon.h"
#include <sys/residual.h>
#include <sys/systemcfg.h>

#include "iplcb_init.h"

u_long inet_addr(char *);
u_long encode_dev(char *);
int ser_to_io(char *pnp_string);
					/* Some defines for naming	*/
#define IPLD ipl_cb_ptr->s0

#ifdef PM_SUPPORT
#define WAKEUP_AREA_LENGTH	(64*1024)	/* wakeup are length 64KB */
#endif /* PM_SUPPORT */

extern uint ProcessorBusHz;

iplcb_init(struct ipldata *ipldata)
{
typedef struct pci_bridge_descriptor {
  unsigned char tag;
  unsigned char length1;                 /* length, bits [7:0] */
  unsigned char length2;                 /* length, bits [15:8] */
  unsigned char rspc_type;
  unsigned char config_base_addr_1;      /* bits [7:0]  */
  unsigned char config_base_addr_2;      /* bits [15:8] */
  unsigned char config_base_addr_3;      /* bits [23:16] */
  unsigned char config_base_addr_4;      /* bits [31:24] */
  unsigned char config_base_addr_5;      /* bits [39:32] */
  unsigned char config_base_addr_6;      /* bits [47:40] */
  unsigned char config_base_addr_7;      /* bits [55:48] */
  unsigned char config_base_addr_8;      /* bits [63:56] */
  unsigned char config_data_addr_1;      /* bits [7:0]  */
  unsigned char config_data_addr_2;      /* bits [15:8] */
  unsigned char config_data_addr_3;      /* bits [23:16] */
  unsigned char config_data_addr_4;      /* bits [31:24] */
  unsigned char config_data_addr_5;      /* bits [39:32] */
  unsigned char config_data_addr_6;      /* bits [47:40] */
  unsigned char config_data_addr_7;      /* bits [55:48] */
  unsigned char config_data_addr_8;      /* bits [63:56] */
  unsigned char bus_num;                 /* PCI bus number */
  unsigned char res1;                    /* reserved    */
  unsigned char res2;                    /* reserved    */
  unsigned char res3;                    /* reserved    */
  } PCI_BRIDGE_DESCRIPTOR;
					/* Pointers to structures	*/
IPL_CB		*ipl_cb_ptr;		/* Pointer to ipl cb struct	*/
IPL_INFO 	*ipl_info;		/* ipl info			*/
SYSTEM_INFO	*sys_info;		/* system info			*/
BUC_DATA	*buc_info;		/* buc data area		*/
PROCESSOR_DATA  *proc_info;		/* processor data area		*/
ROS_ENTRY_TABLE *ros_table;		/* ros_entry_table		*/
L2_DATA		*l2_data;		/* pointer to l2 data struct(s) */
MEM_DATA	*mem_data;		/* memory data area		*/
NET_DATA	*net_data;		/* Network data structure ptr	*/
RESIDUAL	*rp, *res_ptr;		/* Pointer to residual data     */
uint		ros_table_size;		/* Size of ros_entry_table	*/
u_long		tlong;			/* Temporary variable		*/

struct pci_bridge_descriptor *L4_packet;
int     buc_loc[] = {   0x30303030,
                        0x30303130,
                        0x30303230,
                        0x30303330,
                        0x30303430,
                        0x30303530,
                        0x30303630,
                        0x30303730,
                        0x30303830,
                        0x30303930,
                        0x30303a30,
                        0x30303b30,
                        0x30303c30,
                        0x30303d30,
                        0x30303e30,
                        0x30303f30
                   };

char	*iplcb_loc;			/* Pointer to start of memory loc
					   for in core iplcb		*/

int	dma_buffer;			/* Pointer to reserved dma area */

uint	current_off;

char	*mem_loc;			/* Temp pointer for memory	*/
int	iplcb_len;			/* Length of in core area	*/

int	i,j, busnum;
int	resid_string_index;
uchar 	bitmap[BITMAP_SIZE];		/* Temporary bitmap location	*/
uchar	*nvram_cache;			/* NVRAM cache location		*/
char	nv_var[256];			/* Buffer for environment var	*/
int	buc_index = 1;
int	wakeup_area;			/* wakeup area address */
int	pcibus_id;			/* unique identifier	*/

dbgmsg(AMON_D1,"- iplcb_init()");

rp = (RESIDUAL *) residual_save;

				/* Calculate length of iplcb save area	*/

iplcb_len = sizeof(IPL_CB);			/* Set first offset	*/
iplcb_len += sizeof(IPL_INFO);
iplcb_len += sizeof(SYSTEM_INFO);
iplcb_len += MAX_BUC_SLOTS * sizeof(BUC_DATA);
iplcb_len += sizeof(PROCESSOR_DATA);
iplcb_len += (BITMAP_SIZE*2);
iplcb_len += sizeof(ROS_ENTRY_TABLE);
iplcb_len += NVRAM_SIZE;
iplcb_len += MAX_NUM_OF_SIMMS * sizeof(MEM_DATA);
iplcb_len += sizeof(L2_DATA);		/* Just one element for now	*/
iplcb_len += sizeof(NET_DATA);
iplcb_len += rp->ResidualLength;

					/* Adjust length to next 16K    */
i = BYTES_PER_BIT;
if(iplcb_len % i)
	iplcb_len = ((iplcb_len + i) / i)*i;


					/* Get addr of memory area	*/
					/* and Initialize bitmap	*/
*(int *)(&iplcb_loc) = mem_find(bitmap,iplcb_len,&dma_buffer,&wakeup_area);
dbgtst(AMON_D2)
	{
	printf("Returned from mem_find:\n");
	printf("IPLCB addr .... = 0x%X len = %d\n",iplcb_loc,iplcb_len);
	printf("DMA buffer addr = 0x%X \n",dma_buffer);
	}

			/* Calculate data offsets and setup pointers	*/

ipl_cb_ptr = (IPL_CB *) iplcb_loc;		/* point to final address
						   for relocating iplcb */

current_off = sizeof(IPL_CB);			/* Set first offset	*/

IPLD.ipl_info_offset =  current_off;		/* ipl_info		*/
IPLD.ipl_info_size = sizeof(IPL_INFO);
ipl_info = (IPL_INFO *) (iplcb_loc + current_off);
current_off += IPLD.ipl_info_size;

IPLD.system_info_offset = current_off;		/* system_info		*/
IPLD.system_info_size = sizeof(SYSTEM_INFO);
sys_info = (SYSTEM_INFO *) (iplcb_loc + current_off);
current_off += IPLD.system_info_size;

IPLD.buc_info_offset = current_off;		/* buc_data		*/
IPLD.buc_info_size = MAX_BUC_SLOTS * sizeof(BUC_DATA);
buc_info = (BUC_DATA *) (iplcb_loc + current_off);
current_off += IPLD.buc_info_size;

IPLD.processor_info_offset = current_off;	/* processor_info	*/
IPLD.processor_info_size = sizeof(PROCESSOR_DATA);
proc_info = (PROCESSOR_DATA *) (iplcb_loc + current_off);
current_off += IPLD.processor_info_size;

IPLD.net_boot_results_offset = current_off;	/* network data 	*/
IPLD.net_boot_results_size = sizeof(NET_DATA);
net_data = (NET_DATA *) (iplcb_loc + current_off);
current_off += IPLD.net_boot_results_size;

IPLD.mem_data_offset = current_off;		/* mem_data		*/
IPLD.mem_data_size = MAX_NUM_OF_SIMMS * sizeof(MEM_DATA);
mem_data = (MEM_DATA *) (iplcb_loc + current_off);
current_off += IPLD.mem_data_size;

IPLD.l2_data_offset = current_off;		/* L2_data structure	*/
IPLD.l2_data_size = sizeof(L2_DATA);
l2_data = (L2_DATA *) (iplcb_loc + current_off);
current_off += IPLD.l2_data_size;

IPLD.residual_offset = current_off;		/* Residual data struct	*/
IPLD.residual_size = rp->ResidualLength;
res_ptr = (RESIDUAL *) (iplcb_loc + current_off);
memcpy((char*) res_ptr,(char *) rp,IPLD.residual_size);
current_off += IPLD.residual_size;

						/* Setup some IPLCB extensions*/

						/* ros entry table	*/
ros_table_size = sizeof(ROS_ENTRY_TABLE);
ros_table = (ROS_ENTRY_TABLE *) (iplcb_loc + current_off);
current_off += ros_table_size;

IPLD.nvram_cache_offset = current_off;		/* NVRAM Cache area	*/
IPLD.nvram_cache_size = NVRAM_SIZE;
nvram_cache = (uchar *) (iplcb_loc + current_off);
current_off += NVRAM_SIZE;

memset(nvram_cache, 0, NVRAM_SIZE);	/* Initialize NVRAM cache */

				/* For now leave BITMAP_SIZE as a constant*/
				/* Because that's how RAINBOW does it	 */

						/* Put bitmap at the end */
IPLD.bit_map_offset = iplcb_len - BITMAP_SIZE;
IPLD.bit_map_size = BITMAP_SIZE;

if((current_off + BITMAP_SIZE) > iplcb_len)	/* Sanity check		*/
	{
	printf("Internal Error: %d > %d\n",
		current_off+BITMAP_SIZE,iplcb_len);
	return(1);
	}

			/* Move bitmap to final location at the end of  */
					 /*	      the IPLCB area */
mem_loc = iplcb_loc + iplcb_len - BITMAP_SIZE;
memcpy(mem_loc,bitmap,BITMAP_SIZE);

dbgtst(AMON_D2) printf("Relocated bitmap to 0x%8.8X\n",mem_loc);

						/* Setup ASCII ID for ROS */
strcpy(IPLD.ipl_control_block_id, "ROSIPL \n");

IPLD.ipl_cb_and_bit_map_offset  = 0;
IPLD.ipl_cb_and_bit_map_size = IPLD.bit_map_offset + IPLD.bit_map_size;

					/* Fake up a copy of gpr save	*/
for (i=0; i < 32; i++)
	ipl_cb_ptr->gpr_save_area[i] = 0xFFFFFFFF;


/************************************************************************/
/* >>>>>>>>>>>>> INITIALIZE IPL CONTROL BLOCK STRUCTURES <<<<<<<<<<<<<< */
/************************************************************************/

	/**********************************/
	/* >>> IPL INFORMATION <<<<<<<<<< */
	/**********************************/

ipl_info->model = model;		/* Set Model Code		*/
ipl_info->ram_size = rp->GoodMemory;
ipl_info->bit_map_bytes_per_bit = BYTES_PER_BIT;
ipl_info->ros_entry_table_ptr = (void *) ros_table;
ipl_info->ros_entry_table_size = ros_table_size;
ipl_info->nvram_section_1_valid = 1;	/* Say NVRAM was OK		*/
strncpy(ipl_info->ipl_ros_timestamp,rostime,10);

/*
 *  The serial number should be a NULL terminated string of the form:
 *  "IBMxxxxyyyyyyy  "  where xxxx is the model number and yyyyyyy is
 *  the 7 digit serial number. (defect 169943)
 *
 *  The way we extract the 7 digit number is by searching for the NULL
 *  terminator, skipping over the last 2 characters, and taking the
 *  next 7.
 */
i=0;

while ((rp->VitalProductData.Serial[i] != 0x00) && (i < 64))
		i++;

resid_string_index = i - 9;
ipl_info->vpd_processor_serial_number[7] = 0x00;

/*
 * now we need to convert non-hexdigit characters in the serial number to
 * hex characters.  This is done by two different methods.  For older
 * firmware, we use the method of replacing non-hex characters with a "0".
 * For newer firmware, we map non-hex characters across the set of hex
 * characters.  We need to continue using the first method so that the
 * serial numbers on existing customers boxes doesn't change.  We determine
 * old or new firmware by using the type of boot device string, so the
 * logic for updating the serial numbers is included in the conditional
 * for boot device string processing.  (see defects 169943 & 175045)
 */

ipl_info->previpl_device[0] = '!';
ipl_info->previpl_device[1] = '!';
ipl_info->previpl_device[3] = 0;

/* figure out boot device type	   */

if (get_nv_env("fw-boot-device", nv_var)) {
	/* this is the format for vers/rev 0.1 (and newer) of firmware */

	/*
	 * replace non-hex characters in the serial number with hex
	 * characters using an even distribution across the set of
	 * printable ascii characters
	 */
	for(i=0; i<7; i++, resid_string_index++) {
		ipl_info->vpd_processor_serial_number[i] =
		    toupper(rp->VitalProductData.Serial[resid_string_index]);
		if(!isxdigit(ipl_info->vpd_processor_serial_number[i])) {
			ipl_info->vpd_processor_serial_number[i] %= 0x10;
			ipl_info->vpd_processor_serial_number[i] +=
				(ipl_info->vpd_processor_serial_number[i] <= 9)
					? 0x30 : 0x37;
		}
	}

	if (strstr(nv_var, "harddisk"))
		ipl_info->previpl_device[2] = 1;
	else if (strstr(nv_var, "cdrom"))
		ipl_info->previpl_device[2] = 3;
	else if (strstr(nv_var, "tape"))
		ipl_info->previpl_device[2] = 4;
	else if (strstr(nv_var, "floppy"))
		ipl_info->previpl_device[2] = 2;
	else
		ipl_info->previpl_device[2] = 5;


	strcat(ipl_info->previpl_device, nv_var);
}

else if(get_nv_env("fwBD", nv_var)) {
	/* this is the format for the older vers/rev 0.0 of firmware */
	char	*fw_id;			/* Firmware dev id from nvram */

	/* replace non-hex characters in the serial number with "0" */
	for(i=0; i<7; i++, resid_string_index++) {
		ipl_info->vpd_processor_serial_number[i] =
		    toupper(rp->VitalProductData.Serial[resid_string_index]);
		if(!isxdigit(ipl_info->vpd_processor_serial_number[i])) {
			ipl_info->vpd_processor_serial_number[i] = '0';
		}
	}

	dbgtst(AMON_D11) {
		printf("vers/rev 0.0 style NVRAM values: nv_var={%s}", nv_var);
	}

	for(i=0,j=0;i<10 && j<2;i++) {
		/* search for the 3rd field in NVRAM */
		if(nv_var[i] == ' ') {
			nv_var[i] = 0;
			fw_id = &nv_var[i+1];
			j++;
		}
	}

	fw_id[1] = 0;
	dbgtst(AMON_D11) {
		printf("\tfw_id=%s\n", fw_id);
	}

	if (nv_var[0] == '0') {
		/* disk boot */
		ipl_info->previpl_device[2] = 1;
		strcat(ipl_info->previpl_device,
			"/pci@80000000/pci1000,1@c,0/harddisk@");
		strcat(ipl_info->previpl_device, fw_id);
		strcat(ipl_info->previpl_device, ",0");
	}
	else if (nv_var[0] == '5') {
		/* CDROM boot */
		ipl_info->previpl_device[2] = 3;
		strcat(ipl_info->previpl_device,
			"/pci@80000000/pci1000,1@c,0/cdrom@");
		strcat(ipl_info->previpl_device, fw_id);
		strcat(ipl_info->previpl_device, ",0");
	}
	else if (strncmp(nv_var, "20", 2) == 0) {
		/* floppy boot */
		ipl_info->previpl_device[2] = 2;
		strcat(ipl_info->previpl_device,
			"/pci@80000000/pci8086,484@b,0/PNP0700@3f0/floppy@0");
	}
	else if (strncmp(nv_var, "c8", 2) == 0) {
		char port_string[16];

		/* ethernet boot device */
		ipl_info->previpl_device[2] = 5;
		/*
		 * The last field in the ethernet bootdev string indicates
		 * (connection_type,802_3).  There doesn't seem to be a place
		 * in the IPL control block for connection type, but it's
		 * here to be consistent with the boot device strings that
		 * are created by the newer firmware.
		 *	0 TPI (10base-T Compatible Squelch Level)
		 *	1 Thin Ethernet (10base2)
		 *	2 Thick Ethernet (10base5) (AUI port)
		 *	3 TPI (Reduced Squelch Level)
		 */
		strcat(ipl_info->previpl_device,
			"/pci@80000000/pci8086,484@b,0/IBM0010@");
		strcpy(port_string, "IBM0010");
		ser_to_io(port_string);
		strcat(ipl_info->previpl_device, port_string);
		/*
		 * The 3rd field in NVRAM is used for two different purposes
		 * with regard to network boot.
		 *
		 * If this was an ethernet boot, then the 3rd field is a flag
		 * that indicates whether this is an 802.3 ethernet.
		 * "1" = 802.3, "0" = not 802.3.
		 */
		if (fw_id[0] == '0')
			strcat(ipl_info->previpl_device, ":1,0");
		else
			strcat(ipl_info->previpl_device, ":1,1");
		/* do we care about this next one? */

		net_data->network_type = ROS_ENET;
	}
	else if (strncmp(nv_var, "e4", 2) == 0) {
		char port_string[16];

		/* token ring boot device */
		ipl_info->previpl_device[2] = 5;
		strcat(ipl_info->previpl_device,
			"/pci@80000000/pci8086,484@b,0/PNP80CC@");
		strcpy(port_string, "PNP80CC");
		ser_to_io(port_string);
		strcat(ipl_info->previpl_device, port_string);
		/*
		 * The 3rd field in NVRAM is used for two different purposes
		 * with regard to network boot.
		 *
		 * For token ring boot, the 3rd field is used to indicate
		 * ring speed.
		 * "0" = 4Mb, "1" = 16Mb
		 */
		if (fw_id[0] == '0')
			strcat(ipl_info->previpl_device, ":4");
		else
			strcat(ipl_info->previpl_device, ":10");

		/* do we care about this next one? */
		net_data->network_type = ROS_TOKEN;
	}
	else {
		/* it's old style firmware, but no valid data about bootdev */
		ipl_info->previpl_device[2] = '?';
		strcat(ipl_info->previpl_device, "unknown! unknown!");
	}
}

dbgtst(AMON_D2) printf("The IPLCB previpl_device string is:\n\t%s\n",
	ipl_info->previpl_device);

/*
 * If the monitor loadi function is used to load the boot image, the device
 * type and device id must be altered to match the selected device. This
 * value is set in the monitor in is_drive_valid routine.
 */

#ifdef DEBUG
if ((monitor_jump) || (ipl_info->previpl_device[2] == 2)) {
#else
if (monitor_jump) {
#endif /* DEBUG */
	char local1[8];

	ipl_info->previpl_device[3] = 0;
	strcat(ipl_info->previpl_device, "/pci@80000000/pci1000,1@c,0/");
	switch (mon_device_type) {
	case ROS_DASD:
		ipl_info->previpl_device[2] = 1;
		strcat(ipl_info->previpl_device, "harddisk@");
		break;
	case ROS_CDROM:
		ipl_info->previpl_device[2] = 3;
		strcat(ipl_info->previpl_device, "cdrom@");
		break;
	}
	itoa(mon_device_id, local1, 16);
	strcat(ipl_info->previpl_device, local1);
	strcat(ipl_info->previpl_device, ",0");

	dbgtst(AMON_D11) {
		printf("mon_device_type=0x%X, mon_device_id=0x%X\n",
			mon_device_type, mon_device_id);
		printf("The updated bootdev string is:\t%s\n",
			ipl_info->previpl_device);
	}
}

if (ipl_info->previpl_device[2] == 5) {
	unsigned char *c_ptr;

	/**********************************/
	/* >> NETWORK BOOT INFORMATION << */
	/**********************************/

	/*
	 * Check the boot device string and determine if there is an 802.3
	 * indicator.  This takes the form of xx,y at the end of the boot
	 * device string, where y == 0 or 1 to indicate whether 802.3 is true.
	 */
	if (c_ptr = strrchr(ipl_info->previpl_device, ',')) {
		if (c_ptr[1] == '1')
			net_data->is_802_3 = 1;
		else
			net_data->is_802_3 = 0;
	}
	else	/* don't leave garbage there */
		net_data->is_802_3 = 0;

	if(!get_nv_env("ClientIPAddr",&nv_var))
		nv_var[0] = 0;
	net_data->bootpr.yiaddr = net_data->bootpr.ciaddr = inet_addr(nv_var);

	if(!get_nv_env("ServerIPAddr",&nv_var))
		nv_var[0] = 0;
	net_data->bootpr.siaddr = inet_addr(nv_var);

	if(!get_nv_env("GatewayIPAddr",&nv_var))
		nv_var[0] = 0;
	net_data->bootpr.giaddr = inet_addr(nv_var);

	if(!get_nv_env("NetMask",&nv_var))
		nv_var[0] = 0;
	net_data->bootpr.vend[0] = 0x01;
	net_data->bootpr.vend[1] = 0x04;
	tlong = inet_addr(nv_var);
	memcpy(&net_data->bootpr.vend[2], &tlong, 4);

	if(!get_nv_env("boot-file", &nv_var))
		nv_var[0] = 0;
	strcpy(net_data->bootpr.file, nv_var);

	dbgtst(AMON_D11) {
		printf("Client IP Addr  = 0x%8.8X\n", net_data->bootpr.ciaddr);
		printf("Server IP Addr  = 0x%8.8X\n", net_data->bootpr.siaddr);
		printf("Gateway IP Addr = 0x%8.8X\n", net_data->bootpr.giaddr);
		printf("Boot file name  = %s\n", net_data->bootpr.file);
		printf("Vendor data     = ");
		for(i=0; i<8; i++)
			printf("%d.",net_data->bootpr.vend[i]);
		printf("\n");
	}
}


	/**********************************/
	/* >>> ROS INFORMATION <<<<<<<<<< */
	/**********************************/

ros_table->warm_ipl = (uint *) 0x00000100;

	/**********************************/
	/* >>> BUC INFORMATION <<<<<<<<<< */
	/**********************************/

/* ISA BUS information */
busnum = 0;
buc_info->num_of_structs = MAX_BUC_SLOTS;
buc_info->index = buc_index++;
buc_info->struct_size = sizeof(BUC_DATA);
buc_info->bsrr_offset = 0;
buc_info->bsrr_mask = 0;
buc_info->bscr_value = 0;
buc_info->cfg_status = BUC_HW_CONFIGURED;
buc_info->device_type = A_BUS_BRIDGE;
buc_info->num_of_buids = 2;
buc_info->buid_data[0].buid_value = BID_VAL(IO_ISA, ISA_IOMEM, 0);
buc_info->buid_data[0].buid_Sptr = (void *)0x80000000;
buc_info->buid_data[1].buid_value = BID_VAL(IO_ISA, ISA_BUSMEM, 0);
buc_info->buid_data[1].buid_Sptr = (void *)0xC0000000;
buc_info->buid_data[2].buid_value = NOT_APPLICABLE;
buc_info->buid_data[2].buid_Sptr = (void *) 0;
buc_info->buid_data[3].buid_value = NOT_APPLICABLE;
buc_info->buid_data[3].buid_Sptr = (void *) 0;
buc_info->mem_alloc1 = 0;
buc_info->mem_addr1 = (void *) 0;
buc_info->mem_alloc2 = 0;
buc_info->mem_addr2 = (void *) 0;
buc_info->vpd_rom_width = NOT_APPLICABLE;
buc_info->cfg_addr_inc = 0;
buc_info->device_id_reg = ISA_BUS_ID;
buc_info->aux_info_offset = 0;
buc_info->feature_rom_code = 0;
buc_info->IOCC_flag = 1;			/* Indicated NIO region */
*((uint *)buc_info->location) = buc_loc[busnum];
busnum++;
++buc_info;
pcibus_id = 0;

for (i=0;i<rp->ActualNumDevices;i++) {
	if ((rp->Devices[i].DeviceId.BaseType == BridgeController) &&
	    (rp->Devices[i].DeviceId.SubType == PCIBridge)) {
		/* PCI BUS information */
		buc_info->num_of_structs = MAX_BUC_SLOTS;
		buc_info->index = buc_index++;
		buc_info->struct_size = sizeof(BUC_DATA);
		buc_info->bsrr_offset = 0;
		buc_info->bsrr_mask = 0;
		buc_info->cfg_status = BUC_HW_CONFIGURED;
		buc_info->device_type = A_BUS_BRIDGE;
		buc_info->buid_data[0].buid_value =
		    BID_VAL(IO_PCI, PCI_IOMEM, pcibus_id);
		buc_info->buid_data[0].buid_Sptr = (void *)0x80000000;
		buc_info->buid_data[1].buid_value =
		    BID_VAL(IO_PCI, PCI_BUSMEM, pcibus_id);
		buc_info->buid_data[1].buid_Sptr = (void *)0xC0000000;
		if (pci_type == 0) {
			/*
			 * PCI 1.0 type configuration
			 */
			buc_info->num_of_buids = 3;
			buc_info->buid_data[2].buid_value =
			    BID_VAL(IO_PCI, PCI_CFGMEM, pcibus_id);
			buc_info->buid_data[2].buid_Sptr = (void *)0x80800000;
		} else {
			/*
			 * Must be PCI 2.0 type configuration
			 */
			buc_info->num_of_buids = 2;
			buc_info->buid_data[2].buid_value = NOT_APPLICABLE;
			buc_info->buid_data[2].buid_Sptr = (void *) 0;
		}
		pcibus_id++;
		buc_info->buid_data[3].buid_value = NOT_APPLICABLE;
		buc_info->buid_data[3].buid_Sptr = (void *) 0;
		buc_info->mem_alloc1 = 0;
		buc_info->mem_alloc2 = 0;
		buc_info->mem_addr1 = (void *) 0;
		buc_info->mem_addr2 = (void *) 0;
		buc_info->vpd_rom_width = NOT_APPLICABLE;
		buc_info->cfg_addr_inc = 0;
		buc_info->device_id_reg = PCI_BUS_ID;
		buc_info->aux_info_offset = 0;
		buc_info->feature_rom_code = 0;
		buc_info->IOCC_flag = 0;	/* Indicated NIO region */
		*((uint *)buc_info->location) = buc_loc[busnum];
                L4_packet = (PCI_BRIDGE_DESCRIPTOR *)
                            &rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset];

                while (L4_packet->tag != 0x78)
                {
                    if ((L4_packet->tag == 0x84) && (L4_packet->rspc_type == 3))
                    {
                        buc_info->mem_addr1 =
		            (void *) (L4_packet->config_base_addr_1 +
                             (L4_packet->config_base_addr_2 << 8) +
                             (L4_packet->config_base_addr_3 << 16) +
                             ((L4_packet->config_base_addr_4 & 0xf) << 24));

                        buc_info->mem_addr2 =
			    (void *) (L4_packet->config_data_addr_1 +
                             (L4_packet->config_data_addr_2 << 8) +
                             (L4_packet->config_data_addr_3 << 16) +
                             ((L4_packet->config_data_addr_4 & 0xf) << 24));
		        buc_info->bscr_value = L4_packet->bus_num;
                        break;
                    }
                    else
                    {
                        if (L4_packet->tag & 0x80)
                        {
                            L4_packet = (PCI_BRIDGE_DESCRIPTOR *)
		   		         ((int) L4_packet +
                                         (int) L4_packet->length1 +
                                         (int) (L4_packet->length2 << 8) + 3);
                        }
                        else
                        {
                            L4_packet = (PCI_BRIDGE_DESCRIPTOR *)
					 ((int) L4_packet +
                                          (int) (L4_packet->tag & 0x7) + 1);
                        }
                    }
                } /* end while  */
                busnum++;
		++buc_info;
	} else if ((rp->Devices[i].DeviceId.BaseType == BridgeController) &&
	    (rp->Devices[i].DeviceId.SubType == PCMCIABridge)) {
		/* PCMCIA BUS information */
		buc_info->num_of_structs = MAX_BUC_SLOTS;
		buc_info->index = buc_index++;
		buc_info->struct_size = sizeof(BUC_DATA);
		buc_info->bsrr_offset = 0;
		buc_info->bsrr_mask = 0;
		buc_info->bscr_value = 0;
		buc_info->cfg_status = BUC_HW_CONFIGURED;
		buc_info->device_type = A_BUS_BRIDGE;
		buc_info->num_of_buids = 2;
		buc_info->buid_data[0].buid_value =
		    BID_VAL(IO_ISA, ISA_IOMEM, 0);
		buc_info->buid_data[0].buid_Sptr = (void *)0x80000000;
		buc_info->buid_data[1].buid_value =
		    BID_VAL(IO_ISA, ISA_BUSMEM, 0);
		buc_info->buid_data[1].buid_Sptr = (void *)0xC0000000;
		buc_info->buid_data[2].buid_value = NOT_APPLICABLE;
		buc_info->buid_data[2].buid_Sptr = (void *) 0;
		buc_info->buid_data[3].buid_value = NOT_APPLICABLE;
		buc_info->buid_data[3].buid_Sptr = (void *) 0;
		buc_info->mem_alloc1 = 0;
		buc_info->mem_addr1 = (void *) 0;
		buc_info->mem_alloc2 = 0;
		buc_info->mem_addr2 = (void *) 0;
		buc_info->vpd_rom_width = NOT_APPLICABLE;
		buc_info->cfg_addr_inc = 0;
		buc_info->device_id_reg = PCMCIA_BUS_ID;
		buc_info->aux_info_offset = 0;
		buc_info->feature_rom_code = 0;
		buc_info->IOCC_flag = 0;	/* Indicated NIO region */
		*((uint *)buc_info->location) = buc_loc[busnum];
                busnum++;
		++buc_info;
	}
}

/* DMA reserved memory area	*/
if (busnum < MAX_BUC_SLOTS)
{

	buc_info->num_of_structs = MAX_BUC_SLOTS;
	buc_info->index = buc_index++;
	buc_info->struct_size = sizeof(BUC_DATA);
	buc_info->bsrr_offset = 0;
	buc_info->bsrr_mask = 0;
	buc_info->bscr_value = 0;
	buc_info->cfg_status = BUC_HW_CONFIGURED;
	buc_info->device_type = A_BUS_BRIDGE;
	buc_info->num_of_buids = 0;
	buc_info->buid_data[0].buid_value = NOT_APPLICABLE;
	buc_info->buid_data[0].buid_Sptr = (void *) 0;
	buc_info->buid_data[1].buid_value = NOT_APPLICABLE;
	buc_info->buid_data[1].buid_Sptr = (void *) 0;
	buc_info->buid_data[2].buid_value = NOT_APPLICABLE;
	buc_info->buid_data[2].buid_Sptr = (void *) 0;
	buc_info->buid_data[3].buid_value = NOT_APPLICABLE;
	buc_info->buid_data[3].buid_Sptr = (void *) 0;
	buc_info->mem_alloc1 = DMA_BUF_SIZE;
	buc_info->mem_addr1 = (void *)dma_buffer;
	buc_info->mem_alloc2 = 0;
	buc_info->mem_addr2 = (void *) 0;
	buc_info->vpd_rom_width = NOT_APPLICABLE;
	buc_info->cfg_addr_inc = 0;
	buc_info->device_id_reg = DMA_BUC_ID;
	buc_info->aux_info_offset = 0;
	buc_info->feature_rom_code = 0;
	buc_info->IOCC_flag = 0;		/* Indicated NIO region */
	*((uint *)buc_info->location) = buc_loc[busnum];
	busnum++;
	++buc_info;
}

#ifdef PM_SUPPORT
/* hibernation wakeup area */
if (busnum < MAX_BUC_SLOTS)
{
	buc_info->num_of_structs = MAX_BUC_SLOTS;
	buc_info->index = buc_index++;
	buc_info->struct_size = sizeof(BUC_DATA);
	buc_info->bsrr_offset = 0;
	buc_info->bsrr_mask = 0;
	buc_info->bscr_value = 0;
	buc_info->cfg_status = BUC_HW_CONFIGURED;
	buc_info->device_type = A_BUS_BRIDGE;
	buc_info->num_of_buids = 0;
	buc_info->buid_data[0].buid_value = NOT_APPLICABLE;
	buc_info->buid_data[0].buid_Sptr = (void *) 0;
	buc_info->buid_data[1].buid_value = NOT_APPLICABLE;
	buc_info->buid_data[1].buid_Sptr = (void *) 0;
	buc_info->buid_data[2].buid_value = NOT_APPLICABLE;
	buc_info->buid_data[2].buid_Sptr = (void *) 0;
	buc_info->buid_data[3].buid_value = NOT_APPLICABLE;
	buc_info->buid_data[3].buid_Sptr = (void *) 0;
	buc_info->mem_alloc1 = WAKEUP_AREA_LENGTH;
	buc_info->mem_addr1 = (void *)wakeup_area;
	buc_info->mem_alloc2 = 0;
	buc_info->mem_addr2 = (void *) 0;
	buc_info->vpd_rom_width = NOT_APPLICABLE;
	buc_info->cfg_addr_inc = 0;
	buc_info->device_id_reg = HIBERNATION_BUC_ID;
	buc_info->aux_info_offset = 0;
	buc_info->feature_rom_code = 0;
	buc_info->IOCC_flag = 0;
	*((uint *)buc_info->location) = buc_loc[busnum];
	busnum++;
	++buc_info;
}
#endif /* PM_SUPPORT */

	/**********************************/
	/* >>> SYSINFO INFORMATION <<<<<< */
	/**********************************/

sys_info->nvram_size = NVRAM_SIZE;
sys_info->nvram_addr = (void *) (iplcb_loc + IPLD.nvram_cache_offset);
sys_info->architecture = RSPC;
sys_info->implementation = RSPC_UP_PCI;
strcpy(sys_info->pkg_descriptor,"rspc");


	/**********************************/
	/* >>> L2 CACHE INFORMATION <<<<< */
	/**********************************/

/* first initialize the pertinent fields */
l2_data->installed_size = l2_data->configured_size = l2_data->size[0] = 0;
proc_info->L2_cache_asc = 0;

/* now search the dev array for L2 */
for (i=0;i<rp->ActualNumDevices;i++) {
	if (rp->Devices[i].DeviceId.DevId == encode_dev(PNPL2)) {
		unsigned char *c_ptr;

		/* then a DevId for L2 was found */
		c_ptr = &rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset];
		for (j=0; c_ptr[j] != 0x78; j++) {
			if (c_ptr[j] == 0x84 && c_ptr[j+3] == 2) {
				/* this is L2 cache info */
				l2_data->installed_size =
					l2_data->configured_size =
					l2_data->size[0] = c_ptr[j+4] +
					(c_ptr[j+5] << 8) + (c_ptr[j+6] << 16) +
					(c_ptr[j+7] << 24);

				/*
				 * set this field from proc_info while we have
				 * this pnpheap address
				 */
				proc_info->L2_cache_asc = c_ptr[j+8] +
					(c_ptr[j+9] << 8);
				break;
			}
		}
		break;
	}
}

/* set this field from proc_info while we're processing other L2 information */
proc_info->L2_cache_size = l2_data->installed_size * 1024;

l2_data->num_of_structs = 1;
l2_data->index = 0;
l2_data->struct_size = sizeof(L2_DATA);
l2_data->shared_L2_cache = 0;
l2_data->using_resource_offset = IPLD.processor_info_offset;

l2_data->mode = 0;
l2_data->type[0] = l2_data->type[1] = '0';
l2_data->adapter_present = (l2_data->installed_size > 0) ? 1:0;
l2_data->adapter_bad = 0;

	/**********************************/
	/* >>> PROC INFORMATION <<<<<<<<< */
	/**********************************/

proc_info->num_of_structs = 1;
proc_info->index = 0;
proc_info->struct_size = sizeof(PROCESSOR_DATA);
proc_info->per_buc_info_offset = (uint)buc_info;
proc_info->proc_int_area = (void *) NOT_MP;
proc_info->proc_int_area_size = NOT_MP;
proc_info->processor_present = PROCESSOR_RUN_AIX;
proc_info->test_run = 0x000000D5;
proc_info->test_stat = 0;
proc_info->link = NOT_USED;
proc_info->link_address = (void *) NOT_USED;
proc_info->phys_id = NOT_MP;
proc_info->priv_lck_cnt = 0;
proc_info->prob_lck_cnt = 0;
			/* Get values from aixrosmem	*/

proc_info->architecture = ARCHITECTURE;
/*
 * Set implementation and version number based on processor
 */
switch (processor) {
	case VER_601:
		proc_info->implementation = POWER_601;
		proc_info->version = read_pvr();
		strcpy(proc_info->proc_descriptor,"PowerPC_601");
		break;
	case VER_603:
	case VER_603PLUS:
		proc_info->implementation = POWER_603;
		proc_info->version = PV_603;
		strcpy(proc_info->proc_descriptor,"PowerPC_603");
		break;
	case VER_604:
	case VER_604PLUS:
		proc_info->implementation = POWER_604;
		proc_info->version = PV_604;
		strcpy(proc_info->proc_descriptor,"PowerPC_604");
		break;
}

proc_info->width = rp->VitalProductData.WordWidth;
if (rp->VitalProductData.CacheAttrib == NoneCAC)
	/*
	 * No Cache present
	 */
	proc_info->cache_attrib = 0x0;
else if (rp->VitalProductData.CacheAttrib == SplitCAC)
	/*
	 * Split I and D
	 */
	proc_info->cache_attrib = 0x1;
else
	/*
	 * Combined I and D
	 */
	proc_info->cache_attrib = 0x3;
/* cache sizes in the IPLCB are represented in bytes, vs. kb in residual */
proc_info->icache_size = rp->VitalProductData.I_CacheSize << 10;
proc_info->dcache_size = rp->VitalProductData.D_CacheSize << 10;
proc_info->icache_asc = rp->VitalProductData.I_CacheAssoc;
proc_info->dcache_asc = rp->VitalProductData.D_CacheAssoc;
if (rp->VitalProductData.TLBAttrib == NoneTLB)
	/*
	 * No TLB present
	 */
	proc_info->tlb_attrib = 0x0;
else if (rp->VitalProductData.TLBAttrib == SplitTLB)
	/*
	 * Split I and D
	 */
	proc_info->tlb_attrib = 0x1;
else
	/*
	 * Combined I and D
	 */
	proc_info->tlb_attrib = 0x3;
proc_info->itlb_size = rp->VitalProductData.I_TLBSize;
proc_info->dtlb_size = rp->VitalProductData.D_TLBSize;
proc_info->itlb_asc = rp->VitalProductData.I_TLBAssoc;
proc_info->dtlb_asc = rp->VitalProductData.D_TLBAssoc;
proc_info->slb_attrib = SLB_ATTRIB;
proc_info->islb_size = ISLB_SIZE;
proc_info->dslb_size = DSLB_SIZE;
proc_info->islb_asc = ISLB_ASC;
proc_info->dslb_asc = DSLB_ASC;
if (processor == VER_601) {
	proc_info->rtc_type = RTC_POWER;
} else {
	proc_info->rtc_type = RTC_POWER_PC;
}
proc_info->rtcXint = RTCXINT;
proc_info->rtcXfrac = RTCXFRAC;

/*
 *	Set TB frequency.  We compute this as follows:
 *
 *      TB_frequency = Processor_Bus_Hz * 1000000000 / TB_divisor;
 */
if (ProcessorBusHz > 1000) {
	/*
	 * Must be in Hz
	 */
	proc_info->tbCfreq_HZ = ProcessorBusHz /
	   (rp->VitalProductData.TimeBaseDivisor/1000);
} else {
	/*
	 * Must be in MHz (old ROS)
	 */
	proc_info->tbCfreq_HZ = (ProcessorBusHz * 1000000) /
	    (rp->VitalProductData.TimeBaseDivisor/1000);
}
proc_info->coherency_size = rp->VitalProductData.CoherenceBlockSize;
proc_info->resv_size = 0;
proc_info->icache_block = rp->VitalProductData.CoherenceBlockSize;
proc_info->dcache_block = rp->VitalProductData.CoherenceBlockSize;
proc_info->icache_line = rp->VitalProductData.I_CacheLineSize;
proc_info->dcache_line = rp->VitalProductData.D_CacheLineSize;


	/**********************************/
	/* >>> MEM_DATA INFORMATION <<<<< */
	/**********************************/

for(i=0;i<MAX_NUM_OF_SIMMS;i++)
	{
	mem_data[i].num_of_structs = MAX_NUM_OF_SIMMS;
	mem_data[i].struct_size = sizeof(MEM_DATA);
	if (i < rp->VitalProductData.NumSIMMSlots)
		mem_data[i].card_or_SIMM_size = rp->Memories[i].SIMMSize;
	else
		mem_data[i].card_or_SIMM_size = 0;
	mem_data[i].state =
	    (mem_data[i].card_or_SIMM_size) ? IS_GOOD : IS_EMPTY;
	mem_data[i].num_of_bad_simms = 0;
	mem_data[i].card_or_simm_indicator = SIMM;
	mem_data[i].EC_level = 0;
	mem_data[i].PD_bits = 0;
	mem_data[i].location [0][0] = '0';
	mem_data[i].location [0][1] = '0';
	mem_data[i].location [0][2] = '0';
	mem_data[i].location [0][3] = 'A'+ (char) i;
	}

/************************************************************************/
/* >>>>>>>>>>>>>>>>> END OF STRUCTURE INITIALIZATION <<<<<<<<<<<<<<<<<< */
/************************************************************************/


			/* Here is some debug stuff before we exit	*/

dbgtst(AMON_D16)
	{
	printf("Memory address of IPL Control Block = 0x%8.8X\n",ipl_cb_ptr);
	printf("Directory: ......... 0x%8.8X    offset: 0x%8.8X\n",
		&IPLD,4*32);
	printf("IPL_info: .......... 0x%8.8X    offset: 0x%8.8X\n",
		ipl_info,IPLD.ipl_info_offset);
	printf("System area: ....... 0x%8.8X    offset: 0x%8.8X\n",
		sys_info,IPLD.system_info_offset);
	printf("Buc Data area: ..... 0x%8.8X    offset: 0x%8.8X\n",
		buc_info,IPLD.buc_info_offset);
	printf("Processor data area: 0x%8.8X    offset: 0x%8.8X\n",
		proc_info,IPLD.processor_info_offset);
	printf("L2_cache data area:  0x%8.8X    offset: 0x%8.8X\n",
		l2_data,IPLD.l2_data_offset);
	printf("Network data area: . 0x%8.8X    offset: 0x%8.8X\n",
		net_data,IPLD.net_boot_results_offset);
	printf("Memory data area: .. 0x%8.8X    offset: 0x%8.8X\n",
		mem_data,IPLD.mem_data_offset);
	printf("NVRAM cache area . : 0x%8.8X    offset: 0x%8.8X\n",
		nvram_cache,IPLD.nvram_cache_offset);
	printf("Residual data area : 0x%8.8X    offset: 0x%8.8X\n",
		res_ptr,IPLD.residual_offset);
	}






	/****************************************************************/
	/**** ALL DONE - Let's return ***********************************/
	/****************************************************************/

					/* Fill in return structure	*/
ipldata->iplcbptr = (unsigned int) ipl_cb_ptr;
ipldata->bit_point = (uchar *)ipl_cb_ptr->s0.bit_map_offset;
ipldata->bit_size = BYTES_PER_BIT;
ipldata->map_size = BITMAP_SIZE;

}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
	mem_find(bitmap, length, dma_buff)
		This routine checks the bitmap for system memory and
		finds a contiguous chunck of memory beginning at the
		top of memory.
		Input:
			bitmap - pointer to system bitmap
			length - length of reserve area for iplcb
			dma_buffer - pointer to dma buffer

		Returns:
			memory address of start of memory area.
			bitmap is filled in on return
			dma_buffer address filled in on return

*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/


mem_find(uint *bitmap,int length, int *dma_buffer, int *wakeup_area)
{
int	i,j,k;
int	map_bytes;			/* Total valid bytes in bitmap	*/
uint	tot_mem;			/* Total system memory in bytes */
uint	dma_mem;			/* Memory to count for dma      */
int	mask;				/* Mask to check for bits	*/
uint	mem_addr;			/* Memory address		*/
char	*mem_ptr;
int	bits_found;			/* Contiguous good bits found   */
int	dma_bits_found;
int	bits_needed;			/* bits needed			*/
int	dma_bits_needed;
int	bits_used;			/* bits used to find good chunk */
int	dma_bits_used;
uint	dma_addr;
char	*dma_buff;
char	pattern[8] = 			/* Memory fill pattern		*/
	{ 0xde,0xad,0xbe,0xef,
	  0xde,0xad,0xbe,0xef };
RESIDUAL *rp;
#define	SEGSIZE	(256*1024*1024)		/* Segment Size, PowerPC, 256Mb */

#ifdef PM_SUPPORT
int	pm_descriptor;			/* PM event descriptor		*/
char	*p;				/* temporary pointer for PM	*/
#endif /* PM_SUPPORT */

rp = (RESIDUAL *) residual_save;

dbgmsg(AMON_D1,"- mem_find()");

					/* Call setup_bitmap to setup bitmap
					   and get total bytes in map
					   aixrosmem expects the bitmap
					   to be an array of int so divide
					   the size by 4		*/

map_bytes = setup_bitmap(BITMAP_SIZE/4,bitmap);
map_bytes *= 4;				/* Readjust return value to char*/

tot_mem = rp->GoodMemory;

					/* Check the bitmap from top down
					   to find a good memory chunk  */
bits_needed = length/BYTES_PER_BIT;	/* Number of bits to check      */

bits_found = 0;				/* Initialize stuff		*/
bits_used = 0;
mem_addr = 0;

for(i=((map_bytes/4)-1);i>0 && bits_found < bits_needed ;i--)
	{
	mask = 0x00000001;
	for(j=0;j<32 && bits_found < bits_needed ;j++)
		{
		bits_used++;
		if(bitmap[i] & mask)
			bits_found = 0;
		else
			bits_found++;
		mask = mask << 1;
		}
	}
if(i)					/* Compute address 		*/
	{
	i++; 				/* Adjust index for next search */

	/* This code exists to ensure that the IPLCB does not get placed at
	 * such an address that it includes the last page of the first 256Mb
	 * of physical memory, 0x0FFFF000.  That was the default placement on
	 * a machine with 256Mb of real memory, and vmsi() cannot handle there
	 * being valid data there.  This implementation imposes a 128Mb limit
	 * for the top of the IPLCB in Real Memory. */

	if ( tot_mem > SEGSIZE / 2 )	/* 128Mb */
		mem_addr = ( SEGSIZE / 2 ) - (bits_used * BYTES_PER_BIT);
	else
		mem_addr = tot_mem - (bits_used * BYTES_PER_BIT);

	*(&mem_ptr) = (char *)mem_addr;
	}

					/* Start where we left off and find
					   space for a DMA buffer	*/
dma_bits_needed = DMA_BUF_SIZE/BYTES_PER_BIT;	/* Number of bits to check   */

dma_bits_found = 0;			/* Initialize stuff		*/
dma_bits_used = 0;
dma_addr = 0;
dma_mem = (tot_mem > 0x1000000)?0x1000000:tot_mem;

					/* Make sure dma buffer is in 1st 16M */
if(((dma_mem/(32*BYTES_PER_BIT))-1) < i)
	{
	i = ((dma_mem/(32*BYTES_PER_BIT))-1);
	j = 0;
	bits_used = 0;
	mask = 0x00000001;
	}

for(;i>0 && dma_bits_found < dma_bits_needed ;i--)
	{
	for(;j<32 && dma_bits_found < dma_bits_needed ;j++)
		{
		dma_bits_used++;
		if(bitmap[i] & mask)
			dma_bits_found = 0;
		else
			{
			dma_bits_found++;
			bitmap[i] |= mask;	/* Turn the bit off	*/
			}
		mask = mask << 1;
		}
	mask = 0x00000001;
	j = 0;
	}
if(i)					/* Still some left		*/
	{
	dma_addr = dma_mem - ((dma_bits_used+bits_used) * BYTES_PER_BIT);
	*(dma_buffer) = dma_addr;
	*(&dma_buff) = (char *)dma_addr;
	}

for(i=0;i<length;i++)			/* Fill IPL control block with 0*/
	mem_ptr[i] = 0;

for(i=0;i<DMA_BUF_SIZE;i++)		/* Fill DMA memory 	*/
	dma_buff[i] = 0;

#ifdef PM_SUPPORT
/*
 * Create wakeup area for hibernation. It must be countiguous.
 */
#define HIBERNATION		0x01		/* hibernation support */

pm_descriptor = 0;

/* search for Power Management Packet */
for(i = 0; i < rp->ActualNumDevices; i++)
	{
	if(rp->Devices[i].DeviceId.DevId == 0x244d0005 &&
		rp->Devices[i].DeviceId.Interface == GeneralPowerManagement)
		{
		break;
		}
	}

if(i < rp->ActualNumDevices)
	{
	/* search for Power Management Descriptor */
	p = rp->DevicePnPHeap;
	for(i = rp->Devices[i].AllocatedOffset; p[i] != END_TAG; i++)
		{
		if(p[i] == L4_Packet && p[i+3] == 8)
			{
			pm_descriptor = (int)p[i+4] + ((int)p[i+5] << 8) +
				((int)p[i+6] << 16) + ((int)p[i+7] << 24);
			}
		}
	}

if((pm_descriptor & HIBERNATION) != 0)
	{	int	wakeup_bits_found, wakeup_bits_needed;
	wakeup_bits_found = 0;
	wakeup_bits_needed = WAKEUP_AREA_LENGTH / BYTES_PER_BIT;
	i = (tot_mem - 0x100000) / (BYTES_PER_BIT * 32) - 1;
	for(; i > 0 && wakeup_bits_found < wakeup_bits_needed; i--)
		{
		for(j = 0;
			j < 32 && wakeup_bits_found < wakeup_bits_needed; j++)
			{
			if(bitmap[i] & (0x00000001 << j))
				wakeup_bits_found = 0;
			else
				wakeup_bits_found++;
			}
		}
	if(wakeup_bits_found >= wakeup_bits_needed)
		{
		i++;
		*wakeup_area = (i * 32 + (32 - j)) * BYTES_PER_BIT;
		for(; wakeup_bits_needed > 0 && j > 0;
						j--, wakeup_bits_needed--)
			bitmap[i] |= (0x00000001 << (j - 1));
		for(j = 32; wakeup_bits_needed > 0 && j > 0;
						j--, wakeup_bits_needed--)
			bitmap[i + 1] |= (0x00000001 << (j - 1));
		}
	}
#endif /* PM_SUPPORT */

dbgtst(AMON_D3)
	{
	printf("Ready to return from mem_find\n");
	printf("Total memory  = 0x%8.8X  %d  %d K\n",tot_mem,tot_mem,tot_mem/1024);
	tot_mem = rp->GoodMemory;
	printf("GoodMemory    = 0x%8.8X  %d  %d K\n",tot_mem,tot_mem,tot_mem/1024);
	printf("IPLCB address = 0x%8.8X  %d\n",mem_addr,mem_addr);
	printf("Misc Stuff:\n");
	printf("Value  bits: used = %d  needed = %d  found = %d\n",
		bits_used,bits_needed,bits_found);
	printf("Value of map_bytes = 0x%8.8X  %d\n",map_bytes,map_bytes);
	printf("dma_addr = 0x%8.8X 0x%8.8X\n",dma_addr, dma_buff);
	printf("Value  dma_bits: used = %d  needed = %d  found = %d\n",
		dma_bits_used,dma_bits_needed,dma_bits_found);
	}

return(mem_addr);
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
	inet_addr(ip_address)
	Code lifted from libc and used here
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

u_long inet_addr(char *cp)
{
u_long 	val, base, n, num;
char 	c;
u_long 	parts[4], *pp = parts;
int 	i;

again:
	/*
	 * Collect number up to ``.''.
	 * Values are specified as for C:
	 * 0x=hex, 0=octal, other=decimal.
	 */
val = 0; base = 10;
if (*cp == '0')
	{
	base = 8, cp++;
	if (*cp == 'x' || *cp == 'X')
		base = 16, cp++;
	}
while (c = *cp)
	{
	if (isdigit(c))
		{
		num = c - '0';
		if (num >= base)
			return -1;
		val = (val * base) + num;
		cp++;
		continue;
		}
	if (base == 16 && isxdigit(c))
		{
		val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
		cp++;
		continue;
		}
	break;
	}
				/*
				 * Internet format:
				 *	a.b.c.d
				 *	a.b.c	(with c treated as 16-bits)
				 *	a.b	(with b treated as 24 bits)
				 */
if (*cp == '.')
	{
	if (pp >= parts + 4)
		return (-1);
	*pp++ = val, cp++;
	goto again;
	}
				/*
				 * Check for trailing characters.
				 */
if (*cp && !isspace(*cp))
	return (-1);
*pp++ = val;
				/*
				 * Concoct the address according to
				 * the number of parts specified.
				 */
n = pp - parts;
switch (n)
	{
	case 1:				/* a -- 32 bits */
	val = parts[0];
	break;

	case 2:				/* a.b -- 8.24 bits */
	if (parts[0] > (u_long)0xff || parts[1] > (u_long)0xffffff)
		return -1;
	val = (parts[0] << 24) | (parts[1] & 0xffffff);
	break;

	case 3:				/* a.b.c -- 8.8.16 bits */
	if (parts[0] > (u_long)0xff || parts[1] > (u_long)0xff
		|| parts[2] > (u_long)0xffff)
			return -1;
	val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
		(parts[2] & 0xffff);
	break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
	for (i = 0; i < 4; i++)
		if (parts[i] > (u_long)0xff)
			return -1;
	val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
	      ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
	break;

	default:
	return (-1);
	}
			/* htonl not in softros lib should not need it
			   since all work is done in big endian */
/* val = htonl(val); */
return (val);
}

/******************************************************************************/
/*
 * encode_dev
 * This function converts a string that represents a device from pnp.h
 * and converts it to the 32 bit vendor id that is described in section
 * 6.1.1 of Plug and Play ISA Specification, Version 1.0a.  For example,
 * the string "IBM0007" is converted to 244D0007, which can then be used
 * to identify devices in the device array.
 */


unsigned long encode_dev(char *in_string) {

int     i,j;
char    buf[8];
unsigned long ret_val;

/* first copy the string to a char array so we can change each element */
memcpy(buf, in_string, 8);

/*
 * now convert each character from ASCII to that pesky 5 bit compressed
 * PNP heap format.  Subtracting '@' does that for us.  Then mask off all
 * except for the 5 bits we care about and left shift it.  Do this for each
 * of the 3 characters that comprise the vendor identification.
 */
ret_val = (((buf[0] - '@') & 0x1F) << 26) +
	(((buf[1] - '@') & 0x1F) << 21) +
	(((buf[2] - '@') & 0x1F) << 16);

/*
 * now there are 4 characters left, which represent the manufacturer device
 * specific code.  Convert each character from ascii to hex and shift it home.
 */
for (i=3, j=3; i<7; i++, j--) {
	if (buf[i] > '9')
		buf[i] -= '7';
	else
		buf[i] -= '0';
	ret_val += (buf[i] & 0x0F) << (j*4);
}

return(ret_val);
}

/******************************************************************************/
/*
 * Given the pnp number for a network boot device, return the base I/O
 * port address.  We assume that every network boot device will contain
 * an I/O base port address in the Allocated pnpheap area.
 */

int
ser_to_io(char *pnp_string)
{
int i;
RESIDUAL *rp;
unsigned long DevId;
char *c_ptr;

rp = (RESIDUAL *) residual_save;

/* convert string to compressed pnp format */
DevId = encode_dev(pnp_string);

/* search device array for matching string */
for (i=0; i<rp->ActualNumDevices; i++) {
	if (rp->Devices[i].DeviceId.DevId == DevId) {
		/* found a match; now search for I/O port resource packet */
		c_ptr = &rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset];
		while ((*c_ptr != 0x47) && (*c_ptr != 0x4B) &&
			(*c_ptr != 0x78)) {
				c_ptr++;
		}

		/*
		 * now we're pointing to an I/O resource packet OR an end tag.
		 * convert it to an ascii string if it's an I/O resource packet
		 */
		if (c_ptr[0] == 0x47)
			itoa((c_ptr[2] + (c_ptr[3] << 8)), pnp_string, 16);
		else if (c_ptr[0] == 0x4B)
			itoa((c_ptr[1] + (c_ptr[2] << 8)), pnp_string, 16);
		else
			/*
			 * it is 0x78, and we didn't find the I/O packet.
			 * This shouldn't happen.
			 */
			*pnp_string = 0;

		return(0);
	}
}
}
