/*=================================================================================================
 *
 *           ██████╗ ██╗████████╗███╗   ███╗ █████╗ ███╗   ██╗     █████╗ ██████╗
 *           ██╔══██╗██║╚══██╔══╝████╗ ████║██╔══██╗████╗  ██║    ██╔══██╗██╔══██╗
 *           ██████╔╝██║   ██║   ██╔████╔██║███████║██╔██╗ ██║    ███████║██████╔╝
 *           ██╔══██╗██║   ██║   ██║╚██╔╝██║██╔══██║██║╚██╗██║    ██╔══██║██╔══██╗
 *           ██████╔╝██║   ██║   ██║ ╚═╝ ██║██║  ██║██║ ╚████║    ██║  ██║██████╔╝
 *           ╚═════╝ ╚═╝   ╚═╝   ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝    ╚═╝  ╚═╝╚═════╝
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 BitMan AB
 * contact: albin@bitman.se
 *===============================================================================================*/
#include "zerv_test_periodic_thread.h"

#include <zephyr/logging/log.h>

static unsigned int _period_counter = 0;
static unsigned int _target_count = 0;

LOG_MODULE_REGISTER(periodic, CONFIG_ZERV_LOG_LEVEL);

K_SEM_DEFINE(async_sem, 0, 1);
K_SEM_DEFINE(init_start_sem, 0, 1);
K_SEM_DEFINE(init_finished_sem, 0, 1);

static int init(void)
{
	k_sem_take(&init_start_sem, K_FOREVER);
	LOG_DBG("Periodic thread initialized");
	k_sem_give(&init_finished_sem);
	_period_counter = 0;
	return 0;
}

static void tick(void)
{
	LOG_DBG("Periodic thread tick");
	if (_period_counter == _target_count) {
		k_sem_give(&async_sem);
	}
	_period_counter++;
}

ZERV_DEF_PERIODIC_THREAD(periodic_service, 1024, 1024, K_PRIO_PREEMPT(10), K_MSEC(100), init, tick);

ZERV_CMD_HANDLER_DEF(sync_timeout, in, out)
{
	LOG_DBG("Received sync_timeout: %u", in->n);
	_period_counter = 0;
	_target_count = in->n;
	return 0;
}

ZERV_MSG_HANDLER_DEF(async_timeout, msg)
{
	LOG_DBG("Received async_timeout: %u", msg->n);
	_period_counter = 0;
	_target_count = msg->n;
}

ZERV_TOPIC_HANDLER(periodic_service, test_topic, msg)
{
	LOG_DBG("Received test_topic: a=%d, b=%u, c=%c", msg->a, msg->b, msg->c);
}