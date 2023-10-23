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
#include "zit_test_service_poll.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zit_poll_test, LOG_LEVEL_DBG);

// PRIVATE DECLARATIONS ###############################################################################################
static void zit_serv_thread(void);

// PRIVATE DEFINITIONS ################################################################################################

ZIT_SERV_DEF(zit_poll_service_1, 1024, echo1, fail1);
ZIT_SERV_DEF(zit_poll_service_2, 1024, echo2, fail2);

K_THREAD_DEFINE(zit_service_poll_thread, 2048, (k_thread_entry_t)zit_serv_thread, NULL, NULL, NULL,
                0, 0, 0);

// PRIVATE FUNCTION DECLARATIONS ######################################################################################

void zit_serv_thread(void)
{
    enum { ZIT_POLL_SERVICE_1 = 0, ZIT_POLL_SERVICE_2 = 1 };

    struct k_poll_event events[] __aligned(4) = {
        [ZIT_POLL_SERVICE_1] = ZIT_SERV_POLL_EVENT_INITIALIZER(zit_poll_service_1),
        [ZIT_POLL_SERVICE_2] = ZIT_SERV_POLL_EVENT_INITIALIZER(zit_poll_service_2),
    };

    int rc;

    while (true) {
        LOG_DBG("Waiting for request");
        k_poll(events, 2, K_FOREVER);
        LOG_DBG("Received request");

        struct zit_req_params *serv1_req = NULL;
        struct zit_req_params *serv2_req = NULL;

        if (events[ZIT_POLL_SERVICE_1].state == K_POLL_STATE_FIFO_DATA_AVAILABLE) {
            LOG_DBG("Received event on service 1");
            serv1_req = zit_serv_get_req(&zit_poll_service_1, K_NO_WAIT);
            LOG_DBG("Received request on service 1: %p", serv1_req);
        }

        if (events[ZIT_POLL_SERVICE_2].state == K_POLL_STATE_FIFO_DATA_AVAILABLE) {
            LOG_DBG("Received event on service 2");
            serv2_req = zit_serv_get_req(&zit_poll_service_2, K_NO_WAIT);
            LOG_DBG("Received request on service 2: %p", serv2_req);
        }

        events[ZIT_POLL_SERVICE_1].state = K_POLL_STATE_NOT_READY;
        events[ZIT_POLL_SERVICE_2].state = K_POLL_STATE_NOT_READY;

        if (serv1_req != NULL) {
            LOG_DBG("Handling request on service 1");
            rc = zit_serv_handle_request(&zit_poll_service_1, serv1_req);
            if (rc != 0) {
                LOG_ERR("Failed to handle request on service 1");
            }
        }

        if (serv2_req != NULL) {
            LOG_DBG("Handling request on service 2");
            rc = zit_serv_handle_request(&zit_poll_service_2, serv2_req);
            if (rc != 0) {
                LOG_ERR("Failed to handle request on service 2");
            }
        }
    }
}

ZIT_SERV_REQ_HANDLER_DEF(echo1, req, resp)
{
    LOG_DBG("Received request: str: %s", req->str);
    strcpy(resp->str, req->str);
    return ZIT_RC_OK;
}

ZIT_SERV_REQ_HANDLER_DEF(fail1, req, resp)
{
    LOG_DBG("Received request: dummy: %d", req->dummy);
    return ZIT_RC_ERROR;
}

ZIT_SERV_REQ_HANDLER_DEF(echo2, req, resp)
{
    LOG_DBG("Received request: str: %s", req->str);
    strcpy(resp->str, req->str);
    return ZIT_RC_OK;
}

ZIT_SERV_REQ_HANDLER_DEF(fail2, req, resp)
{
    LOG_DBG("Received request: dummy: %d", req->dummy);
    return ZIT_RC_ERROR;
}