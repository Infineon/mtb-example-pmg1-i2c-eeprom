/******************************************************************************
* File Name:  I2CMaster.h
*
* Description:  This file provides constants and parameter values for the I2C
*               Master peripheral to read and write EEPROM.
*
* Related Document: See Readme.md
*
*******************************************************************************
* Copyright 2022, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/


#ifndef SOURCE_I2CMASTER_H_
#define SOURCE_I2CMASTER_H_

#include "cy_pdl.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/

#define I2C_SUCCESS             (0UL)
#define I2C_FAILURE             (1UL)

#define TRANSFER_CMPLT          (0x00UL)
#define READ_CMPLT              (TRANSFER_CMPLT)
#define TRANSFER_ERROR          (0xFFUL)
#define INVALID_DATA_ERROR      (0x0FUL)
#define READ_ERROR              (TRANSFER_ERROR)

#define WRITE_SIZE              (0x40UL)
#define READ_SIZE               (WRITE_SIZE)

#define EEPROM_START_ADDR_HI    (0x00)
#define EEPROM_START_ADDR_LO    (0x00)

#define EEPROM_SLAVE_ADDR       (0x51)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
uint8_t WriteToEEPROM(uint32_t writeSize);
uint8_t ReadFromEEPROM(uint32_t readSize);
uint32_t initI2CMaster(void);
void handle_error(void);
void BlinkUserLED(uint8_t num);

#endif /* SOURCE_I2CMASTER_H_ */
