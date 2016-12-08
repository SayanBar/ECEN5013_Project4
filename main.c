/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */



#include <stdio.h>
#include "MKL25Z4.h"
//#include "capsense.h"
#include "zero.h"
#include "ftoa.h"
//#include "rtc.h"
//#include "i2c.h"
#include "derivative.h" 				// Include peripheral declarations
#include "I2C.h"
#include <math.h>
//#include "display_temp.h"

/******************************************************************************
* Constants and macros
******************************************************************************/

//MMA8451Q Registers

#define STATUS_REG            0x00		// STATUS Register

#define OUT_X_MSB_REG         0x01		// [7:0] are 8 MSBs of the 14-bit X-axis sample
#define OUT_X_LSB_REG         0x02		// [7:2] are the 6 LSB of 14-bit X-axis sample
#define OUT_Y_MSB_REG         0x03		// [7:0] are 8 MSBs of the 14-bit Y-axis sample
#define OUT_Y_LSB_REG         0x04		// [7:2] are the 6 LSB of 14-bit Y-axis sample
#define OUT_Z_MSB_REG         0x05		// [7:0] are 8 MSBs of the 14-bit Z-axis sample
#define OUT_Z_LSB_REG         0x06		// [7:2] are the 6 LSB of 14-bit Z-axis sample

#define F_SETUP_REG           0x09    	// F_SETUP FIFO Setup Register
#define TRIG_CFG_REG          0x0A    	// TRIG_CFG Map of FIFO data capture events
#define SYSMOD_REG            0x0B    	// SYSMOD System Mode Register
#define INT_SOURCE_REG        0x0C    	// INT_SOURCE System Interrupt Status Register
#define WHO_AM_I_REG          0x0D    	// WHO_AM_I Device ID Register
#define XYZ_DATA_CFG_REG      0x0E    	// XYZ_DATA_CFG Sensor Data Configuration Register
#define HP_FILTER_CUTOFF_REG  0x0F    	// HP_FILTER_CUTOFF High Pass Filter Register

#define PL_STATUS_REG         0x10    	// PL_STATUS Portrait/Landscape Status Register
#define PL_CFG_REG            0x11    	// PL_CFG Portrait/Landscape Configuration Register
#define PL_COUNT_REG          0x12    	// PL_COUNT Portrait/Landscape Debounce Register
#define PL_BF_ZCOMP_REG       0x13    	// PL_BF_ZCOMP Back/Front and Z Compensation Register
#define P_L_THS_REG           0x14    	// P_L_THS Portrait to Landscape Threshold Register

#define FF_MT_CFG_REG         0x15    	// FF_MT_CFG Freefall and Motion Configuration Register
#define FF_MT_SRC_REG         0x16    	// FF_MT_SRC Freefall and Motion Source Register
#define FT_MT_THS_REG         0x17    	// FF_MT_THS Freefall and Motion Threshold Register
#define FF_MT_COUNT_REG       0x18    	// FF_MT_COUNT Freefall Motion Count Register

#define TRANSIENT_CFG_REG     0x1D    	// TRANSIENT_CFG Transient Configuration Register
#define TRANSIENT_SRC_REG     0x1E    	// TRANSIENT_SRC Transient Source Register
#define TRANSIENT_THS_REG     0x1F    	// TRANSIENT_THS Transient Threshold Register
#define TRANSIENT_COUNT_REG   0x20    	// TRANSIENT_COUNT Transient Debounce Counter Register

#define PULSE_CFG_REG         0x21    	// PULSE_CFG Pulse Configuration Register
#define PULSE_SRC_REG         0x22    	// PULSE_SRC Pulse Source Register
#define PULSE_THSX_REG        0x23    	// PULSE_THS XYZ Pulse Threshold Registers
#define PULSE_THSY_REG        0x24
#define PULSE_THSZ_REG        0x25
#define PULSE_TMLT_REG        0x26    	// PULSE_TMLT Pulse Time Window Register
#define PULSE_LTCY_REG        0x27    	// PULSE_LTCY Pulse Latency Timer Register
#define PULSE_WIND_REG        0x28    	// PULSE_WIND Second Pulse Time Window Register

#define ASLP_COUNT_REG        0x29    	// ASLP_COUNT Auto Sleep Inactivity Timer Register

#define CTRL_REG1             0x2A    	// CTRL_REG1 System Control 1 Register
#define CTRL_REG2             0x2B    	// CTRL_REG2 System Control 2 Register
#define CTRL_REG3             0x2C    	// CTRL_REG3 Interrupt Control Register
#define CTRL_REG4             0x2D    	// CTRL_REG4 Interrupt Enable Register
#define CTRL_REG5             0x2E    	// CTRL_REG5 Interrupt Configuration Register

#define OFF_X_REG             0x2F    	// XYZ Offset Correction Registers
#define OFF_Y_REG             0x30
#define OFF_Z_REG             0x31

//MMA8451Q 7-bit I2C address

#define MMA845x_I2C_ADDRESS   0x1D		// SA0 pin = 1 -> 7-bit I2C address is 0x1D

//MMA8451Q Sensitivity at +/-2g

#define SENSITIVITY_2G		  4096

#define PI					  3.14159

/******************************************************************************
* Global variables
******************************************************************************/

unsigned char AccData[6];
short Xout_14_bit, Yout_14_bit, Zout_14_bit;
float Xout_g, Yout_g, Zout_g, Pitch, Roll;
char DataReady;
char Xoffset, Yoffset, Zoffset;

/******************************************************************************
* Functions
******************************************************************************/

void MCU_Init(void);
void Accelerometer_Init (void);
void Calibrate(void);

void MCU_Init(void)
{
	//I2C0 module initialization
	SIM_SCGC4 |= SIM_SCGC4_I2C0_MASK;		// Turn on clock to I2C0 module
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;		// Turn on clock to Port E module
	PORTE_PCR24 = PORT_PCR_MUX(5);			// PTE24 pin is I2C0 SCL line
	PORTE_PCR25 = PORT_PCR_MUX(5);			// PTE25 pin is I2C0 SDA line
	I2C0_F  = 0x14; 						// SDA hold time = 2.125us, SCL start hold time = 4.25us, SCL stop hold time = 5.125us *
	I2C0_C1 = I2C_C1_IICEN_MASK;    		// Enable I2C0 module

	//Configure the PTA14 pin (connected to the INT1 of the MMA8451Q) for falling edge interrupts
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;		// Turn on clock to Port A module
	PORTA_PCR14 |= (0|PORT_PCR_ISF_MASK|	// Clear the interrupt flag
					  PORT_PCR_MUX(0x1)|	// PTA14 is configured as GPIO
					  PORT_PCR_IRQC(0xA));	// PTA14 is configured for falling edge interrupts

	//Enable PORTA interrupt on NVIC
//	 NVIC_ICPR |= 1 << ((INT_PORTA - 16)%32);
//	     NVIC_ISER |= 1 << ((INT_PORTA - 16)%32);
	//NVIC_EnableIRQ(PORTA_IRQn);
	//Initialize the red LED (PTB18)
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;		/* Turn on clock to Port B module */
	PORTB_PCR18 = PORT_PCR_MUX(1);			/* PTB18 pin is GPIO */
						/* Turn on the red LED */
	//GPIOB_PDDR |= 1<<18;					/* PTB18 pin is output */

}

/******************************************************************************
* Accelerometer initialization function
******************************************************************************/

void Accelerometer_Init (void)
{
	unsigned char reg_val = 0;

	I2C_WriteRegister(MMA845x_I2C_ADDRESS, CTRL_REG2, 0x40);		// Reset all registers to POR values

	do		// Wait for the RST bit to clear
	{
		reg_val = I2C_ReadRegister(MMA845x_I2C_ADDRESS, CTRL_REG2) & 0x40;
	} 	while (reg_val);

	I2C_WriteRegister(MMA845x_I2C_ADDRESS, XYZ_DATA_CFG_REG, 0x00);		// +/-2g range -> 1g = 16384/4 = 4096 counts
	I2C_WriteRegister(MMA845x_I2C_ADDRESS, CTRL_REG2, 0x02);		// High Resolution mode
	I2C_WriteRegister(MMA845x_I2C_ADDRESS, CTRL_REG1, 0x3D);	// ODR = 1.56Hz, Reduced noise, Active mode
}

/******************************************************************************
* Simple offset calibration
******************************************************************************/

void Calibrate (void)
{
	unsigned char reg_val = 0;

	while (!reg_val)		// Wait for a first set of data
	{
		reg_val = I2C_ReadRegister(MMA845x_I2C_ADDRESS, STATUS_REG) & 0x08;
	}

	I2C_ReadMultiRegisters(MMA845x_I2C_ADDRESS, OUT_X_MSB_REG, 6, AccData);		// Read data output registers 0x01-0x06

	Xout_14_bit = ((short) (AccData[0]<<8 | AccData[1])) >> 2;		// Compute 14-bit X-axis output value
	Yout_14_bit = ((short) (AccData[2]<<8 | AccData[3])) >> 2;		// Compute 14-bit Y-axis output value
	Zout_14_bit = ((short) (AccData[4]<<8 | AccData[5])) >> 2;		// Compute 14-bit Z-axis output value


	Xoffset = Xout_14_bit / 8 * (-1);		// Compute X-axis offset correction value
	Yoffset = Yout_14_bit / 8 * (-1);		// Compute Y-axis offset correction value
	Zoffset = (Zout_14_bit - SENSITIVITY_2G) / 8 * (-1);		// Compute Z-axis offset correction value

	I2C_WriteRegister(MMA845x_I2C_ADDRESS, CTRL_REG1, 0x00);		// Standby mode to allow writing to the offset registers
	I2C_WriteRegister(MMA845x_I2C_ADDRESS, OFF_X_REG, Xoffset);
	I2C_WriteRegister(MMA845x_I2C_ADDRESS, OFF_Y_REG, Yoffset);
	I2C_WriteRegister(MMA845x_I2C_ADDRESS, OFF_Z_REG, Zoffset);
	I2C_WriteRegister(MMA845x_I2C_ADDRESS, CTRL_REG3, 0x00);		// Push-pull, active low interrupt
	I2C_WriteRegister(MMA845x_I2C_ADDRESS, CTRL_REG4, 0x01);		// Enable DRDY interrupt
	I2C_WriteRegister(MMA845x_I2C_ADDRESS, CTRL_REG5, 0x01);		// DRDY interrupt routed to INT1 - PTA14
	I2C_WriteRegister(MMA845x_I2C_ADDRESS, CTRL_REG1, 0x3D);		// ODR = 1.56Hz, Reduced noise, Active mode
}

/******************************************************************************
* PORT A Interrupt handler
******************************************************************************/

void PORTA_IRQHandler()
{
	PORTA_PCR14 |= PORT_PCR_ISF_MASK;			// Clear the interrupt flag
	//GPIOB_PCOR |= 1<<18;
	GPIOB_PDDR |= 1<<18;
	DataReady = 1;
}





int main (void)
{
	DataReady = 0;
	MCU_Init();
  	Accelerometer_Init();
  	Calibrate();
  	//tsi_init((1 << 0) | (1 << 0));
  	//tsi_data();

  	while(1)
    {
		//if (DataReady)		// Is a new set of data ready?
		//{
			DataReady = 0;
			//GPIOB_PTOR |= 1<<18;

			I2C_ReadMultiRegisters(MMA845x_I2C_ADDRESS, OUT_X_MSB_REG, 6, AccData);		// Read data output registers 0x01-0x06

			Xout_14_bit = ((short) (AccData[0]<<8 | AccData[1])) >> 2;		// Compute 14-bit X-axis output value
			Yout_14_bit = ((short) (AccData[2]<<8 | AccData[3])) >> 2;		// Compute 14-bit Y-axis output value
			Zout_14_bit = ((short) (AccData[4]<<8 | AccData[5])) >> 2;		// Compute 14-bit Z-axis output value

			Xout_g = ((float) Xout_14_bit) / SENSITIVITY_2G;		// Compute X-axis output value in g's
			Yout_g = ((float) Yout_14_bit) / SENSITIVITY_2G;		// Compute Y-axis output value in g's
			Zout_g = ((float) Zout_14_bit) / SENSITIVITY_2G;		// Compute Z-axis output value in g's

			if((Zout_g<0) && (Yout_g>0))
				GPIOB_PDDR |= 1<<18;
			if((Zout_g<0) && (Yout_g<0))
			{
				GPIOB_PDDR &=~GPIOB_PDDR;
				UART0_init();
				Calc_temp();
			}


			Pitch = atan2 (-Xout_g, sqrt (Yout_g*Yout_g + Zout_g*Zout_g)) * 180 / PI;		// Equation 37 in the AN3461
			if (Zout_g > 0)																	// Equation 38 in the AN3461
				Roll = atan2 (Yout_g, sqrt (0.01*Xout_g*Xout_g + Zout_g*Zout_g)) * 180 / PI;
			else
				Roll = atan2 (Yout_g, - sqrt (0.01*Xout_g*Xout_g + Zout_g*Zout_g)) * 180 / PI;
		//}
	}
}




