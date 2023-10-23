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
#include "sub.h"
#include "pub.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sub, CONFIG_PUBSUB_LOG_LEVEL);

// PRIVATE DECLARATIONS ###############################################################################################

/**
 * @brief Free a notification from a publisher.
 * @param[in] _sub The subscriber to receive the notification.
 * @param[in] _buf The buffer to free.
*/
static void sub_free_internal(struct sub *_sub);

// PUBLIC FUNCTION DEFINITIONS ########################################################################################

/**
 * @brief Send a notification to a subscriber.
 * @param[in] emittor The publisher that emitted the notification.
 * @param[in] sub The subscriber to receive the notification.
 * @param[in] _data The data to send.
 * @param[in] _data_size The size of the data to send.
 * @return 0 on success, negative error code on failure.
*/
int sub_notify(struct sub *sub, struct pub *emittor, const void *_data, size_t _data_size)
{
    if (sub == NULL || _data == NULL || _data_size == 0) {
        LOG_DBG("Invalid arguments: sub: %p, data: %p, data_size: %d", sub, _data, _data_size);
        return -EINVAL;
    }

    LOG_DBG("Sending notification from publisher %s @ %p to subscriber %s @ %p with size %d",
            emittor->name, emittor, sub->name, sub, _data_size);
    k_mutex_lock(sub->mtx, K_FOREVER);

#if defined(CONFIG_ZTEST) && defined(CONFIG_SYS_HEAP_RUNTIME_STATS)
    struct sys_memory_stats memstat;
    sys_heap_runtime_stats_get(&sub->heap->heap, &memstat);
#endif

    struct notification *p_msg =
            k_heap_alloc(sub->heap, _data_size + sizeof(struct notification), K_NO_WAIT);

#if defined(CONFIG_ZTEST) && defined(CONFIG_SYS_HEAP_RUNTIME_STATS)
    struct sys_memory_stats post_memstat;
    sys_heap_runtime_stats_get(&sub->heap->heap, &post_memstat);
    LOG_DBG("%s heap diff after alloc: %i heap usage: %d bytes of %d total -> %d %%", sub->name,
            (int)post_memstat.free_bytes - (int)memstat.free_bytes, post_memstat.allocated_bytes,
            sub->heap->heap.init_bytes,
            (post_memstat.allocated_bytes * 100) / sub->heap->heap.init_bytes);
#endif

    if (p_msg == NULL) {
        LOG_ERR("Failed to allocate message for subscriber %s", sub->name);
        k_mutex_unlock(sub->mtx);
        return -ENOMEM;
    }

    p_msg->emittor = emittor;
    p_msg->size = _data_size;
    memcpy(p_msg->data, _data, _data_size);
    k_fifo_put(sub->fifo, p_msg);
    k_mutex_unlock(sub->mtx);

    LOG_DBG("Publisher %s @ %p sending notification to subscriber %s @ %p with size %d",
            p_msg->emittor->name, p_msg->emittor, sub->name, sub, p_msg->size);

    return 0;
}

int sub_wait(struct sub *sub, k_timeout_t timeout)
{
    if (sub == NULL) {
        return -EINVAL;
    } else {
        struct notification *notification = k_fifo_get(sub->fifo, timeout);
        if (notification == NULL) {
            return -EAGAIN;
        }

        sub->last_notification = notification;

        LOG_DBG("Received notification from publisher %s @ %p to subscriber %s @ %p with size %d",
                sub->last_notification->emittor->name, sub->last_notification->emittor, sub->name,
                sub, sub->last_notification->size);

        return 0;
    }
}

void *sub_receive(struct sub *sub, size_t *recv_size)
{
    if (sub == NULL) {
        return NULL;
    } else {
        // It might be that the application was signalled through the polling API
        // that a notification is available, but the application has not yet called
        // sub_receive_internal() to actually receive the notification.
        if (sub->last_notification == NULL) {
            sub->last_notification = k_fifo_get(sub->fifo, K_NO_WAIT);
            if (sub->last_notification == NULL) {
                return NULL;
            }
        }

        if (recv_size) {
            *recv_size = sub->last_notification->size;
        }
        return sub->last_notification->data;
    }
}

void sub_free(struct sub *sub, void *buf)
{
    if (k_mutex_lock(sub->mtx, K_FOREVER) < 0) {
        LOG_ERR("Failed to lock mutex for subscriber %s", sub->name);
        return;
    }

#if defined(CONFIG_ZTEST) && defined(CONFIG_SYS_HEAP_RUNTIME_STATS)
    struct sys_memory_stats memstat;
    sys_heap_runtime_stats_get(&sub->heap->heap, &memstat);
#endif

    if (sub->last_notification->data == buf) {
        sub->last_notification = NULL;
    }

    struct notification *notification =
            (struct notification *)CONTAINER_OF(buf, struct notification, data);

    k_heap_free(sub->heap, notification);

#if defined(CONFIG_ZTEST) && defined(CONFIG_SYS_HEAP_RUNTIME_STATS)
    struct sys_memory_stats post_memstat;
    sys_heap_runtime_stats_get(&sub->heap->heap, &post_memstat);
    LOG_DBG("%s heap diff after free: %i heap usage: %d bytes of %d total -> %d %%", sub->name,
            (int)post_memstat.free_bytes - (int)memstat.free_bytes, post_memstat.allocated_bytes,
            sub->heap->heap.init_bytes,
            (post_memstat.allocated_bytes * 100) / sub->heap->heap.init_bytes);
#endif

    k_mutex_unlock(sub->mtx);
}

// PRIVATE FUNCTION DEFINITIONS #######################################################################################

void sub_free_internal(struct sub *_sub)
{
    if (_sub == NULL) {
        return;
    } else {
        k_mutex_lock(_sub->mtx, K_FOREVER);

#if defined(CONFIG_ZTEST) && defined(CONFIG_SYS_HEAP_RUNTIME_STATS)
        struct sys_memory_stats memstat;
        sys_heap_runtime_stats_get(&_sub->heap->heap, &memstat);
#endif

        k_heap_free(_sub->heap, _sub->last_notification);

#if defined(CONFIG_ZTEST) && defined(CONFIG_SYS_HEAP_RUNTIME_STATS)
        struct sys_memory_stats post_memstat;
        sys_heap_runtime_stats_get(&_sub->heap->heap, &post_memstat);
        LOG_DBG("%s heap diff after free: %i heap usage: %d bytes of %d total -> %d %%", _sub->name,
                (int)post_memstat.free_bytes - (int)memstat.free_bytes,
                post_memstat.allocated_bytes, _sub->heap->heap.init_bytes,
                (post_memstat.allocated_bytes * 100) / _sub->heap->heap.init_bytes);
#endif

        k_mutex_unlock(_sub->mtx);
        _sub->last_notification = NULL;
    }
}

/**
 * @brief Wait for a notification from a publisher.
 * @param[in] _sub The subscriber to receive the notification.
 * @param[out] _emittor The publisher that emitted the notification.
 * @param[out] _buf The buffer to store the notification data in.
 * @param[in] _size The size of the buffer.
 * @param[out] _recv_size The size of the received notification data, can be NULL.
 * @param[in] _timeout The timeout for waiting for the notification. 
*/
int sub_wait_and_receive_internal(struct sub *_sub, const struct pub **_emittor, void *_buf,
                                  size_t _size, size_t *_recv_size, k_timeout_t _timeout)
{
    if (_sub == NULL || _buf == NULL || _size == 0) {
        return -EINVAL;
    } else {
        int rc;
        if ((rc = sub_wait(_sub, _timeout)) < 0) {
            return rc;
        }

        if (sub_receive(_sub, _recv_size)) {
            struct notification *data = _sub->last_notification;
            size_t sz = MIN(_size, data->size);
            if (_recv_size) {
                *_recv_size = sz;
            }
            memcpy(_buf, data->data, sz);
            if (_emittor) {
                *_emittor = data->emittor;
            }
            sub_free_internal(_sub);
        } else {
            return -EAGAIN;
        }
    }
    return 0;
}
