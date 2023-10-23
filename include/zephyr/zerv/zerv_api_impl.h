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
#ifndef _ZERV_API_IMPL_H_
#define _ZERV_API_IMPL_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/

/*=================================================================================================
 * ZERV INTERNAL PUBLIC TYPES
 *===============================================================================================*/

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

/*=================================================================================================
 * ZERV INTERNAL PUBLIC MACROS
 *===============================================================================================*/

#define __ZERV_IMPL_CALL(zervice, command, return_code_name, response_handle_name, ...)            \
	__##zervice##_##command##_resp_t __##command##_response;                                   \
	__##zervice##_##command##_resp_t *response_handle_name = &__##command##_response;          \
	int return_code_name;                                                                      \
	{                                                                                          \
		__zerv_response_wrapper_t __##command##_response_wrapper = {                       \
			.is_response_returned = false, .response_handle = response_handle_name};   \
		__##zervice##_##command##_req_t __##command##_request = {__VA_ARGS__};             \
		if (!__##command##_response_wrapper.is_response_returned) {                        \
			response_handle_name = NULL;                                               \
		}                                                                                  \
	}

/*=================================================================================================
 * PUBLIC FUNCTION DECLARATIONS
 *===============================================================================================*/

#endif /* _ZERV_API_IMPL_H_ */