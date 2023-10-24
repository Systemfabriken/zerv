// #####################################################################################################################
// # # #                              Copyright (C) 2023 DevPort Ost AB, all rights reserved. # #
// Unauthorized copying of this file, via any medium is strictly prohibited.                     #
// #                                           Proprietary and confidential. # # # # author:  Albin
// Hjalmas # # company: Systemfabriken # # contact: albin@systemfabriken.tech #
// #####################################################################################################################
#ifndef _ZIT_TEST_FUTURE_SERVICE_H_
#define _ZIT_TEST_FUTURE_SERVICE_H_
/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include <zephyr/zerv/zerv.h> // Include the Zephyr Inter Thread Service (ZIT) header.

// PUBLIC DECLARATIONS
// ################################################################################################

ZERV_CMD_DECL(future_echo, ZERV_IN(bool is_delayed, k_timeout_t delay, char str[30]),
	      ZERV_OUT(char str[30]));

ZERV_CMD_DECL(future_other, ZERV_IN(char str[30]), ZERV_OUT(char str[30]));

// Declare the service.
ZERV_DECL(future_service, future_echo, future_other);

#endif // _ZIT_TEST_FUTURE_SERVICE_H_
