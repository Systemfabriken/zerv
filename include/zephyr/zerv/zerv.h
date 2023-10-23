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
#ifndef _ZERV_H_
#define _ZERV_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include "zerv_api_impl.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=================================================================================================
 * PUBLIC MACROS
 *===============================================================================================*/
#define __ZERV_IMPL_STRUCT_MEMBER(field) field;

#define ZERV_CMD_PARAM(...) FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

#define ZERV_CMD_RETURN(...) FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

#define ZERV_CMD_REGISTER(zervice, name, param, ret)                                               \
	typedef struct __##zervice##_##name##_param {                                              \
		param                                                                              \
	} __##zervice##_##name##_param_t;                                                          \
	typedef struct __##zervice##_##name##_ret {                                                \
		ret                                                                                \
	} __##zervice##_##name##_ret_t

/**
 * @brief Call a zervice command.
 *
 * Request service from the zervice with the given name and command name. If the call is supposed to
 * generate a response, the response will be stored in the memory pointed to by the
 * response_handle_name parameter.
 *
 * @param zervice The name of the zervice to call.
 * @param cmd The name of the command to call.
 * @param[out] retcode The identifier of the variable to store the return code in. The
 * variable is defined by the macro.
 * @param[out] p_ret The identifier of the pointer to the response storage, will be
 * NULL if no response is expected. The pointer is defined by the macro.
 * @param[in] ... The arguments to the command. The arguments should follow the format
 * specified by the ZERV_CMD_PARAM macro.
 *
 * @return int Is returned in the ret variable which is defined by the macro.
 */
#define ZERV_CALL(zervice, cmd, retcode, p_ret, ...)                                               \
	__ZERV_IMPL_CALL(zervice, cmd, retcode, p_ret, ##__VA_ARGS__)

/*=================================================================================================
 * PUBLIC FUNCTION DECLARATIONS
 *===============================================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _ZERV_H_ */