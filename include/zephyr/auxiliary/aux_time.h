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
#ifndef _TIME_H_
#define _TIME_H_

#include <zephyr/kernel.h>
#include <zephyr/sys/time_units.h>
#include "stdint.h"

/**
 * @brief Get the current system clock ticks.
 *
 * @return The current system clock ticks.
 */
static inline uint32_t aux_time_get_ticks()
{
	return k_cycle_get_32();
}

/**
 * @brief Calculate the elapsed system clock ticks between start_ticks and end_ticks.
 *
 * @param start_ticks The start time in ticks.
 * @param end_ticks The end time in ticks.
 *
 * @return The elapsed system clock ticks.
 */
static inline uint32_t aux_time_calculate_delta_ticks(uint32_t start_ticks, uint32_t end_ticks)
{
	// detect if ticks overflowed
	if (end_ticks < start_ticks) {
		return end_ticks + (UINT_MAX - start_ticks);
	} else {
		return end_ticks - start_ticks;
	}
}

/**
 * @brief Get the number of system clock ticks since the start_ticks.
 *
 * @param start_ticks The start time in ticks.
 *
 * @return The number of system clock ticks since the start_ticks.
 */
static inline uint32_t aux_time_get_ticks_since(uint32_t start_ticks)
{
	const uint32_t current_time = aux_time_get_ticks();
	return aux_time_calculate_delta_ticks(start_ticks, current_time);
}

/**
 * @brief Get the number of system clock ticks since the start_ticks and updates start_ticks to the
 * current time.
 *
 * @param start_ticks Pointer to the start time in ticks. Will be updated to the current time in
 * ticks.
 *
 * @return The number of system clock ticks since the start_ticks.
 */
static inline uint32_t aux_time_get_ticks_since_and_update(uint32_t *start_ticks)
{
	const uint32_t current_time = aux_time_get_ticks();
	uint32_t delta_ticks = aux_time_calculate_delta_ticks(*start_ticks, current_time);
	*start_ticks = current_time;
	return delta_ticks;
}

/**
 * @brief Convert system clock ticks to nanoseconds.
 *
 * @param ticks The number of system clock ticks.
 *
 * @return The number of nanoseconds.
 */
static inline uint32_t aux_time_ticks2nanos(uint32_t ticks)
{
	return k_cyc_to_ns_near32(ticks);
}

/**
 * @brief Convert system clock ticks to microseconds.
 *
 * @param ticks The number of system clock ticks.
 *
 * @return The number of microseconds.
 */
static inline uint32_t aux_time_ticks2micros(uint32_t ticks)
{
	return k_cyc_to_us_near32(ticks);
}

/**
 * @brief Convert system clock ticks to milliseconds.
 *
 * @param ticks The number of system clock ticks.
 *
 * @return The number of milliseconds.
 */
static inline uint32_t aux_time_ticks2millis(uint32_t ticks)
{
	return k_cyc_to_ms_near32(ticks);
}

#endif // _TIME_H_