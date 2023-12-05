#include "zerv_msg_test_service.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

K_SEM_DEFINE(print_msg_sem, 0, 1);
K_SEM_DEFINE(cmp_msg_1_sem, 0, 1);
K_SEM_DEFINE(cmp_msg_2_sem, 0, 1);

LOG_MODULE_REGISTER(zerv_msg_test_service, LOG_LEVEL_DBG);

ZERV_DEF_REQUEST_PROCESSOR_THREAD(zerv_msg_test_service, 512, 2048, K_PRIO_PREEMPT(10));

ZERV_MSG_HANDLER_DEF(print_msg, param)
{
	LOG_INF("print_msg: %s", param->msg);
	zassert_mem_equal(param->msg, "Hello World!", 12, "Message is not equal");
	k_sem_give(&print_msg_sem);
}

ZERV_MSG_HANDLER_DEF(cmp_msg_1, param)
{
	LOG_INF("cmp_msg_1: a=%d, b=%u, c=%c, d=%s", param->a, param->b, param->c, param->d);
	zassert_equal(param->a, 10, "a is not equal");
	zassert_equal(param->b, 20, "b is not equal");
	zassert_equal(param->c, 'a', "c is not equal");
	zassert_mem_equal(param->d, "Hello World!", 12, "d is not equal");
	k_sem_give(&cmp_msg_1_sem);
}

ZERV_MSG_HANDLER_DEF(cmp_msg_2, param)
{
	LOG_INF("cmp_msg_2: a=%d, b=%u, c=%c, d=%s", param->a, param->b, param->c, param->d);
	zassert_equal(param->a, 10, "a is not equal");
	zassert_equal(param->b, 20, "b is not equal");
	zassert_equal(param->c, 'a', "c is not equal");
	zassert_mem_equal(param->d, "Hello World!", 12, "d is not equal");
	k_sem_give(&cmp_msg_2_sem);
}

ZERV_RAW_MSG_HANDLER_DEF(raw_msg, size, data)
{
	LOG_INF("raw_msg: size=%d, data=%s", size, data);
	zassert_equal(size, 12, "size is not equal");
	zassert_mem_equal(data, "Hello World!", 12, "data is not equal");
	k_sem_give(&print_msg_sem);
}