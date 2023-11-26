/*=================================================================================================
 *                _____           _                 ______    _          _ _
 *               / ____|         | |               |  ____|  | |        (_) |
 *              | (___  _   _ ___| |_ ___ _ __ ___ | |__ __ _| |__  _ __ _| | _____ _ __
 *               \___ \| | | / __| __/ _ \ '_ ` _ \|  __/ _` | '_ \| '__| | |/ / _ \ '_ \
 *               ____) | |_| \__ \ ||  __/ | | | | | | | (_| | |_) | |  | |   <  __/ | | |
 *              |_____/ \__, |___/\__\___|_| |_| |_|_|  \__,_|_.__/|_|  |_|_|\_\___|_| |_|
 *                       __/ |
 *                      |___/
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 Systemfabriken AB
 * contact: albin@systemfabriken.tech
 *===============================================================================================*/
#ifndef _ZERV_CMD_H_
#define _ZERV_CMD_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include <zephyr/zerv/zerv_internal.h>

/*=================================================================================================
 * ZERV COMMAND MACROS
 *===============================================================================================*/

/**
 * @brief Macro for declaring a zervice command input parameter.
 *
 * Should be used in the ZERV_CMD_DECL macro to declare the input parameters of a command.
 *
 * @param ... The types and names of the input parameters.
 */
#define ZERV_IN(...) FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

/**
 * @brief Macro for declaring an empty zervice command input parameter.
 */
#define ZERV_IN_EMPTY ZERV_IN(char __empty)

/**
 * @brief Macro for declaring a zervice command output parameter.
 *
 * Should be used in the ZERV_CMD_DECL macro to declare the output parameters of a command.
 *
 * @param ... The types and names of the output parameters.
 */
#define ZERV_OUT(...) FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

/**
 * @brief Macro for declaring an empty zervice command output parameter.
 */
#define ZERV_OUT_EMPTY ZERV_OUT(char __empty)

/**
 * @brief Macro for declaring a zervice command in a header file.
 *
 * The macro should be used in the header file. A zervice command can be issued by the client
 * application to request a service from the zervice. The command is declared with a name, input
 * parameters and output parameters. The macros ZERV_IN and ZERV_OUT should be used to declare the
 * input and output parameters.
 *
 * @param name The name of the command.
 * @param in The input parameters of the command. Should be declared with the ZERV_IN macro.
 * @param out The output parameters of the command. Should be declared with the ZERV_OUT macro.
 *
 * @note The command must be defined in the source file with the ZERV_CMD_HANDLER_DEF macro.
 */
#define ZERV_CMD_DECL(name, in, out)                                                               \
	typedef struct name##_param {                                                              \
		in                                                                                 \
	} name##_param_t;                                                                          \
	typedef void (*name##_resp_handler_t)(void);                                               \
	typedef struct name##_ret {                                                                \
		zerv_rc_t rc;                                                                      \
		out                                                                                \
	} name##_ret_t;                                                                            \
	extern zerv_cmd_inst_t __##name

/**
 * @brief Macro for defining a zervice request handler function in a source file.
 *
 * @param cmd_name The name of the request.
 * @param in Pointer to the input parameters of the request. The input type is defined by the
 * ZERV_IN macro used when declaring the command.
 * @param out Pointer to the output parameters of the request. The output type is defined by the
 * ZERV_OUT macro used when declaring the command.
 */
#define ZERV_CMD_HANDLER_DEF(cmd_name, in, out)                                                    \
	static K_SEM_DEFINE(__##cmd_name##_future_sem, 0, 1);                                      \
	zerv_cmd_inst_t __##cmd_name __aligned(4) = {                                              \
		.name = #cmd_name,                                                                 \
		.id = __##cmd_name##_id,                                                           \
		.is_locked = ATOMIC_INIT(false),                                                   \
		.handler = (zerv_cmd_abstract_handler_t)__##cmd_name##_handler,                    \
	};                                                                                         \
	zerv_rc_t __##cmd_name##_handler(const cmd_name##_param_t *in, cmd_name##_ret_t *out)

/*=================================================================================================
 * ZERVICE CMD CLIENT MACROS
 *===============================================================================================*/

/**
 * @brief Macro for commanding a zervice to handle a request.
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
 * @param[in] params... The arguments to the command. The arguments should follow the
 * format specified by the ZERV_IN macro used when declaring the command.
 */
#define ZERV_CALL(zervice, cmd, retcode, p_ret, params...)                                         \
	cmd##_ret_t __##cmd##_response;                                                            \
	cmd##_ret_t *p_ret = &__##cmd##_response;                                                  \
	zerv_rc_t retcode = zerv_internal_client_request_handler(                                  \
		&zervice, &__##cmd, sizeof(cmd##_param_t), &(cmd##_param_t){params},               \
		(zerv_cmd_out_base_t *)p_ret, sizeof(cmd##_ret_t));

#endif // _ZERV_CMD_H_
