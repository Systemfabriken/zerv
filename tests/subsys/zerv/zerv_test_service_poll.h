#ifndef _ZERV_TEST_SERVICE_POLL_H_
#define _ZERV_TEST_SERVICE_POLL_H_

#include <zephyr/zerv/zerv.h>

ZERV_CMD_DECL(echo1, ZERV_IN(char str[30]), ZERV_OUT(char str[30]));
ZERV_CMD_DECL(fail1, ZERV_IN(int dummy), ZERV_OUT(int dummy));
ZERV_DECL(zerv_poll_service_1, echo1, fail1);

ZERV_CMD_DECL(echo2, ZERV_IN(char str[30]), ZERV_OUT(char str[30]));
ZERV_CMD_DECL(fail2, ZERV_IN(int dummy), ZERV_OUT(int dummy));
ZERV_DECL(zerv_poll_service_2, echo2, fail2);

#endif // _ZERV_TEST_SERVICE_POLL_H_
