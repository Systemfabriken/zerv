#include "zerv_future_client_service.h"
#include "zerv_test_future_service.h"
#include <zephyr/zerv/zerv.h>

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(future_client_service, LOG_LEVEL_DBG);

static void zerv_thread(void);

ZERV_DEF(future_client, 128, call_future_echo, call_other);

K_THREAD_DEFINE(future_client_thread, 256, (k_thread_entry_t)zerv_thread, NULL, NULL, NULL, 0, 0,
		0);

void zerv_thread(void)
{
	while (true) {
		struct zerv_req_params *params = zerv_get_req(&future_client, K_FOREVER);
		if (!params) {
			LOG_ERR("Failed to receive request");
			continue;
		}

		LOG_DBG("Received request");
		zerv_rc_t rc = zerv_handle_request(&future_client, params);
		if (rc != 0) {
			LOG_ERR("Failed to handle request");
			continue;
		}
	}
}

ZERV_REQ_HANDLER_DEF(call_future_echo, req, resp)
{
	future_echo_ret_t response = {0};
	zerv_rc_t rc = zerv_call(future_service, future_echo,
				 (&(future_echo_param_t){.is_delayed = false,
							 .delay = K_NO_WAIT,
							 .str = "Hello from client!"}),
				 &response);
	resp->was_expected_rc = rc == req->expected_rc;

	if (rc == ZERV_RC_FUTURE) {
		return ZERV_RC_FUTURE;
	} else {
		return ZERV_RC_OK;
	}
}

ZERV_REQ_HANDLER_DEF(call_other, req, resp)
{
	future_other_ret_t response = {0};
	zerv_rc_t rc = zerv_call(future_service, future_other,
				 (&(future_other_param_t){.str = "Hello from client!"}), &response);
	if (rc < 0) {
		LOG_ERR("Failed to call future_other");
		resp->was_expected_rc = false;
		return rc;
	}
	if (strcmp(response.str, "Hello from client!") != 0) {
		LOG_ERR("Unexpected response from future_other");
		resp->was_expected_rc = false;
		return ZERV_RC_OK;
	}
	resp->was_expected_rc = true;
	return ZERV_RC_OK;
}
