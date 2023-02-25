static char sccsid[] = "@(#)91	1.28  src/rspc/usr/lib/boot/softros/aixmon/aixmon.c, rspc_softros, rspc41J, 9520B_all 5/18/95 17:28:05";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: do_command
 *		main
 *		system_init
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

/* Bringup Monitor for rspc -- aixmon.c			   */

#define IN_MAIN
#include "aixmon.h"
#include <sys/residual.h>
#include "res_patch.h"

extern uint ProcessorBusHz;
#define MAX_DEV_FUNC	36
struct ipldevs {
		unsigned char dev_func;
		unsigned char bus_number;
		} IPLDEVS[MAX_DEV_FUNC];
int numipldevs;
unsigned char setup_flag = 0;

#define NoERROR (0)
#define ERROR   (1)

main()
{
int     i;
char    inbuf[BUFSIZ];
char    **args;
char *c_ptr;
int     argc;
RESIDUAL	*rp;		    /* Pointer to residual data */

struct ipldevs *rddevp;
dbgmsg(AMON_D1,"- main()");		/* Initialize the Environment	*/


rp = (RESIDUAL *) residual_save;	/* Let's point at residual data */

pci_type = 0xff;
nvram_addr = 0xffff;
nvram_data = 0xffff;
scsi_bus_number = 0xff;
scsi_dev_func = 0xff;
numipldevs = 0;
for (i=0; i<MAX_DEV_FUNC; i++)
{
	IPLDEVS[i].dev_func = 0xff;
	IPLDEVS[i].bus_number = 0xff;
}
rddevp = &IPLDEVS[0];

/*
 * patch up residual data if this is old ROS.  The only system that is
 * supported by this patch is the sandalfoot system.  All other systems
 * should have a Revision number of at least 1.  Refer to the section that
 * describes RESIDUAL.Revision in the AIX PowerPC Reference Platform
 * Architecture document for more information.
 */
if (rp->Version == 0 && rp->Revision == 0) {
	dbgtst(AMON_D1)printf("\nVersion 0, Revision 0 - residual data is being patched.\n");
	fix_resid(rp);
	setup_flag |= 0x1;
}

if (strncmp(rp->VitalProductData.PrintableModel,"IBM PPS Model 6015", 18) ==
	0) {
	/* This is an IBM Sandalfoot, so set the model field */
	strcpy(rp->VitalProductData.PrintableModel, "IBM PPS Model 6015 (4dE)");
	setup_flag |= 0x2;
}

/*
 * Now we need to check to see if there is a model
 * identifier. (see section 3.1.5.1 of AIX PowerPC Ref. Plat. Arch.)
 */

model = OTHER_MODEL;		/* initialize to a safe value */

if (c_ptr = strrchr(rp->VitalProductData.PrintableModel, '(')) {
	/* c_ptr points to the '(', so move it to the next character */
	c_ptr++;
	if ((*c_ptr >= '0' && *c_ptr <= '9') ||
		(*c_ptr >= 'a' && *c_ptr <= 'f')) {
		/* then it's a lowercase hex digit, so it's a model identifier */
		unsigned char count1;
		unsigned long model_id = 0;

		/* convert ascii to hex number */
		for (count1=0; count1 <= 1; count1++) {
			/* loop twice - model identifier is 2 lowercase hex chars */
			model_id <<= 4;
			if (*c_ptr > '9')
				/* convert lowercase ascii hexdigit */
				model_id += *c_ptr-0x57;
			else
				/* convert numeric ascii digit */
				model_id += *c_ptr-0x30;
			c_ptr++;
		}
		model = 0x08000000 | model_id;
	}

	/* now look at the system characteristics string, if it exists */
	while (*c_ptr != ')') {
		if (*c_ptr == 'A')
			model |= ANALYZE;
		else if (*c_ptr == 'D')
			model |= DIAG;
		else if (*c_ptr == 'E')
			model |= ENTRY_SRV;
		c_ptr++;
	}
}

ProcessorBusHz = rp->VitalProductData.ProcessorBusHz;

for (i=0;i<rp->ActualNumDevices;i++) {
	if ((rp->Devices[i].DeviceId.BaseType == BridgeController) &&
	    (rp->Devices[i].DeviceId.SubType == PCIBridge)) {
		/*
		 *  Found the device table entry for the PCI Bus.  Get
		 *  the Interface field which is 0 for Type 1, 1 for Type 2
		 */
		pci_type = rp->Devices[i].DeviceId.Interface;
	} else if ((rp->Devices[i].DeviceId.BaseType == SystemPeripheral) &&
	    (rp->Devices[i].DeviceId.SubType == NVRAM)) {
		c_ptr = &rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset];
		/*
		 *  Found the device table entry for NVRAM.  Now search for
		 *  any bus I/O descriptor in the heap in order to find the
		 *  address and data port numbers.
		 */
		while ((*c_ptr != 0x47) && (*c_ptr != 0x4B) &&
			(*c_ptr != 0x78)) {
				c_ptr++;
		}
		if (c_ptr[0] == 0x47)
			nvram_addr = (c_ptr[3] << 8) + c_ptr[2];
		else if (c_ptr[0] == 0x4B)
			nvram_addr = (c_ptr[2] << 8) + c_ptr[1];
		else break;		/* it is 0x78 */
		c_ptr += 2;

		while ((*c_ptr != 0x47) && (*c_ptr != 0x4B) &&
			(*c_ptr != 0x78)) {
				c_ptr++;
		}
		if (c_ptr[0] == 0x47)
			nvram_data = (c_ptr[3] << 8) + c_ptr[2];
		else if (c_ptr[0] == 0x4B)
			nvram_data = (c_ptr[2] << 8) + c_ptr[1];
		else break;		/* it is 0x78 */

	} else if ((rp->Devices[i].DeviceId.BaseType == MassStorageDevice) &&
	    (rp->Devices[i].DeviceId.SubType == SCSIController)) {
		/*
		 *  SCSI Controller - save the BusNumber, DevFuncNumber
		 */
		scsi_bus_number = rp->Devices[i].BusAccess.PCIAccess.BusNumber;
		scsi_dev_func=rp->Devices[i].BusAccess.PCIAccess.DevFuncNumber;
		rddevp->dev_func = rp->Devices[i].BusAccess.PCIAccess.DevFuncNumber;
		rddevp->bus_number = rp->Devices[i].BusAccess.PCIAccess.BusNumber;
		rddevp++;
		numipldevs++;
	}
}

/*
 * now check the hibernation disable flag.  If it is set, then we need to
 * go and turn off a couple of bits in the power management event descriptor.
 */
if (mode_control & 0x1)
	disable_hiber(rp);

processor = rp->Cpus[0].CpuType >> 16;	/* save processor type */

dbgtst(AMON_D1)printf("model = 0x%08X and processor = 0x%08X\n",model, processor);
dbgtst(AMON_D1)printf("pci_type 0x%02X, nvram_addr = 0x%04X, nvram_data = 0x%04X\n",
    pci_type, nvram_addr, nvram_data);
dbgtst(AMON_D1)printf("scsi_bus_number 0x%02X, scsi_dev_func = 0x%02X\n\n",
    scsi_bus_number, scsi_dev_func);

#ifdef DEBUG


if(!(mode_control & JUMPI_MODE))
	system_init();			/* Initialize system if not autojump */

			/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
			/*** BOOT MODE CONTROL ***************************/
			/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

if(mode_control & LODI_MODE)
	{
#endif /*DEBUG*/
	get_boot_image(&ipldata);
#ifdef DEBUG
	if(mode_control & JUMPI_MODE)
		{
#endif /*DEBUG*/
		iplcb_init(&ipldata);	   /* Setup iplcb  */
		if(status & STAT_VALID_BI)
			{
			pre_jump_setup(setup_flag);
			dojumpi( &ipldata, ipldata.load_addr ,
       				 ipldata.image_size, ipldata.jump_addr);
			}
		printf("No valid boot image has been loaded can't jump\n");
#ifdef DEBUG
		}
	}
			/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
			/*** END BOOT MODE CONTROL ***********************/
			/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

system_init();				/* If we're here we fell through
					   the load sequence and are
					   about to enter the monitor   */


					/* Monitor control loop	 */
while(1)				/* Do forever		   */
	{
	reset_msr_ee();			/* Disable external interrupts  */
	putchar(0x0f);		  /* Enable output with ^O	*/
	printf("\nAIXMON> ");
	strcpy(inbuf,(char *)esck());
	args = mkargs(inbuf,&argc);

	dbgtst(AMON_D2)
		{
		printf("From mkargs - argc = %d\n",argc);
		i=0;
		while(args[i]) printf("%s\n",args[i++]);
		}

	if(*args) do_command(argc,args);

	}
#endif /*DEBUG*/
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
	fix_resid -- convert sandalfoot old residual to new residual
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

int fix_resid(RESIDUAL *rp) {

unsigned char i, j;
unsigned long count1, count2;

	SF_res_patch.ResidualLength = sizeof(RESIDUAL);
	SF_res_patch.TotalMemory = rp->TotalMemory;
	SF_res_patch.GoodMemory = rp->GoodMemory;

	/* copy the original VPD */
	memcpy(&SF_res_patch.VitalProductData,
		&rp->VitalProductData, sizeof(VPD));

	/* fix up a couple of fields in the VPD that was copied */
	SF_res_patch.VitalProductData.EndianSwitchMethod = UsePort92;
	SF_res_patch.VitalProductData.SpreadIOMethod = UsePort850;

	/* fix the MaxNumCpus - remember, this is sandalfoot only, so it's 1 */
	SF_res_patch.MaxNumCpus = 1;

	/* save the original CpuType and fix the Number and State */
	SF_res_patch.Cpus[0].CpuType = rp->Cpus[0].CpuType;
	SF_res_patch.Cpus[0].CpuNumber = 00;
	SF_res_patch.Cpus[0].CpuState = CPU_GOOD;

	/* copy the original memory device data */
	SF_res_patch.ActualNumMemories = rp->ActualNumMemories;
	memcpy(SF_res_patch.Memories, rp->Memories, MAX_MEMS*sizeof(PPC_MEM));

	/* copy the original memory segment data */
	SF_res_patch.ActualNumMemSegs = rp->ActualNumMemSegs;
	memcpy(SF_res_patch.Segs, rp->Segs, MAX_MEM_SEGS*sizeof(MEM_MAP));

	/* update the memory segment base address data */
	for (i=0; i<SF_res_patch.ActualNumMemSegs; i++) {
		/* convert from kb to pages */
		SF_res_patch.Segs[i].BasePage /= 4096;
	}

	/*
	 * these next 3 pairs of assignment and memcpy statements are the
	 * base case for initialization of the Offset fields in the device
	 * array and the data in the pnpheap.
	 */

	/* first set the first offset, which is always zero */
	SF_res_patch.Devices[0].AllocatedOffset=0;

	/* now copy from the char array pointed to by [APC]_ptr to the heap */
	memcpy(&SF_res_patch.DevicePnPHeap[SF_res_patch.Devices[0].
		AllocatedOffset], offset_clues[0].A_ptr,
		offset_clues[0].A_count);

	/* subsequent offsets are based on the previous offset and size */
	SF_res_patch.Devices[0].PossibleOffset=
		SF_res_patch.Devices[0].AllocatedOffset +
		offset_clues[0].A_count;

	/* same as above, except for Possible instead of Allocated */
	memcpy(&SF_res_patch.DevicePnPHeap[SF_res_patch.Devices[0].
		PossibleOffset], offset_clues[1].P_ptr,
		offset_clues[1].P_count);

	SF_res_patch.Devices[0].CompatibleOffset=
		SF_res_patch.Devices[0].PossibleOffset +
		offset_clues[1].P_count;
	memcpy(&SF_res_patch.DevicePnPHeap[SF_res_patch.Devices[0].
		CompatibleOffset], offset_clues[2].C_ptr,
		offset_clues[2].C_count);

	for (i=1; i<SF_res_patch.ActualNumDevices; i++) {
	/*
	 * this loop, which consists of 3 pairs of assignment and memcpy
	 * statements, are for the remaining devices after the first.
	 */
		SF_res_patch.Devices[i].AllocatedOffset=
			SF_res_patch.Devices[i-1].CompatibleOffset +
			offset_clues[i-1].C_count;
		memcpy(&SF_res_patch.DevicePnPHeap[SF_res_patch.Devices[i].
			AllocatedOffset], offset_clues[i].A_ptr,
			offset_clues[i].A_count);
		SF_res_patch.Devices[i].PossibleOffset=
			SF_res_patch.Devices[i].AllocatedOffset +
			offset_clues[i].A_count;
		memcpy(&SF_res_patch.DevicePnPHeap[SF_res_patch.Devices[i].
			PossibleOffset], offset_clues[i].P_ptr,
			offset_clues[i].P_count);
		SF_res_patch.Devices[i].CompatibleOffset=
			SF_res_patch.Devices[i].PossibleOffset +
			offset_clues[i].P_count;
		memcpy(&SF_res_patch.DevicePnPHeap[SF_res_patch.Devices[i].
			CompatibleOffset], offset_clues[i].C_ptr,
			offset_clues[i].C_count);
	}

	/* now set the count1 variable to point to the end of the pnpheap */
	count1 = SF_res_patch.Devices[SF_res_patch.ActualNumDevices-1].
		CompatibleOffset + offset_clues[SF_res_patch.
		ActualNumDevices-1].C_count;

	/* search the device array for an L2 cache */
	for (i=0; i<rp->ActualNumDevices; i++) {
		if (rp->Devices[i].DeviceId.DevId == 0x07004D24) {

			memcpy(&SF_res_patch.Devices[SF_res_patch.ActualNumDevices],
				&(rp->Devices[i]), sizeof(PPC_DEVICE));
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].DeviceId.
				DevId = swap_endian(rp->Devices[i].DeviceId.DevId);

			/* copy the hardcoded L2 cache I/O port addresses into heap */
			memcpy(&SF_res_patch.DevicePnPHeap[count1],
				&L2_io_port_patch, sizeof(L2_io_port_patch));

			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
				AllocatedOffset = count1;
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
				PossibleOffset = SF_res_patch.Devices[SF_res_patch.
				ActualNumDevices].AllocatedOffset + rp->Devices[i].
				PossibleOffset - rp->Devices[i].AllocatedOffset +
				sizeof(L2_io_port_patch);
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
				CompatibleOffset = SF_res_patch.Devices[SF_res_patch.
				ActualNumDevices].PossibleOffset + rp->Devices[i].
				CompatibleOffset - rp->Devices[i].PossibleOffset;

			/* now add the size of the hardcoded L2 heap data to count1 */
			count1 += sizeof(L2_io_port_patch);

			/*
			 * this "for" loop copies the pnpheap Allocated and Possible
			 * data from the original to the replacement residual data
			 */
			for (count2 = rp->Devices[i].AllocatedOffset;
				count2 < rp->Devices[i].CompatibleOffset;
				count2++, count1++) {
				SF_res_patch.DevicePnPHeap[count1] =
					rp->DevicePnPHeap[count2];
			}
			while (rp->DevicePnPHeap[count2] != 0x78) {
				SF_res_patch.DevicePnPHeap[count1++] =
					rp->DevicePnPHeap[count2++];
			}
			SF_res_patch.DevicePnPHeap[count1++] = 0x78;

			SF_res_patch.ActualNumDevices++;
		}
	}

	/* search the device array for an ethernet network device */
	for (i=0; i<rp->ActualNumDevices; i++) {
		unsigned int port_table[]={
			0x300, 0x240, 0x280, 0x2C0, 0x320, 0x340, 0x360
		};

		if (rp->Devices[i].DeviceId.DevId == 0x01204D24) {
			memcpy(&SF_res_patch.Devices[SF_res_patch.ActualNumDevices],
				&(rp->Devices[i]), sizeof(PPC_DEVICE));
			/* set the Boot bit */
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].DeviceId.
				Flags |= Boot;
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].DeviceId.
				DevId = 0x244D0010;	/* replacement ethernet DevId */

			/*
			 * generate the serial number like this:
			 *	find I/O port packet
			 *	find the I/O port number in the port_number array
			 *	set the serial number to the array index
			 */
			for (count2 = rp->Devices[i].AllocatedOffset;
				count2 < rp->Devices[i].PossibleOffset; count2++) {
				/* search for the fixed I/O port packet */
				if (rp->DevicePnPHeap[count2] == 0x4B) {
					for (j=0; j <= 6; j++) {
						if ((rp->DevicePnPHeap[count2+1] +
							(rp->DevicePnPHeap[count2+2] << 8)) ==
							port_table[j])
								SF_res_patch.Devices[SF_res_patch.
									ActualNumDevices].DeviceId.SerialNum = j+1;
					}
					break;
				}
			}

			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
				AllocatedOffset = count1;
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
				PossibleOffset = SF_res_patch.Devices[SF_res_patch.
				ActualNumDevices].AllocatedOffset + rp->Devices[i].
				PossibleOffset - rp->Devices[i].AllocatedOffset;
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
				CompatibleOffset = SF_res_patch.Devices[SF_res_patch.
				ActualNumDevices].PossibleOffset + rp->Devices[i].
				CompatibleOffset - rp->Devices[i].PossibleOffset;
			/*
			 * this "for" loop copies the pnpheap Allocated and Possible
			 * data from the original to the replacement residual data
			 */
			for (count2 = rp->Devices[i].AllocatedOffset;
				count2 < rp->Devices[i].CompatibleOffset;
				count2++, count1++) {
				if (rp->DevicePnPHeap[count2] == 0x22) {
					int irq_mask;
					/*
					 * then this is a IRQ resource packet and it needs to be
					 * converted from a number to a bit map
					 */
					irq_mask = 1 << (rp->DevicePnPHeap[count2 + 1]);
					rp->DevicePnPHeap[count2 + 1] = irq_mask & 0xFF;
					rp->DevicePnPHeap[count2 + 2] = (irq_mask >> 8) & 0xFF;
				}
				SF_res_patch.DevicePnPHeap[count1] =
					rp->DevicePnPHeap[count2];
			}
			while (rp->DevicePnPHeap[count2] != 0x78) {
				SF_res_patch.DevicePnPHeap[count1++] =
					rp->DevicePnPHeap[count2++];
			}
			SF_res_patch.DevicePnPHeap[count1++] = 0x78;

			SF_res_patch.ActualNumDevices++;
		}
	}

	/* search the device array for a tokenring network device */
	for (i=0; i<rp->ActualNumDevices; i++) {
		unsigned int port_table[]={0xA20, 0xA24};

		if (rp->Devices[i].DeviceId.DevId == 0x2783D041) {
			unsigned int saved_port;

			memcpy(&SF_res_patch.Devices[SF_res_patch.ActualNumDevices],
				&(rp->Devices[i]), sizeof(PPC_DEVICE));
			/* set the Boot bit */
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].DeviceId.
				Flags |= Boot;
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].DeviceId.
				DevId = 0x41D080CC;	/* replacement tokenring DevId */

			/*
			 * generate the serial number like this:
			 *	find I/O port packet
			 *	find the I/O port number in the port_number array
			 *	set the serial number to the array index
			 */
			for (count2 = rp->Devices[i].AllocatedOffset;
				count2 < rp->Devices[i].PossibleOffset; count2++) {
				/* search for the fixed I/O port packet */
				if (rp->DevicePnPHeap[count2] == 0x4B) {
					for (j=0; j <= 1; j++) {
						saved_port = rp->DevicePnPHeap[count2+1] +
							(rp->DevicePnPHeap[count2+2] << 8);
						if (saved_port == port_table[j])
								SF_res_patch.Devices[SF_res_patch.
									ActualNumDevices].DeviceId.SerialNum = j+1;
					}
					break;
				}
			}

			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
				AllocatedOffset = count1;
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
				PossibleOffset = SF_res_patch.Devices[SF_res_patch.
				ActualNumDevices].AllocatedOffset + rp->Devices[i].
				PossibleOffset - rp->Devices[i].AllocatedOffset;
			SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
				CompatibleOffset = SF_res_patch.Devices[SF_res_patch.
				ActualNumDevices].PossibleOffset + rp->Devices[i].
				CompatibleOffset - rp->Devices[i].PossibleOffset;
			/*
			 * this "for" loop copies the pnpheap Allocated and Possible
			 * data from the original to the replacement residual data
			 */
			for (count2 = rp->Devices[i].AllocatedOffset;
				count2 < rp->Devices[i].CompatibleOffset;
				count2++, count1++) {

				if (rp->DevicePnPHeap[count2] == 0x4B) {
					/*
					 * then this is a fixed I/O packet, and it will be
					 * converted from a 10 bit descriptor packet to a
					 * 16 bit descriptor packet.  This fixes the packet
					 * that is incorrectly built by firmware in sandalfoot
					 *
					 * NOTE: this assumes that the fixed I/O packets will
					 * be found in the Alloc or Poss areas of the pnp heap
					 */
					SF_res_patch.DevicePnPHeap[count1++] = 0x47;
					SF_res_patch.DevicePnPHeap[count1++] = 0x01;
					SF_res_patch.DevicePnPHeap[count1++] =
						rp->DevicePnPHeap[count2 + 1];
					SF_res_patch.DevicePnPHeap[count1++] =
						rp->DevicePnPHeap[count2 + 2];
					SF_res_patch.DevicePnPHeap[count1++] =
						rp->DevicePnPHeap[count2 + 1];
					SF_res_patch.DevicePnPHeap[count1++] =
						rp->DevicePnPHeap[count2 + 2];
					SF_res_patch.DevicePnPHeap[count1++] = 0x01;
					SF_res_patch.DevicePnPHeap[count1] =
						rp->DevicePnPHeap[count2 + 3];

					count2 += 3;
					/*
					 * now count1 is 8 greater than it was before this
					 * conditional and count is 4 greater.  Now patch up
					 * the offsets and go back to the top of the loop.
					 * The difference between the packet sizes is 4 bytes.
					 */
					if (count2 < rp->Devices[i].PossibleOffset) {
						/* the replacement packet is in the Allocated area */
						SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
							PossibleOffset += 4;
					}
					SF_res_patch.Devices[SF_res_patch.ActualNumDevices].
					CompatibleOffset += 4;
					continue;
				}
				if (rp->DevicePnPHeap[count2] == 0x81  &&
					rp->DevicePnPHeap[count2 + 1] == 0x09 &&
					rp->DevicePnPHeap[count2 + 3] == 0x21) {

					int smem_size;
					/*
					 * then this is a memory range packet, and it also
					 * needs fixing.
					 * we do this by modifying the original data, which
					 * gets copied to patched data at end of this loop.
					 * this is easier than creating the entire packet.
					 */
					rp->DevicePnPHeap[count2 + 5] = 0x0D;
					rp->DevicePnPHeap[count2 + 7] = 0x0D;
					/*
					 * read shared memory setting using IO port number
					 */
					smem_size = get_tr_smem(in8(saved_port));
					rp->DevicePnPHeap[count2 + 10] = smem_size & 0xFF;
					rp->DevicePnPHeap[count2 + 11] = (smem_size >> 8) & 0xFF;
				}
				if (rp->DevicePnPHeap[count2] == 0x22) {
					int irq_mask;
					/*
					 * then this is a IRQ resource packet and it also
					 * needs to be fixed.  Use the info at the port address.
					 */
					irq_mask = get_irq_mask(in8(saved_port));
					rp->DevicePnPHeap[count2 + 1] = irq_mask & 0xFF;
					rp->DevicePnPHeap[count2 + 2] = (irq_mask >> 8) & 0xFF;
				}
				SF_res_patch.DevicePnPHeap[count1] =
					rp->DevicePnPHeap[count2];
			}
			while (rp->DevicePnPHeap[count2] != 0x78) {
				SF_res_patch.DevicePnPHeap[count1++] =
					rp->DevicePnPHeap[count2++];
			}
			SF_res_patch.DevicePnPHeap[count1++] = 0x78;

			SF_res_patch.ActualNumDevices++;
		}
	}

	/* now copy the patched data over the original data */
	memcpy(rp, &SF_res_patch, sizeof(RESIDUAL));
	return(0);
}

/******************************************************************************/
/*
 * This function finds the power management device in the residual device
 * array, then disables hibernation by turning off the "hibernation_support"
 * and "suspend_support" bits in the fifth byte of the power management
 * event descriptor.
 */
int
disable_hiber(RESIDUAL *rp)
{
int i;
unsigned char *c_ptr;

	for (i=0; i<rp->ActualNumDevices; i++) {
		if ((rp->Devices[i].DeviceId.BaseType == SystemPeripheral) &&
		    (rp->Devices[i].DeviceId.SubType == PowerManagement)) {

			c_ptr = &rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset];
			while (((*c_ptr != 0x84) || (*(c_ptr + 3) != 0x8)) && (c_ptr <
				&rp->DevicePnPHeap[rp->Devices[i + 1].AllocatedOffset])) {
				/*
				 * search for power management descriptor packet until we
				 * find a tag with 0x84 followed 3 bytes later with a byte
				 * indicating powman descriptor, or until end of this
				 * device's pnp heap area
				 */
				c_ptr++;
			}
			if ((*c_ptr == 0x84) && (*(c_ptr + 3) == 0x8)) {
				/*
				 * we have the powman descriptor; now mask away the last 2
				 * bits of the fifth byte, which is the low order event bits
				 */
				*(c_ptr + 4) &= 0xFC;
				dbgtst(AMON_D1)printf(
					"disabling hibernation in power management\n");
			}
		}
	}
	return(0);
}

/******************************************************************************/
/*
 * read the shared memory dip switch settings and return the indicated size.
 * This code was derived from ROS source code and the ISA token ring device
 * driver code.
 */
int
get_tr_smem(unsigned int setup)
{
	unsigned char switch_byte;
	unsigned int ret_val;

	/*
	 * first read the byte that indicates the shared memory switch setting
	 * Here's how that offset is calculated:
	 *	0xC0000000 is the start of bus memory
	 *	0x00080000 is the offset to the token ring adapter IO memory map
	 *	0x00001E00 is the offset to the attachment control area
	 *	0x00000001 is the offset to the relevant byte
	 */
	switch_byte = *((unsigned char *)((setup & 0xFC) << 11) + 0xC0081E01);
	/*
	 * switch_byte should now equal 0, 4, 8, or 0xC
	 *
	 * convert it and return 0x20, 0x40, 0x80, or 0x100
	 */
	return(1 << (((switch_byte & 0x0C) >> 2) + 5));
}

/******************************************************************************/
/*
 * read the interrupt level dip switch settings and return the interrupt mask.
 */
int
get_irq_mask(unsigned int setup)
{
	switch (setup & 0x3) {
		case 0:
			return(0x0200);		/* irq level 9 */
		case 1:
			return(0x0008);		/* irq level 3 */
		case 2:
			return(0x0040);		/* irq level 6 */
		case 3:
			return(0x0080);		/* irq level 7 */
		default:
			/* set it to 9 */
			return(0x0200);
	}
}

#ifdef DEBUG
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
	do_command(argc,args) -- Process a monitor command
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

void do_command(int argc,char ** args)
{
int     i;
int     index = 0;			      /* Command index found  */

dbgmsg(AMON_D1,"- do_command()");
dbgtst(AMON_D2) printf("Looking for command name >%s<\n",args[0]);

i=0;
while(command[i].cmd[0] != 0)		   /* Check for command name */
	{
	if(strcmp(args[0],command[i].cmd) == 0)
		{
		(*(command[i].cmd_func))(argc,args);
		dbgtst(AMON_D2) printf("Command >%s< complete\n",args[0]);
		return;
		}
	i++;
	}
					/* Didn't find it get help      */
dbgmsg(AMON_D2,"command not found in command structure");
gencmd(argc,args);
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
	system_init() -- Initialize system for monitor usage
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

system_init()
{
uint		inval;

					/* Initialize the Environment	*/
static initialized = FALSE;

if(initialized)
	return(0);

initialized = TRUE;

if (pci_type == 0) {
	/*
	 * Type 1 PCI config, call siosetup()
	 */
	siosetup();
} else {
	/*
	 * Must be PCI 2.0 Configuration
	 */
	out32le(0x80000cf8, 0x80005840);  /* set up address reg */
	out32(0x80000cfc, 0x23000400);  /* value0 */

	out32le(0x80000cf8, 0x80005844);  /* set up address reg */
	out32(0x80000cfc, 0x00100f00);  /* value4 */

	out32le(0x80000cf8, 0x80005848);  /* set up address reg */
	out32(0x80000cfc, 0x0100100f);  /* value8 */

	out32le(0x80000cf8, 0x8000584c);  /* set up address reg */
	inval = in32(0x80000cfc);
	inval = (0x561007ff & 0xFFF8FFFF) | (inval & 0x00070000); /* valuec */
	out32(0x80000cfc, inval);       /* valuec */
}

init_uart();
diskette_init(0);			/* Initialize diskette drive A: */
scsiinit(SINIT_CHECK);			/* Initialize SCSI subsystem    */

is_drive_valid(Selected_Disk,FALSE);	/* Setup monitor boot device values */

}
#endif /*DEBUG*/
