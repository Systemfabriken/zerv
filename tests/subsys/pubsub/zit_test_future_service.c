// #####################################################################################################################
// # # #                              Copyright (C) 2023 DevPort Ost AB, all rights reserved. # #
// Unauthorized copying of this file, via any medium is strictly prohibited.                     #
// #                                           Proprietary and confidential. # # # # author:  Albin
// Hjalmas # # company: Systemfabriken # # contact: albin@systemfabriken.tech #
// #####################################################################################################################
//  INCLUDES
//  ###########################################################################################################
#include "zit_test_future_service.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zit_test_future_service, LOG_LEVEL_DBG);

// PRIVATE DECLARATIONS
// ###############################################################################################
static void zit_serv_thread(void);

// PRIVATE DEFINITIONS
// ################################################################################################

ZIT_SERV_DEF(future_service, 1024, future_echo, future_other);

K_THREAD_DEFINE(zit_test_future_service_thread, 1024, (k_thread_entry_t)zit_serv_thread, NULL, NULL,
		NULL, 0, 0, 0);

// PRIVATE FUNCTION DECLARATIONS
// ######################################################################################

void zit_serv_thread(void)
{
	while (true) {
		struct zit_req_params *p_params = zit_serv_get_req(&future_service, K_FOREVER);
		LOG_DBG("Received request");

		zit_rc_t rc = zit_serv_handle_request(&future_service, p_params);
		if (rc < ZIT_RC_OK) {
			LOG_ERR("Failed to handle request");
			continue;
		}

		if (rc == ZIT_RC_FUTURE) {
			// Initialize the future
			struct zit_serv_req_instance *req_instance;
			rc = zit_serv_get_req_instance(
				&future_service, zit_serv_get_req_id(future_echo), &req_instance);
			if (rc < ZIT_RC_OK) {
				LOG_ERR("Failed to get request instance");
				continue;
			}
			rc = zit_serv_future_init(&future_service, req_instance, p_params);
			if (rc < ZIT_RC_OK) {
				LOG_ERR("Failed to initialize future");
				continue;
			}

			// Now the request context is stored in the request instance future. The
			// request can now be responded to at a later time.

			zit_serv_future_get_params(future_echo, params);
			LOG_DBG("Future request: %s", params->str);
			k_sleep(params->delay);
			LOG_DBG("Future request done");
			LOG_DBG("Future request: %s", params->str);
			zit_serv_future_get_response(future_echo, resp);
			memcpy(resp->str, params->str, strlen(params->str) + 1);

			LOG_DBG("Releasing future");
			zit_serv_future_signal_response(future_echo, ZIT_RC_OK);
		} else {
			LOG_DBG("Request is handled");
		}
	}
}

ZIT_SERV_REQ_HANDLER_DEF(future_echo, req, resp)
{
	if (req->is_delayed) {
		return ZIT_RC_FUTURE;
	} else {
		strcpy(resp->str, req->str);
		return ZIT_RC_OK;
	}
}

ZIT_SERV_REQ_HANDLER_DEF(future_other, req, resp)
{
	LOG_DBG("Received request: %s", req->str);
	strcpy(resp->str, req->str);
	return ZIT_RC_OK;
}