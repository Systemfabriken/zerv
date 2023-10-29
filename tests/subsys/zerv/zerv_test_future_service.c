#include "zerv_test_future_service.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zerv_test_future_service, LOG_LEVEL_DBG);

static void zerv_thread(void);

ZERV_DEF_NO_THREAD(future_service, 256);

K_THREAD_DEFINE(zerv_test_future_service_thread, 256, (k_thread_entry_t)zerv_thread, NULL, NULL,
		NULL, 0, 0, 0);

void zerv_thread(void)
{
	while (true) {
		zerv_cmd_in_t *p_params = zerv_get_cmd_input(&future_service, K_FOREVER);
		LOG_DBG("Received request");

		zerv_rc_t rc = zerv_handle_request(&future_service, p_params);
		if (rc < ZERV_RC_OK) {
			LOG_ERR("Failed to handle request");
			continue;
		}

		if (rc == ZERV_RC_FUTURE) {
			// Initialize the future
			zerv_cmd_inst_t *req_instance;
			rc = zerv_get_cmd_input_instance(
				&future_service, zerv_get_cmd_input_id(future_echo), &req_instance);
			if (rc < ZERV_RC_OK) {
				LOG_ERR("Failed to get request instance");
				continue;
			}
			rc = zerv_future_init(&future_service, req_instance, p_params);
			if (rc < ZERV_RC_OK) {
				LOG_ERR("Failed to initialize future");
				continue;
			}

			// Now the request context is stored in the request instance future. The
			// request can now be responded to at a later time.

			zerv_future_get_params(future_echo, params);
			LOG_DBG("Future request: %s", params->str);
			k_sleep(params->delay);
			LOG_DBG("Future request done");
			LOG_DBG("Future request: %s", params->str);
			zerv_future_get_response(future_echo, resp);
			memcpy(resp->str, params->str, strlen(params->str) + 1);

			LOG_DBG("Releasing future");
			zerv_future_signal_response(future_echo, ZERV_RC_OK);
		} else {
			LOG_DBG("Request is handled");
		}
	}
}

ZERV_CMD_HANDLER_DEF(future_echo, req, resp)
{
	if (req->is_delayed) {
		return ZERV_RC_FUTURE;
	} else {
		strcpy(resp->str, req->str);
		return ZERV_RC_OK;
	}
}

ZERV_CMD_HANDLER_DEF(future_other, req, resp)
{
	LOG_DBG("Received request: %s", req->str);
	strcpy(resp->str, req->str);
	return ZERV_RC_OK;
}