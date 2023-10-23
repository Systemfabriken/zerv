//#####################################################################################################################
//#                                                                                                                   #
//#                              Copyright (C) 2023 DevPort Ost AB, all rights reserved.                              #
//#                     Unauthorized copying of this file, via any medium is strictly prohibited.                     #
//#                                           Proprietary and confidential.                                           #
//#                                                                                                                   #
//# author:  Albin Hjalmas                                                                                            #
//# company: Systemfabriken                                                                                           #
//# contact: albin@systemfabriken.tech                                                                                #
//#####################################################################################################################
// INCLUDES ###########################################################################################################
#include "zit_test_service.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zit_test_service, LOG_LEVEL_DBG);

// PRIVATE DECLARATIONS ###############################################################################################
static void zit_serv_thread(void);

// PRIVATE DEFINITIONS ################################################################################################

ZIT_SERV_DEF(zit_test_service, 1024, get_hello_world, echo, fail);

K_THREAD_DEFINE(zit_test_service_thread, 1024, (k_thread_entry_t)zit_serv_thread, NULL, NULL, NULL,
                0, 0, 0);

// PRIVATE FUNCTION DECLARATIONS ######################################################################################

void zit_serv_thread(void)
{
    while (true) {
        struct zit_req_params *params = zit_serv_get_req(&zit_test_service, K_FOREVER);
        if (!params) {
            LOG_ERR("Failed to receive request");
            continue;
        }
        LOG_DBG("Received request");

        zit_rc_t rc = zit_serv_handle_request(&zit_test_service, params);
        if (rc != 0) {
            LOG_ERR("Failed to handle request");
            continue;
        }
    }
}

ZIT_SERV_REQ_HANDLER_DEF(get_hello_world, req, resp)
{
    LOG_DBG("Received request: a: %d, b: %d", req->a, req->b);
    resp->a = req->a;
    resp->b = req->b;
    strcpy(resp->str, "Hello World!");
    return ZIT_RC_OK;
}

ZIT_SERV_REQ_HANDLER_DEF(echo, req, resp)
{
    LOG_DBG("Received request: str: %s", req->str);
    strcpy(resp->str, req->str);
    return ZIT_RC_OK;
}

ZIT_SERV_REQ_HANDLER_DEF(fail, req, resp)
{
    LOG_DBG("Received request: dummy: %d", req->dummy);
    return ZIT_RC_ERROR;
}