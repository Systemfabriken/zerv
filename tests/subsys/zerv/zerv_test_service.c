#include "zerv_test_service.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zerv_test_service, LOG_LEVEL_DBG);

K_SEM_DEFINE(test_msg_sem, 0, 1);

ZERV_DEF_REQUEST_PROCESSOR_THREAD(zerv_test_service, 128, 256, K_PRIO_PREEMPT(10));

ZERV_CMD_HANDLER_DEF(get_hello_world, req, resp)
{
	LOG_DBG("Received request: a: %d, b: %d", req->a, req->b);
	resp->a = req->a;
	resp->b = req->b;
	strcpy(resp->str, "Hello World!");
	return ZERV_RC_OK;
}

ZERV_CMD_HANDLER_DEF(echo, req, resp)
{
	LOG_DBG("Received request: str: %s", req->str);
	strcpy(resp->str, req->str);
	return ZERV_RC_OK;
}

ZERV_CMD_HANDLER_DEF(fail, req, resp)
{
	return ZERV_RC_ERROR;
}

ZERV_CMD_HANDLER_DEF(read_hello_world, req, resp)
{
	LOG_DBG("Received request");
	strcpy(resp->str, "Hello World!");
	return ZERV_RC_OK;
}

ZERV_CMD_HANDLER_DEF(print_hello_world, req, resp)
{
	LOG_DBG("Received request");
	LOG_INF("Hello World!");
	return ZERV_RC_OK;
}

ZERV_MSG_HANDLER_DEF(test_msg, msg)
{
	LOG_DBG("Received message: str: %s, a: %d, b: %d", msg->str, msg->a, msg->b);
	k_sem_give(&test_msg_sem);
}
