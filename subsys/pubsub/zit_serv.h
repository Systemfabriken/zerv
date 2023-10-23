// #####################################################################################################################
// # # #                              Copyright (C) 2023 DevPort Ost AB, all rights reserved. # #
// Unauthorized copying of this file, via any medium is strictly prohibited.                     #
// #                                           Proprietary and confidential. # # # # author:  Albin
// Hjalmas # # company: Systemfabriken # # contact: albin@systemfabriken.tech #
// #####################################################################################################################
#ifndef _ZIT_SERV_H_
#define _ZIT_SERV_H_

// INCLUDES
// ###########################################################################################################
#include "zit_serv_internal.h"

// PUBLIC MACROS FOR DECALARING A SERVICE (Used in header file)
// #######################################################

#define __ZERV_IMPL_STRUCT_MEMBER(field) field;

#define ZERV_IN(...) FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

#define ZERV_OUT(...) FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

#define ZERV_CMD_DECL(name, param, ret)                                                            \
	typedef struct name##_param {                                                              \
		param                                                                              \
	} name##_param_t;                                                                          \
	typedef void (*name##_resp_handler_t)(void);                                               \
	typedef struct name##_ret {                                                                \
		zit_rc_t rc;                                                                       \
		name##_resp_handler_t on_delayed_response;                                         \
		ret                                                                                \
	} name##_ret_t;                                                                            \
	extern struct zit_serv_req_instance __##name

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
	cmd##_ret_t __##cmd##_response;                                                            \
	cmd##_ret_t *p_ret = &__##cmd##_response;                                                  \
	zit_rc_t retcode = 0;                                                                      \
	{                                                                                          \
		cmd##_param_t __##cmd##_request = {__VA_ARGS__};                                   \
		retcode = zit_serv_internal_client_request_handler(                                \
			&zervice, &__##cmd, sizeof(cmd##_param_t), &__##cmd##_request,             \
			(struct zit_serv_resp_base *)p_ret, sizeof(cmd##_ret_t));                  \
	}

/**
 * @brief Macro for declaring a service in the header file.
 *
 * @param serv_name The name of the service.
 * @param ... The requests of the service, provided as a list of request names.
 *
 * @note The requests must be declared before the service. The service needs to be defined in the
 * source file
 *
 * @example
 * ZIT_SERV_DECL(zit_test_service, get_hello_world, echo, fail);
 *
 * Here the service is declared with the name zit_test_service, and the requests get_hello_world,
 * echo and fail.
 */
#define ZIT_SERV_DECL(serv_name, ...)                                                              \
	enum __##serv_name##_reqs_e{FOR_EACH(__ZIT_SERV_REQ_ID_DECL, (, ), __VA_ARGS__),           \
				    __##serv_name##_req_cnt};                                      \
	extern struct zit_service serv_name

// PUBLIC MACROS FOR DEFINING A SERVICE (Used in source file)
// #########################################################

/**
 * @brief Macro for defining a service request handler function.
 *
 * @param req_name The name of the request.
 * @param p_req The request pointer name.
 * @param p_resp The response pointer name.
 *
 * @note If the response is delayed the handler function must return ZIT_RC_FUTURE. This will
 * trigger the service to initialize a future and signal the client that the response is delayed.
 *
 * @example
 * ZIT_SERV_REQ_HANDLER_DEF(get_hello_world, const get_hello_world_req_t *p_req,
 * get_hello_world_ret_t *p_resp)
 * {
 *    strcpy(p_resp->str, "Hello World!");
 *    p_resp->a = p_req->a;
 *    p_resp->b = p_req->b;
 *    return 0;
 * }
 *
 * Here the request handler for the request get_hello_world is defined.
 */
#define ZIT_SERV_REQ_HANDLER_DEF(req_name, p_req, p_resp)                                          \
	static K_SEM_DEFINE(__##req_name##_future_sem, 0, 1);                                      \
	static req_name##_ret_t __##req_name##_future_response;                                    \
	struct zit_serv_req_instance __##req_name __aligned(4) = {                                 \
		.name = #req_name,                                                                 \
		.id = __##req_name##_id,                                                           \
		.is_locked = ATOMIC_INIT(false),                                                   \
		.handler = (zit_serv_abstract_req_handler_t)__##req_name##_handler,                \
		.future =                                                                          \
			{                                                                          \
				.is_active = false,                                                \
				.sem = &__##req_name##_future_sem,                                 \
				.req_params = NULL,                                                \
				.resp_len = sizeof(req_name##_ret_t),                              \
				.resp = (struct zit_serv_resp_base                                 \
						 *)&__##req_name##_future_response,                \
			},                                                                         \
	};                                                                                         \
	zit_rc_t __##req_name##_handler(const req_name##_param_t *p_req, req_name##_ret_t *p_resp)

/**
 * @brief Macro for defining a service in a source file.
 *
 * @param serv_name The name of the service.
 * @param ... The requests of the service, provided as a list of request names.
 *
 * @note The requests must be defined before the service.
 *
 * @example
 * ZIT_SERV_DEF(zit_test_service, 1024, get_hello_world, echo, fail);
 *
 * Here the service is defined with the name zit_test_service, a queue size of 1024 bytes and the
 * requests get_hello_world, echo and fail.
 */
#define ZIT_SERV_DEF(serv_name, queue_mem_size, ...)                                               \
	FOR_EACH(__ZIT_SERV_HANDLER_FN_DECL, (;), __VA_ARGS__)                                     \
		static K_HEAP_DEFINE(__##serv_name##_heap, queue_mem_size);                        \
	static K_FIFO_DEFINE(__##serv_name##_fifo);                                                \
	static K_MUTEX_DEFINE(__##serv_name##_mtx);                                                \
	__ZIT_SERV_DEFINE_REQUEST_INSTANCE_LIST(serv_name, __VA_ARGS__)                            \
	struct zit_service serv_name __aligned(4) = {                                              \
		.name = #serv_name,                                                                \
		.heap = &__##serv_name##_heap,                                                     \
		.fifo = &__##serv_name##_fifo,                                                     \
		.mtx = &__##serv_name##_mtx,                                                       \
		.req_instance_cnt = __##serv_name##_req_cnt,                                       \
		.req_instances = serv_name##_req_instances,                                        \
	};

/**
 * @brief Macro for defining a subscriber event.
 * @param event_name The name of the event.
 * @param sub_name The name of the subscriber.
 */
#define ZIT_SERV_POLL_EVENT_INITIALIZER(serv_name)                                                 \
	K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY,  \
					serv_name.fifo, 0)

// PUBLIC FUNCTION DECLARATIONS
// #######################################################################################

/**
 * @brief Wait for a service request to be available and return it.
 *
 * @param[in] serv The service to wait for a request on.
 * @param[in] timeout The timeout to wait for a request.
 *
 * @return Pointer to the request parameters if a request was received, NULL if the timeout was
 * reached.
 */
struct zit_req_params *zit_serv_get_req(struct zit_service *serv, k_timeout_t timeout);

/**
 * @brief Used from the service thread to handle a request.
 *
 * @param[in] serv The service to handle the request on.
 * @param[in] req The request to handle.
 *
 * @return ZIT_RC return code from the request handler function.
 */
zit_rc_t zit_serv_handle_request(struct zit_service *serv, struct zit_req_params *req);

/**
 * @brief Get the id of a request.
 *
 * @param[in] req_name The name of the request.
 *
 * @return The id of the request.
 */
#define zit_serv_get_req_id(req_name) __ZIT_SERV_REQ_ID_DECL(req_name)

/**
 * @brief Used from the service thread to get a request instance that the request parameters are
 * aimed for.
 *
 * @param[in] serv The service to get the request instance from.
 * @param[in] req_params The request parameters.
 * @param[out] req_instance The request instance that the request parameters are aimed for.
 *
 * @return  ZIT_RC return code.
 */
zit_rc_t zit_serv_get_req_instance(struct zit_service *serv, int req_id,
				   struct zit_serv_req_instance **req_instance);

/**
 * @brief Used from the service thread to initialize a future.
 *
 * @param[in] serv The service to initialize the future on.
 * @param[in] req_instance The request instance that the future is for.
 * @param[in] req_params The request parameters.
 *
 * @return N/A
 */
zit_rc_t zit_serv_future_init(struct zit_service *serv, struct zit_serv_req_instance *req_instance,
			      struct zit_req_params *req_params);

/**
 * @brief Check if a future request has a delayed response.
 */
#define zit_serv_future_is_active(req_name) __##req_name.future.is_active

/**
 * @brief Define a pointer to the request parameters of a future request.
 *
 * @param[in] req_name The name of the request.
 * @param[in] req_params_handle_name The name of the pointer to the request parameters.
 *
 * @return N/A
 *
 * @note This macro should be used in the service thread to get the request parameters of a future
 * request.
 */
#define zit_serv_future_get_params(req_name, req_params_handle_name)                               \
	req_name##_param_t *req_params_handle_name =                                               \
		(req_name##_param_t *)&__##req_name.future.req_params->client_req_params.data

/**
 * @brief Get the response storage of a future request.
 *
 * @param[in] req_name The name of the request.
 * @param[in] resp_handle_name The name of the pointer, to be created, to the response storage.
 *
 * @return N/A
 */
#define zit_serv_future_get_response(req_name, resp_handle_name)                                   \
	req_name##_ret_t *resp_handle_name = (req_name##_ret_t *)__##req_name.future.resp

/**
 * @brief Send response to a delayed request.
 *
 * @param[in] serv The service to respond on.
 * @param[in] req_id The id of the request.
 *
 * @return N/A
 */
#define zit_serv_future_signal_response(req_name, return_code)                                     \
	_zit_serv_future_signal_response(&__##req_name, return_code)

#endif // _ZIT_SERV_H_
