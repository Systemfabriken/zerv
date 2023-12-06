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
#ifndef _ZERV_TOPIC_H_
#define _ZERV_TOPIC_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include <zephyr/zerv/zerv_internal.h>
#include <zephyr/sys/slist.h>

/*=================================================================================================
 * ZERV TOPIC MACROS
 *===============================================================================================*/

/**
 * @brief Macro for declaring a zervice topic in a header file.
 *
 * @param name The name of the topic.
 * @param params The parameters of the topic.
 *
 * @note The topic must be defined in a source file using the ZERV_TOPIC_DEF macro.
 */
#define ZERV_TOPIC_DECL(name, params...)                                                           \
	typedef struct {                                                                           \
		FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), params)                                    \
	} name##_zerv_topic_t;                                                                     \
	extern sys_slist_t name##_subscribers;

/**
 * @brief Macro for defining a zervice topic in a source file.
 *
 * @param name The name of the topic.
 */
#define ZERV_TOPIC_DEF(name) __unused sys_slist_t name##_subscribers = {NULL};

/**
 * @brief Macro for defining a zervice message handler function in a source file.
 *
 * @param msg_name The name of the message.
 * @param params The name of the parameters of the message.
 *
 * @note The message handler function must be defined in the same source file as the zervice
 * 	 definition.
 */
#define ZERV_TOPIC_HANDLER(zervice_name, topic, params)                                            \
	__unused static void __##zervice_name##_##topic##_handler(                                 \
		const topic##_zerv_topic_t *params);                                               \
	zerv_msg_inst_t __##zervice_name##_##topic##_msg __aligned(4) = {                          \
		.name = #zervice_name "_" #topic "_subscriber",                                    \
		.id = __##zervice_name##_##topic##_id,                                             \
		.is_locked = ATOMIC_INIT(false),                                                   \
		.handler = (zerv_msg_abstract_handler_t)__##zervice_name##_##topic##_handler,      \
		.is_raw = false,                                                                   \
		.raw_handler = NULL};                                                              \
	zerv_topic_subscriber_t __##zervice_name##_##topic __aligned(4) = {                        \
		.msg_instance = &__##zervice_name##_##topic##_msg,                                 \
		.serv = &zervice_name,                                                             \
	};                                                                                         \
	void __##zervice_name##_##topic##_handler(const topic##_zerv_topic_t *params)

/*=================================================================================================
 * ZERVICE TOPIC CLIENT MACROS
 *===============================================================================================*/

/**
 * @brief Macro for emitting a event over a topic.
 *
 * @param name The name of the topic.
 * @param params The parameters of the event.
 */
#define ZERV_TOPIC_EMIT(name, params...)                                                           \
	zerv_internal_emit_topic(&name##_subscribers, sizeof(name##_zerv_topic_t),                 \
				 &((name##_zerv_topic_t){params}));

#endif /* _ZERV_TOPIC_H_ */
