// #####################################################################################################################
// # # #                              Copyright (C) 2023 DevPort Ost AB, all rights reserved. # #
// Unauthorized copying of this file, via any medium is strictly prohibited.                     #
// #                                           Proprietary and confidential. # # # # author:  Albin
// Hjalmas # # company: Systemfabriken # # contact: albin@systemfabriken.tech #
// #####################################################################################################################
#ifndef _ZIT_TEST_SERVICE_H_
#define _ZIT_TEST_SERVICE_H_
// INCLUDES
// ###########################################################################################################
#include "zit_serv.h" // Include the Zephyr Inter Thread Service (ZIT) header.

// PUBLIC DECLARATIONS
// ################################################################################################

// Define a requests of the service that will retrieve the string "Hello World!" and respond with
// the integers a and b.
ZERV_CMD_DECL(get_hello_world, ZERV_IN(int a, int b), ZERV_OUT(char str[30], int32_t a, int32_t b));

// Define a request of the service that will echo the string.
ZERV_CMD_DECL(echo, ZERV_IN(char str[30]), ZERV_OUT(char str[30]));

// Define a request of the service that will always fail.
ZERV_CMD_DECL(fail, ZERV_IN(int dummy), ZERV_OUT(int dummy));

// Declare the service.
ZIT_SERV_DECL(zit_test_service, get_hello_world, echo, fail);

#endif // _ZIT_TEST_SERVICE_H_
