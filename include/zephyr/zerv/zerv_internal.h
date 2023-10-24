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

/*=================================================================================================
 * DECLARATIONS
 *===============================================================================================*/

typedef enum {
	ZERV_RC_NOMEM = -ENOMEM,
	ZERV_RC_NULLPTR = -EINVAL,
	ZERV_RC_ERROR = -EFAULT,
	ZERV_RC_TIMEOUT = -EAGAIN,
	ZERV_RC_LOCKED = -EBUSY,
	ZERV_RC_OK = 0,
	ZERV_RC_FUTURE = 1,
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
	case ZERV_RC_FUTURE:
		return "ZERV_RC_FUTURE";
	default:
		return "UNKNOWN";
	}
}

typedef zerv_rc_t (*zerv_abstract_req_handler_t)(const void *req, void *resp);
typedef void (*zerv_abstract_resp_handler_t)(void);

struct zerv_resp_base {
	zerv_rc_t rc;
	zerv_abstract_resp_handler_t handler;
} __aligned(4);

/**
 * @brief Used internally to store the parameters to a service request on the service's heap.
 */
struct zerv_allocated_req_params {
	size_t data_len;
	uint8_t data[];
};

struct zerv_req_params {
	uint32_t unused; // Managed by the k_fifo.
	int id;
	struct k_sem *response_sem;
	size_t resp_len;
	struct zerv_resp_base *resp;
	struct zerv_allocated_req_params client_req_params;
} __aligned(4);

struct zerv_future_response {
	bool is_active;
	struct k_sem *sem;
	struct zerv_req_params *req_params;
	size_t resp_len;
	struct zerv_resp_base *resp;
} __aligned(4);

/**
 * @brief The type of a service request.
 * @note This is used internally to represent a service request - not request parameters.
 */
struct zerv_req_instance {
	const char *name;
	int id;
	atomic_t is_locked;
	zerv_abstract_req_handler_t handler;
	struct zerv_future_response future;
};

typedef struct {
	const char *name;
	struct k_heap *heap;
	struct k_fifo *fifo;
	struct k_mutex *mtx;
	size_t cmd_instance_cnt;
	struct zerv_req_instance **cmd_instances;
} zervice_t;

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
zerv_rc_t zerv_internal_client_request_handler(zervice_t *serv,
					       struct zerv_req_instance *req_instance,
					       size_t client_req_params_len,
					       const void *client_req_params,
					       struct zerv_resp_base *resp, size_t resp_len);

zerv_rc_t zerv_internal_get_future_resp(zervice_t *serv, struct zerv_req_instance *req_instance,
					void *resp, k_timeout_t timeout);

void _zerv_future_signal_response(struct zerv_req_instance *req_instance, zerv_rc_t rc);

/*=================================================================================================
 * PUBLIC MACROS
 *===============================================================================================*/

#define __ZERV_IMPL_STRUCT_MEMBER(field) field;

#define __ZERV_HANDLER_FN_DECL(cmd_name)                                                           \
	__unused static zerv_rc_t __##cmd_name##_handler(const cmd_name##_param_t *req,            \
							 cmd_name##_ret_t *resp);

#define __ZERV_HANDLER_FN_IDENTIFIER(cmd_name) (zerv_abstract_req_handler_t) __##cmd_name##_handler

#define __ZERV_REQ_ID_DECL(cmd_name) __##cmd_name##_id

#define __ZERV_REQ_INSTANCE_POINTER(cmd_name) &__##cmd_name

#define __ZERV_DEFINE_REQUEST_INSTANCE_LIST(zervice, ...)                                          \
	static struct zerv_req_instance *zervice##_cmd_instances[] = {                             \
		FOR_EACH(__ZERV_REQ_INSTANCE_POINTER, (, ), __VA_ARGS__)};

#define __ZERV_REQ_GET_PARAMS(cmd_name, zervice)                                                   \
	static inline cmd_name##_param_t *zerv_get_##cmd_name##_params(void)                       \
	{                                                                                          \
		return (cmd_name##_param_t *)zerv_get_last_##zervice##_req()                       \
			->client_req_params.data;                                                  \
	}

#define __ZERV_REQ_GET_PARAMS_DEF(zervice, ...)                                                    \
	FOR_EACH_FIXED_ARG(__ZERV_REQ_GET_PARAMS, (), zervice, __VA_ARGS__)

#define __ZERV_REQ_FUTURE_HANDLER_DEF(cmd_name)

#endif // _ZERV_INTERNAL_H_
