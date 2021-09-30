/**
 * @file   message.hh
 * @author Otiose email
 * @date   Thu Sep 30 11:04:31 2021
 *
 * @brief  Defines the Message struct.
 *
 * This struct encapsulates the type of a message and its respective code. It
 * is currently the only type of message supported. You will find the
 * definitions of all supported tags and codes in message-codes.hh.
 *
 */
#pragma once

struct Message {
  int tag;  /**< type of the message */
  int code; /**< code of the message  */
};
