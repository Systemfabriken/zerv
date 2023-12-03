/*=================================================================================================
 *                _____           _                 ______    _          _ _
 *               / ____|         | |               |  ____|  | |        (_) |
 *              | (___  _   _ ___| |_ ___ _ __ ___ | |__ __ _| |__  _ __ _| | _____ _ __
 *               \___ \| | | / __| __/ _ \ '_ ` _ \|  __/ _` | '_ \| '__| | |/ / _ \ '_ \
 *               ____) | |_| \__ \ ||  __/ | | | | | | | (_| | |_) | |  | |   <  __/ | | |
 *              |_____/ \__, |___/\__\___|_| |_| |_|_|  \__,_|_.__/|_|  |_|_|\_\___|_| |_|
 *                       __/ |
 *                      |___/
 * Description:
 *     In header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 Systemfabriken AB
 * contact: albin@systemfabriken.tech
 *=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include <zephyr/zerv/zerv.h>
#include <zephyr/zerv/zerv_internal.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/*=================================================================================================
 * PRIVATE MACROS
 ================================================================================================*/
LOG_MODULE_REGISTER(zerv, CONFIG_ZERV_LOG_LEVEL);

/*=================================================================================================
 * PRIVATE FUNCTION DECLARATIONS
 ================================================================================================*/

zerv_rc_t zerv_internal_client_request_handler(const zervice_t *serv, zerv_cmd_inst_t *req_instance,
					       size_t client_req_params_len,
					       const void *client_req_params, void *resp,
					       size_t resp_len)
{
	if (serv == NULL || req_instance == NULL || client_req_params == NULL || resp == NULL) {
		return ZERV_RC_NULLPTR;
	}

	// Prevent other threads from calling this service request while we are using it.
	// Critical section is used when "locking" the service request as we dont want to
	// be interrupted by the scheduler while doing this.
	k_sched_lock();
	if (atomic_get(&req_instance->is_locked)) {
		k_sched_unlock();
		return ZERV_RC_LOCKED;
	}
	atomic_set(&req_instance->is_locked, true);
	k_sched_unlock();

	LOG_DBG("Calling %s: %s", serv->name, req_instance->name);

	// Allocate the request on the service's heap and copy the request data. The request
	// is then put in the service's fifo.
	zerv_cmd_in_t *p_req_params =
		k_heap_alloc(serv->heap, client_req_params_len + sizeof(zerv_cmd_in_t), K_NO_WAIT);
	if (p_req_params == NULL) {
		LOG_DBG("Failed to allocate request params to %s: %s", serv->name,
			req_instance->name);
		atomic_set(&req_instance->is_locked, false);
		return ZERV_RC_NOMEM;
	}
	p_req_params->id = req_instance->id;
	p_req_params->resp_len = resp_len;
	p_req_params->resp = resp;
	p_req_params->client_req_params.data_len = client_req_params_len;
	memcpy(p_req_params->client_req_params.data, client_req_params, client_req_params_len);
	struct k_sem response_sem;
	int rc = k_sem_init(&response_sem, 0, 1);
	if (rc != 0) {
		k_heap_free(serv->heap, p_req_params);
		atomic_set(&req_instance->is_locked, false);
		return ZERV_RC_ERROR;
	}
	p_req_params->response_sem = &response_sem;
	k_fifo_put(serv->fifo, p_req_params);

	// Now it's time to let the client thread wait for the response from the service.
	// TODO: Add timeout. Remember to free the request from the heap and reset the fifo if the
	// timeout is reached.
	LOG_DBG("Waiting for response from %s: %s", serv->name, req_instance->name);
	rc = k_sem_take(&response_sem, K_FOREVER);
	if (rc != 0) {
		LOG_ERR("Failed to wait for response from %s", serv->name);
		k_heap_free(serv->heap, p_req_params);
		atomic_set(&req_instance->is_locked, false);
		return ZERV_RC_TIMEOUT;
	}

	LOG_DBG("Received response from %s: %s", serv->name, req_instance->name);
	rc = p_req_params->rc;
	k_heap_free(serv->heap, p_req_params);
	atomic_set(&req_instance->is_locked, false);
	return rc;
}

/*=================================================================================================
 * PUBLIC FUNCTION DEFINITIONS
 ================================================================================================*/

zerv_cmd_in_t *zerv_get_cmd_input(const zervice_t *serv, k_timeout_t timeout)
{
	if (serv == NULL) {
		return NULL;
	}

	return k_fifo_get(serv->fifo, timeout);
}

zerv_rc_t zerv_handle_request(const zervice_t *serv, zerv_cmd_in_t *req_params)
{
	if (serv == NULL || req_params == NULL) {
		return ZERV_RC_NULLPTR;
	}

	if (req_params->id >= serv->cmd_instance_cnt || req_params->response_sem == NULL ||
	    req_params->client_req_params.data_len == 0) {
		return ZERV_RC_ERROR;
	}

	LOG_DBG("Handling on %s", serv->name);
	zerv_rc_t rc = serv->cmd_instances[req_params->id]->handler(
		req_params->client_req_params.data, req_params->resp);

	if (rc < ZERV_RC_OK) {
		LOG_ERR("Failed to handle request on %s", serv->name);
	}
	req_params->rc = rc;
	k_sem_give(req_params->response_sem);
	return rc;
}

zerv_rc_t zerv_get_cmd_input_instance(const zervice_t *serv, int req_id,
				      zerv_cmd_inst_t **req_instance)
{
	if (serv == NULL || req_instance == NULL) {
		return ZERV_RC_NULLPTR;
	}

	if (req_id >= serv->cmd_instance_cnt) {
		return ZERV_RC_ERROR;
	}

	*req_instance = serv->cmd_instances[req_id];
	return ZERV_RC_OK;
}

void __zerv_cmd_processor_thread_body(const zervice_t *p_zervice)
{
	if (p_zervice == NULL) {
		LOG_ERR("Invalid arguments");
		return;
	}

	LOG_DBG("Starting command processor for %s", p_zervice->name);

	while (true) {
		zerv_cmd_in_t *p_req_params = k_fifo_get(p_zervice->fifo, K_FOREVER);
		if (p_req_params == NULL) {
			LOG_ERR("Failed to get request params from %s", p_zervice->name);
			continue;
		}

		zerv_rc_t rc = zerv_handle_request(p_zervice, p_req_params);
		if (rc != 0) {
			LOG_ERR("Failed to handle request on %s", p_zervice->name);
			continue;
		}
	}
}

void __zerv_event_processor_thread_body(const zervice_t *p_zervice, zerv_events_t *zervice_events)
{
	if (p_zervice == NULL || zervice_events == NULL) {
		return;
	}

	struct k_poll_event events[zervice_events->event_cnt];
	for (size_t i = 0; i < zervice_events->event_cnt; i++) {
		memcpy(&events[i], zervice_events->events[i]->event, sizeof(struct k_poll_event));
	}

	LOG_DBG("Starting event processor for %s, num events %d", p_zervice->name,
		zervice_events->event_cnt);

	for (size_t i = 0; i < zervice_events->event_cnt; i++) {
		if (events[i].type == K_POLL_TYPE_SIGNAL) {
			if (!events[i].signal || events[i].signal != events[i].obj) {
				LOG_ERR("Zerv k_poll_signal event is erronously initiated!");
				continue;
			}
			k_poll_signal_init(events[i].signal);
			k_poll_event_init(&events[i], K_POLL_TYPE_SIGNAL, events[i].mode,
					  events[i].signal);
			k_poll_signal_reset(events[i].signal);
		}
		events[i].state = K_POLL_STATE_NOT_READY;
	}

	while (true) {
		int rc = k_poll(events, zervice_events->event_cnt, K_FOREVER);
		if (rc != 0) {
			LOG_ERR("Failed to poll events for %s", p_zervice->name);
			continue;
		}

		// Handle the Zervice commands first
		if (events[0].state == K_POLL_TYPE_FIFO_DATA_AVAILABLE) {
			events[0].state = K_POLL_STATE_NOT_READY;
			LOG_DBG("Received request on %s", p_zervice->name);
			zerv_cmd_in_t *p_req_params = k_fifo_get(p_zervice->fifo, K_NO_WAIT);
			if (p_req_params == NULL) {
				LOG_ERR("Failed to get request params from %s", p_zervice->name);
				continue;
			}

			zerv_rc_t rc = zerv_handle_request(p_zervice, p_req_params);
			if (rc != 0) {
				LOG_ERR("Failed to handle request on %s", p_zervice->name);
				continue;
			}
		}

		// Handle the Zervice events
		LOG_DBG("Received event on %s", p_zervice->name);
		for (size_t i = 1; i < zervice_events->event_cnt; i++) {
			if (events[i].state == events[i].type) {
				zervice_events->events[i]->handler(events[i].obj);
			}
			if (events[i].type == K_POLL_TYPE_SIGNAL) {
				k_poll_signal_reset(events[i].signal);
			}
			events[i].state = K_POLL_STATE_NOT_READY;
		}
	}
}
