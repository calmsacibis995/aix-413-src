static char sccsid[] = "@(#)34	1.5  src/rspc/kernext/pm/pmmd/IBM8301/pmmdmisc.c, pwrsysx, rspc41J, 9517A_all 4/25/95 17:05:24";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: GetandAnalizeResidualData, iplcb_get, get_residual,
 *		FillwAssumption, get_processor_info, GetL2CacheSize,
 *		decode_dev, write_ctl_bit, read_ctl_bit,
 *		write_host_pin, pulse_host_pin, read_host_pin,
 *		write_data_word, write_port, read_port
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1995
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

#include <sys/pm.h>
#include <pmmi.h>
#include "pm_sig750.h"


#define SIG_DELAY 100	/* microseconds delay between Signetics writes */

/*-------------------------*/
/*    external variable	   */
/*-------------------------*/
extern pm_md_data_t	pm_md_data;

/*-------------------------*/
/*    function proto type  */
/*-------------------------*/
void	write_ctl_bit(int byte_addr, int bit_addr, int data);
int	read_ctl_bit(int byte_addr, int bit_addr);
void	write_host_pin(int pin_addr, int data);
void	pulse_host_pin(int pin_addr, int data);
int	read_host_pin(int pin_addr);
void	write_data_word(int byte_addr, int data);
void	write_port(int addr, int data);
int	read_port(int addr);


/*
 * NAME:  write_ctl_bit
 *
 * FUNCTION:  Write a Signetics control bit
 *
 * DATA STRUCTURES:
 *	byte_addr	- low 3 bits of byte address
 *	bit_addr	- bit address (3 bits)
 *	data		- one bit
 *
 * NOTES:
 *	Write b' 0  0  0  1  0  d A5 A4  (to PM Control port 0x82A)
 *	delay 10 usec
 *	Write b' 0  0  0  1 A3 A2 A1 A0  (to PM Control port 0x82A)
 *		where	d = data
 *			A5 A4 A3 = low 3 bits of byte address
 *			A2 A1 A0 = bit address
 *
 * RETURN VALUE DESCRIPTION:
 *      None.
 */
void
write_ctl_bit(int byte_addr, int bit_addr, int data)
{
	volatile caddr_t	io;
	int			opri;
#define WRITE_CTL_BIT_1		0x10
#define WRITE_CTL_BIT_2		0x10

	opri = disable_lock(pm_md_data.pmdi.intr_priority, &(pm_md_data.lock));

	/*
	 * Set up addressability to PM Control port
	 */
	io = (volatile caddr_t)iomem_att(&(pm_md_data.io_map)) +
	    pm_md_data.pmdi.pmc_port1;

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 1st byte
	 */
	*io = (char) (WRITE_CTL_BIT_1 | ((byte_addr & 0x06) >> 1) |
	    ((data & 0x01) << 2));
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 2nd byte
	 */
	*io = (char) (WRITE_CTL_BIT_2 | ((byte_addr & 0x01) << 3) |
	    ((bit_addr & 0x07)));
	eieio();

	iomem_det((void *)io);

	unlock_enable(opri, &(pm_md_data.lock));

	return;
}
/*
 * NAME:  read_ctl_bit
 *
 * FUNCTION:  Read a Signetics control bit
 *
 * DATA STRUCTURES:
 *	byte_addr	- low 3 bits of byte address
 *	bit_addr	- bit address (3 bits)
 *
 * NOTES:
 *	Write b' 0  0  0  1  1  0 A5 A4  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1 A3 A2 A1 A0  (to PM Control port 0x82A)
 *	delay 100 usec
 *      Read PM Control port & 0x01
 *		where	A5 A4 A3 = low 3 bits of byte address
 *			A2 A1 A0 = bit address
 *
 * RETURN VALUE DESCRIPTION:
 *      Bit read is in low-order bit.
 */
int
read_ctl_bit(int byte_addr, int bit_addr)
{
	volatile caddr_t	io;
	int			rc;
	int			opri;
#define READ_CTL_BIT_1		0x18
#define READ_CTL_BIT_2		0x10

	opri = disable_lock(pm_md_data.pmdi.intr_priority, &(pm_md_data.lock));

	/*
	 * Set up addressability to PM Control port
	 */
	io = (volatile caddr_t)iomem_att(&(pm_md_data.io_map)) +
	    pm_md_data.pmdi.pmc_port1;

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 1st byte
	 */
	*io = (char) (READ_CTL_BIT_1 | ((byte_addr & 0x06) >> 1));
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 2nd byte
	 */
	*io = (char) (READ_CTL_BIT_2 | ((byte_addr & 0x01) << 3) |
	    ((bit_addr & 0x07)));
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Read the data.
	 */
	rc = (int)*io;

	iomem_det((void *)io);

	unlock_enable(opri, &(pm_md_data.lock));

	return rc;
}

/*
 * NAME:  write_host_pin
 *
 * FUNCTION:  Write a Signetics host pin
 *
 * DATA STRUCTURES:
 *	pin_addr	- pin address (4 bits)
 *	data		- one bit
 *
 * NOTES:
 *	Write b' 0  0  0  1  1  1  0  0  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1  0  0  1  d  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1 P3 P2 P1 P0  (to PM Control port 0x82A)
 *		where	d = data
 *			P3 P2 P1 P0 = pin address
 *
 * RETURN VALUE DESCRIPTION:
 *      None.
 */
void
write_host_pin(int pin_addr, int data)
{
	volatile caddr_t	io;
	int			opri;
#define WRITE_HOST_PIN_1	0x1c
#define WRITE_HOST_PIN_2	0x12
#define WRITE_HOST_PIN_3	0x10

	opri = disable_lock(pm_md_data.pmdi.intr_priority, &(pm_md_data.lock));

	/*
	 * Set up addressability to PM Control port
	 */
	io = (volatile caddr_t)iomem_att(&(pm_md_data.io_map)) +
	    pm_md_data.pmdi.pmc_port1;

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 1st byte
	 */
	*io = (char) (WRITE_HOST_PIN_1);
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 2nd byte
	 */
	*io = (char) (WRITE_HOST_PIN_2 | (data & 0x01));
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 3rd byte
	 */
	*io = (char) (WRITE_HOST_PIN_3 | (pin_addr & 0x0f));
	eieio();

	iomem_det((void *)io);

	unlock_enable(opri, &(pm_md_data.lock));

	return;
}

/*
 * NAME:  pulse_host_pin
 *
 * FUNCTION:  Pulse a Signetics host pin
 *
 * DATA STRUCTURES:
 *	pin_addr	- pin address (4 bits)
 *	data		- one bit
 *
 * NOTES:
 *	Write b' 0  0  0  1  1  1  0  0  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1  0  1  0  d  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1 P3 P2 P1 P0  (to PM Control port 0x82A)
 *		where	d = data
 *			P3 P2 P1 P0 = pin address
 *
 * RETURN VALUE DESCRIPTION:
 *      None.
 */
void
pulse_host_pin(int pin_addr, int data)
{
	volatile caddr_t	io;
	int			opri;
#define PULSE_HOST_PIN_1	0x1c
#define PULSE_HOST_PIN_2	0x14
#define PULSE_HOST_PIN_3	0x10

	opri = disable_lock(pm_md_data.pmdi.intr_priority, &(pm_md_data.lock));

	/*
	 * Set up addressability to PM Control port
	 */
	io = (volatile caddr_t)iomem_att(&(pm_md_data.io_map)) +
	    pm_md_data.pmdi.pmc_port1;

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 1st byte
	 */
	*io = (char) (PULSE_HOST_PIN_1);
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 2nd byte
	 */
	*io = (char) (PULSE_HOST_PIN_2 | (data & 0x01));
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 3rd byte
	 */
	*io = (char) (PULSE_HOST_PIN_3 | (pin_addr & 0x0f));
	eieio();

	iomem_det((void *)io);

	unlock_enable(opri, &(pm_md_data.lock));

	return;
}

/*
 * NAME:  read_host_pin
 *
 * FUNCTION:  Read a Signetics host pin
 *
 * DATA STRUCTURES:
 *	pin_addr	- pin address (4 bits)
 *
 * NOTES:
 *	Write b' 0  0  0  1  1  1  0  0  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1  1  0  0  0  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1 P3 P2 P1 P0  (to PM Control port 0x82A)
 *      delay 100 usec
 *      Read PM Control port & 0x01
 *
 *		where	P3 P2 P1 P0 = pin address
 *
 * RETURN VALUE DESCRIPTION:
 *      None.
 */
int
read_host_pin(int pin_addr)
{
	volatile caddr_t	io;
	int			rc;
	int			opri;
#define READ_HOST_PIN_1		0x1c
#define READ_HOST_PIN_2		0x18
#define READ_HOST_PIN_3		0x10

	opri = disable_lock(pm_md_data.pmdi.intr_priority, &(pm_md_data.lock));

	/*
	 * Set up addressability to PM Control port
	 */
	io = (volatile caddr_t)iomem_att(&(pm_md_data.io_map)) +
	    pm_md_data.pmdi.pmc_port1;

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 1st byte
	 */
	*io = (char) (READ_HOST_PIN_1);
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 2nd byte
	 */
	*io = (char) (READ_HOST_PIN_2);
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 3rd byte
	 */
	*io = (char) (READ_HOST_PIN_3 | (pin_addr & 0x0f));
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Read data bit
	 */
	rc = *io & 0x01;
	eieio();

	iomem_det((void *)io);

	unlock_enable(opri, &(pm_md_data.lock));

	return rc;
}

/*
 * NAME:  write_data_word
 *
 * FUNCTION:  Write a Signetics data word
 *
 * DATA STRUCTURES:
 *	byte_addr	- 8 bit address
 *	data		- 8 bit data
 *
 * NOTES:
 *	Write b' 0  0  0  1  1  1  0  1  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1 A7 A6 A5 A4  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1 A3 A2 A1 A0  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1  1  1  1  0  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1 D7 D6 D5 D4  (to PM Control port 0x82A)
 *	delay 100 usec
 *	Write b' 0  0  0  1 D3 D2 D1 D0  (to PM Control port 0x82A)
 *
 *		where	A7 A6 A5 A4 A3 A2 A1 A0 = address
 *		     	D7 D6 D5 D4 D3 D2 D1 D0 = data
 *
 * RETURN VALUE DESCRIPTION:
 *      None.
 */
void
write_data_word(int byte_addr, int data)
{
	volatile caddr_t	io;
	int			opri;
#define WRITE_WORD_ADDR_1	0x1d
#define WRITE_WORD_ADDR_2	0x10
#define WRITE_WORD_ADDR_3	0x10
#define WRITE_DATA_WORD_1	0x1e
#define WRITE_DATA_WORD_2	0x10
#define WRITE_DATA_WORD_3	0x10

	opri = disable_lock(pm_md_data.pmdi.intr_priority, &(pm_md_data.lock));

	/*
	 * Set up addressability to PM Control port
	 */
	io = (volatile caddr_t)iomem_att(&(pm_md_data.io_map)) +
	    pm_md_data.pmdi.pmc_port1;

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 1st byte
	 */
	*io = (char) (WRITE_WORD_ADDR_1);
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 2nd byte
	 */
	*io = (char) (WRITE_WORD_ADDR_2 | ((byte_addr >> 4) & 0x0f));
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 3rd byte
	 */
	*io = (char) (WRITE_WORD_ADDR_3 | (byte_addr & 0x0f));
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 1st byte
	 */
	*io = (char) (WRITE_DATA_WORD_1);
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 2nd byte
	 */
	*io = (char) (WRITE_DATA_WORD_2 | ((data >> 4) & 0x0f));
	eieio();

	/*
	 * Delay 100 usec
	 */
	io_delay(SIG_DELAY);

	/*
	 * Write 3rd byte
	 */
	*io = (char) (WRITE_DATA_WORD_3 | (data & 0x0f));
	eieio();

	iomem_det((void *)io);

	unlock_enable(opri, &(pm_md_data.lock));
}

/*
 * NAME:  write_port
 *
 * FUNCTION:  Write a 1 byte port
 *
 * DATA STRUCTURES:
 *	addr		- port address
 *	data		- 1 byte data
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 *      None.
 */
void
write_port(int addr, int data)
{
	volatile caddr_t	io;

	/*
	 * Set up addressability to port
	 */
	io = (volatile caddr_t)iomem_att(&(pm_md_data.io_map)) + addr;

	/*
	 * Write byte
	 */
	*io = (char) data;
	eieio();

	iomem_det((void *)io);

	return;
}
/*
 * NAME:  read_port
 *
 * FUNCTION:  Read a 1 byte port
 *
 * DATA STRUCTURES:
 *	addr		- port address
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 *      Byte read is in low-order byte.
 */
int
read_port(int addr)
{
	volatile caddr_t	io;
	int			rc;

	/*
	 * Set up addressability to port
	 */
	io = (volatile caddr_t)iomem_att(&(pm_md_data.io_map)) + addr;

	/*
	 * Read byte
	 */
	rc = (int)*io;
	eieio();

	iomem_det((void *)io);

	return rc;
}
