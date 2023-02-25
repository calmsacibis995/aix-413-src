static char sccsid[] = "@(#)65	1.7  src/rspc/usr/lib/boot/softros/aixmon/res_debug.c, rspc_softros, rspc41J, 9519A_all 5/4/95 11:58:56";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: decode_dev
 *		dump_residual
 *		nvram_dump
 *		qmore
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* res_debug.c -- Residual data and NVRAM data dump	*/

#ifdef DEBUG

#include	"aixmon.h"
#include	"nvram.h"
#include	<sys/residual.h>

#define MAX_HEAP 2*MAX_DEVICES*AVE_PNP_SIZE

char *decode_dev(uint);


			/**** Some debug stuff, here's a lookup  table   ****/
			/**** for the PNP device types                   ****/
char *pnp_types[][2] = {
	{ "IBM0005", "[IBM0005] Power Management"},
	{ "IBM0007", "[IBM0007] L2 Cache"},
	{ "IBM0008", "[IBM0008] NVRAM"},
	{ "IBM0009", "[IBM0009] CMOS"},
	{ "IBM000A", "[IBM000A] Memory controller"},
	{ "IBM000B", "[IBM000B] Operator's panel"},
	{ "IBM000D", "[IBM000D] PowerPC Extended Interrupt Controller"},
	{ "IBM000E", "[IBM000E] Crystal CS4231 Audio Device"},
	{ "IBM000F", "[IBM000F] NCR 810 SCSI Controller"},
	{ "IBM0010", "[IBM0010] IBM Ethernet used by Power PC"},
	{ "IBM0011", "[IBM0011] IBM Service Processor"},
	{ "IBM0015", "[IBM0015] ASCII V7310 Video Capture Device"},
	{ "IBM0016", "[IBM0016] AMD 79C970 (PCI Ethernet)"},
	{ "IBM0017", "[IBM0017] Crystal CS4232 Audio Device"},
	{ "IBM001B", "[IBM001B] NCR 825 SCSI Controller"},
	{ "IBM001C", "[IBM001C] EPP Parallel Port"},
	{ "IBM001D", "[IBM001D] YMF 289B chip (Yamaha)"},
	{ "IBM2001", "[IBM2001] IBM Ethernet EISA adapter"},
	{ "PNP0000", "[PNP0000] AT Interrupt Controller"},
	{ "PNP0001", "[PNP0001] EISA Interrupt Controller"},
	{ "PNP0002", "[PNP0002] MCA Interrupt Controller"},
	{ "PNP0003", "[PNP0003] APIC"},
	{ "PNP0100", "[PNP0100] AT Timer"},
	{ "PNP0101", "[PNP0101] EISA Timer"},
	{ "PNP0102", "[PNP0102] MCA Timer"},
	{ "PNP0200", "[PNP0200] AT DMA Controller"},
	{ "PNP0201", "[PNP0201] EISA DMA Controller"},
	{ "PNP0202", "[PNP0202] MCA DMA Controller"},
	{ "PNP0300", "[PNP0300] IBM PC/XT Keyboard Controller (83 key, no mouse)"},
	{ "PNP0301", "[PNP0301] Olivetti ICO (102 key)"},
	{ "PNP0302", "[PNP0302] IBM PC/AT Keyboard Controller (84 key)"},
	{ "PNP0303", "[PNP0303] IBM Enhanced (101/102 key, PS/2 mouse)"},
	{ "PNP0304", "[PNP0304] Nokia 1050 Keyboard Controller"},
	{ "PNP0305", "[PNP0305] Nokia 9140 Keyboard Controller"},
	{ "PNP0306", "[PNP0306] Standard Japanese Keyboard Controller"},
	{ "PNP0307", "[PNP0307] Microsoft Windows (R) Keyboard Controller"},
	{ "PNP0400", "[PNP0400] Standard LPT Parallel Port"},
	{ "PNP0401", "[PNP0401] ECP Parallel Port"},
	{ "PNP0500", "[PNP0500] Standard PC Serial port"},
	{ "PNP0501", "[PNP0501] 16550A Compatible Serial port"},
	{ "PNP0600", "[PNP0600] Generic ESDI/IDE/ATA Compatible Hard Disk Controller"},
	{ "PNP0601", "[PNP0601] Plus Hardcard II"},
	{ "PNP0602", "[PNP0602] Plus Hardcard IIXL/EZ"},
	{ "PNP0700", "[PNP0700] PC Standard Floppy Disk Controller"},
	{ "PNP0800", "[PNP0800] AT Style Speaker Sound"},
	{ "PNP0900", "[PNP0900] VGA Compatible"},
	{ "PNP0901", "[PNP0901] Video Seven VGA"},
	{ "PNP0902", "[PNP0902] 8514/A Compatible"},
	{ "PNP0903", "[PNP0903] Trident VGA"},
	{ "PNP0904", "[PNP0904] Cirrus Logic Laptop VGA"},
	{ "PNP0905", "[PNP0905] Cirrus Logic VGA"},
	{ "PNP0906", "[PNP0906] Tseng ET4000 or ET4000/W32"},
	{ "PNP0907", "[PNP0907] Western Digital VGA"},
	{ "PNP0908", "[PNP0908] Western Digital Laptop VGA"},
	{ "PNP0909", "[PNP0909] S3"},
	{ "PNP090A", "[PNP090A] ATI Ultra Pro/Plus (Mach 32)"},
	{ "PNP090B", "[PNP090B] ATI Ultra (Mach 8)"},
	{ "PNP090C", "[PNP090C] XGA Compatible"},
	{ "PNP090D", "[PNP090D] ATI VGA Wonder"},
	{ "PNP090E", "[PNP090E] Weitek P9000 Graphics Adapter"},
	{ "PNP090F", "[PNP090F] Oak Technology VGA"},
	{ "PNP0A00", "[PNP0A00] ISA Bus"},
	{ "PNP0A01", "[PNP0A01] EISA Bus"},
	{ "PNP0A02", "[PNP0A02] MCA Bus"},
	{ "PNP0A03", "[PNP0A03] PCI Bus"},
	{ "PNP0A04", "[PNP0A04] VESA/VL Bus"},
	{ "PNP0B00", "[PNP0B00] AT RTC"},
	{ "PNP0C00", "[PNP0C00] PNP BIOS (only created by root enumerator)"},
	{ "PNP0C01", "[PNP0C01] System Board Memory Device"},
	{ "PNP0C02", "[PNP0C02] Math Coprocessor"},
	{ "PNP0C03", "[PNP0C03] PNP BIOS Event Notification Interrupt"},
	{ "PNP0E00", "[PNP0E00] Intel 82365 Compatible PCMCIA Controller"},
	{ "PNP0F00", "[PNP0F00] Microsoft Bus Mouse"},
	{ "PNP0F01", "[PNP0F01] Microsoft Serial Mouse"},
	{ "PNP0F02", "[PNP0F02] Microsoft Inport Mouse"},
	{ "PNP0F03", "[PNP0F03] Microsoft PS/2 Mouse"},
	{ "PNP0F04", "[PNP0F04] Mousesystems Mouse"},
	{ "PNP0F05", "[PNP0F05] Mousesystems 3 Button Mouse - COM2"},
	{ "PNP0F06", "[PNP0F06] Genius Mouse - COM1"},
	{ "PNP0F07", "[PNP0F07] Genius Mouse - COM2"},
	{ "PNP0F08", "[PNP0F08] Logitech Serial Mouse"},
	{ "PNP0F09", "[PNP0F09] Microsoft Ballpoint Serial Mouse"},
	{ "PNP0F0A", "[PNP0F0A] Microsoft PNP Mouse"},
	{ "PNP0F0B", "[PNP0F0B] Microsoft PNP Ballpoint Mouse"},
	{ "PNP80C9", "[PNP80C9] IBM Token Ring"},
	{ "PNP80CA", "[PNP80CA] IBM Token Ring II"},
	{ "PNP80CB", "[PNP80CB] IBM Token Ring II/Short"},
	{ "PNP80CC", "[PNP80CC] IBM Token Ring 4/16Mbs"},
	{ "PNP8327", "[PNP8327] IBM Token Ring (All types)"},
	{ "PNP9000", "[PNP9000] Specific IDs TBD"},
	{ "PNPA000", "[PNPA000] Adaptec 154x Compatible SCSI Controller"},
	{ "PNPA001", "[PNPA001] Adaptec 174x Compatible SCSI Controller"},
	{ "PNPA002", "[PNPA002] Future Domain 16-700 Compatible SCSI Controller"},
	{ "PNPA003", "[PNPA003] Panasonic Porprietary CDROM Adapter (SBPro/SB16)"},
	{ "PNPA00F", "[PNPA00F] NCR 810 SCSI Controller"},
	{ "PNPB000", "[PNPB000] Sound Blaster Compatible Sound Device"},
	{ "PNPB001", "[PNPB001] Microsoft Windows Sound System Compatible Sound Device"},
};

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
	dump_residual() - Output residual data for debug
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

void dump_residual(int num, char** args)
{

RESIDUAL	*rp;			/* Pointer to residual structure */
int		i;

rp = (RESIDUAL *) residual_save;

printf("\nResidual ID:\n");
printf("   Length = 0x%x\n",rp->ResidualLength);
printf("   Version = %d\n",rp->Version);
printf("   Revision = %d\n",rp->Revision);
printf("   Address = 0x%x\n",rp);

printf("\nResidual VPD:\n");
printf("   Model  = %s\n",rp->VitalProductData.PrintableModel);
printf("   Serial = %s\n",rp->VitalProductData.Serial);
printf("   FW Support    = 0x%x\n",rp->VitalProductData.FirmwareSupports);
printf("   NVRAM size   = %d\n",rp->VitalProductData.NvramSize);
printf("   SIMM slots   = %d\n",rp->VitalProductData.NumSIMMSlots);
printf("   CPU Hz       = %d\n",rp->VitalProductData.ProcessorHz);
printf("   CPU bus Hz   = %d\n",rp->VitalProductData.ProcessorBusHz);
printf("   Time base div = 0x%x\n",rp->VitalProductData.TimeBaseDivisor);

if(!qmore()) return;

printf("   Word width = %d\n",rp->VitalProductData.WordWidth);
printf("   Page size  = %d\n",rp->VitalProductData.PageSize);
printf("   Block size = %d\n",rp->VitalProductData.CoherenceBlockSize);
printf("   Cache size   = %d\n",rp->VitalProductData.CacheSize);
printf("   Cache attrib = %d\n",rp->VitalProductData.CacheAttrib);
printf("   Cache assoc  = %d\n",rp->VitalProductData.CacheAssoc);
printf("   Cache line   = %d\n",rp->VitalProductData.CacheLineSize);
printf("   I-Cache size   = %d\n",rp->VitalProductData.I_CacheSize);
printf("   I-Cache assoc  = %d\n",rp->VitalProductData.I_CacheAssoc);
printf("   I-Cache line   = %d\n",rp->VitalProductData.I_CacheLineSize);
printf("   D-Cache size   = %d\n",rp->VitalProductData.D_CacheSize);
printf("   D-Cache assoc  = %d\n",rp->VitalProductData.D_CacheAssoc);
printf("   D-Cache line   = %d\n",rp->VitalProductData.D_CacheLineSize);
printf("   TLB size   = %d\n",rp->VitalProductData.TLBSize);
printf("   TLB attrib = %d\n",rp->VitalProductData.TLBAttrib);
printf("   TLB assoc  = %d\n",rp->VitalProductData.TLBAssoc);
printf("   I_TLB size   = %d\n",rp->VitalProductData.I_TLBSize);
printf("   I_TLB assoc  = %d\n",rp->VitalProductData.I_TLBAssoc);
printf("   D_TLB size   = %d\n",rp->VitalProductData.D_TLBSize);
printf("   D_TLB assoc  = %d\n",rp->VitalProductData.D_TLBAssoc);
printf("   ExtVPD ptr = 0x%x\n",NULL);


if(!qmore()) return;

printf("\nResidual CPU:\n");
printf("   Num CPUs = %d\n",rp->ActualNumCpus);

	/* search the dev array for L2 */
for (i=0;i<rp->ActualNumDevices;i++) {
	if (rp->Devices[i].DeviceId.DevId == encode_dev(PNPL2)) {
		unsigned char *c_ptr;

		/* then a DevId for L2 was found */
		c_ptr = &rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset];
		for (i=0; c_ptr[i] != 0x78; i++) {
			if (c_ptr[i] == 0x84 && c_ptr[i+3] == 2) {
				/* this is L2 cache info */
				printf("            L2 size    = %d\n",
					c_ptr[i+4] + (c_ptr[i+5] << 8) +
					(c_ptr[i+6] << 16) +
					(c_ptr[i+7] << 24));
				printf("            L2 assoc   = %d\n",
					c_ptr[i+8] + (c_ptr[i+9] << 8));
				break;
			}
		}
		break;
	}
}

for (i=0; i<rp->ActualNumCpus; i++)
	{
	printf("     CPU[%d] CPU Type   = 0x%8x\n",i,rp->Cpus[i].CpuType);
	} /* endfor */


if(!qmore()) return;

printf("\nResidual Memory:\n");
printf("   Total Memory = 0x%x\n",rp->TotalMemory);
printf("   Good Memory  = 0x%x\n",rp->GoodMemory);

for (i=0; i<rp->ActualNumMemSegs; i++)
	{
	if (i && (i % 5 == 0))
		{
		if(!qmore()) return;
      		}

	printf("   Mem seg[%d] =\n",i);
	if (rp->Segs[i].Usage & ResumeBlock)
		printf("     Mem seg is ResumeBlock\n");
	if (rp->Segs[i].Usage & SystemROM)
		printf("     Mem seg is SystemROM\n");
	if (rp->Segs[i].Usage & UnPopSystemROM)
		printf("     Mem seg is UnPopSystemROM\n");
	if (rp->Segs[i].Usage & IOMemory)
		printf("     Mem seg is IOMemory\n");
	if (rp->Segs[i].Usage & SystemIO)
		{
		printf("     Mem seg is SystemIO\n");
		if (rp->Segs[i].Usage & SystemRegs)
			printf("     Mem seg is SystemRegs\n");
		if (rp->Segs[i].Usage & PCIAddr)
			printf("     Mem seg is PCIAddr\n");
		if (rp->Segs[i].Usage & PCIConfig)
			printf("     Mem seg is PCIConfig\n");
		if (rp->Segs[i].Usage & ISAAddr)
			printf("     Mem seg is ISAAddr\n");
		} /* endif */

	if (rp->Segs[i].Usage & Unpopulated)
		printf("     Mem seg is Unpopulated\n");
	if (rp->Segs[i].Usage & Free)
		printf("     Mem seg is Free\n");
	if (rp->Segs[i].Usage & BootImage)
		printf("     Mem seg is Boot Image\n");
	if (rp->Segs[i].Usage & FirmwareCode)
		printf("     Mem seg is Firm Code\n");
	if (rp->Segs[i].Usage & FirmwareHeap)
		printf("     Mem seg is Firm Heap\n");
	if (rp->Segs[i].Usage & FirmwareStack)
		printf("     Mem seg is Firm Stack\n");
	printf("     Mem seg base page  = 0x%x\n",rp->Segs[i].BasePage);
	printf("     Mem seg page count = 0x%x\n",rp->Segs[i].PageCount);
	} /* endfor */


if(!qmore()) return;

printf("   Total SIMMs = %d\n",rp->ActualNumMemories);
for (i=0; i<rp->VitalProductData.NumSIMMSlots; i++)
	{
	printf("     SIMM[%d] size = %d\n",i,rp->Memories[i].SIMMSize);
	} /* endfor */


if(!qmore()) return;

printf("\nResidual Devices:\n");
printf("   Total Devices = %d\n",rp->ActualNumDevices);
for (i=0; i<rp->ActualNumDevices; i++)
	{
	printf("   Device [%d] = %s\n",i,decode_dev(rp->Devices[i].DeviceId.DevId));
	printf("     Dev ID = 0x%8x\n",rp->Devices[i].DeviceId.DevId);
	printf("     Serial = 0x%8x\n",rp->Devices[i].DeviceId.SerialNum);

	if (rp->Devices[i].DeviceId.BusId & ISADEVICE)
		printf("     Device is ISA\n");
	if (rp->Devices[i].DeviceId.BusId & EISADEVICE)
		printf("     Device is EISA\n");
	if (rp->Devices[i].DeviceId.BusId & PCIDEVICE)
		printf("     Device is PCI\n");
	if (rp->Devices[i].DeviceId.BusId & PCMCIADEVICE)
		printf("     Device is PCMCIA\n");
	if (rp->Devices[i].DeviceId.BusId & PNPISADEVICE)
		printf("     Device is PNPISA\n");
	if (rp->Devices[i].DeviceId.BusId & MCADEVICE)
		printf("     Device is MCA\n");

	if (rp->Devices[i].DeviceId.Flags & Failed)
		printf("     Device has Failed\n");
	if (rp->Devices[i].DeviceId.Flags & Static)
		printf("     Device is Static\n");
	if (rp->Devices[i].DeviceId.Flags & Dock)
		printf("     Device is Docking\n");
	if (rp->Devices[i].DeviceId.Flags & Boot)
		printf("     Device is IPLable\n");
	if (rp->Devices[i].DeviceId.Flags & Configurable)
		printf("     Device is Configurable\n");
	if (rp->Devices[i].DeviceId.Flags & Disableable)
		printf("     Device is Disableable\n");
	if (rp->Devices[i].DeviceId.Flags & PowerManaged)
		printf("     Device is PowerManaged\n");
	if (rp->Devices[i].DeviceId.Flags & ReadOnly)
		printf("     Device is ReadOnly\n");
	if (rp->Devices[i].DeviceId.Flags & Removable)
		printf("     Device is Removable\n");
	if (rp->Devices[i].DeviceId.Flags & ConsoleIn)
		printf("     Device is ConsoleIn\n");
	if (rp->Devices[i].DeviceId.Flags & ConsoleOut)
		printf("     Device is ConsoleOut\n");
	if (rp->Devices[i].DeviceId.Flags & Input)
		printf("     Device is Input\n");
	if (rp->Devices[i].DeviceId.Flags & Output)
		printf("     Device is Output\n");

	printf("     Base  Type = %d\n",rp->Devices[i].DeviceId.BaseType);
	printf("     Sub   Type = %d\n",rp->Devices[i].DeviceId.SubType);
	printf("     Inter Type = %d\n",rp->Devices[i].DeviceId.Interface);
	printf("     Spare Type = %d\n",rp->Devices[i].DeviceId.Spare);
	printf("     Bus Access[0] = %d\n",rp->Devices[i].BusAccess.PnPAccess.CSN);
	printf("     Bus Access[1] = %d\n",rp->Devices[i].BusAccess.PnPAccess.LogicalDevNumber);
	printf("     Bus Access[2] = %d\n",rp->Devices[i].BusAccess.PnPAccess.ReadDataPort);
	printf("     Allocated  = 0x%X\n",rp->Devices[i].AllocatedOffset);
	printf("     Possible   = 0x%X\n",rp->Devices[i].PossibleOffset);
	printf("     Compatible = 0x%X\n",rp->Devices[i].CompatibleOffset);


	if(!qmore()) return;

	} /* endfor */

printf("\nResidual Device Heap:\n");
for (i=0; i<MAX_HEAP; i++)
	{
	if (i % 10 == 0) printf("\n");
	if (i && (i % 200 == 0))
		{
		if(!qmore()) return;
		}
	printf("  %2x",rp->DevicePnPHeap[i]);
	}
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
	nvram_dump() - Format and dump the contents of NVRAM
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

void nvram_dump(int num, char** args)
{
NVRAM_MAP	nv;
char		*nvbuf;			/* Pointer to nvram buffer	  */
char		*envbuf;		/* Pointer to environment string  */
char		*envendbuf;		/* Pointer to end of env string   */
char		*env_name, *env_value;	/* Environment var pair		  */
int		i;
int		state;
int		lines;
					/* Get the NVRAM info into buffer */
nvbuf = (char *) &nv;
for(i=0;i<NVSIZE;i++)
	nvbuf[i] = read_nvram(i);

					/* Dump the Header Information	*/

printf("NVRAM contents - Header Info\n");
printf("NVRAM Size (in K) ........ %d\n",nv.Header.Size);
printf("NVRAM Version ............ %d\n",nv.Header.Version);
printf("NVRAM Revision ........... %d\n",nv.Header.Revision);
printf("NVRAM Crc1 ............... %8.8X\n",nv.Header.Crc1);
printf("NVRAM Crc2 ............... %8.8X\n",nv.Header.Crc2);
printf("NVRAM LastOS ............. %d\n",nv.Header.LastOS);
printf("NVRAM Endian ............. %c\n",nv.Header.Endian);
printf("NVRAM OSAreaUsage ........ %X\n",nv.Header.OSAreaUsage);
printf("NVRAM PMMode ............. %d\n",nv.Header.PMMode);
printf("NVRAM GEAddress .......... %8.8X\n",nv.Header.GEAddress);
printf("NVRAM GELength ........... %8.8X\n",nv.Header.GELength);
printf("NVRAM GELastWriteDT[0] ... %8.8X\n",nv.Header.GELastWriteDT[0]);
printf("NVRAM GELastWriteDT[1] ... %8.8X\n",nv.Header.GELastWriteDT[1]);
printf("NVRAM ConfigAddress ...... %8.8X\n",nv.Header.ConfigAddress);
printf("NVRAM ConfigLength ....... %8.8X\n",nv.Header.ConfigLength);
printf("NVRAM ConfigLastWriteDT[0] %8.8X\n",nv.Header.ConfigLastWriteDT[0]);
printf("NVRAM ConfigLastWriteDT[1] %8.8X\n",nv.Header.ConfigLastWriteDT[1]);
printf("NVRAM OSAreaAddress ...... %8.8X\n",nv.Header.OSAreaAddress);
printf("NVRAM OSAreaLength ....... %8.8X\n",nv.Header.OSAreaLength);
printf("NVRAM OSAreaLastWriteDT[0] %8.8X\n",nv.Header.OSAreaLastWriteDT[0]);
printf("NVRAM OSAreaLastWriteDT[1] %8.8X\n",nv.Header.OSAreaLastWriteDT[1]);


if(!qmore()) return;

envbuf = nvbuf + (int) nv.Header.GEAddress;
envendbuf = envbuf+nv.Header.GELength;
state = 0;

printf("Dumping Global Environment Space\n\n");
while(state < 4)
	{
	switch(state)
		{
		case	0:		/* Find the name	*/
		while(envbuf[0] == ' ') envbuf++;
		env_name = envbuf;
		while(envbuf[0] != ' ' && envbuf[0] != '=' && envbuf[0] != 0) envbuf++;
		switch(envbuf[0])
			{
			case	' ':
			envbuf[0] = 0;
			envbuf++;
			state = 1;
			break;

			case	'=':
			envbuf[0] = 0;
			envbuf++;
			while(envbuf[0] == ' ' && envbuf[0] != 0) envbuf++;
			env_value = envbuf;
			state = 2;
			break;

			case	0:
			state = 4;
			break;
			}
		break;

		case	1:		/* Find start of variable */
		while(envbuf[0] != '=' && envbuf[0] != 0) envbuf++;
		switch(envbuf[0])
			{
			case	'=':
			while(envbuf[0] == ' ' && envbuf[0] != 0) envbuf++;
			env_value = envbuf;
			state = 2;
			break;

			case	0:
			state = 4;
			break;
			}
		break;

		case	2:		/* Move pointer to the end */
		while(envbuf[0]) envbuf++;
		state = 3;
		break;

		case	3:		/* Process output and setup for next one */
		while(envbuf < envendbuf && envbuf[0] == 0) envbuf++;

		printf("Name = %s  <--> Value = %s\n",env_name, env_value);
		lines++;
		if(!(lines%18)) qmore();

		if(envbuf < envendbuf)
			state = 0;
		else
			state = 4;
		break;

		}
	}
}





/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
	decode_dev() - Decode the devid into a string
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

char *decode_dev(uint devid)
{
					/* Convert back to string rep first */
int     i,j=3;
char    code[8];

code[0] = ((devid >> 26) & 0x0000001F)+'@';
code[1] = ((devid >> 21) & 0x0000001F)+'@';
code[2] = ((devid >> 16) & 0x0000001F)+'@';

for (i=3; i<7; i++)
	{
	if(((devid >> (j*4)) & 0x0000000F) > 9)
		code[i] = (((devid >> (j*4)) & 0x0000000F)-9)+'@';
	else
		code[i] = ((devid >> (j*4)) & 0x0000000F)+'0';
	j--;
	}
code[7] = 0;

i = 0;

while(pnp_types[i][0])
	{
	if(strcmp(pnp_types[i][0],code) == 0)
		return(pnp_types[i][1]);
	i++;
	}

return((char *) code);
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
	qmore() - Wait for more or quit the listing
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

qmore()
{
char	ch;

printf("  ...More... ");
ch = getchar();
printf("\n");

if(ch == 'q' || ch == 'Q')
	return(0);
else
	return(1);
}

#endif /*DEBUG*/
