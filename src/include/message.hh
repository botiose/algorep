/**
 * @file   message.hh
 * @author Otiose email
 * @date   Thu Sep 30 11:04:31 2021
 *
 * @brief  Defines the Message struct.
 *
 * This struct encapsulates the tag of a message and its respective code. It
 * is currently the only tag of message supported. You will find the
 * definitions of all supported tags and codes in message-codes.hh.
 *
 */
#pragma once

#include <string>

#include "message-info.hh"

class MessagePassKey {
  friend class Messenger;

private:
  MessagePassKey() = default;
};

class Message {
public:
  Message() = default;

  MessageTag
  getTag() const;

  int
  getTagInt() const;

  template <typename T>
  T
  getCode() const;

  int
  getId() const;

  std::string
  getCommand() const;

  bool
  getIsValid() const;

  void
  setTag(const MessagePassKey&, const int& tag);

  template <typename T>
  void
  setCode(const MessagePassKey&, const T& code);

  void
  setId(const MessagePassKey&, const int& id);

  void
  setCommand(const MessagePassKey&, const std::string& command);

  void
  setToValid(const MessagePassKey&);

private:
  int m_tag;
  int m_code;            /**< code of the message  */
  int m_id;              /**< unique id of the message */
  std::string m_command; /**< command of the message */

  bool m_isValid = false;
};

#include "message.hxx"
