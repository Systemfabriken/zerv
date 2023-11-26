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
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2023 Systemfabriken AB
 * contact: albin@systemfabriken.tech
 *===============================================================================================*/
#ifndef _ZERV_PUB_H_
#define _ZERV_PUB_H_

/*=================================================================================================
 * INCLUDES
 *===============================================================================================*/

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

#endif /* _ZERV_PUB_H_ */