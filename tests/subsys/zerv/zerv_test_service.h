#ifndef _ZERV_TEST_SERVICE_H_
#define _ZERV_TEST_SERVICE_H_

#include <zephyr/kernel.h>
#include <zephyr/zerv/zerv.h>
#include <zephyr/zerv/zerv_cmd.h>
#include <zephyr/zerv/zerv_msg.h>

extern struct k_sem test_msg_sem;

// Define a requests of the service that will retrieve the string "Hello World!" and respond with
// the integers a and b.
ZERV_CMD_DECL(get_hello_world, ZERV_IN(int a, int b), ZERV_OUT(char str[30], int32_t a, int32_t b));

// Define a request of the service that will echo the string.
ZERV_CMD_DECL(echo, ZERV_IN(char str[30]), ZERV_OUT(char str[30]));

// Define a request of the service that will always fail.
ZERV_CMD_DECL(fail, ZERV_IN_EMPTY, ZERV_OUT(int dummy));

// Define a request of the service that will read the string "Hello World!".
ZERV_CMD_DECL(read_hello_world, ZERV_IN_EMPTY, ZERV_OUT(char str[30]));

// Define a request that will print the string "Hello World!".
ZERV_CMD_DECL(print_hello_world, ZERV_IN_EMPTY, ZERV_OUT_EMPTY);

ZERV_MSG_DECL(test_msg, char str[30], int32_t a, int32_t b);

// Declare the service.
ZERV_DECL(zerv_test_service,
	  ZERV_CMDS(get_hello_world, echo, fail, read_hello_world, print_hello_world),
	  ZERV_MSGS(test_msg));

#endif // _ZERV_TEST_SERVICE_H_
