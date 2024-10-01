#include "zerv_test_supervised_service.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zerv_test_supervised_service, LOG_LEVEL_DBG);

ZERV_DEF(zerv_test_supervised_service, 2048u);

static void zerv_test_supervised_service_thread_entry(void)
{
	LOG_DBG("zerv_test_supervised_service_thread_entry");

	while (true) {
		zerv_request_t *p_req =
			zerv_get_pending_request(&zerv_test_supervised_service, K_FOREVER);
		if (p_req == NULL) {
			LOG_ERR("Failed to get request");
			return;
		}

		zerv_rc_t rc = zerv_handle_request(&zerv_test_supervised_service, p_req);
		if (rc != ZERV_RC_OK) {
			LOG_ERR("Failed to handle request");
			return;
		}
	}
}

K_THREAD_DEFINE(zerv_test_supervised_service_thread, 2048u,
		(k_thread_entry_t)zerv_test_supervised_service_thread_entry, NULL, NULL, NULL, 7, 0,
		0);

ZERV_CMD_HANDLER_DEF(set_state, req, resp)
{
	LOG_DBG("Received request: state: %d", req->state);
	return ZERV_RC_OK;
}

ZERV_CMD_HANDLER_DEF(set_sampling_interval, req, resp)
{
	LOG_DBG("Received request: interval: %d", req->interval);
	return ZERV_RC_OK;
}
