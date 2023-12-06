#ifndef _ZERV_MSG_TEST_SERVICE_H_
#define _ZERV_MSG_TEST_SERVICE_H_

#include <zephyr/zerv/zerv.h>
#include <zephyr/zerv/zerv_msg.h>
#include <zephyr/zerv/zerv_cmd.h>
#include <zephyr/zerv/zerv_topic.h>

extern struct k_sem print_msg_sem;
extern struct k_sem cmp_msg_1_sem;
extern struct k_sem cmp_msg_2_sem;

ZERV_MSG_DECL(print_msg, char msg[15]);
ZERV_MSG_DECL(cmp_msg_1, int a, unsigned int b, char c, char d[15]);
ZERV_MSG_DECL(cmp_msg_2, int a, unsigned int b, char c, char d[15]);
ZERV_MSG_RAW_DECL(raw_msg);

ZERV_CMD_DECL(emit_on_test_topic, ZERV_IN(int a, unsigned int b, char c), ZERV_OUT_EMPTY);
ZERV_TOPIC_DECL(test_topic, int a, unsigned int b, char c);

// Declare the service.
ZERV_DECL(zerv_msg_test_service, ZERV_CMDS(emit_on_test_topic),
	  ZERV_MSGS(print_msg, cmp_msg_1, cmp_msg_2, raw_msg), EMPTY);

#endif // _ZERV_MSG_TEST_SERVICE_H_
