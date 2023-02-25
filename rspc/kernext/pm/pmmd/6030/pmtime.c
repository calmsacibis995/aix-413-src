static char sccsid[] = "@(#)98  1.4  src/rspc/kernext/pm/pmmd/6030/pmtime.c, pwrsysx, rspc41J, 9520A_all 5/12/95 12:48:40";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: adjust_rtc_chip,  Reinit_SysTime
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

#include <sys/types.h>		/* always needed			*/
#include <sys/adspace.h>        /* for the WRITE_CLOCK macro to work    */
#include <sys/time.h>		/* for the timeval structure		*/
#include <sys/param.h>		/* to define the HZ label		*/
#include <sys/mstsave.h>	/* mstsave area def. for asserting	*/
#include <sys/user.h>		/* the u structure to return errnos	*/
#include <sys/errno.h>		/* define the errno's to be returned	*/
#include <sys/intr.h>		/* for the serialization stuff		*/
#include <sys/rtc.h>		/* for real time clock related defines	*/
#include <sys/low.h>		/* access the csa for asserts		*/
#include <sys/machine.h>	/* for machine model macros		*/
#include <sys/systemcfg.h>	/* for system config structure          */
#include <sys/sys_resource.h>	/* for system resource structure        */
#include <sys/inline.h>		/* for eieio()				*/
#include <sys/system_rspc.h>	/* for IO addresses			*/
#include <sys/seg.h>		/* for the I/O segment register def.	*/

#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/pm.h>
#include "slave.h"
#include "../../pmmi/pmmi.h"


/*------------------------------*/
/*    proto type declaration    */
/*------------------------------*/
void adjust_rtc_chip();
void Reinit_SysTime();
void set_RTC_resume_time(struct tms *tm);
char read_rtc(int reg);



struct tms mytm;              /* date to secs conversion structure */
extern ISA_DATA    pm_RTC_data;      /* RTC access data block             */

 
#define	RTCclock	0
#define	RTCalarm	1
 	

/*
 * ------------------------------------------------------------------
 *  The following code is brought from ./kernel/proc/POWER/m_clock.c 
 * ------------------------------------------------------------------
 */
/*
 * NAME: write_rtc
 *
 * FUNCTION: write a byte to DS1285 real time clock chip 
 *
 * EXECUTION ENVIORNMENT:
 *	called by init_ds1285 and write_ds1285
 *
 *	caller should be disabled
 *
 * RETURNS: None
 */
void
write_rtc(int reg, char value)
{
   	volatile uchar  *io;

      	/*
         * I/O access start
         */
        io = (volatile uchar *)iomem_att(&(pm_RTC_data.map))
                                                + pm_RTC_data.base;
  	*((volatile uchar *)io) = reg;
	eieio();
        *((volatile uchar *)(io + 1)) = value;
	SYNC();

        /* I/O access end */
        iomem_det((void *)io);

	return;
}




/*
 * NAME update_rtc
 *
 * FUNCTION: update time in DS1285
 *
 * EXECUTION ENVIORNMENT:
 *	called by write_ds1285 and init_ds1285.  
 *
 * RETURNS:
 *	None
 */
void
update_rtc(int opt)
{
	/* write new time.  RTC_YEAR is the actual year of the century.
	 * When the value is greater than 0x30, overflow would occur, so
	 * we check for that here and wrap the year 2000 back to 0.
	 */
	if (mytm.yrs + 0x70 < 0xa0)
		write_rtc(RTC_YEAR, mytm.yrs + 0x70);
	else
		write_rtc(RTC_YEAR, mytm.yrs - 0x30);

	if (opt == RTCclock) {
		write_rtc(RTC_MONTH, mytm.mths);
		write_rtc(RTC_DOM, mytm.dom);
		write_rtc(RTC_HRS, mytm.hrs);
		write_rtc(RTC_MIN, mytm.mins);
		write_rtc(RTC_SEC, mytm.no_secs);
	} else {
#ifdef PM_DEBUG
    printf("RTC resume time is %X:%X:%X\n", mytm.hrs,mytm.mins,mytm.no_secs);
#endif
                write_rtc(RTC_SEC_ALARM, mytm.no_secs);
                write_rtc(RTC_MIN_ALARM, mytm.mins);
                write_rtc(RTC_HRS_ALARM, mytm.hrs);
#ifdef PM_DEBUG
       		printf("RTC alarm sec = %X\n", read_rtc(RTC_SEC_ALARM));
        	printf("RTC alarm min = %X\n", read_rtc(RTC_MIN_ALARM));
        	printf("RTC alarm hrs = %X\n", read_rtc(RTC_HRS_ALARM));
#endif
	} /* endif */
	
}


/*
 * NAME write_ds1285
 *
 * FUNCTION: Update time in DS1285 real time clock chip
 *
 * EXECUTION ENVIORNMENT:
 *	only called by write_clock
 *
 * RETURNS: None
 */
static void
write_ds1285(int opt)
{

	/* disable update of clock
	 */
	write_rtc(RTC_REGB, CRB_SET|CRB_24HR);

	update_rtc(opt);

	/* enable update of clock
	 */
	write_rtc(RTC_REGB, CRB_24HR+CRB_SQUE);
}


/*
 * NAME: write_clock
 *
 * FUNCTION: Update time in real time clock
 *
 * EXECUTION ENVIORNMENT:
 *	Caller should be disabled, or non-premptable.  This routine
 *	is not re-entrant
 *
 * RETURNS: None
 */
void
write_clock()
{
	struct   timestruc_t ct;
	
	/*
	 * Get the current time to set the clock.  Need to convert
	 * nanoseconds to hundred seconds.
	 */
	curtime(&ct);
	mytm.secs = ct.tv_sec;
	mytm.ms = ct.tv_nsec / (NS_PER_SEC/100); /* nano secs to 100th secs */
	secs_to_date(&mytm);
	write_ds1285(RTCclock);

}

/*
 * NAME: set_RTC_resume_time
 *
 * FUNCTION: Program RTC alarm field with user's specified time
 *
 * STRUCTURE: tms (<sys/rtc.h>)
 *
 * RETURN VALUE DESCRIPTION:
 *              none
 */
void   
set_RTC_resume_time(struct tms *Ptm)
{
	mytm.no_secs = Ptm->no_secs;
	mytm.mins = Ptm->mins;
	mytm.hrs = Ptm->hrs;
	write_ds1285(RTCalarm);
        return;
}


/*
 * ------------------------------------------------------------------
 *  The following code is brought from ./kernel/proc/POWER/gettod.c
 * ------------------------------------------------------------------
 */
#define  SECPERMINUTE    (60)
#define  SECPERHOUR      (SECPERMINUTE*60)
#define  SECPERDAY       (SECPERHOUR*24)
#define  SECPERYEAR      (SECPERDAY*365)
#define  NO_MONTHS       (12)
#define  DAYSPERYEAR     (365)
#define  MINPERHOUR      (60)
#define  HOURSPERDAY     (24)
#define  SECSTOYEAR(y)   (((y)*SECPERYEAR)+((((y)+1)/4)*SECPERDAY))

/* The following are used as array indices */
#define  JAN_INDEX   0
#define  FEB_INDEX   (JAN_INDEX + 1)

/* An array of the number of days per month. */
static char dpm[NO_MONTHS] = {
	31,  /* JAN */        /* Note: When used, a copy is */
	28,  /* FEB */        /*   made so that FEB can be  */
	31,  /* MAR */        /*   changed to 29 without    */
	30,  /* APR */        /*   affecting re-entrancy.   */
	31,  /* MAY */
	30,  /* JUN */
	31,  /* JUL */
	31,  /* AUG */
	30,  /* SEP */
	31,  /* OCT */
	30,  /* NOV */
	31   /* DEC */
};


/*
 * NAME:  date_to_secs
 *
 * FUNCTION: Convert MM DD YY to seconds since the Epoch.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called under a process at system initialization to
 *	restore the system time from the time-of-day chip.
 *
 * NOTES:  POSIX 1003.1 defines 'seconds since the Epoch' as "a value to be
 *	interpreted as the number of seconds between a specified time and
 *	the Epoch".  1003.1 further specifies that "a Coordinated Universal
 *	Time name . . . is related to a time represented as seconds since
 *	the Epoch according to the expression:
 *		tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
 *		(tm_year-70)*31536000 + ((tm_year-69)/4)*86400"
 */
date_to_secs(struct tms *timestruct)
{
	register int	year;
	register int	day;

	year = (((int)timestruct->yrs / 16 * 10) + ((int)timestruct->yrs % 16));
	day = ((int)timestruct->jul_100 * 100) + 
		((int)timestruct->jul_dig / 16 * 10) +
		((int)timestruct->jul_dig % 16);
	/*  Correct for julian day starting at 1 rather than 0.  */
	day--;
	timestruct->secs =	
		((int)timestruct->no_secs / 16 * 10) + 
		((int)timestruct->no_secs % 16) + 
		(((int)timestruct->mins / 16 * 10) * 60) +
		(((int)timestruct->mins % 16) * 60) +
		(((int)timestruct->hrs / 16 * 10) * 3600) +
		(((int)timestruct->hrs % 16) * 3600) +
		(day * 86400) +
		(year * 31536000) +
		(((year + 1)/4) * 86400);
	timestruct->ms = (int)(timestruct->ms / 16 * 10) +
		(int)(timestruct->ms % 16);
}

/*
 * NAME:  secs_to_date
 *
 * FUNCTION: Convert seconds since the Epoch to MM DD YY.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called under a process from settimer() to save the
 *	system time to the time-of-day chip.
 *
 * NOTES:  POSIX 1003.1 defines 'seconds since the Epoch' as "a value to be
 *	interpreted as the number of seconds between a specified time and
 *	the Epoch".  1003.1 further specifies that "a Coordinated Universal
 *	Time name . . . is related to a time represented as seconds since
 *	the Epoch according to the expression:
 *		tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
 *		(tm_year-70)*31536000 + ((tm_year-69)/4)*86400"
 */
secs_to_date(register struct tms *timestruct)
{
	register unsigned int    i, tempsecs;
	char     tempdpm[NO_MONTHS];

	/* Make a copy of the days per month structure for re-entrancy.*/
	for (i=0; i < NO_MONTHS; i++)  {
		tempdpm[i] = dpm[i];
	}

	/*  Make a copy of the number of seconds since the Epoch.  */
	tempsecs = timestruct->secs;

	/*  Initialize all the values.  */
	timestruct->yrs = 0;
	timestruct->mths = timestruct->dom = 0;
	timestruct->hrs = timestruct->mins = timestruct->no_secs = 0;
	timestruct->jul_100 = timestruct->jul_dig = 0;

	/*
	 * Calculate the current year.  We guess at the year by assuming
	 * there are no leapyears.  If the number of seconds in all the
	 * years up to this point is more than the number of seconds I
	 * am converting, I must have guessed wrong, so I take off the
	 * year I shouldn't have added in.
	 */

	timestruct->yrs = tempsecs / SECPERYEAR;
	if ((i = SECSTOYEAR(timestruct->yrs)) > tempsecs) {
		timestruct->yrs--;
		i = SECSTOYEAR(timestruct->yrs);
	}
	tempsecs -= i;

	/*  
	 * Correct days per month for February if this is a leap year.  
	 * Add 2 for base of 68.  A leap year. 
	 */
	if(!((timestruct->yrs+2) % 4))  {
		tempdpm[FEB_INDEX] = 29;
	}

	/*  Calculate the current month.  */
	while(tempsecs >= (tempdpm[timestruct->mths] * SECPERDAY))  {
		tempsecs -= (tempdpm[timestruct->mths] * SECPERDAY);
		if(((int)timestruct->jul_dig + tempdpm[timestruct->mths]) > 99){
			timestruct->jul_100++;
			timestruct->jul_dig = (int)timestruct->jul_dig +
					      tempdpm[timestruct->mths] - 100;
		}
		else  {
			timestruct->jul_dig += tempdpm[timestruct->mths];
		}
		timestruct->mths++;
	}
	timestruct->mths++;

	/*  Calculate the current day.  */
	timestruct->jul_dig += timestruct->dom = tempsecs / SECPERDAY;
	tempsecs %= SECPERDAY;
	timestruct->dom++;
	timestruct->jul_dig++;
	if(timestruct->jul_dig > 99)  {
		timestruct->jul_100++;
		timestruct->jul_dig -= 100;
	}

	/*  Calculate the current hour.  */
	timestruct->hrs = tempsecs / SECPERHOUR;
	tempsecs %= SECPERHOUR;

	/*  Calculate the current minute.  */
	timestruct->mins = tempsecs / SECPERMINUTE;
	tempsecs %= SECPERMINUTE;

	/*  The remainder is the current seconds.  */
	timestruct->no_secs = tempsecs;

	/*  Convert to Binary Coded Decimal.  */
	timestruct->ms = (int)timestruct->ms / 10 * 16 +
				((int)timestruct->ms % 10);
	timestruct->mins = (int)timestruct->mins / 10 * 16 +
				((int)timestruct->mins % 10);
	timestruct->hrs = (int)timestruct->hrs / 10 * 16 +
				((int)timestruct->hrs % 10);
	timestruct->dom = (int)timestruct->dom / 10 * 16 +
				((int)timestruct->dom % 10);
	timestruct->mths = (int)timestruct->mths / 10 * 16 +
				((int)timestruct->mths % 10);
	timestruct->yrs = (int)timestruct->yrs / 10 * 16 +
				((int)timestruct->yrs % 10);
	timestruct->no_secs = (int)timestruct->no_secs / 10 * 16 + 
				((int)timestruct->no_secs % 10);
	timestruct->jul_dig = (int)timestruct->jul_dig / 10 * 16 + 
				((int)timestruct->jul_dig % 10);
}


/*
 * ------------------------------------------------------------------
 *  The following code is brought from ./kernel/proc/POWER/m_tinit.c
 * ------------------------------------------------------------------
 */

/*
 * NAME: read_rtc
 *
 * FUNCTION: read a byte from the DS1285 real time clock chip 
 *
 * EXECUTION ENVIORNMENT:
 *	called by init_ds1285
 *
 *	caller should be disabled
 *
 * RETURNS: Byte read
 */
char
read_rtc(reg)
int reg;
{
   	volatile uchar  *io;
	char byte;

      	/*
         * I/O access start
         */
        io = (volatile uchar *)iomem_att(&(pm_RTC_data.map))
                                                + pm_RTC_data.base;
  	*((volatile uchar *)io) = reg;
	eieio();
        byte = *((volatile uchar *)(io + 1));

        /* I/O access end */
        iomem_det((void *)io);

	return(byte);
}


/*
 * NAME: date_to_jul
 *
 * FUNCTION: converts month, day of month, and year to julian date
 *
 * NOTES:
 *	Only the yrs mths and dom fields of the tms structure need be
 *	filled out.
 *
 *	The jul_100 and jul_dig will be set
 *
 * EXECUTION ENVIORNMENT:
 *	called only by init_ds1285
 *
 *	Only called once.
 *
 * RETURNS: None
 */
void
date_to_jul(tmblk)
struct tms *tmblk;
{
	int jday;
	int month;
	int year;
	int i;
	static char days[12] = {31 /* JAN */, 28 /* FEB */, 31 /* MAR */,
			   30 /* APR */, 31 /* MAY */, 30 /* JUN */,
			   31 /* JUL */, 31 /* AUG */, 30 /* SEP */,
			   31 /* OCT */, 30 /* NOV */, 31 /* DEC */ };

	/* convert BCD format years into a decimal years
	 */
	year = (tmblk->yrs / 16 ) * 10 + (tmblk->yrs % 16);

	/* check if the year is a leap year.  Year 0 is 1970 so 2 must
	 * be added to year before checking for leap year.  If it is
	 * a leap year adjust Feb.'s days to 29
	 */
	if (((year + 2) % 4) == 0)
		days[1] = 29;

	/* convert day of month and day to a julian date
	 */
	jday = (tmblk->dom / 16) * 10 + (tmblk->dom % 16);
	month = (tmblk->mths / 16) * 10 + (tmblk->mths % 16);
	for (i = 0; i < (month - 1); i++)
		jday += days[i];

	/* store away the julian date in BCD
	 */
	tmblk->jul_100 = jday / 100;
	jday %= 100;
	tmblk->jul_dig = (jday / 10) * 16 + (jday % 10);
}

/*
 * NAME:  init_ds1285
 *
 * FUNCTION:  Setup the DS1285 time-of-day chip 
 *
 * RETURNS: None
 */
static void
init_ds1285()
{
	int oldpri;
	int addr;

	/* check if the current contents of the real time clock are valid,
	 * and that the time was not being set when the machine went down
	 */
	if ( (read_rtc(RTC_REGD) & CRD_VRT) &&
	     (read_rtc(RTC_REGB) & CRB_24HR) &&
	     ((read_rtc(RTC_REGA) & (CRA_DV2+CRA_DV1+CRA_DV0)) == CRA_DV1) )
	{
		/* Sync time read with transition in seconds.  First wait
		 * for update in progress to be set.  The read time after
		 * it is clear.  This allows tm.ms to be 0
		 */
		while(!(read_rtc(RTC_REGA) & CRA_UIP));
		while(read_rtc(RTC_REGA) & CRA_UIP);

		/* Read date and time from real time clock.  The "year"
		 * may be offset by 0x70.  If the "year" is less that 0x70,
		 * it must be in the next century, so that adjustment is
		 * made.  This code breaks in 2069.
		 */
		mytm.yrs = read_rtc(RTC_YEAR);
		if (mytm.yrs >= 0x70)
			mytm.yrs -= 0x70;
		else
			mytm.yrs += 0x30;

		mytm.dom = read_rtc(RTC_DOM);
		mytm.mths = read_rtc(RTC_MONTH);
		mytm.hrs = read_rtc(RTC_HRS);
		mytm.mins = read_rtc(RTC_MIN);
		mytm.no_secs = read_rtc(RTC_SEC);
		mytm.ms = 0;

                /* this chip has no julian date so calculate it
                 */
                date_to_jul(&mytm);
	}
	else
	{
		/* set the date counter to the Epoch using BCD
		 */

		/* shut off oscillator
		 */
		write_rtc(RTC_REGA, 0);

		/* Turn on set bit (to set time) and configure to 24 hour
		 * mode to write date
		 */
		write_rtc(RTC_REGB, CRB_SET|CRB_24HR);

		/* zero all the time registers
		 */
		for (addr = RTC_SEC; addr < RTC_REGA; addr++)
			write_rtc(addr, 0);

		/* zero all the ram locations
		 */
		for (addr = RTC_RAMSTART; addr <= RTC_RAMEND; addr++)
			write_rtc(addr, 0);

		/* initialize day of week.  This value is not used by
		 * the system
		 */
		write_rtc(RTC_DAY, 0x01);

		/* initialize year to 0x70 for leapyear calculation
		 * on the RTC chip.
		 */
		write_rtc(RTC_YEAR, 0x70);

		/* set the in memory date
		 */
		mytm.yrs = 0x00;
		mytm.jul_100 = 0x00;
		mytm.jul_dig = 0x01;
		mytm.mths = 0x01;
		mytm.dom = 0x01;
		mytm.hrs = 0x00;
		mytm.mins = 0x00;
		mytm.no_secs = 0x00;
		mytm.ms = 0x00;

		/* set time of day on the clock
		 */
		update_rtc(RTCclock);

		/* enable the oscilator and start the clock.  Register C
		 * is read-only
		 */
		write_rtc(RTC_REGA, CRA_DV1);
		write_rtc(RTC_REGB, CRB_24HR+CRB_SQUE);
	} /* endif */

} /* init_ds1285 */



/*
 * NAME: adjust_rtc_chip
 *
 * FUNCTION: reflect the current processor time to rtc chip because 
 *	     the time maintained by processor clock is more accurate 
 *	     than one by rtc chip.
 * 
 * INPUT: none
 *
 * OUTPUT: none
 *
 */
void
adjust_rtc_chip()
{
	struct timestruc_t	ct,bt;

      	curtime(&bt);
#ifdef PM_DEBUG
	printf("Adjusted handled seconds = %X\n",bt.tv_sec);
#endif
        do {
                curtime(&ct);
        } while ( bt.tv_sec == ct.tv_sec );
        write_clock();
	return;
}



/*
 * NAME:  init_clock
 *
 * FUNCTION:  Initialize all clocks by:
 * 	Initialize non-volatile time of day chip. 
 * 	Use the time saved in the non-volatile clock to init processor clock.
 * 	Initialize the memory mapped time variables.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called at resume from suspend or hibernation.
 *
 * EXTERNAL PROCEDURES CALLED: init_ds1285, ksettimer, data_to_secs
 */
void Reinit_SysTime()
{
	struct timestruc_t nct;

	init_ds1285();
	date_to_secs(&mytm);
	nct.tv_sec = mytm.secs;
#ifdef PM_DEBUG
	printf("Reinited handled seconds = %X\n",nct.tv_sec);
#endif
	nct.tv_nsec = 0; 
	ksettimer(&nct); /* kernel static: tm will be updated, too. */
	return;
}

/* ------------------------ End of file --------------------*/
