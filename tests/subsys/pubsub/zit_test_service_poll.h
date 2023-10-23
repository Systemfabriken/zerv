// #####################################################################################################################
// # # #                              Copyright (C) 2023 DevPort Ost AB, all rights reserved. # #
// Unauthorized copying of this file, via any medium is strictly prohibited.                     #
// #                                           Proprietary and confidential. # # # # author:  Albin
// Hjalmas # # company: Systemfabriken # # contact: albin@systemfabriken.tech #
// #####################################################################################################################
#ifndef _ZIT_TEST_SERVICE_POLL_H_
#define _ZIT_TEST_SERVICE_POLL_H_
// INCLUDES
// ###########################################################################################################
#include "zit_serv.h" // Include the Zephyr Inter Thread Service (ZIT) header.

// PUBLIC DECLARATIONS
// ################################################################################################

// Service #1 Requests and Responses
// ---------------------------------------------------------------------------------- Define a
// request of the service that will echo the string.
ZERV_CMD_DECL(echo1, ZERV_IN(char str[30]), ZERV_OUT(char str[30]));

// Define a request of the service that will always fail.
ZERV_CMD_DECL(fail1, ZERV_IN(int dummy), ZERV_OUT(int dummy));

// Declare the service.
ZIT_SERV_DECL(zit_poll_service_1, echo1, fail1);

// Service #2 Requests and Responses
// ---------------------------------------------------------------------------------- Define a
// request of the service that will echo the string.
ZERV_CMD_DECL(echo2, ZERV_IN(char str[30]), ZERV_OUT(char str[30]));

// Define a request of the service that will always fail.
ZERV_CMD_DECL(fail2, ZERV_IN(int dummy), ZERV_OUT(int dummy));

// Declare the service.
ZIT_SERV_DECL(zit_poll_service_2, echo2, fail2);

#endif // _ZIT_TEST_SERVICE_POLL_H_
