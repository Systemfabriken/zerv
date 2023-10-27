#include "zerv_test_service.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zerv_test_service, LOG_LEVEL_DBG);

static void zerv_thread(void);

ZERV_DEF(zerv_test_service, 128, get_hello_world, echo, fail);

K_THREAD_DEFINE(zerv_test_service_thread, 256, (k_thread_entry_t)zerv_thread, NULL, NULL, NULL, 0,
		0, 0);

void zerv_thread(void)
{
	while (true) {
		struct zerv_req_params *params = zerv_get_req(&zerv_test_service, K_FOREVER);
		if (!params) {
			LOG_ERR("Failed to receive request");
			continue;
		}
		LOG_DBG("Received request");

		zerv_rc_t rc = zerv_handle_request(&zerv_test_service, params);
		if (rc != 0) {
			LOG_ERR("Failed to handle request");
			continue;
		}
	}
}

ZERV_REQ_HANDLER_DEF(get_hello_world, req, resp)
{
	LOG_DBG("Received request: a: %d, b: %d", req->a, req->b);
	resp->a = req->a;
	resp->b = req->b;
	strcpy(resp->str, "Hello World!");
	return ZERV_RC_OK;
}

ZERV_REQ_HANDLER_DEF(echo, req, resp)
{
	LOG_DBG("Received request: str: %s", req->str);
	strcpy(resp->str, req->str);
	return ZERV_RC_OK;
}

ZERV_REQ_HANDLER_DEF(fail, req, resp)
{
	LOG_DBG("Received request: dummy: %d", req->dummy);
	return ZERV_RC_ERROR;
}