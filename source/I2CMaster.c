/******************************************************************************
* File Name:  I2CMaster.c
*
* Description:  This file contains all the functions and variables required for
*               proper operation of I2C master peripheral to read and write
*               EEPROM.
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


/* Header file includes */
#include "I2CMaster.h"
#include "stdio.h"
#include "stdlib.h"
/*******************************************************************************
* Macros
*******************************************************************************/
/* I2C master interrupt macros */
#define I2C_INTR_NUM            CYBSP_I2C_IRQ
#define I2C_INTR_PRIORITY       (3UL)
#define LED_DELAY_MS            (500)
/* Command valid status */

/* Combine master error statuses in single mask  */
#define MASTER_ERROR_MASK   (CY_SCB_I2C_MASTER_DATA_NAK | CY_SCB_I2C_MASTER_ADDR_NAK   | \
                            CY_SCB_I2C_MASTER_ARB_LOST | CY_SCB_I2C_MASTER_ABORT_START | \
                            CY_SCB_I2C_MASTER_BUS_ERR)

/*******************************************************************************
* Global variables
*******************************************************************************/
/* Structure for master transfer configuration */
cy_stc_scb_i2c_master_xfer_config_t masterTransferCfg =
{
    .slaveAddress = EEPROM_SLAVE_ADDR,
    .buffer       = NULL,
    .bufferSize   = 0U,
    .xferPending  = false
};

/** The instance-specific context structure.
 * It is used by the driver for internal configuration and
 * data keeping for the I2C. Do not modify anything in this structure.
 */
cy_stc_scb_i2c_context_t CYBSP_I2C_context;

/* Read buffer */
uint8_t readbuffer[READ_SIZE];

/* Write buffer */
uint8_t writebuffer[WRITE_SIZE+2UL];

/*******************************************************************************
* Function Declaration
*******************************************************************************/
void CYBSP_I2C_Interrupt(void);

/*******************************************************************************
* Function Name: CYBSP_I2C_Interrupt
****************************************************************************//**
*
* Summary:
*   Invokes the Cy_SCB_I2C_Interrupt() PDL driver function.
*
*******************************************************************************/
void CYBSP_I2C_Interrupt(void)
{
    Cy_SCB_I2C_Interrupt(CYBSP_I2C_HW, &CYBSP_I2C_context);
}

/*******************************************************************************
* Function Name: WriteToEEPROM
********************************************************************************
* Summary:
* This function writes writeSize number of bytes of sequential data (0 -
* (writeSize-1)) to the EEPROM memory starting from address 0x0000.
*
* Parameters:
*  uint32_t writeSize
*
* Return:
*  uint8_t status
*
*******************************************************************************/
uint8_t WriteToEEPROM(uint32_t writeSize)
{
    uint8_t status = TRANSFER_ERROR;

    cy_en_scb_i2c_status_t  errorStatus;
    uint32_t masterStatus;

    char str[5] = {0};

    /* Timeout counter */
    uint32_t timeout = 1000000UL;

    /* Memory address of the EEPROM */
    writebuffer[0] = EEPROM_START_ADDR_HI;
    writebuffer[1] = EEPROM_START_ADDR_LO;

    if(writeSize != 0)
    {
        /* Send string over serial terminal*/
        Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n Data being written to EEPROM : ");
    }
    /* Append write buffer with data (0 - (writeSize-1)) to write to EEPROM */
    for(uint8_t i = 0 ; i < writeSize; i++)
    {
        writebuffer[i+2]  = i;
        sprintf (str, "%d ", writebuffer[i+2]);
        Cy_SCB_UART_PutString(CYBSP_UART_HW, str);
    }

    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n");

    /* Setup transfer specific parameters */
    masterTransferCfg.buffer     = writebuffer;
    masterTransferCfg.bufferSize = writeSize+2;

    if(writeSize == 0)
         masterTransferCfg.xferPending  = true;

    /* Initiate write transaction */
    errorStatus = Cy_SCB_I2C_MasterWrite(CYBSP_I2C_HW, &masterTransferCfg, &CYBSP_I2C_context);
    if(errorStatus == CY_SCB_I2C_SUCCESS)
    {
        /* Wait until master complete write transfer or time out has occurred */
        do
        {
            masterStatus  = Cy_SCB_I2C_MasterGetStatus(CYBSP_I2C_HW, &CYBSP_I2C_context);
            Cy_SysLib_DelayUs(CY_SCB_WAIT_1_UNIT);
            timeout--;

        } while ((0UL != (masterStatus & CY_SCB_I2C_MASTER_BUSY)) && (timeout > 0));

        if (timeout == 0)
        {
            /* Timeout recovery */
            Cy_SCB_I2C_Disable(CYBSP_I2C_HW, &CYBSP_I2C_context);
            Cy_SCB_I2C_Enable(CYBSP_I2C_HW, &CYBSP_I2C_context);
        }
        else
        {
            if ((0u == (MASTER_ERROR_MASK & masterStatus)) &&
                ((writeSize+2) == Cy_SCB_I2C_MasterGetTransferCount(CYBSP_I2C_HW, &CYBSP_I2C_context)))
            {
                status = TRANSFER_CMPLT;
            }
        }
    }
    return (status);
}

/*******************************************************************************
* Function Name: ReadFromEEPROM
********************************************************************************
* Summary:
* This function reads the readSize number of bytes of data from EEPROM memory
* starting from address 0x0000. It also verifies whether the data read matches
* with the data that was written earlier and returns the "status" appropriately.
*
* Parameters:
*  uint32_t readSize
*
* Return:
*  uint8_t status
*
*******************************************************************************/
uint8_t ReadFromEEPROM(uint32_t readSize)
{
    uint8_t status = TRANSFER_ERROR;
    cy_en_scb_i2c_status_t errorStatus;
    uint32_t masterStatus;

    char str[5] = {0};

    /* Timeout counter */
    uint32_t timeout = 1000000UL;

    /* Write the starting address of memory to read data from EEPROM */
    if(TRANSFER_CMPLT != WriteToEEPROM(0))
        return INVALID_DATA_ERROR;

    /* Setup transfer specific parameters */
    masterTransferCfg.buffer     = readbuffer;
    masterTransferCfg.bufferSize = readSize;
    masterTransferCfg.xferPending = false;

    /* Initiate read transaction */
    errorStatus = Cy_SCB_I2C_MasterRead(CYBSP_I2C_HW, &masterTransferCfg, &CYBSP_I2C_context);
    if(errorStatus == CY_SCB_I2C_SUCCESS)
    {
        /* Wait until master complete read transfer or time out has occurred */
        do
        {
            masterStatus  = Cy_SCB_I2C_MasterGetStatus(CYBSP_I2C_HW, &CYBSP_I2C_context);
            Cy_SysLib_DelayUs(CY_SCB_WAIT_1_UNIT);
            timeout--;

        } while ((0UL != (masterStatus & CY_SCB_I2C_MASTER_BUSY)) && (timeout > 0));

        if (timeout == 0)
        {
            /* Timeout recovery */
            Cy_SCB_I2C_Disable(CYBSP_I2C_HW, &CYBSP_I2C_context);
            Cy_SCB_I2C_Enable(CYBSP_I2C_HW, &CYBSP_I2C_context);
        }
        else
        {
            /* Check transfer status */
            if (0u == (MASTER_ERROR_MASK & masterStatus))
            {
                status = TRANSFER_CMPLT;
            }
        }

        /* Send string over serial terminal*/
        Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n Data read back from EEPROM : ");

        /* Verify if data read back matches with the data written to EEPROM */
        for(uint8_t i = 0; i < readSize; i++)
        {
            sprintf (str, "%d ", readbuffer[i]);
            Cy_SCB_UART_PutString(CYBSP_UART_HW, str);
            if(readbuffer[i] != writebuffer[i+2])
            {
                status = INVALID_DATA_ERROR;
                break;
            }

        }
        Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n");
    }
    return (status);
}

/*******************************************************************************
* Function Name: initI2CMaster
********************************************************************************
* Summary:
*  This function initializes and enables SCB as I2C Master
*
* Parameters:
*  none
*
* Return:
*  uint32_t
*
*******************************************************************************/
uint32_t initI2CMaster(void)
{
    cy_en_scb_i2c_status_t initStatus;
    cy_en_sysint_status_t sysStatus;
    cy_stc_sysint_t CYBSP_I2C_SCB_IRQ_cfg =
    {
            /*.intrSrc =*/ CYBSP_I2C_IRQ,
            /*.intrPriority =*/ 3u
    };

    /*Initialize and enable the I2C in master mode*/
    initStatus = Cy_SCB_I2C_Init(CYBSP_I2C_HW, &CYBSP_I2C_config, &CYBSP_I2C_context);
    if(initStatus != CY_SCB_I2C_SUCCESS)
    {
        return I2C_FAILURE;
    }

    /* Hook interrupt service routine */
    sysStatus = Cy_SysInt_Init(&CYBSP_I2C_SCB_IRQ_cfg, &CYBSP_I2C_Interrupt);
    if(sysStatus != CY_SYSINT_SUCCESS)
    {
        return I2C_FAILURE;
    }
    NVIC_EnableIRQ((IRQn_Type) CYBSP_I2C_SCB_IRQ_cfg.intrSrc);

    /*Enable the I2C in master mode*/
    Cy_SCB_I2C_Enable(CYBSP_I2C_HW, &CYBSP_I2C_context);
    return I2C_SUCCESS;
}

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
*  Function to handle errors
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void handle_error(void)
{
    /* Disable all interrupts. */
    __disable_irq();

    /* Infinite loop. */
    while(1u) {}
}

/*******************************************************************************
* Function Name: BlinkUserLED
********************************************************************************
* Summary:
*  This function is used to blink on-board User LED "count" number of times with
*  1Hz frequency
*
* Parameters:
*  uint8_t number
*
* Return:
*  none
*
*******************************************************************************/
void BlinkUserLED(uint8_t count)
{
    /* Loop to blink LED number of times specified by 'count' variable */
    for(uint8_t i = 0; i < count; i++)
    {
        Cy_GPIO_Clr(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
        Cy_SysLib_Delay(LED_DELAY_MS);

        Cy_GPIO_Set(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
        Cy_SysLib_Delay(LED_DELAY_MS);
    }
}
