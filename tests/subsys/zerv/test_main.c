
#include "pub.h"
#include "sub.h"
#include "zerv_test_service.h"
#include "zerv_test_service_poll.h"
#include <zephyr/zerv/zerv.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/auxiliary/utils.h>

LOG_MODULE_REGISTER(test_main, LOG_LEVEL_DBG);

PUB_DEFINE(pub1);
PUB_DEFINE(pub2);
SUB_DEFINE(sub1, 128);
SUB_DEFINE(sub2, 128);
SUB_DEFINE(sub3, 128);
SUB_DEFINE(sub4, 128);

// No buffer publisher
PUB_DEFINE(pub3);
SUB_DEFINE(sub5, 200);

// Even though the publisher is never defined the below line should compile
SUB_DEFINE(dummy_subscriber, 0);

// Define a publisher with no subscribers
PUB_DEFINE(lone_publisher);

ZTEST_SUITE(zerv, NULL, NULL, NULL, NULL, NULL);

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

	ztest_run_test_suite(zerv);
	LOG_PRINTK("\n\n");
}

static bool polling_test_2 = false;

char sub_1_msg[50];
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
K_THREAD_DEFINE(sub1_thread_id, 256, (k_thread_entry_t)sub1_thread, NULL, NULL, NULL, 5, 0, 0);

char sub_2_msg[50];
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
K_THREAD_DEFINE(sub2_thread_id, 256, (k_thread_entry_t)sub2_thread, NULL, NULL, NULL, 5, 0, 0);

char sub_3_msg[50];
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
K_THREAD_DEFINE(sub3_thread_id, 256, (k_thread_entry_t)sub3_thread, NULL, NULL, NULL, 5, 0, 0);

char sub_4_msg[50];
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
K_THREAD_DEFINE(sub4_thread_id, 256, (k_thread_entry_t)sub4_thread, NULL, NULL, NULL, 5, 0, 0);

char sub_5_msg[50];
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
K_THREAD_DEFINE(sub5_thread_id, 256, (k_thread_entry_t)sub5_thread, NULL, NULL, NULL, 5, 0, 0);

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
K_THREAD_DEFINE(test_polling_thread_id, 256, (k_thread_entry_t)test_polling_thread, NULL, NULL,
		NULL, 5, 0, 0);

// ZTEST(zerv, test_polling)
// {
// 	// Exit all the threads
// 	polling_test_2 = true;
// 	char msg[50] = "Exit!";
// 	int rc;
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sleep(K_MSEC(100));

// 	// Start polling thread
// 	k_sem_give(&test_polling_sem);
// 	k_sleep(K_MSEC(100));

// 	// Test data
// 	strcpy(msg, "Hello from polling test!");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sleep(K_MSEC(100));
// 	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
// 	strcpy(msg, "Hello from polling test again!");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sleep(K_MSEC(100));
// 	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);

// 	strcpy(msg, "Hello from polling test!");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub2, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sleep(K_MSEC(100));
// 	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
// 	strcpy(msg, "Hello from polling test again!");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub2, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sleep(K_MSEC(100));
// 	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
// }

// ZTEST(zerv, test_no_subscribers)
// {
// 	// Exit all the threads
// 	char msg[50] = "Testing no subscribers!";
// 	PRINTLN("Publishing: %s", msg);
// 	int rc;
// 	rc = pub_emit(&lone_publisher, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// }

// ZTEST(zerv, test_pub_sub)
// {
// 	k_sleep(K_MSEC(100));
// 	// Test data
// 	char msg[50] = "Hello world!";
// 	int rc;
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sleep(K_MSEC(100));
// 	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
// 	strcpy(msg, "Hello world again!");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub1, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sleep(K_MSEC(100));
// 	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);

// 	strcpy(msg, "Hello world from publisher 2!");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub2, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sleep(K_MSEC(100));
// 	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);
// 	strcpy(msg, "Hello world again! but from pub 2");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub2, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sleep(K_MSEC(100));
// 	zassert_mem_equal(sub_1_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_2_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_3_msg, msg, strlen(msg) + 1, NULL);
// 	zassert_mem_equal(sub_4_msg, msg, strlen(msg) + 1, NULL);

// 	// Try to break it
// 	rc = pub_emit(&pub1, msg, 0);
// 	zassert_equal(rc, -EINVAL, NULL);
// 	rc = pub_emit(&pub2, NULL, 1);
// 	zassert_equal(rc, -EINVAL, NULL);
// 	rc = pub_emit(&pub1, NULL, 0);
// 	zassert_equal(rc, -EINVAL, NULL);
// }

// ZTEST(zerv, test_pub_sub_no_buffer)
// {
// 	k_sleep(K_MSEC(100));
// 	// Test data
// 	char msg[100] = "Hello world!";
// 	int rc;
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sem_take(&sub5_sem, K_FOREVER);
// 	zassert_mem_equal(sub_5_msg, msg, strlen(msg) + 1, NULL);

// 	strcpy(msg, "Hello world again!");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sem_take(&sub5_sem, K_FOREVER);
// 	zassert_mem_equal(sub_5_msg, msg, strlen(msg) + 1, NULL);

// 	strcpy(msg, "Hello world again!");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sem_take(&sub5_sem, K_FOREVER);
// 	strcpy(msg, "Hello world again! 22");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sem_take(&sub5_sem, K_FOREVER);

// 	dont_free_message = true;
// 	strcpy(msg, "This message should be auto freed");
// 	PRINTLN("Publishing: %s", msg);
// 	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);
// 	k_sem_take(&sub5_sem, K_FOREVER);
// 	strcpy(msg, "So this message should be received");
// 	PRINTLN("Publishing: %s", msg);
// 	dont_receive_message = true;
// 	dont_free_message = false;
// 	rc = pub_emit(&pub3, msg, strlen(msg) + 1);
// 	zassert_equal(rc, 0, NULL);

// 	k_sleep(K_MSEC(100));

// 	// Try to overflow subscriber
// 	PRINTLN("Now overflowing the subscriber");
// 	char scrap[130]; // Should overflow after seconf publish!
// 	rc = pub_emit(&pub3, scrap, ARRAY_SIZE(scrap));
// 	zassert_not_equal(rc, 0, NULL);

// 	PRINTLN("Receiving messages");
// 	dont_receive_message = false;
// 	k_sem_give(&sub5_lock_sem);
// 	k_sem_take(&sub5_sem, K_FOREVER);
// }

ZTEST(zerv, hello_world)
{
	{
		ZERV_CALL(zerv_test_service, get_hello_world, rc, p_ret, 10, 20);
		zassert_equal(rc, 0, NULL);
		zassert_equal(p_ret->a, 10, NULL);
		zassert_equal(p_ret->b, 20, NULL);
		zassert_equal(strcmp(p_ret->str, "Hello World!"), 0, NULL);
		if (p_ret->a == 10 && p_ret->b == 20) {
			PRINTLN("get_hello_world returned: %s", p_ret->str);
		} else {
			PRINTLN("get_hello_world returned: %s, but the parameters were wrong!",
				p_ret->str);
		}
	}

	{
		ZERV_CALL(zerv_test_service, echo, rc, p_ret, "Hello World!");
		zassert_equal(rc, 0, NULL);
		zassert_equal(strcmp(p_ret->str, "Hello World!"), 0, NULL);
	}

	{
		ZERV_CALL(zerv_test_service, echo, rc, p_ret, "Hello World! 2");
		zassert_equal(rc, 0, NULL);
		zassert_equal(strcmp(p_ret->str, "Hello World! 2"), 0, NULL);
	}

	{
		ZERV_CALL(zerv_test_service, fail, rc, p_ret);
		zassert_equal(rc, ZERV_RC_ERROR, NULL);
	}

	{
		ZERV_CALL(zerv_test_service, read_hello_world, rc, p_ret);
		zassert_equal(rc, 0, NULL);
		zassert_equal(strcmp(p_ret->str, "Hello World!"), 0, NULL);
	}

	{
		ZERV_CALL(zerv_test_service, print_hello_world, rc, p_ret);
		zassert_equal(rc, 0, NULL);
	}
}

ZTEST(zerv, event_processor_thread)
{
	PRINTLN("Sending echo1 request");
	{
		ZERV_CALL(zerv_poll_service_1, echo1, rc, echo1_resp, "Hello World!");
		zassert_equal(rc, 0, NULL);
		zassert_equal(strcmp(echo1_resp->str, "Hello World!"), 0, NULL);
		PRINTLN("OK");
	}

	PRINTLN("Sending echo2 request");
	{
		ZERV_CALL(zerv_poll_service_2, echo2, rc, echo2_resp, "Hello World!");
		zassert_equal(rc, 0, NULL);
		zassert_equal(strcmp(echo2_resp->str, "Hello World!"), 0, NULL);
		PRINTLN("OK");
	}

	PRINTLN("Sending echo1 request");
	{
		ZERV_CALL(zerv_poll_service_1, echo1, rc, echo1_resp, "Hello World! 2");
		zassert_equal(rc, 0, NULL);
		zassert_equal(strcmp(echo1_resp->str, "Hello World! 2"), 0, NULL);
		PRINTLN("OK");
	}

	PRINTLN("Sending echo2 request");
	{
		ZERV_CALL(zerv_poll_service_2, echo2, rc, echo2_resp, "Hello World! 2");
		zassert_equal(rc, 0, NULL);
		zassert_equal(strcmp(echo2_resp->str, "Hello World! 2"), 0, NULL);
		PRINTLN("OK");
	}

	PRINTLN("Sending fail1 request");
	{
		ZERV_CALL(zerv_poll_service_1, fail1, rc, p_ret);
		zassert_equal(rc, ZERV_RC_ERROR, NULL);
		PRINTLN("OK");
	}

	PRINTLN("Sending fail2 request");
	{
		ZERV_CALL(zerv_poll_service_2, fail2, rc, p_ret);
		zassert_equal(rc, ZERV_RC_ERROR, NULL);
		PRINTLN("OK");
	}

	PRINTLN("Signalling event processor thread by sem");
	k_sem_give(&event_sem);
	int rc = k_sem_take(&event_sem_response, K_SECONDS(1));
	zassert_equal(rc, 0, NULL);
	PRINTLN("OK");
}
