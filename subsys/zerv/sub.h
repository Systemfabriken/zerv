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
#ifndef _SUB_H_
#define _SUB_H_

// INCLUDES ###########################################################################################################
#include "pub.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/slist.h>
#include <stdint.h>
#include <stddef.h>

// PUBLIC DECLARATIONS ################################################################################################

struct notification {
    uint32_t unused;
    const struct pub *emittor; // The publisher that emitted the notification
    size_t size;
    uint8_t data[];
} __aligned(4);

struct sub {
    sys_snode_t node;
    const char *name;
    struct k_heap *heap;
    struct k_fifo *fifo;
    struct k_mutex *mtx;
    struct notification *last_notification;
} __aligned(4);

// PUBLIC MACROS ######################################################################################################

/**
 * @brief Macro for defining a subscriber.
 * @param sub_name The name of the subscriber.
 * @param queue_mem_size The size of the queue memory.
 */
#define SUB_DEFINE(sub_name, queue_mem_size)                                                       \
    static K_HEAP_DEFINE(sub_name##_heap, queue_mem_size);                                         \
    static K_FIFO_DEFINE(sub_name##_fifo);                                                         \
    static K_MUTEX_DEFINE(sub_name##_mtx);                                                         \
    struct sub sub_name = {                                                                        \
        .name = #sub_name,                                                                         \
        .heap = &sub_name##_heap,                                                                  \
        .fifo = &sub_name##_fifo,                                                                  \
        .mtx = &sub_name##_mtx,                                                                    \
        .last_notification = NULL,                                                                 \
    };

/**
 * @brief Macro for defining a subscriber event.
 * @param event_name The name of the event.
 * @param sub_name The name of the subscriber.
 */
#define SUB_K_POLL_EVENT_INITIALIZER(sub_name)                                                     \
    K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY,      \
                                    sub_name.fifo, 0)

// PUBLIC FUNCTION DECLARATIONS #######################################################################################

/**
 * @brief Wait for data from a publisher.
 * 
 * @param[in] sub_name The name of the subscriber.
 * @param[in] timeout The timeout for waiting for data.
 * 
 * @return 0 on success, negative error code on failure.
*/
int sub_wait(struct sub *sub, k_timeout_t timeout);

/**
 * @brief Receive data from a subscriber.
 * 
 * @param[in] sub_name The name of the subscriber.
 * @param[out] recv_size Pointer to the size of the received data.
 * 
 * @return Pointer to the received data on success, NULL on failure.
 */
void *sub_receive(struct sub *sub, size_t *recv_size);

/**
 * @brief Free data from a subscriber.
 * 
 * @param[in] sub_name The name of the subscriber.
 * @param[in] buf The buffer to free.
 * 
 * @return N/A
 */
void sub_free(struct sub *sub, void *buf);

/**
 * @brief Wait for data to be received from a publisher.
 * 
 * @param[in] sub_name The name of the subscriber.
 * @param[out] buf The buffer to store the data in.
 * @param[in] buf_size The size of the buffer.
 * @param[out] recv_size The size of the received data, can be NULL.
 * @param[in] timeout The timeout for waiting for data.
 * 
 * @return Pointer to the publisher on success, NULL on failure.
*/
static inline const struct pub *sub_wait_and_receive(struct sub *sub, void *buf, size_t buf_size,
                                                     size_t *recv_size, k_timeout_t timeout)
{
    const struct pub *emittor = NULL;
    extern int sub_wait_and_receive_internal(struct sub * _sub, const struct pub **_emittor,
                                             void *_buf, size_t _size, size_t *_recv_size,
                                             k_timeout_t _timeout);
    int rc = sub_wait_and_receive_internal(sub, &emittor, buf, buf_size, recv_size, timeout);
    if (rc != 0)
        emittor = NULL;
    return emittor;
}

/**
 * @brief Send a notification to a subscriber.
 * 
 * @param[in] emittor The publisher that emitted the notification.
 * @param[in] sub The subscriber to receive the notification.
 * @param[in] _data The data to send.
 * @param[in] _data_size The size of the data to send.
 * 
 * @return 0 on success, negative error code on failure.
*/
int sub_notify(struct sub *sub, struct pub *emittor, const void *_data, size_t _data_size);

#endif // _SUB_H_
