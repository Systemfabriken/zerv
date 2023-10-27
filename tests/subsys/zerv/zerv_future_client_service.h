#ifndef _ZERV_FUTURE_CLIENT_SERVICE_H_
#define _ZERV_FUTURE_CLIENT_SERVICE_H_

#include <zephyr/zerv/zerv.h> // Include the Zephyr Inter Thread Service (ZIT) header.

ZERV_CMD_DECL(call_future_echo, ZERV_IN(zerv_rc_t expected_rc), ZERV_OUT(bool was_expected_rc));
ZERV_CMD_DECL(call_other, ZERV_IN(zerv_rc_t expected_rc), ZERV_OUT(bool was_expected_rc));

ZERV_DECL(future_client, call_future_echo, call_other);

#endif
