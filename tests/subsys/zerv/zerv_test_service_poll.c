#include "zerv_test_service_poll.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zerv_poll_test, LOG_LEVEL_DBG);

ZERV_DEF_NO_THREAD(zerv_poll_service_1, 128);
ZERV_EVENT_HANDLER_DECL(zerv_poll_service_1_event, K_POLL_TYPE_FIFO_DATA_AVAILABLE,
			K_POLL_MODE_NOTIFY_ONLY, zerv_poll_service_1.fifo);

ZERV_DEF_EVENT_PROCESSOR_THREAD(zerv_poll_service_2, 128, 256, 0, zerv_poll_service_1_event);

ZERV_EVENT_HANDLER_DEF(zerv_poll_service_1_event)
{
	zerv_cmd_in_t *p_req = zerv_get_cmd_input(&zerv_poll_service_1, K_NO_WAIT);
	if (p_req == NULL) {
		LOG_ERR("Failed to get request");
		return;
	}

	zerv_rc_t rc = zerv_handle_request(&zerv_poll_service_1, p_req);
	if (rc != ZERV_RC_OK) {
		LOG_ERR("Failed to handle request");
		return;
	}
}

ZERV_CMD_HANDLER_DEF(echo1, req, resp)
{
	LOG_DBG("Received request: str: %s", req->str);
	strcpy(resp->str, req->str);
	return ZERV_RC_OK;
}

ZERV_CMD_HANDLER_DEF(fail1, req, resp)
{
	LOG_DBG("Received request: dummy: %d", req->dummy);
	return ZERV_RC_ERROR;
}

ZERV_CMD_HANDLER_DEF(echo2, req, resp)
{
	LOG_DBG("Received request: str: %s", req->str);
	strcpy(resp->str, req->str);
	return ZERV_RC_OK;
}

ZERV_CMD_HANDLER_DEF(fail2, req, resp)
{
	LOG_DBG("Received request: dummy: %d", req->dummy);
	return ZERV_RC_ERROR;
}