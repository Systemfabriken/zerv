// #####################################################################################################################
// # # #                              Copyright (C) 2023 DevPort Ost AB, all rights reserved. # #
// Unauthorized copying of this file, via any medium is strictly prohibited.                     #
// #                                           Proprietary and confidential. # # # # author:  Albin
// Hjalmas # # company: Systemfabriken # # contact: albin@systemfabriken.tech #
// #####################################################################################################################
#ifndef _ZIT_SERV_INTERNAL_H_
#define _ZIT_SERV_INTERNAL_H_
// INCLUDES
// ###########################################################################################################
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

typedef enum {
	ZIT_RC_NOMEM = -5,
	ZIT_RC_NULLPTR = -4,
	ZIT_RC_ERROR = -3,
	ZIT_RC_TIMEOUT = -2,
	ZIT_RC_LOCKED = -1,
	ZIT_RC_OK = 0,
	ZIT_RC_FUTURE = 1,
} zit_rc_t;

static inline const char *zit_rc_to_str(zit_rc_t rc)
{
	switch (rc) {
	case ZIT_RC_NOMEM:
		return "ZIT_RC_NOMEM";
	case ZIT_RC_NULLPTR:
		return "ZIT_RC_NULLPTR";
	case ZIT_RC_ERROR:
		return "ZIT_RC_ERROR";
	case ZIT_RC_TIMEOUT:
		return "ZIT_RC_TIMEOUT";
	case ZIT_RC_LOCKED:
		return "ZIT_RC_LOCKED";
	case ZIT_RC_OK:
		return "ZIT_RC_OK";
	case ZIT_RC_FUTURE:
		return "ZIT_RC_FUTURE";
	default:
		return "UNKNOWN";
	}
}

// DECLARATIONS USED INTERNALLY BY ZIT_SERV
// ###########################################################################

typedef zit_rc_t (*zit_serv_abstract_req_handler_t)(const void *req, void *resp);
typedef void (*zit_serv_abstract_resp_handler_t)(void);

struct zit_serv_resp_base {
	zit_rc_t rc;
	zit_serv_abstract_resp_handler_t handler;
} __aligned(4);

/**
 * @brief Used internally to store the parameters to a service request on the service's heap.
 */
struct zit_allocated_req_params {
	size_t data_len;
	uint8_t data[];
};

struct zit_req_params {
	uint32_t unused; // Managed by the k_fifo.
	int id;
	struct k_sem *response_sem;
	size_t resp_len;
	struct zit_serv_resp_base *resp;
	struct zit_allocated_req_params client_req_params;
} __aligned(4);

struct zit_future_response {
	bool is_active;
	struct k_sem *sem;
	struct zit_req_params *req_params;
	size_t resp_len;
	struct zit_serv_resp_base *resp;
} __aligned(4);

/**
 * @brief The type of a service request.
 * @note This is used internally to represent a service request - not request parameters.
 */
struct zit_serv_req_instance {
	const char *name;
	int id;
	atomic_t is_locked;
	zit_serv_abstract_req_handler_t handler;
	struct zit_future_response future;
};

struct zit_service {
	const char *name;
	struct k_heap *heap;
	struct k_fifo *fifo;
	struct k_mutex *mtx;
	size_t req_instance_cnt;
	struct zit_serv_req_instance **req_instances;
} __aligned(4);

/**
 * @brief Wrapper for a zervice response.
 *
 * This struct is used to wrap a zervice response in a struct that can be passed to the zervice call
 * function. The struct is used to check if a response has been returned or not. If no response has
 * been returned, the response_handle from the application will be set to NULL.
 */
typedef struct __zerv_response_wrapper {
	bool is_response_returned;
	void *response_handle;
} __zerv_response_wrapper_t;

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
 * @return ZIT_RC return code from the service request handler function except for ZIT_RC_FUTURE
 * that is returned if the request is delayed.
 */
zit_rc_t zit_serv_internal_client_request_handler(struct zit_service *serv,
						  struct zit_serv_req_instance *req_instance,
						  size_t client_req_params_len,
						  const void *client_req_params,
						  struct zit_serv_resp_base *resp, size_t resp_len);

zit_rc_t zit_serv_internal_get_future_resp(struct zit_service *serv,
					   struct zit_serv_req_instance *req_instance, void *resp,
					   k_timeout_t timeout);

void _zit_serv_future_signal_response(struct zit_serv_req_instance *req_instance, zit_rc_t rc);

// MACROS USED INTERNALLY BY ZIT_SERV
// #################################################################################

#define __DECL_FIELD(field) field

#define __ZIT_SERV_HANDLER_FN_DECL(req_name)                                                       \
	__unused static zit_rc_t __##req_name##_handler(const req_name##_param_t *req,             \
							req_name##_ret_t *resp);

#define __ZIT_SERV_HANDLER_FN_IDENTIFIER(req_name)                                                 \
	(zit_serv_abstract_req_handler_t) __##req_name##_handler

#define __ZIT_SERV_REQ_ID_DECL(req_name) __##req_name##_id

#define __ZIT_SERV_REQ_INSTANCE_POINTER(req_name) &__##req_name

#define __ZIT_SERV_DEFINE_REQUEST_INSTANCE_LIST(serv_name, ...)                                    \
	static struct zit_serv_req_instance *serv_name##_req_instances[] = {                       \
		FOR_EACH(__ZIT_SERV_REQ_INSTANCE_POINTER, (, ), __VA_ARGS__)};

// PRIVATE REQUEST HANDLER CODE GENERATION MACROS USED BY ZIT_SERV
// ####################################################

#define __ZIT_SERV_REQ_GET_PARAMS(req_name, serv_name)                                             \
	static inline req_name##_param_t *zit_serv_get_##req_name##_params(void)                   \
	{                                                                                          \
		return (req_name##_param_t *)zit_serv_get_last_##serv_name##_req()                 \
			->client_req_params.data;                                                  \
	}

#define __ZIT_SERV_REQ_GET_PARAMS_DEF(serv_name, ...)                                              \
	FOR_EACH_FIXED_ARG(__ZIT_SERV_REQ_GET_PARAMS, (), serv_name, __VA_ARGS__)

#define __ZIT_SERV_REQ_FUTURE_HANDLER_DEF(req_name)

#endif // _ZIT_SERV_INTERNAL_H_
