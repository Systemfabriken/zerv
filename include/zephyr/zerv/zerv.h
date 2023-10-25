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
#include "zerv_internal.h"

/*=================================================================================================
 * PUBLIC MACROS
 *===============================================================================================*/

#define ZERV_IN(...) FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

#define ZERV_OUT(...) FOR_EACH(__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

#define ZERV_CMD_DECL(name, param, ret)                                                            \
	typedef struct name##_param {                                                              \
		param                                                                              \
	} name##_param_t;                                                                          \
	typedef void (*name##_resp_handler_t)(void);                                               \
	typedef struct name##_ret {                                                                \
		zerv_rc_t rc;                                                                      \
		name##_resp_handler_t on_delayed_response;                                         \
		ret                                                                                \
	} name##_ret_t;                                                                            \
	extern struct zerv_req_instance __##name

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
 * specified by the ZERV_CMD_PARAM macro. The last argument should be the code block to
 * execute when the response is received. The code block should be surrounded by curly
 * brackets.
 *
 * @return int Is returned in the ret variable which is defined by the macro.
 */
#define ZERV_CALL(zervice, cmd, retcode, p_ret, ...)                                               \
	{                                                                                          \
		cmd##_ret_t __##cmd##_response;                                                    \
		cmd##_ret_t *p_ret = &__##cmd##_response;                                          \
		zerv_rc_t retcode = 0;                                                             \
		{                                                                                  \
			cmd##_param_t __##cmd##_request = {                                        \
				REVERSE_ARGS(GET_ARGS_LESS_N(1, REVERSE_ARGS(__VA_ARGS__)))};      \
			retcode = zerv_internal_client_request_handler(                            \
				&zervice, &__##cmd, sizeof(cmd##_param_t), &__##cmd##_request,     \
				(struct zerv_resp_base *)p_ret, sizeof(cmd##_ret_t));              \
		}                                                                                  \
		GET_ARG_N(1, REVERSE_ARGS(__VA_ARGS__));                                           \
	}

/**
 * @brief Macro for declaring a service in the header file.
 *
 * @param zervice The name of the service.
 * @param ... The requests of the service, provided as a list of request names.
 *
 * @note The requests must be declared before the service. The service needs to be defined in the
 * source file
 *
 * @example
 * ZERV_DECL(zerv_test_service, get_hello_world, echo, fail);
 *
 * Here the service is declared with the name zerv_test_service, and the requests get_hello_world,
 * echo and fail.
 */
#define ZERV_DECL(zervice, ...)                                                                    \
	enum __##zervice##_cmds_e{FOR_EACH(__ZERV_REQ_ID_DECL, (, ), __VA_ARGS__),                 \
				  __##zervice##_cmd_cnt};                                          \
	extern zervice_t zervice

/**
 * @brief Macro for defining a service request handler function.
 *
 * @param cmd_name The name of the request.
 * @param p_req The request pointer name.
 * @param p_resp The response pointer name.
 *
 * @note If the response is delayed the handler function must return ZERV_RC_FUTURE. This will
 * trigger the service to initialize a future and signal the client that the response is delayed.
 *
 * @example
 * ZERV_REQ_HANDLER_DEF(get_hello_world, const get_hello_world_req_t *p_req,
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
#define ZERV_REQ_HANDLER_DEF(cmd_name, p_req, p_resp)                                              \
	static K_SEM_DEFINE(__##cmd_name##_future_sem, 0, 1);                                      \
	static cmd_name##_ret_t __##cmd_name##_future_response;                                    \
	struct zerv_req_instance __##cmd_name __aligned(4) = {                                     \
		.name = #cmd_name,                                                                 \
		.id = __##cmd_name##_id,                                                           \
		.is_locked = ATOMIC_INIT(false),                                                   \
		.handler = (zerv_abstract_req_handler_t)__##cmd_name##_handler,                    \
		.future =                                                                          \
			{                                                                          \
				.is_active = false,                                                \
				.sem = &__##cmd_name##_future_sem,                                 \
				.req_params = NULL,                                                \
				.resp_len = sizeof(cmd_name##_ret_t),                              \
				.resp = (struct zerv_resp_base *)&__##cmd_name##_future_response,  \
			},                                                                         \
	};                                                                                         \
	zerv_rc_t __##cmd_name##_handler(const cmd_name##_param_t *p_req, cmd_name##_ret_t *p_resp)

/**
 * @brief Macro for defining a service in a source file.
 *
 * @param zervice The name of the service.
 * @param ... The requests of the service, provided as a list of request names.
 *
 * @note The requests must be defined before the service.
 *
 * @example
 * ZERV_DEF(zerv_test_service, 1024, get_hello_world, echo, fail);
 *
 * Here the service is defined with the name zerv_test_service, a queue size of 1024 bytes and the
 * requests get_hello_world, echo and fail.
 */
#define ZERV_DEF(zervice, queue_mem_size, ...)                                                     \
	FOR_EACH(__ZERV_HANDLER_FN_DECL, (;), __VA_ARGS__)                                         \
		static K_HEAP_DEFINE(__##zervice##_heap, queue_mem_size);                          \
	static K_FIFO_DEFINE(__##zervice##_fifo);                                                  \
	static K_MUTEX_DEFINE(__##zervice##_mtx);                                                  \
	__ZERV_DEFINE_REQUEST_INSTANCE_LIST(zervice, __VA_ARGS__)                                  \
	zervice_t zervice __aligned(4) = {                                                         \
		.name = #zervice,                                                                  \
		.heap = &__##zervice##_heap,                                                       \
		.fifo = &__##zervice##_fifo,                                                       \
		.mtx = &__##zervice##_mtx,                                                         \
		.cmd_instance_cnt = __##zervice##_cmd_cnt,                                         \
		.cmd_instances = zervice##_cmd_instances,                                          \
	};

/**
 * @brief Macro for defining a subscriber event.
 * @param event_name The name of the event.
 * @param sub_name The name of the subscriber.
 */
#define ZERV_POLL_EVENT_INITIALIZER(zervice)                                                       \
	K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY,  \
					zervice.fifo, 0)

/*=================================================================================================
 * PUBLIC FUNCTION DECLARATIONS
 *===============================================================================================*/

/**
 * @brief Wait for a service request to be available and return it.
 *
 * @param[in] serv The service to wait for a request on.
 * @param[in] timeout The timeout to wait for a request.
 *
 * @return Pointer to the request parameters if a request was received, NULL if the timeout was
 * reached.
 */
struct zerv_req_params *zerv_get_req(zervice_t *serv, k_timeout_t timeout);

/**
 * @brief Used from the service thread to handle a request.
 *
 * @param[in] serv The service to handle the request on.
 * @param[in] req The request to handle.
 *
 * @return ZERV_RC return code from the request handler function.
 */
zerv_rc_t zerv_handle_request(zervice_t *serv, struct zerv_req_params *req);

/**
 * @brief Get the id of a request.
 *
 * @param[in] cmd_name The name of the request.
 *
 * @return The id of the request.
 */
#define zerv_get_req_id(cmd_name) __ZERV_REQ_ID_DECL(cmd_name)

/**
 * @brief Used from the service thread to get a request instance that the request parameters are
 * aimed for.
 *
 * @param[in] serv The service to get the request instance from.
 * @param[in] req_params The request parameters.
 * @param[out] req_instance The request instance that the request parameters are aimed for.
 *
 * @return  ZERV_RC return code.
 */
zerv_rc_t zerv_get_req_instance(zervice_t *serv, int req_id,
				struct zerv_req_instance **req_instance);

/**
 * @brief Used from the service thread to initialize a future.
 *
 * @param[in] serv The service to initialize the future on.
 * @param[in] req_instance The request instance that the future is for.
 * @param[in] req_params The request parameters.
 *
 * @return N/A
 */
zerv_rc_t zerv_future_init(zervice_t *serv, struct zerv_req_instance *req_instance,
			   struct zerv_req_params *req_params);

/**
 * @brief Check if a future request has a delayed response.
 */
#define zerv_future_is_active(cmd_name) __##cmd_name.future.is_active

/**
 * @brief Define a pointer to the request parameters of a future request.
 *
 * @param[in] cmd_name The name of the request.
 * @param[in] req_params_handle_name The name of the pointer to the request parameters.
 *
 * @return N/A
 *
 * @note This macro should be used in the service thread to get the request parameters of a future
 * request.
 */
#define zerv_future_get_params(cmd_name, req_params_handle_name)                                   \
	cmd_name##_param_t *req_params_handle_name =                                               \
		(cmd_name##_param_t *)&__##cmd_name.future.req_params->client_req_params.data

/**
 * @brief Get the response storage of a future request.
 *
 * @param[in] cmd_name The name of the request.
 * @param[in] resp_handle_name The name of the pointer, to be created, to the response storage.
 *
 * @return N/A
 */
#define zerv_future_get_response(cmd_name, resp_handle_name)                                       \
	cmd_name##_ret_t *resp_handle_name = (cmd_name##_ret_t *)__##cmd_name.future.resp

/**
 * @brief Send response to a delayed request.
 *
 * @param[in] serv The service to respond on.
 * @param[in] req_id The id of the request.
 *
 * @return N/A
 */
#define zerv_future_signal_response(cmd_name, return_code)                                         \
	_zerv_future_signal_response(&__##cmd_name, return_code)

/**
 * @brief Calls a service request.
 *
 * @param zervice The name of the service.
 * @param cmd_name The name of the request.
 * @param p_req_params Pointer to the parameters to the request.
 * @param p_response_dst Pointer to the destination of the response.
 *
 * @return ZERV_RC_OK if the request was successfully called, ZERV_RC_FUTURE if the response will be
 * delayed ZERV_RC_ERROR if the request failed.
 *
 * @example
 *   future_echo_ret_t future_echo_resp = { 0 };
 *   zerv_rc_t rc = zerv_call(future_service, future_echo,
 *                                 (&(future_echo_param_t){ .is_delayed = false, .str = "Hello
 * World!" }), &future_echo_resp);
 *
 * In this example we call a request called "future echo" on a service called "future service". If
 * the response is delayed this call will return ZERV_RC_FUTURE and the response will be received
 * later. If the response is not delayed the response will be received immediately and the call will
 * return the result of the request handler function.
 */
#define zerv_call(zervice, cmd_name, p_req_params, p_response_dst)                                 \
	zerv_internal_client_request_handler(                                                      \
		&zervice, &__##cmd_name, sizeof(cmd_name##_param_t), p_req_params,                 \
		(struct zerv_resp_base *)p_response_dst, sizeof(cmd_name##_ret_t));

/**
 * @brief Get the future response of a request.
 *
 * @param zervice The name of the service.
 * @param cmd_name The name of the request.
 * @param p_response_dst Pointer to the destination of the response.
 * @param timeout The timeout of the wait for the response.
 *
 * @return ZERV_RC_OK if the response was successfully received, ZERV_RC_TIMEOUT if the response
 * timed out and ZERV_RC_ERROR if the response failed.
 *
 * @example
 *   future_echo_ret_t future_echo_resp = { 0 };
 *   zerv_rc_t rc = zerv_get_future(future_service, future_echo, &future_echo_resp, K_MSEC(100));
 *
 * In this example we get the response of a request called "future echo" on a service called "future
 * service". If the response is not received within 100 milliseconds the call will return
 * ZERV_RC_TIMEOUT and the response will not be received. If the response is received within 100
 * milliseconds the call will return the result of the request handler function and the response
 * will be stored in future_echo_resp.
 */
#define zerv_get_future(zervice, cmd_name, p_response_dst, timeout)                                \
	zerv_internal_get_future_resp(&zervice, &__##cmd_name, p_response_dst, timeout)

#endif // _ZERV_H_
