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
#ifndef _ZERV_TEST_PERIODIC_THREAD_H_
#define _ZERV_TEST_PERIODIC_THREAD_H_

#include <zephyr/kernel.h>
#include <zephyr/zerv/zerv.h>
#include <zephyr/zerv/zerv_cmd.h>
#include <zephyr/zerv/zerv_msg.h>

extern struct k_sem async_sem;
extern struct k_sem init_start_sem;
extern struct k_sem init_finished_sem;

ZERV_CMD_DECL(sync_timeout, ZERV_IN(unsigned int n), ZERV_OUT_EMPTY);
ZERV_MSG_DECL(async_timeout, unsigned int n);

ZERV_DECL(periodic_service, ZERV_CMDS(sync_timeout), ZERV_MSGS(async_timeout));

#endif /* _ZERV_TEST_PERIODIC_THREAD_H_ */