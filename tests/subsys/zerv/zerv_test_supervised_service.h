#ifndef _ZERV_TEST_SUPERVISED_SERVICE_H_
#define _ZERV_TEST_SUPERVISED_SERVICE_H_

#include <zephyr/kernel.h>
#include <zephyr/zerv/zerv.h>
#include <zephyr/zerv/zerv_cmd.h>
#include <zephyr/zerv/zerv_msg.h>

typedef enum {
	ZERV_TEST_SUPERVICED_STATE_RUNNING,
	ZERV_TEST_SUPERVICED_STATE_STOPPED,
	ZERV_TEST_SUPERVICED_STATE_ERROR,
} zerv_test_supervised_state_t;

ZERV_CMD_DECL(set_state, ZERV_IN(zerv_test_supervised_state_t state), ZERV_OUT_EMPTY);
ZERV_CMD_DECL(set_sampling_interval, ZERV_IN(unsigned int interval), ZERV_OUT_EMPTY);

ZERV_DECL(zerv_test_supervised_service, ZERV_CMDS(set_state, set_sampling_interval), ZERV_MSGS(),
	  ZERV_SUBSCRIBED_TOPICS());

#endif // _ZERV_TEST_SUPERVISED_SERVICE_H_