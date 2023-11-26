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
#ifndef _ZERV_FUTURE_H_
#define _ZERV_FUTURE_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include <zephyr/zerv/zerv_internal.h>

/*=================================================================================================
 * PUBLIC MACROS
 *===============================================================================================*/

/**
 * @brief Used from the service thread to initialize a future.
 *
 * @param[in] serv The service to initialize the future on.
 * @param[in] req_instance The request instance that the future is for.
 * @param[in] req_params The request parameters.
 *
 * @return N/A
 */
zerv_rc_t zerv_future_init(const zervice_t *serv, zerv_cmd_inst_t *req_instance,
			   zerv_cmd_in_t *req_params);

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

#endif /* _ZERV_FUTURE_H_ */