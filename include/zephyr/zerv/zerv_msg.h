/*=================================================================================================
 *
 *           ██████╗ ██╗████████╗███╗   ███╗ █████╗ ███╗   ██╗     █████╗ ██████╗
 *           ██╔══██╗██║╚══██╔══╝████╗ ████║██╔══██╗████╗  ██║    ██╔══██╗██╔══██╗
 *           ██████╔╝██║   ██║   ██╔████╔██║███████║██╔██╗ ██║    ███████║██████╔╝
 *           ██╔══██╗██║   ██║   ██║╚██╔╝██║██╔══██║██║╚██╗██║    ██╔══██║██╔══██╗
 *           ██████╔╝██║   ██║   ██║ ╚═╝ ██║██║  ██║██║ ╚████║    ██║  ██║██████╔╝
 *           ╚═════╝ ╚═╝   ╚═╝   ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝    ╚═╝  ╚═╝╚═════╝
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 BitMan AB
 * contact: albin@bitman.se
 *===============================================================================================*/
#ifndef _ZERV_MSG_H_
#define _ZERV_MSG_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include <zephyr/zerv/zerv_internal.h>

/*=================================================================================================
 * ZERV MESSAGE MACROS
 *===============================================================================================*/

/**
 * @brief Macro for declaring a zervice message in a header file.
 */
#define ZERV_MSG_DECL(name, params...)                                                             \
	typedef struct name##_param {                                                              \
		FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), params)                                    \
	} name##_param_t;                                                                          \
	extern zerv_msg_inst_t __##name

/**
 * @brief Macro for defining a zervice message handler function in a source file.
 */
#define ZERV_MSG_HANDLER_DEF(msg_name, params)                                                     \
	zerv_msg_inst_t __##msg_name __aligned(4) = {                                              \
		.name = #msg_name,                                                                 \
		.id = __##msg_name##_id,                                                           \
		.is_locked = ATOMIC_INIT(false),                                                   \
		.handler = (zerv_msg_abstract_handler_t)__##msg_name##_handler,                    \
	};                                                                                         \
	void __##msg_name##_handler(const msg_name##_param_t *params)

/*=================================================================================================
 * ZERVICE CMD CLIENT MACROS
 *===============================================================================================*/

/**
 * @brief Macro for sending a message to a zervice.
 */
#define ZERV_MSG(zervice, msg, retcode, params...)                                                 \
	zerv_rc_t retcode = zerv_internal_client_message_handler(                                  \
		&zervice, &__##msg, sizeof(msg##_param_t), &(msg##_param_t){params})

#endif // _ZERV_MSG_H_
