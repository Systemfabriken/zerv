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
 *  This file contains the public API of the zerv library. The zerv library is a library for
 *  easing the development of event driven applications on Zephyr. The library helps the developer
 *  to create a modular application, following the principles of the micro service architecture. The
 *  library provides a way to create services that can be requested by other modules in the system.
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 Systemfabriken AB
 * contact: albin@systemfabriken.tech
 *===============================================================================================*/
#ifndef _ZERV_H_
#define _ZERV_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/
#include <zephyr/zerv/zerv_internal.h>

/*=================================================================================================
 * ZERVICE MACROS
 *===============================================================================================*/

/**
 * @brief Macro for declaring a zervice in a header file.
 *
 * @param name The name of the service.
 * @param commands... The commands of the service, provided as a list of command names. The names
 * should be the same as the names used in the ZERV_CMD_DECL macro
 *
 * @note The requests must be declared before the service. The service needs to be defined in the
 * source file
 */
#define ZERV_DECL(name, commands...)                                                               \
    FOR_EACH (__ZERV_CMD_HANDLER_FN_DECL, (;), commands)                                           \
        __ZERV_DEFINE_CMD_INSTANCE_LIST(name, commands)                                            \
    enum __##name##_cmds_e{ FOR_EACH (__ZERV_CMD_ID_DECL, (, ), commands), __##name##_cmd_cnt };   \
    extern const zervice_t name

/**
 * @brief Macro for defining a thread-less zervice in a source file.
 *
 * @param zervice_name The name of the service.
 * @param heap_size The size of the heap of the service. The heap is used to store the command
 * inputs and outputs while they are being processed.
 */
#define ZERV_DEF(zervice_name, heap_size)                                                          \
    static K_HEAP_DEFINE(__##zervice_name##_heap, heap_size);                                      \
    static K_FIFO_DEFINE(__##zervice_name##_fifo);                                                 \
    static K_MUTEX_DEFINE(__##zervice_name##_mtx);                                                 \
    const zervice_t zervice_name __aligned(4) = {                                                  \
        .name = #zervice_name,                                                                     \
        .heap = &__##zervice_name##_heap,                                                          \
        .fifo = &__##zervice_name##_fifo,                                                          \
        .mtx = &__##zervice_name##_mtx,                                                            \
        .cmd_instance_cnt = __##zervice_name##_cmd_cnt,                                            \
        .cmd_instances = zervice_name##_cmd_instances,                                             \
    };

/**
 * @brief Macro for defining a zervice that is handeled by a thread. The thread will process
 * commands sent to the zervice.
 *
 * @param zervice The name of the zervice. This should be the same name as declared with the
 * ZERV_DECL macro.
 * @param heap_size The size of the heap of the zervice. The heap is used to store the command
 * inputs and outputs while they are being processed.
 * @param stack_size The size of the stack of the zervice thread.
 * @param prio The priority of the zervice thread.
 */
#define ZERV_DEF_CMD_PROCESSOR_THREAD(zervice, heap_size, stack_size, prio)                        \
    ZERV_DEF(zervice, heap_size);                                                                  \
    static K_THREAD_DEFINE(__##zervice##_thread, stack_size,                                       \
                           (k_thread_entry_t)__zerv_cmd_processor_thread_body, &zervice, NULL,     \
                           NULL, prio, 0, 0)

/**
 * @brief Macro for defining a zervice that is handled on a thread that processes both zerv commands
 * and events.
 *
 * @param zervice The name of the zervice. This should be the same name as declared with the
 * ZERV_DECL macro.
 * @param heap_size The size of the heap of the zervice. The heap is used to store the command
 * inputs and outputs while they are being processed.
 * @param stack_size The size of the stack of the zervice thread.
 * @param prio The priority of the zervice thread.
 * @param zerv_events... The events of the zervice, provided as a list of event names. The events
 * must be declared before the zervice thread.
 */
#define ZERV_EVENT_PROCESSOR_THREAD_DEF(zervice, heap_size, stack_size, prio, zerv_events...)      \
    ZERV_DEF(zervice, heap_size);                                                                  \
    static const struct k_poll_event __##zervice##_k_poll_event = K_POLL_EVENT_STATIC_INITIALIZER( \
            K_POLL_TYPE_FIFO_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &__##zervice##_fifo, 0);     \
    static zerv_event_t __zerv_event_##zervice = { .event = &__##zervice##_k_poll_event,           \
                                                   .handler = NULL,                                \
                                                   .type = 0 };                                    \
    static zerv_event_t *__##zervice##_events[] = {                                                \
        &__zerv_event_##zervice, FOR_EACH (__zerv_event_t_INIT, (, ), zerv_events)                 \
    };                                                                                             \
    static zerv_events_t __##zervice##_events_arg = {                                              \
        .events = __##zervice##_events,                                                            \
        .event_cnt = ARRAY_SIZE(__##zervice##_events),                                             \
    };                                                                                             \
    static K_THREAD_DEFINE(__##zervice##_thread, stack_size,                                       \
                           (k_thread_entry_t)__zerv_event_processor_thread_body, &zervice,         \
                           &__##zervice##_events_arg, NULL, prio, 0, 0)

/*=================================================================================================
 * ZERV COMMAND MACROS
 *===============================================================================================*/

/**
 * @brief Macro for declaring a zervice command input parameter.
 *
 * Should be used in the ZERV_CMD_DECL macro to declare the input parameters of a command.
 *
 * @param ... The types and names of the input parameters.
 */
#define ZERV_IN(...) FOR_EACH (__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

/**
 * @brief Macro for declaring an empty zervice command input parameter.
 */
#define ZERV_IN_EMPTY ZERV_IN(char __empty)

/**
 * @brief Macro for declaring a zervice command output parameter.
 *
 * Should be used in the ZERV_CMD_DECL macro to declare the output parameters of a command.
 *
 * @param ... The types and names of the output parameters.
 */
#define ZERV_OUT(...) FOR_EACH (__ZERV_IMPL_STRUCT_MEMBER, (), ##__VA_ARGS__)

/**
 * @brief Macro for declaring an empty zervice command output parameter.
 */
#define ZERV_OUT_EMPTY ZERV_OUT(char __empty)

/**
 * @brief Macro for declaring a zervice command in a header file.
 *
 * The macro should be used in the header file. A zervice command can be issued by the client
 * application to request a service from the zervice. The command is declared with a name, input
 * parameters and output parameters. The macros ZERV_IN and ZERV_OUT should be used to declare the
 * input and output parameters.
 *
 * @param name The name of the command.
 * @param in The input parameters of the command. Should be declared with the ZERV_IN macro.
 * @param out The output parameters of the command. Should be declared with the ZERV_OUT macro.
 *
 * @note The command must be defined in the source file with the ZERV_CMD_HANDLER_DEF macro.
 */
#define ZERV_CMD_DECL(name, in, out)                                                               \
    typedef struct name##_param {                                                                  \
        in                                                                                         \
    } name##_param_t;                                                                              \
    typedef void (*name##_resp_handler_t)(void);                                                   \
    typedef struct name##_ret {                                                                    \
        zerv_rc_t rc;                                                                              \
        name##_resp_handler_t on_delayed_response;                                                 \
        out                                                                                        \
    } name##_ret_t;                                                                                \
    extern zerv_cmd_inst_t __##name

/**
 * @brief Macro for defining a zervice request handler function in a source file.
 *
 * @param cmd_name The name of the request.
 * @param in Pointer to the input parameters of the request. The input type is defined by the
 * ZERV_IN macro used when declaring the command.
 * @param out Pointer to the output parameters of the request. The output type is defined by the
 * ZERV_OUT macro used when declaring the command.
 */
#define ZERV_CMD_HANDLER_DEF(cmd_name, in, out)                                                    \
    static K_SEM_DEFINE(__##cmd_name##_future_sem, 0, 1);                                          \
    static cmd_name##_ret_t __##cmd_name##_future_response;                                        \
    zerv_cmd_inst_t __##cmd_name __aligned(4) = {                                              \
		.name = #cmd_name,                                                                 \
		.id = __##cmd_name##_id,                                                           \
		.is_locked = ATOMIC_INIT(false),                                                   \
		.handler = (zerv_cmd_abstract_handler_t)__##cmd_name##_handler,                    \
		.future =                                                                          \
			{                                                                          \
				.is_active = false,                                                \
				.sem = &__##cmd_name##_future_sem,                                 \
				.req_params = NULL,                                                \
				.resp_len = sizeof(cmd_name##_ret_t),                              \
				.resp = (zerv_cmd_out_base_t *)&__##cmd_name##_future_response,    \
			},                                                                         \
	};   \
    zerv_rc_t __##cmd_name##_handler(const cmd_name##_param_t *in, cmd_name##_ret_t *out)

/*=================================================================================================
 * ZERV EVENT MACROS
 *===============================================================================================*/

/**
 * @brief Macro for declaring a k_poll_event that is to be handled by a zervice.
 *
 * @param name The name of the event.
 * @param _event_type The type of the event. Should be K_POLL_TYPE_*.
 * @param _event_mode The mode of the event. Should be K_POLL_MODE_*.
 * @param _event_obj The object of the event. Should be a pointer to a supported k_poll_event
 * object.
 */
#define ZERV_EVENT_DEF(name, _event_type, _event_mode, _event_obj)                                 \
    static const struct k_poll_event __##name##_event =                                            \
            K_POLL_EVENT_STATIC_INITIALIZER(_event_type, _event_mode, _event_obj, 0);              \
    static void __##name##_event_handler(void *obj);                                               \
    static zerv_event_t __zerv_event_##name = { .event = &__##name##_event,                        \
                                                .handler = __##name##_event_handler,               \
                                                .type = _event_type }

/**
 * @brief Macro for defining a service event handler function.
 *
 * @param name The name of the event.
 */
#define ZERV_EVENT_HANDLER_DEF(name) void __##name##_event_handler(void *obj)

/**
 * @brief Macro for initializing a k_poll_event struct for a zervice.
 *
 * @param zervice The name of the zervice.
 */
#define ZERV_K_POLL_EVENT_INITIALIZER(zervice)                                                     \
    K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY,      \
                                    &__##zervice##_fifo, 0)

/*=================================================================================================
 * ZERV PUB-SUB MACROS
 *===============================================================================================*/

/**
 * @brief Macro for declaring a zervice publisher topic in a header file.
 *
 * @param name The name of the topic.
 * @param params... The parameters of the topic. Should be declared with the ZERV_IN macro.
 */
#define ZERV_PUB_TOPIC_DECL(name, params...)

/**
 * @def ZERV_PUB_TOPIC_EMIT(name, params...)
 * @brief Macro for emitting a message on a Zerv publisher topic.
 *
 * This macro is used to emit a message on a Zerv publisher topic. The name of the topic
 * and any parameters to be included in the message are passed as arguments to the macro.
 *
 * Example usage:
 * @code{.c}
 * ZERV_PUB_TOPIC_EMIT(my_topic, "Hello, world!", 42);
 * @endcode
 *
 * @param name The name of the Zerv publisher topic to emit the message on.
 * @param params The parameters to include in the message.
 */
#define ZERV_PUB_TOPIC_EMIT(name, params...)

/*=================================================================================================
 * ZERVICE CLIENT MACROS
 *===============================================================================================*/

/**
 * @brief Macro for commanding a zervice to handle a request.
 *
 * Request service from the zervice with the given name and command name. If the call is supposed to
 * generate a response, the response will be stored in the memory pointed to by the
 * response_handle_name parameter.
 *
 * @param zervice The name of the zervice to call.
 * @param cmd The name of the command to call.
 * @param[out] retcode The identifier of the variable to store the return code in. The
 * variable is defined by the macro.
 * @param[out] p_ret The identifier of the pointer to the response storage, will be
 * NULL if no response is expected. The pointer is defined by the macro.
 * @param[in] params... The arguments to the command. The arguments should follow the
 * format specified by the ZERV_CMD_PARAM macro. The last argument should be the code block to
 * execute when the response is received. The code block should be surrounded by curly
 * brackets.
 */
#define ZERV_CALL(zervice, cmd, retcode, p_ret, params...)                                         \
    cmd##_ret_t __##cmd##_response;                                                                \
    cmd##_ret_t *p_ret = &__##cmd##_response;                                                      \
    zerv_rc_t retcode = zerv_internal_client_request_handler(                                      \
            &zervice, &__##cmd, sizeof(cmd##_param_t), &(cmd##_param_t){ params },                 \
            (zerv_cmd_out_base_t *)p_ret, sizeof(cmd##_ret_t));

/*=================================================================================================
 * PUBLIC FUNCTION DECLARATIONS
 *===============================================================================================*/

/**
 * @brief Wait for a service request to be available and return it.
 *
 * @param[in] serv The service to wait for a request on.
 * @param[in] timeout The timeout to wait for a request.
 *
 * @return Pointer to the request parameters if a request was received, NULL if the timeout was
 * reached.
 */
zerv_cmd_in_t *zerv_get_cmd_input(const zervice_t *serv, k_timeout_t timeout);

/**
 * @brief Used from the service thread to handle a request.
 *
 * @param[in] serv The service to handle the request on.
 * @param[in] req The request to handle.
 *
 * @return ZERV_RC return code from the request handler function.
 */
zerv_rc_t zerv_handle_request(const zervice_t *serv, zerv_cmd_in_t *req);

/**
 * @brief Get the id of a request.
 *
 * @param[in] cmd_name The name of the request.
 *
 * @return The id of the request.
 */
#define zerv_get_cmd_input_id(cmd_name) __ZERV_CMD_ID_DECL(cmd_name)

/**
 * @brief Used from the service thread to get a request instance that the request parameters are
 * aimed for.
 *
 * @param[in] serv The service to get the request instance from.
 * @param[in] req_params The request parameters.
 * @param[out] req_instance The request instance that the request parameters are aimed for.
 *
 * @return  ZERV_RC return code.
 */
zerv_rc_t zerv_get_cmd_input_instance(const zervice_t *serv, int req_id,
                                      zerv_cmd_inst_t **req_instance);

/**
 * @brief Used from the service thread to initialize a future.
 *
 * @param[in] serv The service to initialize the future on.
 * @param[in] req_instance The request instance that the future is for.
 * @param[in] req_params The request parameters.
 *
 * @return N/A
 */
zerv_rc_t zerv_future_init(const zervice_t *serv, zerv_cmd_inst_t *req_instance,
                           zerv_cmd_in_t *req_params);

/**
 * @brief Check if a future request has a delayed response.
 */
#define zerv_future_is_active(cmd_name) __##cmd_name.future.is_active

/**
 * @brief Define a pointer to the request parameters of a future request.
 *
 * @param[in] cmd_name The name of the request.
 * @param[in] req_params_handle_name The name of the pointer to the request parameters.
 *
 * @return N/A
 *
 * @note This macro should be used in the service thread to get the request parameters of a future
 * request.
 */
#define zerv_future_get_params(cmd_name, req_params_handle_name)                                   \
    cmd_name##_param_t *req_params_handle_name =                                                   \
            (cmd_name##_param_t *)&__##cmd_name.future.req_params->client_req_params.data

/**
 * @brief Get the response storage of a future request.
 *
 * @param[in] cmd_name The name of the request.
 * @param[in] resp_handle_name The name of the pointer, to be created, to the response storage.
 *
 * @return N/A
 */
#define zerv_future_get_response(cmd_name, resp_handle_name)                                       \
    cmd_name##_ret_t *resp_handle_name = (cmd_name##_ret_t *)__##cmd_name.future.resp

/**
 * @brief Send response to a delayed request.
 *
 * @param[in] serv The service to respond on.
 * @param[in] req_id The id of the request.
 *
 * @return N/A
 */
#define zerv_future_signal_response(cmd_name, return_code)                                         \
    _zerv_future_signal_response(&__##cmd_name, return_code)

/**
 * @brief Calls a service request.
 *
 * @param zervice The name of the service.
 * @param cmd_name The name of the request.
 * @param p_req_params Pointer to the parameters to the request.
 * @param p_response_dst Pointer to the destination of the response.
 *
 * @return ZERV_RC_OK if the request was successfully called, ZERV_RC_FUTURE if the response will be
 * delayed ZERV_RC_ERROR if the request failed.
 *
 * @example
 *   future_echo_ret_t future_echo_resp = { 0 };
 *   zerv_rc_t rc = zerv_call(future_service, future_echo,
 *                                 (&(future_echo_param_t){ .is_delayed = false, .str = "Hello
 * World!" }), &future_echo_resp);
 *
 * In this example we call a request called "future echo" on a service called "future service". If
 * the response is delayed this call will return ZERV_RC_FUTURE and the response will be received
 * later. If the response is not delayed the response will be received immediately and the call will
 * return the result of the request handler function.
 */
#define zerv_call(zervice, cmd_name, p_req_params, p_response_dst)                                 \
    zerv_internal_client_request_handler(&zervice, &__##cmd_name, sizeof(cmd_name##_param_t),      \
                                         p_req_params, (zerv_cmd_out_base_t *)p_response_dst,      \
                                         sizeof(cmd_name##_ret_t));

/**
 * @brief Get the future response of a request.
 *
 * @param zervice The name of the service.
 * @param cmd_name The name of the request.
 * @param p_response_dst Pointer to the destination of the response.
 * @param timeout The timeout of the wait for the response.
 *
 * @return ZERV_RC_OK if the response was successfully received, ZERV_RC_TIMEOUT if the response
 * timed out and ZERV_RC_ERROR if the response failed.
 *
 * @example
 *   future_echo_ret_t future_echo_resp = { 0 };
 *   zerv_rc_t rc = zerv_get_future(future_service, future_echo, &future_echo_resp, K_MSEC(100));
 *
 * In this example we get the response of a request called "future echo" on a service called "future
 * service". If the response is not received within 100 milliseconds the call will return
 * ZERV_RC_TIMEOUT and the response will not be received. If the response is received within 100
 * milliseconds the call will return the result of the request handler function and the response
 * will be stored in future_echo_resp.
 */
#define zerv_get_future(zervice, cmd_name, p_response_dst, timeout)                                \
    zerv_internal_get_future_resp(&zervice, &__##cmd_name, p_response_dst, timeout)

#endif // _ZERV_H_
