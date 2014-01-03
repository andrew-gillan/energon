/*
 * ADE7816.c
 *
 *  Created on: Dec 14, 2012
 *      Author: haliax
 */

#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "ADE7816.h"

#define CAL_FILE_NAME    "cal.txt"
#define CONFIG_FILE_NAME "config.txt"

// Calibration Constants.  Read in from a cal.txt file.
float vrms_k, iarms_k, ibrms_k, icrms_k, idrms_k, ierms_k, ifrms_k;
float awatthr_k, bwatthr_k, cwatthr_k, dwatthr_k, ewatthr_k, fwatthr_k;
float avarhr_k, bvarhr_k, cvarhr_k, dvarhr_k, evarhr_k, fvarhr_k;

unsigned int read_S32_ZPSE(const unsigned char *reg_val);
unsigned int read_S32(const unsigned char *reg_val);
unsigned int read_U32(const unsigned char *reg_val);
unsigned int read_U16(const unsigned char *reg_val);
unsigned int read_U8(const unsigned char *reg_val);

#define I2C_FILE_NAME "/dev/i2c-3"
#define ADE7816_ADDR (0x38)

// Calibration and Power Quality Registers
#define VGAIN		(0x4380)   // Voltage gain adjustment.
#define IAGAIN		(0x4381)   // Current Channel A current gain adjustment.
#define IBGAIN		(0x4382)   // Current Channel B current gain adjustment.
#define ICGAIN		(0x4383)   // Current Channel C current gain adjustment.
#define IDGAIN		(0x4384)   // Current Channel D current gain adjustment.
#define IEGAIN		(0x4385)   // Current Channel E current gain adjustment.
#define IFGAIN		(0x4386)   // Current Channel F current gain adjustment.
#define DICOEFF		(0x4388)   // Register used in the digital integrator algorithm. When the integrator is enabled, this register should be set to (0xFFF8000).
#define HPFDIS 		(0x4389)   // Disables the high-pass filter for all channels.
#define VRMSOS 		(0x438A)   // Voltage rms offset.
#define IARMSOS 	(0x438B)   // Current Channel A current rms offset.
#define IBRMSOS 	(0x438C)   // Current Channel B current rms offset.
#define ICRMSOS 	(0x438D)   // Current Channel C current rms offset.
#define IDRMSOS 	(0x438E)   // Current Channel D current rms offset.
#define IERMSOS 	(0x438F)   // Current Channel E current rms offset.
#define IFRMSOS 	(0x4390)   // Current Channel F current rms offset.
#define AWGAIN 		(0x4391)   // Channel A active power gain adjust.
#define AWATTOS 	(0x4392)   // Channel A active power offset adjust.
#define BWGAIN 		(0x4393)   // Channel B active power gain adjust.
#define BWATTOS 	(0x4394)   // Channel B active power offset adjust.
#define CWGAIN 		(0x4395)   // Channel C active power gain adjust.
#define CWATTOS 	(0x4396)   // Channel C active power offset adjust.
#define DWGAIN 		(0x4397)   // Channel D active power gain adjust
#define DWATTOS 	(0x4398)   // Channel D active power offset adjust.
#define EWGAIN 		(0x4399)   // Channel E active power gain adjust.
#define EWATTOS 	(0x439A)   // Channel E active power offset adjust.
#define FWGAIN 		(0x439B)   // Channel F active power gain adjust.
#define FWATTOS 	(0x439C)   // Channel F active power offset adjust.
#define AVARGAIN 	(0x439D)   // Channel A reactive power gain adjust.
#define AVAROS 		(0x439E)   // Channel A reactive power offset adjust.
#define BVARGAIN 	(0x439F)   // Channel B reactive power gain adjust.
#define BVAROS 		(0x43A0)   // Channel B reactive power offset adjust.
#define CVARGAIN 	(0x43A1)   // Channel C reactive power gain adjust.
#define CVAROS 		(0x43A2)   // Channel C reactive power offset adjust.
#define DVARGAIN 	(0x43A3)   // Channel D reactive power gain adjust.
#define DVAROS 		(0x43A4)   // Channel D reactive power offset adjust.
#define EVARGAIN 	(0x43A5)   // Channel E reactive power gain adjust.
#define EVAROS 		(0x43A6)   // Channel E reactive power offset adjust.
#define FVARGAIN 	(0x43A7)   // Channel F reactive power gain adjust.
#define FVAROS 		(0x43A8)   // Channel F reactive power offset adjust.
#define WTHR1 		(0x43AB)   // Most significant 24 bits of the WTHR[47:0] threshold.
#define WTHR0 		(0x43AC)   // Least significant 24 bits of the WTHR[47:0] threshold.
#define VARTHR1		(0x43AD)   // Most significant 24 bits of the VARTHR[47:0] threshold.
#define VARTHR0		(0x43AE)   // Least significant 24 bits of the VARTHR[47:0] threshold.
#define APNOLOAD	(0x43AF)   // No load threshold in the active power datapath.
#define VARNOLOAD	(0x43B0)   // No load threshold in the reactive power datapath.
#define PCF_A_COEFF	(0x43B1)   // Phase calibration coefficient for Channel A. Set to (0x400C4A for a 50 Hz system and (0x401235 for a 60 Hz system.
#define PCF_B_COEFF	(0x43B2)   // Phase calibration coefficient for Channel B. Set to (0x400C4A for a 50 Hz system and (0x401235 for a 60 Hz system.
#define PCF_C_COEFF	(0x43B3)   // Phase calibration coefficient for Channel C. Set to (0x400C4A for a 50 Hz system and (0x401235 for a 60 Hz system.
#define PCF_D_COEFF	(0x43B4)   // Phase calibration coefficient for Channel D. Set to (0x400C4A for a 50 Hz system and (0x401235 for a 60 Hz system.
#define PCF_E_COEFF	(0x43B5)   // Phase calibration coefficient for Channel E. Set to (0x400C4A for a 50 Hz system and (0x401235 for a 60 Hz system.
#define PCF_F_COEFF	(0x43B6)   // Phase calibration coefficient for Channel F. Set to (0x400C4A for a 50 Hz system and (0x401235 for a 60 Hz system.
#define VRMS		(0x43C0)   // Voltage rms value.
#define IARMS		(0x43C1)   // Current Channel A current rms value.
#define IBRMS		(0x43C2)   // Current Channel B current rms value.
#define ICRMS		(0x43C3)   // Current Channel C current rms value.
#define IDRMS		(0x43C4)   // Current Channel D current rms value.
#define IERMS		(0x43C5)   // Current Channel E current rms value.
#define IFRMS		(0x43C6)   // Current Channel F current rms value.

// Run Register
#define Run	(0xE228)   // This register starts and stops the DSP

// Billable Registers
#define AWATTHR	(0xE400)   // Channel A active energy accumulation.
#define BWATTHR	(0xE401)   // Channel B active energy accumulation.
#define CWATTHR	(0xE402)   // Channel C active energy accumulation.
#define DWATTHR	(0xE403)   // Channel D active energy accumulation.
#define EWATTHR	(0xE404)   // Channel E active energy accumulation.
#define FWATTHR	(0xE405)   // Channel F active energy accumulation.
#define AVARHR	(0xE406)   // Channel A reactive energy accumulation.
#define BVARHR	(0xE407)   // Channel B reactive energy accumulation.
#define CVARHR	(0xE408)   // Channel C reactive energy accumulation.
#define DVARHR	(0xE409)   // Channel D reactive energy accumulation.
#define EVARHR	(0xE40A)   // Channel E reactive energy accumulation.
#define FVARHR	(0xE40B)   // Channel F reactive energy accumulation.

// Configuration and Power Quality Registers
#define IPEAK 		(0xE500)   // Current peak register.
#define VPEAK 		(0xE501)   // Voltage peak register.
#define STATUS0 	(0xE502)   // Interrupt Status Register 0.
#define STATUS1 	(0xE503)   // Interrupt Status Register 1.
#define OILVL 		(0xE507)   // Overcurrent threshold.
#define OVLVL 		(0xE508)   // Overvoltage threshold.
#define SAGLVL 		(0xE509)   // Voltage sag level threshold.
#define MASK0 		(0xE50A)   // Interrupt Enable Register 0.
#define MASK1 		(0xE50B)   // Interrupt Enable Register 1.
#define IAWV_IDWV 	(0xE50C)   // Instantaneous Current Channel A and Instantaneous Current Channel D.
#define IBWV_IEWV 	(0xE50D)   // Instantaneous Current Channel B and Instantaneous Current Channel E.
#define ICWV_IFWV 	(0xE50E)   // Instantaneous Current Channel C and Instantaneous Current Channel F.
#define Reserved 	(0xE50F)   // This register should be ignored.
#define VWV 		(0xE510)   // Instantaneous voltage.
#define Checksum 	(0xE51F)   // Checksum verification (see the Checksum section for details)   //.
#define CHSTATUS 	(0xE600)   // Channel peak register.
#define ANGLE0 		(0xE601)   // Time Delay 0 (see the Angle Measurements section for details)   //.
#define ANGLE1 		(0xE602)   // Time Delay 1 (see the Angle Measurements section for details)   //.
#define ANGLE2 		(0xE603)   // Time Delay 2 (see the Angle Measurements section for details)   //.
#define Period		(0xE607)   // Line period.
#define CHNOLOAD	(0xE608)   // Channel no load register.
#define LINECYC 	(0xE60C)   // Line cycle accumulation mode count.
#define ZXTOUT 		(0xE60D)   // Zero-crossing timeout count.
#define COMPMODE 	(0xE60E)   // Computation mode register.
#define Gain 		(0xE60F)   // PGA gains at ADC inputs (see Table 22)   //.
#define CHSIGN 		(0xE617)   // Power sign register.
#define CONFIG 		(0xE618)   // Configuration register.
#define MMODE 		(0xE700)   // Measurement mode register.
#define ACCMODE 	(0xE701)   // Accumulation mode register.
#define LCYCMODE 	(0xE702)   // Line accumulation mode.
#define PEAKCYC 	(0xE703)   // Peak detection half line cycles.
#define SAGCYC 		(0xE704)   // Sag detection half line cycles.
#define HSDC_CFG 	(0xE706)   // HSDC configuration register.
#define Version 	(0xE707)   // Version of die.
// #define Reserved 	(0xE7E3)   // Register protection (see the Register Protection section).
// #define Reserved 	(0xE7FE)   // Register protection key (see the Register Protection section).
// #define Reserved 	(0xEBFF)   // This address can be used in manipulating the SS/HSA pin when SPI is chosen as the active port (see the Communication section for details)   //.
#define CONFIG2		(0xEC01)   // Configuration register (see Table 29).

// Register Defaults
#define WTHR1_DEFAULT (0x000002)
#define WTHR0_DEFAULT (0x000000)

#define VARTHR1_DEFAULT (0x000002)
#define VARTHR0_DEFAULT (0x000000)

#define PCF_A_COEFF_50HZ (0x400CA4)
#define PCF_B_COEFF_50HZ (0x400CA4)
#define PCF_C_COEFF_50HZ (0x400CA4)
#define PCF_D_COEFF_50HZ (0x400CA4)
#define PCF_E_COEFF_50HZ (0x400CA4)
#define PCF_F_COEFF_50HZ (0x400CA4)

#define PCF_A_COEFF_60HZ (0x401235)
#define PCF_B_COEFF_60HZ (0x401235)
#define PCF_C_COEFF_60HZ (0x401235)
#define PCF_D_COEFF_60HZ (0x401235)
#define PCF_E_COEFF_60HZ (0x401235)
#define PCF_F_COEFF_60HZ (0x401235)

#define DICOEFF_DEFAULT (0xFFF8000)

//// IPEAK Register (Address 0xE500)
//typedef struct
//{
//	unsigned int            : 5;  // [31:27] These bits should be ignored.
//	unsigned int IPCHANNEL2 : 1;  // [26] The C or F current channel generated the IPEAKVAL[23:0] value.
//	unsigned int IPCHANNEL1 : 1;  // [25] The B or E current channel generated the IPEAKVAL[23:0] value.
//	unsigned int IPCHANNEL0 : 1;  // [24] The A or D current channel generated the IPEAKVAL[23:0] value.
//	unsigned int IPEAKVAL   : 24; // [23:0] Current channel peak value
//} ipeak_t;
//
//// VPEAK Register (Address 0xE501)
//typedef struct
//{
//	unsigned int          : 8;  // [31:24] These bits should be ignored.
//	unsigned int VPEAKVAL : 24; // [23:0]	0x0	Voltage channel peak value.
//} vpeak_t;
//
//// CHSTATUS Register (Address 0xE600)
//typedef struct
//{
//	unsigned int            : 10; // [15:6] Reserved. These bits should be ignored.
//	unsigned int OICHANNEL2 : 1;  // [5] The C or F current channel generated the overcurrent event.
//	unsigned int OICHANNEL1 : 1;  // [4] The B or E current channel generated the overcurrent event.
//	unsigned int OICHANNEL0 : 1;  // [3] The A or D current channel generated the overcurrent event.
//	unsigned int            : 3;  // [2:0] Reserved. These bits are always 0.
//} chstatus_t;
//
//// CHNOLOAD Register (Address 0xE608)
//typedef struct
//{
//	unsigned int         :10; // [15:6] Reserved. These bits should be ignored.
//	unsigned int NOLOADF : 1; // Channel F is out of the no load condition. 1: Channel F is in the no load condition.
//	unsigned int NOLOADE : 1; // Channel E is out of the no load condition. 1: Channel E is in the no load condition.
//	unsigned int NOLOADD : 1; // Channel D is out of the no load condition. 1: Channel D is in the no load condition.
//	unsigned int NOLOADC : 1; // Channel C is out of the no load condition. 1: Channel C is in the no load condition.
//	unsigned int NOLOADB : 1; // Channel B is out of the no load condition. 1: Channel B is in the no load condition.
//	unsigned int NOLOADA : 1; // Channel A is out of the no load condition. 1: Channel A is in the no load condition.
//} chnoload_t;
//
//// COMPMODE Register (Address 0xE60E)
//typedef struct
//{
//	unsigned int             : 1; // This bit should be ignored.
//	unsigned int CHANNEL_SEL : 1; // 0: the A, B, and C current channels are used for the peak, overcurrent, zero crossing, angle, and waveform measurements. 1: the D, E, and F current channels are used for the peak, overcurrent, zero crossing, angle, and waveform measurements.
//	unsigned int             : 3; // These bits should be ignored.
//	unsigned int ANGLESEL    : 2; // 00: the time delays between the voltage and currents are measured. 01: reserved. 10: the angles between current channels are measured. 11: no angles are measured.
//	unsigned int             : 9; // These bits should be ignored and not modified.
//} compmode_t;
//
//// Gain Register (Address 0xE60F)
//typedef struct
//{
//	unsigned int      : 7; // [15:9] These bits should be ignored.
//	unsigned int PGA3 : 3; // [8:6] Gain selection for the D, E, and F current channels. 000: gain = 1. 001: gain = 2. 010: gain = 4. 011: gain = 8. 100: gain = 16. 101, 110, 111: reserved.
//	unsigned int PGA2 : 3; // [5:3] Voltage channel gain selection. 000: gain = 1. 001: gain = 2. 010: gain = 4. 011: gain = 8. 100: gain = 16. 101, 110, 111: reserved.
//	unsigned int PGA1 : 3; // [2:0] Gain selection for the A, B, and C current channels. 000: gain = 1. 001: gain = 2. 010: gain = 4. 011: gain = 8. 100: gain = 16. 101, 110, 111: reserved.
//} gain_t;
//
//// CHSIGN Register (Address 0xE617)
//typedef struct
//{
//	unsigned int          : 9; // [15:7] These bits should be ignored.
//	unsigned int VAR3SIGN : 1; // 6 0: the reactive power on the C or F channel is positive. 1: the reactive power on the C or F channel is negative.
//	unsigned int VAR2SIGN : 1; // 5 0: the reactive power on the B or E channel is positive. 1: the reactive power on the B or E channel is negative.
//	unsigned int VAR1SIGN : 1; // 4 0: the reactive power on the A or D channel is positive. 1: the reactive power on the A or D channel is negative.
//	unsigned int          : 1; // 3 This bit should be ignored.
//	unsigned int W3SIGN   : 1; // 2 0: the active power on the C or F channel is positive. 1: the active power on the C or F channel is negative.
//	unsigned int W2SIGN   : 1; // 1 0: the active power on the B or E channel is positive. 1: the active power on the B or E channel is negative.
//	unsigned int W1SIGN   : 1; // 0 0: the active power on the A or D channel is positive. 1: the active power on the A or D channel is negative.
//} chsign_t;
//
//// CONFIG Register (Address 0xE618)
//typedef struct
//{
//	unsigned int        : 8; // [15:8] These bits should be ignored.
//	unsigned int SWRST  : 1; // 7 Initiates a software reset.
//	unsigned int HSDCEN : 1; // 6 Enables the HSDC serial port.
//	unsigned int        : 5; // [5:1] These bits should be ignored.
//	unsigned int INTEN  : 1; // 0 Enables the digital integrator
//} config_t;
//
//// MMODE Register (Address 0xE700)
//typedef struct
//{
//	unsigned int          : 3; // [7:5] These bits should be ignored.
//	unsigned int PEAKSEL2 : 1; // 4 The C or F current channel is selected for peak detection.
//	unsigned int PEAKSEL1 : 1; // 3 The B or E current channel is selected for peak detection.
//	unsigned int PEAKSEL0 : 1; // 2 The A or D current channel is selected for peak detection.
//	unsigned int          : 2; // [1:0] These bits should be ignored.
//} mmode_t;
//
//// ACCMODE Register (Address 0xE701)
//typedef struct
//{
//	unsigned int REVRPSEL : 1; // 7 0: the sign of the reactive power is monitored on the A, B, and C channels. 1: the sign of the reactive power is monitored on the D, E, and F channels.
//	unsigned int REVAPSEL : 1; // 6 0: the sign of the active power is monitored on the A, B, and C channels. 1: the sign of the active power is monitored on the D, E, and F channels.
//	unsigned int          : 2; // [5:4] These bits should be ignored and not modified.
//	unsigned int VARACC   : 2; // [3:2] 00: signed accumulation for all reactive power measurements. 01: reserved. 10: reserved. 11: reserved.
//	unsigned int WATTACC  : 2; // [1:0] 00: signed accumulation for all active power measurements. 01: reserved. 10: reserved. 11: reserved.
//} accmode_t;
//
//// LCYCMODE Register (Address 0xE702)
//typedef struct
//{
//	unsigned int         : 1; // 7 This bit does not control any functionality.
//	unsigned int RSTREAD : 1; // 6 Enables read-with-reset for all energy registers. Note that this bit has no function in line cycle accumulation mode and should be set to 0 when this mode is in use.
//	unsigned int         : 2; // [5:4] These bits should be ignored.
//	unsigned int ZX_SEL  : 1; // 3 Enables the voltage channel zero-crossing counter for line cycle accumulation mode.
//	unsigned int         : 1; // 2 These bits should be ignored.
//	unsigned int LVAR    : 1; // 1 Enables the reactive energy line cycle accumulation mode.
//	unsigned int LWATT   : 1; // 0 Enables the active energy line cycle accumulation mode.
//} lcycmode_t;
//
//// HSDC_CFG Register (Address 0xE706)
//typedef struct
//{
//	unsigned int        : 2; // [7:6] These bits should be ignored.
//	unsigned int HSAPOL : 1; // 5 0: SS/HSA output pin is active low (default). 1: SS/HSA output pin is active high.
//	unsigned int HXFER  : 2; // [4:3] 00 = reserved. 01 = HSDC transmits current and voltage waveform data. 10 = reserved. 11 = reserved.
//	unsigned int HGAP   : 1; // 2 0: no gap is introduced between packages (default). 1: a gap of seven HCLK cycles is introduced between packages.
//	unsigned int HSIZE  : 1; // 1 0: HSDC transmits the 32-bit registers in 32-bit packages, most significant bit first (default). 1: HSDC transmits the 32-bit registers in 8-bit packages, most significant bit first.
//	unsigned int HCLK   : 1; // 0 0: HSCLK = 8 MHz (default). 1: HSCLK = 4 MHz.
//} hsdc_cfg_t;
//
//// CONFIG2 Register (Address 0xEC01)
//typedef struct
//{
//	unsigned int          : 6; // [7:2] These bits should be ignored.
//	unsigned int I2C_LOCK : 1; // 1 Serial port lock.
//	unsigned int EXTREFEN : 1; // 0 Set to 1 to use with an external reference.
//} config2_t;
//
//// STATUS0 Register (Address 0xE502) and MASK0 Register (Address 0xE50A)
//typedef struct
//{
//	unsigned int         : 14; // [31:18] These bits should be ignored.
//	unsigned int DREADY  : 1; // 17 New waveform data is ready.
//	unsigned int         : 1; // 16 This bit should be ignored.
//	unsigned int         : 1; // 15 This bit should be ignored.
//	unsigned int         : 1; // 14 This bit should be ignored.
//	unsigned int         : 1; // 13 This bit should be ignored.
//	unsigned int REVRP3  : 1; // 12 The sign of the reactive power has changed (C or F channel).
//	unsigned int REVRP2  : 1; // 11 The sign of the reactive power has changed (B or E channel).
//	unsigned int REVRP1  : 1; // 10 The sign of the reactive power has changed (A or D channel).
//	unsigned int         : 1; // 9 This bit should be ignored.
//	unsigned int REVAP3  : 1; // 8 The sign of the active power has changed (C or F channel).
//	unsigned int REVAP2  : 1; // 7 The sign of the active power has changed (B or E channel).
//	unsigned int REVAP1  : 1; // 6 The sign of the active power has changed (A or D channel).
//	unsigned int LENERGY : 1; // 5 The end of a line cycle accumulation period.
//	unsigned int         : 1; // 4 This bit should be ignored.
//	unsigned int REHF2   : 1; // 3 The active energy register is half full (D, E, or F channel).
//	unsigned int REHF1   : 1; // 2 The reactive energy register is half full (A, B, or C channel).
//	unsigned int AEHF2   : 1; // 1 The active energy register is half full (D, E, or F channel)
//	unsigned int AEHF1   : 1; // 0 The active energy register is half full (A, B, or C channel).
//} status0_t;
//
//// STATUS1 Register (Address 0xE503) and MASK1 Register (Address 0xE50B)
//typedef struct
//{
//	unsigned int         : 7; // [31:25] These bits should be ignored.
//	unsigned int PKV     : 1; // 24 The end of the voltage channel peak detection period.
//	unsigned int PKI     : 1; // 23 The end of the current channel peak detection period.
//	unsigned int         : 1; // 22 This bit should be ignored.
//	unsigned int         : 1; // 21 This bit should be ignored.
//	unsigned int         : 1; // 20 This bit should be ignored.
//	unsigned int         : 1; // 19 This bit should be ignored.
//	unsigned int OV      : 1; // 18 An overvoltage event has occurred.
//	unsigned int OI      : 1; // 17 An overcurrent event has occurred.
//	unsigned int Sag     : 1; // 16 A sag event has occurred.
//	unsigned int RSTDONE : 1; // 15 The end of a software or hardware reset.
//	unsigned int ZXI3    : 1; // 14 C or F current channel zero crossing.
//	unsigned int ZXI2    : 1; // 13 B or E current channel zero crossing.
//	unsigned int ZXI1    : 1; // 12 A or D current channel zero crossing.
//	unsigned int         : 1; // 11 This bit should be ignored.
//	unsigned int         : 1; // 10 This bit should be ignored.
//	unsigned int ZXV     : 1; // 9 Voltage channel zero crossing.
//	unsigned int ZXTOI3  : 1; // 8 A zero crossing on the C or F current channel is missing.
//	unsigned int ZXTOI2  : 1; // 7 A zero crossing on the B or E current channel is missing.
//	unsigned int ZXTOI1  : 1; // 6 A zero crossing on the A or D current channel is missing.
//	unsigned int         : 1; // 5 This bit should be ignored.
//	unsigned int         : 1; // 4 This bit should be ignored.
//	unsigned int ZXTOV   : 1; // 3 A zero crossing on the voltage channel is missing.
//	unsigned int         : 1; // 2 This bit should be ignored.
//	unsigned int NLOAD2  : 1; // 1 Active and reactive no load condition on the D, E, or F current channel.
//	unsigned int NLOAD1  : 1; // 0 Active and reactive no load condition on the A, B, or C current channel.
//} status1_t;

typedef union {
	unsigned int uint32;
	int int32;
} anydata_t;

typedef enum
{
	u8,
	u16,
	u32,
	s32,
	s32_ZPSE
} reg_type_t;

typedef struct
{
	const unsigned int reg;
	const char *const regstr;
	reg_type_t regtype;
	unsigned int (*readReg)(const unsigned char *);
} reg_t;

static reg_t reg_table[] = {
		{VGAIN,       "VGAIN",       s32_ZPSE, &read_S32_ZPSE},
		{IAGAIN,      "IAGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{IBGAIN,      "IBGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{ICGAIN,      "ICGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{IDGAIN,      "IDGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{IEGAIN,      "IEGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{IFGAIN,      "IFGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{DICOEFF,     "DICOEFF",     s32_ZPSE, &read_S32_ZPSE},
		{HPFDIS,      "HPFDIS",      s32_ZPSE, &read_S32_ZPSE},
		{VRMSOS,      "VRMSOS",      s32_ZPSE, &read_S32_ZPSE},
		{IARMSOS,     "IARMSOS",     s32_ZPSE, &read_S32_ZPSE},
		{IBRMSOS,     "IBRMSOS",     s32_ZPSE, &read_S32_ZPSE},
		{ICRMSOS,     "ICRMSOS",     s32_ZPSE, &read_S32_ZPSE},
		{IDRMSOS,     "IDRMSOS",     s32_ZPSE, &read_S32_ZPSE},
		{IERMSOS,     "IERMSOS",     s32_ZPSE, &read_S32_ZPSE},
		{IFRMSOS,     "IFRMSOS",     s32_ZPSE, &read_S32_ZPSE},
		{AWGAIN,      "AWGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{AWATTOS,     "AWATTOS",     s32_ZPSE, &read_S32_ZPSE},
		{BWGAIN,      "BWGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{BWATTOS,     "BWATTOS",     s32_ZPSE, &read_S32_ZPSE},
		{CWGAIN,      "CWGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{CWATTOS,     "CWATTOS",     s32_ZPSE, &read_S32_ZPSE},
		{DWGAIN,      "DWGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{DWATTOS,     "DWATTOS",     s32_ZPSE, &read_S32_ZPSE},
		{EWGAIN,      "EWGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{EWATTOS,     "EWATTOS",     s32_ZPSE, &read_S32_ZPSE},
		{FWGAIN,      "FWGAIN",      s32_ZPSE, &read_S32_ZPSE},
		{FWATTOS,     "FWATTOS",     s32_ZPSE, &read_S32_ZPSE},
		{AVARGAIN,    "AVARGAIN",    s32_ZPSE, &read_S32_ZPSE},
		{AVAROS,      "AVAROS",      s32_ZPSE, &read_S32_ZPSE},
		{BVARGAIN,    "BVARGAIN",    s32_ZPSE, &read_S32_ZPSE},
		{BVAROS,      "BVAROS",      s32_ZPSE, &read_S32_ZPSE},
		{CVARGAIN,    "CVARGAIN",    s32_ZPSE, &read_S32_ZPSE},
		{CVAROS,      "CVAROS",      s32_ZPSE, &read_S32_ZPSE},
		{DVARGAIN,    "DVARGAIN",    s32_ZPSE, &read_S32_ZPSE},
		{DVAROS,      "DVAROS",      s32_ZPSE, &read_S32_ZPSE},
		{EVARGAIN,    "EVARGAIN",    s32_ZPSE, &read_S32_ZPSE},
		{EVAROS,      "EVAROS",      s32_ZPSE, &read_S32_ZPSE},
		{FVARGAIN,    "FVARGAIN",    s32_ZPSE, &read_S32_ZPSE},
		{FVAROS,      "FVAROS",      s32_ZPSE, &read_S32_ZPSE},
		{WTHR1,       "WTHR1",       u32,      &read_U32},
		{WTHR0,       "WTHR0",       u32,      &read_U32},
		{VARTHR1,     "VARTHR1",     u32,      &read_U32},
		{VARTHR0,     "VARTHR0",     u32,      &read_U32},
		{APNOLOAD,    "APNOLOAD",    u32,      &read_U32},
		{VARNOLOAD,   "VARNOLOAD",   s32_ZPSE, &read_S32_ZPSE},
		{PCF_A_COEFF, "PCF_A_COEFF", u32,      &read_U32},
		{PCF_B_COEFF, "PCF_B_COEFF", u32,      &read_U32},
		{PCF_C_COEFF, "PCF_C_COEFF", u32,      &read_U32},
		{PCF_D_COEFF, "PCF_D_COEFF", u32,      &read_U32},
		{PCF_E_COEFF, "PCF_E_COEFF", u32,      &read_U32},
		{PCF_F_COEFF, "PCF_F_COEFF", u32,      &read_U32},
		{VRMS,        "VRMS",        s32,      &read_S32},
		{IARMS,       "IARMS",       s32,      &read_S32},
		{IBRMS,       "IBRMS",       s32,      &read_S32},
		{ICRMS,       "ICRMS",       s32,      &read_S32},
		{IDRMS,       "IDRMS",       s32,      &read_S32},
		{IERMS,       "IERMS",       s32,      &read_S32},
		{IFRMS,       "IFRMS",       s32,      &read_S32},
		{Run,         "Run",         u16,      &read_U16},
		{AWATTHR,     "AWATTHR",     s32,      &read_S32},
		{BWATTHR,     "BWATTHR",     s32,      &read_S32},
		{CWATTHR,     "CWATTHR",     s32,      &read_S32},
		{DWATTHR,     "DWATTHR",     s32,      &read_S32},
		{EWATTHR,     "EWATTHR",     s32,      &read_S32},
		{FWATTHR,     "FWATTHR",     s32,      &read_S32},
		{AVARHR,      "AVARHR",      s32,      &read_S32},
		{BVARHR,      "BVARHR",      s32,      &read_S32},
		{CVARHR,      "CVARHR",      s32,      &read_S32},
		{DVARHR,      "DVARHR",      s32,      &read_S32},
		{EVARHR,      "EVARHR",      s32,      &read_S32},
		{FVARHR,      "FVARHR",      s32,      &read_S32},
		{IPEAK,       "IPEAK",       u32,      &read_U32},
		{VPEAK,       "VPEAK",       u32,      &read_U32},
		{STATUS0,     "STATUS0",     u32,      &read_U32},
		{STATUS1,     "STATUS1",     u32,      &read_U32},
		{OILVL,       "OILVL",       u32,      &read_U32},
		{OVLVL,       "OVLVL",       u32,      &read_U32},
		{SAGLVL,      "SAGLVL",      u32,      &read_U32},
		{MASK0,       "MASK0",       u32,      &read_U32},
		{MASK1,       "MASK1",       u32,      &read_U32},
		{IAWV_IDWV,   "IAWV_IDWV",   s32,      &read_S32},
		{IBWV_IEWV,   "IBWV_IEWV",   s32,      &read_S32},
		{ICWV_IFWV,   "ICWV_IFWV",   s32,      &read_S32},
		{VWV,         "VWV",         s32,      &read_S32},
		{Checksum,    "Checksum",    u32,      &read_U32},
		{CHSTATUS,    "CHSTATUS",    u16,      &read_U16},
		{ANGLE0,      "ANGLE0",      u16,      &read_U16},
		{ANGLE1,      "ANGLE1",      u16,      &read_U16},
		{ANGLE2,      "ANGLE2",      u16,      &read_U16},
		{Period,      "Period",      u16,      &read_U16},
		{CHNOLOAD,    "CHNOLOAD",    u16,      &read_U16},
		{LINECYC,     "LINECYC",     u16,      &read_U16},
		{ZXTOUT,      "ZXTOUT",      u16,      &read_U16},
		{COMPMODE,    "COMPMODE",    u16,      &read_U16},
		{Gain,        "Gain",        u16,      &read_U16},
		{CHSIGN,      "CHSIGN",      u16,      &read_U16},
		{CONFIG,      "CONFIG",      u16,      &read_U16},
		{MMODE,       "MMODE",       u8,       &read_U8},
		{ACCMODE,     "ACCMODE",     u8,       &read_U8},
		{LCYCMODE,    "LCYCMODE",    u8,       &read_U8},
		{PEAKCYC,     "PEAKCYC",     u8,       &read_U8},
		{SAGCYC,      "SAGCYC",      u8,       &read_U8},
		{HSDC_CFG,    "HSDC_CFG",    u8,       &read_U8},
		{Version,     "Version",     u8,       &read_U8},
		{CONFIG2,     "CONFIG2",     u8,       &read_U8}
};

enum {
	kVRMS = 0,
	kIARMS,
	kIBRMS,
	kICRMS,
	kIDRMS,
	kIERMS,
	kIFRMS,
	kAWATTHR,
	kAVARHR,
	kBWATTHR,
	kBVARHR,
	kCWATTHR,
	kCVARHR,
	kDWATTHR,
	kDVARHR,
	kEWATTHR,
	kEVARHR,
	kFWATTHR,
	kFVARHR
};

typedef struct {
	const char *const regstr;
	float *cal_const_value;
} cal_t;

// Enum above can be used to as map into cal_reg table.
static cal_t cal_reg_table[] = {
		{"kVRMS", &vrms_k},
		{"kIARMS", &iarms_k},
		{"kIBRMS", &ibrms_k},
		{"kICRMS", &icrms_k},
		{"kIDRMS", &idrms_k},
		{"kIERMS", &ierms_k},
		{"kIFRMS", &ifrms_k},
		{"kAWATTHR", &awatthr_k},
		{"kAVARHR", &avarhr_k},
		{"kBWATTHR", &bwatthr_k},
		{"kBVARHR", &bvarhr_k},
		{"kCWATTHR", &cwatthr_k},
		{"kCVARHR", &cvarhr_k},
		{"kDWATTHR", &dwatthr_k},
		{"kDVARHR", &dvarhr_k},
		{"kEWATTHR", &ewatthr_k},
		{"kEVARHR", &evarhr_k},
		{"kFWATTHR", &fwatthr_k},
		{"kFVARHR", &fvarhr_k}
};

void set_buffer(unsigned char *buf, const unsigned int reg, unsigned int value, const reg_type_t reg_type)
{
	buf[0] = reg >> 8;
	buf[1] = reg & 0xFF;

	switch(reg_type)
	{
	case u8:
		buf[2] = value & 0xFF;
		break;
	case u16:
		buf[2] = (value >> 8) & 0xFF;
		buf[3] = value & 0xFF;
		break;
	case u32:
	case s32:
	case s32_ZPSE:
		buf[2] = (value >> 24) & 0xFF;
		buf[3] = (value >> 16) & 0xFF;
		buf[4] = (value >> 8) & 0xFF;
		buf[5] = value & 0xFF;
		break;
	default:
		break;
	}
}

unsigned int read_S32_ZPSE(const unsigned char *reg_val)
{
	unsigned int value;

	if (reg_val[0] == 0x0F)
	{
		value = 0xFF000000 |reg_val[1] << 16 | reg_val[2] << 8 | reg_val[3];
	}
	else
	{
		value = reg_val[0] << 24 | reg_val[1] << 16 | reg_val[2] << 8 | reg_val[3];
	}
	return value;
}

unsigned int read_S32(const unsigned char *reg_val)
{
	unsigned int value;
	value = reg_val[0] << 24 | reg_val[1] << 16 | reg_val[2] << 8 | reg_val[3];
	return value;
}

unsigned int read_U32(const unsigned char *reg_val)
{
	unsigned int value;
	value = reg_val[0] << 24 | reg_val[1] << 16 | reg_val[2] << 8 | reg_val[3];
	return value;
}

unsigned int read_U16(const unsigned char *reg_val)
{
	unsigned int value;
	value = reg_val[0] << 8 | reg_val[1];
	return value;
}

unsigned int read_U8(const unsigned char *reg_val)
{
	return *reg_val;
}

int readRegister(const unsigned int reg, unsigned char *buf)
{
	// read all registers into structure
	char *namebuf = I2C_FILE_NAME;
    //unsigned char buf[4];
	int file;
    int numBytes, bytesRead;

    if ((file = open(namebuf, O_RDWR)) < 0){
            printf ("Failed to open %s I2C Bus.\n", namebuf);
            return(-1);
    }

    if (ioctl(file, I2C_SLAVE, ADE7816_ADDR) < 0){
            printf ("Failed to set I2C_SLAVE address.\n");
            return(-2);
    }

	// According to the ADE7816 datasheet, you need to send the first address
	// in write mode and then a stop/start condition is issued.
	buf[0] = (reg >> 8);
	buf[1] = (reg & 0xFF);
	numBytes = 2;
	bytesRead = write(file, buf, numBytes);
	if (bytesRead !=2){
		printf("Failed to Reset Address err = %d\n", bytesRead);
		return(-3);
	}

	numBytes = 4;
	bytesRead = read(file, buf, numBytes);
	if (bytesRead == -1){
		puts("Failure to read Byte Stream in readFullSensorState()");
		return(-4);
	}

    close(file);

    return (bytesRead);
}

int ADE7816_readRegister(const unsigned int reg)
{
	// read all registers into structure
    unsigned char buf[4];
    int bytesRead;
    int i;
    unsigned int regval;

    bytesRead = readRegister(reg, buf);

    for (i=0;i<(sizeof(reg_table)/sizeof(reg_t));i++)
    {
    	if (reg == reg_table[i].reg)
    	{
    		printf("read 0x%X %s=", reg, reg_table[i].regstr);
    		switch(reg_table[i].regtype)
    		{
    		case u8:
    		case u16:
    		case u32:
    			printf("0x%X\n", reg_table[i].readReg(buf));
    			break;
    		case s32:
    			regval = reg_table[i].readReg(buf);
    			printf("%d\n", *(int*)&regval);
    			break;
    		case s32_ZPSE:
    			regval = reg_table[i].readReg(buf);
    			printf("%d\n", *(int*)&regval);
    			break;
    		default:
    			return (-5);
    			break;
    		}
    		break;
    	}
    }

    return (bytesRead);
}

int ADE7816_writeRegister(const unsigned int reg, const unsigned int value)
{
	// Startup routine as per datasheet
	char *namebuf = I2C_FILE_NAME;
    unsigned char buf[6];
	int file, i;
    int result = 0, length = 0;

    // Open i2c bus for communications
    if ((file = open(namebuf, O_RDWR)) < 0){
            printf ("Failed to open %s I2C Bus.\n", namebuf);
            result = 1;
            goto error;
    }
    if (ioctl(file, I2C_SLAVE, ADE7816_ADDR) < 0){
            printf ("Failed to set I2C_SLAVE address.\n");
            result = 2;
            goto error;
   }
    for (i=0;i<(sizeof(reg_table)/sizeof(reg_t));i++)
    {
    	if (reg == reg_table[i].reg)
    	{
    		printf("write 0x%X %s=", reg, reg_table[i].regstr);
    		switch(reg_table[i].regtype)
    		{
    		case u8:
    			length = 1;
    			printf("0x%X\n", value);
    			break;
    		case u16:
    			length = 2;
    			printf("0x%X\n", value);
    			break;
    		case u32:
    			length = 4;
    			printf("0x%X\n", value);
    			break;
    		case s32:
    		case s32_ZPSE:
    			length = 4;
    			printf("%d\n", value);
    			break;
    		default:
    			break;
    		}
    		break;
    	}
    }

	// Write register
    set_buffer(buf, reg, value, reg_table[i].regtype);
	if (write(file, buf, length+2) != length+2) {result = 3; goto error;}

	// CHECK REGISTERS
	error:
    close(file);
    // maybe add an if statement here to do some cleanup or proper stderror output
	return result;
}

int ADE7816_writeMultipleRegisters(const char *filename)
{
	FILE *file;
	int result = 0;
	char line_buffer[BUFSIZ];

	if ((file = fopen(filename, "r")) < 0){
	            printf ("Failed to open %s\n", filename);
	            result = -1;
	            goto error;
	    }

	while(fgets(line_buffer, sizeof(line_buffer), file))
	{
		char regname_str[20], reg_str[20], value_str[20];
		unsigned int reg;
		int i;

		sscanf(line_buffer, "%s %s %s", regname_str, reg_str, value_str);
		reg = strtoul(reg_str, NULL, 0);
		for (i=0; i<(sizeof(reg_table)/sizeof(reg_t));i++)
		{
			if(reg == reg_table[i].reg)
			{
	    		switch(reg_table[i].regtype)
	    		{
	    		case u8:
	    		case u16:
	    		case u32:
	    			ADE7816_writeRegister(reg, strtoul(value_str, NULL, 0));
	    			break;
	    		case s32:
	    		case s32_ZPSE:
	    			ADE7816_writeRegister(reg, strtol(value_str, NULL, 0)); // need check this with -ve numbers
	    			break;
	    		default:
	    			break;
	    		}
	    		break;
			}
		}

		ADE7816_readRegister(reg);
	}

	error:
    fclose(file);
    // maybe add an if statement here to do some cleanup or proper stderror output
	return result;
}

int ADE7816_runDSP(void)
{
	// Startup routine as per datasheet
	char *namebuf = I2C_FILE_NAME;
    unsigned char buf[6];
	int file;
    unsigned int val;
    int result = 0;

    // Open i2c bus for communications
    if ((file = open(namebuf, O_RDWR)) < 0){
            printf ("Failed to open %s I2C Bus.\n", namebuf);
            result = 1;
            goto error;
    }
    if (ioctl(file, I2C_SLAVE, ADE7816_ADDR) < 0){
            printf ("Failed to set I2C_SLAVE address.\n");
            result = 2;
            goto error;
    }

	// Enable the energy metering DSP
    val = 1;
    set_buffer(buf, Run, val, u16);
	if (write(file, buf, 4) != 4) {result = 3; goto error;}

	// Flush the pipeline after starting the DSP
	if (write(file, buf, 4) != 4) {result = 3; goto error;}
	if (write(file, buf, 4) != 4) {result = 3; goto error;}
	if (write(file, buf, 4) != 4) {result = 3; goto error;}

	// CHECK REGISTERS
	error:
	close(file);
	// maybe add an if statement here to do some cleanup or proper stderror output
	return result;
}


int ADE7816_stopDSP(void)
{
	// Startup routine as per datasheet
	char *namebuf = I2C_FILE_NAME;
    unsigned char buf[6];
	int file;
    unsigned int val;
    int result = 0;

    // Open i2c bus for communications
    if ((file = open(namebuf, O_RDWR)) < 0){
            printf ("Failed to open %s I2C Bus.\n", namebuf);
            result = 1;
            goto error;
    }
    if (ioctl(file, I2C_SLAVE, ADE7816_ADDR) < 0){
            printf ("Failed to set I2C_SLAVE address.\n");
            result = 2;
            goto error;
    }

	// Disable the energy metering DSP
    val = 0;
    set_buffer(buf, Run, val, u16);
	if (write(file, buf, 4) != 4) {result = 3; goto error;}

	// CHECK REGISTERS
	error:
	close(file);
	// maybe add an if statement here to do some cleanup or proper stderror output
	return result;
}


int ADE7816_readRmsRegisters(void) {
	unsigned char buf[4];
	float rms;
	unsigned int regval;
	char *cal_filename = "cal.txt";

	ADE7816_writeCalConstants(cal_filename);

	readRegister(IARMS, buf);
	regval = read_S32(buf);
	rms = (*(int*)&regval) * *(cal_reg_table[kIARMS].cal_const_value) / 1000 / 65536;
	printf("\nIa rms = 0x%X, %.1f\n", regval, rms);

	readRegister(IBRMS, buf);
	regval = read_S32(buf);
	rms = (*(int*)&regval) * *(cal_reg_table[kIBRMS].cal_const_value) / 1000 / 65536;
	printf("Ib rms = 0x%X, %.1f\n", regval, rms);

	readRegister(ICRMS, buf);
	regval = read_S32(buf);
	rms = (*(int*)&regval) * *(cal_reg_table[kICRMS].cal_const_value) / 1000 / 65536;
	printf("Ic rms = 0x%X, %.1f\n", regval, rms);

	readRegister(IDRMS, buf);
	regval = read_S32(buf);
	rms = (*(int*)&regval) * *(cal_reg_table[kIDRMS].cal_const_value) / 1000 / 65536;
	printf("Id rms = 0x%X, %.1f\n", regval, rms);

	readRegister(IERMS, buf);
	regval = read_S32(buf);
	rms = (*(int*)&regval) * *(cal_reg_table[kIERMS].cal_const_value) / 1000 / 65536;
	printf("Ie rms = 0x%X, %.1f\n", regval, rms);

	readRegister(IFRMS, buf);
	regval = read_S32(buf);
	rms = (*(int*)&regval) * *(cal_reg_table[kIFRMS].cal_const_value) / 1000 / 65536;
	printf("If rms = 0x%X, %.1f\n", regval, rms);

	readRegister(VRMS, buf);
	regval = read_S32(buf);
	rms = (*(int*)&regval) * *(cal_reg_table[kVRMS].cal_const_value) / 100 / 65536;
	printf("V rms = 0x%X, %.1f\n", regval, rms);

	return 0;
}

int ADE7816_readEnergyRegisters(void) {
	unsigned char buf[4];
	float energy;
	unsigned int regval;
	char *cal_filename = "cal.txt";

	ADE7816_writeCalConstants(cal_filename);

	readRegister(AWATTHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kAWATTHR].cal_const_value);
	printf("\na Whr = 0x%X, %.3f\n", regval, energy);
	readRegister(AVARHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kAVARHR].cal_const_value);
	printf("a VARhr = 0x%X, %.3f\n", regval, energy);

	readRegister(BWATTHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kBWATTHR].cal_const_value);
	printf("\nb Whr = 0x%X, %.3f\n", regval, energy);
	readRegister(BVARHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kBVARHR].cal_const_value);
	printf("b VARhr = 0x%X, %.3f\n", regval, energy);

	readRegister(CWATTHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kCWATTHR].cal_const_value);
	printf("\nc Whr = 0x%X, %.3f\n", regval, energy);
	readRegister(CVARHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kCVARHR].cal_const_value);
	printf("c VARhr = 0x%X, %.3f\n", regval, energy);

	readRegister(DWATTHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kDWATTHR].cal_const_value);
	printf("\nd Whr = 0x%X, %.3f\n", regval, energy);
	readRegister(DVARHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kDVARHR].cal_const_value);
	printf("d VARhr = 0x%X, %.3f\n", regval, energy);

	readRegister(EWATTHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kEWATTHR].cal_const_value);
	printf("\ne Whr = 0x%X, %.3f\n", regval, energy);
	readRegister(EVARHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kEVARHR].cal_const_value);
	printf("e VARhr = 0x%X, %.3f\n", regval, energy);

	readRegister(FWATTHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kFWATTHR].cal_const_value);
	printf("\nf Whr = 0x%X, %.3f\n", regval, energy);
	readRegister(FVARHR, buf);
	regval = read_S32(buf);
	energy = (*(int*)&regval) * *(cal_reg_table[kFVARHR].cal_const_value);
	printf("f VARhr = 0x%X, %.3f\n", regval, energy);

	return 0;
}

int ADE7816_writeCalConstants(const char *filename) {
	FILE *file;
	int result = 0;
	char line_buffer[BUFSIZ];
	int i;

	if ((file = fopen(filename, "r")) < 0){
	            printf ("Failed to open %s\n", filename);
	            result = -1;
	            goto error;
	    }

	while(fgets(line_buffer, sizeof(line_buffer), file) != NULL)
	{
		char cal_const_name_str[20], value_str[20];
		float calConst;

		sscanf(line_buffer, "%s %s", cal_const_name_str, value_str);
		calConst = strtof(value_str, NULL);
		for (i=0; i<(sizeof(cal_reg_table)/sizeof(cal_t));i++)
		{
			if(strcmp(cal_const_name_str, cal_reg_table[i].regstr) == 0)
			{
				*(cal_reg_table[i].cal_const_value) = calConst;
				break;
			}
		}
	}

	error:
    fclose(file);
    // maybe add an if statement here to do some cleanup or proper stderror output

	return result;
}

int ADE7816_init(void)
{
	char *cal_namebuf = CAL_FILE_NAME;
	char *config_namebuf = CONFIG_FILE_NAME;
	int cal_result = 0;
	int config_result = 0;

	cal_result = ADE7816_writeCalConstants(cal_namebuf);
	config_result = ADE7816_writeMultipleRegisters(config_namebuf);

	return (cal_result + config_result);
}
