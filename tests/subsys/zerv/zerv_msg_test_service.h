#ifndef _ZERV_MSG_TEST_SERVICE_H_
#define _ZERV_MSG_TEST_SERVICE_H_

#include <zephyr/zerv/zerv.h>
#include <zephyr/zerv/zerv_msg.h>

ZERV_MSG_DECL(print_msg, char msg[15]);
ZERV_MSG_DECL(cmp_msg_1, int a, unsigned int b, char c, char d[15]);
ZERV_MSG_DECL(cmp_msg_2, int a, unsigned int b, char c, char d[15]);

// Declare the service.
ZERV_DECL(zerv_msg_test_service, EMPTY, ZERV_MSGS(print_msg, cmp_msg_1, cmp_msg_2));

#endif // _ZERV_MSG_TEST_SERVICE_H_
