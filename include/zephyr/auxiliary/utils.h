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
#ifndef _UTILS_H_
#define _UTILS_H_

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <math.h>

/**
 * @brief Print a message on its own line.
 */
#define PRINTLN(fmt, ...) LOG_PRINTK(fmt "\n", ##__VA_ARGS__)

#if defined(__CDT_PARSER__)
#undef LOG_PRINTK
#define LOG_PRINTK(...) (void)0
#endif

#define _ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Defines the rounding error for floating-point conversions.
 */
#define ROUNDED_INT(val) ((int)((val) < 0 ? (val)-0.5f : (val) + 0.5f))

/**
 * @brief Defines the format for printing floating-point numbers as integers with two decimal
 * places.
 */
#define FMT_FLOAT_D2 "%c%d.%02d"

/**
 * @brief Converts a float to its integer components for printing with two decimal precision.
 *
 * @param val The floating-point number to be converted.
 *
 * @return Returns three components: the sign as a char ('-' or ' '), the whole number, and the
 * two-decimal fraction.
 *
 * @example
 * float speed_value = 3.14;
 * LOG_INF("Speed: " FMT_FLOAT_D2, FLOAT_TO_D2(speed_value));
 */
#define FLOAT_TO_D2(val)                                                                           \
	((val) < 0 ? '-' : ' '), _ABS((int)(val)), _ABS(ROUNDED_INT((val)*100) % 100)

/**
 * @brief Defines the format for printing floating-point numbers as integers with four decimal
 * places.
 */
#define FMT_FLOAT_D4 "%c%d.%04d"

/**
 * @brief Converts a float to its integer components for printing with four decimal precision.
 *
 * @param val The floating-point number to be converted.
 *
 * @return Returns three components: the sign as a char ('-' or ' '), the whole number, and the
 * four-decimal fraction.
 *
 * @example
 * float temperature_value = 25.6785917;
 * LOG_INF("Temperature: " FMT_FLOAT_D4, FLOAT_TO_D4(temperature_value));
 * // Prints "Temperature: 25.6786"
 */
#define FLOAT_TO_D4(val)                                                                           \
	((val) < 0 ? '-' : ' '), _ABS((int)(val)), _ABS(ROUNDED_INT((val)*10000) % 10000)

static inline float signf(float val)
{
	return (val > 0.0f) - (val < 0.0f);
}

#endif // _UTILS_H_
