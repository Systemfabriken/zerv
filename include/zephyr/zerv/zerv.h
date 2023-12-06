/*=================================================================================================
 *
 *           ██████╗ ██╗████████╗███╗   ███╗ █████╗ ███╗   ██╗     █████╗ ██████╗
 *           ██╔══██╗██║╚══██╔══╝████╗ ████║██╔══██╗████╗  ██║    ██╔══██╗██╔══██╗
 *           ██████╔╝██║   ██║   ██╔████╔██║███████║██╔██╗ ██║    ███████║██████╔╝
 *           ██╔══██╗██║   ██║   ██║╚██╔╝██║██╔══██║██║╚██╗██║    ██╔══██║██╔══██╗
 *           ██████╔╝██║   ██║   ██║ ╚═╝ ██║██║  ██║██║ ╚████║    ██║  ██║██████╔╝
 *           ╚═════╝ ╚═╝   ╚═╝   ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝    ╚═╝  ╚═╝╚═════╝
 * Description:
 *  This file contains the public API of the zerv library. The zerv library is a library for
 *  easing the development of event driven applications on Zephyr. The library helps the developer
 *  to create a modular application, following the principles of the micro service architecture. The
 *  library provides a way to create services that can be requested by other modules in the system.
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 BitMan AB
 * contact: albin@bitman.se
 *===============================================================================================*/
#ifndef _ZERV_H_
#define _ZERV_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include <zephyr/zerv/zerv_internal.h>
#include <zephyr/sys/slist.h>

/*=================================================================================================
 * ZERVICE MACROS
 *===============================================================================================*/

/**
 * @brief Macro for appending message requests to a zervice.
 *
 * @param messages... The message requests to append to the zervice.
 */
#define ZERV_MSGS(messages...) messages

/**
 * @brief Macro for appending command requests to a zervice.
 *
 * @param cmds... The command requests to append to the zervice.
 */
#define ZERV_CMDS(cmds...) cmds

/**
 * @brief Macro for subscribing to topics.
 *
 * @param topics... The topics to subcribe to.
 */
#define ZERV_SUBSCRIBED_TOPICS(topics...) topics

/**
 * @brief Macro for declaring a zervice in a header file.
 *
 * @param name The name of the service.
 * @param zerv_cmds... The commands of the service, provided as a list of command names.
 * @param zerv_msgs... The messages of the service, provided as a list of message names.
 * @param subscribed_topics... The topics that the service subscribes to, provided as a list of
 * topic names.
 *
 * @note The requests must be declared before the service. The service needs to be defined in the
 * source file
 */
#define ZERV_DECL(name, zerv_cmds, zerv_msgs, subscribed_topics)                                   \
	__ZERV_MSG_LIST(name, zerv_msgs);                                                          \
	__ZERV_CMD_LIST(name, zerv_cmds);                                                          \
	__ZERV_SUBSCRIBED_TOPICS_LIST(name, subscribed_topics);                                    \
	extern const zervice_t name

/**
 * @brief Macro for defining a thread-less zervice in a source file.
 *
 * @param zervice_name The name of the service.
 * @param heap_size The size of the heap of the service. The heap is used to store the command
 * inputs and outputs while they are being processed.
 */
#define ZERV_DEF(zervice_name, heap_size)                                                          \
	static K_HEAP_DEFINE(__##zervice_name##_heap, heap_size);                                  \
	static K_FIFO_DEFINE(__##zervice_name##_fifo);                                             \
	static K_MUTEX_DEFINE(__##zervice_name##_mtx);                                             \
	const zervice_t zervice_name __aligned(4) = {                                              \
		.name = #zervice_name,                                                             \
		.heap = &__##zervice_name##_heap,                                                  \
		.fifo = &__##zervice_name##_fifo,                                                  \
		.mtx = &__##zervice_name##_mtx,                                                    \
		.cmd_instance_cnt = __##zervice_name##_cmd_cnt,                                    \
		.cmd_instances = zervice_name##_cmd_instances,                                     \
		.msg_instance_cnt = __##zervice_name##_msg_cnt,                                    \
		.msg_instances = zervice_name##_msg_instances,                                     \
		.topic_subscribers_cnt =                                                           \
			__##zervice_name##_topic_msg_cnt - __ZERV_TOPIC_MSG_ID_OFFSET - 1,         \
		.topic_subscriber_instances = zervice_name##_topic_subscriber_instances,           \
		.topic_subscriber_lists = zervice_name##_subscribed_topics,                        \
	};

/**
 * @brief Macro for defining a zervice that is handled on a thread that processes both zerv commands
 * and events.
 *
 * @param zervice The name of the zervice. This should be the same name as declared with the
 * ZERV_DECL macro.
 * @param heap_size The size of the heap of the zervice. The heap is used to store the command
 * inputs and outputs while they are being processed.
 * @param stack_size The size of the stack of the zervice thread.
 * @param prio The priority of the zervice thread.
 * @param on_init_cb The callback function that is called when the zervice thread is started.
 * @param zerv_events... The events of the zervice, provided as a list of event names. The events
 * must be declared before the zervice thread.
 */
#define ZERV_DEF_THREAD(zervice, heap_size, stack_size, prio, on_init_cb, zerv_events...)          \
	ZERV_DEF(zervice, heap_size);                                                              \
	static const struct k_poll_event __##zervice##_k_poll_event =                              \
		K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE,                   \
						K_POLL_MODE_NOTIFY_ONLY, &__##zervice##_fifo, 0);  \
	static zerv_event_t __zerv_event_##zervice = {                                             \
		.event = &__##zervice##_k_poll_event, .handler = NULL, .type = 0};                 \
	static zerv_event_t *__##zervice##_events[] = {                                            \
		&__zerv_event_##zervice,                                                           \
		FOR_EACH_NONEMPTY_TERM(__zerv_event_t_INIT, (, ), zerv_events)};                   \
	static zerv_events_t __##zervice##_events_arg = {                                          \
		.events = __##zervice##_events,                                                    \
		.event_cnt = ARRAY_SIZE(__##zervice##_events),                                     \
	};                                                                                         \
	static K_THREAD_DEFINE(__##zervice##_thread, stack_size, (k_thread_entry_t)__zerv_thread,  \
			       &zervice, &__##zervice##_events_arg, on_init_cb, prio, 0, 0)

/**
 * @brief Macro for defining a zervice that is handled on a thread that processes requests and
 * executes a low-jitter periodic callback.
 *
 * @param zervice The name of the zervice. This should be the same name as declared with the
 * ZERV_DECL macro.
 * @param heap_size The size of the heap of the zervice. The heap is used to store the request
 * inputs and outputs while they are being processed.
 * @param stack_size The size of the stack of the zervice thread.
 * @param prio The priority of the zervice thread.
 * @param period The period of the periodic callback.
 * @param on_init_cb The callback function that is called when the zervice thread is started.
 * @param periodic_cb The callback function that is called periodically.
 * @param events... The events of the zervice, provided as a list of event names. The events must
 * be declared before the zervice thread.
 *
 * @note The periodic callback is called from the context of the zervice thread. The callback
 * function should not block.
 */
#define ZERV_DEF_PERIODIC_THREAD(zervice, heap_size, stack_size, prio, period, on_init_cb,         \
				 periodic_cb, events...)                                           \
	static K_SEM_DEFINE(__##zervice##_sem, 0, 1);                                              \
	static void __##zervice##_periodic_timer_expired(struct k_timer *dummy)                    \
	{                                                                                          \
		k_sem_give(&__##zervice##_sem);                                                    \
	}                                                                                          \
	static K_TIMER_DEFINE(__##zervice##_timer, __##zervice##_periodic_timer_expired, NULL);    \
	ZERV_EVENT_DEF(__##zervice##_event, K_POLL_TYPE_SEM_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY,    \
		       &__##zervice##_sem);                                                        \
	ZERV_EVENT_HANDLER_DEF(__##zervice##_event, obj)                                           \
	{                                                                                          \
		struct k_sem *sem = (struct k_sem *)obj;                                           \
		k_sem_take(sem, K_NO_WAIT);                                                        \
		if (periodic_cb != NULL) {                                                         \
			periodic_cb();                                                             \
		}                                                                                  \
	}                                                                                          \
	static int __##zervice##_init(void)                                                        \
	{                                                                                          \
		int rc = 0;                                                                        \
		if (on_init_cb != NULL) {                                                          \
			rc = on_init_cb();                                                         \
		}                                                                                  \
		k_timer_start(&__##zervice##_timer, period, period);                               \
		return rc;                                                                         \
	}                                                                                          \
	ZERV_DEF_THREAD(zervice, heap_size, stack_size, prio, __##zervice##_init,                  \
			__##zervice##_event, events)

/*=================================================================================================
 * ZERV EVENT MACROS
 *===============================================================================================*/

/**
 * @brief Macro for declaring a k_poll_event that is to be handled by a zervice.
 *
 * @param name The name of the event.
 * @param _event_type The type of the event. Should be K_POLL_TYPE_*.
 * @param _event_mode The mode of the event. Should be K_POLL_MODE_*.
 * @param _event_obj The object of the event. Should be a pointer to a supported k_poll_event
 * object.
 */
#define ZERV_EVENT_DEF(name, _event_type, _event_mode, _event_obj)                                 \
	static const struct k_poll_event __##name##_event =                                        \
		K_POLL_EVENT_STATIC_INITIALIZER(_event_type, _event_mode, _event_obj, 0);          \
	static void __##name##_event_handler(void *obj);                                           \
	static zerv_event_t __zerv_event_##name = {.event = &__##name##_event,                     \
						   .handler = __##name##_event_handler,            \
						   .type = _event_type}

/**
 * @brief Macro for defining a service event handler function.
 *
 * @param name The name of the event.
 * @param obj The object of the event.
 */
#define ZERV_EVENT_HANDLER_DEF(name, obj) void __##name##_event_handler(void *obj)

/**
 * @brief Macro for initializing a k_poll_event struct for a zervice.
 *
 * @param zervice The name of the zervice.
 */
#define ZERV_K_POLL_EVENT_INITIALIZER(zervice)                                                     \
	K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY,  \
					&__##zervice##_fifo, 0)

/*=================================================================================================
 * PUBLIC FUNCTION DECLARATIONS
 *===============================================================================================*/

/**
 * @brief Wait for a service request to be available and return it.
 *
 * @param[in] serv The service to wait for a request on.
 * @param[in] timeout The timeout to wait for a request.
 *
 * @return Pointer to the request parameters if a request was received, NULL if the timeout
 * was reached.
 */
zerv_request_t *zerv_get_pending_request(const zervice_t *serv, k_timeout_t timeout);

/**
 * @brief Used from the service thread to handle a request.
 *
 * @param[in] serv The service to handle the request on.
 * @param[in] req The request to handle.
 *
 * @return ZERV_RC return code from the request handler function.
 */
zerv_rc_t zerv_handle_request(const zervice_t *serv, zerv_request_t *req);

#endif // _ZERV_H_
