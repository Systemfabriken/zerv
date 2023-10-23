// #####################################################################################################################
// # # #                              Copyright (C) 2023 DevPort Ost AB, all rights reserved. # #
// Unauthorized copying of this file, via any medium is strictly prohibited.                     #
// #                                           Proprietary and confidential. # # # # author:  Albin
// Hjalmas # # company: Systemfabriken # # contact: albin@systemfabriken.tech #
// #####################################################################################################################
//  INCLUDES
//  ###########################################################################################################
#include "pub.h"
#include "sub.h"
#include "zit_test_service.h"
#include "zit_test_service_poll.h"
#include "zit_test_future_service.h"
#include "zit_future_client_service.h"
#include "zit_client.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/auxiliary/utils.h>

// PRIVATE DECLARATIONS
// ###############################################################################################
LOG_MODULE_REGISTER(test_main, LOG_LEVEL_DBG);

PUB_DEFINE(pub1);
PUB_DEFINE(pub2);
SUB_DEFINE(sub1, 256);
SUB_DEFINE(sub2, 256);
SUB_DEFINE(sub3, 256);
SUB_DEFINE(sub4, 256);

// No buffer publisher
PUB_DEFINE(pub3);
SUB_DEFINE(sub5, 256);

// Even though the publisher is never defined the below line should compile
SUB_DEFINE(dummy_subscriber, 256);

// Define a publisher with no subscribers
PUB_DEFINE(lone_publisher);

// PRIVATE FUNCTION DECLARATIONS
// ######################################################################################
void test_polling(void);
void test_no_subscribers(void);
void test_pub_sub(void);
void test_pub_sub_no_buffer(void);
void test_zit_get_hello_world(void);
void test_zit_service_poll(void);
void test_zit_future_service(void);

// PROGRAM ENTRY
// ######################################################################################################
void test_main(void)
{
	PRINTLN("Starting test_main");

	pub_add_subscriber(&pub1, &sub1);
	pub_add_subscriber(&pub1, &sub2);
	pub_add_subscriber(&pub1, &sub3);
	pub_add_subscriber(&pub1, &sub4);

	pub_add_subscriber(&pub2, &sub1);
	pub_add_subscriber(&pub2, &sub2);
	pub_add_subscriber(&pub2, &sub3);
	pub_add_subscriber(&pub2, &sub4);

	pub_add_subscriber(&pub3, &sub5);

	k_sleep(K_MSEC(100));

	// Run tests
	ztest_test_suite(
		pubsub_test, ztest_unit_test(test_pub_sub), ztest_unit_test(test_polling),
		ztest_unit_test(test_no_subscribers), ztest_unit_test(test_pub_sub_no_buffer),
		ztest_unit_test(test_zit_get_hello_world), ztest_unit_test(test_zit_service_poll),
		ztest_unit_test(test_zit_future_service));
	ztest_run_test_suite(pubsub_test);

	LOG_PRINTK("\n\n");
}

// TESTS
// ##############################################################################################################

static bool polling_test_2 = false;

char sub_1_msg[100];
void sub1_thread(void)
{
	PRINTLN("sub1_thread started");
	while (1) {
		const struct pub *p_pub =
			sub_wait_and_receive(&sub1, sub_1_msg, sizeof(sub_1_msg), NULL, K_FOREVER);
		if (polling_test_2) {
			return;
		}
		PRINTLN("from publisher %s sub1 received: %s", p_pub->name, sub_1_msg);
	}
}
K_THREAD_DEFINE(sub1_thread_id, 1024, (k_thread_entry_t)sub1_thread, NULL, NULL, NULL, 5, 0, 0);

char sub_2_msg[100];
void sub2_thread(void)
{
	PRINTLN("sub2_thread started");
	while (1) {
		const struct pub *p_pub =
			sub_wait_and_receive(&sub2, sub_2_msg, sizeof(sub_2_msg), NULL, K_FOREVER);
		if (polling_test_2) {
			return;
		}
		PRINTLN("from publisher %s received: %s", p_pub->name, sub_2_msg);
	}
}
K_THREAD_DEFINE(sub2_thread_id, 1024, (k_thread_entry_t)sub2_thread, NULL, NULL, NULL, 5, 0, 0);

char sub_3_msg[100];
void sub3_thread(void)
{
	PRINTLN("sub3_thread started");
	while (1) {
		const struct pub *p_pub =
			sub_wait_and_receive(&sub3, sub_3_msg, sizeof(sub_3_msg), NULL, K_FOREVER);
		if (polling_test_2) {
			return;
		}
		PRINTLN("from publisher %s sub3 received: %s", p_pub->name, sub_3_msg);
	}
}
K_THREAD_DEFINE(sub3_thread_id, 1024, (k_thread_entry_t)sub3_thread, NULL, NULL, NULL, 5, 0, 0);

char sub_4_msg[100];
void sub4_thread(void)
{
	PRINTLN("sub4_thread started");
	while (1) {
		const struct pub *p_pub =
			sub_wait_and_receive(&sub4, sub_4_msg, sizeof(sub_4_msg), NULL, K_FOREVER);
		if (polling_test_2) {
			return;
		}
		PRINTLN("from publisher %s sub4 received: %s", p_pub->name, sub_4_msg);
	}
}
K_THREAD_DEFINE(sub4_thread_id, 1024, (k_thread_entry_t)sub4_thread, NULL, NULL, NULL, 5, 0, 0);

char sub_5_msg[100];
static bool dont_free_message = false;
static bool dont_receive_message = false;
K_SEM_DEFINE(sub5_sem, 0, 1);
K_SEM_DEFINE(sub5_lock_sem, 0, 1);
void sub5_thread(void)
{
	PRINTLN("sub5_thread started");
	while (1) {
		if (dont_receive_message) {
			PRINTLN("Locking sub5_thread");
			k_sem_take(&sub5_lock_sem, K_FOREVER);
		}

		int rc = sub_wait(&sub5, K_FOREVER);
		zassert_equal(rc, 0, NULL);

		size_t size = 0;
		char *p_msg = sub_receive(&sub5, &size);
		zassert_not_null(p_msg, NULL);

		memcpy(sub_5_msg, p_msg, size);

		PRINTLN("sub5 received: %s", sub_5_msg);

		if (!dont_free_message) {
			sub_free(&sub5, p_msg);
		} else {
			PRINTLN("Not freeing the received notification!");
		}

		k_sem_give(&sub5_sem);
	}
}
K_THREAD_DEFINE(sub5_thread_id, 1024, (k_thread_entry_t)sub5_thread, NULL, NULL, NULL, 5, 0, 0);

K_SEM_DEFINE(test_polling_sem, 0, 1);
void test_polling_thread(void)
{
	struct k_poll_event events[] = {
		SUB_K_POLL_EVENT_INITIALIZER(sub1),
		SUB_K_POLL_EVENT_INITIALIZER(sub2),
		SUB_K_POLL_EVENT_INITIALIZER(sub3),
		SUB_K_POLL_EVENT_INITIALIZER(sub4),
	};

	k_sem_take(&test_polling_sem, K_FOREVER);
	PRINTLN("test_polling_thread started");
	while (1) {
		k_poll(events, 4, K_FOREVER);

		if (events[0].state == K_POLL_STATE_FIFO_DATA_AVAILABLE) {
			const struct pub *p_pub = sub_wait_and_receive(
				&sub1, sub_1_msg, sizeof(sub_1_msg), NULL, K_FOREVER);
			PRINTLN("sub1 received: %s from %s", sub_1_msg, p_pub->name);
		}

		if (events[1].state == K_POLL_STATE_FIFO_DATA_AVAILABLE) {
			const struct pub *p_pub = sub_wait_and_receive(
				&sub2, sub_2_msg, sizeof(sub_2_msg), NULL, K_FOREVER);
			PRINTLN("sub2 received: %s from %s", sub_2_msg, p_pub->name);
		}

		if (events[2].state == K_POLL_STATE_FIFO_DATA_AVAILABLE) {
			const struct pub *p_pub = sub_wait_and_receive(
				&sub3, sub_3_msg, sizeof(sub_3_msg), NULL, K_FOREVER);
			PRINTLN("sub3 received: %s from %s", sub_3_msg, p_pub->name);
		}

		if (events[3].state == K_POLL_STATE_FIFO_DATA_AVAILABLE) {
			const struct pub *p_pub = sub_wait_and_receive(
				&sub4, sub_4_msg, sizeof(sub_4_msg), NULL, K_FOREVER);
			PRINTLN("sub4 received: %s from %s", sub_4_msg, p_pub->name);
		}

		events[0].state = K_POLL_STATE_NOT_READY;
		events[1].state = K_POLL_STATE_NOT_READY;
		events[2].state = K_POLL_STATE_NOT_READY;
		events[3].state = K_POLL_STATE_NOT_READY;
	}
}
K_THREAD_DEFINE(test_polling_thread_id, 2048, (k_thread_entry_t)test_polling_thread, NULL, NULL,
		NULL, 5, 0, 0);

void test_polling(void)
{
	// Exit all the threads
	polling_test_2 = true;
	char msg[50] = "Exit!";
	int rc;
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sleep(K_MSEC(100));

	// Start polling thread
	k_sem_give(&test_polling_sem);
	k_sleep(K_MSEC(100));

	// Test data
	strcpy(msg, "Hello from polling test!");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sleep(K_MSEC(100));
	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
	strcpy(msg, "Hello from polling test again!");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sleep(K_MSEC(100));
	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);

	strcpy(msg, "Hello from polling test!");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub2, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sleep(K_MSEC(100));
	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
	strcpy(msg, "Hello from polling test again!");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub2, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sleep(K_MSEC(100));
	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
}

void test_no_subscribers(void)
{
	// Exit all the threads
	char msg[50] = "Testing no subscribers!";
	PRINTLN("Publishing: %s", msg);
	int rc;
	rc = pub_emit(&lone_publisher, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
}

void test_pub_sub(void)
{
	k_sleep(K_MSEC(100));
	// Test data
	char msg[100] = "Hello world!";
	int rc;
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sleep(K_MSEC(100));
	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
	strcpy(msg, "Hello world again!");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sleep(K_MSEC(100));
	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);

	strcpy(msg, "Hello world from publisher 2!");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub2, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sleep(K_MSEC(100));
	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
	strcpy(msg, "Hello world again! but from pub 2");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub2, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sleep(K_MSEC(100));
	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);

	// Try to break it
	rc = pub_emit(&pub1, msg, 0);
	zassert_equal(rc, -EINVAL, NULL);
	rc = pub_emit(&pub2, NULL, 1);
	zassert_equal(rc, -EINVAL, NULL);
	rc = pub_emit(&pub1, NULL, 0);
	zassert_equal(rc, -EINVAL, NULL);
}

void test_pub_sub_no_buffer(void)
{
	k_sleep(K_MSEC(100));
	// Test data
	char msg[100] = "Hello world!";
	int rc;
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sem_take(&sub5_sem, K_FOREVER);
	zassert_mem_equal(sub_5_msg, msg, strlen(msg) + 1, NULL);

	strcpy(msg, "Hello world again!");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sem_take(&sub5_sem, K_FOREVER);
	zassert_mem_equal(sub_5_msg, msg, strlen(msg) + 1, NULL);

	strcpy(msg, "Hello world again!");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sem_take(&sub5_sem, K_FOREVER);
	strcpy(msg, "Hello world again! 22");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sem_take(&sub5_sem, K_FOREVER);

	dont_free_message = true;
	strcpy(msg, "This message should be auto freed");
	PRINTLN("Publishing: %s", msg);
	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);
	k_sem_take(&sub5_sem, K_FOREVER);
	strcpy(msg, "So this message should be received");
	PRINTLN("Publishing: %s", msg);
	dont_receive_message = true;
	dont_free_message = false;
	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
	zassert_equal(rc, 0, NULL);

	k_sleep(K_MSEC(100));

	// Try to overflow subscriber
	PRINTLN("Now overflowing the subscriber");
	char scrap[130]; // Should overflow after seconf publish!
	rc = pub_emit(&pub3, scrap, ARRAY_SIZE(scrap));
	zassert_not_equal(rc, 0, NULL);

	PRINTLN("Receiving messages");
	dont_receive_message = false;
	k_sem_give(&sub5_lock_sem);
	k_sem_take(&sub5_sem, K_FOREVER);
}

void test_zit_get_hello_world(void)
{
	{
		ZERV_CALL(zit_test_service, get_hello_world, rc, p_ret, 10, 20);
		zassert_equal(rc, 0, NULL);
		zassert_equal(p_ret->a, 10, NULL);
		zassert_equal(p_ret->b, 20, NULL);
		zassert_equal(strcmp(p_ret->str, "Hello World!"), 0, NULL);
	}

	{
		ZERV_CALL(zit_test_service, echo, rc, p_ret, "Hello World!");
		zassert_equal(rc, 0, NULL);
		zassert_equal(strcmp(p_ret->str, "Hello World!"), 0, NULL);
	}

	{
		ZERV_CALL(zit_test_service, echo, rc, p_ret, "Hello World! 2");
		zassert_equal(rc, 0, NULL);
		zassert_equal(strcmp(p_ret->str, "Hello World! 2"), 0, NULL);
	}

	{
		ZERV_CALL(zit_test_service, fail, rc, p_ret);
		zassert_equal(rc, ZIT_RC_ERROR, NULL);
	}
}

void test_zit_service_poll(void)
{
	PRINTLN("Sending echo1 request");
	echo1_ret_t echo1_resp = {0};

	int rc = zit_client_call(zit_poll_service_1, echo1,
				 (&(echo1_param_t){.str = "Hello World!"}), &echo1_resp);
	zassert_equal(rc, 0, NULL);
	zassert_equal(strcmp(echo1_resp.str, "Hello World!"), 0, NULL);

	PRINTLN("Sending echo2 request");
	echo2_ret_t echo2_resp = {0};
	rc = zit_client_call(zit_poll_service_2, echo2, (&(echo2_param_t){.str = "Hello World!"}),
			     &echo2_resp);
	zassert_equal(rc, 0, NULL);
	zassert_equal(strcmp(echo2_resp.str, "Hello World!"), 0, NULL);

	PRINTLN("Sending echo1 request");
	rc = zit_client_call(zit_poll_service_1, echo1, (&(echo1_param_t){.str = "Hello World! 2"}),
			     &echo1_resp);
	zassert_equal(rc, 0, NULL);
	zassert_equal(strcmp(echo1_resp.str, "Hello World! 2"), 0, NULL);

	PRINTLN("Sending echo2 request");
	rc = zit_client_call(zit_poll_service_2, echo2, (&(echo2_param_t){.str = "Hello World! 2"}),
			     &echo2_resp);
	zassert_equal(rc, 0, NULL);
	zassert_equal(strcmp(echo2_resp.str, "Hello World! 2"), 0, NULL);

	PRINTLN("Sending fail1 request");
	rc = zit_client_call(zit_poll_service_1, fail1, &(fail1_param_t){.dummy = 0},
			     &(fail1_ret_t){0});
	zassert_equal(rc, ZIT_RC_ERROR, NULL);

	PRINTLN("Sending fail2 request");
	rc = zit_client_call(zit_poll_service_2, fail2, &(fail2_param_t){.dummy = 0},
			     &(fail2_ret_t){0});
	zassert_equal(rc, ZIT_RC_ERROR, NULL);

	PRINTLN("Sending NULL request");
	rc = zit_client_call(zit_poll_service_1, echo1, NULL, &echo1_resp);
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);

	PRINTLN("Sending NULL request");
	rc = zit_client_call(zit_poll_service_2, echo2, NULL, &echo2_resp);
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);

	PRINTLN("Sending NULL response");
	rc = zit_client_call(zit_poll_service_1, echo1, (&(echo1_param_t){.str = "Hello World!"}),
			     NULL);
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);

	PRINTLN("Sending NULL response");
	rc = zit_client_call(zit_poll_service_2, echo2, (&(echo2_param_t){.str = "Hello World!"}),
			     NULL);
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);

	PRINTLN("Sending NULL request and response");
	rc = zit_client_call(zit_poll_service_1, echo1, NULL, NULL);
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);

	PRINTLN("Sending NULL request and response");
	rc = zit_client_call(zit_poll_service_2, echo2, NULL, NULL);
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);

	PRINTLN("Sending NULL request");
	rc = zit_client_call(zit_poll_service_1, fail1, NULL, &(fail1_ret_t){0});
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);

	PRINTLN("Sending NULL request");
	rc = zit_client_call(zit_poll_service_2, fail2, NULL, &(fail2_ret_t){0});
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);

	PRINTLN("Sending NULL response");
	rc = zit_client_call(zit_poll_service_1, fail1, &(fail1_param_t){.dummy = 0}, NULL);
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);

	PRINTLN("Sending NULL response");
	rc = zit_client_call(zit_poll_service_2, fail2, &(fail2_param_t){.dummy = 0}, NULL);
	zassert_equal(rc, ZIT_RC_NULLPTR, NULL);
}

K_SEM_DEFINE(future_sem, 0, 1);
void on_future_cb(void)
{
	future_echo_ret_t future_echo_resp = {0};
	zit_rc_t rc =
		zit_client_get_future(future_service, future_echo, &future_echo_resp, K_NO_WAIT);
	PRINTLN("Future response received: %s, rc: %d", future_echo_resp.str, rc);
	k_sem_give(&future_sem);
}

void test_zit_future_service(void)
{
	extern struct zit_service future_service;
	struct zit_serv_req_instance *future_echo_instance =
		future_service.req_instances[__future_echo_id];

	PRINTLN("Sending syncronous echo request");
	future_echo_ret_t future_echo_resp = {0};
	zit_rc_t rc = zit_client_call(
		future_service, future_echo,
		(&(future_echo_param_t){.is_delayed = false, .str = "Hello World!"}),
		&future_echo_resp);

	zassert_equal(rc, ZIT_RC_OK);
	zassert_equal(strcmp(future_echo_resp.str, "Hello World!"), 0);
	zassert_equal(future_echo_instance->future.is_active, false);
	zassert_is_null(future_echo_instance->future.req_params);
	zassert_not_null(future_echo_instance->future.resp);
	zassert_equal(future_echo_instance->future.resp_len, sizeof(future_echo_ret_t));

	PRINTLN("Sending asyncronous echo request and blocking until it is handled");
	future_echo_param_t params = {
		.is_delayed = true, .delay = K_MSEC(100), .str = "Async Hello Blocking!"};
	rc = zit_client_call(future_service, future_echo, &params, &future_echo_resp);
	zassert_equal(rc, ZIT_RC_FUTURE);
	zassert_true(future_echo_instance->future.is_active);
	zassert_not_null(future_echo_instance->future.req_params);
	zassert_not_null(future_echo_instance->future.resp);
	future_echo_param_t *p_params = (future_echo_param_t *)&future_echo_instance->future
						.req_params->client_req_params.data;
	zassert_mem_equal(p_params, &params, sizeof(future_echo_param_t));

	PRINTLN("Waiting for asyncronous echo request to be handled");
	rc = zit_client_get_future(future_service, future_echo, &future_echo_resp, K_FOREVER);
	PRINTLN("Asyncronous echo request handled");
	zassert_equal(rc, ZIT_RC_OK);
	zassert_equal(strcmp(future_echo_resp.str, "Async Hello Blocking!"), 0);
	zassert_equal(future_echo_instance->future.is_active, false);
	zassert_is_null(future_echo_instance->future.req_params);
	zassert_not_null(future_echo_instance->future.resp);

	PRINTLN("Sending asyncronous echo request and polling until it is handled");
	params = (future_echo_param_t){
		.is_delayed = true, .delay = K_MSEC(100), .str = "Async Hello Polling!"};
	rc = zit_client_call(future_service, future_echo, &params, &future_echo_resp);
	zassert_equal(rc, ZIT_RC_FUTURE);
	zassert_true(future_echo_instance->future.is_active);
	zassert_not_null(future_echo_instance->future.req_params);
	zassert_not_null(future_echo_instance->future.resp);
	static int cnt = 0;
	while ((rc = zit_client_get_future(future_service, future_echo, &future_echo_resp,
					   K_NO_WAIT)) == ZIT_RC_TIMEOUT) {
		zassert_true(future_echo_instance->future.is_active);
		zassert_not_null(future_echo_instance->future.req_params);
		zassert_not_null(future_echo_instance->future.resp);
		cnt++;

		if (cnt == 5) {
			PRINTLN("Sending another request to the same service");
			call_other_ret_t call_other_resp = {0};
			zit_rc_t client_call_rc =
				zit_client_call(future_client, call_other,
						(&(call_other_param_t){.expected_rc = ZIT_RC_OK}),
						&call_other_resp);
			zassert_equal(client_call_rc, ZIT_RC_OK);
			zassert_true(call_other_resp.was_expected_rc);
			PRINTLN("Another request to the same service sent and handled");
		}

		k_sleep(K_MSEC(10));
	}
	PRINTLN("Asyncronous echo request handled");
	zassert_equal(rc, ZIT_RC_OK);
	zassert_equal(strcmp(future_echo_resp.str, "Async Hello Polling!"), 0);
	zassert_equal(future_echo_instance->future.is_active, false);
	zassert_is_null(future_echo_instance->future.req_params);

	PRINTLN("Calling asynchronous request from another thread");
	call_future_echo_ret_t call_future_echo_resp = {0};
	call_future_echo_resp.on_delayed_response = on_future_cb;
	rc = zit_client_call(future_client, call_future_echo,
			     (&(call_future_echo_param_t){.expected_rc = ZIT_RC_OK}),
			     &call_future_echo_resp);
	zassert_equal(rc, ZIT_RC_OK);
	zassert_true(call_future_echo_resp.was_expected_rc);

	PRINTLN("Sending asyncronous echo request and handling response in callback");
	params = (future_echo_param_t){
		.is_delayed = true, .delay = K_MSEC(100), .str = "Async Hello Callback!"};
	future_echo_ret_t future_echo_resp2 = {0};
	future_echo_resp2.on_delayed_response = on_future_cb;
	rc = zit_client_call(future_service, future_echo, &params, &future_echo_resp2);
	zassert_equal(rc, ZIT_RC_FUTURE);
	zassert_true(future_echo_instance->future.is_active);
	zassert_not_null(future_echo_instance->future.req_params);
	zassert_not_null(future_echo_instance->future.resp);

	// Waiting for callback
	rc = k_sem_take(&future_sem, K_SECONDS(1));
	zassert_equal(rc, 0);
	PRINTLN("Asyncronous echo request handled");
}