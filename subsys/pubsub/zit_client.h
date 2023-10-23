//#####################################################################################################################
//#                                                                                                                   #
//#                              Copyright (C) 2023 DevPort Ost AB, all rights reserved.                              #
//#                     Unauthorized copying of this file, via any medium is strictly prohibited.                     #
//#                                           Proprietary and confidential.                                           #
//#                                                                                                                   #
//# author:  Albin Hjalmas                                                                                            #
//# company: Systemfabriken                                                                                           #
//# contact: albin@systemfabriken.tech                                                                                #
//#####################################################################################################################
#ifndef _ZIT_CLIENT_H_
#define _ZIT_CLIENT_H_

// INCLUDES ###########################################################################################################
#include "zit_serv_internal.h"

// PUBLIC CLIENT API ##################################################################################################

/**
 * @brief Calls a service request.
 * 
 * @param serv_name The name of the service.
 * @param req_name The name of the request.
 * @param p_req_params Pointer to the parameters to the request.
 * @param p_response_dst Pointer to the destination of the response.
 * 
 * @return ZIT_RC_OK if the request was successfully called, ZIT_RC_FUTURE if the response will be delayed
 *        ZIT_RC_ERROR if the request failed.
 * 
 * @example 
 *   future_echo_ret_t future_echo_resp = { 0 };
 *   zit_rc_t rc = zit_client_call(future_service, future_echo,
 *                                 (&(future_echo_param_t){ .is_delayed = false, .str = "Hello World!" }),
 *                                 &future_echo_resp);
 * 
 * In this example we call a request called "future echo" on a service called "future service". If the response is delayed 
 * this call will return ZIT_RC_FUTURE and the response will be received later. If the response is not delayed the response
 * will be received immediately and the call will return the result of the request handler function.
*/
#define zit_client_call(serv_name, req_name, p_req_params, p_response_dst)                         \
    zit_serv_internal_client_request_handler(&serv_name, &__##req_name,                            \
                                             sizeof(req_name##_param_t), p_req_params,            \
                                             (struct zit_serv_resp_base *)p_response_dst,          \
                                             sizeof(req_name##_ret_t));

/**
 * @brief Get the future response of a request.
 * 
 * @param serv_name The name of the service.
 * @param req_name The name of the request.
 * @param p_response_dst Pointer to the destination of the response.
 * @param timeout The timeout of the wait for the response.
 * 
 * @return ZIT_RC_OK if the response was successfully received, ZIT_RC_TIMEOUT if the response timed out and
 *       ZIT_RC_ERROR if the response failed.
 * 
 * @example
 *   future_echo_ret_t future_echo_resp = { 0 };
 *   zit_rc_t rc = zit_client_get_future(future_service, future_echo, &future_echo_resp, K_MSEC(100));
 * 
 * In this example we get the response of a request called "future echo" on a service called "future service". If the response
 * is not received within 100 milliseconds the call will return ZIT_RC_TIMEOUT and the response will not be received. If the
 * response is received within 100 milliseconds the call will return the result of the request handler function and the response
 * will be stored in future_echo_resp.
*/
#define zit_client_get_future(serv_name, req_name, p_response_dst, timeout)                        \
    zit_serv_internal_get_future_resp(&serv_name, &__##req_name, p_response_dst, timeout)

#endif // _ZIT_CLIENT_H_
