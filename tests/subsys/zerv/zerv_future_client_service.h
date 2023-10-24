// #####################################################################################################################
// # # #                              Copyright (C) 2023 DevPort Ost AB, all rights reserved. # #
// Unauthorized copying of this file, via any medium is strictly prohibited.                     #
// #                                           Proprietary and confidential. # # # # author:  Albin
// Hjalmas # # company: Systemfabriken # # contact: albin@systemfabriken.tech #
// #####################################################################################################################
#ifndef _ZIT_FUTURE_CLIENT_SERVICE_H_
#define _ZIT_FUTURE_CLIENT_SERVICE_H_
/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include <zephyr/zerv/zerv.h> // Include the Zephyr Inter Thread Service (ZIT) header.

// PUBLIC DECLARATIONS
// ################################################################################################

ZERV_CMD_DECL(call_future_echo, ZERV_IN(zerv_rc_t expected_rc), ZERV_OUT(bool was_expected_rc));
ZERV_CMD_DECL(call_other, ZERV_IN(zerv_rc_t expected_rc), ZERV_OUT(bool was_expected_rc));

// Declare the service.
ZERV_DECL(future_client, call_future_echo, call_other);

#endif // _ZIT_FUTURE_CLIENT_SERVICE_H_
