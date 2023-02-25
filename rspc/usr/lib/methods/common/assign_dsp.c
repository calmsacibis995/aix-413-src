static char sccsid[] = "@(#)29	1.1  src/rspc/usr/lib/methods/common/assign_dsp.c, dispcfg, rspc41J, 9516B_all 4/18/95 09:51:43";
/*
 *
 * COMPONENT_NAME: (dispcfg) generic function 
 *
 * FUNCTIONS:  assign_dsp 
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

#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"

/*---------------------------------------------------------------------------------
 |  
 |  This function should be called by each configuration method of the PCI graphics
 |  device drivers , except for Icecube (s3/864).  We don't care about the Icecube 
 |  because there can only be one device in a  system.  For PowerPC boxes such as 
 |  Carolina, Delmar, Southwind, etc, the Icecube is in on the planar so there can't 
 |  be a second device installed.  For other PCI graphics devices such as Baby Blue, 
 |  Glacier, etc, which are plugged into the PCI bus, there is a concern that when 
 |  users install these adapters they want to use them instead of the the Icecube 
 |  on the planar.  Feature 172304 has been opened to address this issue.  This code 
 |  is implementation of the feature.  The main logic is as follows:  
 |
 |  if (this is the first time the device is configured) AND 
 |     (device is not a planr graphics                 )
 |  {
 |     if (console is defined to tty)
 |     {
 |        if (default display is planar device)
 |            change default display to the new device.  The rational is
 |            that the new device is installed because users want to use it.
 |     }
 |     else
 |     {
 |        remove console attr to force console finder so that the next IPL 
 |        the system will prompt users to specify which display(s) to be the console. 
 |     }       
 |  }
 |
 |
 |  The following test is carried out to determine if a graphics device is on the
 |  planar.  The "location" field in each CuDv contains a string in this format:  
 |  "xx-yy".  The 2 digit "xx" represents the bus type.  For PCI bus, it is always 
 |  "04".  The 2 digit "yy" represents the slot number.  The string of "00" means 
 |  this PCI device is on the planar and a string of non-zero means the slot where
 |  the device is connected.  
 |
 |  Note the value stored in the "connwhere" field of the CuDv is also referred to
 |  as the slot number by graphics device's configuration code.  However this slot
 |  number is deferent from the slot number in the location code.
 |
 |  We don't use the bus type field in the "location" field of the CuDv because we 
 |  know that only PCI graphics devices' configuration methods invoke the 
 |  "assign_dsp" function. 
 |
 ---------------------------------------------------------------------------------*/

assign_dsp(lname, location_slot)

   char * lname;                 /* logical device name  */

   char * location_slot;         /* slot number from "location" code, 
                                    not "connwhere" value. 
                                  */

{
	struct CuAt * p_cuat;
	struct CuDv   cudv;
	int rc,val, slot_number;	
	
	char def_console_dev[30],      /* default console device -- null, 
                                                                   "/dev/tty0",  
                                                                   "/dev/lft0" 
                                        */ 

	     def_dsp[30],               /* default display -- some display or null */ 

	     crit[50];	


	rc = val = 0;

	DEBUG_2("enter assign_dsp: lname = %s location slot=%s\n",lname,location_slot);

	def_console_dev[0] = '\0';
	def_dsp[0] = '\0';

        p_cuat = (struct CuAt *)get_attrval(lname,"cfg_first_time",(char *)NULL,&val,&rc);
        if( p_cuat == NULL)
        {
           DEBUG_1("assign_dsp: device %s does not have cfg_first_time attr\n", lname);
	   goto done;
        }

        DEBUG_1("assign_dsp: cfg_first_time = %d\n",val);

        /* non-zero means this device is configured for the very first time.
         * Now it is being configured.  We set the flag to zero so that
         * when the next time the machine is reboot, we know it has been 
         * configured at least once
         */ 
	if (val != 0)
 	{
         	sprintf(p_cuat->value,"0");
                if ((rc = putattr(p_cuat)) < 0)
                {
                   DEBUG_0("assign_dsp: failed to change cfg_first_time attr\n");
		   goto done;
                }
	}
	
	/* 
         * EXPECTING SLOT NUMBER TO BE IN BASE 10.  If not we are in trouble!!!
         */

	slot_number = strtoul(location_slot,NULL,10);   /* convert to numeric value */

        /*  if device is configured for the very first time and
	 *     not planar device (i.e.,  slot != 0) 
         */

	DEBUG_2("assign_dsp: location_slot =%d, first time =%d\n",slot_number, val);

	if (val != 0  && slot_number != 0) 
	{
           p_cuat = (struct CuAt *)get_attrval("sys0","syscons",(char *)def_console_dev,NULL,&rc);

           if(p_cuat == NULL)
           {
              DEBUG_0("assign_dsp: failed to get syscons attr\n");
	      goto done;
           }

	   DEBUG_1("assign_dsp: default syscon = <%s>\n",def_console_dev);

	   val = strlen(def_console_dev);   /* if length == 0, console is not selected */ 

	   if (val && (strncmp("/dev/tty",def_console_dev,8) == 0))       /* console is tty */	
	   {
              /* Get the name of the default display.  Notice "lft0" is 
               * used in the search and the LFT is usually configured after
               * graphics devices.  This query could fail becuase the there is
               * no CuDv for LFT.  This happens when the machine is installed so
               * we don't care.  There is nothing to do in this situation.  We 
               * mainly concern with someone installing additional graphics 
               * devices to an installed system afterward 
               */

              p_cuat = (struct CuAt *)get_attrval("lft0","default_disp",(char *)def_dsp, NULL,&rc);

	      if (p_cuat == NULL)
	      {
		 DEBUG_0("assign_dsp: not default display -- probably lft0 not defined yet\n");
		 rc = 0;               /* this is not an error */
	      }
	      else if (p_cuat)   /* default display is assigned to some display */
	      {
	         /* Get CuDv of this defualt display.  Check its slot number.
                 *  A non-zero slot number means the device is not on the planar
                 *  and a zero slot number means the device is on the planar.
	         */

	         if (strlen(def_dsp))   /* is some display assigned to be the default display?*/
	         {
		    DEBUG_1("assign_dsp: default display =%s\n",def_dsp);

		    sprintf(crit,"name=%s",def_dsp);
                    if ((rc = (int)odm_get_first(CuDv_CLASS, crit, &cudv)) == 0) 
		    {
		       DEBUG_1("found no CuDv object for crit=%s\n", crit);
	            }
		    else if (rc == -1)
		    {
		       DEBUG_1("failed to retrieve CuDv object for crit=%s\n", crit);
		    }
		    else /* found CuDv of the default display */
		    {
		       rc = 0;

		       /* 
                          "location" code is of the form: "xx-yy".  The "xx" is the bus type
                          and "yy" is the slot number where the device is plugged into.

			  PCI bus adapters have value of "04" for the bus type.  Slot value of
                          zero means the device is on the system planar.

                        */

	               sprintf(location_slot,"%s",cudv.location+3);   /* get slot number */ 
		       DEBUG_1("assign_dsp: location code of default display %s\n",cudv.location); 

		       /* 
         		* EXPECTING SLOT NUMBER TO BE IN BASE 10.  If not we are in trouble!!!
         		*/

	               slot_number =  strtoul(location_slot,NULL,10);   /* get slot number */ 

		       DEBUG_1("assign_dsp: location slot of default display %d\n",slot_number); 

		       if (slot_number == 0)      /* default graphics device is on planar */
		       {
			   strcpy(p_cuat->value,lname);
                	   if ((rc = putattr(p_cuat)) < 0)
                	   {
                   	      DEBUG_0("assign_dsp: failed to set new default display\n");
                	   }
		       }
		    }
	         }
		 else
		 {
   		    DEBUG_0("assign_dsp: no display assigned as default display\n");
		 }
	      }
	   }
	   else if (val && (strncmp("/dev/lft",def_console_dev,8) == 0))  /* console is lft */	
	   {
	     	p_cuat->value[0] = '\0';           /* remove default console device attribute */ 
	        if ((rc = putattr(p_cuat)) < 0)
                {
                   DEBUG_0("assign_dsp: failed to zero out CuAt for default console\n");
		}
	   }
	}

done:

	DEBUG_1("exit assign_dsp rc=%d\n",rc);

	return(rc);
}
