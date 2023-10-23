// #####################################################################################################################
// # # #                              Copyright (C) 2023 DevPort Ost AB, all rights reserved. # #
// Unauthorized copying of this file, via any medium is strictly prohibited.                     #
// #                                           Proprietary and confidential. # # # # author:  Albin
// Hjalmas # # company: Systemfabriken # # contact: albin@systemfabriken.tech #
// #####################################################################################################################
//  INCLUDES
//  ###########################################################################################################
#include "zit_future_client_service.h"
#include "zit_test_future_service.h"
#include "zit_client.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(future_client_service, LOG_LEVEL_DBG);

// PRIVATE DECLARATIONS
// ###############################################################################################
static void zit_serv_thread(void);

// PRIVATE DEFINITIONS
// ################################################################################################

ZIT_SERV_DEF(future_client, 1024, call_future_echo, call_other);

K_THREAD_DEFINE(future_client_thread, 1024, (k_thread_entry_t)zit_serv_thread, NULL, NULL, NULL, 0,
		0, 0);

// PRIVATE FUNCTION DECLARATIONS
// ######################################################################################

void zit_serv_thread(void)
{
	while (true) {
		struct zit_req_params *params = zit_serv_get_req(&future_client, K_FOREVER);
		if (!params) {
			LOG_ERR("Failed to receive request");
			continue;
		}

		LOG_DBG("Received request");
		zit_rc_t rc = zit_serv_handle_request(&future_client, params);
		if (rc != 0) {
			LOG_ERR("Failed to handle request");
			continue;
		}
	}
}

ZIT_SERV_REQ_HANDLER_DEF(call_future_echo, req, resp)
{
	future_echo_ret_t response = {0};
	zit_rc_t rc = zit_client_call(future_service, future_echo,
				      (&(future_echo_param_t){.is_delayed = false,
							      .delay = K_NO_WAIT,
							      .str = "Hello from client!"}),
				      &response);
	resp->was_expected_rc = rc == req->expected_rc;

	if (rc == ZIT_RC_FUTURE) {
		return ZIT_RC_FUTURE;
	} else {
		return ZIT_RC_OK;
	}
}

ZIT_SERV_REQ_HANDLER_DEF(call_other, req, resp)
{
	future_other_ret_t response = {0};
	zit_rc_t rc =
		zit_client_call(future_service, future_other,
				(&(future_other_param_t){.str = "Hello from client!"}), &response);
	if (rc < 0) {
		LOG_ERR("Failed to call future_other");
		resp->was_expected_rc = false;
		return rc;
	}
	if (strcmp(response.str, "Hello from client!") != 0) {
		LOG_ERR("Unexpected response from future_other");
		resp->was_expected_rc = false;
		return ZIT_RC_OK;
	}
	resp->was_expected_rc = true;
	return ZIT_RC_OK;
}
