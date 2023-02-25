static char sccsid[] = "@(#)01  1.3  src/rspc/kernext/pm/pmmi/pmresidual.c, pwrsysx, rspc41J 6/6/95 23:52:59";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: ChangeSystemState, PresetTriggers,  
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/errno.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>
#include <sys/rtc.h>
#include <sys/time.h>
#include <sys/residual.h>
#include <sys/pnp.h>
#include "pmsptpnp.h"

#include <sys/pm.h>
#include "../../pmmi/pmmi.h"


/*-------------------------*/
/*    function proto type  */
/*-------------------------*/
void 	GetandAnalizeResidualData(pm_machine_profile_t *arg);

void 	FillwUnknown(pm_machine_profile_t *arg);

int	iplcb_get(void *dest, int address, int num_bytes, int iocall);

int 	get_residual(RESIDUAL *rp);

ulong 	GetPMPnpHeapOffset(RESIDUAL * presidual);

PMsupportPack *	GetPMmachineUniqueData(RESIDUAL * presidual);

char * decode_dev(ulong devid);


#define SWAP_ENDIAN(num)						\
	(((num & 0x000000FF) << 24) + ((num & 0x0000FF00) << 8) +	\
	 ((num & 0x00FF0000) >> 8) + ((num & 0xFF000000) >> 24))


/*
 * NAME: GetandAnalizeResidualData
 *
 * FUNCTION: Getting the essential PM machine profile data from 
 * 	     "RESIDUAL DATA" which was given from ROS and saved in 
 *	     kernel static data area.   
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called only from the process environment.
 *      It can page fault.
 *
 * INPUT:  pm_machine_profile_t *arg
 * OUTPUT: pm_machine_profile_t *arg
 *
 * RETURN VALUE DESCRIPTION:
 *	   	none
 */
void
GetandAnalizeResidualData(pm_machine_profile_t *arg)
{
	RESIDUAL residual;
	PMsupportPack * p;

	arg->hibernation_support = UNKNOWN;
	arg->suspend_support = UNKNOWN;
	FillwUnknown(arg);	

	if (get_residual(&residual) == -1) {
		return;
	}

	if (!(p = GetPMmachineUniqueData(&residual))) {
		return;
	}		
	
#if PM_DEBUG
	printf("PMattribute is %08X\n", p->PMattribute);
#endif
	p->PMattribute = SWAP_ENDIAN(p->PMattribute);

#if PM_DEBUG
	printf("PMattribute is %08X\n", p->PMattribute);
#endif

        arg->hibernation_support 
	= (((p->PMattribute) & hibernation_support) != 0);
        arg->suspend_support 
	= (((p->PMattribute) & suspend_support) != 0);
        arg->LID_support 
	= (((p->PMattribute) & LID_support) != 0);
        arg->battery_support 
	= (((p->PMattribute) & battery_support) != 0); 
        arg->software_controllable_main_power_switch_support 
	= (((p->PMattribute) & software_controllable_main_power_switch) != 0);
        arg->fail_safe_HW_main_power_off_switch_support 
	= (((p->PMattribute) & fail_safe_hardware_main_power_off_switch) != 0);
	arg->inhibit_auto_transition
	= (((p->PMattribute) & inhibit_auto_transition) != 0);
        arg->resume_events.MDM_ring_resume_from_suspend 
	= (((p->PMattribute) & ringing_resume_from_suspend) != 0);
        arg->resume_events.MDM_ring_resume_from_hibernation 
	= (((p->PMattribute) & ringing_resume_from_hibernation) != 0);
        arg->resume_events.critical_low_battery_hibernation_from_suspend 
	= (((p->PMattribute) & battery_support) != 0); 

        arg->resume_events.GP_input_pin_for_resume_from_suspend 
        =(((p->PMattribute) & general_purpose_input_pin_for_resume_trigger)!=0);
        arg->resume_events.GP_input_pin_for_resume_from_hibernation 
        =(((p->PMattribute) & general_purpose_input_pin_for_resume_trigger)!=0);
        arg->resume_events.internal_keyboard_resume_from_suspend =  UNKNOWN; 
        arg->resume_events.external_keyboard_resume_from_hibernation = UNKNOWN;
        arg->resume_events.internal_mouse_resume_from_suspend =  UNKNOWN;
        arg->resume_events.external_mouse_resume_from_hibernation = UNKNOWN; 
        arg->resume_events.LID_open_resume_from_suspend 
	= (((p->PMattribute) & LID_support) != 0);
        arg->resume_events.LID_open_resume_from_hibernation 
	= (((p->PMattribute) & LID_support) != 0);
        arg->resume_events.main_power_switch_resume_from_suspend 
	= (((p->PMattribute) & software_controllable_main_power_switch) != 0);
        arg->resume_events.main_power_switch_resume_from_hibernation
	= (((p->PMattribute) & software_controllable_main_power_switch) != 0);

	arg->resume_events.resume_from_suspend_at_specified_time
	= (((p->PMattribute) & resume_from_suspend_at_specified_time) != 0);
	arg->resume_events.resume_from_hibernation_at_specified_time
	= (((p->PMattribute)& resume_from_hibernation_at_specified_time) != 0);
	arg->resume_events.hibernation_from_suspend_at_specified_time
	= (((p->PMattribute) & hibernation_from_suspend_at_specified_time) !=0);
	arg->resume_events.resume_button_support 
	= (((p->PMattribute) & resume_button_support) != 0);


#ifdef PM_DEBUG
        printf("MODEL name: %s\n",(residual.VitalProductData.PrintableModel));
#endif
	memcpy(arg->mach_name,(residual.VitalProductData.PrintableModel),32);

	return;

} /* GetandAnalizeResidualData */


/*
 * NAME: GetPMPnpHeapOffset 
 *
 * FUNCTION: Get the offset value of PM pnp packet in residual PNP heap 
 *
 * INPUT: pointer to the data block having residual data 
 *
 * RETURN VALUE DESCRIPTION:
 *	 ulong offset
 *  	 (If offset is equal to 0, no PM packet couldn't be found out.)
 */
ulong
GetPMPnpHeapOffset(RESIDUAL * presidual)
{
	PPC_DEVICE  *search_ptr;
	char *	cp; 
	int	i;
	char * 	PMID = PNPPM;

	for (search_ptr = presidual->Devices, i=0; i < MAX_DEVICES; i++) {
		cp = decode_dev(search_ptr->DeviceId.DevId);
		if (strcmp(cp ,PMID) == 0) {
			return (search_ptr->AllocatedOffset);
		}
		search_ptr++;

	} /* endfor */
	return NULL;
}


/*
 * NAME: GetPMmachineUniqueData 
 *
 * FUNCTION: Get the address pointing to PMattribute of PM packet 
 *
 * INPUT: pointer to the data block having residual data 
 *
 * RETURN VALUE DESCRIPTION:
 *	 
 *  	 (If offset is equal to 0, no PM packet couldn't be found out.)
 */
PMsupportPack *
GetPMmachineUniqueData(RESIDUAL * presidual)
{
	char * p;
	ulong m,n;
	char * i;
	int	s,t;

	p = presidual->DevicePnPHeap;
	if (!(m = GetPMPnpHeapOffset(presidual))) {
		return NULL;
	}
	n = (ulong)p;
	n += m;
	p = (char *)n;

#if PM_DEBUG
	i = p;
	for (s=0; s < 0x10; s++) {
		printf ("%08X	",i);
		for (t=0; t < 0x10; t++) {
			printf("%02X ", *(i++));
		} /* endfor */
		printf("\n");
	} /* endfor */ 
#endif

	while (p < (presidual->DevicePnPHeap) + 2*MAX_DEVICES*AVE_PNP_SIZE) {
#if PM_DEBUG
		printf ("packet boundary address 	%08X\n",p);
#endif
		if (*p == PMsupport_Packet) {
			if ( ((PMsupportPack *)p)->Type == 0x08 ) {
				return (PMsupportPack *)p;
			}
		}
		if (tag_type(*p)) { 
			m = (ulong)(*(p+1));
			n = (ulong)(*(p+2));
			n = (n<<8) + m +3;	/* because of little endian */
			m = (ulong)p;
			m += n;
			p = (char *)m;
		} else {
			p = p + tag_small_count(*p) + 1;
		} /* endif */
	} /* endwhile */

	return NULL;
}


		
/*
 * NAME: FillwUnknown
 *
 * FUNCTION: The unknown items in profile area are filled with "unknown" value. 
 *
 * INPUT:  pointer of pm_machine_profile structure 
 * OUTPUT: none 
 *
 * RETURN VALUE DESCRIPTION:
 *	        none
 */
void
FillwUnknown(pm_machine_profile_t *arg)
{
        arg->LID_support = 
        arg->battery_support = 
        arg->software_controllable_main_power_switch_support = 
        arg->fail_safe_HW_main_power_off_switch_support = 
        arg->resume_events.MDM_ring_resume_from_suspend = 
        arg->resume_events.MDM_ring_resume_from_hibernation = 
        arg->resume_events.critical_low_battery_hibernation_from_suspend = 
        arg->resume_events.GP_input_pin_for_resume_from_suspend = 
        arg->resume_events.GP_input_pin_for_resume_from_hibernation = 
        arg->resume_events.internal_keyboard_resume_from_suspend =
        arg->resume_events.external_keyboard_resume_from_hibernation = 
        arg->resume_events.internal_mouse_resume_from_suspend =
        arg->resume_events.external_mouse_resume_from_hibernation = 
        arg->resume_events.LID_open_resume_from_suspend = 
        arg->resume_events.LID_open_resume_from_hibernation = 
        arg->resume_events.main_power_switch_resume_from_suspend = 
        arg->resume_events.main_power_switch_resume_from_hibernation = UNKNOWN;
   	return;
}



/*
 * 
 *  Access IPL control block using mdd 
 *
 */
int
iplcb_get(void *dest, int address, int num_bytes, int iocall)
{
        MACH_DD_IO      mdd;
        struct file     *fp;

	/* Open NVRAM and BUS DEVICES  to obtain basic machine information */
        if(fp_open("/dev/nvram", 0, 0, 0, SYS_ADSPACE, &fp) != 0){
                return -1;
        }

        mdd.md_addr = (long)address;
        mdd.md_data = dest;
        mdd.md_size = (long)num_bytes;
        mdd.md_incr = MV_BYTE;

        if(fp_ioctl(fp, iocall, &mdd, 0) != 0){
                return -1;
        }

        fp_close(fp);
        return 0;

} /* iplcb_get */


/*
 * NAME: get_residual 
 *
 * FUNCTION: Get the pointer to the data structure of residual data which 
 *           is located in IPL control block in base kernel global data. 
 * 	     Originally, this data structure is given by ROS at boot.
 *
 * INPUT/OUTPUT: RESIDUAL *rp
 * 
 * NOTE:  Get Processor information in IPL control block which can be obtained 
 * 	  by machine DD(/dev/nvram). MachineDD(/kernel/io/machdd/md.c) gets the
 *        IPL control block which resides in vmker(kernel base global data).
 *        Although kernel extension can not access that global data, kernel
 *        extension module can ask machine DD to get it through a specific 
 * 	  IOctrl.  (Refer to <sys/vmker.h> for the structure of vmker which 
 * 	  contains the pointer to IPL control block.
 */
int
get_residual(RESIDUAL *rp)
{
	IPL_CB		ipl_cb;

	/* get the whole data of IPL control block */
        if(iplcb_get(&ipl_cb, 0, sizeof(IPL_CB), MIOIPLCB) != 0){
                return -1;
        }

        if(iplcb_get(rp, ipl_cb.s0.residual_offset, 
				ipl_cb.s0.residual_size, MIOIPLCB) != 0){
                return -1;
        }
        return 0;

} /* get_residual */



/*
 * NAME:  decode_dev
 *
 * FUNCTION:  decodes the devid into a string.
 *
 * DATA STRUCTURES:
 *      uint devid     specifies residual.Devices[?].DeviceId.DevId
 *
 * NOTE: Note that this field is big-endian. I don't know why.
 *
 * RETURN VALUE DESCRIPTION:
 *      a poiner to the decoded string.
 */
char *
decode_dev(ulong devid)
{
	int		i, j;
	static char    code[8];

/*	devid = SWAP_ENDIAN(devid); */

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

