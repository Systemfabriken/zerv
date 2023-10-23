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
#include "pub.h"
#include "sub.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <stddef.h>

LOG_MODULE_REGISTER(pub, CONFIG_PUBSUB_LOG_LEVEL);

// PUBLIC FUNCTION DEFINITIONS ########################################################################################

int pub_add_subscriber(struct pub *pub, struct sub *sub)
{
    if (pub == NULL || sub == NULL) {
        LOG_DBG("Invalid arguments: pub: %p, sub: %p", pub, sub);
        return -EINVAL;
    }

    int rc = 0;
    rc = k_mutex_lock(pub->mtx, K_FOREVER);
    if (rc < 0) {
        LOG_ERR("Failed to lock %s mutex (%i) %s", pub->name, rc, strerror(-rc));
        return rc;
    }
    sys_slist_append(&pub->subscribers, &sub->node);
    k_mutex_unlock(pub->mtx);
    return rc;
}

int pub_remove_subscriber(struct pub *pub, struct sub *sub)
{
    if (pub == NULL || sub == NULL) {
        LOG_DBG("Invalid arguments: pub: %p, sub: %p", pub, sub);
        return -EINVAL;
    }

    int rc = 0;
    rc = k_mutex_lock(pub->mtx, K_FOREVER);
    if (rc < 0) {
        LOG_ERR("Failed to lock %s mutex (%i) %s", pub->name, rc, strerror(-rc));
        return rc;
    }

    sys_slist_find_and_remove(&pub->subscribers, &sub->node);
    k_mutex_unlock(pub->mtx);
    return rc;
}

int pub_emit(struct pub *pub, const void *data, size_t size)
{
    if (pub == NULL || data == NULL || size == 0) {
        LOG_DBG("Invalid arguments: pub: %p, data: %p, size: %d", pub, data, size);
        return -EINVAL;
    }

    int rc = 0;
    rc = k_mutex_lock(pub->mtx, K_FOREVER);
    if (rc < 0) {
        LOG_ERR("Failed to lock %s mutex (%i) %s", pub->name, rc, strerror(-rc));
        return rc;
    }
    sys_snode_t *node = sys_slist_peek_head(&pub->subscribers);
    while (node) {
        struct sub *sub = CONTAINER_OF(node, struct sub, node);
        rc = sub_notify(sub, pub, data, size);
        if (rc < 0) {
            LOG_ERR("Failed to notify subscriber %s", sub->name);
            break;
        }
        node = sys_slist_peek_next(node);
    }
    k_mutex_unlock(pub->mtx);
    return rc;
}
