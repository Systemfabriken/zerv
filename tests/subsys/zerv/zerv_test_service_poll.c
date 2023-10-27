#include "zerv_test_service_poll.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zerv_poll_test, LOG_LEVEL_DBG);

static void zerv_thread(void);

ZERV_DEF(zerv_poll_service_1, 128, echo1, fail1);
ZERV_DEF(zerv_poll_service_2, 128, echo2, fail2);

K_THREAD_DEFINE(zervice_poll_thread, 256, (k_thread_entry_t)zerv_thread, NULL, NULL, NULL, 0, 0, 0);

void zerv_thread(void)
{
	enum {
		ZIT_POLL_SERVICE_1 = 0,
		ZIT_POLL_SERVICE_2 = 1
	};

	struct k_poll_event events[] __aligned(4) = {
		[ZIT_POLL_SERVICE_1] = ZERV_POLL_EVENT_INITIALIZER(zerv_poll_service_1),
		[ZIT_POLL_SERVICE_2] = ZERV_POLL_EVENT_INITIALIZER(zerv_poll_service_2),
	};

	int rc;

	while (true) {
		LOG_DBG("Waiting for request");
		k_poll(events, 2, K_FOREVER);
		LOG_DBG("Received request");

		struct zerv_req_params *serv1_req = NULL;
		struct zerv_req_params *serv2_req = NULL;

		if (events[ZIT_POLL_SERVICE_1].state == K_POLL_STATE_FIFO_DATA_AVAILABLE) {
			LOG_DBG("Received event on service 1");
			serv1_req = zerv_get_req(&zerv_poll_service_1, K_NO_WAIT);
			LOG_DBG("Received request on service 1: %p", serv1_req);
		}

		if (events[ZIT_POLL_SERVICE_2].state == K_POLL_STATE_FIFO_DATA_AVAILABLE) {
			LOG_DBG("Received event on service 2");
			serv2_req = zerv_get_req(&zerv_poll_service_2, K_NO_WAIT);
			LOG_DBG("Received request on service 2: %p", serv2_req);
		}

		events[ZIT_POLL_SERVICE_1].state = K_POLL_STATE_NOT_READY;
		events[ZIT_POLL_SERVICE_2].state = K_POLL_STATE_NOT_READY;

		if (serv1_req != NULL) {
			LOG_DBG("Handling request on service 1");
			rc = zerv_handle_request(&zerv_poll_service_1, serv1_req);
			if (rc != 0) {
				LOG_ERR("Failed to handle request on service 1");
			}
		}

		if (serv2_req != NULL) {
			LOG_DBG("Handling request on service 2");
			rc = zerv_handle_request(&zerv_poll_service_2, serv2_req);
			if (rc != 0) {
				LOG_ERR("Failed to handle request on service 2");
			}
		}
	}
}

ZERV_REQ_HANDLER_DEF(echo1, req, resp)
{
	LOG_DBG("Received request: str: %s", req->str);
	strcpy(resp->str, req->str);
	return ZERV_RC_OK;
}

ZERV_REQ_HANDLER_DEF(fail1, req, resp)
{
	LOG_DBG("Received request: dummy: %d", req->dummy);
	return ZERV_RC_ERROR;
}

ZERV_REQ_HANDLER_DEF(echo2, req, resp)
{
	LOG_DBG("Received request: str: %s", req->str);
	strcpy(resp->str, req->str);
	return ZERV_RC_OK;
}

ZERV_REQ_HANDLER_DEF(fail2, req, resp)
{
	LOG_DBG("Received request: dummy: %d", req->dummy);
	return ZERV_RC_ERROR;
}