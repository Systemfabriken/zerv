#include "zerv_msg_test_service.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zerv_msg_test_service, LOG_LEVEL_DBG);

ZERV_DEF_REQUEST_PROCESSOR_THREAD(zerv_msg_test_service, 512, 2048, K_PRIO_PREEMPT(10));

ZERV_MSG_HANDLER_DEF(print_msg, param)
{
	LOG_INF("print_msg: %s", param->msg);
}

ZERV_MSG_HANDLER_DEF(cmp_msg_1, param)
{
	LOG_INF("cmp_msg_1: a=%d, b=%u, c=%c, d=%s", param->a, param->b, param->c, param->d);
}

ZERV_MSG_HANDLER_DEF(cmp_msg_2, param)
{
	LOG_INF("cmp_msg_2: a=%d, b=%u, c=%c, d=%s", param->a, param->b, param->c, param->d);
}