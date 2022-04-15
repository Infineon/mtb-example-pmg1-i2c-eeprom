/******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the PMG1: I2C EEPROM code example
*
* Related Document: See README.md
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


/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cybsp.h"
#include "cyhal.h"
#include "cy_pdl.h"
#include "stdio.h"
#include "I2CMaster.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define CY_ASSERT_FAILED          (0u)

/*******************************************************************************
* Global Variable
*******************************************************************************/
uint32_t interrupt_flag = 0u;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void Switch_IntHandler(void);

/******************************************************************************
 * Switch interrupt configuration structure
 *****************************************************************************/
const cy_stc_sysint_t switch_interrupt_config =
{
    .intrSrc = CYBSP_USER_BTN_IRQ,             //Source of interrupt signal
    .intrPriority = 3u,
};


/*******************************************************************************
* Function Name: Switch_IntHandler
********************************************************************************
*
* Summary:
*  This function is executed when interrupt is triggered through the switch.
*
*******************************************************************************/

void Switch_IntHandler(void)
{

    /* Clears the triggered pin interrupt */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_NUM);
    NVIC_ClearPendingIRQ(switch_interrupt_config.intrSrc);

    /* Set interrupt flag */
    interrupt_flag = 1u;
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function performs
*  - initial setup of device
*  - configure the SCB block as UART interface
*  - prints out "Hello World" via UART interface
*  - Blinks an LED under firmware control at 1 Hz
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    cy_stc_scb_uart_context_t CYBSP_UART_context;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(CY_ASSERT_FAILED);
    }

    /* Configure and enable the UART peripheral */
    Cy_SCB_UART_Init(CYBSP_UART_HW, &CYBSP_UART_config, &CYBSP_UART_context);
    Cy_SCB_UART_Enable(CYBSP_UART_HW);

    /* Enable global interrupts */
    __enable_irq();

    uint32_t status;

    /*I2C master initialisation*/
    status = initI2CMaster();
    if(status != I2C_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /*User switch (CYBSP_USER_BTN) interrupt initialization*/
    result = Cy_SysInt_Init(&switch_interrupt_config, &Switch_IntHandler);
    if (result != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    NVIC_ClearPendingIRQ(switch_interrupt_config.intrSrc);

    /*Enabling the User switch interrupt*/
    NVIC_EnableIRQ(switch_interrupt_config.intrSrc);

    /* Send a string over serial terminal */
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\x1b[2J\x1b[;H");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n**************PMG1 MCU:I2C EEPROM**************\r\n");

    for(;;)
    {
        if (interrupt_flag == 1)
        {
            Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n Start writing to EEPROM \r\n");

            /* Write WRITE_SIZE bytes to EEPROM memory starting from address 0x0000  */
            if (TRANSFER_CMPLT == WriteToEEPROM(WRITE_SIZE))
            {
                Cy_SysLib_Delay(5);

                Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n Start reading back from EEPROM \r\n");

                /* Read back and verify written data from EEPROM*/
                status = ReadFromEEPROM(READ_SIZE);

                if (status == TRANSFER_CMPLT)
                {
                    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n Read back successful \r\n");

                    /*Blink User LED (CYBSP_USER_LED) once if read back is successful*/
                    BlinkUserLED(1);
                }
                else if(status == INVALID_DATA_ERROR)
                {
                    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n Mismatch between data written and read back \r\n");

                    /*Blink User LED (CYBSP_USER_LED) twice if read back returns invalid data*/
                    BlinkUserLED(2);
                }
            }
            else
            {
                /*Blink User LED (CYBSP_USER_LED) thrice if writing to EEPROM failed*/
                BlinkUserLED(3);

                Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n Writing to EEPROM failed \r\n");
            }

            /* Clear the interrupt*/
            interrupt_flag = 0u;
        }
    }
}

/* [] END OF FILE */

