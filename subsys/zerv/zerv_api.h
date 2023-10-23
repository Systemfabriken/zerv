/*=================================================================================================
 *                _____           _                 ______    _          _ _
 *               / ____|         | |               |  ____|  | |        (_) |
 *              | (___  _   _ ___| |_ ___ _ __ ___ | |__ __ _| |__  _ __ _| | _____ _ __
 *               \___ \| | | / __| __/ _ \ '_ ` _ \|  __/ _` | '_ \| '__| | |/ / _ \ '_ \
 *               ____) | |_| \__ \ ||  __/ | | | | | | | (_| | |_) | |  | |   <  __/ | | |
 *              |_____/ \__, |___/\__\___|_| |_| |_|_|  \__,_|_.__/|_|  |_|_|\_\___|_| |_|
 *                       __/ |
 *                      |___/
 * Description:
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 Systemfabriken AB
 * contact: albin@systemfabriken.tech
 *===============================================================================================*/
#ifndef _ZERV_API_H_
#define _ZERV_API_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include "zerv_api_impl.h"

/*=================================================================================================
 * PUBLIC MACROS
 *===============================================================================================*/

/**
 * @brief Call a zervice command.
 *
 * Request service from the zervice with the given name and command name. If the call is supposed to
 * generate a response, the response will be stored in the memory pointed to by the
 * response_handle_name parameter.
 *
 * @param zervice_name The name of the zervice to call.
 * @param command_name The name of the command to call.
 * @param[out] return_code_name The identifier of the variable to store the return code in. The
 * variable is defined by the macro.
 * @param[out] response_handle_name The identifier of the pointer to the response storage, will be
 * NULL if no response is expected. The pointer is defined by the macro.
 * @param[in] ... The arguments to the command.
 *
 * @return int Is returned in the return_code_name variable which is defined by the macro.
 */
#define ZERV_CALL(zervice_name, command_name, return_code_name, response_handle_name, ...)         \
	__ZERV_IMPL_CALL(zervice_name, command_name, return_code_name, response_handle_name, ...)

/*=================================================================================================
 * PUBLIC FUNCTION DECLARATIONS
 *===============================================================================================*/

#endif /* _ZERV_API_H_ */