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
#ifndef _PUB_H_
#define _PUB_H_

// INCLUDES ###########################################################################################################
#include <zephyr/kernel.h>
#include <zephyr/sys/slist.h>
#include <stdint.h>
#include <stddef.h>

struct sub; // Forward declaration.

// PUBLIC DECLARATIONS ################################################################################################

struct pub {
    const char *name;
    struct k_mutex *mtx;
    sys_slist_t subscribers;
};

// PUBLIC MACROS ######################################################################################################

/**
 * @brief Macro for defining a publisher.
 * @param pub_name The name of the publisher.
 */
#define PUB_DEFINE(pub_name)                                                                       \
    K_MUTEX_DEFINE(pub_##pub_name##_mtx);                                                          \
    struct pub pub_name = { .name = #pub_name,                                                     \
                            .mtx = &pub_##pub_name##_mtx,                                          \
                            .subscribers = { NULL } };

/**
 * @brief Macro for declaring a publisher.
 * @param pub_name The name of the publisher.
 */
#define PUB_DECLARE(pub_name) extern struct pub pub_name;

// PUBLIC FUNCTION DECLARATIONS #######################################################################################

/**
 * @brief Add a subscriber to a publisher.
 * 
 * @param pub The publisher.
 * @param sub The subscriber.
 * 
 * @return 0 on success, negative errno otherwise.
 */
int pub_add_subscriber(struct pub *pub, struct sub *sub);

/**
 * @brief Remove a subscriber from a publisher.
 * 
 * @param pub The publisher.
 * @param sub The subscriber.
 * 
 * @return 0 on success, negative errno otherwise.
 */
int pub_remove_subscriber(struct pub *pub, struct sub *sub);

/**
 * @brief Emit data to all subscribers of a publisher.
 * 
 * @param pub The publisher.
 * @param data The data to emit.
 * @param size The size of the data.
 * 
 * @return 0 on success, negative errno otherwise.
 */
int pub_emit(struct pub *pub, const void *data, size_t size);

#endif // _PUB_H_
