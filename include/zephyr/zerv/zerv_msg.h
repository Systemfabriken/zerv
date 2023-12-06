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

#define ZERV_MSG_RAW_DECL(name) extern zerv_msg_inst_t __##name

/**
 * @brief Macro for defining a zervice message handler function in a source file.
 *
 * @param msg_name The name of the message.
 * @param params The name of the parameters of the message.
 *
 * @note The message handler function must be defined in the same source file as the zervice
 * 	 definition.
 */
#define ZERV_MSG_HANDLER_DEF(msg_name, params)                                                     \
	__unused static void __##msg_name##_handler(const msg_name##_param_t *params);             \
	zerv_msg_inst_t __##msg_name __aligned(4) = {                                              \
		.name = #msg_name,                                                                 \
		.id = __##msg_name##_id,                                                           \
		.is_locked = ATOMIC_INIT(false),                                                   \
		.handler = (zerv_msg_abstract_handler_t)__##msg_name##_handler,                    \
		.is_raw = false,                                                                   \
		.raw_handler = NULL};                                                              \
	void __##msg_name##_handler(const msg_name##_param_t *params)

/**
 * @brief Macro for defining a zervice message handler function in a source file.
 *
 * @param msg_name The name of the message.
 * @param size_name The name of the size parameter.
 * @param data_name The name of the data parameter.
 *
 * @note The message handler function must be defined in the same source file as the zervice
 * 	 definition.
 */
#define ZERV_MSG_RAW_HANDLER_DEF(msg_name, size_name, data_name)                                   \
	__unused static void __##msg_name##_raw_handler(size_t size_name, void *data_name);        \
	zerv_msg_inst_t __##msg_name __aligned(4) = {                                              \
		.name = #msg_name,                                                                 \
		.id = __##msg_name##_id,                                                           \
		.is_locked = ATOMIC_INIT(false),                                                   \
		.handler = NULL,                                                                   \
		.is_raw = true,                                                                    \
		.raw_handler = (zerv_raw_msg_abstract_handler_t)__##msg_name##_raw_handler};       \
	void __##msg_name##_raw_handler(size_t size_name, void *data_name)

/*=================================================================================================
 * ZERVICE CMD CLIENT MACROS
 *===============================================================================================*/

/**
 * @brief Macro for sending a message to a zervice.
 */
#define ZERV_MSG(zervice, msg, retcode, params...)                                                 \
	zerv_rc_t retcode = zerv_internal_client_message_handler(                                  \
		&zervice, &__##msg, sizeof(msg##_param_t), &(msg##_param_t){params})

/**
 * @brief Macro for sending a message to a zervice with a pointer to a message struct.
 *
 * @param zervice The zervice to send the message to.
 * @param msg The name of the message.
 * @param retcode The return code variable.
 * @param p_msg_struct Pointer to the message struct.
 *
 * @note This macro is used when the message struct is too large to be copied on the stack.
 * 	 The message struct must be allocated on the heap and the pointer to it is passed
 * 	 to the zervice.
 */
#define ZERV_MSG_RAW(zervice, msg, retcode, size, data)                                            \
	zerv_rc_t retcode = zerv_internal_client_message_handler(&zervice, &__##msg, size, data)

#endif /* _ZERV_MSG_H_ */
