/* @(#)65    1.6  src/rspc/usr/lib/methods/cfgc1x/cfgc1x.c, isax25, rspc41J, 9519A_all  5/8/95  17:39:22 */
/*
 * COMPONENT_NAME: (ISAX25) cfgc1x
 *
 * FUNCTIONS: build_dds, generate_minor, make_special_files
 * FUNCTIONS: download_microcode, query_vpd, define_children
 * FUNCTIONS: point_indirect
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/mdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"
#include <sys/artic.h>
#include "lducode.h"

#define S_LENGTH  256           /* length of buffer for get commands */
#define ROS_SZ		0x40	/* length of ROS area */
#define PTRREG		0x02	/* Pointer Register     (PTRREG) */
#define DREG		0x03	/* Data Register          (DREG) */
#define MP_GENERIC_MSG	"180"


/*
 *
 * NAME: build_dds
 *
 * FUNCTION: Builds a DDS (Defined Data Structure) describing a device's
 *	characteristics to it's device driver.
 *
 * EXECUTION ENVIRONMENT:
 *	Operates as a device dependent subroutine called by the generic
 *	configure method for all devices.
 *
 * NOTES: There is no DDS for the ARTIC2 adapter (DDS is built for the
 *        specific device drivers) this is then a NULL function.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  build_dds( lognam, addr, len )
char	*lognam;		/* logical name of device */
uchar	*addr;			/* receiving pointer for DDS address */
long	*len;			/* receiving variable for DDS length */
{ 
	return 0; 
}
/*
 *
 * NAME: generate_minor
 *
 * FUNCTION: To provide a unique minor number for the current device instance.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: There is no minor number required for the ATRIC2 adapter,
 *        this function is NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int generate_minor( lognam, majorno, minordest )
char	*lognam;
long	majorno;
long	*minordest;
{ 
	return 0; 
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: To create the special character file on /dev for the current device
 *           instance.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the ARTIC2 adapter, there is no special character file on
 *        /dev, this function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int make_special_files( lognam, devno )
char	*lognam;
dev_t	devno;
{ 
	return 0; 
}

/*
 *
 * NAME: download_microcode
 *
 * FUNCTION: To download the micro code for the MP/2 or X.25 adapter.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the ARTIC2 adapter, micro code is downloaded through the
 *        device driver, this function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  download_microcode( lognam )
char  *lognam;
{ 
	return 0; 
}

/*
 *
 * NAME: query_vpd
 *
 * FUNCTION: To query the ARTIC2 device for VPD information (Vital Product Data)
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the ARTIC2 adapter, VPD information is not currently supported,
 *        this function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  query_vpd( newobj, kmid, devno, vpd_dest )
char   *newobj;
mid_t   kmid;
dev_t	devno;
char	*vpd_dest;
{ 
	return 0; 
}

/*
 * NAME     : ucfg_ampx
 *
 * FUNCTION : This function changes the adapter's state from AVAILABLE to
 *	DEFINED when an error occurs.
 *
 * EXECUTION ENVIRONMENT :
 *      This function returns 0 on success, < 0 on failure.
 *      Operates as a device dependent subroutine called by the generic
 *      configure method for all devices.
 *
 * NOTES :
 *      The articmpx adapter's state will be AVAILABLE at this time,
 *      so since there was an error detected the adapter's state should
 *      be changed to DEFINED.           
 *
 */

#define AMPX_UNIQUETYPE "uniquetype = 'adapter/isa/articmpx'"

void ucfg_ampx(lname)
char	*lname;			/* logical name of the device           */
{
	struct  PdDv   pddvadptr;
	int rc;
	char string[S_LENGTH];

	if(( rc = odm_get_obj( PdDv_CLASS, AMPX_UNIQUETYPE, &pddvadptr, ODM_FIRST)) == 0 ) {
		DEBUG_2("ucfg_ampx: get failed lname=%s rc=%d\n", lname ,rc)
		return;
	}

	sprintf( string, "-l %s ", lname );
	if(odm_run_method(pddvadptr.Unconfigure,string,NULL,NULL)){
		return;
	}

	return;
}

/*
 * NAME     : define_children
 *
 * FUNCTION : This function returns the names of devices that are
 *            attached (defined) to this device.  This functions
 *            searches the customized database for devices that have
 *            this device listed as the parent device.
 *
 * EXECUTION ENVIRONMENT :
 *      This function returns 0 on success, < 0 on failure.
 *      Operates as a device dependent subroutine called by the generic
 *      configure method for all devices.
 *
 * NOTES :
 *      The X.25 and multiport adapter's child devices are the device drivers
 *      that may be configured to run with this adapter.  The standard
 *      device is the X.25 device driver.
 *
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 *      Returns 0 on success, > 0 on failure.
 */

int
define_children(lname,ipl_phase)
char	*lname;			/* logical name of the device           */
int	ipl_phase;		/* ipl phase                            */
{
	struct	Class	*predev ;	/* predefined devices class ptr */
	struct	Class	*cusdev ;	/* customized devices class ptr */
	struct	PdDv	preobj ;	/* predefined device object     */
	struct	CuDv	cusobj ;	/* customized device object     */
	char	sstring[S_LENGTH];	/* search criteria              */
	int	rc;			/* return code                  */
	char	*out_p;

	/* declarations for card query */
	char	busdev[32] ;
	int	fd ;
	struct	CuAt	*cusatt;
	ulong	ulong_val;
	char	msg_no[4];	/* string to hold NLS test msg num. */
	POS_INFO	posinfo;	/* pos register information */
	ulong	intr_lvl;	/* interrupt level */
        char	subtype[4];	/* adapter EIB type */
	MACH_DD_IO	mddRecord;
	uchar	eibid[2];	/* storage for EIB value */
	uchar	port[2];	/* storage for MP/2 port value */
	uchar	pos_regs[6];	/* storage for pos register value */
	uchar	temp_io[1];	/* storage for io register value */
	int	slot_num;
	ushort	locreg;
	static uchar intr_lookup[16] = {
					0xff,0xff,0x03,0x00,
					0x01,0xff,0xff,0x02,
					0xff,0x03,0x04,0x05,
					0x06,0xff,0xff,0x07};
	uchar	level,	card_num;
	int	c_i;
	uchar		roscopy[ROS_SZ];

	DEBUG_0("cfgc1x: define children\n");

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("cfgc1x: open of CuDv failed\n");
		ucfg_ampx(lname);
		err_exit(E_ODMOPEN);
	}

	DEBUG_0("cfgc1x: card query\n");

	/* Get parent device and where connected */
	sprintf(sstring,"name = '%s'",lname);
	rc =(int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("cfgc1x: No device CuDv object\n");
		ucfg_ampx(lname);
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("cfgc1x: ODM error getting CuDv object\n");
		ucfg_ampx(lname);
		err_exit(E_ODMGET);
	}

	DEBUG_3("cfgc1x: name = %s, parent = %s, connwhere = %s\n",
		lname,cusobj.parent, cusobj.connwhere)

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &posinfo.win_base_addr, 'i', CuAt_CLASS,
		PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
		"bus_mem_addr", (struct attr *) NULL )) > 0 )
	{
		DEBUG_0("cfgc1x: ODM error getting win_base_addr\n");
		ucfg_ampx(lname);
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &posinfo.baseio, 'i', CuAt_CLASS,
		PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
		"bus_io_addr", (struct attr *) NULL )) > 0 )
	{
		DEBUG_0("cfgc1x: ODM error getting baseio\n");
		ucfg_ampx(lname);
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &posinfo.win_size, 'i', CuAt_CLASS,
		PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
		"window_size", (struct attr *) NULL )) > 0 )
	{
		DEBUG_0("cfgc1x: ODM error getting win_size\n");
		ucfg_ampx(lname);
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &intr_lvl, 'i', CuAt_CLASS,
		PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
		"bus_intr_lvl", (struct attr *) NULL )) > 0 ) 
	{
		DEBUG_0("cfgc1x: ODM error getting intr_lvl\n");
		ucfg_ampx(lname);
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
	if ( (( cusatt = get_attrval( lname, "_subtype",
		sstring, &ulong_val, &rc ) ) != NULL ) &&
		( rc == E_OK ) ) 
	{

		strcpy( subtype, cusatt->value );
	}
	else
	{
		DEBUG_0("cfgc1x: ODM error getting _subtype\n");
				ucfg_ampx(lname);            
		return E_BADATTR;
	}

	sprintf(busdev, "/dev/%s", cusobj.parent) ;
	DEBUG_1("cfgc1x: opening bus device %s \n", busdev);

	if (0 > (fd = open(busdev, O_RDWR)) )
	{
		DEBUG_0("cfgc1x: Failed opening bus device\n");
		perror("[busquery]open()");
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}

	slot_num = atoi(cusobj.connwhere) - 1;
	eibid[0]=NOBOARD;

	/* set pos reg 2 (INITREG0), to enable adapter  */
	if (point_indirect(fd, 0x08, posinfo.baseio))
	{
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}

	mddRecord.md_size = 1;          /* build mdd record */
	mddRecord.md_incr = MV_BYTE;
	mddRecord.md_addr = (posinfo.baseio + DREG);

	/* pos 2 - base addr, interrupt level, enable bit */
	pos_regs[2] = (unsigned char) 0;
	card_num = ((posinfo.baseio - 0x2a0) / 0x400);
	pos_regs[2] = card_num << 4;

	mddRecord.md_data = &pos_regs[2];	 /* addr of data */
	if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
	{
		perror("[busquery]ioctl()");
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}

	/* set (LOCREG0) ne POS 3 */

	mddRecord.md_size = 1;          /* build mdd record */
	mddRecord.md_incr = MV_SHORT;
	mddRecord.md_addr = (posinfo.baseio + 0x00);

	/* locreg0 - bits 13-20 of bus_mem_addr */
	pos_regs[3] = (unsigned char) 0;
	pos_regs[3] = (posinfo.win_base_addr & 0x0fe000) >> 13;


	/* set (LOCREG1) ne POS 4 */
	pos_regs[4] = (unsigned char) 0;
	pos_regs[4] = (posinfo.win_base_addr & 0xf00000) >> 20; 

	locreg = ((pos_regs[3] << 8) | pos_regs[4]);

	DEBUG_2("cfgc1x: write to LOCREG0,1 = 0x%X, slot_num = %d\n",
			locreg,slot_num+1)

	mddRecord.md_data = (uchar*)(&locreg);        /* addr of data */

	if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
	{
		perror("[busquery]ioctl()");
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}

	/* write to I/O base registers to enable memory */

	mddRecord.md_size = 1;          /* build mdd record */
	mddRecord.md_incr = MV_BYTE;
	mddRecord.md_addr = posinfo.baseio + 5;

	/* write 0 to I/O reg + 5 */
	temp_io[0] = (unsigned char) 0;

	mddRecord.md_data = temp_io;        /* addr of data */

	if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
	{
		perror("[busquery]ioctl()");
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}

	DEBUG_2("cfgc1x: write I/O REG = %x at addr = %x\n",
			temp_io[0],mddRecord.md_addr)

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ wait (up to 3 sec) for I/O to come             ³
	³ on before attempting to access adapter memory  ³
	³ Note that the PROM ready bit in INITREG1 does  ³
	³ not work on the C1X, so here we just make sure ³
	³ PTRREG is alive, and reset() checks that the   ³
	³ PROM has completed initialization.             ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	for ( c_i = 0 ; c_i < 3; c_i++ )
	{

		/* See if the adapter ROS indicates READY */
		mddRecord.md_size = 1;		/* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		temp_io[0] = c_i;		/* value to test for */
		temp_io[1] = (unsigned char) 0;	/* value to compare */
		mddRecord.md_data = temp_io;	/* addr of data */
		mddRecord.md_addr = posinfo.baseio + PTRREG;

		if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
		{
			DEBUG_1("cfgc1x: Attempting to write to addr %x\n",
					mddRecord.md_addr );
			perror("[busquery]ioctl()");
			close(fd);
			ucfg_ampx(lname);            
			return(E_DEVACCESS);
		}
		DEBUG_2("cfgc1x: Wrote %x to addr %x\n", c_i,
				mddRecord.md_addr );

		sleep( 1 );			/* wait 1 sec */

		mddRecord.md_sla = 1;          /* build mdd record */
		mddRecord.md_size = 1;          /* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_data = &temp_io[1]; /* addr of data */
		mddRecord.md_addr = (posinfo.baseio + PTRREG);
		if (0 > ioctl(fd, MIOBUSGET, &mddRecord))
		{
			DEBUG_1("cfgc1x: Attempting to read addr %x\n",
					mddRecord.md_addr );
			perror("[busquery]ioctl()");
			close(fd);
			ucfg_ampx(lname);            
			return(E_DEVACCESS);
		}

		DEBUG_2("cfgc1x: Read %x at addr %x\n", temp_io[1],
				mddRecord.md_addr );
		if ( temp_io[0] == temp_io[1] )
        	break;
	}
	if( c_i >= 3 )
	{
		DEBUG_1("cfgc1x: timeout trying to read addr %x\n",
				mddRecord.md_addr );
		perror("[busquery]ioctl()");
		close(fd);
		ucfg_ampx(lname);            
		return(E_DEVACCESS);
	}

	strcpy(msg_no,X25_C2X_MSG);     /* X.25 Co-Processor/2 */

	DEBUG_3("cfgc1x: addr = %x, value read = %x, msg_no =%s\n",
			mddRecord.md_addr,eibid[0],msg_no)

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ After loading and running geteibid.com        ³
	³ Reset card so it returns to its initial state ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if (rc = reset(fd, &posinfo))
	{
		DEBUG_1("Error from reset(), rc = %d\n",rc)
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}

	/* set (INITREG0), to disable adapter  */
	if (point_indirect(fd, 0x08, posinfo.baseio))
	{
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}


	mddRecord.md_size = 1;          /* build mdd record */
	mddRecord.md_incr = MV_BYTE;
	mddRecord.md_addr = (posinfo.baseio + DREG);

	/* pos 2 - base addr, interrupt level, enable bit */
	pos_regs[2] = (unsigned char) 0;
	pos_regs[2] = card_num << 4;

	mddRecord.md_data = &pos_regs[2];        /* addr of data */
	if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
	{
		perror("[busquery]ioctl()");
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}

	DEBUG_1("cfgc1x: write to INITREG0 = %x\n", pos_regs[2])
	/* read interrupt level from adapter  */
	if (point_indirect(fd, 0x12, posinfo.baseio))
	{
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}

	if (readio_reg(fd, posinfo.baseio + DREG, &pos_regs[2]))
	{
		perror("[busquery]ioctl()");
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}
	if ((pos_regs[2] & 0x07) != intr_lookup[intr_lvl])
	{
		DEBUG_1("cfgc1x: ODM bus_intr_lvl does not match board (%d)\n",
			pos_regs[2]);
		if(intr_lookup[intr_lvl] != 0xFF)
		{
			char onstr[]=" ON";
			char offstr[]="OFF";
			char	*sw1,*sw2,*sw3;

			sw1 = (intr_lookup[intr_lvl] & 0x01) ? offstr : onstr;
			sw2 = (intr_lookup[intr_lvl] & 0x02) ? offstr : onstr;
			sw3 = (intr_lookup[intr_lvl] & 0x04) ? offstr : onstr;
			fprintf(stderr,
			"cfgc1x:\nThe interrupt level switch settings on this adapter (switches 1,2, and 3)\nare incorrect for the specified interrupt value.\n");
			fprintf(stderr,
			"The switch settings for interrupt level %d should be:\n\n\tswitch 1 - %s\n\tswitch 2 - %s\n\tswitch 3 - %s\n\n",
				intr_lvl,
				sw1,
				sw2,
				sw3);
			fprintf(stderr,
			"Please set these switches to the above positions, then try again.\n");
		}
		else
		{
		fprintf(stderr,
		"This interrupt value is not supported by this adapter\n");
		}
		close(fd);
		ucfg_ampx(lname);
		return E_BADATTR;
	}


	    close(fd);

	    /*------------------------------------------------------|
	     | get the customized attribute (_subtype) used to      |
	     | define which type of adapter and EIB pair we have    |
	     |------------------------------------------------------*/

	    if ( (( cusatt = get_attrval( lname, "_subtype", sstring,
		&ulong_val, &rc ) ) != NULL )
		&&
		( rc == E_OK ) )
	    {
		/*----------------------------------------------|
		 | we got a pointer, therefore we  have a       |
		 | _subtype attribute.  Update the value.       |
		 | CuAt updates always take strings as values.  |
		 |----------------------------------------------*/

		strcpy( cusatt->value, msg_no );
		if ( putattr( cusatt ) < 0 )
		{
			close(fd);
			DEBUG_0("cfgc1x: cannot update subtype\n");
			ucfg_ampx(lname);
			rc = E_ODMUPDATE;
			return( rc ) ;
		}
	    }
	    close(fd);

	DEBUG_0("cfgc1x: define children: check children\n")

	/* search CusDevices for customized object with this logical
	   name as parent */
	sprintf(sstring, "parent = '%s'", lname);
	if ((rc = (int)odm_get_obj(cusdev, sstring, &cusobj, TRUE)) == 0) {
		/* odm objects not found */
		DEBUG_0("cfgc1x: no objects found\n")
		return(E_OK);
	}
	else if (rc == -1)
	{       DEBUG_1("cfgc1x: couldn't get child of %s\n",lname)
		return(E_ODMGET);
	}
	DEBUG_1("cfgc1x: name of child %s\n",cusobj.name)
	fprintf(stdout,"%s ",cusobj.name);

	rc = odm_close_class(cusdev);
	if(rc < 0){
		/* error closing object class */
		DEBUG_0("cfgc1x: close object class CuDv failed\n")
		return(E_ODMCLOSE);
	}
	return(E_OK);
}


int
point_indirect(int fd, int offset, int busaddr)
{
	MACH_DD_IO	mddRecord;
	uchar ofst=(uchar)offset;
	mddRecord.md_size = 1;			/* build mdd record */
	mddRecord.md_incr = MV_BYTE;
	mddRecord.md_addr = (busaddr + PTRREG);
	mddRecord.md_data = &ofst;		/* addr of data */
	mddRecord.md_sla = 0;			/* not used */

	if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
	{
		perror("[point_indirect]ioctl()");
		return(E_DEVACCESS);
	}
	return(0);
}

