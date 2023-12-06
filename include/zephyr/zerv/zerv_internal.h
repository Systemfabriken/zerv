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
#ifndef _ZERV_INTERNAL_H_
#define _ZERV_INTERNAL_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util_macro.h>
#include <zephyr/sys/slist.h>

/*=================================================================================================
 * DECLARATIONS
 *===============================================================================================*/

typedef enum {
	ZERV_RC_NOMEM = -ENOMEM,
	ZERV_RC_NULLPTR = -EINVAL,
	ZERV_RC_ERROR = -EFAULT,
	ZERV_RC_TIMEOUT = -EAGAIN,
	ZERV_RC_LOCKED = -EBUSY,
	ZERV_RC_OK = 0
} zerv_rc_t;

static inline const char *zerv_rc_to_str(zerv_rc_t rc)
{
	switch (rc) {
	case ZERV_RC_NOMEM:
		return "ZERV_RC_NOMEM";
	case ZERV_RC_NULLPTR:
		return "ZERV_RC_NULLPTR";
	case ZERV_RC_ERROR:
		return "ZERV_RC_ERROR";
	case ZERV_RC_TIMEOUT:
		return "ZERV_RC_TIMEOUT";
	case ZERV_RC_LOCKED:
		return "ZERV_RC_LOCKED";
	case ZERV_RC_OK:
		return "ZERV_RC_OK";
	default:
		return "UNKNOWN";
	}
}

typedef zerv_rc_t (*zerv_cmd_abstract_handler_t)(const void *req, void *resp);
typedef void (*zerv_msg_abstract_handler_t)(const void *params);
typedef void (*zerv_raw_msg_abstract_handler_t)(size_t size, const void *data);

/**
 * @brief Used internally to store the parameters to a service request on the service's heap.
 */
typedef struct {
	size_t data_len;
	uint8_t data[];
} zerv_cmd_in_bytes_t;

typedef struct {
	uint32_t unused; // Managed by the k_fifo.
	int id;
	struct k_sem *response_sem;
	size_t resp_len;
	void *resp;
	int rc; // Return code from the service request handler.
	zerv_cmd_in_bytes_t client_req_params;
} zerv_request_t;

/**
 * @brief The type of a zervice command.
 * @note This is used internally to represent a zervice command.
 */
typedef struct {
	const char *name;
	int id;
	atomic_t is_locked;
	zerv_cmd_abstract_handler_t handler;
} zerv_cmd_inst_t;

/**
 * @brief The type of a zervice message.
 * @note This is used internally to represent a zervice message.
 */
typedef struct {
	const char *name;
	int id;
	atomic_t is_locked;
	zerv_msg_abstract_handler_t handler;
	bool is_raw;
	zerv_raw_msg_abstract_handler_t raw_handler;
} zerv_msg_inst_t;

struct zerv_topic_subscriber;
typedef struct {
	const char *name;
	struct k_heap *heap;
	struct k_fifo *fifo;
	struct k_mutex *mtx;
	size_t cmd_instance_cnt;
	zerv_cmd_inst_t **cmd_instances;
	size_t msg_instance_cnt;
	zerv_msg_inst_t **msg_instances;
	size_t topic_subscribers_cnt;
	struct zerv_topic_subscriber **topic_subscriber_instances;
	sys_slist_t **topic_subscriber_lists;
} zervice_t;

typedef zerv_rc_t (*zerv_msg_function_t)(const zervice_t *serv, zerv_msg_inst_t *msg_instance,
					 size_t client_msg_params_len,
					 const void *client_msg_params);

typedef struct zerv_topic_subscriber {
	sys_snode_t node;
	zerv_msg_inst_t *msg_instance;
	const zervice_t *serv;
} zerv_topic_subscriber_t;

/**
 * @brief Used internally to store an event that is to be handled by the zervice.
 */
typedef struct {
	const struct k_poll_event *event;
	void (*handler)(void *obj);
	uint32_t type;
} zerv_event_t;

typedef struct {
	zerv_event_t **events;
	size_t event_cnt;
} zerv_events_t;

/**
 * @brief DONT TOUCH, USED INTERNALLY to call a service request from the client thread.
 *
 * @param[in] serv The service to call.
 * @param[in] req_instance The type of the request to call.
 * @param[in] client_req_params_len The length of the request.
 * @param[in] client_req_params The request parameters from the client.
 * @param[out] resp The response.
 * @param[in] resp_len The length of the response.
 *
 * @return ZERV_RC return code from the service request handler function except for ZERV_RC_FUTURE
 * that is returned if the request is delayed.
 */
zerv_rc_t zerv_internal_client_request_handler(const zervice_t *serv, zerv_cmd_inst_t *req_instance,
					       size_t client_req_params_len,
					       const void *client_req_params, void *resp,
					       size_t resp_len);

/**
 * @brief DONT TOUCH, USED INTERNALLY to pass a message from the client thread to the service
 * thread.
 *
 * @param[in] serv The service to call.
 * @param[in] msg_instance The type of the message to call.
 * @param[in] client_msg_params_len The length of the message.
 * @param[in] client_msg_params The message parameters from the client.
 *
 * @return ZERV_RC return code from the service message handler function.
 */
zerv_rc_t zerv_internal_client_message_handler(const zervice_t *serv, zerv_msg_inst_t *msg_instance,
					       size_t client_msg_params_len,
					       const void *client_msg_params);

zerv_rc_t zerv_internal_emit_topic(sys_slist_t *subscribers, size_t params_size,
				   const void *params);

void __zerv_thread(const zervice_t *p_zervice, zerv_events_t *zervice_events,
		   int (*on_init_cb)(void));

/*=================================================================================================
 * PUBLIC MACROS
 *===============================================================================================*/

#define __ZERV_IMPL_STRUCT_MEMBER(field) field;

#define __ZERV_HANDLER_FN_IDENTIFIER(cmd_name) (zerv_cmd_abstract_handler_t) __##cmd_name##_handler

#define __ZERV_CMD_ID_DECL(cmd_name) __##cmd_name##_id

#define __ZERV_MSG_ID_DECL(msg_name) __##msg_name##_id

#define __ZERV_TOPIC_MSG_ID_DECL(topic_msg_name, zervice_name)                                     \
	__##zervice_name##_##topic_msg_name##_id

#define __ZERV_CMD_INSTANCE_POINTER(cmd_name) &__##cmd_name

#define __ZERV_TOPIC_MSG_INSTANCE_POINTER(topic_msg_name, zervice_name)                            \
	&__##zervice_name##_##topic_msg_name

#define __ZERV_TOPIC_MSG_EXTERN(topic_msg_name, zervice_name)                                      \
	extern zerv_topic_subscriber_t __##zervice_name##_##topic_msg_name

#define __ZERV_SUBSCRIBED_TOPIC_POINTER(topic_name) &topic_name##_subscribers

#define __ZERV_DEFINE_CMD_INSTANCE_LIST(zervice, ...)                                              \
	__unused static zerv_cmd_inst_t *zervice##_cmd_instances[] = {                             \
		FOR_EACH_NONEMPTY_TERM(__ZERV_CMD_INSTANCE_POINTER, (, ), __VA_ARGS__)};

#define __ZERV_DEFINE_MSG_INSTANCE_LIST(zervice, ...)                                              \
	__unused static zerv_msg_inst_t *zervice##_msg_instances[] = {                             \
		FOR_EACH_NONEMPTY_TERM(__ZERV_CMD_INSTANCE_POINTER, (, ), __VA_ARGS__)};

#define FOR_EACH_FIXED_NONEMPTY_TERM(F, term, fixed, ...)                                          \
	COND_CODE_0(/* are there zero non-empty arguments ? */                                     \
		    NUM_VA_ARGS_LESS_1(                                                            \
			    LIST_DROP_EMPTY(__VA_ARGS__, _)), /* if so, expand to nothing */       \
		    (),                                       /* otherwise, expand to: */          \
		    (/* FOR_EACH() on nonempty elements, */                                        \
		     FOR_EACH_FIXED_ARG(                                                           \
			     F, term, fixed,                                                       \
			     LIST_DROP_EMPTY(__VA_ARGS__)) /* plus a final terminator */           \
		     __DEBRACKET term))

#define __ZERV_DEFINE_TOPIC_MSG_INSTANCE_LIST(zervice, ...)                                        \
	FOR_EACH_FIXED_NONEMPTY_TERM(__ZERV_TOPIC_MSG_EXTERN, (;), zervice, __VA_ARGS__);          \
	__unused static zerv_topic_subscriber_t *zervice##_topic_subscriber_instances[] = {        \
		FOR_EACH_FIXED_NONEMPTY_TERM(__ZERV_TOPIC_MSG_INSTANCE_POINTER, (), zervice,       \
					     __VA_ARGS__)};

#define __ZERV_GET_CMD_INPUT(cmd_name, zervice)                                                    \
	static inline cmd_name##_param_t *zerv_get_##cmd_name##_params(void)                       \
	{                                                                                          \
		return (cmd_name##_param_t *)zerv_get_last_##zervice##_req()                       \
			->client_req_params.data;                                                  \
	}

#define __ZERV_DEFINE_SUBSCRIBED_TOPICS_LIST(zervice, ...)                                         \
	__unused static sys_slist_t *zervice##_subscribed_topics[] __aligned(4) = {                \
		FOR_EACH_NONEMPTY_TERM(__ZERV_SUBSCRIBED_TOPIC_POINTER, (, ), __VA_ARGS__)};

#define __ZERV_GET_CMD_INPUT_DEF(zervice, ...)                                                     \
	FOR_EACH_FIXED_ARG(__ZERV_GET_CMD_INPUT, (), zervice, __VA_ARGS__)

#define __zerv_event_t_INIT(name) &__zerv_event_##name

#define __ZERV_MSG_ID_OFFSET 0
#define __ZERV_MSG_LIST(name, messages...)                                                         \
	__ZERV_DEFINE_MSG_INSTANCE_LIST(name, messages)                                            \
	enum __##name##_msgs_e                                                                     \
	{                                                                                          \
		__##name##_MSG_ID_OFFSET = __ZERV_MSG_ID_OFFSET,                                   \
		FOR_EACH_NONEMPTY_TERM(__ZERV_MSG_ID_DECL, (, ), messages) __##name##_msg_cnt      \
	}

#define __ZERV_CMD_ID_OFFSET 10000
#define __ZERV_CMD_LIST(name, commands...)                                                         \
	__ZERV_DEFINE_CMD_INSTANCE_LIST(name, commands)                                            \
	enum __##name##_cmds_e                                                                     \
	{                                                                                          \
		__##name##_CMD_ID_OFFSET = __ZERV_CMD_ID_OFFSET,                                   \
		FOR_EACH_NONEMPTY_TERM(__ZERV_CMD_ID_DECL, (, ), commands) __##name##_cmd_cnt      \
	}

#define __ZERV_TOPIC_MSG_ID_OFFSET 20000
#define __ZERV_SUBSCRIBED_TOPICS_LIST(name, topics...)                                             \
	__ZERV_DEFINE_SUBSCRIBED_TOPICS_LIST(name, topics)                                         \
	__ZERV_DEFINE_TOPIC_MSG_INSTANCE_LIST(name, topics)                                        \
	enum __##name##_topic_msgs_e                                                               \
	{                                                                                          \
		__##name##_TOPIC_MSG_ID_OFFSET = __ZERV_TOPIC_MSG_ID_OFFSET,                       \
		FOR_EACH_FIXED_NONEMPTY_TERM(__ZERV_TOPIC_MSG_ID_DECL, (, ), name, topics)         \
			__##name##_topic_msg_cnt                                                   \
	}
#endif // _ZERV_INTERNAL_H_
