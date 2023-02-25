static char sccsid[] = "@(#)68	1.3  src/rspc/usr/lib/methods/probe_ibment/probe_ibment.c, isaent, rspc411, 9438C411a 9/23/94 12:57:32";
/*
 *   COMPONENT_NAME: isaent
 *
 *   FUNCTIONS: err_exit
 *		get_isa_port
 *		iplcb_get
 *		main
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* ISA bus device probe - detect device type at port and define it	*/
/* This probe is specifically for Ethernet for adapter/isa/ethernet	*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"

#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/residual.h>
#include <iplcb.h>



				/* Declare external functions		*/
extern  int genseq();
int iplcb_get(void *, int, int, int);

				/* Define constants by device type	*/
		/*  START *--* START *--* START *--* START *--* START   */
		/* DEVICE SPECIFIC SECTION *--* DEVICE SPECIFIC SECTION */

#define		TOKEN_BOOT	'O'
#define		ETHERNET_BOOT	'D'
#define		THIS_TYPE  	ETHERNET_BOOT

#define		PNPID	  0x1204d24	
#define		DEVUT	  "adapter/isa/ethernet"
#define		DTYPE	  "ethernet"
#define		DCLASS	  "adapter"
#define		SCLASS	  "isa"

		/* DEVICE SPECIFIC SECTION *--* DEVICE SPECIFIC SECTION */
		/*   END  *--*  END  *--*  END  *--*  END  *--*  END    */

main(argc,argv,envp)
int	argc;
char	*argv[];
char	*envp[];

{
extern  int     optind;         	/* for getopt function */
extern  char    *optarg;        	/* for getopt function */

char	*class,*subclass,*type,		/* parameter pointers */
	*parent,*lname;
char	connect[16];
char	devname[16];
char	sstring[256];			/* search string */
char	*gattr;
char    parloc[LOCSIZE],                /* parents location */
	parut[UNIQUESIZE];              /* parents unique type */

struct  Class   *predev,        /* handle for predefined devices class    */
		*cusdev;        /* handle for customized devices class    */

struct listinfo  cudv_info;	/* info for list of cudv objects	  */


struct	PdDv	PdDv;
struct	PdCn	PdCn;
struct	CuDv	CuDv, *cudv_list;
struct	CuAt	*cuat;

char	foundit;		/* Flag for existing adapter search	*/
int	found;
int	heap_offset;		/* Heap walker variable			*/
int     seqno;
int	rc,i,slot;
char	bus_name[16];

				/* Some variables for ISA devices	*/

		/*  START *--* START *--* START *--* START *--* START   */
		/* DEVICE SPECIFIC SECTION *--* DEVICE SPECIFIC SECTION */

IPL_DIRECTORY 	iplcb_dir;      /* IPL control block directory 		*/
RESIDUAL      	*rp;		/* Residual data struct pointer		*/
IPL_INFO 	iplcb_info;	/* IPL control blk info section 	*/
NET_DATA	net_data;	/* IPL control blk network data section */

struct _S9_Pack	*s9_packet;	/* Pointer to device descriptor packet	*/

unsigned char	port_val;	/* Value found in the port from ISA I/O	*/
char		*media_type;	/* Media type attribute for ethernet	*/
int		io_address;	/* I/O address found in residual 	*/
int		boot_device;	/* Is this the boot device		*/

		/* DEVICE SPECIFIC SECTION *--* DEVICE SPECIFIC SECTION */
		/*   END  *--*  END  *--*  END  *--*  END  *--*  END    */

strcpy(bus_name,"bus1");	/* Set default bus name 		*/

if(argc > 1)			/* Allow bus name as optional parameter */
	while ((i = getopt(argc,argv,"p:")) != EOF) 
		{
		switch (i) 
			{
			case 'p':
			strcpy(bus_name,optarg);
			break;

			default:
			exit(E_ARGS);
			break;
			}
		}

				/* Setup variables for this probe	*/
class = DCLASS;
subclass = SCLASS;
type = DTYPE;
parent = bus_name;

				/* Check residual data for this adapter */
		/*  START *--* START *--* START *--* START *--* START   */
		/* DEVICE SPECIFIC SECTION *--* DEVICE SPECIFIC SECTION */

foundit = FALSE;

if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB)) 
	{
	perror("Error reading IPL-Ctrl block directory");
	exit(1);
        }

					/* It's variable size 		*/
rp = (RESIDUAL *) malloc(iplcb_dir.residual_size);	

if (iplcb_get(rp,iplcb_dir.residual_offset,iplcb_dir.residual_size,MIOIPLCB)) 
	{
	perror("Error reading Residual Data");
	exit(1);
        }

DEBUG_1("Residual Model = %s\n",rp->VitalProductData.PrintableModel);


for (i=0; i<rp->ActualNumDevices; i++) 
	{
	if(rp->Devices[i].DeviceId.DevId == PNPID)
		{
		heap_offset = 0;
		s9_packet =(struct _S9_Pack *)&rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset+heap_offset];
						/* Find the I/O address packet	*/
						/* Better be the 1st one	*/
		while(s9_packet->Tag != 0x4B &&
			(rp->Devices[i].AllocatedOffset+heap_offset < rp->Devices[i].PossibleOffset))		
			{
			if(s9_packet->Tag & 0x80)	/* Large packet tag	*/
				heap_offset += (s9_packet->Range[1]*256)+s9_packet->Range[0]+3;
			else
				heap_offset += (s9_packet->Tag & 0x07)+1;
			s9_packet =(struct _S9_Pack *)&rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset+heap_offset];
			}

		io_address = (s9_packet->Range[1] * 256) + s9_packet->Range[0];

		DEBUG_4("IO Address for Devid: 0x%8.8X = 0x%02X%02X  0x%4.4X\n",
			rp->Devices[i].DeviceId.DevId,
			s9_packet->Range[1],
			s9_packet->Range[0],
			io_address);

		i = rp->ActualNumDevices;	/* Found it get out	*/

					/* Get a byte from the IO Port	*/
		port_val = get_isa_port(io_address+0x1b, bus_name);
		DEBUG_2("Value of port %3.3X = %4.4X\n",io_address,port_val);
		foundit = TRUE;
		}
	} 

if(foundit && (port_val != 0xFF))	/* Found in residual and valid port	*/
	{
	switch(port_val & 0x03)
		{
		case	0:
		media_type = "twisted-pair";
		break;

		case	1:
		media_type = "bnc";
		break;

		case	2:
		media_type = "dix";
		break;

		default:
		media_type = "bnc";	/* Default in case of failure		*/
		break;
		}
	
	DEBUG_1("Media type = %s \n", media_type);

					/* Is it the boot device		*/

	boot_device = FALSE;
	if (iplcb_get(&iplcb_info,iplcb_dir.ipl_info_offset,
			sizeof(iplcb_info), MIOIPLCB))
		{
		DEBUG_0("Error reading IPL-Ctrl block info section");
		err_exit(0x88);
		}
					/* Check for network boot		*/
	if(iplcb_info.previpl_device[1] == THIS_TYPE) 
		boot_device = TRUE;
	}
else
	err_exit(E_NODETECT);		/* No device detected			*/

		/* DEVICE SPECIFIC SECTION *--* DEVICE SPECIFIC SECTION */
		/*   END  *--*  END  *--*  END  *--*  END  *--*  END    */
				/* We found it in residual data so define */

				/* start up odm */
if (odm_initialize() == -1) 
	{
	DEBUG_0("probe: odm_initialize() failed\n")
	exit(E_ODMINIT);
	}

DEBUG_0 ("ODM initialized\n")

				/* get predefined object for this device */
sprintf(sstring,"uniquetype = '%s'",DEVUT);

rc = (int)odm_get_first(PdDv_CLASS,sstring,&PdDv);
if (rc==0) 
	{
	DEBUG_0("probe: No PdDv object\n")
	err_exit(E_NOPdDv);
	}
else if (rc==-1) 				/* ODM error */
	{
	DEBUG_0("probe: ODM error getting PdDv object\n")
	err_exit(E_ODMGET);
	}

					/* open CuDv			*/
if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) 
	{
	DEBUG_0("probe: open of CuDv failed\n")
	err_exit(E_ODMOPEN);
	}

					/* Attempt to find this one	
					   Check for bus_io_addr	*/
foundit = FALSE;

sprintf(sstring,"PdDvLn = %s",DEVUT);
cudv_list = odm_get_list(CuDv_CLASS, sstring, &cudv_info,50, 1) ;
if ((int)cudv_list != -1)
	{
	DEBUG_1("probe: Got %d devices from CuDv\n", cudv_info.num)

	if (cudv_info.num > 0)
		{
		for (i = 0; i < cudv_info.num; i++)
			{
			DEBUG_1("probe: Looking at %s\n",cudv_list->name);
			cuat = getattr(cudv_list->name,"bus_io_addr",FALSE,&found);
			if (cuat != (struct CuAt *)NULL)
				{
				DEBUG_3("probe: For Adapter: %s found bus_io_addr = %x  wanted = %x\n",
					 cuat->name,strtoul(cuat->value,NULL,0), io_address);
				if(strtoul(cuat->value,NULL,0) == io_address)
					{
					strcpy(devname,cuat->name);
					lname = devname;
					foundit = TRUE;
					DEBUG_1("probe: Found a match %s\n",lname);
					}
				}
			cudv_list++;
			}
		}
	else
		{
		DEBUG_0("probe: No CuDv objects found \n");
		}
	}
else
	{
	DEBUG_1("probe: odm_get_list failed odmerrno = %s\n",odmerrno);
	}
	
odm_free_list(cudv_list, &cudv_info);

				/* If no device found then define it	*/
if(!foundit)
	{
	DEBUG_0("probe: Define a new device for this isa adapter\n");

					/* Get a connection point	*/
	sprintf(sstring, "parent=%s", parent);
	cudv_list = odm_get_list(cusdev,sstring,&cudv_info,1,1 );
		     
	if ( (int)cudv_list == -1 ) 
		exit(E_ODMGET);
		
	if ( cudv_list==NULL ) 	/* there are no isa slots Use first one (1) */
		slot = 0;
	else  			/* find the lower number un-used slot */
		{
		for (i=0; i<cudv_info.num; i++) 
			cudv_list[i]._scratch = FALSE;
			
		for (i=0; i<cudv_info.num; i++) 
			{
			slot = atol(cudv_list[i].connwhere) - 1;
		
			if (slot < 0)
				{
				DEBUG_0("connwhere -1 is less than 0.  loop. \n");
				continue;
				}
			if (slot < cudv_info.num)
				cudv_list[slot]._scratch = TRUE;
			}
						/* Check for holes		*/		
		slot = cudv_info.num;  
		
		for (i=0; i<cudv_info.num; i++)
			{
			if (cudv_list[i]._scratch == FALSE)	/* Found an unused number */
				{
				slot = i;
				break;
				}
			}
		}

	slot++;				/* Convert back to 1 based index	*/
	sprintf(connect, "%d", slot);

				     /* free the list from the odm_get call */
	odm_free_list(cudv_list, &cudv_info);
		


				/* get parent customized device object */
			
	sprintf(sstring,"name = '%s'",parent);
	rc =(int)odm_get_first(cusdev,sstring,&CuDv);
	if (rc==0) 
		{
		DEBUG_0("probe: No parent CuDv object\n")
		err_exit(E_NOCuDvPARENT);
		}
	else if (rc==-1) 
		{
		DEBUG_0("probe: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
		}

				/* Get parent info for location		*/
	strcpy(parut,CuDv.PdDvLn_Lvalue);
	strcpy(parloc,CuDv.location);
	DEBUG_3("probe: Parent info name: %s  ut: %s  loc: %s\n",
		parent,parut,parloc); 

				/* create a new customized device object */
	DEBUG_1 ("creating customized object: %s\n",lname)
	CuDv.status      = DEFINED;
	CuDv.chgstatus   = PdDv.chgstatus;
	strcpy(CuDv.ddins,PdDv.DvDr);
	strcpy(CuDv.parent,parent);
	strcpy(CuDv.connwhere,connect);
	strcpy(CuDv.PdDvLn_Lvalue,PdDv.uniquetype);
	CuDv.location[0] = '\0';
				/* Special location if was the boot device */
	if(boot_device)
		{
		strcpy(CuDv.location,parloc);
		CuDv.location[4] = 'B';
		}
	else
		location(parut,parloc,&CuDv);

				/* now run the define method 		*/

	sprintf(sstring, "-c %s -s %s -t %s -p %s -w %s -L %s",
		class,subclass,type,parent,connect,CuDv.location);

	DEBUG_2("run method:%s %s\n", PdDv.Define, sstring);
	if (odm_run_method(PdDv.Define, sstring, &lname, NULL)) 
		{
		DEBUG_1("error running %s\n", PdDv.Define);
		return;
		}

				/* strip off the trailing character 	*/
	lname[strlen(lname)-1] = '\0';

	DEBUG_1 ("created customized device: %s\n",lname);

	}
				/* Update customized object class 	*/

		/*  START *--* START *--* START *--* START *--* START   */
		/* DEVICE SPECIFIC SECTION *--* DEVICE SPECIFIC SECTION */

gattr = "bus_io_addr";
DEBUG_2("probe: Updating %s attr for %s\n",gattr,lname);
if ((cuat=getattr(lname,gattr,FALSE,&found))==NULL) 
	{
	DEBUG_2("error getting %s attr for %s\n",gattr,lname)
	err_exit(E_ODMGET);
	}
sprintf(sstring,"0x%x",io_address);
strcpy(cuat->value,sstring);
if((putattr(cuat)) == -1)
	{
	DEBUG_2("error updating %s attr for %s\n",gattr,lname)
	err_exit(E_ODMUPDATE);
	}

gattr = "media_type";
DEBUG_2("probe: Updating %s attr for %s\n",gattr,lname);
if ((cuat=getattr(lname,gattr,FALSE,&found))==NULL) 
	{
	DEBUG_2("error getting %s attr for %s\n",gattr,lname)
	err_exit(E_ODMGET);
	}
sprintf(sstring,"%s",media_type);
strcpy(cuat->value,sstring);
if((putattr(cuat)) == -1)
	{
	DEBUG_2("error updating %s attr for %s\n",gattr,lname)
	err_exit(E_ODMUPDATE);
	}

		/* DEVICE SPECIFIC SECTION *--* DEVICE SPECIFIC SECTION */
		/*   END  *--*  END  *--*  END  *--*  END  *--*  END    */


				/* All done shut it down		*/
if (odm_close_class(CuDv_CLASS) == -1) 
	{
	DEBUG_0("probe: failure closing CuDv object class\n")
	err_exit(E_ODMCLOSE);
	}

odm_terminate();

fprintf(stdout,"%s ",lname);
exit(E_OK);

}


/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * RETURNS:
 *               None
 */

err_exit(exitcode)
char    exitcode;
{
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_terminate();
	exit(exitcode);
}


/* --------------------------------------------------------------------------*/
/*
 * iplcb_get: Read in section of the IPL control block.  The directory
 *		section starts at offset 128.
 */
int
iplcb_get(void *dest, int address, int num_bytes, int iocall)
{
	int		fd;		/* file descriptor */
	MACH_DD_IO	mdd;


	if ((fd = open("/dev/nvram",0)) < 0) {
		return(-1);
	}

	mdd.md_addr = (long)address;
	mdd.md_data = dest;
	mdd.md_size = (long)num_bytes;
	mdd.md_incr = MV_BYTE;

	if (ioctl(fd,iocall,&mdd)) {
		return(-1);
	}

	close(fd);
	return(0);
}

/****************************************************************************
   Reads an 8 bit byte from a given I/O port 
 ****************************************************************************/

get_isa_port(int ioaddr, char *bus_name)
{
int fd;                				/* Handle for /dev/busx     */
uchar data_ptr;          			/* Put reg contents here    */
MACH_DD_IO mddRecord; 				/* machine dd ioctl buffer  */
char	fnam[NAMESIZE];

strcpy(fnam,"/dev/");
strcat(fnam,bus_name);

if ((fd = open(fnam, O_RDWR)) <= 0)
	{
	printf("Couldn't open bus program stopping\n");
	exit(1);
	}
	
mddRecord.md_size = 1;
mddRecord.md_incr = MV_BYTE;
mddRecord.md_data = &data_ptr;
mddRecord.md_addr = ioaddr;

if(ioctl(fd, MIOBUSGET, &mddRecord))
	{
	perror("ioctl:");
	return(0);
	}
return((int) data_ptr);
}
