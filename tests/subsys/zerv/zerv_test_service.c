#include "zerv_test_service.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zerv_test_service, LOG_LEVEL_DBG);

ZERV_DEF_EVENT_PROCESSOR(zerv_test_service, 128, 256, K_PRIO_PREEMPT(10), NULL, NULL);

ZERV_CMD_DEF(get_hello_world, req, resp)
{
	LOG_DBG("Received request: a: %d, b: %d", req->a, req->b);
	resp->a = req->a;
	resp->b = req->b;
	strcpy(resp->str, "Hello World!");
	return ZERV_RC_OK;
}

ZERV_CMD_DEF(echo, req, resp)
{
	LOG_DBG("Received request: str: %s", req->str);
	strcpy(resp->str, req->str);
	return ZERV_RC_OK;
}

ZERV_CMD_DEF(fail, req, resp)
{
	LOG_DBG("Received request: dummy: %d", req->dummy);
	return ZERV_RC_ERROR;
}
