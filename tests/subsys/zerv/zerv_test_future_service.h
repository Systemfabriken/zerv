#ifndef _ZIT_TEST_FUTURE_SERVICE_H_
#define _ZIT_TEST_FUTURE_SERVICE_H_

#include <zephyr/zerv/zerv.h>
#include <zephyr/zerv/zerv_cmd.h>

ZERV_CMD_DECL(future_echo, ZERV_IN(bool is_delayed, k_timeout_t delay, char str[30]),
	      ZERV_OUT(char str[30]));
ZERV_CMD_DECL(future_other, ZERV_IN(char str[30]), ZERV_OUT(char str[30]));
ZERV_DECL(future_service, future_echo, future_other);

#endif // _ZIT_TEST_FUTURE_SERVICE_H_
