/* @(#)70       1.6  src/rspc/kernext/pm/pmmd/6030/slave.h, pwrsysx, rspc41J, 9517A_all 4/24/95 08:46:25 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Power Management interrupt(IRQ11) header file
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

#ifndef _H_SLAVE
#define _H_SLAVE

#ifndef  true
#define  true     1
#endif
#ifndef  false
#define  false    0
#endif

/*
 *******************************************************
 * Debug purpose                                       *
 *******************************************************
 */

/*
 * #define  pmdebug
 */

#ifdef   pmdebug
#define  DbgPMint(dbg_msg) printf("Slave I/F(PM int):   dbg_msg\n");
#define  DbgKproc(dbg_msg) printf("Slave I/F(Kproc):    dbg_msg\n");
#define  DbgKmode(dbg_msg) printf("Slave I/F(Kmode):    dbg_msg\n");
#define  DbgDump(dbg_msg) printf("Slave I/F:    dbg_msg\n");
#else
#define  DbgPMint(dbg_msg) ;
#define  DbgKproc(dbg_msg) ;
#define  DbgKmode(dbg_msg) ;
#define  DbgDump(dbg_msg)  ;
#endif

#define  IRR            1
#define  ISR            2
#define  PICMASK        3


/*
 *******************************************************
 * RTC I/O offset and essential control flags          *
 *******************************************************
 */
#define     RTC_Status_regA      0x0a
#define        RTC_CountDownChain      0x40
#define        RTC_Osc_enable          0x20
#define        RTC_BankSelect          0x10
#define        RTC_RegA_modifiedBits ( RTC_CountDownChain+     \
                                       RTC_Osc_enable+         \
                                       RTC_BankSelect     )
#define     RTC_Status_regB      0x0b
#define        RTC_SQWE                0x08
#define     Ext_Control_reg4A    0x4a

#define     Ext_Control_reg4B    0x4b
#define        AuxiliaryBatteryEnable  0x80
#define        Enable32kSQWoutput      0x40



/*
 *******************************************************
 * Slave CPU interface register sub-address(index)     *
 *******************************************************
 */
#define slave_BCR          0x00000001     /* basic status register     */
#define slave_SCR          0x00000002     /* system control register   */
#define slave_SDR          0x00000003     /* system data register      */
#define slave_UCR          0x00000004     /* slave command register    */
#define slave_USR          0x00000005     /* slave status register     */
#define slave_UDR0         0x00000008     /* slave data register 0     */
#define slave_UDR1         0x00000009     /* slave data register 1     */
#define slave_UDR2         0x0000000A     /* slave data register 2     */
#define slave_UDR3         0x0000000B     /* slave data register 3     */
#define PMMR1              0x00000019     /* Keyboard ID register      */


/*
 *****************************
 * Slave CPU(slave) BCR      *
 *****************************
 */
#define slave_interface_reset 0x00000001
#define slave_int_enable    0x00000004


/*
 *****************************
 * Slave CPU(slave) BSR      *
 *****************************
 */
#define slave_busy          0x00000001
#define slave_status_ready  0x00000002
#define slave_interrupt     0x00000040


/*
 **************************************
 * slave interface hardware command   *
 **************************************
 */
#define slave_EOI           0x02
#define Clear_Data_FIFO     0x03

/*
 **************************************
 * Message of slave action manager    *
 **************************************
 */
#define action_manager      0x20
/* inbound message */
#define SetOnLineEventFlag  0x10
#define SetOffLineEventFlag 0x11
#define EnableEventNotice   0x12
#define SetPowerOffWatchDog 0x13
#define ResetAction         0x14
/* outbound message */
#define NoMoreEvent         0x10


/*
 *************************************
 * Message of slave State manager    *
 *************************************
 */
#define StateManager        0x40
/* inbound message */
#define StartSuspend        0x10
#define CompSuspend         0x11
#define StartResume         0x12
#define CompResume          0x13
#define MainPowerOff        0x14


/*
 *************************************
 * Message of slave HOST IF manager  *
 *************************************
 */
#define HOST_IF_Manager     0x41
/* inbound message */
#define UnFreezeOthers      0x10
#define FreezeOthers        0x11
/* outbound message */
#define InvalidObjectID     0xFE


/*
 *************************************
 * Message of Revision manager       *
 *************************************
 */
#define Revision_Manager    0x43
/* inbound message */
#define QueryVersion        0x10
/* outbound message */
#define VersionInformation  0x10

/*
 ************************************
 * Message of Battery device        *
 ************************************
 */
#define  Battery_Device     0x87
/* inbound message */
#define EnabelRefresh       0x10
#define DisableRefresh      0x11
#define QueryBatteryStatus  0x12
#define QueryTotalCapacity  0x13
#define QueryCurrentCapacity 0x14
#define DischargedCapacity  0x15
#define QueryDischargedTime 0x16
#define QueryRawVI          0x17
#define QueryErrorStatus    0x18
#define SetBatteryNoticeTime 0x1b
/* outbound message */
#define BatteryStatusReport 0x11
#define TotalCapacityReport 0x12
#define CurrentCapacityReport 0x13
#define DischargedCapacityReport 0x14
#define DischargedTimeReport 0x15
#define RawVIreport         0x16


/*
 ************************************
 * For "SetBatteryNoticeTime        *
 ************************************
 */
#define  Normal60sec    1
#define  Early120sec    2
#define  Early240sec    3
#define  Early360sec    4
#define  Early480sec    5


/*
 ************************************
 * Message of Beep device           *
 ************************************
 */
#define Beep_Device         0x83
#define Make_beep           0x10
#define     Low_battery_warning  1
#define     Low_battery_Beep     2
#define     Suspend_Beep         3
#define     Partial_Suspend_Beep 4
#define     Resume_Beep          5
#define     DC_IN_OUT_Beep       6
#define     Power_Off_Beep       7
#define     WarningInOffLine     8
#define     Battery_Error        10
#define     F880Hz               11
#define     F315Hz               12
#define     F315Hzx2             13
#define     F315Hzx3             14
#define     ONE_SHOT             0
#define StopRepeatedBeep   0x11
#define EnableFuelRelated  0x12
#define DisableFuelRelated 0x13
#define EnableAllBeep      0x14
#define DisableAllBeep     0x15


/*
 ************************************
 * Message of Power LED device      *
 * Message of Suspend LED device    *
 ************************************
 */
#define  Power_LED_Device      0x90
#define  Suspend_LED_device    0x91
/* Inbound message             */
#define  TurnOnLED             0x10
#define  TurnOffLED            0x11
#define  BlinkLED              0x12
#define  BlinkFrequency_05Hz   0x01
#define  BlinkFrequency_025Hz  0x02
#define  BlinkFrequency_01251Hz 0x03
#define  BlinkFrequency_1Hz    0x81
#define  BlinkFrequency_4Hz    0x82
#define  BlinkFrequency_8Hz    0x83


/*
 ***********************************
 * Message of Power monitor device *
 ***********************************
 */
/* Inbound message             */
#define  QueryExternalPower    0x10h   /* not used */
/* outbound message             */
#define  ExernalPowerStatusReport 0x11


/*
 ***********************************
 * Common Message                  *
 ***********************************
 */
/* outbound message             */
#define  EventNotification     0x10


/*
 ***********************************
 * Slave CPU device event request  *
 ***********************************
 */
#define     Event_Status_reset 0x80    /* Only supported at final version */
#define  MainPowerDevice     0x80
#define     power_off_request  0x01
#define  KeyboardDevice      0x85
#define     KBD_any_key        0x01
#define     ext_KBD_plug_in    0x02
#define     ext_KBD_plug_out   0x04    /* Only supported at resume and init */
#define     ext_KBD_off_line   0x08
#define  KBDcoverDevice      0x8C
#define     KBDclose_notice    0x01
#define     KBDopen_notice     0x02
#define  LIDSwitchDevice     0x8D
#define     LIDopen_notice     0x01
#define     LIDclose_notice    0x02
#define  PointD_ClickDevice  0x8E
#define     Click_released     0x01
#define     Click_depressed    0x02
#define  PowerMonitorDevice  0x96
#define     low_battery        0x08
#define     early_low_battery  0x10
#define	    SuspendToHiber     0x40
#define     AC_mode            0x80
#define     DC_mode            0x01


/*
 ***********************************
 *  Internal PM state              *
 ***********************************
 */
#define  operational_state          0
#define  partial_suspend_state      1
#define  full_suspend_state         2
#define  hibernation_state          3


/*
 ***********************************
 *  Transmission data block        *
 ***********************************
 */
#define build_slave_block(whom, msg, parm0, parm1, parm2)    \
               Pslave_Txblock->TRx_whom = whom;              \
               Pslave_Txblock->TRx_msg = msg;                \
               Pslave_Txblock->TRx_para0 = parm0;            \
               Pslave_Txblock->TRx_para1 = parm1;            \
               Pslave_Txblock->TRx_para2 = parm2;


/*
 ***********************************
 *  Slave I/F register IO mapping  *
 ***********************************
 */
typedef struct _pm_ISA_data {
        Simple_lock     lock;
        int             base;
        int             bus_id;
        struct io_map   map;
} ISA_DATA;


/*
 ***********************************
 *
 * Communication block between private
 *    kernel process and PM interrupt
 *
 ***********************************
 */
typedef  struct  _Event_Control {
   int   pmeventkprocpid;           /* Kproc ID: PM event kproc(private)   */
   int   EventAssertion;            /* sync between intr & kproc           */
   int   battery_status_ready;      /* sync between battery syscall & kproc*/
   int   internal_state;            /* Current internal PM state           */
   int   ExternalKBD;               /* External keyboard is attached.      */
   int   ACDCmode;                  /* AC: 1, DC: 0                        */
   int   H8cmdSend;                 /* Request to send H8 cmd from PM int  */
   int   mainpw_failsafe;           /* For main power sw fail safe         */
   int   pmmd_kproc_running;	    /* pmmd kproc run flag 		   */
   int	 KillPMMD_Kproc;	    /* unconfig pmmd request		   */
   int   in_sleep;		    /* pmmd kproc is in e_sleep 	   */	
} EVENTCONTROL;

#define  AC    1                    /* For ACDCmode of EVENTCONTROL */
#define  DC    0                    /* For ACDCmode of EVENTCONTROL */

#define	 on_run	1		    /* For pmmd kproc running       */


typedef struct _PMeventCell {
   volatile int   EventType;        /* PM event type     */
   volatile int   EventDetail;      /* PM event detail   */
} PMEVENTCELL;


/*
 ***********************************
 * Request to send H8 command in
 *  kernel process instead of interrupt
 ***********************************
 */

#define  SuppressKBevent         1     /* Disable any key resume (partial)    */
#define  EnableExtKBDactivity    2     /* Enable any key resume (partial)     */
#define  DisRsmByExtKBDactivity  3     /* Disable external kbd resume(full)   */
#define  DisPDclickresume        4     /* Disable PD click depress resume     */
#define  ReqKBDcoverBeep         5     /* Request to generate kbd cover open  */
#define  ReqStopBeep             6     /* Request to stop the current beep    */
#define  OnceDisablePluginNotice 7     /* Disable ext-KBD plug-in notice once */
#define  ReqAC_inoutBeep         8     /* AC/DC transition beep               */
#define  ReqStopBeepForKBDclose  9     /* Request to beep during partial susp */


/*
 ***********************************
 *    PM interrupt Event type
 ***********************************
 */
#define no_event_occurrence   0        /* no event                            */
			               
#define MonitorSQW            2        /* Use for monitoring SQW availability */
#define RTS_batteryStatus     3        /* Use for new battery status obtained */
#define ExternalEvent         4        /* Use for notifying external PM event */


/*
 *************************************
 **   PM event detail                *
 *************************************        suspend trigger resume trigger
 *#define PM_EVENT_NONE               0            N/A            N/A
 *#define PM_EVENT_LID_OPEN           1            no             yes
 *#define PM_EVENT_LID_CLOSE          2            yes            no
 *#define PM_EVENT_MAIN_POWER         3            yes            yes
 *#define PM_EVENT_PD_CLICK           4            no             yes
 *#define PM_EVENT_KEYBOARD           5            no             yes
 *#define PM_EVENT_LOW_BATTERY        6            yes            no
 *#define PM_EVENT_CRITICAL_LOW_BATTERY  7         no             yes
 *#define PM_EVENT_RING               8            no             yes
 *#define PM_EVENT_RESUME_TIMER       9            no             yes
 *#define PM_EVENT_SYSTEM_IDLE_TIMER  10           yes            no
 *#define PM_EVENT_SOFTWARE_REQUEST   11           yes            no
 *#define PM_EVENT_SUSPEND_TIMER      12           yes            no
 *#define PM_EVENT_DISPLAY_MESSAGE    13           N/A            N/A
 *#define PM_EVENT_DATA_CHANGE        14           N/A            N/A
 *#define PM_EVENT_AC                 15           N/A            N/A
 *#define PM_EVENT_DC                 16           N/A            N/A
 */


/*
 ***********************************
 *  Speaker mute for 6030
 ***********************************
 */
#define MUTE_ON   1
#define MUTE_OFF  0

#define PORT_AUDIOINDEX    0x830
#define PORT_AUDIODATA     0x831
#define BITAUDIOINDEX      0X0FFFFFFFF
#define PINCONTROLREG      0x00000000a
#define BITXCTL0           0x000000040


/*
 ***********************************
 *  Slave CPU communication buffer *
 ***********************************
 */
#define  NoExternalAttached      0
#define  ExternalAttached        1


/*
 ***********************************
 *  Slave CPU communication buffer *
 ***********************************
 */
typedef struct _slave_block {
        uchar  TRx_whom;
        uchar  TRx_msg;
        uchar  TRx_para0;
        uchar  TRx_para1;
        uchar  TRx_para2;
} SLAVE_BLOCK;


/*
 ***********************************
 *  PM Battery control             *
 ***********************************
 */
typedef struct _battery_ctrl {
        Simple_lock lock;
        volatile int    battery_count;
        volatile int    sig_unconfig;
	volatile int	On_going_flags;
} BATTERY_CTRL;

#define	battery_query	1


/*
 ***********************************
 *  Battery status block for query *
 ***********************************
 */
/* Bit field format of H8 transferred data */
#define slave_bat_attached    0x0001      /* Attached: 1         */
#define slave_bat_NiMH        0x0002      /* NiCd: 0, NiMH: 1    */
#define slave_bat_charge      0x0004      /* Being charged: 1    */
#define slave_bat_discharge   0x0008      /* Being discharged: 1 */
#define slave_bat_dischg_req  0x0010      /* Discharge request from H8: 1 */

/* Bit field format of high layer */

      /* Refer to pm.h */
#define battery_dischg_req    0x00008000  /* Discharge request from H8: 1 */


typedef struct battery_data_from_slave {
        int    attrib;
        uchar  pad1[2];
        uchar  capacity_high;
        uchar  capacity_low;
        uchar  pad2[2];
        uchar  remain_high;
        uchar  remain_low;
        uchar  pad3[2];
        uchar  discharge_remain_high;
        uchar  discharge_remain_low;
        uchar  pad4[2];
        uchar  discharge_time_high;
        uchar  discharge_time_low;
        uchar  pad5[2];
        uchar  full_charge_count_high;
        uchar  full_charge_count_low;
} _pm_battery_input;


typedef struct pm_battery _pm_battery_output;


typedef union  battery_status {
        _pm_battery_input     ibat;
        _pm_battery_output    obat;
} BATTERY_DATA;



/*
 ***********************************
 *  Suspend Control block          *
 ***********************************
 */
typedef struct PM_Suspend_control {
        int    LID_Open_Resume;        /* All of these resume events are  */
        int    ExtKBD_Resume;          /* initiated by Slave CPU. Hence,  */
        int    PD_Click_Resume;        /* pre-operation to H8 is needed   */
        int    Susp2Hibernation;       /* right before entering suspend.  */
} SCB;

/*_______________________ slave special header ends here _________________*/

#endif /* _H_SLAVE */
