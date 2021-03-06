/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** USBX Component                                                        */
/**                                                                       */
/**   STM32 Controller Driver                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE
#define UX_DCD_STM32_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_dcd_stm32.h"
#include "ux_device_stack.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_stm32_endpoint_status                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will retrieve the status of the endpoint.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_stm32                             Pointer to device controller  */
/*    endpoint_index                        Endpoint index                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    STM32 Controller Driver                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s), used ST  */
/*                                            HAL library to drive the    */
/*                                            controller,                 */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _ux_dcd_stm32_endpoint_status(UX_DCD_STM32 *dcd_stm32, ULONG endpoint_index)
{

UX_DCD_STM32_ED      *ed;

    /* Fetch the address of the physical endpoint.  */
#ifdef UX_DEVICE_BIDIRECTIONAL_ENDPOINT_SUPPORT
ULONG                   ed_addr =  endpoint_index; /* Passed value as endpoint address.  */
ULONG                   ed_dir  =  ed_addr & UX_ENDPOINT_DIRECTION;
ULONG                   ed_index = ed_addr & ~UX_ENDPOINT_DIRECTION;

    /* Fetch the address of the physical endpoint.  */
    ed = ((ed_addr == 0) ? &dcd_stm32 -> ux_dcd_stm32_ed[0] :
            ((ed_dir) ? &dcd_stm32 -> ux_dcd_stm32_ed_in[ed_index] :
                        &dcd_stm32 -> ux_dcd_stm32_ed[ed_index]));
#else
    ed =  &dcd_stm32 -> ux_dcd_stm32_ed[endpoint_index];
#endif

    /* Check the endpoint status, if it is free, we have a illegal endpoint.  */
    if ((ed -> ux_dcd_stm32_ed_status & UX_DCD_STM32_ED_STATUS_USED) == 0)
        return(UX_ERROR);

    /* Check if the endpoint is stalled.  */
    if ((ed -> ux_dcd_stm32_ed_status & UX_DCD_STM32_ED_STATUS_STALLED) == 0)
        return(UX_FALSE);
    else
        return(UX_TRUE);
}

